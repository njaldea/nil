#pragma once

#include "identity.hpp"

#include <nil/gate/IEdge.hpp>
#include <nil/gate/edges/Compatible.hpp>
#include <nil/gate/edges/Mutable.hpp>
#include <nil/gate/edges/ReadOnly.hpp>

namespace nil::gatex
{
    class RelaxedEdge
    {
    public:
        template <typename T>
        explicit RelaxedEdge(nil::gate::edges::ReadOnly<T>* init_edge)
            : edge(init_edge)
            , identity(identity_v<T>)
        {
        }

        template <typename T>
        void set_value(T value)
        {
            if (identity_v<T> != identity)
            {
                throw std::runtime_error("incompatible types");
            }
            static_cast<nil::gate::edges::Mutable<T>*>(edge)->set_value(std::move(value));
        }

        template <typename T>
        operator nil::gate::edges::Mutable<T>*() const // NOLINT(hicpp-explicit-conversions)
        {
            if (identity_v<T> != identity)
            {
                throw std::runtime_error("incompatible types");
            }
            return static_cast<nil::gate::edges::Mutable<T>*>(edge);
        }

        template <typename T>
        operator nil::gate::edges::Compatible<T>() const // NOLINT(hicpp-explicit-conversions)
        {
            if (identity_v<T> != identity)
            {
                throw std::runtime_error("incompatible types");
            }
            return static_cast<nil::gate::edges::ReadOnly<T>*>(edge);
        }

    private:
        nil::gate::IEdge* edge;
        const void* identity;
    };
}
