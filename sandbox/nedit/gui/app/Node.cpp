#include "Node.hpp"
#include "Pin.hpp"

#include <imgui.h>

Node::Node(std::uint64_t init_type, ax::NodeEditor::NodeId init_id, std::string_view init_label)
    : type(init_type)
    , id(init_id)
    , label(init_label)
{
}

void Node::render() const
{
    ax::NodeEditor::BeginNode(id);
    {
        ImGui::BeginGroup();
        ImGui::TextColored(ImVec4(0, 0, 0, 1), "id[%ld]", id.Get());
        ImGui::TextColored(ImVec4(0, 0, 0, 1), "%s", label.data());
        ImGui::EndGroup();
    }
    const auto width = ImGui::GetItemRectSize().x;
    {
        ImGui::BeginGroup();
        for (const auto& pin : pins_i)
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
        for (const auto& pin : pins_o)
        {
            pin->render();
        }
        ImGui::EndGroup();
    }
    ax::NodeEditor::EndNode();
}
