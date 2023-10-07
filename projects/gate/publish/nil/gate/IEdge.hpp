#pragma once

namespace nil::gate
{
    class IEdge
    {
    public:
        virtual ~IEdge() = default;
        virtual bool has_value() const = 0;
    };
}
