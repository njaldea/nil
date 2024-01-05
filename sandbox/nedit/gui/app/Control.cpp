#include "Control.hpp"

#include <string>

Control::Control(ax::NodeEditor::PinId init_id)
    : id(init_id)
{
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
    ImGui::PushItemWidth(50.f);
    need_update = ImGui::SliderFloat("Slider", &value, min, max) || need_update;
    ImGui::SameLine();
    need_update = ImGui::InputFloat("", &value) || need_update;
    ImGui::PopItemWidth();
    ImGui::PopID();

    if (need_update && notify)
    {
        notify(value);
    }
}

TextControl::TextControl(ax::NodeEditor::PinId init_id, std::string init_value)
    : Control(init_id)
{
    value.resize(100); // size?
    std::copy(init_value.cbegin(), init_value.cend(), value.begin());
}

void TextControl::render()
{
    ImGui::PushID(int(id.Get()));
    ImGui::PushItemWidth(100.f);
    ImGui::InputText("Text", value.data(), value.size());
    ImGui::PopItemWidth();
    ImGui::PopID();
}
