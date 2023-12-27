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
     *      -  not pointer
     *      -  not reference
     *      -  if std::unique_ptr/std::shared_ptr, T should be const
     *  -  Node type qualifications:
     *      -  not pointer
     *      -  if reference, should be const
     *      -  copy-able
     *      -  if std::unique_ptr/std::shared_ptr, T should be const
     */
    class Core final
    {
    public:
        Core() = default;
        ~Core() noexcept = default;

        Core(Core&&) = default;
        Core(const Core&) = delete;
        Core& operator=(Core&&) = default;
        Core& operator=(const Core&) = delete;

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
            node(typename detail::traits<T>::i::readonly_edges input_edges, Args&&... args)
        {
            auto node = std::make_unique<detail::Node<T>>(input_edges, std::forward<Args>(args)...);
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(node)).get())
                ->output_edges();
        }

        // Invalid type passed
        template <typename... T>
        std::enable_if_t<
            !(true && (... && detail::edge_validate<T>::value)),
            std::tuple<MutableEdge<T>*...>>
            edges() = delete;

        /**
         * @tparam T                                - edge type
         * @return `std::tuple<MutableEdge<T>*...>` - edge instances (still owned by core)
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
         * @tparam T                    - edge type
         * @return `MutableEdge<T>*`    - edge instance (still owned by core)
         */
        template <typename T>
        std::enable_if_t<detail::edge_validate<T>::value, MutableEdge<T>*> edge()
        {
            return static_cast<MutableEdge<T>*>(
                required_edges.emplace_back(std::make_unique<MutableEdge<T>>()).get()
            );
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
                    throw std::runtime_error("value for a required edge is missing");
                }
            }
            for (const auto& node : owned_nodes)
            {
                node->exec();
            }
        }

    private:
        std::vector<std::unique_ptr<detail::INode>> owned_nodes;
        std::vector<std::unique_ptr<IEdge>> required_edges;
    };
}
