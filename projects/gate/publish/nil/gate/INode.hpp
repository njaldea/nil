#pragma once

namespace nil::gate
{
    class INode
    {
    public:
        enum class State
        {
            Pending,
            Done
        };

        virtual ~INode() = default;
        virtual State state() const = 0;
        virtual void exec() = 0;
        virtual void pend() = 0;
        virtual bool is_runnable() const = 0;
    };
}
