#pragma once

#include <imgui-node-editor/imgui_node_editor.h>

#include <functional>
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
    SliderControl(
        ax::NodeEditor::PinId init_id,
        float init_value,
        float init_min,
        float init_max,
        std::function<void(float)> init_notify
    );

    void render() override;

    float value;
    float min;
    float max;
    std::function<void(float)> notify;
};

struct TextControl: Control
{
    TextControl(ax::NodeEditor::PinId init_id, std::string init_value);

    void render() override;

    std::string value;
};
