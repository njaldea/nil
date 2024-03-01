#pragma once

#include "../MEdge.hpp"
#include "INode.hpp"
#include "Tasks.hpp"

#include <optional>
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
    class DataEdge final: public MutableEdge<T>
    {
    public:
        // this is called when instantiated from Node
        DataEdge()
            : MutableEdge<T>()
            , data(std::nullopt)
            , tasks(nullptr)
            , depth_value(0u)
        {
        }

        // this is called when instantiated from Core
        template <typename... Args>
        DataEdge(Tasks* init_tasks, Args&&... args)
            : MutableEdge<T>()
            , data(std::forward<Args>(args)...)
            , tasks(init_tasks)
            , depth_value(0u)
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
#ifdef NIL_GATE_CHECKS
            if (!tasks)
            {
                return;
            }
#endif
            tasks->push(make_callable(
                [this, new_data = std::move(new_data)]() mutable
                {
                    if (!data.has_value() || !std::equal_to()(*data, new_data))
                    {
                        if (exec(std::move(new_data)))
                        {
                            pend();
                        }
                    }
                }
            ));
        }

        bool exec(T new_data)
        {
            if (!data.has_value() || !std::equal_to<T>()(data.value(), new_data))
            {
                data.emplace(std::move(new_data));
                return true;
            }
            return false;
        }

        void pend()
        {
            for (auto* out : this->outs)
            {
                out->pend();
            }
        }

        void attach_output(INode* node)
        {
            outs.push_back(node);
        }

        void attach_tasks(Tasks* new_tasks)
        {
            tasks = new_tasks;
        }

        void set_depth(std::uint64_t value)
        {
            depth_value = value;
        }

        std::uint64_t depth() const
        {
            return depth_value;
        }

    private:
        std::optional<T> data;
        Tasks* tasks;
        std::uint64_t depth_value;
        std::vector<detail::INode*> outs;
    };

    template <typename U>
    MutableEdge<U>* as_mutable(DataEdge<U>* edge)
    {
        return edge;
    }
}
