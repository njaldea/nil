#include "Pin.hpp"
#include "IDs.hpp"
#include "Node.hpp"

#include <imgui-node-editor/imgui_node_editor.h>
#include <imgui.h>
#include <imgui_internal.h>

namespace gui
{
    Pin::Pin(
        ID init_id,
        ax::NodeEditor::PinKind init_kind,
        const void* init_type,
        std::string_view init_label,
        const FlowIcon& init_icon
    )
        : id(std::move(init_id))
        , kind(init_kind)
        , type(init_type)
        , label(init_label)
        , icon(init_icon)
    {
    }

    void Pin::render() const
    {
        ax::NodeEditor::BeginPin(id.value, kind);

        constexpr const auto offset = 7.5f;
        if (kind == ax::NodeEditor::PinKind::Input)
        {
            ax::NodeEditor::PinPivotAlignment(ImVec2(0.0f, 0.5));
            auto cursorPos = ImGui::GetCursorScreenPos();
            ax::NodeEditor::PinRect(
                cursorPos - ImVec2(offset, 0.0f),
                cursorPos + ImVec2(15.0f, 15.0f) - ImVec2(offset, 0.0f)
            );
            icon.render(-offset);
            if (ax::NodeEditor::GetHoveredPin().Get() == id.value)
            {
                ax::NodeEditor::Suspend();
                ImGui::SetTooltip("%s", label.data());
                ax::NodeEditor::Resume();
            }
        }
        else
        {
            ax::NodeEditor::PinPivotAlignment(ImVec2(1.0f, 0.5));
            auto cursorPos = ImGui::GetCursorScreenPos();
            ax::NodeEditor::PinRect(
                cursorPos + ImVec2(offset + 5.0f, 0.0f),
                cursorPos + ImVec2(15.0f, 15.0f) + ImVec2(offset + 5.0f, 0.0f)
            );
            icon.render(+offset + 5.0f);
            if (ax::NodeEditor::GetHoveredPin().Get() == id.value)
            {
                ax::NodeEditor::Suspend();
                ImGui::SetTooltip("%s", label.data());
                ax::NodeEditor::Resume();
            }
        }

        ax::NodeEditor::EndPin();
    }
}
