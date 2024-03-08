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
        if (current_node->type == 0)
        {
            return false;
        }
        for (const auto& pin : current_node->*pins_getter)
        {
            for (const auto& pin_links : pin_mapping.at(pin.id.value).links)
            {
                const auto* npin = pin_links->*next_pin;

                const auto* next_node = pin_mapping.at(npin->id.value).node;
                if (next_node->type == 0)
                {
                    return false;
                }

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

        if (detail_a.pin->kind == detail_b.pin->kind || detail_a.node == detail_b.node)
        {
            ax::NodeEditor::RejectNewItem(ImVec4(1, 0, 0, 1), 1.0);
            return;
        }

        if ((detail_a.node->type == 0 || detail_a.node->type == 1)
            && (detail_a.pin->icon != &pin_infos[0].icon)
            && (detail_a.pin->icon != detail_b.pin->icon))
        {
            ax::NodeEditor::RejectNewItem(ImVec4(1, 0, 0, 1), 1.0);
            return;
        }

        if ((detail_b.node->type == 0 || detail_b.node->type == 1)
            && (detail_b.pin->icon != &pin_infos[0].icon)
            && (detail_b.pin->icon != detail_a.pin->icon))
        {
            ax::NodeEditor::RejectNewItem(ImVec4(1, 0, 0, 1), 1.0);
            return;
        }

        if ((detail_a.node->type != 0 && detail_a.node->type != 1)
            && !(detail_a.node->type != 0 && detail_a.node->type != 1)
            && (detail_a.pin->type != detail_b.pin->type))
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
            // TODO: propagate forward in case of chained feedback/delay nodes
            if (start.pin->type == 0)
            {
                start.pin->icon = end.pin->icon;
                start.node->pins_i[0].icon = end.pin->icon;

                if (end.pin->type == 0)
                {
                    start.pin->alias = end.pin->alias;
                    start.node->pins_i[0].alias = end.pin->alias;
                }
                else
                {
                    start.pin->alias = end.pin->type;
                    start.node->pins_i[0].alias = end.pin->type;
                }
            }
            else if (end.pin->type == 0)
            {
                end.pin->icon = start.pin->icon;
                end.node->pins_o[0].icon = start.pin->icon;
                if (start.pin->type == 0)
                {
                    end.pin->alias = start.pin->alias;
                    end.node->pins_o[0].alias = start.pin->alias;
                }
                else
                {
                    end.pin->alias = start.pin->type;
                    end.node->pins_o[0].alias = start.pin->type;
                }
            }

            auto link = std::make_unique<Link>(ids.reserve(), Link::Info{start.pin, end.pin});
            auto* ptr = link.get();
            const auto id = link->id.value;
            links.emplace(id, std::move(link));
            start.links.emplace(ptr);
            end.links.emplace(ptr);
            changed = true;
        }
    }

    void App::render(ax::NodeEditor::EditorContext& context)
    {
        const auto [b, a] = [this]()
        {
            const auto _ = std::unique_lock(this->mutex);
            return std::make_tuple(
                std::exchange(this->before_render, {}),
                std::exchange(this->after_render, {})
            );
        }();

        ax::NodeEditor::SetCurrentEditor(&context);
        ax::NodeEditor::Begin("My Editor", ImVec2(0.0, 0.0f));

        for (const auto& before : b)
        {
            before();
        }

        push_style();
        render();
        if (allow_editing)
        {
            edit_create();
            edit_delete();
        }
        pop_style();

        for (const auto& after : a)
        {
            after();
        }

        ax::NodeEditor::End();
        ax::NodeEditor::SetCurrentEditor(nullptr);
    }

    void App::reset()
    {
        nodes.clear();
        links.clear();
        pins.clear();
        tmp.reset();
        node_infos.clear();
        pin_infos.clear();
        ids = {};
    }

    void App::render_panel()
    {
        ImGui::Text("Pin Types");
        for (auto& pin_info : pin_infos)
        {
            ImGui::PushItemWidth(30.0f);
            ImGui::LabelText("", " >  ");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            pin_info.render();
        }

        ImGui::Text("Node Type (drag)");
        for (auto n = 0u; n < node_infos.size(); n++)
        {
            auto& info = node_infos[n];
            ImGui::PushItemWidth(30.0f);
            ImGui::LabelText("", " >  ");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            info.render();
            if (allow_editing)
            {
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
    }

    void App::render()
    {
        if (tmp)
        {
            tmp->render(!allow_editing);
            const auto pos = ImGui::GetIO().MousePos;
            ax::NodeEditor::SetNodePosition(tmp->id.value, ImVec2(pos.x - 20, pos.y - 20));
        }
        for (auto& node : nodes)
        {
            node.second->render(!allow_editing);
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
        ax_ne::PushStyleVar(ax_ne::StyleVar_NodePadding, ImVec4(4, 4, 4, 4));
        ax_ne::PushStyleVar(ax_ne::StyleVar_NodeRounding, 2.0f);
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

    void App::delete_node(std::uint64_t node_id)
    {
        auto node_it = nodes.find(node_id);
        if (node_it != nodes.end())
        {
            using Mapping = std::unordered_map<std::uint64_t, PinMapping>;
            auto& node = node_it->second;
            const auto delete_links
                = [this](const std::vector<Pin>& current_pins, const Mapping& stored_pins)
            {
                for (const auto& pin : current_pins)
                {
                    const auto& pin_detail = stored_pins.at(pin.id.value);
                    const auto l = pin_detail.links;
                    for (auto* link : l)
                    {
                        delete_link(link->id.value);
                    }
                }
            };
            const auto delete_pins
                = [this](const std::vector<Pin>& current_pins, Mapping& stored_pins)
            {
                for (const auto& pin : current_pins)
                {
                    stored_pins.erase(pin.id.value);
                }
            };
            delete_links(node->pins_i, pins);
            delete_links(node->pins_o, pins);
            delete_pins(node->pins_i, pins);
            delete_pins(node->pins_o, pins);
            nodes.erase(node_it);
            changed = true;
        }
    }

    void App::delete_link(std::uint64_t link_id)
    {
        auto link = links.find(link_id);
        if (link != links.end())
        {
            Link* current_link = link->second.get();
            auto& pin_in = pins.at(current_link->entry->id.value);
            if (pin_in.pin->type == 0)
            {
                if (pins.at(pin_in.node->pins_i[0].id.value).links.empty())
                {
                    auto* icon = &pin_infos[0].icon;
                    pin_in.pin->icon = icon;
                    pin_in.pin->alias = 0;
                    pin_in.node->pins_i[0].icon = icon;
                    pin_in.node->pins_i[0].alias = 0;
                }
            }
            pin_in.links.erase(current_link);

            auto& pin_out = pins.at(current_link->exit->id.value);
            if (pin_out.pin->type == 0)
            {
                if (pins.at(pin_out.node->pins_o[0].id.value).links.empty())
                {
                    auto* icon = &pin_infos[0].icon;
                    pin_out.pin->icon = icon;
                    pin_out.pin->alias = 0;
                    pin_out.node->pins_o[0].icon = icon;
                    pin_out.node->pins_o[0].alias = 0;
                }
            }
            pin_out.links.erase(current_link);
            links.erase(link);
            changed = true;
        }
    }

    void App::load_node(
        std::uint64_t node_id,
        std::uint64_t type_index,
        std::vector<std::uint64_t> input_ids,
        std::vector<std::uint64_t> output_ids,
        std::vector<std::uint64_t> control_ids
    )
    {
        auto& node_info = node_infos[type_index];
        auto tmp_node = std::make_unique<Node>(
            ids.reserve(node_id),
            type_index,
            type_index == 0 ? "" : node_info.label.c_str(),
            type_index == 0
        );

        // input_ids should be equal to node_info.inputs
        tmp_node->pins_i.reserve(node_info.inputs.size());
        for (auto index = 0u; index < input_ids.size(); ++index)
        {
            auto pin_type = node_info.inputs[index];
            auto& pin_info = pin_infos[pin_type];
            auto& pin = tmp_node->pins_i.emplace_back(
                ids.reserve(input_ids[index]),
                ax::NodeEditor::PinKind::Input,
                pin_type,
                pin_info.label,
                &pin_info.icon,
                type_index == 0
            );
            pins.emplace(pin.id.value, PinMapping{tmp_node.get(), &pin, {}});
        }

        // output_ids should be equal to node_info.outputs
        tmp_node->pins_o.reserve(node_info.outputs.size());
        for (auto index = 0u; index < output_ids.size(); ++index)
        {
            auto pin_type = node_info.outputs[index];
            auto& pin_info = pin_infos[pin_type];
            auto& pin = tmp_node->pins_o.emplace_back(
                ids.reserve(output_ids[index]),
                ax::NodeEditor::PinKind::Output,
                pin_type,
                pin_info.label,
                &pin_info.icon,
                type_index == 0
            );
            pins.emplace(pin.id.value, PinMapping{tmp_node.get(), &pin, {}});
        }

        // control_ids should be equal to node_info.controls
        for (auto index = 0u; index < control_ids.size(); ++index)
        {
            tmp_node->controls.push_back( //
                node_info.controls[index](ids.reserve(control_ids[index]))
            );
        }

        nodes.emplace(node_id, std::move(tmp_node));
    }

    void App::load_link(
        std::uint64_t link_id,
        std::uint64_t entry_pin_id,
        std::uint64_t exit_pin_id
    )
    {
        auto& start = pins[entry_pin_id];
        auto& end = pins[exit_pin_id];
        auto link = std::make_unique<Link>(ids.reserve(link_id), Link::Info{start.pin, end.pin});
        start.links.emplace(link.get());
        end.links.emplace(link.get());

        if (start.pin->type == 0)
        {
            start.pin->icon = end.pin->icon;
            start.node->pins_i[0].icon = end.pin->icon;
        }
        if (end.pin->type == 0)
        {
            end.pin->icon = start.pin->icon;
            end.node->pins_o[0].icon = start.pin->icon;
        }
        links.emplace(link_id, std::move(link));
    }

    void App::prepare_create(std::uint64_t type_index)
    {
        if (!tmp)
        {
            auto& node_info = node_infos[type_index];
            tmp = std::make_unique<Node>(
                ids.reserve(),
                type_index,
                type_index == 0 ? "" : node_info.label.c_str(),
                type_index == 0
            );

            tmp->pins_i.reserve(node_info.inputs.size());
            for (const auto& pin_type : node_info.inputs)
            {
                auto& pin_info = pin_infos[pin_type];
                auto& pin = tmp->pins_i.emplace_back(
                    ids.reserve(),
                    ax::NodeEditor::PinKind::Input,
                    pin_type,
                    pin_info.label,
                    &pin_info.icon,
                    type_index == 0
                );
                pins.emplace(pin.id.value, PinMapping{tmp.get(), &pin, {}});
            }

            tmp->pins_o.reserve(node_info.outputs.size());
            for (const auto& pin_type : node_info.outputs)
            {
                auto& pin_info = pin_infos[pin_type];
                auto& pin = tmp->pins_o.emplace_back(
                    ids.reserve(),
                    ax::NodeEditor::PinKind::Output,
                    pin_type,
                    pin_info.label,
                    &pin_info.icon,
                    type_index == 0
                );
                pins.emplace(pin.id.value, PinMapping{tmp.get(), &pin, {}});
            }

            for (const auto& control : node_info.controls)
            {
                tmp->controls.push_back(control(ids.reserve()));
            }
        }
    }

    void App::confirm_create(std::uint64_t type)
    {
        if (tmp && tmp->type == type)
        {
            before_render.emplace_back(
                [this]()
                {
                    const auto id = tmp->id.value;
                    nodes.emplace(id, std::move(tmp));
                    changed = true;
                }
            );
        }
    }
}
