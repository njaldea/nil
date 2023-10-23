#include "FlowIcon.hpp"

#include <imgui_internal.h>

FlowIcon::FlowIcon(ImVec4 init_color)
    : color(std::move(init_color))
{
}

void FlowIcon::render(float x_offset)
{
    const ImVec2 size = {15, 15};
    if (ImGui::IsRectVisible(size))
    {
        auto cursorPos = ImGui::GetCursorScreenPos();
        auto drawList = ImGui::GetWindowDrawList();

        const auto a = cursorPos + ImVec2(x_offset, 0.0f);
        const auto b = cursorPos + ImVec2(x_offset, 0.0f) + size;

        auto rect = ImRect(a, b);

        const auto rounding = 0.1f * 1;
        const auto tip_round = 0.7f;
        const auto canvas = ImRect(rect.Min.x, rect.Min.y, rect.Max.x, rect.Max.y);

        const auto canvas_x = canvas.Min.x;
        const auto canvas_y = canvas.Min.y;
        const auto canvas_w = canvas.Max.x - canvas.Min.x;
        const auto canvas_h = canvas.Max.y - canvas.Min.y;

        const auto left = canvas_x + canvas_w * 0.5f * 0.3f;
        const auto right = canvas_x + canvas_w - canvas_w * 0.5f * 0.3f;
        const auto top = canvas_y + canvas_h * 0.5f * 0.2f;
        const auto bottom = canvas_y + canvas_h - canvas_h * 0.5f * 0.2f;
        const auto center_y = (top + bottom) * 0.5f;

        const auto tip_top = ImVec2(canvas_x + canvas_w * 0.5f, top);
        const auto tip_right = ImVec2(right, center_y);
        const auto tip_bottom = ImVec2(canvas_x + canvas_w * 0.5f, bottom);

        drawList->PathLineTo(ImVec2(left, top) + ImVec2(0, rounding));
        drawList->PathBezierCubicCurveTo(
            ImVec2(left, top),
            ImVec2(left, top),
            ImVec2(left, top) + ImVec2(rounding, 0)
        );
        drawList->PathLineTo(tip_top);
        drawList->PathLineTo(tip_top + (tip_right - tip_top) * tip_round);
        drawList->PathBezierCubicCurveTo(
            tip_right,
            tip_right,
            tip_bottom + (tip_right - tip_bottom) * tip_round
        );
        drawList->PathLineTo(tip_bottom);
        drawList->PathLineTo(ImVec2(left, bottom) + ImVec2(rounding, 0));
        drawList->PathBezierCubicCurveTo(
            ImVec2(left, bottom),
            ImVec2(left, bottom),
            ImVec2(left, bottom) - ImVec2(0, rounding)
        );

        drawList->PathFillConvex(ImColor(color));
    }

    ImGui::Dummy(size);
}
