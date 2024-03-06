#pragma once

#include "IDs.hpp"

#include <imgui-node-editor/imgui_node_editor.h>

#include <memory>
#include <string_view>
#include <vector>

namespace gui
{
    struct Pin;
    struct Control;

    struct Node final
    {
        Node(ID init_id, std::uint64_t init_type, std::string_view init_label, bool init_flipped);
        ~Node() noexcept = default;

        Node(Node&&) = default;
        Node(const Node&) = delete;
        Node& operator=(Node&&) = default;
        Node& operator=(const Node&) = delete;

        void render(bool interactive_controls) const;

        ID id;
        std::uint64_t type;
        std::string_view label;
        std::vector<Pin> pins_i;
        std::vector<Pin> pins_o;
        std::vector<std::unique_ptr<Control>> controls;

        bool flipped;
        bool activated = false;
    };
}
