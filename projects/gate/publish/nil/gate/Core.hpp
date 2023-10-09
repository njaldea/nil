#pragma once

#include "detail/Edge.hpp"
#include "detail/Node.hpp"

namespace nil::gate
{
    class Core final
    {
    public:
        /**
         * @brief create a node
         *
         * @tparam T                            node type
         * @tparam Args
         * @param edges                         REdge/MEdge pointers
         * @param args                          additional constructor arguments
         * @return `std::tuple<REdge*, ...>`    output edges (still owned by core)
         */
        template <typename T, typename... Args>
        std::conditional_t< //
            detail::traits<T>::o::size == 0,
            void,
            typename detail::traits<T>::o::redged>
            node(typename detail::traits<T>::i::redged edges, Args&&... args)
        {
            return create<T>(
                typename detail::traits<T>::o::type(),
                typename detail::traits<T>::o::make_sequence(),
                typename detail::traits<T>::i::make_sequence(),
                edges,
                std::forward<Args>(args)...
            );
        }

        /**
         * @brief create an edge
         *
         * @tparam T            edge type
         * @return `MEdge<T>*`  edge instance (still owned by core)
         */
        template <typename T>
        MEdge<T>* edge()
        {
            edges.emplace_back(std::make_unique<detail::Edge<T>>(nullptr));
            required_edges.push_back(edges.back().get());
            return static_cast<MEdge<T>*>(edges.back().get());
        }

        /**
         * @brief run all pending nodes
         */
        void run()
        {
            for (const auto& edge : required_edges)
            {
                if (!edge->has_value())
                {
                    throw std::runtime_error("value for a required edge not missing");
                }
            }

            for (const auto& node : this->nodes)
            {
                node->exec();
            }
        }

    private:
        template <
            typename T,
            typename... Outputs,
            std::size_t... o_indices,
            std::size_t... i_indices,
            typename... Args>
        auto create(
            detail::types<Outputs...>,
            std::index_sequence<o_indices...>,
            std::index_sequence<i_indices...> indices,
            typename detail::traits<T>::i::redged edges,
            Args&&... args
        )
        {
            nodes.emplace_back(
                std::make_unique<detail::Node<T>>(edges, indices, std::forward<Args>(args)...)
            );
            auto node = static_cast<detail::Node<T>*>(nodes.back().get());
            // attach node to input edges' output
            (down_cast(std::get<i_indices>(edges))->attach_output(node), ...);
            // create output edges and attach it's input to node output
            if constexpr (sizeof...(Outputs) > 0)
            {
                return std::make_tuple(this->output_edge<T, Outputs, o_indices>(*node)...);
            }
        }

        template <typename U>
        static detail::Edge<U>* down_cast(REdge<U>* edge)
        {
            return static_cast<detail::Edge<U>*>(edge);
        }

        template <typename T, typename U, std::size_t index>
        REdge<U>* output_edge(detail::Node<T>& node)
        {
            edges.emplace_back(std::make_unique<detail::Edge<U>>(&node));
            auto edge = static_cast<detail::Edge<U>*>(edges.back().get());
            node.template attach_output<index>(edge);
            return edge;
        }

        std::vector<std::unique_ptr<detail::INode>> nodes;
        std::vector<std::unique_ptr<IEdge>> edges;
        std::vector<IEdge*> required_edges;
    };
}
