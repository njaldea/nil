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
    std::string label;
    std::vector<std::uint64_t> inputs;
    std::vector<std::uint64_t> outputs;
    std::vector<std::function<std::unique_ptr<Control>(std::uint64_t)>> controls;
};

struct PinInfo
{
    std::string label;
    FlowIcon icon;
};

struct App
{
public:
    void render(ax::NodeEditor::EditorContext* context);

    void create_link(const ax::NodeEditor::PinId& i, const ax::NodeEditor::PinId& o);

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
