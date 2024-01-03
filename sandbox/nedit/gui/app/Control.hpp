#pragma once

#include <imgui-node-editor/imgui_node_editor.h>

#include <string>

struct Control
{
    Control(ax::NodeEditor::PinId init_id);
    virtual ~Control() noexcept = default;
    virtual void render() = 0;

    ax::NodeEditor::PinId id;
};

struct SliderControl: Control
{
    SliderControl(ax::NodeEditor::PinId init_id, float init_value, float init_min, float init_max);

    void render() override;

    float value;
    float min;
    float max;
};

struct TextControl: Control
{
    TextControl(ax::NodeEditor::PinId init_id, std::string init_value);

    void render() override;

    std::string value;
};
