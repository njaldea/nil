#pragma once

#include <imgui-node-editor/imgui_node_editor.h>

#include <memory>
#include <vector>

struct Pin;

struct Node
{
    Node(ax::NodeEditor::NodeId init_id);

    void render();

    ax::NodeEditor::NodeId id;
    std::vector<std::unique_ptr<Pin>> pins_i;
    std::vector<std::unique_ptr<Pin>> pins_o;
};
