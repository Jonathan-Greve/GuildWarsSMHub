#pragma once
extern bool is_shutting_down;

class InstanceParty
{
public:
    InstanceParty(const std::unordered_map<std::string, SM::ClientData*>& client_datas,
                  const std::unordered_set<std::string>& client_names_in_party)
        : m_client_datas(client_datas)
        , m_client_names_in_party(client_names_in_party)
    {
    }

    void run()
    {
        while (! is_shutting_down && m_client_names_in_party.size() > 0)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

private:
    const std::unordered_map<std::string, SM::ClientData*>& m_client_datas;
    const std::unordered_set<std::string>& m_client_names_in_party;
};
