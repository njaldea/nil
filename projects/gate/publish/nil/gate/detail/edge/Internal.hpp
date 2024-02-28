#pragma once

#include "../../MEdge.hpp"

namespace nil::gate::detail
{
    /**
     * @brief Edge type returned by Core::edge.
     *  For internal use.
     *
     * @tparam T
     */
    template <typename T>
    class InternalEdge: public MutableEdge<T>
    {
    public:
        InternalEdge() = default;
        ~InternalEdge() noexcept = default;

        InternalEdge(InternalEdge&&) = delete;
        InternalEdge(const InternalEdge&) = delete;
        InternalEdge& operator=(InternalEdge&&) = delete;
        InternalEdge& operator=(const InternalEdge&) = delete;

        virtual bool exec(T new_data) = 0;
        virtual void pend() = 0;
        virtual void attach_output(INode* node) = 0;
        virtual std::uint64_t depth() const = 0;
    };
}
