#pragma once

#include "REdge.hpp"
#include "detail/INode.hpp"

#include <vector>

namespace nil::gate
{
    /**
     * @brief Mutable/Readable Edge type returned by Core::edge.
     *
     * @tparam T
     */
    template <typename T>
    class MEdge: public REdge<T>
    {
    public:
        /**
         * @brief set value. marks all sub nodes as pending for execution
         *
         * @param data
         */
        void set_value(T data)
        {
            if (this->data != data)
            {
                this->data = std::move(data);
                for (auto* out : outs)
                {
                    out->pend();
                }
            }
        }

    protected:
        std::vector<detail::INode*> outs;
    };
}
