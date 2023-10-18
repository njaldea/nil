#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui-node-editor/imgui_node_editor.h>

#include <unordered_set>

struct Link;
struct Node;

struct Pin
{
    Pin(ax::NodeEditor::PinId init_id,
        ax::NodeEditor::PinKind init_kind,
        std::uint32_t init_type,
        ImVec4 init_color);
    ~Pin();

    void render();

    std::unordered_set<std::uint64_t> links;
    ax::NodeEditor::PinId id;
    ax::NodeEditor::PinKind kind;
    std::uint32_t type;
    ImVec4 color;
};
