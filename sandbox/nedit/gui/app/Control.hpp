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
    using Control::Control;

    void render() override;

    float value = 0.0f;
};

struct TextControl: Control
{
    using Control::Control;

    void render() override;

    std::string value = []()
    {
        std::string v;
        v.resize(100);
        return v;
    }();
};
