#pragma once
extern bool is_shutting_down;

class ConnectionData
{
public:
    ConnectionData()
    {
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
    void update()
    {
        auto connected_clients = m_connections_shared_memory->get_connected_clients();

        // Wait for event to be signaled
        WaitForSingleObject(m_connections_shared_memory->get_event_handle(), INFINITE);
        if (is_shutting_down)
            return;
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

        for (auto& name : diff)
        {
            // Remove dropped connections and remove shared memory
            if (! new_connected_shared_memory_names.contains(name))
            {
                m_client_shared_memories.erase(name);
                shared_memory_object::remove(name.c_str());
                m_connected_clients_shared_memory_names.erase(name);
            }
            // Add new connections and create_or_open shared memory for client
            else
            {
                m_connected_clients_shared_memory_names.insert(name);

                auto p_new_client_shared_memory = std::make_unique<ClientSharedMemory>();
                p_new_client_shared_memory->init(name);
                m_client_shared_memories.insert({name, std::move(p_new_client_shared_memory)});
            }
        }
        // Assert that our locally cached set of connection match the one in shared memory
        assert(m_connected_clients_shared_memory_names == new_connected_shared_memory_names);
    }

    // Get the connected clients emails. This is not neccesarily up to date.
    // Call update first to update.
    const std::set<std::string>& get_connected_client_emails()
    {
        return m_connected_clients_shared_memory_names;
    };

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
    void run()
    {
        while (! is_shutting_down)
        {
            // Blocking until connection_data is changed.
            connection_data.update();
        }
    }

    ConnectionData connection_data;

private:
    uint32_t m_sleep_ms = 100;
};
