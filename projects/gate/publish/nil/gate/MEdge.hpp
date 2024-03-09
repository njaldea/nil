#pragma once

#include "REdge.hpp"

namespace nil::gate
{
    /**
     * @brief Mutable/Readable Edge type returned by Core::edge.
     *
     * @tparam T
     */
    template <typename T>
    class MutableEdge: public ReadOnlyEdge<T>
    {
    public:
        MutableEdge() = default;
        ~MutableEdge() noexcept override = default;

        MutableEdge(MutableEdge&&) = delete;
        MutableEdge(const MutableEdge&) = delete;
        MutableEdge<T>& operator=(MutableEdge&&) = delete;
        MutableEdge<T>& operator=(const MutableEdge&) = delete;

        virtual void set_value(T new_data) = 0;
    };
}
