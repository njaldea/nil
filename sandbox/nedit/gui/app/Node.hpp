#pragma once

#include <imgui-node-editor/imgui_node_editor.h>

#include <memory>
#include <vector>

struct Pin;

struct Node
{
    Node(std::uint64_t init_type, ax::NodeEditor::NodeId init_id, std::string_view init_label);

    void render();

    std::uint64_t type;
    ax::NodeEditor::NodeId id;
    std::string_view label;
    std::vector<std::unique_ptr<Pin>> pins_i;
    std::vector<std::unique_ptr<Pin>> pins_o;
};
