#pragma once

#include "IDs.hpp"
#include "Link.hpp"
#include "Node.hpp"
#include "Pin.hpp"

#include <imgui-node-editor/imgui_node_editor.h>

#include <functional>
#include <memory>
#include <mutex>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace gui
{
    struct NodeInfo final
    {
        void render()
        {
            ImGui::Selectable(label.c_str());
        }

        bool is_dragged()
        {
            constexpr auto src_flags                      //
                = ImGuiDragDropFlags_SourceNoDisableHover //
                | ImGuiDragDropFlags_SourceNoPreviewTooltip
                | ImGuiDragDropFlags_SourceNoHoldToOpenOthers;
            if (ImGui::BeginDragDropSource(src_flags))
            {
                ImGui::EndDragDropSource();
                return true;
            }
            return false;
        }

        bool is_dropped()
        {
            if (ImGui::BeginDragDropTarget())
            {
                ImGui::EndDragDropTarget();
                return true;
            }
            return false;
        }

        std::string label;
        std::vector<std::uint64_t> inputs;
        std::vector<std::uint64_t> outputs;
        std::vector<std::function<std::unique_ptr<Control>(ID)>> controls;
    };

    struct PinInfo final
    {
        void render()
        {
            ImGui::TextColored(icon.color, "%s", label.data());
        }

        std::string label;
        FlowIcon icon;
    };

    struct PinMapping final
    {
        Node* node;
        Pin* pin;
        std::unordered_set<Link*> links;
    };

    struct App final
    {
    public:
        void render(ax::NodeEditor::EditorContext& context);
        void render_panel();

        void create_link(const ax::NodeEditor::PinId& a, const ax::NodeEditor::PinId& b);

        void prepare_create(std::uint64_t type);
        void confirm_create(std::uint64_t type);

        void load_node(
            std::uint64_t node_id,
            std::uint64_t type_index,
            std::vector<std::uint64_t> input_ids,
            std::vector<std::uint64_t> output_ids,
            std::vector<std::uint64_t> control_ids
        );

        void load_link(
            std::uint64_t link_id,
            std::uint64_t entry_pin_id,
            std::uint64_t exit_pin_id
        );

        void reset();

        bool pop_diff();

    private:
        static void push_style();
        static void pop_style();
        void render();
        void edit_create();
        void edit_delete();

        void delete_node(std::uint64_t node_id);

    public:
        // Node/Pin/Link/Controls share the same ID
        IDs ids;

        // Used unordered_map for easier search for deletion
        // Used unique_ptr for easier memory stability (for PinMapping)
        std::unordered_map<std::uint64_t, std::unique_ptr<Node>> nodes;
        std::unordered_map<std::uint64_t, std::unique_ptr<Link>> links;
        std::unordered_map<std::uint64_t, PinMapping> pins;

        std::unique_ptr<Node> tmp;

        std::vector<NodeInfo> node_infos;
        std::vector<PinInfo> pin_infos;

        bool changed = false;
        std::mutex mutex;
        std::vector<std::function<void()>> before_render;
        std::vector<std::function<void()>> after_render;

        bool allow_editing = false;
    };
}
