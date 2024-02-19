#pragma once

#include "Internal.hpp"

namespace nil::gate::detail
{
    template <typename T, typename = void>
    struct compare_helpers: std::false_type
    {
        static bool is_not_equal(const std::optional<T>& /* l */, const T& /* r */)
        {
            return true;
        }
    };

    template <typename T>
    struct compare_helpers<T, std::void_t<decltype(std::declval<T>() == std::declval<T>())>>
        : std::true_type
    {
        static bool is_not_equal(const std::optional<T>& l, const T& r)
        {
            return !l.has_value() || !(l.value() == r);
        }
    };

    /**
     * @brief Edge type returned by Core::edge.
     *  For internal use.
     *
     * @tparam T
     */
    template <typename T>
    class DataEdge final: public InternalEdge<T>
    {
    public:
        DataEdge()
            : InternalEdge<T>()
            , data(std::nullopt)
            , tasks(nullptr)
        {
        }

        template <typename... Args>
        DataEdge(Tasks* init_tasks, Args&&... args)
            : InternalEdge<T>()
            , data(std::forward<Args>(args)...)
            , tasks(init_tasks)
        {
        }

        ~DataEdge() noexcept = default;

        DataEdge(DataEdge&&) = delete;
        DataEdge(const DataEdge&) = delete;
        DataEdge& operator=(DataEdge&&) = delete;
        DataEdge& operator=(const DataEdge&) = delete;

        const T& value() const override
        {
            return *data;
        }

        void set_value(T new_data) override
        {
            struct Callable: ICallable
            {
                Callable(T init_data, DataEdge<T>* init_parent)
                    : data(std::move(init_data))
                    , parent(init_parent)
                {
                }

                void call() override
                {
                    if (compare_helpers<T>::is_not_equal(parent->data, data))
                    {
                        parent->exec(std::move(data));
                        parent->pend();
                    }
                }

                T data;
                DataEdge<T>* parent;
            };

            this->tasks->push(std::make_unique<Callable>(std::move(new_data), this));
        }

        bool exec(T new_data) override
        {
            if (compare_helpers<T>::is_not_equal(data, new_data))
            {
                data = std::move(new_data);
                return true;
            }
            return false;
        }

        void pend() override
        {
            for (auto* out : this->outs)
            {
                out->pend();
            }
        }

        void attach_input(INode* node) override
        {
            ins = node;
        }

        void attach_output(INode* node) override
        {
            outs.push_back(node);
        }

        void attach_tasks(Tasks* new_tasks)
        {
            tasks = new_tasks;
        }

    private:
        std::optional<T> data;
        Tasks* tasks;
        INode* ins = nullptr;
        std::vector<detail::INode*> outs;
    };
}
