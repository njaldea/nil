#include "App.hpp"

#include <iostream>

void App::create(std::uint64_t type_index)
{
    const auto node_id = ids.reserve();
    nodes.emplace(node_id, std::make_unique<Node>(node_id));
    auto& n = nodes[node_id];

    for (const auto& type_i : node_infos[type_index].inputs)
    {
        auto pin_id_i = ids.reserve();
        n->pins_i.emplace_back(std::make_unique<Pin>( //
            pin_id_i,
            ax::NodeEditor::PinKind::Input,
            type_i,
            *pin_infos[type_i].icon
        ));
        pins.emplace(pin_id_i, std::make_tuple(n.get(), n->pins_i.back().get()));
    }

    for (const auto& type_o : node_infos[type_index].outputs)
    {
        auto pin_id_o = ids.reserve();
        n->pins_o.emplace_back(std::make_unique<Pin>( //
            pin_id_o,
            ax::NodeEditor::PinKind::Output,
            type_o,
            *pin_infos[type_o].icon
        ));
        pins.emplace(pin_id_o, std::make_tuple(n.get(), n->pins_o.back().get()));
    }
}

void App::link(const ax::NodeEditor::PinId& i, const ax::NodeEditor::PinId& o)
{
    auto& [node_i, pin_i] = pins[i.Get()];
    auto& [node_o, pin_o] = pins[o.Get()];
    if (node_i != node_o && pin_i->kind != pin_o->kind && pin_i->type == pin_o->type)
    {
        if (pin_o->links.size() == 1)
        {
            ax::NodeEditor::RejectNewItem({1, 0, 0, 1}, 1.0);
        }
        else if (ax::NodeEditor::AcceptNewItem())
        {
            auto link_id = ids.reserve();
            links.emplace(link_id, std::make_unique<Link>(link_id, pin_i, pin_o));
            pin_i->links.emplace(link_id);
            pin_o->links.emplace(link_id);
        }
    }
    else
    {
        ax::NodeEditor::RejectNewItem({1, 0, 0, 1}, 1.0);
    }
}

void App::render(ax::NodeEditor::EditorContext* context)
{
    ax::NodeEditor::SetCurrentEditor(context);
    ax::NodeEditor::Begin("My Editor", ImVec2(0.0, 0.0f));

    style();
    render();
    edit_create();
    edit_delete();
    pop_style();

    ax::NodeEditor::End();
    ax::NodeEditor::SetCurrentEditor(nullptr);
}

void App::render()
{
    if (tmp)
    {
        if (tmp->is_ready)
        {
            auto n = tmp->consume(ids);
            for (const auto& pin : n->pins_i)
            {
                pins.emplace(pin->id.Get(), std::make_tuple(n.get(), pin.get()));
            }
            for (const auto& pin : n->pins_o)
            {
                pins.emplace(pin->id.Get(), std::make_tuple(n.get(), pin.get()));
            }
            const auto id = n->id.Get();
            nodes.emplace(id, std::move(n));
            tmp = {};
        }
        else
        {
            tmp->render();
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

void App::style()
{
    ax::NodeEditor::PushStyleColor( //
        ax::NodeEditor::StyleColor_NodeBg,
        ImColor(255, 255, 255, 64)
    );
    ax::NodeEditor::PushStyleColor(
        ax::NodeEditor::StyleColor_NodeBorder,
        ImColor(255, 255, 255, 64)
    );
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.75f);
    ax::NodeEditor::PushStyleVar(ax::NodeEditor::StyleVar_NodeBorderWidth, 0.0f);
    ax::NodeEditor::PushStyleVar(ax::NodeEditor::StyleVar_NodePadding, ImVec4(2, 3, 3, 2));
    ax::NodeEditor::PushStyleVar(ax::NodeEditor::StyleVar_NodeRounding, 0.0f);
    // ax::NodeEditor::PushStyleVar(ax::NodeEditor::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
    // ax::NodeEditor::PushStyleVar(ax::NodeEditor::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
    // ax::NodeEditor::PushStyleVar(ax::NodeEditor::StyleVar_LinkStrength, 1.0f);
    ax::NodeEditor::PushStyleVar(ax::NodeEditor::StyleVar_PinCorners, 0.0f);
    ax::NodeEditor::PushStyleVar(ax::NodeEditor::StyleVar_PinRounding, 0.0f);
    ax::NodeEditor::PushStyleVar(ax::NodeEditor::StyleVar_PinBorderWidth, 0.0f);
    ax::NodeEditor::PushStyleVar(ax::NodeEditor::StyleVar_PinRadius, 0.0f);
    // ax::NodeEditor::PushStyleVar(ax::NodeEditor::StyleVar_FlowSpeed, 5.0f);
    // ax::NodeEditor::PushStyleVar(ax::NodeEditor::StyleVar_FlowMarkerDistance, 100.0f);
}

void App::pop_style()
{
    ax::NodeEditor::PopStyleVar(7);
    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(2);
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
                link(pin_id_i, pin_id_o);
            }
        }
        ax::NodeEditor::EndCreate();
    }
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
        const auto cleanup_pins = //
            [&links_to_delete, this](const auto& current_pins)
        {
            for (const auto& pin : current_pins)
            {
                for (const auto& link_id : pin->links)
                {
                    links_to_delete.emplace(link_id);
                }
                this->pins.erase(pin->id.Get());
            }
        };
        cleanup_pins(node->second->pins_i);
        cleanup_pins(node->second->pins_o);
        for (const auto& link_id : links_to_delete)
        {
            delete_link(link_id);
        }
        nodes.erase(node);
    }
}

void App::prepare_create(std::uint64_t type)
{
    if (!tmp)
    {
        tmp = std::make_unique<ShadowNode>(type, node_infos[type], pin_infos);
    }
}

void App::confirm_create(std::uint64_t type)
{
    if (tmp && tmp->type == type)
    {
        tmp->ready();
    }
}
