#include "App.hpp"

#include "Control.hpp"

#include <iostream>

namespace
{
    bool is_reachable(
        const Node* current_node,
        const Node* target_node,
        const std::unordered_map<std::uint64_t, std::tuple<Node*, Pin*>>& pin_to_node,
        const std::unordered_map<std::uint64_t, std::unique_ptr<Link>>& links,
        std::vector<std::unique_ptr<Pin>>(Node::*pins_getter),
        Pin*(Link::*next_pin)
    )
    {
        for (const auto& pins : current_node->*pins_getter)
        {
            for (const auto& link : pins->links)
            {
                const auto& l_ptr = links.at(link);
                const auto* pin = l_ptr.get()->*next_pin;
                if (pin == nullptr)
                {
                    continue;
                }

                const auto* next_node = std::get<0>(pin_to_node.at(pin->id.Get()));
                if (next_node == target_node)
                {
                    return true;
                }

                if (is_reachable(next_node, target_node, pin_to_node, links, pins_getter, next_pin))
                {
                    return true;
                }
            }
        }
        return false;
    }
}

void App::create_link(const ax::NodeEditor::PinId& a, const ax::NodeEditor::PinId& b)
{
    if (a == b)
    {
        ax::NodeEditor::RejectNewItem(ImVec4(1, 0, 0, 1), 1.0);
        return;
    }

    auto [node_a, pin_a] = pins[a.Get()];
    auto [node_b, pin_b] = pins[b.Get()];

    if (pin_a->kind == pin_b->kind)
    {
        ax::NodeEditor::RejectNewItem(ImVec4(1, 0, 0, 1), 1.0);
        return;
    }

    if (pin_a->type != pin_b->type)
    {
        ax::NodeEditor::RejectNewItem(ImVec4(1, 0, 0, 1), 1.0);
        return;
    }

    const auto [node_start, pin_start, node_end, pin_end] //
        = pin_a->kind == ax::NodeEditor::PinKind::Output
        ? std::make_tuple(node_a, pin_a, node_b, pin_b)
        : std::make_tuple(node_b, pin_b, node_a, pin_a);

    // end pin (output) should only have 1 link connected to it
    if (pin_end->links.size() == 1)
    {
        ax::NodeEditor::RejectNewItem(ImVec4(1, 0, 0, 1), 1.0);
        return;
    }

    // [TODO] do i need to do it in both directions? one pass might be enough
    if (is_reachable(node_start, node_end, pins, links, &Node::pins_i, &Link::entry)
        || is_reachable(node_end, node_start, pins, links, &Node::pins_o, &Link::exit))
    {
        ax::NodeEditor::RejectNewItem(ImVec4(1, 0, 0, 1), 1.0);
        return;
    }

    if (ax::NodeEditor::AcceptNewItem())
    {
        auto link_id = ids.reserve();
        links.emplace(link_id, std::make_unique<Link>(link_id, Link::Info{pin_start, pin_end}));
        pin_start->links.emplace(link_id);
        pin_end->links.emplace(link_id);
    }
}

void App::render(ax::NodeEditor::EditorContext& context)
{
    ax::NodeEditor::SetCurrentEditor(&context);
    ax::NodeEditor::Begin("My Editor", ImVec2(0.0, 0.0f));

    push_style();
    render();
    edit_create();
    edit_delete();
    pop_style();

    ax::NodeEditor::End();
    ax::NodeEditor::SetCurrentEditor(nullptr);
}

void App::render_panel()
{
    ImGui::Text("Pin Types");
    for (auto& pin_info : pin_infos)
    {
        pin_info.render();
    }

    ImGui::Text("Node Type (drag)");
    for (auto n = 0u; n < node_infos.size(); n++)
    {
        auto& info = node_infos[n];
        info.render();
        if (info.is_dragged())
        {
            prepare_create(n);
        }
        else
        {
            confirm_create(n);
        }
    }
}

void App::render()
{
    if (tmp)
    {
        if (finalize_node)
        {
            const auto id = tmp->id.Get();
            nodes.emplace(id, std::move(tmp));
            finalize_node = false;
        }
        else
        {
            tmp->render();
            const auto pos = ImGui::GetIO().MousePos;
            ax::NodeEditor::SetNodePosition(tmp->id, ImVec2(pos.x - 20, pos.y - 20));
        }
    }
    for (auto& node : nodes)
    {
        node.second->render();
    }
    for (auto& link : links)
    {
        link.second->render();
    }
}

void App::push_style()
{
    using namespace ax::NodeEditor;
    PushStyleColor(StyleColor_NodeBg, ImColor(255, 255, 255, 255));
    PushStyleColor(StyleColor_NodeBorder, ImColor(255, 255, 255, 0));

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.75f);

    PushStyleVar(StyleVar_NodeBorderWidth, 0.0f);
    PushStyleVar(StyleVar_NodePadding, ImVec4(2, 3, 3, 2));
    PushStyleVar(StyleVar_NodeRounding, 0.0f);
    PushStyleVar(StyleVar_PinCorners, 0.0f);
    PushStyleVar(StyleVar_PinRounding, 0.0f);
    PushStyleVar(StyleVar_PinBorderWidth, 0.0f);
    PushStyleVar(StyleVar_PinRadius, 0.0f);

    // PushStyleVar(StyleVar_FlowSpeed, 5.0f);
    // PushStyleVar(StyleVar_FlowMarkerDistance, 100.0f);
    // PushStyleVar(StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
    // PushStyleVar(StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
    // PushStyleVar(StyleVar_LinkStrength, 1.0f);
}

void App::pop_style()
{
    ax::NodeEditor::PopStyleVar(7);
    ImGui::PopStyleVar(1);
    ax::NodeEditor::PopStyleColor(2);
}

void App::edit_create()
{
    if (ax::NodeEditor::BeginCreate())
    {
        ax::NodeEditor::PinId pin_id_i;
        ax::NodeEditor::PinId pin_id_o;
        if (ax::NodeEditor::QueryNewLink(&pin_id_i, &pin_id_o))
        {
            if (pin_id_i && pin_id_o)
            {
                create_link(pin_id_i, pin_id_o);
            }
        }
    }
    ax::NodeEditor::EndCreate();
}

void App::edit_delete()
{
    if (ax::NodeEditor::BeginDelete())
    {
        ax::NodeEditor::LinkId link_id;
        while (ax::NodeEditor::QueryDeletedLink(&link_id))
        {
            if (ax::NodeEditor::AcceptDeletedItem())
            {
                delete_link(link_id.Get());
            }
        }

        ax::NodeEditor::NodeId node_id;
        while (ax::NodeEditor::QueryDeletedNode(&node_id))
        {
            if (ax::NodeEditor::AcceptDeletedItem())
            {
                delete_node(node_id.Get());
            }
        }
    }
    ax::NodeEditor::EndDelete();
}

void App::delete_link(std::uint64_t link_id)
{
    auto link = links.find(link_id);
    if (link != links.end())
    {
        link->second->entry->links.erase(link_id);
        link->second->exit->links.erase(link_id);
        links.erase(link_id);
    }
}

void App::delete_node(std::uint64_t node_id)
{
    auto node = nodes.find(node_id);
    if (node != nodes.end())
    {
        std::unordered_set<std::uint64_t> links_to_delete;
        const auto cleanup_pins = [&links_to_delete, this](const auto& current_pins)
        {
            for (auto it = current_pins.rbegin(); it != current_pins.rend(); ++it)
            {
                const auto* pin = it->get();
                const auto pin_id = pin->id.Get();
                for (const auto& link_id : pin->links)
                {
                    links_to_delete.emplace(link_id);
                }
                ids.release(pin_id);
            }
        };
        cleanup_pins(node->second->pins_i);
        cleanup_pins(node->second->pins_o);
        for (const auto& link_id : links_to_delete)
        {
            delete_link(link_id);
            ids.release(link_id);
        }
        for (const auto& control : node->second->controls)
        {
            ids.release(control->id.Get());
        }
        nodes.erase(node);
        ids.release(node_id);
    }
}

void App::prepare_create(std::uint64_t type_index)
{
    if (!tmp)
    {
        const auto node_id = ids.reserve();
        tmp = std::make_unique<Node>(type_index, node_id, node_infos[type_index].label);

        for (const auto& type_i : node_infos[type_index].inputs)
        {
            auto pin_id_i = ids.reserve();
            tmp->pins_i.emplace_back(
                pin_infos[type_i].create(pin_id_i, ax::NodeEditor::PinKind::Input)
            );
            pins.emplace(pin_id_i, std::make_tuple(tmp.get(), tmp->pins_i.back().get()));
        }

        for (const auto& type_o : node_infos[type_index].outputs)
        {
            auto pin_id_o = ids.reserve();
            tmp->pins_o.emplace_back(
                pin_infos[type_o].create(pin_id_o, ax::NodeEditor::PinKind::Output)
            );
            pins.emplace(pin_id_o, std::make_tuple(tmp.get(), tmp->pins_o.back().get()));
        }
        for (const auto& control : node_infos[type_index].controls)
        {
            tmp->controls.push_back(control(ids.reserve()));
        }
    }
}

void App::confirm_create(std::uint64_t type)
{
    if (tmp && tmp->type == type)
    {
        finalize_node = true;
    }
}
