#pragma once

#include "IDs.hpp"
#include "Link.hpp"
#include "Node.hpp"
#include "Pin.hpp"
#include "ShadowNode.hpp"

#include <imgui-node-editor/imgui_node_editor.h>

#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

struct App
{
public:
    App();
    ~App();

    void render(ax::NodeEditor::EditorContext* context);

    void create(std::uint64_t type);
    void link(const ax::NodeEditor::PinId& i, const ax::NodeEditor::PinId& o);

    void prepare_create(std::uint64_t type);
    void confirm_create(std::uint64_t type);

    void add_node_type(NodeInfo node_info)
    {
        // validate if all pin types are correct first.
        node_infos.emplace_back(std::move(node_info));
    }

    std::size_t node_type_count() const
    {
        return node_infos.size();
    }

    const char* node_type_label(std::size_t index) const
    {
        return node_infos[index].label.c_str();
    }

    void add_pin_type(PinInfo pin_info)
    {
        pin_infos.emplace_back(std::move(pin_info));
    }

    std::size_t pin_type_count() const
    {
        return pin_infos.size();
    }

    const char* pin_type_label(std::size_t index) const
    {
        return pin_infos[index].label.c_str();
    }

private:
    void style();
    void pop_style();
    void render();
    void edit_create();
    void edit_delete();

    void delete_link(std::uint64_t link_id);
    void delete_node(std::uint64_t node_id);

    IDs ids;
    // TODO: reevaluate container type
    // creation/deletion is easier with unordered_map
    // rendering (iteration) is more expensive with unordered_map
    std::unordered_map<std::uint64_t, std::unique_ptr<Node>> nodes;
    std::unordered_map<std::uint64_t, std::tuple<Node*, Pin*>> pins;
    std::unordered_map<std::uint64_t, std::unique_ptr<Link>> links;

    std::unique_ptr<ShadowNode> tmp;

    std::vector<NodeInfo> node_infos;
    std::vector<PinInfo> pin_infos;
};
