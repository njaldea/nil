#include "Node.hpp"
#include "Control.hpp"
#include "IDs.hpp"
#include "Pin.hpp"

#include <imgui.h>

namespace gui
{
    Node::Node(ID init_id, std::uint64_t init_type, std::string_view init_label)
        : id(std::move(init_id))
        , type(init_type)
        , label(init_label)
    {
    }

    void Node::render(bool interactive_controls) const
    {
        if (activated)
        {
            ax::NodeEditor::PushStyleColor(
                ax::NodeEditor::StyleColor_NodeBg,
                {1.0f, 0.0f, 0.0f, 1.0f}
            );
        }
        ax::NodeEditor::BeginNode(id.value);
        {
            ImGui::BeginGroup();
            ImGui::TextColored(ImVec4(0, 0, 0, 1), "%s", label.data());
            ImGui::EndGroup();
        }
        const auto width = ImGui::GetItemRectSize().x;
        {
            ImGui::BeginGroup();
            if (pins_i.empty())
            {
                ImGui::Dummy(ImVec2(15.0f, 0.0f));
            }
            else
            {
                for (const auto& pin : pins_i)
                {
                    pin.render();
                }
            }
            ImGui::EndGroup();
        }
        ImGui::SameLine();
        {
            ImGui::BeginGroup();
            if (controls.empty())
            {
                ImGui::Dummy(ImVec2(width - 15.0f, 0.0f));
            }
            else
            {
                for (const auto& control : controls)
                {
                    control->render(interactive_controls);
                }
            }
            ImGui::EndGroup();
        }
        ImGui::SameLine();
        {
            ImGui::BeginGroup();
            if (pins_o.empty())
            {
                ImGui::Dummy(ImVec2(15.0f, 0.0f));
            }
            else
            {
                for (const auto& pin : pins_o)
                {
                    pin.render();
                }
            }
            ImGui::EndGroup();
        }
        ax::NodeEditor::EndNode();
        if (activated)
        {
            ax::NodeEditor::PopStyleColor();
        }
    }
}
