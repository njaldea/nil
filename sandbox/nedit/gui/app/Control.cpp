#include "Control.hpp"

#include <string>

Control::Control(ax::NodeEditor::PinId init_id)
    : id(init_id)
{
}

void SliderControl::render()
{
    ImGui::PushItemWidth(100.f);
    ImGui::InputFloat(("SliderText##" + std::to_string(id.Get())).c_str(), &value);
    ImGui::SliderFloat(("Slider##" + std::to_string(id.Get())).c_str(), &value, 0.0f, 1.0f);
    ImGui::PopItemWidth();
}

void TextControl::render()
{
    ImGui::PushItemWidth(100.f);
    ImGui::InputText(("Text##" + std::to_string(id.Get())).c_str(), value.data(), value.size());
    ImGui::PopItemWidth();
}
