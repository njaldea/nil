#pragma once

namespace nil::gate::detail
{
    class INode
    {
    public:
        enum class State
        {
            Pending,
            Done
        };

        virtual ~INode() noexcept = default;

        virtual void exec() = 0;
        virtual void pend() = 0;
    };
}
