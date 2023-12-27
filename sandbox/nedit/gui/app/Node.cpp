#include "Node.hpp"
#include "Control.hpp"
#include "IDs.hpp"
#include "Pin.hpp"

#include <imgui.h>

namespace gui
{
    Node::Node(IDs& init_ids, std::uint64_t init_type, std::string_view init_label)
        : id(init_ids)
        , type(init_type)
        , label(init_label)
    {
    }

    void Node::render() const
    {
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
                    control->render();
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
    }
}
