#include "ShadowNode.hpp"
#include "Pin.hpp"

#include <imgui.h>

ShadowNode::ShadowNode(
    std::uint64_t init_type,
    const NodeInfo& init_node_info,
    const std::vector<PinInfo>& init_pin_infos,
    IDs& init_ids
)
    : type(init_type)
    , node_info(init_node_info)
    , pin_infos(init_pin_infos)
    , ids(init_ids)
{
}

void ShadowNode::ready()
{
    is_ready = true;
}

void ShadowNode::render()
{
    auto id = std::numeric_limits<std::uint64_t>::max();
    ax::NodeEditor::BeginNode(id);
    pos = ImGui::GetIO().MousePos;
    ax::NodeEditor::SetNodePosition(id, ImVec2(pos.x - 20, pos.y - 20));
    ImGui::Text("Node: type[%lu]", type);
    ImGui::BeginGroup();
    for (const auto& i : node_info.inputs)
    {
        std::make_unique<Pin>(--id, ax::NodeEditor::PinKind::Input, i, pin_infos[i].color)
            ->render();
    }
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Dummy(ImVec2(0, 0));
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    for (const auto& i : node_info.outputs)
    {
        std::make_unique<Pin>(--id, ax::NodeEditor::PinKind::Input, i, pin_infos[i].color)
            ->render();
    }
    ImGui::EndGroup();
    ax::NodeEditor::EndNode();
}

std::unique_ptr<Node> ShadowNode::consume()
{
    const auto node_id = ids.reserve();
    auto node = std::make_unique<Node>(node_id);

    ax::NodeEditor::SetNodePosition(node_id, ImVec2(pos.x - 20, pos.y - 20));
    for (const auto& type_i : node_info.inputs)
    {
        auto pin_id_i = ids.reserve();
        node->pins_i.emplace_back(std::make_unique<Pin>( //
            pin_id_i,
            ax::NodeEditor::PinKind::Input,
            type_i,
            pin_infos[type_i].color
        ));
        // pins.emplace(pin_id_i, std::make_tuple(n.get(), n->pins_i.back().get()));
    }

    for (const auto& type_o : node_info.outputs)
    {
        auto pin_id_o = ids.reserve();
        node->pins_o.emplace_back(std::make_unique<Pin>( //
            pin_id_o,
            ax::NodeEditor::PinKind::Output,
            type_o,
            pin_infos[type_o].color
        ));
        // pins.emplace(pin_id_o, std::make_tuple(n.get(), n->pins_o.back().get()));
    }
    return node;
}
