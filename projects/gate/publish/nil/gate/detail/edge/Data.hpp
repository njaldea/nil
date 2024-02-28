#pragma once

#include "Internal.hpp"

namespace nil::gate::detail
{
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

        bool has_value() const override
        {
            return data.has_value();
        }

        void set_value(T new_data) override
        {
            if (tasks)
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
                        if (!parent->data.has_value() || !std::equal_to()(*parent->data, data))
                        {
                            if (parent->exec(std::move(data)))
                            {
                                parent->pend();
                            }
                        }
                    }

                    T data;
                    DataEdge<T>* parent;
                };

                tasks->push(std::make_unique<Callable>(std::move(new_data), this));
            }
        }

        bool exec(T new_data) override
        {
            if (!data.has_value() || !std::equal_to<T>()(data.value(), new_data))
            {
                data.emplace(std::move(new_data));
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
        Tasks* tasks = nullptr;
        INode* ins = nullptr;
        std::vector<detail::INode*> outs;
    };
}
