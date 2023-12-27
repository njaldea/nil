#include "Control.hpp"

#include <string>

Control::Control(ax::NodeEditor::PinId init_id)
    : id(init_id)
{
}

void SliderControl::render()
{
    ImGui::SliderFloat2(("Slider##" + std::to_string(id.Get())).c_str(), &value, 0.0f, 1.0f);
}
