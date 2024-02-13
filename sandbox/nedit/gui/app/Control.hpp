#pragma once

#include "IDs.hpp"

#include <imgui-node-editor/imgui_node_editor.h>

#include <functional>
#include <string>

namespace nil::service
{
    class IService;
}

namespace gui
{
    class Control
    {
    public:
        Control(nil::service::IService& init_service, ID init_id);
        virtual ~Control() noexcept = default;
        virtual void render(bool interactive_controls) = 0;

    protected:
        nil::service::IService& service;

    public:
        ID id;
    };

    class ToggleControl final: public Control
    {
    public:
        ToggleControl(nil::service::IService& init_service, ID init_id, bool init_value);

        void render(bool interactive_controls) override;

    private:
        bool value;
    };

    class SpinboxControl final: public Control
    {
    public:
        SpinboxControl(
            nil::service::IService& init_service,
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
            nil::service::IService& init_service,
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
        TextControl(nil::service::IService& init_service, ID init_id, std::string init_value);

        void render(bool interactive_controls) override;

    private:
        std::string value;
    };

    class ComboBoxControl final: public Control
    {
    public:
        ComboBoxControl(
            nil::service::IService& init_service,
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
