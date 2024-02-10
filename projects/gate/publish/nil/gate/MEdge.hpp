#pragma once

#include "REdge.hpp"
#include "detail/INode.hpp"

#include <utility>
#include <vector>

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
        template <typename... Args>
        MutableEdge(Args&&... args)
            : ReadOnlyEdge<T>(std::forward<Args>(args)...)
        {
        }

        MutableEdge() = default;
        ~MutableEdge() noexcept override = default;

        MutableEdge(MutableEdge&&) = delete;
        MutableEdge(const MutableEdge&) = delete;
        MutableEdge& operator=(MutableEdge&&) = delete;
        MutableEdge& operator=(const MutableEdge&&) = delete;

        virtual void set_value(T new_data) = 0;
    };
}
