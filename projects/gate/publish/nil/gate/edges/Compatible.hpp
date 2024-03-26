#pragma once

#include "../IEdge.hpp"
#include "../detail/DataEdge.hpp"
#include "../errors.hpp"
#include "../traits/compatibility.hpp"

namespace nil::gate::detail
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
            , attach_output_impl( //
                  +[](IEdge* e, detail::INode* node)
                  { static_cast<detail::edges::Data<U>*>(e)->attach_output(node); }
              )
            , depth_impl( //
                  +[](IEdge* e)
                  {
                      using type = detail::edges::Data<U>;
                      return static_cast<type*>(e)->depth();
                  }
              )
            , value_impl( //
                  +[](IEdge* e) -> const T&
                  {
                      using type = edges::ReadOnly<U>;
                      return traits::compatibility<T, U>::convert(static_cast<type*>(e)->value());
                  }
              )
        {
        }

        ~Compatible() noexcept = default;

        Compatible(Compatible&&) = delete;
        Compatible(const Compatible&) = default;
        Compatible& operator=(Compatible&&) = delete;
        Compatible& operator=(const Compatible&) = default;

        const T& value() const
        {
            return value_impl(edge);
        }

        void attach_output(detail::INode* node)
        {
            return attach_output_impl(edge, node);
        }

        std::uint64_t depth() const
        {
            return depth_impl(edge);
        }

        nil::gate::IEdge* edge = nullptr;
        void (*attach_output_impl)(IEdge*, detail::INode*);
        std::uint64_t (*depth_impl)(IEdge*);
        const T& (*value_impl)(IEdge*);
    };
}
