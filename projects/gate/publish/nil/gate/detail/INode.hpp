#pragma once

#include "ICallable.hpp"

#include <cstdint>

namespace nil::gate
{
    class Core;
}

namespace nil::gate::detail
{
    class INode
    {
    public:
        INode() = default;
        virtual ~INode() noexcept = default;

        INode(INode&&) = delete;
        INode(const INode&) = delete;
        INode& operator=(INode&&) = delete;
        INode& operator=(const INode&) = delete;

        virtual void exec(Core* core) = 0;
        virtual void pend() = 0;
        virtual std::uint64_t depth() const = 0;

    protected:
        enum class EState
        {
            Pending,
            Done
        };
    };
}
