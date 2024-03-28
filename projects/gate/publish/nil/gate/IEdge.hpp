#pragma once

namespace nil::gate
{
    class IEdge
    {
    public:
        IEdge() = default;
        virtual ~IEdge() noexcept = default;

        IEdge(IEdge&&) noexcept = delete;
        IEdge& operator=(IEdge&&) noexcept = delete;

        IEdge(const IEdge&) = delete;
        IEdge& operator=(const IEdge&) = delete;
    };
}
