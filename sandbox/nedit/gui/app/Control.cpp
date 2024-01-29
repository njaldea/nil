#include "Control.hpp"

#include <string>

#include <imgui_stdlib.h>

namespace gui
{
    Control::Control(ID init_id)
        : id(std::move(init_id))
    {
    }

    ToggleControl::ToggleControl(
        ID init_id,
        bool init_value,
        std::function<void(std::uint64_t, bool)> init_notify
    )
        : Control(std::move(init_id))
        , value(init_value)
        , notify(std::move(init_notify))
    {
    }

    void ToggleControl::render()
    {
        bool need_update = false;
        ImGui::PushID(int(id.value));
        ImGui::PushItemWidth(100.f);
        need_update = ImGui::Checkbox("Checkbox", &value);
        ImGui::PopItemWidth();
        ImGui::PopID();
        if (need_update && notify)
        {
            notify(id.value, value);
        }
    }

    SpinboxControl::SpinboxControl(
        ID init_id,
        std::int32_t init_value,
        std::int32_t init_min,
        std::int32_t init_max,
        std::function<void(std::uint64_t, std::int32_t)> init_notify
    )
        : Control(std::move(init_id))
        , value(init_value)
        , min(init_min)
        , max(init_max)
        , notify(std::move(init_notify))
    {
    }

    void SpinboxControl::render()
    {
        bool need_update = false;
        ImGui::PushID(int(id.value));
        ImGui::PushItemWidth(100.f);
        need_update = ImGui::SliderInt("Spinbox", &value, min, max);
        ImGui::PopItemWidth();
        ImGui::PopID();
        if (need_update && notify)
        {
            notify(id.value, value);
        }
    }

    SliderControl::SliderControl(
        ID init_id,
        float init_value,
        float init_min,
        float init_max,
        std::function<void(std::uint64_t, float)> init_notify
    )
        : Control(std::move(init_id))
        , value(init_value)
        , min(init_min)
        , max(init_max)
        , notify(std::move(init_notify))
    {
    }

    void SliderControl::render()
    {
        bool need_update = false;
        ImGui::PushID(int(id.value));
        ImGui::PushItemWidth(100.f);
        need_update = ImGui::SliderFloat("Slider", &value, min, max) || need_update;
        ImGui::PopItemWidth();
        ImGui::PopID();

        if (need_update && notify)
        {
            notify(id.value, value);
        }
    }

    TextControl::TextControl(
        ID init_id,
        std::string init_value,
        std::function<void(std::uint64_t, const std::string&)> init_notify
    )
        : Control(std::move(init_id))
        , value(std::move(init_value))
        , notify(std::move(init_notify))
    {
    }

    void TextControl::render()
    {
        bool need_update = false;
        ImGui::PushID(int(id.value));
        ImGui::PushItemWidth(100.f);
        need_update = ImGui::InputText("Text", &value);
        ImGui::PopItemWidth();
        ImGui::PopID();

        if (need_update && notify)
        {
            notify(id.value, value);
        }
    }

    ComboBoxControl::ComboBoxControl(
        ID init_id,
        std::string init_value,
        std::vector<std::string> init_selection,
        std::function<void(std::uint64_t, const std::string&)> init_notify
    )
        : Control(std::move(init_id))
        , value(std::move(init_value))
        , selection(std::move(init_selection))
        , notify(std::move(init_notify))
    {
    }

    void ComboBoxControl::render()
    {
        bool need_update = false;
        ImGui::PushID(int(id.value));
        ImGui::PushItemWidth(100.f);

        ax::NodeEditor::Suspend();
        if (ImGui::BeginCombo("ComboBox", value.c_str()))
        {
            for (const auto& item : selection)
            {
                const bool is_selected = value == item;
                if (ImGui::Selectable(item.c_str(), is_selected))
                {
                    value = item;
                    need_update = true;
                }
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ax::NodeEditor::Resume();

        ImGui::PopItemWidth();
        ImGui::PopID();

        if (need_update && notify)
        {
            notify(id.value, value);
        }
    }
}
