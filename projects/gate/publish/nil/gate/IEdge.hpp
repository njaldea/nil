#pragma once

namespace nil::gate
{
    class IEdge
    {
    public:
        IEdge() = default;
        virtual ~IEdge() noexcept = default;

        IEdge(IEdge&&) = delete;
        IEdge(const IEdge&) = delete;
        IEdge& operator=(IEdge&&) = delete;
        IEdge& operator=(const IEdge&) = delete;
    };
}
