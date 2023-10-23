#include "Pin.hpp"
#include "Node.hpp"

#include <imgui-node-editor/imgui_node_editor.h>
#include <imgui.h>
#include <imgui_internal.h>

Pin::Pin(
    ax::NodeEditor::PinId init_id,
    ax::NodeEditor::PinKind init_kind,
    std::uint32_t init_type,
    FlowIcon& init_icon
)
    : id(init_id)
    , kind(init_kind)
    , type(init_type)
    , icon(init_icon)
{
}

void Pin::render()
{
    ax::NodeEditor::BeginPin(id, kind);

    constexpr const auto offset = 7.5f;
    if (kind == ax::NodeEditor::PinKind::Input)
    {
        ax::NodeEditor::PinPivotAlignment(ImVec2{0.0f, 0.5});
        auto cursorPos = ImGui::GetCursorScreenPos();
        ax::NodeEditor::PinRect(
            cursorPos - ImVec2{offset, 0.0f},
            cursorPos + ImVec2{15.0f, 15.0f} - ImVec2{offset, 0.0f}
        );
        icon.render(-offset);
        if (ax::NodeEditor::GetHoveredPin() == id)
        {
            ax::NodeEditor::Suspend();
            ImGui::SetTooltip("Input  - %d", type);
            ax::NodeEditor::Resume();
        }
    }
    else
    {
        ax::NodeEditor::PinPivotAlignment(ImVec2{1.0f, 0.5});
        auto cursorPos = ImGui::GetCursorScreenPos();
        ax::NodeEditor::PinRect(
            cursorPos + ImVec2{offset + 5.0f, 0.0f},
            cursorPos + ImVec2{15.0f, 15.0f} + ImVec2{offset + 5.0f, 0.0f}
        );
        icon.render(+offset + 5.0f);
        if (ax::NodeEditor::GetHoveredPin() == id)
        {
            ax::NodeEditor::Suspend();
            ImGui::SetTooltip("Output - %d", type);
            ax::NodeEditor::Resume();
        }
    }

    ax::NodeEditor::EndPin();
}
