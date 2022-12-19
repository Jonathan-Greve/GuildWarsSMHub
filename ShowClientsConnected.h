#pragma once
#include "PartyManager.h"

// Show the number of clients connected and sharing data.
// Also show their email addresses and uptime.
class ShowClientsConnected
{
public:
    void operator()(ConnectionData& connection_data, const std::unordered_map<int, GW_skill> skills)
    {
        const auto connected_client_names = connection_data.get_connected_clients_shared_memory_names();

        ImGui::Begin("Connected clients");

        ImGui::Text("Number of connections: ");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(ImVec4(0.2, 1, 0.3, 1), std::to_string(connected_client_names.size()).c_str());

        for (const auto& name : connected_client_names)
        {
            if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_OpenOnDoubleClick))
            {
                ImGui::Text("IsItemHovered: %d", ImGui::IsItemHovered());
            }
        }

        ImGui::End();
    }
};
