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

        Core(Core&&) = delete;
        Core(const Core&) = delete;
        Core& operator=(Core&&) = delete;
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
            auto node = std::make_unique<detail::Node<T>>(
                &deferrer,
                input_edges,
                std::forward<Args>(args)...
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(node)).get())
                ->output_edges();
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
        template <typename T, typename... Args>
        std::enable_if_t<detail::edge_validate<T>::value, MutableEdge<T>*> edge(Args&&... args)
        {
            auto new_edge = std::make_unique<detail::Edge<T>>(std::forward<Args>(args)...);
            auto* e = required_edges.emplace_back(std::move(new_edge)).get();
            static_cast<detail::Edge<T>*>(e)->attach_deferrer(&deferrer);
            return static_cast<MutableEdge<T>*>(e);
        }

        /**
         * @brief create an edge
         *
         * @tparam T                    - edge type
         * @return `MutableEdge<T>*`    - edge instance (still owned by core)
         */
        template <typename T>
        std::enable_if_t<detail::edge_validate<T>::value, MutableEdge<T>*> edge(T&& value)
        {
            auto new_edge = std::make_unique<detail::Edge<T>>(std::forward<T>(value));
            auto* e = required_edges.emplace_back(std::move(new_edge)).get();
            static_cast<detail::Edge<T>*>(e)->attach_deferrer(&deferrer);
            return static_cast<MutableEdge<T>*>(e);
        }

        /**
         * @brief run all pending nodes
         */
        void run()
        {
            bool first = true;
            while (true)
            {
                const auto tasks = [this]()
                {
                    std::lock_guard _(deferrer.mutex);
                    return std::exchange(deferrer.tasks, {});
                }();
                if (!first && tasks.empty())
                {
                    return;
                }
                for (const auto& d : tasks)
                {
                    d->call();
                }

                for (const auto& node : owned_nodes)
                {
                    node->exec();
                }
                first = false;
            }
        }

    private:
        detail::Deferrer deferrer;
        std::vector<std::unique_ptr<detail::INode>> owned_nodes;
        std::vector<std::unique_ptr<IEdge>> required_edges;
    };
}
