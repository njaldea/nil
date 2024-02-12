#include "Control.hpp"
#include "../../codec.hpp"

#include <nil/service.hpp>

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

    void ToggleControl::render()
    {
        bool need_update = false;
        ImGui::PushID(int(id.value));
        ImGui::PushItemWidth(100.f);
        need_update = ImGui::Checkbox("Checkbox", &value);
        ImGui::PopItemWidth();
        ImGui::PopID();
        if (need_update)
        {
            nil::nedit::proto::ControlUpdateB msg;
            msg.set_id(id.value);
            msg.set_value(value);
            service.publish(nil::nedit::proto::message_type::ControlUpdateB, msg);
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

    void SpinboxControl::render()
    {
        bool need_update = false;
        ImGui::PushID(int(id.value));
        ImGui::PushItemWidth(100.f);
        need_update = ImGui::SliderInt("Spinbox", &value, min, max);
        ImGui::PopItemWidth();
        ImGui::PopID();
        if (need_update)
        {
            nil::nedit::proto::ControlUpdateI msg;
            msg.set_id(id.value);
            msg.set_value(value);
            service.publish(nil::nedit::proto::message_type::ControlUpdateI, msg);
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

    void SliderControl::render()
    {
        bool need_update = false;
        ImGui::PushID(int(id.value));
        ImGui::PushItemWidth(100.f);
        need_update = ImGui::SliderFloat("Slider", &value, min, max) || need_update;
        ImGui::PopItemWidth();
        ImGui::PopID();

        if (need_update)
        {
            nil::nedit::proto::ControlUpdateF msg;
            msg.set_id(id.value);
            msg.set_value(value);
            service.publish(nil::nedit::proto::message_type::ControlUpdateF, msg);
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

    void TextControl::render()
    {
        bool need_update = false;
        ImGui::PushID(int(id.value));
        ImGui::PushItemWidth(100.f);
        need_update = ImGui::InputText("Text", &value);
        ImGui::PopItemWidth();
        ImGui::PopID();

        if (need_update)
        {
            nil::nedit::proto::ControlUpdateS msg;
            msg.set_id(id.value);
            msg.set_value(value);
            service.publish(nil::nedit::proto::message_type::ControlUpdateS, msg);
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

    void ComboBoxControl::render()
    {
        bool need_update = false;
        ImGui::PushID(int(id.value));
        ImGui::PushItemWidth(100.f);

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

        ImGui::PopItemWidth();
        ImGui::PopID();

        if (need_update)
        {
            nil::nedit::proto::ControlUpdateS msg;
            msg.set_id(id.value);
            msg.set_value(value);
            service.publish(nil::nedit::proto::message_type::ControlUpdateS, msg);
        }
    }
}
