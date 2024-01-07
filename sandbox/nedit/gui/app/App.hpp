#pragma once

#include "IDs.hpp"
#include "Link.hpp"
#include "Node.hpp"
#include "Pin.hpp"

#include <imgui-node-editor/imgui_node_editor.h>

#include <functional>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

struct NodeInfo
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
    std::vector<std::function<std::unique_ptr<Control>(std::uint64_t)>> controls;
};

struct PinInfo
{
    void render()
    {
        ImGui::TextColored(icon.color, "%s", label.data());
    }

    std::unique_ptr<Pin> create(std::uint64_t id, ax::NodeEditor::PinKind kind) const
    {
        return std::make_unique<Pin>(id, kind, this, label, icon);
    }

    std::string label;
    FlowIcon icon;
};

struct App
{
public:
    void render(ax::NodeEditor::EditorContext& context);
    void render_panel();

    void create_link(const ax::NodeEditor::PinId& a, const ax::NodeEditor::PinId& b);

    void prepare_create(std::uint64_t type);
    void confirm_create(std::uint64_t type);

private:
    static void push_style();
    static void pop_style();
    void render();
    void edit_create();
    void edit_delete();

    void delete_link(std::uint64_t link_id);
    void delete_node(std::uint64_t node_id);

public:
    IDs ids;
    // [TODO] reevaluate container type
    // creation/deletion is easier with unordered_map
    // rendering (iteration) is more expensive with unordered_map
    std::unordered_map<std::uint64_t, std::unique_ptr<Node>> nodes;
    std::unordered_map<std::uint64_t, std::tuple<Node*, Pin*>> pins;
    std::unordered_map<std::uint64_t, std::unique_ptr<Link>> links;

    std::unique_ptr<Node> tmp;
    bool finalize_node = false;

    std::vector<NodeInfo> node_infos;
    std::vector<PinInfo> pin_infos;
};
