#include "sort_nodes.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <utility>

namespace ext
{
    std::vector<NodeData> sort_by_score(const std::vector<NodeData>& nodes)
    {
        struct Edge final
        {
            const NodeData* input = nullptr;
            std::vector<const NodeData*> outputs;
        };

        struct Scorer final
        {
            static std::uint32_t recurse(
                const std::unordered_map<std::uint64_t, Edge>& edges,
                const NodeData* current_node,
                std::unordered_map<const NodeData*, std::uint32_t>& cache
            )
            {
                if (current_node == nullptr)
                {
                    return 0;
                }
                const auto it = cache.find(current_node);
                if (it != cache.end())
                {
                    return it->second;
                }
                auto score = 0u;
                for (const auto& i : current_node->inputs)
                {
                    score = std::max(recurse(edges, edges.at(i).input, cache) + 1u, score);
                }
                cache.emplace(current_node, score);
                return score;
            }
        };

        const auto edges = [&nodes]()
        {
            std::unordered_map<std::uint64_t, Edge> retval;
            for (const auto& node : nodes)
            {
                for (const auto& i : node.inputs)
                {
                    retval[i].outputs.push_back(&node);
                }
                for (const auto& o : node.outputs)
                {
                    retval[o].input = &node;
                }
            }
            return retval;
        }();

        const auto scores = [&nodes, &edges]()
        {
            std::multimap<std::uint32_t, const NodeData*> retval;
            std::unordered_map<const NodeData*, std::uint32_t> cache;
            for (const auto& node : nodes)
            {
                retval.emplace(Scorer::recurse(edges, &node, cache), &node);
            }
            return retval;
        }();

        return [&scores]()
        {
            std::vector<NodeData> retval;
            retval.reserve(scores.size());
            for (const auto& [score, node] : scores)
            {
                retval.push_back(*node);
            }
            return retval;
        }();
    }
}
