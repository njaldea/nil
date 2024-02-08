#include "sort_nodes.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <utility>

namespace ext
{
    // NodeData input ids in input are mapped to the output pin of the connected node
    std::vector<NodeData> sort_by_score(const std::vector<NodeData>& nodes)
    {
        struct Scorer final
        {
            static std::uint32_t recurse(
                const std::unordered_map<std::uint64_t, const NodeData*>& edge_to_node,
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
                    if (edge_to_node.contains(i))
                    {
                        score = std::max(
                            recurse(edge_to_node, edge_to_node.at(i), cache) + 1u,
                            score
                        );
                    }
                }
                cache.emplace(current_node, score);
                return score;
            }
        };

        const auto edge_to_node = [&nodes]()
        {
            std::unordered_map<std::uint64_t, const NodeData*> retval;
            for (const auto& node : nodes)
            {
                for (const auto& o : node.outputs)
                {
                    retval.emplace(o, &node);
                }
            }
            return retval;
        }();

        const auto scores = [&nodes, &edge_to_node]()
        {
            std::multimap<std::uint32_t, const NodeData*> retval;
            std::unordered_map<const NodeData*, std::uint32_t> cache;
            for (const auto& node : nodes)
            {
                retval.emplace(Scorer::recurse(edge_to_node, &node, cache), &node);
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
