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

struct ToggleControl: Control
{
    ToggleControl(
        ax::NodeEditor::PinId init_id,
        bool init_value,
        std::function<void(bool)> init_notify
    );

    void render() override;

    bool value;
    std::function<void(bool)> notify;
};

struct SpinboxControl: Control
{
    SpinboxControl(
        ax::NodeEditor::PinId init_id,
        std::int32_t init_value,
        std::int32_t init_min,
        std::int32_t init_max,
        std::function<void(std::int32_t)> init_notify
    );

    void render() override;

    std::int32_t value;
    std::int32_t min;
    std::int32_t max;
    std::function<void(std::int32_t)> notify;
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
    TextControl(
        ax::NodeEditor::PinId init_id,
        std::string init_value,
        std::function<void(const std::string&)> init_notify
    );

    void render() override;

    std::string value;
    std::function<void(const std::string&)> notify;
};
