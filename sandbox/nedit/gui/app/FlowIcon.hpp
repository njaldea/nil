#pragma once

#include <imgui-node-editor/imgui_node_editor.h>

namespace gui
{
    struct FlowIcon final
    {
        FlowIcon(ImVec4 init_color);
        ~FlowIcon() noexcept = default;
        FlowIcon(const FlowIcon&) = default;
        FlowIcon(FlowIcon&&) noexcept = default;
        FlowIcon& operator=(const FlowIcon&) = default;
        FlowIcon& operator=(FlowIcon&&) noexcept = default;

        void render(const ImVec2& pos1, const ImVec2& pos2, float padding, bool flipped) const;

        ImVec4 color;
    };
}
