#include "Control.hpp"

namespace ext
{
    void to_message(
        const Value<bool>& control,
        nil::nedit::proto::State::Types::Node& info //
    )
    {
        auto* s = info.add_controls()->mutable_toggle();
        s->set_value(control.value);
    }

    void to_message(
        const MinMax<int>& control,
        nil::nedit::proto::State::Types::Node& info //
    )
    {
        auto* s = info.add_controls()->mutable_spinbox();
        s->set_value(control.value);
        s->set_min(control.min);
        s->set_max(control.max);
    }

    void to_message(
        const MinMax<float>& control,
        nil::nedit::proto::State::Types::Node& info //
    )
    {
        auto* s = info.add_controls()->mutable_slider();
        s->set_value(control.value);
        s->set_min(control.min);
        s->set_max(control.max);
    }

    void to_message(
        const Value<std::string>& control,
        nil::nedit::proto::State::Types::Node& info //
    )
    {
        auto* s = info.add_controls()->mutable_text();
        s->set_value(control.value);
    }

    void to_message(
        const Enum& control,
        nil::nedit::proto::State::Types::Node& info //
    )
    {
        auto* s = info.add_controls()->mutable_combobox();
        s->set_value(control.value);
        for (const auto& selection : control.selection)
        {
            s->add_selection(selection);
        }
    }
}
