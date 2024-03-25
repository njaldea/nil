#pragma once

#include "ReadOnly.hpp"

namespace nil::gate::edges
{
    /**
     * @brief Mutable/Readable Edge type returned by Core::edge.
     *
     * @tparam T
     */
    template <typename T>
    class Mutable: public ReadOnly<T>
    {
    public:
        Mutable() = default;
        ~Mutable() noexcept override = default;

        Mutable(Mutable&&) = delete;
        Mutable(const Mutable&) = delete;
        Mutable<T>& operator=(Mutable&&) = delete;
        Mutable<T>& operator=(const Mutable&) = delete;

        virtual void set_value(T new_data) = 0;
    };
}
