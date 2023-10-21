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
    ax::NodeEditor::PinPivotAlignment(
        ImVec2{kind == ax::NodeEditor::PinKind::Input ? 0.0f : 1.0f, 0.5}
    );
    icon.render();
    if (ImGui::IsItemHovered())
    {
        ax::NodeEditor::Suspend();
        if (kind == ax::NodeEditor::PinKind::Input)
        {
            ImGui::SetTooltip("Input  - %d", type);
        }
        else
        {
            ImGui::SetTooltip("Output - %d", type);
        }
        ax::NodeEditor::Resume();
    }
    ax::NodeEditor::EndPin();
}
