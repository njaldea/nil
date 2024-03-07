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
        Pin(ID init_id,
            ax::NodeEditor::PinKind init_kind,
            std::uint64_t init_type,
            std::string_view init_label,
            const FlowIcon* init_icon,
            bool init_flipped);
        ~Pin() noexcept = default;

        Pin(Pin&&) = default;
        Pin(const Pin&) = delete;
        Pin& operator=(Pin&&) = delete;
        Pin& operator=(const Pin&) = delete;

        void render() const;

        ID id;
        ax::NodeEditor::PinKind kind;
        std::uint64_t type;
        std::uint64_t alias;
        std::string_view label;
        const FlowIcon* icon;
        bool flipped;
    };
}
