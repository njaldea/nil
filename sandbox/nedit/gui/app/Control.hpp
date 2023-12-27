#pragma once

#include "IDs.hpp"

#include <imgui-node-editor/imgui_node_editor.h>

#include <functional>
#include <string>

namespace gui
{
    struct Control
    {
        Control(IDs& init_ids);
        virtual ~Control() noexcept = default;
        virtual void render() = 0;

        ID id;
    };

    struct ToggleControl final: Control
    {
        ToggleControl(
            IDs& init_ids,
            bool init_value,
            std::function<void(std::uint64_t, bool)> init_notify
        );

        void render() override;

        bool value;
        std::function<void(std::uint64_t, bool)> notify;
    };

    struct SpinboxControl final: Control
    {
        SpinboxControl(
            IDs& init_ids,
            std::int32_t init_value,
            std::int32_t init_min,
            std::int32_t init_max,
            std::function<void(std::uint64_t, std::int32_t)> init_notify
        );

        void render() override;

        std::int32_t value;
        std::int32_t min;
        std::int32_t max;
        std::function<void(std::uint64_t, std::int32_t)> notify;
    };

    struct SliderControl final: Control
    {
        SliderControl(
            IDs& init_ids,
            float init_value,
            float init_min,
            float init_max,
            std::function<void(std::uint64_t, float)> init_notify
        );

        void render() override;

        float value;
        float min;
        float max;
        std::function<void(std::uint64_t, float)> notify;
    };

    struct TextControl final: Control
    {
        TextControl(
            IDs& init_ids,
            std::string init_value,
            std::function<void(std::uint64_t, const std::string&)> init_notify
        );

        void render() override;

        std::string value;
        std::function<void(std::uint64_t, const std::string&)> notify;
    };

    struct ComboBoxControl final: Control
    {
        ComboBoxControl(
            IDs& init_ids,
            std::string init_value,
            std::vector<std::string> init_selection,
            std::function<void(std::uint64_t, const std::string&)> init_notify
        );

        void render() override;

        std::string value;
        std::vector<std::string> selection;
        std::function<void(std::uint64_t, const std::string&)> notify;
    };
}
