#pragma once

#include "Edge.hpp"
#include "Node.hpp"

namespace nil::gate
{
    class Core final
    {
    public:
        template <typename T, typename... Args>
        typename detail::traits<T>::uedged_o createNode(
            typename detail::traits<T>::uedged_i edges,
            Args&&... args
        )
        {
            return create<T>(
                typename detail::traits<T>::type_o(),
                std::make_index_sequence<detail::traits<T>::size_o>(),
                std::make_index_sequence<detail::traits<T>::size_i>(),
                edges,
                std::forward<Args>(args)...
            );
        }

        template <typename T>
        Edge<T>* createEdge()
        {
            edges.emplace_back(Edge<T>::create());
            required_edges.push_back(edges.back().get());
            return static_cast<Edge<T>*>(edges.back().get());
        }

        void run()
        {
            for (const auto& edge : required_edges)
            {
                if (!edge->has_value())
                {
                    throw std::runtime_error("value for a required edge is not missing");
                }
            }

            std::size_t count = 0;
            for (auto& node : nodes)
            {
                if (node->state() == INode::State::Done)
                {
                    ++count;
                }
            }

            while (count != nodes.size())
            {
                const auto cache = count;
                for (auto& node : nodes)
                {
                    if (node->state() == INode::State::Pending && node->is_runnable())
                    {
                        node->exec();
                        ++count;
                    }
                }
                if (cache == count)
                {
                    throw std::runtime_error("nothing is resolved");
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
        typename detail::traits<T>::uedged_o create(
            detail::types<Outputs...>,
            std::index_sequence<o_indices...>,
            std::index_sequence<i_indices...>,
            detail::traits<T>::uedged_i edges,
            Args&&... args
        )
        {
            nodes.emplace_back(Node<T>::create(edges, std::forward<Args>(args)...));
            Node<T>& node = *static_cast<Node<T>*>(nodes.back().get());
            // attach node to input edges' output
            ( //
                static_cast<typename detail::traits<T>::template edge_at_i<i_indices>*>(
                    std::get<i_indices>(edges)
                )
                    ->attach_output(&node),
                ...
            );
            // create output edges and attach it's input to node output
            return {this->createEdge<Outputs, T, o_indices>(node)...};
        }

        template <typename T, typename U, std::size_t index>
        UEdge<T>* createEdge(Node<U>& node)
        {
            edges.emplace_back(Edge<T>::create(&node));
            auto edge = static_cast<Edge<T>*>(edges.back().get());
            node.template attach_output<index>(edge);
            return edge;
        }

        std::vector<std::unique_ptr<INode>> nodes;
        std::vector<std::unique_ptr<IEdge>> edges;
        std::vector<IEdge*> required_edges;
    };
}
