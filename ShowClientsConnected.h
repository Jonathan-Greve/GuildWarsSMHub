#pragma once

// Show the number of clients connected and sharing data.
// Also show their email addresses and uptime.
class ShowClientsConnected
{
public:
    void operator()(const std::vector<ClientData> client_datas)
    {
        ImGui::Begin("Connected clients");

        ImGui::Text("Number of connections: ");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(ImVec4(0.2, 1, 0.3, 1), std::to_string(client_datas.size()).c_str());

        if (ImGui::CollapsingHeader("Header", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("IsItemHovered: %d", ImGui::IsItemHovered());
        }

        ImGui::End();
    }
};
