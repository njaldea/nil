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
    ImGui::Text("Node: %ld", id.Get());
    ImGui::BeginGroup();
    for (auto& pin : pins_i)
    {
        pin->render();
    }
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Dummy(ImVec2(0, 0));
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    for (auto& pin : pins_o)
    {
        pin->render();
    }
    ImGui::EndGroup();
    ax::NodeEditor::EndNode();
}
