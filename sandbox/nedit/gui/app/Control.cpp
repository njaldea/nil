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
    float init_max
)
    : Control(init_id)
    , value(init_value)
    , min(init_min)
    , max(init_max)
{
}

void SliderControl::render()
{
    ImGui::PushID(int(id.Get()));
    ImGui::PushItemWidth(50.f);
    ImGui::SliderFloat("Slider", &value, min, max);
    ImGui::SameLine();
    ImGui::InputFloat("", &value);
    ImGui::PopItemWidth();
    ImGui::PopID();
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
