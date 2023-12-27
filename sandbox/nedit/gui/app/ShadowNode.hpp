#pragma once

#include "FlowIcon.hpp"
#include "IDs.hpp"
#include "Node.hpp"

#include <imgui-node-editor/imgui_node_editor.h>

#include <memory>
#include <vector>

struct Pin;

struct NodeInfo
{
    std::string label;
    std::vector<std::uint64_t> inputs;
    std::vector<std::uint64_t> outputs;
};

struct PinInfo
{
    std::string label;
    FlowIcon icon;
};

struct ShadowNode
{
    ShadowNode(
        std::uint64_t init_type,
        const NodeInfo& init_node_info,
        const std::vector<PinInfo>& init_pin_infos
    );

    ~ShadowNode() noexcept = default;
    ShadowNode(const Pin&) = delete;
    ShadowNode(ShadowNode&&) = delete;
    ShadowNode& operator=(ShadowNode&&) = delete;
    ShadowNode& operator=(const ShadowNode&) = delete;

    void render() const;
    void ready();
    std::unique_ptr<Node> consume(IDs& ids);

    std::uint64_t type;
    const NodeInfo& node_info;
    const std::vector<PinInfo>& pin_infos;

    mutable ImVec2 pos;
    bool is_ready = false;
};
