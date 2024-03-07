#include "Pin.hpp"
#include "IDs.hpp"
#include "Node.hpp"

#include <imgui-node-editor/imgui_node_editor.h>
#include <imgui.h>

namespace gui
{
    Pin::Pin(
        ID init_id,
        ax::NodeEditor::PinKind init_kind,
        std::uint64_t init_type,
        std::string_view init_label,
        const FlowIcon* init_icon,
        bool init_flipped
    )
        : id(std::move(init_id))
        , kind(init_kind)
        , type(init_type)
        , alias(type)
        , label(init_label)
        , icon(init_icon)
        , flipped(init_flipped)
    {
    }

    void Pin::render() const
    {
        constexpr auto padding = 1.0f;
        const auto is_input = kind == ax::NodeEditor::PinKind::Input;
        const auto alignment = (flipped != is_input) ? 0.0f : 1.0f;

        const auto final_offset //
            = (flipped ? 1.0f : -1.0f) * (is_input ? 8.5f : -14.5f);

        ax::NodeEditor::PushStyleVar(
            ax::NodeEditor::StyleVar_SourceDirection,
            ImVec2(flipped ? -1.0f : 1.0f, 0.0f)
        );
        ax::NodeEditor::PushStyleVar(
            ax::NodeEditor::StyleVar_TargetDirection,
            ImVec2(flipped ? 1.0f : -1.0f, 0.0f)
        );

        ax::NodeEditor::BeginPin(id.value, kind);
        ax::NodeEditor::PinPivotAlignment(ImVec2(alignment, 0.5));

        auto cursorPos = ImGui::GetCursorScreenPos();
        const auto top_left = ImVec2(cursorPos.x + final_offset, cursorPos.y);
        const auto bottom_right = ImVec2(top_left.x + 15.0f, top_left.y + 15.0f);
        ax::NodeEditor::PinRect(top_left, bottom_right);
        icon->render(top_left, bottom_right, padding, flipped);

        if (ax::NodeEditor::GetHoveredPin().Get() == id.value)
        {
            ax::NodeEditor::Suspend();
            ImGui::SetTooltip("%s[%s]", is_input ? "i" : "o", label.data());
            ax::NodeEditor::Resume();
        }

        ax::NodeEditor::EndPin();
        ax::NodeEditor::PopStyleVar(2);
    }
}
