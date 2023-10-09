#pragma once

#include "../MEdge.hpp"

#include <memory>
#include <vector>

namespace nil::gate
{
    class Core;
}

namespace nil::gate::detail
{
    /**
     * @brief Edge type returned by Core::edge.
     *  For internal use.
     *
     * @tparam T
     */
    template <typename T>
    class Edge final: public MEdge<T>
    {
        friend Core;

    public:
        /**
         * @brief set value. propagate node execution
         *
         * @param data
         */
        void exec(T data)
        {
            this->data = std::move(data);
            for (auto* out : this->outs)
            {
                if (out->state() == detail::INode::State::Pending && out->is_runnable())
                {
                    out->exec();
                }
            }
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

    private:
        Edge(INode* node)
            : ins(node)
        {
        }

        static std::unique_ptr<IEdge> create(INode* node)
        {
            return std::unique_ptr<IEdge>(new Edge<T>(node));
        }

        void attach_output(INode* node)
        {
            this->outs.push_back(node);
        }

    private:
        INode* ins;
    };
}
