#include "pch.h"
#include "ShowClientsConnected.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

void ShowClientsConnected::operator()(ConnectionData& connection_data,
                                      const std::array<GW_skill, 3432> skills)
{
    {
        const auto connected_client_ids = connection_data.get_connected_client_ids();

        ImGui::Begin("Connected clients info");
        // Display the number of connections
        ImGui::Text("Number of connections:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.2, 1, 0.3, 1), std::to_string(connected_client_ids.size()).c_str());

        // Display a list of character names, character classes, locations, party IDs, and party sizes
        ImGui::Columns(8, "connection_columns");
        ImGui::Separator();
        ImGui::Text("Game state");
        ImGui::NextColumn();
        ImGui::Text("Email");
        ImGui::NextColumn();
        ImGui::Text("Character Name");
        ImGui::NextColumn();
        ImGui::Text("Agent ID");
        ImGui::NextColumn();
        ImGui::Text("Character Class");
        ImGui::NextColumn();
        ImGui::Text("Location");
        ImGui::NextColumn();
        ImGui::Text("Party ID");
        ImGui::NextColumn();
        ImGui::Text("Party Size");
        ImGui::NextColumn();
        ImGui::Separator();

        for (const auto& id : connected_client_ids)
        {
            auto client_data = connection_data.get_client_data(id);
            if (client_data)
            {
                auto game_state_name = GWIPC::EnumNamesGameState()[client_data->game_state()];
                if (game_state_name)
                {
                    ImGui::Text("%s", game_state_name);
                }
                ImGui::NextColumn();

                ImGui::Text("%s", id.c_str());
                ImGui::NextColumn();

                if (client_data->character() && client_data->character()->agent_living() &&
                    client_data->character()->agent_living()->agent())
                {
                    auto name = client_data->character()->agent_living()->name();
                    ImGui::Text("%s", name->c_str());
                    ImGui::NextColumn();

                    ImGui::Text("%u", client_data->character()->agent_living()->agent()->agent_id());
                    ImGui::NextColumn();

                    auto primary = client_data->character()->agent_living()->primary_profession();
                    auto primary_name = GWIPC::EnumNamesProfession()[primary];
                    auto secondary = client_data->character()->agent_living()->secondary_profession();
                    auto secondary_name = GWIPC::EnumNamesProfession()[secondary];
                    ImGui::Text("%s/%s", primary_name, secondary_name);
                    ImGui::NextColumn();
                }
                else
                {
                    ImGui::NextColumn();
                    ImGui::NextColumn();
                    ImGui::NextColumn();
                }

                int instance_id = -1;
                if (client_data->instance())
                {
                    auto map_id = client_data->instance()->map_id();
                    ImGui::Text("%s", GW::Constants::NAME_FROM_ID[map_id]);

                    instance_id = client_data->instance()->instance_id();
                }
                ImGui::NextColumn();

                uint32_t num_hero_members = 0;
                uint32_t num_player_members = 0;
                uint32_t num_henchman_members = 0;
                if (client_data->party())
                {
                    auto party_id = client_data->party()->party_id();
                    ImGui::Text("(%u, %u)", instance_id, party_id);

                    auto hero_members = client_data->party()->hero_members();
                    if (hero_members)
                    {
                        num_hero_members += hero_members->size();
                    }

                    auto player_members = client_data->party()->player_members();
                    if (player_members)
                    {
                        num_player_members += player_members->size();
                    }

                    auto henchman_members = client_data->party()->henchman_members();
                    if (henchman_members)
                    {
                        num_henchman_members += henchman_members->size();
                    }
                }
                ImGui::NextColumn();

                ImGui::Text("%d(A)|%d(P)|%d(H)|%d(h)",
                            num_hero_members + num_player_members + num_henchman_members, num_player_members,
                            num_hero_members, num_henchman_members);
                ImGui::NextColumn();
            }
        }
        ImGui::Columns(1);
        ImGui::End();

        // Set the size constraints for the window
        ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(FLT_MAX, FLT_MAX));

        // Begin the window
        ImGui::Begin("Unit Frame 4");

        // Get the available content region within the window
        auto contentRegion = ImGui::GetContentRegionAvail();

        // Calculate the size and position of the name label
        auto nameLabelSize = ImVec2(contentRegion.x, 20);
        auto nameLabelPos = ImVec2(0, 0);

        // Draw the name label
        ImGui::SetCursorPos(nameLabelPos);
        ImGui::Text("Unit Name");

        // Calculate the size and position of the health bar
        auto healthBarSize = ImVec2(contentRegion.x, 20);
        auto healthBarPos = ImVec2(0, 20);

        // Push the red color style for the health bar
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

        // Draw the health bar
        ImGui::SetCursorPos(healthBarPos);
        ImGui::ProgressBar(0.5, healthBarSize);

        // Pop the red color style for the health bar
        ImGui::PopStyleColor();

        // Calculate the size and position of the energy bar
        auto energyBarSize = ImVec2(contentRegion.x, 20);
        auto energyBarPos = ImVec2(0, 40);

        // Push the blue color style for the energy bar
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

        // Draw the energy bar
        ImGui::SetCursorPos(energyBarPos);
        ImGui::ProgressBar(0.25, energyBarSize);

        // Pop the blue color style for the energy bar
        ImGui::PopStyleColor();

        // Calculate the size and position of the skill bar
        auto skillBarSize = ImVec2(contentRegion.x, 20);
        auto skillBarPos = ImVec2(0, 60);

        // Calculate the size of each image button based on the available content region and the number of buttons
        ImVec2 buttonSize(contentRegion.x / 8, contentRegion.x / 8);

        // Draw the skill bar
        for (int i = 0; i < 8; i++)
        {
            auto skill = skills[i + 1];

            ImVec2 buttonPos(skillBarPos.x + i * buttonSize.x + 5, skillBarPos.y);
            ImGui::SetCursorPos(buttonPos);
            // Set the border color
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            if (ImGui::ImageButton(skill.skill_icon_texture, buttonSize))
            {
                // Handle button click
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
        }
        ImGui::End();
    }
}
