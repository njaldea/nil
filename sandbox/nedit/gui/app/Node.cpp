#include "Node.hpp"
#include "Pin.hpp"

#include <imgui.h>

Node::Node(ax::NodeEditor::NodeId init_id)
    : id(init_id)
{
}

void Node::render()
{
    ax::NodeEditor::BeginNode(id);
    {
        ImGui::BeginGroup();
        ImGui::TextColored(ImVec4(0, 0, 0, 1), "Node: %ld", id.Get());
        ImGui::EndGroup();
    }
    const auto width = ImGui::GetItemRectSize().x;
    {
        ImGui::BeginGroup();
        for (auto& pin : pins_i)
        {
            pin->render();
        }
        ImGui::EndGroup();
    }
    ImGui::SameLine();
    {
        ImGui::BeginGroup();
        ImGui::Dummy(ImVec2(width - 15.0f, 0.0f));
        ImGui::EndGroup();
    }
    ImGui::SameLine();
    {
        ImGui::BeginGroup();
        for (auto& pin : pins_o)
        {
            pin->render();
        }
        ImGui::EndGroup();
    }
    ax::NodeEditor::EndNode();
}
