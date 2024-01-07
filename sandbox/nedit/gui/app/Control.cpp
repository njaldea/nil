#include "Control.hpp"

#include <string>

#include <imgui_stdlib.h>

Control::Control(ax::NodeEditor::PinId init_id)
    : id(init_id)
{
}

ToggleControl::ToggleControl(
    ax::NodeEditor::PinId init_id,
    bool init_value,
    std::function<void(bool)> init_notify
)
    : Control(init_id)
    , value(init_value)
    , notify(std::move(init_notify))
{
}

void ToggleControl::render()
{
    bool need_update = false;
    ImGui::PushID(int(id.Get()));
    ImGui::PushItemWidth(100.f);
    need_update = ImGui::Checkbox("Checkbox", &value);
    ImGui::PopItemWidth();
    ImGui::PopID();
    if (need_update && notify)
    {
        notify(value);
    }
}

SpinboxControl::SpinboxControl(
    ax::NodeEditor::PinId init_id,
    std::int32_t init_value,
    std::int32_t init_min,
    std::int32_t init_max,
    std::function<void(std::int32_t)> init_notify
)
    : Control(init_id)
    , value(init_value)
    , min(init_min)
    , max(init_max)
    , notify(std::move(init_notify))
{
}

void SpinboxControl::render()
{
    bool need_update = false;
    ImGui::PushID(int(id.Get()));
    ImGui::PushItemWidth(100.f);
    need_update = ImGui::SliderInt("Spinbox", &value, min, max);
    ImGui::PopItemWidth();
    ImGui::PopID();
    if (need_update && notify)
    {
        notify(value);
    }
}

SliderControl::SliderControl(
    ax::NodeEditor::PinId init_id,
    float init_value,
    float init_min,
    float init_max,
    std::function<void(float)> init_notify
)
    : Control(init_id)
    , value(init_value)
    , min(init_min)
    , max(init_max)
    , notify(std::move(init_notify))
{
}

void SliderControl::render()
{
    bool need_update = false;
    ImGui::PushID(int(id.Get()));
    ImGui::PushItemWidth(100.f);
    need_update = ImGui::SliderFloat("Slider", &value, min, max) || need_update;
    ImGui::PopItemWidth();
    ImGui::PopID();

    if (need_update && notify)
    {
        notify(value);
    }
}

TextControl::TextControl(
    ax::NodeEditor::PinId init_id,
    std::string init_value,
    std::function<void(const std::string&)> init_notify
)
    : Control(init_id)
    , value(std::move(init_value))
    , notify(std::move(init_notify))
{
}

void TextControl::render()
{
    bool need_update = false;
    ImGui::PushID(int(id.Get()));
    ImGui::PushItemWidth(100.f);
    need_update = ImGui::InputText("Text", &value);
    ImGui::PopItemWidth();
    ImGui::PopID();

    if (need_update && notify)
    {
        notify(value);
    }
}
