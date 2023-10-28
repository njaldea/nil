#pragma once

#include <imgui-node-editor/imgui_node_editor.h>

struct Pin;

struct Link
{
    struct Info
    {
        Pin* entry;
        Pin* exit;
    };

    Link(ax::NodeEditor::LinkId init_id, Info info);

    void render() const;

    ax::NodeEditor::LinkId id;
    Pin* entry;
    Pin* exit;
};
