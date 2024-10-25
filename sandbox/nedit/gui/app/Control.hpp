#pragma once

#include "IDs.hpp"

#include <nil/service.hpp>

#include <imgui-node-editor/imgui_node_editor.h>

#include <string>
#include <vector>

namespace gui
{
    class Control
    {
    public:
        Control(nil::service::MessagingService& init_service, ID init_id);

        Control() = delete;
        virtual ~Control() noexcept = default;

        Control(Control&&) noexcept = delete;
        Control& operator=(Control&&) noexcept = delete;

        Control(const Control&) = delete;
        Control& operator=(const Control&) = delete;

        virtual void render(bool interactive_controls) = 0;

    protected:
        nil::service::MessagingService& service;

    public:
        ID id;
    };

    class ToggleControl final: public Control
    {
    public:
        ToggleControl(nil::service::MessagingService& init_service, ID init_id, bool init_value);

        void render(bool interactive_controls) override;

    private:
        bool value;
    };

    class SpinboxControl final: public Control
    {
    public:
        SpinboxControl(
            nil::service::MessagingService& init_service,
            ID init_id,
            std::int32_t init_value,
            std::int32_t init_min,
            std::int32_t init_max
        );

        void render(bool interactive_controls) override;

    private:
        std::int32_t value;
        std::int32_t min;
        std::int32_t max;
    };

    class SliderControl final: public Control
    {
    public:
        SliderControl(
            nil::service::MessagingService& init_service,
            ID init_id,
            float init_value,
            float init_min,
            float init_max
        );

        void render(bool interactive_controls) override;

    private:
        float value;
        float min;
        float max;
    };

    class TextControl final: public Control
    {
    public:
        TextControl(
            nil::service::MessagingService& init_service,
            ID init_id,
            std::string init_value
        );

        void render(bool interactive_controls) override;

    private:
        std::string value;
    };

    class ComboBoxControl final: public Control
    {
    public:
        ComboBoxControl(
            nil::service::MessagingService& init_service,
            ID init_id,
            std::string init_value,
            std::vector<std::string> init_selection
        );

        void render(bool interactive_controls) override;

    private:
        std::string value;
        std::vector<std::string> selection;
    };
}
