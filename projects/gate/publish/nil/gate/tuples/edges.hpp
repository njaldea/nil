#pragma once

#include <tuple>

namespace nil::gate
{
    template <template <typename> typename EdgeType, typename... T>
    class edges final
    {
    public:
        edges(EdgeType<T>*... init_e)
            : e(init_e...)
        {
        }

        ~edges() = default;

        edges(const edges&) = default;
        edges(edges&&) = delete;

        edges& operator=(const edges&) = default;
        edges& operator=(edges&&) = delete;

        template <std::size_t index>
        auto get() const
        {
            return std::get<index>(e);
        }

    private:
        std::tuple<EdgeType<T>*...> e;
    };

    template <std::size_t index, template <typename> typename EdgeType, typename... T>
    auto get(const edges<EdgeType, T...>& tuplelike)
    {
        return tuplelike.template get<index>();
    }
}

template <template <typename> typename EdgeType, typename... T>
struct std::tuple_size<nil::gate::edges<EdgeType, T...>>
    : std::integral_constant<std::size_t, sizeof...(T)>
{
};

template <std::size_t I, template <typename> typename EdgeType, typename... T>
struct std::tuple_element<I, nil::gate::edges<EdgeType, T...>>
{
    using type = decltype(get<I>(std::declval<nil::gate::edges<EdgeType, T...>>()));
};
