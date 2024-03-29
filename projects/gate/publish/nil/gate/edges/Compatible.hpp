#pragma once

#include "../IEdge.hpp"
#include "../detail/DataEdge.hpp"
#include "../errors.hpp"
#include "../traits/compatibility.hpp"

namespace nil::gate
{
    class INode;
}

namespace nil::gate::errors
{
    template <typename T, typename U, typename = void>
    struct is_compatible
    {
        static constexpr bool value = false;
    };

    template <typename T, typename U>
    struct is_compatible<
        T,
        U,
        std::void_t<decltype(traits::compatibility<T, U>::convert(std::declval<U>()))>>
    {
        static constexpr bool value = true;
    };

    template <typename T, typename U>
    struct CompatibilityError
    {
        Error compatibility = Check<is_compatible<T, U>::value>();
    };
}

namespace nil::gate::edges
{
    template <typename T>
    class Compatible final
    {
    public:
        template <typename U>
            requires(!errors::is_compatible<T, U>::value)
        Compatible(edges::ReadOnly<U>* init_edge, errors::CompatibilityError<T, U> = {}) = delete;

        template <typename U>
            requires(errors::is_compatible<T, U>::value)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        Compatible(edges::ReadOnly<U>* init_edge)
            : edge(init_edge)
            , attach_impl( //
                  +[](IEdge* e, INode* node)
                  { static_cast<detail::edges::Data<U>*>(e)->attach(node); }
              )
            , validate_impl( //
                  +[](IEdge* e, Diffs* diffs) -> bool
                  { return static_cast<detail::edges::Data<U>*>(e)->validate(diffs); }
              )
            , is_pending_impl( //
                  +[](IEdge* e) -> bool
                  { return static_cast<detail::edges::Data<U>*>(e)->is_pending(); }
              )
            , value_impl( //
                  +[](IEdge* e) -> const T&
                  {
                      using compat = traits::compatibility<T, U>;
                      using type = detail::edges::Data<U>;
                      return compat::convert(static_cast<type*>(e)->value());
                  }
              )
        {
        }

        ~Compatible() noexcept = default;

        Compatible(Compatible&&) noexcept = delete;
        Compatible& operator=(Compatible&&) noexcept = delete;

        Compatible(const Compatible&) = default;
        Compatible& operator=(const Compatible&) = default;

        const T& value() const
        {
            return value_impl(edge);
        }

        void attach(INode* node)
        {
            return attach_impl(edge, node);
        }

        bool is_pending() const
        {
            return is_pending_impl(edge);
        }

        bool validate(Diffs* diffs) const
        {
            return validate_impl(edge, diffs);
        }

    private:
        nil::gate::IEdge* edge = nullptr;
        void (*attach_impl)(IEdge*, INode*);
        bool (*validate_impl)(IEdge*, Diffs* diffs);
        bool (*is_pending_impl)(IEdge*);
        const T& (*value_impl)(IEdge*);
    };
}
