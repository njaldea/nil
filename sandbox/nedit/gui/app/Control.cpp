#include "Control.hpp"
#include "../../codec.hpp"

#include <nil/service/IService.hpp>
#include <nil/service/concat.hpp>

#include <gen/nedit/messages/control_update.pb.h>
#include <gen/nedit/messages/type.pb.h>

#include <string>

#include <imgui_stdlib.h>

namespace gui
{
    Control::Control(nil::service::IService& init_service, ID init_id)
        : service(init_service)
        , id(std::move(init_id))
    {
    }

    ToggleControl::ToggleControl(nil::service::IService& init_service, ID init_id, bool init_value)
        : Control(init_service, std::move(init_id))
        , value(init_value)
    {
    }

    void ToggleControl::render(bool interactive_controls)
    {
        ImGui::PushID(int(id.value));
        ImGui::PushItemWidth(100.f);
        ImGui::BeginDisabled(!interactive_controls);
        const auto need_update = ImGui::Checkbox("Checkbox", &value);
        ImGui::EndDisabled();
        ImGui::PopItemWidth();
        ImGui::PopID();

        if (need_update && interactive_controls)
        {
            nil::nedit::proto::ControlUpdateB msg;
            msg.set_id(id.value);
            msg.set_value(value);
            service.publish(
                nil::service::concat(nil::nedit::proto::message_type::ControlUpdateB, msg)
            );
        }
    }

    SpinboxControl::SpinboxControl(
        nil::service::IService& init_service,
        ID init_id,
        std::int32_t init_value,
        std::int32_t init_min,
        std::int32_t init_max
    )
        : Control(init_service, std::move(init_id))
        , value(init_value)
        , min(init_min)
        , max(init_max)
    {
    }

    void SpinboxControl::render(bool interactive_controls)
    {
        ImGui::PushID(int(id.value));
        ImGui::PushItemWidth(100.f);
        ImGui::BeginDisabled(!interactive_controls);
        const auto need_update = ImGui::SliderInt("Spinbox", &value, min, max);
        ImGui::EndDisabled();
        ImGui::PopItemWidth();
        ImGui::PopID();

        if (need_update && interactive_controls)
        {
            nil::nedit::proto::ControlUpdateI msg;
            msg.set_id(id.value);
            msg.set_value(value);
            service.publish(
                nil::service::concat(nil::nedit::proto::message_type::ControlUpdateI, msg)
            );
        }
    }

    SliderControl::SliderControl(
        nil::service::IService& init_service,
        ID init_id,
        float init_value,
        float init_min,
        float init_max
    )
        : Control(init_service, std::move(init_id))
        , value(init_value)
        , min(init_min)
        , max(init_max)
    {
    }

    void SliderControl::render(bool interactive_controls)
    {
        ImGui::PushID(int(id.value));
        ImGui::PushItemWidth(100.f);
        ImGui::BeginDisabled(!interactive_controls);
        const auto need_update = ImGui::SliderFloat("Slider", &value, min, max);
        ImGui::EndDisabled();
        ImGui::PopItemWidth();
        ImGui::PopID();

        if (need_update && interactive_controls)
        {
            nil::nedit::proto::ControlUpdateF msg;
            msg.set_id(id.value);
            msg.set_value(value);
            service.publish(
                nil::service::concat(nil::nedit::proto::message_type::ControlUpdateF, msg)
            );
        }
    }

    TextControl::TextControl(
        nil::service::IService& init_service,
        ID init_id,
        std::string init_value
    )
        : Control(init_service, std::move(init_id))
        , value(std::move(init_value))
    {
    }

    void TextControl::render(bool interactive_controls)
    {
        ImGui::PushID(int(id.value));
        ImGui::PushItemWidth(100.f);
        ImGui::BeginDisabled(!interactive_controls);
        const auto need_update = ImGui::InputText("Text", &value);
        ImGui::EndDisabled();
        ImGui::PopItemWidth();
        ImGui::PopID();

        if (need_update && interactive_controls)
        {
            nil::nedit::proto::ControlUpdateS msg;
            msg.set_id(id.value);
            msg.set_value(value);
            service.publish(
                nil::service::concat(nil::nedit::proto::message_type::ControlUpdateS, msg)
            );
        }
    }

    ComboBoxControl::ComboBoxControl(
        nil::service::IService& init_service,
        ID init_id,
        std::string init_value,
        std::vector<std::string> init_selection
    )
        : Control(init_service, std::move(init_id))
        , value(std::move(init_value))
        , selection(std::move(init_selection))
    {
    }

    void ComboBoxControl::render(bool interactive_controls)
    {
        bool need_update = false;
        ImGui::PushID(int(id.value));
        ImGui::PushItemWidth(100.f);

        ImGui::BeginDisabled(!interactive_controls);
        ax::NodeEditor::Suspend();
        if (ImGui::BeginCombo("ComboBox", value.c_str()))
        {
            for (const auto& item : selection)
            {
                const bool is_selected = value == item;
                if (ImGui::Selectable(item.c_str(), is_selected))
                {
                    value = item;
                    need_update = true;
                }
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ax::NodeEditor::Resume();
        ImGui::EndDisabled();

        ImGui::PopItemWidth();
        ImGui::PopID();

        if (need_update && interactive_controls)
        {
            nil::nedit::proto::ControlUpdateS msg;
            msg.set_id(id.value);
            msg.set_value(value);
            service.publish(
                nil::service::concat(nil::nedit::proto::message_type::ControlUpdateS, msg)
            );
        }
    }
}
