#pragma once

#include "../MEdge.hpp"
#include "Deferrer.hpp"

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
        template <typename... Args>
        Edge(Args&&... args)
            : MutableEdge<T>(std::forward<Args>(args)...)
            , deferrer(nullptr)
            , ins(nullptr)
        {
        }

        ~Edge() noexcept = default;

        Edge(Edge&&) = delete;
        Edge(const Edge&) = delete;
        Edge& operator=(Edge&&) = delete;
        Edge& operator=(const Edge&) = delete;

        void set_value(T new_data) override
        {
            struct Callable: ICallable
            {
                Callable(T init_data, Edge<T>* init_parent)
                    : data(std::move(init_data))
                    , parent(init_parent)
                {
                }

                void call() override
                {
                    if (parent->data != data)
                    {
                        parent->data = std::move(data);
                        parent->pend();
                    }
                }

                T data;
                Edge<T>* parent;
            };

            deferrer->push(std::make_unique<Callable>(std::move(new_data), this));
        }

        void exec(T new_data)
        {
            this->data = std::move(new_data);
        }

        void pend()
        {
            for (auto* out : this->outs)
            {
                out->pend();
            }
        }

        void attach_input(INode* node)
        {
            ins = node;
        }

        void attach_output(INode* node)
        {
            outs.push_back(node);
        }

        void attach_deferrer(Deferrer* new_deferrer)
        {
            deferrer = new_deferrer;
        }

    private:
        Deferrer* deferrer;
        INode* ins;
        std::vector<detail::INode*> outs;
    };
}
