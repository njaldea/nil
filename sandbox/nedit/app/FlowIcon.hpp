#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui-node-editor/imgui_node_editor.h>

struct FlowIcon
{
    FlowIcon(ImVec4 init_color);

    void render();

    ImVec4 color;
};
