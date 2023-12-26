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
    class CoreBuilder
    {
    private:
        using EdgeIDs = std::vector<std::uint64_t>;
        using NodeFactory
            = std::function<void(CoreBuilder&, nil::gate::Core&, const EdgeIDs&, const EdgeIDs&)>;

        template <typename T, typename... Args, std::size_t... indices>
        auto create(
            nil::gate::Core& core,
            const EdgeIDs& inputs,
            std::index_sequence<indices...>,
            Args... args //
        )
        {
            using edges_t = typename nil::gate::detail::traits<T>::i::readonly_edges;
            return core.node<T>(edges_t(edges.at(inputs[indices])...), std::move(args)...);
        }

        template <typename T, std::size_t... indices>
        void store(
            const EdgeIDs& outputs,
            const typename nil::gate::detail::traits<T>::o::readonly_edges& result,
            std::index_sequence<indices...> //
        )
        {
            (edges.emplace(outputs[indices], std::get<indices>(result)), ...);
        }

        struct Node
        {
            std::uint64_t type;
            std::vector<std::uint64_t> inputs;
            std::vector<std::uint64_t> outputs;
        };

        struct Edge
        {
            const Node* input;
            std::vector<const Node*> outputs;
        };

        static std::uint32_t recurse_score(
            const std::unordered_map<std::uint64_t, Edge>& edges,
            const Node* current_node
        ) //
        {
            auto score = 0u;
            for (const auto& i : current_node->inputs)
            {
                score = std::max(recurse_score(edges, edges.at(i).input) + 1u, score);
            }
            return score;
        }

        static std::vector<Node> sort_by_score(const std::vector<Node>& nodes)
        {
            std::vector<Node> retval;

            std::unordered_map<std::uint64_t, Edge> edges;
            for (const auto& node : nodes)
            {
                for (const auto& i : node.inputs)
                {
                    edges[i].outputs.push_back(&node);
                }
                for (const auto& o : node.outputs)
                {
                    edges[o].input = &node;
                }
            }

            struct Edge
            {
                const Node* input;
                std::vector<const Node*> outputs;
            };

            std::multimap<std::uint32_t, const Node*> scores;
            for (const auto& node : nodes)
            {
                scores.emplace(recurse_score(edges, &node), &node);
            }
            for (const auto& [score, node] : scores)
            {
                retval.push_back(*node);
            }
            return retval;
        }

    public:
        CoreBuilder() = default;
        ~CoreBuilder() = default;

        CoreBuilder(CoreBuilder&&) = default;
        CoreBuilder(const CoreBuilder&) = default;
        CoreBuilder& operator=(CoreBuilder&&) = default;
        CoreBuilder& operator=(const CoreBuilder&) = default;

        template <typename T, typename... Args>
        std::enable_if_t<nil::gate::detail::traits<T>::is_valid, CoreBuilder&> add_node_type(
            Args&&... args
        )
        {
            node_factories.push_back(
                [... args = args](         //
                    CoreBuilder& self,     //
                    nil::gate::Core& core, //
                    const EdgeIDs& inputs, //
                    const EdgeIDs& outputs //
                )
                {
                    using traits = nil::gate::detail::traits<T>;
                    const auto i_seq = typename traits::i::make_index_sequence();
                    const auto o_seq = typename traits::o::make_index_sequence();
                    const auto result = self.create<T>(core, inputs, i_seq, args...);
                    if constexpr (traits::o::size > 0)
                    {
                        self.store<T>(outputs, result, o_seq);
                    }
                }
            );
            return *this;
        }

        CoreBuilder& add_node(
            std::uint64_t type,
            std::vector<std::uint64_t> inputs,
            std::vector<std::uint64_t> outputs
        )
        {
            graph_nodes.push_back({type, std::move(inputs), std::move(outputs)});
            return *this;
        }

        Core build()
        {
            Core core;
            for (const auto& node : sort_by_score(graph_nodes))
            {
                node_factories[node.type](*this, core, node.inputs, node.outputs);
            }
            edges.clear();
            return core;
        }

    private:
        class RelaxedEdge
        {
        public:
            template <typename T>
            RelaxedEdge(nil::gate::ReadOnlyEdge<T>* init_edge)
                : edge(init_edge)
                , type_id(&utils::traits::type<T>::value)
            {
            }

            template <typename T>
            operator nil::gate::ReadOnlyEdge<T>*() const
            {
                if (&utils::traits::type<T>::value != type_id)
                {
                    // this is needed in case user side has misalignment with the node type index
                    // message registration (sent to gui) and graph registration (gate)
                    throw std::runtime_error("incompatible edges detected.");
                }
                return static_cast<nil::gate::ReadOnlyEdge<T>*>(edge);
            }

        private:
            nil::gate::IEdge* edge;
            const void* type_id;
        };

        std::vector<Node> graph_nodes;
        std::vector<NodeFactory> node_factories;
        std::unordered_map<std::uint64_t, RelaxedEdge> edges;
    };
}
