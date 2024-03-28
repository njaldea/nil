#pragma once

#include "../INode.hpp"
#include "../edges/Mutable.hpp"
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
            , state(EState::Pending)
            , data(std::nullopt)
            , tasks(nullptr)
        {
        }

        // this is called when instantiated from Core
        explicit Data(nil::gate::detail::Tasks* init_tasks, T init_data)
            : nil::gate::edges::Mutable<T>()
            , state(EState::Done)
            , data(std::make_optional<T>(std::move(init_data)))
            , tasks(init_tasks)
        {
        }

        ~Data() noexcept override = default;

        Data(Data&&) noexcept = delete;
        Data& operator=(Data&&) noexcept = delete;

        Data(const Data&) = delete;
        Data& operator=(const Data&) = delete;

        const T& value() const override
        {
            return *data;
        }

        void set_value(T new_data) override
        {
            tasks->push(make_callable(
                [this, new_data = std::move(new_data)]() mutable
                {
                    if (exec(std::move(new_data)))
                    {
                        pend();
                        done();
                    }
                }
            ));
        }

        bool exec(T new_data)
        {
            if (!data.has_value() || !(data.value() == new_data))
            {
                data.emplace(std::move(new_data));
                return true;
            }
            return false;
        }

        void pend()
        {
            if (state != EState::Pending)
            {
                state = EState::Pending;
                for (auto* out : this->outs)
                {
                    out->pend();
                }
            }
        }

        void done()
        {
            if (state != EState::Done)
            {
                state = EState::Done;
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

        bool is_pending() const
        {
            return state == EState::Pending;
        }

        bool validate(nil::gate::detail::Tasks* reference_tasks) const
        {
            return tasks == reference_tasks;
        }

    private:
        enum class EState
        {
            Done = 0b0001,
            Pending = 0b0010
        };

        EState state;
        std::optional<T> data;
        nil::gate::detail::Tasks* tasks;
        std::vector<INode*> outs;
    };

    template <typename U>
    Data<U>* as_data(nil::gate::edges::Mutable<U>* edge)
    {
        return static_cast<Data<U>*>(edge);
    }
}
