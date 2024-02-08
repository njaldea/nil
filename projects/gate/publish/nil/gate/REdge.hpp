#pragma once

#include "IEdge.hpp"

#include <optional>
#include <utility>

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
        template <typename... Args>
        ReadOnlyEdge(Args&&... args)
            : data(std::make_optional<T>(std::forward<Args>(args)...))
        {
        }

        ReadOnlyEdge() = default;
        ~ReadOnlyEdge() noexcept override = default;

        ReadOnlyEdge(ReadOnlyEdge&&) = delete;
        ReadOnlyEdge(const ReadOnlyEdge&) = delete;
        ReadOnlyEdge& operator=(ReadOnlyEdge&&) = delete;
        ReadOnlyEdge& operator=(const ReadOnlyEdge&&) = delete;

        // For edges created for a Node, make sure to call core.run() before accessing value.
        // For edges created on its own, it should always have a value due to Core's api.
        const T& value() const
        {
            return *data;
        }

    protected:
        /**
         * @brief This is stored as optional to allow creation of edges for output of nodes.
         *  Creation of Edge from Core guarantees that the edge always has a value.
         */
        std::optional<T> data;
    };
}
