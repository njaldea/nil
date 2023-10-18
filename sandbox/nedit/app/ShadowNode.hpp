#pragma once

#include "IDs.hpp"
#include "Node.hpp"

#include <imgui-node-editor/imgui_node_editor.h>

#include <memory>
#include <vector>

struct Pin;

struct NodeInfo
{
    std::string label;
    std::vector<std::uint32_t> inputs;
    std::vector<std::uint32_t> outputs;
};

struct PinInfo
{
    std::string label;
    ImVec4 color;
};

// TODO: move node creation in a common place
struct ShadowNode
{
    ShadowNode(
        std::uint64_t init_type,
        const NodeInfo& init_node_info,
        const std::vector<PinInfo>& init_pin_infos,
        IDs& init_ids
    );

    void render();
    void ready();
    std::unique_ptr<Node> consume();

    std::uint64_t type;
    const NodeInfo& node_info;
    const std::vector<PinInfo>& pin_infos;
    IDs& ids;

    ImVec2 pos;
    bool is_ready = false;
};
