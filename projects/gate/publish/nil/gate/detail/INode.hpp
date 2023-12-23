#pragma once

namespace nil::gate::detail
{
    class INode
    {
    public:
        enum class State
        {
            Pending,
            Cancelled, // [TODO] add handling, how?
            Done
        };

        virtual ~INode() = default;
        virtual void exec() = 0;
        virtual void pend() = 0;
        virtual void cancel() = 0;
    };
}
