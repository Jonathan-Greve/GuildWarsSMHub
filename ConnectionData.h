#pragma once
extern bool is_shutting_down;

class ConnectionData
{
public:
    ConnectionData()
    {
        // Signal event so we always read the connection data
        // the first time update() is called.
        SetEvent(m_connections_shared_memory.get_event_handle());
    }

    void terminate()
    {
        // update() might be waiting on the event so release it.
        SetEvent(m_connections_shared_memory.get_event_handle());

        //for (auto& [_, sm] : client_sm)
        //    sm.terminate();
    }

    // Update data. Blocking until connection data changes.
    void run()
    {
        while (! is_shutting_down)
        {
            // Wait for event to be signaled
            auto wait_result = WaitForSingleObject(m_connections_shared_memory.get_event_handle(), INFINITE);
            if (is_shutting_down || wait_result != WAIT_OBJECT_0)
                return;

            // Then lock mutex before accessing shared memory data
            WaitForSingleObject(m_connections_shared_memory.get_mutex_handle(), INFINITE);
            auto connection_ids = m_connections_shared_memory.get_connections_ids();
            ResetEvent(m_connections_shared_memory.get_event_handle());
            ReleaseMutex(m_connections_shared_memory.get_mutex_handle());

            std::set<std::string> new_connection_ids(connection_ids.begin(), connection_ids.end());

            // Get set difference between new and locally cached connection names.
            std::vector<std::string> diff;
            std::set_symmetric_difference(new_connection_ids.begin(), new_connection_ids.end(),
                                          m_connection_ids.begin(), m_connection_ids.end(),
                                          std::inserter(diff, diff.begin()));

            for (const auto& id : diff)
            {
                // Remove dropped connections and remove shared memory
                if (! new_connection_ids.contains(id))
                {
                    {
                        std::scoped_lock lock(connection_ids_mutex_);
                        m_connection_ids.erase(id);
                    }
                    //client_sm[id].terminate();
                    client_sm.erase(id);
                }
                // Add new connections and create_or_open shared memory for client
                else
                {
                    {
                        std::scoped_lock lock(connection_ids_mutex_);
                        m_connection_ids.insert(id);
                    }
                    client_sm.try_emplace(id, GWIPC::SharedMemory(id, GWIPC::CLIENTDATA_SIZE));
                }
            }
            // Assert that our locally cached set of connection match the one in shared memory
            assert(m_connection_ids == new_connection_ids);
        }
    }

    const std::set<std::string> get_connected_client_ids()
    {
        std::scoped_lock lock(connection_ids_mutex_);
        return m_connection_ids;
    };

    const GWIPC::ClientData* GetClientData(std::string connection_id)
    {
        const auto sm = get_client_shared_memory(connection_id);
        if (sm)
        {
            auto sm_data = static_cast<uint8_t*>(sm->data());
            if (sm_data)
            {
                const auto it = client_data_bufs.find(connection_id);
                if (it == client_data_bufs.end())
                {
                    client_data_bufs.insert({connection_id, std::vector<uint8_t>(GWIPC::CLIENTDATA_SIZE)});
                }

                // Copy the shared memory to a local buffer
                {
                    GWIPC::SharedMemoryLock lock(*sm);
                    memcpy(client_data_bufs[connection_id].data(), sm->data(), GWIPC::CLIENTDATA_SIZE);
                }

                auto client_data = GWIPC::GetClientData(client_data_bufs[connection_id].data());

                return client_data;
            }
        }

        return nullptr;
    }

private:
    std::mutex connection_ids_mutex_;

    GWIPC::ConnectionManager m_connections_shared_memory;

    // Hold the shared memory objects containing the client data
    std::unordered_map<std::string, GWIPC::SharedMemory> client_sm;

    std::unordered_map<std::string, std::vector<uint8_t>> client_data_bufs;

    // Contains the email addresses of the connected clients.
    std::set<std::string> m_connection_ids;

    GWIPC::SharedMemory* get_client_shared_memory(std::string connection_id)
    {
        const auto it = client_sm.find(connection_id);
        if (it != client_sm.end())
        {
            return &(it->second);
        }

        return nullptr;
    }
};
