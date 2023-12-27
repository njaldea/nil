#pragma once

#include <imgui-node-editor/imgui_node_editor.h>

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
