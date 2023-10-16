#pragma once

#include "Link.hpp"
#include "Node.hpp"
#include "Pin.hpp"

#include <imgui-node-editor/imgui_node_editor.h>

#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

struct IDs
{
    std::uint64_t reserve()
    {
        return ++current;
    }

    std::uint64_t current = 0;
};

struct App
{
public:
    App();
    ~App();

    void render(ax::NodeEditor::EditorContext* context);

    void create(std::uint64_t type);
    void link(const ax::NodeEditor::PinId& i, const ax::NodeEditor::PinId& o);

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
};
