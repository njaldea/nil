#pragma once

#include "sort_nodes.hpp"

#include <nil/gate.hpp>
#include <nil/utils/traits/identity.hpp>

#include <gen/nedit/messages/state.pb.h>

#include <array>
#include <functional>

namespace ext
{
    template <typename T>
    struct Pin
    {
        std::string label;
        std::array<float, 4> color;
    };

    template <typename T, typename... Controls>
    struct Node;

    template <typename T>
    struct Node<T>
    {
        std::string label;
    };

    template <typename T, typename FirstControl, typename... Controls>
    struct Node<T, FirstControl, Controls...>
    {
        std::string label;
        std::tuple<FirstControl, Controls...> controls;
    };

    struct GraphState
    {
        struct RelaxedEdge
        {
            nil::gate::IEdge* edge;

            template <typename T>
            operator nil::gate::MutableEdge<T>*() const
            {
                return static_cast<nil::gate::MutableEdge<T>*>(edge);
            }

            template <typename T>
            operator nil::gate::ReadOnlyEdge<T>*() const
            {
                return static_cast<nil::gate::ReadOnlyEdge<T>*>(edge);
            }

            template <typename T>
            void set_value(T value)
            {
                static_cast<nil::gate::MutableEdge<T>*>(edge)->set_value(value);
            }
        };

        std::unique_ptr<nil::gate::Core> core;
        std::unordered_map<std::uint64_t, RelaxedEdge> control_edges;
        std::unordered_map<std::uint64_t, RelaxedEdge> internal_edges;
        std::function<void(std::uint64_t)> activate;
        std::function<void(std::uint64_t)> deactivate;
    };

    struct Scoped
    {
        Scoped(GraphState& init_state, std::uint64_t init_id)
            : state(init_state)
            , id(init_id)
        {
            state.activate(id);
        }

        ~Scoped()
        {
            state.deactivate(id);
        }

        GraphState& state;
        std::uint64_t id;
    };

    template <typename T, typename Inputs, typename Asyncs>
    struct Activatable;

    template <typename T, typename... Inputs>
    struct Activatable<T, nil::utils::traits::types<Inputs...>, nil::utils::traits::types<>>
    {
        template <typename... Args>
        Activatable(GraphState& init_state, std::uint64_t init_id, const Args&... args)
            : state(init_state)
            , id(init_id)
            , object{args...}
        {
        }

        auto operator()(const Inputs&... args) const
        {
            Scoped _(state, id);
            return object.operator()(args...);
        }

        GraphState& state;
        std::uint64_t id;
        T object;
    };

    template <typename T, typename... Inputs, typename... Asyncs>
    struct Activatable<
        T,
        nil::utils::traits::types<Inputs...>,
        nil::utils::traits::types<Asyncs...>>
    {
        template <typename... Args>
        Activatable(GraphState& init_state, std::uint64_t init_id, const Args&... args)
            : state(init_state)
            , id(init_id)
            , object{args...}
        {
        }

        auto operator()(
            std::tuple<nil::gate::MutableEdge<Asyncs>*...> asyncs,
            const Inputs&... args
        ) const
        {
            Scoped _(state, id);
            return object.operator()(asyncs, args...);
        }

        GraphState& state;
        std::uint64_t id;
        T object;
    };

    struct AppState
    {
        using IDs = std::vector<std::uint64_t>;
        using Factory
            = std::function<void(GraphState&, std::uint64_t, const IDs&, const IDs&, const IDs&)>;

        nil::nedit::proto::State info;
        std::vector<Factory> node_factories;
        std::unordered_map<const void*, std::uint64_t> type_to_pin_index;
    };

    namespace detail
    {
        template <typename T, typename FirstControl, typename... Controls>
        const auto& get_controls(const Node<T, FirstControl, Controls...>& node)
        {
            return node.controls;
        }

        template <typename T>
        auto get_controls(const Node<T>& /* unused */)
        {
            return std::tuple<>();
        }

        namespace msg
        {
            template <typename... Inputs, std::size_t... indices>
            void add_inputs(
                const std::unordered_map<const void*, std::uint64_t>& type_to_pin_index,
                nil::nedit::proto::State::Types::Node& info,
                nil::utils::traits::types<Inputs...> /* unused */,
                std::index_sequence<indices...> /* unused */
            )
            {
                ( //
                    info.add_inputs(type_to_pin_index.at(
                        nil::utils::traits::identity_v<
                            std::tuple_element_t<indices, std::tuple<Inputs...>>> //
                    )),
                    ...
                );
            }

            template <typename... Outputs>
            void add_outputs(
                const std::unordered_map<const void*, std::uint64_t>& type_to_pin_index,
                nil::nedit::proto::State::Types::Node& info,
                nil::utils::traits::types<Outputs...> /* unused */
            )
            {
                (info.add_outputs(type_to_pin_index.at(nil::utils::traits::identity_v<Outputs>)),
                 ...);
            }

            template <typename... Controls, std::size_t... indices>
            void add_controls(
                nil::nedit::proto::State::Types::Node& info,
                const std::tuple<Controls...>& controls,
                std::index_sequence<indices...> /* unused */
            )
            {
                (to_message(std::get<indices>(controls), info), ...);
            }
        }

        namespace creation
        {
            template <typename C>
            void create_control_edge(GraphState& state, std::uint64_t id, const C& control)
            {
                state.control_edges.emplace(
                    id,
                    GraphState::RelaxedEdge{
                        state.core->edge<decltype(std::declval<C>().value)>(control.value)
                    }
                );
            }

            template <typename T, typename... C, std::size_t... indices>
            void create_control_edges(
                GraphState& state,
                const std::vector<std::uint64_t>& ids,
                std::index_sequence<indices...> /* unused */,
                const std::tuple<C...>& controls
            )

            {
                if constexpr (sizeof...(C) > 0)
                {
                    (create_control_edge<C>(state, ids[indices], std::get<indices>(controls)), ...);
                }
            }

            template <
                typename T,
                typename... A,
                std::size_t... i_indices,
                std::size_t... o_indices,
                std::size_t... c_indices,
                typename... Args>
            void create_node(
                GraphState& state,
                std::uint64_t id,
                std::tuple<A...> a,
                const std::vector<std::uint64_t>& i,
                const std::vector<std::uint64_t>& o,
                const std::vector<std::uint64_t>& c,
                std::index_sequence<i_indices...> /* unused */,
                std::index_sequence<o_indices...> /* unused */,
                std::index_sequence<c_indices...> /* unused */,
                const Args&... args
            )
            {
                using RE = GraphState::RelaxedEdge;
                using N = Activatable<
                    T,
                    typename nil::gate::detail::traits<T>::i::type,
                    typename nil::gate::detail::traits<T>::a::type>;
                const auto i_edges = typename nil::gate::detail::traits<T>::i::readonly_edges{
                    state.internal_edges.at(i.at(i_indices))...,
                    state.control_edges.at(c.at(c_indices))...
                };
                if constexpr (sizeof...(o_indices) > 0u)
                {
                    if constexpr (sizeof...(A) == 0)
                    {
                        const auto r = state.core->node<N>(i_edges, state, id, args...);
                        (state.internal_edges.emplace(o.at(o_indices), RE{std::get<o_indices>(r)}),
                         ...);
                    }
                    else
                    {
                        const auto r
                            = state.core->node<N>(std::move(a), i_edges, state, id, args...);
                        (state.internal_edges.emplace(o.at(o_indices), RE{std::get<o_indices>(r)}),
                         ...);
                    }
                }
                else
                {
                    if constexpr (sizeof...(A) == 0)
                    {
                        state.core->node<N>(i_edges, state, id, args...);
                    }
                    else
                    {
                        state.core->node<N>(std::move(a), i_edges, state, id, args...);
                    }
                }
            }
        }

        namespace api
        {
            template <typename T>
            void to_message(nil::nedit::proto::State::Types::Pin& info, const Pin<T>& pin)
            {
                info.mutable_color()->set_r(pin.color[0]);
                info.mutable_color()->set_g(pin.color[1]);
                info.mutable_color()->set_b(pin.color[2]);
                info.mutable_color()->set_a(pin.color[3]);
                info.set_label(pin.label);
            }

            template <typename T, typename... Controls>
            void to_message(
                nil::nedit::proto::State::Types::Node& info,
                const Node<T, Controls...>& node,
                std::unordered_map<const void*, std::uint64_t> type_to_pin_index
            )
            {
                using input_t = typename nil::gate::detail::traits<T>::i;
                using output_t = typename nil::gate::detail::traits<T>::outs;

                detail::msg::add_inputs(
                    type_to_pin_index,
                    info,
                    typename input_t::type(),
                    std::make_index_sequence<input_t::size - sizeof...(Controls)>()
                );
                detail::msg::add_outputs(
                    type_to_pin_index,
                    info,
                    typename output_t::type() //
                );
                detail::msg::add_controls(
                    info,
                    get_controls(node),
                    std::make_index_sequence<sizeof...(Controls)>()
                );
                info.set_label(node.label);
            }

            template <typename T, typename... Controls, typename... A, typename... Args>
            AppState::Factory factory(
                const Node<T, Controls...>& node,
                std::tuple<A...> a,
                const Args&... args
            )
            {
                using input_t = typename nil::gate::detail::traits<T>::i;
                using output_t = typename nil::gate::detail::traits<T>::outs;

                const auto controls = get_controls(node);
                return                                             //
                    [controls, a = std::move(a), ... args = args]( //
                        GraphState& state,
                        std::uint64_t id,
                        const std::vector<std::uint64_t>& input_ids,
                        const std::vector<std::uint64_t>& output_ids,
                        const std::vector<std::uint64_t>& control_ids
                    )
                {
                    if (sizeof...(Controls) > 0)
                    {
                        detail::creation::create_control_edges<T>(
                            state,
                            control_ids,
                            std::make_index_sequence<sizeof...(Controls)>(),
                            controls
                        );
                    }
                    constexpr auto input_count = input_t::size - sizeof...(Controls);
                    detail::creation::create_node<T>(
                        state,
                        id,
                        a,
                        input_ids,
                        output_ids,
                        control_ids,
                        std::make_index_sequence<input_count>(),
                        std::make_index_sequence<output_t::size>(),
                        std::make_index_sequence<sizeof...(Controls)>(),
                        args...
                    );
                };
            }
        }
    }

    // [TODO]
    //  -  find a way to abstract out the message type
    //  -  create a library. if possible, gate can take ownership of this, only if the message
    //  handling is abstracted out.
    class App final
    {
    public:
        App(AppState& init_state)
            : state(init_state)
        {
        }

        ~App() = default;
        App(const App&) = delete;
        App(App&&) = delete;
        App& operator=(const App&) = delete;
        App& operator=(App&&) = delete;

        template <typename T>
        App& add_pin(const Pin<T>& pin)
        {
            constexpr auto identity = nil::utils::traits::identity_v<T>;
            state.type_to_pin_index.emplace(identity, state.info.types().pins_size());
            detail::api::to_message(*state.info.mutable_types()->add_pins(), pin);
            return *this;
        }

        template <typename T, typename... Controls, typename... Args>
        std::enable_if_t<(nil::gate::detail::traits<T>::a::size == 0), App&> add_node(
            const Node<T, Controls...>& node,
            const Args&... args
        )
        {
            auto& node_message = *state.info.mutable_types()->add_nodes();
            detail::api::to_message(node_message, node, state.type_to_pin_index);
            state.node_factories.push_back(detail::api::factory(node, std::tuple<>(), args...));
            return *this;
        }

        template <typename T, typename... Controls, typename... A, typename... Args>
        std::enable_if_t<(nil::gate::detail::traits<T>::a::size > 0), App&> add_node(
            const Node<T, Controls...>& node,
            nil::gate::detail::traits<T>::a::tuple a,
            const Args&... args
        )
        {
            auto& node_message = *state.info.mutable_types()->add_nodes();
            detail::api::to_message(node_message, node, state.type_to_pin_index);
            state.node_factories.push_back(detail::api::factory(node, a, args...));
            return *this;
        }

    private:
        AppState& state;
    };
}
