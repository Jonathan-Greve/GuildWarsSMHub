#pragma once
extern bool is_shutting_down;

class ConnectionData
{
public:
    ConnectionData()
    {
        //shared_memory_object::remove("connections");
        m_connections_managed_shared_memory = managed_shared_memory(open_or_create, "connections", 65536);

        m_connected_clients =
          m_connections_managed_shared_memory.find<ConnectedClients>(unique_instance).first;
        if (! m_connected_clients)
        {
            void_allocator void_alloc(m_connections_managed_shared_memory.get_segment_manager());
            m_connected_clients =
              m_connections_managed_shared_memory.construct<ConnectedClients>(unique_instance)(void_alloc);
        }
    }

    void terminate() { m_connected_clients->cond_connection_changed.notify_all(); }

    // Update data. Blocking until connection data changes.
    void update()
    {
        //m_connections_managed_shared_memory.destroy_ptr(m_connected_clients);
        //shared_memory_object::remove("connections");
        if (m_connected_clients)
        {
            scoped_lock<interprocess_mutex> lock(m_connected_clients->m_mutex);
            if (m_connected_shared_memory_names.size() ==
                m_connected_clients->get_connected_shared_memory_names().size())
            {
                m_connected_clients->cond_connection_changed.wait(lock);
            }
            if (is_shutting_down)
                return;

            auto new_data = m_connected_clients->get_connected_shared_memory_names();
            std::set<std::string> new_connected_shared_memory_names;
            for (const auto& char_string_name : new_data)
            {
                std::string name(char_string_name.begin(), char_string_name.end());
                new_connected_shared_memory_names.insert(name);
            }

            // Get set difference between new and locally cached connection names.
            std::vector<std::string> diff;
            std::set_symmetric_difference(
              new_connected_shared_memory_names.begin(), new_connected_shared_memory_names.end(),
              m_connected_shared_memory_names.begin(), m_connected_shared_memory_names.end(),
              std::inserter(diff, diff.begin()));

            for (const auto& name : diff)
            {
                // Remove dropped connections and remove shared memory
                if (new_connected_shared_memory_names.find(name) == new_connected_shared_memory_names.end())
                {
                    m_client_shared_memories.erase(name);
                    shared_memory_object::remove(name.c_str());
                    m_connected_shared_memory_names.erase(name);
                }
                // Add new connections and create_or_open shared memory for client
                else if (m_connected_shared_memory_names.find(name) == m_connected_shared_memory_names.end())
                {
                    m_connected_shared_memory_names.insert(name);
                    m_client_shared_memories.emplace(
                      name, std::make_unique<managed_shared_memory>(open_or_create, name.c_str(), 65536));
                }
            }
            // Assert that our locally cached set of connection match the one in shared memory
            assert(m_connected_shared_memory_names == new_connected_shared_memory_names);
        }
    }

    // Get the connected clients emails. This is not neccesarily up to date.
    // Call update first to update.
    const std::set<std::string>& get_connected_client_emails() { return m_connected_shared_memory_names; };

private:
    std::set<std::string> m_connected_shared_memory_names;

    managed_shared_memory m_connections_managed_shared_memory;
    ConnectedClients* m_connected_clients;

    std::map<std::string, std::unique_ptr<managed_shared_memory>> m_client_shared_memories;
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
