#pragma once

#include "IEdge.hpp"

#include <optional>

namespace nil::gate
{
    /**
     * @brief Readable Edge type returned by Core::node.
     *
     * @tparam T
     */
    template <typename T>
    class ReadOnlyEdge: public IEdge
    {
    public:
        const T& value() const
        {
            return *data;
        }

        bool has_value() const override
        {
            return data.has_value();
        }

    protected:
        std::optional<T> data;
    };
}
