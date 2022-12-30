#pragma once
#include "PartyManager.h"
#include "ConnectionData.h"

struct Character
{
    std::string name;
    float health;
    float mana;
    std::array<std::string, 8> skills;
};

// Show the number of clients connected and sharing data.
// Also show their email addresses and uptime.
class ShowClientsConnected
{
public:
    void operator()(ConnectionData& connection_data, const std::array<GW_skill, 3432> skills);
};
