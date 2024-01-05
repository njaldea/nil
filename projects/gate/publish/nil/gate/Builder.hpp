#pragma once

#include "Core.hpp"

#include <nil/utils/traits/type.hpp>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <utility>
#include <vector>

namespace nil::gate
{
    namespace builder
    {
        struct Node
        {
            // node's index when registered to builder
            std::uint64_t type;
            std::vector<std::uint64_t> inputs;
            std::vector<std::uint64_t> outputs;
            std::vector<std::uint64_t> controls;
        };

        std::vector<Node> sort_by_score(const std::vector<Node>& nodes)
        {
            struct Edge
            {
                const Node* input;
                std::vector<const Node*> outputs;
            };

            struct Scorer
            {
                static std::uint32_t recurse(
                    const std::unordered_map<std::uint64_t, Edge>& edges,
                    const Node* current_node
                )
                {
                    auto score = 0u;
                    for (const auto& i : current_node->inputs)
                    {
                        score = std::max(recurse(edges, edges.at(i).input) + 1u, score);
                    }
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
                std::multimap<std::uint32_t, const Node*> retval;
                for (const auto& node : nodes)
                {
                    retval.emplace(Scorer::recurse(edges, &node), &node);
                }
                return retval;
            }();

            return [&scores]()
            {
                std::vector<Node> retval;
                retval.reserve(scores.size());
                for (const auto& [score, node] : scores)
                {
                    retval.push_back(*node);
                }
                return retval;
            }();
        }
    }
}
