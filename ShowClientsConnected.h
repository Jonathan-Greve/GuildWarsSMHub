#pragma once
#include "PartyManager.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <array>

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

        // Get the current draw list
        ImGui::End();

        ImGui::Begin("Draw 2d");
        // Get the current draw list and the position of the current window
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();

        // Define the coordinates of the rectangle, relative to the window
        ImVec2 p1(10, 10);
        ImVec2 p2(100, 100);

        // Offset the rectangle coordinates by the window position
        p1 += windowPos;
        p2 += windowPos;

        // Set the color of the rectangle
        ImU32 color = IM_COL32(255, 0, 0, 255);

        // Draw the rectangle
        drawList->AddRect(p1, p2, color);

        // Check if the mouse is hovering over the rectangle
        if (ImGui::IsMouseHoveringRect(p1, p2))
        {
            // Begin a tooltip
            ImGui::BeginTooltip();

            // Display the info for the rectangle
            ImGui::Text("Info for rectangle");
            ImGui::Separator();
            ImGui::Text("Top-left corner: (%.1f, %.1f)", p1.x, p1.y);
            ImGui::Text("Bottom-right corner: (%.1f, %.1f)", p2.x, p2.y);

            // End the tooltip
            ImGui::EndTooltip();
        }
        ImGui::End();

        Character character;

        // Initialize the character data
        character.name = "Bob";
        character.health = 100.0f;
        character.mana = 50.0f;
        character.skills = {"Skill 1", "Skill 2", "Skill 3", "Skill 4",
                            "Skill 5", "Skill 6", "Skill 7", "Skill 8"};

        // Create the unit frame
        ImGui::Begin("Unit Frame");

        // Display the character name
        ImGui::Text("%s", character.name.c_str());

        // Display the character health and mana
        ImGui::ProgressBar(character.health / 100.0f, ImVec2(0.0f, 0.0f), "Health");
        ImGui::SameLine();
        ImGui::ProgressBar(character.mana / 100.0f, ImVec2(0.0f, 0.0f), "Mana");

        // Display the skillbar
        ImGui::Separator();
        for (int i = 0; i < 8; i++)
        {
            ImGui::PushID(i);
            ImGui::Button(character.skills[i].c_str());
            ImGui::PopID();
            ImGui::SameLine();
        }
        ImGui::End();

        // Set the size constraints for the window
        ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Unit Frame 2");
        ImVec2 contentRegion = ImGui::GetContentRegionAvail();
        // Load the image data for the skill icons
        ImTextureID skillIcons[8];
        skillIcons[0] = ImGui::GetIO().Fonts->TexID; // Use the default font as the icon for the first skill
        //skillIcons[1] = LoadImage("skill2.png"); // Load an image file for the second skill
        //skillIcons[2] = LoadImage("skill3.png"); // Load an image file for the third skill
        // ...

        // Create the unit frame

        // Display the character name
        ImGui::Text("%s", character.name.c_str());

        // Display the character health and mana
        ImGui::ProgressBar(character.health / 100.0f, ImVec2(0.0f, 0.0f), "Health");
        ImGui::SameLine();
        ImGui::ProgressBar(character.mana / 100.0f, ImVec2(0.0f, 0.0f), "Mana");

        // Display the skillbar
        ImGui::Separator();
        for (int i = 0; i < 8; i++)
        {
            auto it = skills.find(i + 1);

            ImGui::PushID(i);
            ImVec2 buttonSize(contentRegion.x / 10, contentRegion.x / 10);
            if (ImGui::ImageButton(it->second.skill_icon_texture, buttonSize))
            {
                // The skill button was clicked, handle the click event here
            }
            ImGui::PopID();
            ImGui::SameLine();
        }
        ImGui::End();

        // Set the size constraints for the window
        ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(FLT_MAX, FLT_MAX));

        // Set the size constraints for the window
        ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(FLT_MAX, FLT_MAX));

        // Begin the window
        ImGui::Begin("Unit Frame 3");

        // Get the available content region within the window
        contentRegion = ImGui::GetContentRegionAvail();

        // Calculate the size and position of the name label
        ImVec2 nameLabelSize(contentRegion.x, 20);
        ImVec2 nameLabelPos(0, 0);

        // Draw the name label
        ImGui::SetCursorPos(nameLabelPos);
        ImGui::Text("Unit Name");

        // Calculate the size and position of the health bar
        ImVec2 healthBarSize(contentRegion.x, 20);
        ImVec2 healthBarPos(0, 20);

        // Push the red color style for the health bar
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

        // Draw the health bar
        ImGui::SetCursorPos(healthBarPos);
        ImGui::ProgressBar(0.5, healthBarSize);

        // Pop the red color style for the health bar
        ImGui::PopStyleColor();

        // Calculate the size and position of the energy bar
        ImVec2 energyBarSize(contentRegion.x, 20);
        ImVec2 energyBarPos(0, 40);

        // Push the blue color style for the energy bar
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

        // Draw the energy bar
        ImGui::SetCursorPos(energyBarPos);
        ImGui::ProgressBar(0.25, energyBarSize);

        // Pop the blue color style for the energy bar
        ImGui::PopStyleColor();

        // Calculate the size and position of the skill bar
        ImVec2 skillBarSize(contentRegion.x, 20);
        ImVec2 skillBarPos(0, 60);

        // Draw the skill bar
        for (int i = 0; i < 8; i++)
        {
            auto it = skills.find(i + 1);
            ImVec2 buttonPos(skillBarPos.x + i * 20, skillBarPos.y);
            ImGui::PushID(i);
            if (ImGui::ImageButton(it->second.skill_icon_texture, ImVec2(20, 20)))
            {
                // Handle button click
            }
            ImGui::PopID();
            ImGui::SameLine();
        }

        // End the window
        ImGui::End();

        // Set the size constraints for the window
        ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(FLT_MAX, FLT_MAX));

        // Begin the window
        ImGui::Begin("Unit Frame 4");

        // Get the available content region within the window
        contentRegion = ImGui::GetContentRegionAvail();

        // Calculate the size and position of the name label
        nameLabelSize = ImVec2(contentRegion.x, 20);
        nameLabelPos = ImVec2(0, 0);

        // Draw the name label
        ImGui::SetCursorPos(nameLabelPos);
        ImGui::Text("Unit Name");

        // Calculate the size and position of the health bar
        healthBarSize = ImVec2(contentRegion.x, 20);
        healthBarPos = ImVec2(0, 20);

        // Push the red color style for the health bar
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

        // Draw the health bar
        ImGui::SetCursorPos(healthBarPos);
        ImGui::ProgressBar(0.5, healthBarSize);

        // Pop the red color style for the health bar
        ImGui::PopStyleColor();

        // Calculate the size and position of the energy bar
        energyBarSize = ImVec2(contentRegion.x, 20);
        energyBarPos = ImVec2(0, 40);

        // Push the blue color style for the energy bar
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

        // Draw the energy bar
        ImGui::SetCursorPos(energyBarPos);
        ImGui::ProgressBar(0.25, energyBarSize);

        // Pop the blue color style for the energy bar
        ImGui::PopStyleColor();

        // Calculate the size and position of the skill bar
        skillBarSize = ImVec2(contentRegion.x, 20);
        skillBarPos = ImVec2(0, 60);

        // Calculate the size of each image button based on the available content region and the number of buttons
        ImVec2 buttonSize(contentRegion.x / 8, contentRegion.x / 8);

        // Draw the skill bar
        for (int i = 0; i < 8; i++)
        {
            auto it = skills.find(i + 1);

            ImVec2 buttonPos(skillBarPos.x + i * buttonSize.x + 5, skillBarPos.y);
            ImGui::SetCursorPos(buttonPos);
            // Set the border color
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            if (ImGui::ImageButton(it->second.skill_icon_texture, buttonSize))
            {
                // Handle button click
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
        }
        ImGui::End();
    }
};
