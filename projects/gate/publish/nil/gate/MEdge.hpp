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

        /**
         * @brief set value. marks all sub nodes as pending for execution
         *
         * @param data
         */
        void set_value(T new_data)
        {
            if (this->data != new_data)
            {
                this->data = std::move(new_data);
                this->pend();
            }
        }

    protected:
        void pend()
        {
            for (auto* out : this->outs)
            {
                out->pend();
            }
        }

        std::vector<detail::INode*> outs;
    };
}
