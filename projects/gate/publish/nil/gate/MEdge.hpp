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
    class MutableEdge: public ReadOnlyEdge<T>
    {
    public:
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
