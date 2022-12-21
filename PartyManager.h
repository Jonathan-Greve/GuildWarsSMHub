#pragma once
#include "InstanceParty.h"
extern bool is_shutting_down;

// <instance_id, party_id>
using InstancePartyId = std::pair<uint32_t, uint32_t>;

class ConnectionData
{
public:
    ConnectionData()
    {
        //shared_memory_object::remove("connections");
        m_connections_shared_memory = std::make_unique<ConnectionsSharedMemory>();
        m_connections_shared_memory->init();

        // Signal event so we always read the connection data
        // the first time update() is called.
        SetEvent(m_connections_shared_memory->get_event_handle());
    }

    void terminate()
    {
        // update() might be waiting on the event so release it.
        SetEvent(m_connections_shared_memory->get_event_handle());
    }

    // Update data. Blocking until connection data changes.
    std::pair<std::vector<std::string>, std::vector<std::string>> update(DWORD event_timeout_duration)
    {
        auto connected_clients = m_connections_shared_memory->get_connected_clients();

        // Wait for event to be signaled
        auto wait_result =
          WaitForSingleObject(m_connections_shared_memory->get_event_handle(), event_timeout_duration);
        if (is_shutting_down || wait_result != WAIT_OBJECT_0)
            return {};
        // Then lock mutex before accessing shared memory data
        WaitForSingleObject(m_connections_shared_memory->get_mutex_handle(), INFINITE);

        // Convert data from char_string_set to the more common std::set representation.
        auto new_data = connected_clients->get_connected_shared_memory_names();
        std::set<std::string> new_connected_shared_memory_names;
        for (const auto& char_string_name : new_data)
        {
            std::string name(char_string_name.begin(), char_string_name.end());
            new_connected_shared_memory_names.insert(name);
        }
        ResetEvent(m_connections_shared_memory->get_event_handle());
        ReleaseMutex(m_connections_shared_memory->get_mutex_handle());

        // Get set difference between new and locally cached connection names.
        std::vector<std::string> diff;
        std::set_symmetric_difference(
          new_connected_shared_memory_names.begin(), new_connected_shared_memory_names.end(),
          m_connected_clients_shared_memory_names.begin(), m_connected_clients_shared_memory_names.end(),
          std::inserter(diff, diff.begin()));

        std::vector<std::string> connected_client_names;
        std::vector<std::string> disconnected_client_names;
        for (const auto& name : diff)
        {
            // Remove dropped connections and remove shared memory
            if (! new_connected_shared_memory_names.contains(name))
            {
                m_client_shared_memories[name].get()->get().destroy<SM::ClientData>(unique_instance);
                m_client_shared_memories.erase(name);
                shared_memory_object::remove(name.c_str());
                m_connected_clients_shared_memory_names.erase(name);

                disconnected_client_names.push_back(name);
            }
            // Add new connections and create_or_open shared memory for client
            else
            {
                m_connected_clients_shared_memory_names.insert(name);

                auto p_new_client_shared_memory = std::make_unique<ClientSharedMemory>();
                p_new_client_shared_memory->init(name);
                m_client_shared_memories.insert({name, std::move(p_new_client_shared_memory)});

                connected_client_names.push_back(name);
            }
        }
        // Assert that our locally cached set of connection match the one in shared memory
        assert(m_connected_clients_shared_memory_names == new_connected_shared_memory_names);

        return {connected_client_names, disconnected_client_names};
    }

    // Get the connected clients emails. This is not neccesarily up to date.
    // Call update first to update.
    const std::set<std::string>& get_connected_clients_shared_memory_names()
    {
        return m_connected_clients_shared_memory_names;
    };

    ClientSharedMemory* get_client_shared_memory(std::string client_shared_memory_name)
    {
        const auto it = m_client_shared_memories.find(client_shared_memory_name);
        if (it != m_client_shared_memories.end())
            return it->second.get();

        return nullptr;
    }

private:
    std::unique_ptr<ConnectionsSharedMemory> m_connections_shared_memory;

    std::set<std::string> m_connected_clients_shared_memory_names;
    std::unordered_map<std::string, std::unique_ptr<ClientSharedMemory>> m_client_shared_memories;
};

class PartyManager
{
public:
    // Main function. Should run in its own thread.
    // Exists when the program terminates.
    void terminate()
    {
        connection_data.terminate();
        for (auto& [_, thread] : party_id_to_party_thread)
        {
            thread.join();
        }
    }
    void run()
    {

        while (! is_shutting_down)
        {
            // Blocking until connection_data is changed or timeout is reached.
            const auto [connected_client_names, disconnected_client_names] =
              connection_data.update(timeout_duration_ms);

            // Update client_datas
            for (const auto& name : connected_client_names)
            {
                add_client_data(name);
            }

            for (const auto& name : disconnected_client_names)
            {
                remove_client_data(name);
            }

            // PartyId might not be valid yet such as if the dll was injected on
            // the login screen, loading screen, character select menu etc.
            for (const auto& [name, client_data] : client_datas)
            {
                InstancePartyId party_id = {0, 0};
                if (! is_party_id_valid(party_id))
                    continue;

                // If the client is already in a party and it is different then remove them from the
                // current party thread and add them to the thread for the clients current party.
                // Otherwise continue without doing anything.
                const auto curr_party_it = client_name_to_party_id.find(name);
                if (curr_party_it != client_name_to_party_id.end())
                {
                    if (curr_party_it->second == party_id)
                        continue;

                    // The client is currently in a different party so we must remove them.
                    // 1) Remove from its current party thread.
                    party_id_to_client_names.find(curr_party_it->second)->second.erase(name);
                    party_id_to_party_thread[curr_party_it->second].join();
                    party_id_to_client_names.erase(curr_party_it->second);

                    party_id_to_party_thread_objects.erase(curr_party_it->second);
                    party_id_to_party_thread.erase(curr_party_it->second);
                    client_name_to_party_id.erase(name);
                }

                // At this point the player is not in a party. So we must add them.
                client_name_to_party_id.insert({name, party_id});
                const auto it = party_id_to_client_names.find(party_id);
                if (it == party_id_to_client_names.end())
                {
                    party_id_to_client_names.insert({party_id, std::unordered_set<std::string>()});
                }
                party_id_to_client_names[party_id].insert(name);

                // Create party thread if it does not already exist
                const auto it2 = party_id_to_party_thread_objects.find(party_id);
                if (it2 == party_id_to_party_thread_objects.end())
                {
                    auto it3 =
                      party_id_to_party_thread_objects
                        .insert({party_id, InstanceParty(client_datas, party_id_to_client_names[party_id])})
                        .first;
                    auto new_party_thread = std::thread(&InstanceParty::run, &it3->second);
                    party_id_to_party_thread.insert({party_id, std::move(new_party_thread)});
                }
            }
        }
    }
    const SM::ClientData* const get_client_data(std::string client_name) const
    {
        const auto it = client_datas.find(client_name);
        if (it != client_datas.end())
            return it->second;

        return nullptr;
    }

    const std::unordered_map<std::string, SM::ClientData*>& get_client_datas(std::string client_name)
    {
        return client_datas;
    }

    ConnectionData connection_data;

private:
    const DWORD timeout_duration_ms = 1000;

    std::unordered_map<std::string, SM::ClientData*> client_datas;

    std::map<InstancePartyId, InstanceParty> party_id_to_party_thread_objects;
    std::map<InstancePartyId, std::thread> party_id_to_party_thread;

    std::map<InstancePartyId, std::unordered_set<std::string>> party_id_to_client_names;
    std::unordered_map<std::string, InstancePartyId> client_name_to_party_id;

    // Each client has a <InstanceId, PartyId> pair which uniquely identifies a party.
    // This data structure keeps track of which players are in each party and their
    // Corresponding party_thread.
    std::unordered_map<int, std::unordered_map<int, std::pair<std::vector<string>, std::thread>>>
      existing_parties;

    bool is_party_id_valid(InstancePartyId party_id) { return party_id.first > 0 && party_id.second > 0; }

    void remove_client_data(const std::string& name) { client_datas.erase(name); }

    void add_client_data(const std::string& name)
    {
        auto client_shared_memory = connection_data.get_client_shared_memory(name);
        assert(name == client_shared_memory->get_sm_name());

        auto new_client_data =
          client_shared_memory->get().find_or_construct<SM::ClientData>(unique_instance)();
        client_datas.insert({name, new_client_data});
    }
};
