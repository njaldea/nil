#pragma once

#include "FlowIcon.hpp"

#include <unordered_set>

struct Link;
struct Node;

struct Pin
{
    Pin(ax::NodeEditor::PinId init_id,
        ax::NodeEditor::PinKind init_kind,
        std::uint32_t init_type,
        FlowIcon& init_icon);

    void render();

    std::unordered_set<std::uint64_t> links;
    ax::NodeEditor::PinId id;
    ax::NodeEditor::PinKind kind;
    std::uint32_t type;
    FlowIcon& icon;
};
