#pragma once

#include "IDs.hpp"

#include <imgui-node-editor/imgui_node_editor.h>

namespace gui
{
    struct Pin;

    struct Link final
    {
        struct Info
        {
            Pin* entry;
            Pin* exit;
        };

        Link(ID init_id, Info info);
        ~Link() noexcept = default;

        Link() = delete;

        Link(Link&&) noexcept = delete;
        Link& operator=(Link&&) noexcept = delete;

        Link(const Link&) = delete;
        Link& operator=(const Link&) = delete;

        void render() const;

        ID id;
        Pin* entry;
        Pin* exit;
    };
}
