#pragma once
class ConnectionData
{
public:
    std::vector<string> connected_clients;
};

class PartyManager
{
    ConnectionData m_connection_data;

    bool m_is_shutting_down;

public:
    // Main function. Spawned from the main thread.
    // Exists when the program terminates.
    PartyManager operator()()
    {
        while (! m_is_shutting_down)
        {
            std::this_thread::sleep_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1000));
        }
    }
};
