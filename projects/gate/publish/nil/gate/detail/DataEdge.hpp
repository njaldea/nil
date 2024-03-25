#pragma once

#include "../edges/Mutable.hpp"
#include "INode.hpp"
#include "Tasks.hpp"

#include <optional>
#include <vector>

namespace nil::gate::detail::edges
{
    /**
     * @brief Edge type returned by Core::edge.
     *  For internal use.
     *
     * @tparam T
     */
    template <typename T>
    class Data final: public nil::gate::edges::Mutable<T>
    {
    public:
        // this is called when instantiated from Node
        Data()
            : nil::gate::edges::Mutable<T>()
            , data(std::nullopt)
            , tasks(nullptr)
            , depth_value(0u)
        {
        }

        // this is called when instantiated from Core
        explicit Data(nil::gate::detail::Tasks* init_tasks, T init_data)
            : nil::gate::edges::Mutable<T>()
            , data(std::make_optional<T>(std::move(init_data)))
            , tasks(init_tasks)
            , depth_value(0u)
        {
        }

        ~Data() noexcept override = default;

        Data(Data&&) = delete;
        Data(const Data&) = delete;
        Data& operator=(Data&&) = delete;
        Data& operator=(const Data&) = delete;

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
        nil::gate::detail::Tasks* tasks;
        std::uint64_t depth_value;
        std::vector<detail::INode*> outs;
    };

    template <typename U>
    Data<U>* as_data(nil::gate::edges::Mutable<U>* edge)
    {
        return static_cast<Data<U>*>(edge);
    }
}
