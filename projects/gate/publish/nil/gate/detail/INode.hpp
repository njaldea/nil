#pragma once

#include "ICallable.hpp"

namespace nil::gate::detail
{
    class INode
    {
    public:
        virtual ~INode() noexcept = default;

        virtual void exec(ICallable* callable) = 0;
        virtual void pend() = 0;

    protected:
        enum class EState
        {
            Pending,
            Done
        };
    };
}
