#pragma once

#include "../MEdge.hpp"

#include <memory>
#include <vector>

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

        /**
         * @brief set value. propagate node execution
         *
         * @param data
         */
        void exec(T new_data)
        {
            this->data = std::move(new_data);
            // TODO: evaluate if depth traversal is needed.
            //  by using variables, the graph becomes
            //  topologically sorted by default
            // for (auto* out : this->outs)
            // {
            //     out->exec();
            // }
        }

        /**
         * @brief clear value. marks all sub nodes as pending for execution
         */
        void pend()
        {
            this->data = {};
            for (auto* out : this->outs)
            {
                out->pend();
            }
        }

        /**
         * @brief allow cancellation?
         */
        void cancel()
        {
            this->data = {};
            for (auto* out : this->outs)
            {
                out->cancel();
            }
        }

        void attach_output(INode* node)
        {
            this->outs.push_back(node);
        }

    private:
        INode* ins;
    };
}