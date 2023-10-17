#include "ShadowNode.hpp"
#include "Pin.hpp"

#include <imgui.h>

ShadowNode::ShadowNode(std::uint64_t init_type, const NodeInfo& init_info, IDs& init_ids)
    : type(init_type)
    , info(init_info)
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
    for (const auto& i : info.inputs)
    {
        std::make_unique<Pin>(--id, ax::NodeEditor::PinKind::Input, i)->render();
    }
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Dummy(ImVec2(0, 0));
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    for (const auto& i : info.outputs)
    {
        std::make_unique<Pin>(--id, ax::NodeEditor::PinKind::Input, i)->render();
    }
    ImGui::EndGroup();
    ax::NodeEditor::EndNode();
}

std::unique_ptr<Node> ShadowNode::consume()
{
    const auto node_id = ids.reserve();
    auto node = std::make_unique<Node>(node_id);

    ax::NodeEditor::SetNodePosition(node_id, ImVec2(pos.x - 20, pos.y - 20));
    for (const auto& type_i : info.inputs)
    {
        auto pin_id_i = ids.reserve();
        node->pins_i.emplace_back(std::make_unique<Pin>( //
            pin_id_i,
            ax::NodeEditor::PinKind::Input,
            type_i
        ));
        // pins.emplace(pin_id_i, std::make_tuple(n.get(), n->pins_i.back().get()));
    }

    for (const auto& type_o : info.outputs)
    {
        auto pin_id_o = ids.reserve();
        node->pins_o.emplace_back(std::make_unique<Pin>( //
            pin_id_o,
            ax::NodeEditor::PinKind::Output,
            type_o
        ));
        // pins.emplace(pin_id_o, std::make_tuple(n.get(), n->pins_o.back().get()));
    }
    return node;
}
