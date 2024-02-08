#pragma once

#include "../MEdge.hpp"

namespace nil::gate::detail
{
    /**
     * @brief Edge type returned by Core::edge.
     *  For internal use.
     *
     * @tparam T
     */
    template <typename T>
    class Edge final: public MutableEdge<T>
    {
    public:
        Edge(INode* node)
            : ins(node)
        {
        }

        ~Edge() noexcept = default;

        Edge(Edge&&) = delete;
        Edge(const Edge&) = delete;
        Edge& operator=(Edge&&) = delete;
        Edge& operator=(const Edge&) = delete;

        /**
         * @brief set value without propagate node execution.
         *      to be used only by the node.
         *
         * @param data
         */
        void exec(T new_data)
        {
            this->data = std::move(new_data);
        }

        /**
         * @brief clear value. marks all sub nodes as pending for execution
         */
        using MutableEdge<T>::pend;

        void attach_output(INode* node)
        {
            this->outs.push_back(node);
        }

    private:
        INode* ins;
    };
}
