#pragma once

#include "IEdge.hpp"
#include "INode.hpp"

#include <memory>
#include <vector>

namespace nil::gate
{
    /**
     * @brief Edge type returned by Core::createNode.
     *  Prevents mutation.
     *
     * @tparam T
     */
    template <typename T>
    class UEdge: public IEdge
    {
    public:
        const T& value() const
        {
            return *data;
        }

        bool has_value() const override
        {
            return data.has_value();
        }

    protected:
        std::optional<T> data;
    };

    /**
     * @brief Edge type returned by Core::createEdge.
     *  Allows mutation.
     *
     * @tparam T
     */
    template <typename T>
    class Edge final: public UEdge<T>
    {
        friend class Core;

    public:
        void set_value(T data)
        {
            if (this->data != data)
            {
                this->data = std::move(data);
                for (auto* out : outs)
                {
                    out->pend();
                }
            }
        }

        void unset_value()
        {
            if (!this->data.has_value())
            {
                this->data = {};
                for (auto* out : outs)
                {
                    out->pend();
                }
            }
        }

    private:
        Edge(INode* node)
            : ins(node)
        {
        }

        static std::unique_ptr<IEdge> create(INode* node = nullptr)
        {
            return std::unique_ptr<IEdge>(new Edge<T>(node));
        }

        void attach_output(INode* node)
        {
            outs.push_back(node);
        }

    private:
        INode* ins;
        std::vector<INode*> outs;
    };
}
