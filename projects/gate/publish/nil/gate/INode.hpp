#pragma once

namespace nil::gate
{
    class Core;
}

namespace nil::gate
{
    class INode
    {
    public:
        INode() = default;
        virtual ~INode() noexcept = default;

        INode(INode&&) noexcept = delete;
        INode& operator=(INode&&) noexcept = delete;

        INode(const INode&) = delete;
        INode& operator=(const INode&) = delete;

        virtual void exec() = 0;
        virtual void pend() = 0;
        virtual void done() = 0;

        virtual bool is_pending() const = 0;
        virtual bool is_ready() const = 0;

    protected:
        enum class EState
        {
            Done = 0b0001,
            Pending = 0b0010
        };
    };
}
