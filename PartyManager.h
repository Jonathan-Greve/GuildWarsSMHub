#pragma once
class ConnectionData
{
public:
    ConnectionData()
    {
        m_connections_managed_shared_memory = managed_shared_memory(open_or_create, "connections", 65536);
        void_allocator void_alloc(m_connections_managed_shared_memory.get_segment_manager());
        m_connected_clients = std::unique_ptr<ConnectedClients>(
          m_connections_managed_shared_memory.find_or_construct<ConnectedClients>(unique_instance)(
            void_alloc));

        update();
    }

    // Update data
    void update()
    {
        std::unique_lock lock(m_shared_mutex);
        if (m_connected_clients)
        {
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
    const std::set<std::string>& get_connected_client_emails()
    {
        std::shared_lock shared_lock(m_shared_mutex);
        return m_connected_shared_memory_names;
    };

private:
    std::set<std::string> m_connected_shared_memory_names;
    std::shared_mutex m_shared_mutex;

    managed_shared_memory m_connections_managed_shared_memory;
    std::unique_ptr<ConnectedClients> m_connected_clients;

    std::map<std::string, std::unique_ptr<managed_shared_memory>> m_client_shared_memories;
};

class PartyManager
{
public:
    // Main function. Should run in its own thread.
    // Exists when the program terminates.
    void run()
    {
        while (! m_is_shutting_down)
        {
            connection_data.update();

            // Don't waste resources reading the SM data too often. Since it doesn't change that often.
            // TODO: Implement a shared memory condition variable that is notified when a connection is
            // added or dropped.
            std::this_thread::sleep_until(std::chrono::steady_clock::now() +
                                          std::chrono::milliseconds(m_sleep_ms));
        }
    }

    ConnectionData connection_data;

private:
    bool m_is_shutting_down = false;

    uint32_t m_sleep_ms = 100;
};
