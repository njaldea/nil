#include "FlowIcon.hpp"

#include <imgui_internal.h>

namespace gui
{
    FlowIcon::FlowIcon(ImVec4 init_color)
        : color(init_color)
    {
    }

    void FlowIcon::render(const ImVec2& pos1, const ImVec2& pos2, float padding, bool flipped) const
    {
        constexpr auto size = ImVec2(15, 15);
        if (ImGui::IsRectVisible(size))
        {
            auto* drawList = ImGui::GetWindowDrawList();

            const auto left = pos1.x + padding;
            const auto right = pos2.x - padding;
            const auto mid_x = left + ((right - left) * 0.5f);

            const auto bottom = pos1.y + padding;
            const auto top = pos2.y - padding;
            const auto mid_y = bottom + ((top - bottom) * 0.5f);

            const auto ref_x1 = flipped ? right : left;
            const auto ref_x2 = mid_x;
            const auto ref_x3 = flipped ? left : right;

            const auto ref_y1 = flipped ? top : bottom;
            const auto ref_y2 = mid_y;
            const auto ref_y3 = flipped ? bottom : top;

            drawList->PathLineTo({ref_x1, ref_y1});
            drawList->PathLineTo({ref_x2, ref_y1});
            drawList->PathLineTo({ref_x3, ref_y2});
            drawList->PathLineTo({ref_x2, ref_y3});
            drawList->PathLineTo({ref_x1, ref_y3});

            drawList->PathFillConvex(ImColor(color));
        }

        ImGui::Dummy(size);
    }
}
