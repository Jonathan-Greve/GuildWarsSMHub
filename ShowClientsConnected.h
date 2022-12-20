#pragma once
#include "PartyManager.h"

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
    void operator()(PartyManager& party_manager, const std::array<GW_skill, 3432> skills);
};
