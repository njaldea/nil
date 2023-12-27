#include "App.hpp"
#include "Control.hpp"

#include <set>

namespace
{
    bool is_reachable(
        const gui::Node* current_node,
        const gui::Node* target_node,
        const std::unordered_map<std::uint64_t, gui::PinMapping>& pin_mapping,
        const std::unordered_map<std::uint64_t, std::unique_ptr<gui::Link>>& links,
        std::vector<gui::Pin>(gui::Node::*pins_getter),
        gui::Pin*(gui::Link::*next_pin)
    )
    {
        for (const auto& pin : current_node->*pins_getter)
        {
            for (const auto& pin_links : pin_mapping.at(pin.id.value).links)
            {
                const auto* npin = pin_links->*next_pin;
                if (npin == nullptr)
                {
                    continue;
                }

                const auto* next_node = pin_mapping.at(npin->id.value).node;
                if (next_node == target_node)
                {
                    return true;
                }

                if (is_reachable(next_node, target_node, pin_mapping, links, pins_getter, next_pin))
                {
                    return true;
                }
            }
        }
        return false;
    }
}

namespace gui
{
    void App::create_link(const ax::NodeEditor::PinId& a, const ax::NodeEditor::PinId& b)
    {
        if (a == b)
        {
            ax::NodeEditor::RejectNewItem(ImVec4(1, 0, 0, 1), 1.0);
            return;
        }

        auto& detail_a = pins.at(a.Get());
        auto& detail_b = pins.at(b.Get());

        if (detail_a.pin->kind == detail_b.pin->kind    //
            || detail_a.pin->type != detail_b.pin->type //
            || detail_a.node == detail_b.node)
        {
            ax::NodeEditor::RejectNewItem(ImVec4(1, 0, 0, 1), 1.0);
            return;
        }

        const auto is_a_start = detail_a.pin->kind == ax::NodeEditor::PinKind::Output;
        auto& start = is_a_start ? detail_a : detail_b;
        auto& end = is_a_start ? detail_b : detail_a;

        // end pin (output) should only have 1 link connected to it
        if (end.links.size() == 1)
        {
            ax::NodeEditor::RejectNewItem(ImVec4(1, 0, 0, 1), 1.0);
            return;
        }

        // [TODO] do i need to do it in both directions? one pass might be enough
        if (is_reachable(start.node, end.node, pins, links, &Node::pins_i, &Link::entry)
            || is_reachable(end.node, start.node, pins, links, &Node::pins_o, &Link::exit))
        {
            ax::NodeEditor::RejectNewItem(ImVec4(1, 0, 0, 1), 1.0);
            return;
        }

        if (ax::NodeEditor::AcceptNewItem())
        {
            auto link = std::make_unique<Link>(ids, Link::Info{start.pin, end.pin});
            auto* ptr = link.get();
            const auto id = link->id.value;
            links.emplace(id, std::move(link));
            start.links.emplace(ptr);
            end.links.emplace(ptr);
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
                const auto id = tmp->id.value;
                nodes.emplace(id, std::move(tmp));
                finalize_node = false;
            }
            else
            {
                tmp->render();
                const auto pos = ImGui::GetIO().MousePos;
                ax::NodeEditor::SetNodePosition(tmp->id.value, ImVec2(pos.x - 20, pos.y - 20));
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
        namespace ax_ne = ax::NodeEditor;
        ax_ne::PushStyleColor(ax_ne::StyleColor_NodeBg, ImColor(255, 255, 255, 255));
        ax_ne::PushStyleColor(ax_ne::StyleColor_NodeBorder, ImColor(255, 255, 255, 0));

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.75f);

        ax_ne::PushStyleVar(ax_ne::StyleVar_NodeBorderWidth, 0.0f);
        ax_ne::PushStyleVar(ax_ne::StyleVar_NodePadding, ImVec4(2, 3, 3, 2));
        ax_ne::PushStyleVar(ax_ne::StyleVar_NodeRounding, 0.0f);
        ax_ne::PushStyleVar(ax_ne::StyleVar_PinCorners, 0.0f);
        ax_ne::PushStyleVar(ax_ne::StyleVar_PinRounding, 0.0f);
        ax_ne::PushStyleVar(ax_ne::StyleVar_PinBorderWidth, 0.0f);
        ax_ne::PushStyleVar(ax_ne::StyleVar_PinRadius, 0.0f);

        // PushStyleVar(StyleVar_FlowSpeed, 5.0f);
        // PushStyleVar(StyleVar_FlowMarkerDistance, 100.0f);
        // PushStyleVar(StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
        // PushStyleVar(StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
        // PushStyleVar(StyleVar_LinkStrength, 1.0f);
    }

    void App::pop_style()
    {
        namespace ax_ne = ax::NodeEditor;
        ax_ne::PopStyleVar(7);
        ImGui::PopStyleVar(1);
        ax_ne::PopStyleColor(2);
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
                    auto link = links.find(link_id.Get());
                    if (link != links.end())
                    {
                        Link* current_link = link->second.get();
                        pins.at(current_link->entry->id.value).links.erase(current_link);
                        pins.at(current_link->exit->id.value).links.erase(current_link);
                        links.erase(link);
                    }
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

    void App::delete_node(std::uint64_t node_id)
    {
        auto node_it = nodes.find(node_id);
        if (node_it != nodes.end())
        {
            auto& node = node_it->second;
            std::set<Link*> links_to_delete;
            const auto cleanup_pins = [&links_to_delete, this](const auto& current_pins)
            {
                for (const auto& pin : current_pins)
                {
                    for (Link* link : pins.at(pin.id.value).links)
                    {
                        links_to_delete.emplace(link);
                    }
                }
            };
            cleanup_pins(node->pins_i);
            cleanup_pins(node->pins_o);
            for (auto it = links_to_delete.rbegin(); it != links_to_delete.rend(); ++it)
            {
                Link* link = *it;
                pins.at(link->entry->id.value).links.erase(link);
                pins.at(link->exit->id.value).links.erase(link);
                links.erase(link->id.value);
            }
            for (const auto& pin : node->pins_o)
            {
                pins.erase(pin.id.value);
            }
            for (const auto& pin : node->pins_i)
            {
                pins.erase(pin.id.value);
            }
            nodes.erase(node_it);
        }
    }

    void App::prepare_create(std::uint64_t type_index)
    {
        if (!tmp)
        {
            tmp = std::make_unique<Node>(ids, type_index, node_infos[type_index].label);

            tmp->pins_i.reserve(node_infos[type_index].inputs.size());
            for (const auto& type_i : node_infos[type_index].inputs)
            {
                auto& pin_info = pin_infos[type_i];
                auto& pin = tmp->pins_i.emplace_back(
                    ids,
                    ax::NodeEditor::PinKind::Input,
                    &pin_info,
                    pin_info.label,
                    pin_info.icon
                );
                pins.emplace(pin.id.value, PinMapping{tmp.get(), &pin, {}});
            }

            tmp->pins_o.reserve(node_infos[type_index].outputs.size());
            for (const auto& type_o : node_infos[type_index].outputs)
            {
                auto& pin_info = pin_infos[type_o];
                auto& pin = tmp->pins_o.emplace_back(
                    ids,
                    ax::NodeEditor::PinKind::Output,
                    &pin_info,
                    pin_info.label,
                    pin_info.icon
                );
                pins.emplace(pin.id.value, PinMapping{tmp.get(), &pin, {}});
            }
            for (const auto& control : node_infos[type_index].controls)
            {
                tmp->controls.push_back(control(ids));
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
}
