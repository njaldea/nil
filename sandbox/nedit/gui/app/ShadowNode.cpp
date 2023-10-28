#include "ShadowNode.hpp"
#include "Pin.hpp"

#include <imgui.h>

#include <iostream>
#include <limits>

ShadowNode::ShadowNode(
    std::uint64_t init_type,
    const NodeInfo& init_node_info,
    const std::vector<PinInfo>& init_pin_infos
)
    : type(init_type)
    , node_info(init_node_info)
    , pin_infos(init_pin_infos)
{
}

void ShadowNode::ready()
{
    is_ready = true;
}

void ShadowNode::render() const
{
    auto id = std::numeric_limits<std::uint64_t>::max();
    ax::NodeEditor::BeginNode(id);
    {
        pos = ImGui::GetIO().MousePos;
        ax::NodeEditor::SetNodePosition(id, ImVec2(pos.x - 20, pos.y - 20));
    }
    {
        ImGui::BeginGroup();
        ImGui::TextColored(ImVec4(0, 0, 0, 1), "type[%lu]", type);
        ImGui::TextColored(ImVec4(0, 0, 0, 1), "%s", node_info.label.data());
        ImGui::EndGroup();
    }
    const auto width = ImGui::GetItemRectSize().x;
    {
        ImGui::BeginGroup();
        for (const auto& i : node_info.inputs)
        {
            Pin(--id, ax::NodeEditor::PinKind::Input, i, pin_infos[i].label, pin_infos[i].icon)
                .render();
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
        for (const auto& o : node_info.outputs)
        {
            Pin(--id, ax::NodeEditor::PinKind::Output, o, pin_infos[o].label, pin_infos[o].icon)
                .render();
        }
        ImGui::EndGroup();
    }
    ax::NodeEditor::EndNode();
}

std::unique_ptr<Node> ShadowNode::consume(IDs& ids)
{
    const auto node_id = ids.reserve();
    auto node = std::make_unique<Node>(type, node_id, node_info.label);

    ax::NodeEditor::SetNodePosition(node_id, ImVec2(pos.x - 20, pos.y - 20));
    for (const auto& type_i : node_info.inputs)
    {
        auto pin_id_i = ids.reserve();
        node->pins_i.emplace_back(std::make_unique<Pin>(
            pin_id_i,
            ax::NodeEditor::PinKind::Input,
            type_i,
            pin_infos[type_i].label,
            pin_infos[type_i].icon
        ));
    }

    for (const auto& type_o : node_info.outputs)
    {
        auto pin_id_o = ids.reserve();
        node->pins_o.emplace_back(std::make_unique<Pin>(
            pin_id_o,
            ax::NodeEditor::PinKind::Output,
            type_o,
            pin_infos[type_o].label,
            pin_infos[type_o].icon
        ));
    }
    return node;
}
