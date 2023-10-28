#pragma once

#include "detail/Edge.hpp"
#include "detail/Node.hpp"

#include <stdexcept>

namespace nil::gate
{
    /**
     * @brief Owns the whole graph (Nodes/Edges)
     *  -  Nodes represent a runnable object
     *  -  Edges owns the data
     *  -  Edge type qualifications:
     *      - not pointer
     *      - not reference
     *      - if std::unique_ptr/std::shared_ptr, T should be const
     *  -  Node type qualifications:
     *      - not pointer
     *      - if reference, should be const
     *      - copy-able
     *      - if std::unique_ptr/std::shared_ptr, T should be const
     */
    class Core final
    {
    public:
        // Invalid types for input/output detected
        template <typename T, typename... Args>
        std::enable_if_t<
            !detail::traits<T>::is_valid,
            typename detail::traits<T>::o::readonly_edges> //
            node(typename detail::traits<T>::i::readonly_edges edges, Args&&... args) = delete;

        /**
         * @brief create a node
         *
         * @tparam T                                node type
         * @tparam Args
         * @param edges                             ReadOnlyEdge/MutableEdge pointers
         * @param args                              additional constructor arguments
         * @return `std::tuple<ReadOnlyEdge*, ...>` output edges (still owned by core)
         */
        template <typename T, typename... Args>
        std::enable_if_t<
            detail::traits<T>::is_valid,
            typename detail::traits<T>::o::readonly_edges> //
            node(typename detail::traits<T>::i::readonly_edges edges, Args&&... args)
        {
            return create<T>(
                typename detail::traits<T>::o::type(),
                typename detail::traits<T>::o::make_sequence(),
                typename detail::traits<T>::i::make_sequence(),
                edges,
                std::forward<Args>(args)...
            );
        }

        // Invalid type passed
        template <typename... T>
        std::enable_if_t<
            !(true && (... && detail::edge_validate<T>::value)),
            std::tuple<MutableEdge<T>*...>>
            edges() = delete;

        /**
         * @tparam T                                edge type
         * @return std::tuple<MutableEdge<T>*...>   edge instances (still owned by core)
         */
        template <typename... T>
        std::enable_if_t<
            (true && (... && detail::edge_validate<T>::value)),
            std::tuple<MutableEdge<T>*...>>
            edges()
        {
            return std::tuple(this->edge<T>()...);
        }

        // Invalid type passed
        template <typename T>
        std::enable_if_t<!detail::edge_validate<T>::value, MutableEdge<T>*> edge() = delete;

        /**
         * @brief create an edge
         *
         * @tparam T                    edge type
         * @return `MutableEdge<T>*`    edge instance (still owned by core)
         */
        template <typename T>
        std::enable_if_t<detail::edge_validate<T>::value, MutableEdge<T>*> edge()
        {
            auto e_ptr = std::make_unique<detail::Edge<T>>(nullptr);
            auto* e = e_ptr.get();
            owned_edges.emplace_back(std::move(e_ptr));
            required_edges.push_back(e);
            return e;
        }

        /**
         * @brief run all pending nodes
         */
        void run()
        {
            for (const auto& node : this->owned_nodes)
            {
                node->exec();
            }
        }

        void validate()
        {
            for (const auto& edge : required_edges)
            {
                if (!edge->has_value())
                {
                    throw std::runtime_error("value for a required edge is missing");
                }
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
            typename detail::traits<T>::i::readonly_edges edges,
            Args&&... args
        )
        {
            auto node_ptr = std::make_unique<detail::Node<T>>( //
                edges,
                indices,
                std::forward<Args>(args)...
            );
            auto* n = node_ptr.get();
            owned_nodes.emplace_back(std::move(node_ptr));
            // attach node to input edges' output
            (..., attach_output(down_cast(std::get<i_indices>(edges)), *n));
            // create output edges and attach it's input to node output
            return std::make_tuple(this->edge<T, Outputs, o_indices>(*n)...);
        }

        template <typename T, typename U>
        void attach_output(detail::Edge<U>* target, detail::Node<T>& node)
        {
            if (target == nullptr)
            {
                throw std::runtime_error("null edge detected!");
            }
            target->attach_output(&node);
        }

        template <typename U>
        static detail::Edge<U>* down_cast(ReadOnlyEdge<U>* edge)
        {
            return static_cast<detail::Edge<U>*>(edge);
        }

        template <typename T, typename U, std::size_t index>
        ReadOnlyEdge<U>* edge(detail::Node<T>& node)
        {
            auto e_ptr = std::make_unique<detail::Edge<U>>(&node);
            auto* e = e_ptr.get();
            owned_edges.emplace_back(std::move(e_ptr));
            node.template attach_output<index>(e);
            return e;
        }

        std::vector<std::unique_ptr<detail::INode>> owned_nodes;
        std::vector<std::unique_ptr<IEdge>> owned_edges;
        std::vector<IEdge*> required_edges;
    };
}
