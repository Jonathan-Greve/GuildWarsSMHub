#pragma once
#include "PartyManager.h"

// Show the number of clients connected and sharing data.
// Also show their email addresses and uptime.
class ShowClientsConnected
{
public:
    void operator()(ConnectionData& connection_data)
    {
        const auto connections = connection_data.get_connected_client_emails();

        ImGui::Begin("Connected clients");

        ImGui::Text("Number of connections: ");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(ImVec4(0.2, 1, 0.3, 1), std::to_string(connections.size()).c_str());

        if (connections.size() > 0)
        {
            auto first_conn = *connections.begin();
            if (ImGui::CollapsingHeader(first_conn.c_str(), ImGuiTreeNodeFlags_OpenOnDoubleClick))
            {
                ImGui::Text("IsItemHovered: %d", ImGui::IsItemHovered());
            }
        }

        ImGui::End();
    }
};
