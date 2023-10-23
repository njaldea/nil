#pragma once

#include <imgui-node-editor/imgui_node_editor.h>

struct Pin;

struct Link
{
    Link(ax::NodeEditor::LinkId init_id, Pin* init_entry, Pin* init_exit);

    void render();

    ax::NodeEditor::LinkId id;
    Pin* entry;
    Pin* exit;
};
