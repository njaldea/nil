#include "Node.hpp"
#include "Control.hpp"
#include "IDs.hpp"
#include "Pin.hpp"

#include <imgui.h>

namespace
{
    void render(
        const std::vector<std::unique_ptr<gui::Control>>& controls,
        bool interactive_controls,
        float width
    )
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

    void render(const std::vector<gui::Pin>& pins)
    {
        ImGui::BeginGroup();
        if (pins.empty())
        {
            ImGui::Dummy(ImVec2(15.0f, 0.0f));
        }
        else
        {
            for (const auto& pin : pins)
            {
                pin.render();
            }
        }
        ImGui::EndGroup();
    }

    void render(
        std::uint64_t id,
        std::string_view label,
        const std::vector<gui::Pin>& pins_a,
        const std::vector<gui::Pin>& pins_b,
        const std::vector<std::unique_ptr<gui::Control>>& controls,
        bool interactive_controls
    )
    {
        ax::NodeEditor::BeginNode(id);
        if (!label.empty())
        {
            ImGui::TextColored(ImVec4(0, 0, 0, 1), "%s", label.data());
        }
        const auto width = label.empty() ? 30 : ImGui::GetItemRectSize().x;
        render(pins_a);
        ImGui::SameLine();
        render(controls, interactive_controls, width);
        ImGui::SameLine();
        render(pins_b);
        ax::NodeEditor::EndNode();
    }
}

namespace gui
{
    Node::Node(ID init_id, std::uint64_t init_type, std::string_view init_label, bool init_flipped)
        : id(std::move(init_id))
        , type(init_type)
        , label(init_label)
        , flipped(init_flipped)
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

        ::render(
            id.value,
            label,
            flipped ? pins_o : pins_i,
            flipped ? pins_i : pins_o,
            controls,
            interactive_controls
        );

        if (activated)
        {
            ax::NodeEditor::PopStyleColor();
        }
    }
}
