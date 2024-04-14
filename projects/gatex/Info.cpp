#include <nil/gatex/Info.hpp>

#include <stdexcept>

namespace
{
    std::uint64_t populate_score(
        std::unordered_map<std::uint64_t, std::uint64_t>& cache,
        nil::gatex::GraphInfo& info,
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
                const auto opposite_pin = info.opposite_output_pin(pin_id);
                const auto& [prev_id, prev_node] = *info.pin_to_node.at(opposite_pin);
                if (prev_node.type != 0)
                {
                    score = std::max(score, populate_score(cache, info, prev_id) + 1);
                }
            }
            cache.emplace(node_id, score);
            info.scores.emplace(score, info.nodes.find(node_id));
            return score;
        }
        return it->second;
    }
}

namespace nil::gatex
{
    void GraphInfo::add_node(NodeInfo info)
    {
        const auto id = info.id;

        auto node_it = nodes.emplace(id, std::move(info));
        if (!node_it.second)
        {
            throw std::runtime_error("duplicate node[" + std::to_string(id) + "]");
        }
        const auto process = [&](const auto& pins)
        {
            for (const auto& p : pins)
            {
                if (!pin_to_node.emplace(p, node_it.first).second)
                {
                    throw std::runtime_error( //
                        "duplicate edge["     //
                        + std::to_string(p)   //
                        + "] from node["      //
                        + std::to_string(id)  //
                        + "]"
                    );
                }
            }
        };
        const auto& n = node_it.first->second;
        process(n.inputs);
        process(n.outputs);
        process(n.controls);
    }

    void GraphInfo::add_link(LinkInfo info)
    {
        const auto id = info.id;

        auto link_it = links.emplace(id, info);
        if (!link_it.second)
        {
            throw std::runtime_error("duplicate link[" + std::to_string(id) + "]");
        }

        if (!input_pin_to_link.emplace(info.output, link_it.first).second)
        {
            throw std::runtime_error("duplicate link output[" + std::to_string(info.output) + "]");
        }
    }

    void GraphInfo::score()
    {
        std::unordered_map<std::uint64_t, std::uint64_t> cache;
        for (const auto& [id, key] : nodes)
        {
            populate_score(cache, *this, id);
        }
    }

    std::uint64_t GraphInfo::opposite_output_pin(std::uint64_t input_pin) const
    {
        return input_pin_to_link.at(input_pin)->second.input;
    }

    std::uint64_t GraphInfo::link_type(std::uint64_t input_pin) const
    {
        return input_pin_to_link.at(input_pin)->second.type;
    }
}
