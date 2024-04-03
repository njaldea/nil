#include "Info.hpp"

#include <iostream>

void dump(const nil::nedit::proto::State& state)
{
    const auto& graph = state.graph();

    std::cout << "nodes\n";
    for (const auto& node : graph.nodes())
    {
        std::cout                 //
            << "  node: "         //
            << node.id() << ':'   //
            << node.type() << ':' //
            << node.alias();

        std::cout << "\n    inputs   : ";
        for (const auto& p : node.inputs())
        {
            std::cout << p << ' ';
        }
        std::cout << "\n    outputs  : ";
        for (const auto& p : node.outputs())
        {
            std::cout << p << ' ';
        }
        std::cout << "\n    controls : ";
        for (const auto& p : node.controls())
        {
            std::cout << p << ' ';
        }
        std::cout << '\n';
    }

    std::cout << "links\n";
    for (const auto& link : graph.links())
    {
        std::cout                   //
            << "  link: "           //
            << link.id() << ':'     //
            << link.input() << ':'  //
            << link.output() << ':' //
            << link.type() << '\n';
    }

    std::cout << std::flush;
}

std::uint64_t populate_score(
    std::unordered_map<std::uint64_t, std::uint64_t>& cache,
    GraphInfo& info,
    std::uint64_t node_id
)
{
    auto it = cache.find(node_id);
    if (it == cache.end())
    {
        std::uint64_t score = 0u;
        const auto& node = info.nodes.at(node_id);
        // type 0 is feedback loop
        // feedback should reset the score
        for (const auto& pin_id : node.inputs)
        {
            const auto& link_input = info.pin_to_link.at(pin_id).it->second.input;
            const auto& prev_node = info.pin_to_node.at(link_input).it;
            if (prev_node->second.type != 0)
            {
                score = std::max(score, populate_score(cache, info, prev_node->first) + 1);
            }
        }
        cache.emplace(node_id, score);
        info.scores.emplace(score, info.nodes.find(node_id));
        return score;
    }
    return it->second;
}

GraphInfo populate(const nil::nedit::proto::State& state)
{
    GraphInfo info;

    const auto& graph = state.graph();
    for (const auto& link : graph.links())
    {
        auto link_it
            = info.links.emplace(link.id(), LinkInfo{link.input(), link.output(), link.type()});
        info.pin_to_link.emplace(link.input(), PinInfo<LinkInfo>{.it = link_it.first});
        info.pin_to_link.emplace(link.output(), PinInfo<LinkInfo>{.it = link_it.first});
    }

    for (const auto& node : graph.nodes())
    {
        auto node_it = info.nodes.emplace(
            node.id(),
            NodeInfo{
                .type = node.type(),
                .inputs = {node.inputs().begin(), node.inputs().end()},
                .outputs = {node.outputs().begin(), node.outputs().end()},
                .controls = {node.controls().begin(), node.controls().end()}
            }
        );
        for (const auto& p : node.inputs())
        {
            info.pin_to_node.emplace(p, PinInfo<NodeInfo>{.it = node_it.first});
        }
        for (const auto& p : node.outputs())
        {
            info.pin_to_node.emplace(p, PinInfo<NodeInfo>{.it = node_it.first});
        }
        // probably dead code
        for (const auto& p : node.controls())
        {
            info.pin_to_node.emplace(p, PinInfo<NodeInfo>{.it = node_it.first});
        }
    }

    std::unordered_map<std::uint64_t, std::uint64_t> cache;
    for (const auto& [id, key] : info.nodes)
    {
        populate_score(cache, info, id);
    }

    return info;
}
