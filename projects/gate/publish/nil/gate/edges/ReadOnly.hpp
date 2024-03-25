#pragma once

#include "../IEdge.hpp"

namespace nil::gate::edges
{
    /**
     * @brief Readable Edge type returned by Core::node.
     *
     * @tparam T
     */
    template <typename T>
    class ReadOnly: public IEdge
    {
    public:
        ReadOnly() = default;
        ~ReadOnly() noexcept override = default;

        ReadOnly(ReadOnly&&) = delete;
        ReadOnly(const ReadOnly&) = delete;
        ReadOnly& operator=(ReadOnly&&) = delete;
        ReadOnly& operator=(const ReadOnly&) = delete;

        // For edges created for a Node, make sure to call core.run() before accessing value.
        // For edges created on its own, it should always have a value due to Core's api.
        virtual const T& value() const = 0;
    };
}
