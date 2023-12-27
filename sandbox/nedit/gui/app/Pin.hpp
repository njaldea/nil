#pragma once

#include "FlowIcon.hpp"
#include "IDs.hpp"

#include <string_view>
#include <unordered_set>

namespace gui
{
    struct Link;
    struct Node;

    struct Pin final
    {
        Pin(IDs& init_ids,
            ax::NodeEditor::PinKind init_kind,
            const void* init_type,
            std::string_view init_label,
            const FlowIcon& init_icon);
        ~Pin() noexcept = default;

        Pin(Pin&&) = default;
        Pin(const Pin&) = delete;
        Pin& operator=(Pin&&) = delete;
        Pin& operator=(const Pin&) = delete;

        void render() const;

        ID id;
        ax::NodeEditor::PinKind kind;
        const void* type;
        std::string_view label;
        const FlowIcon& icon;
    };
}
