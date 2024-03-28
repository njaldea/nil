#pragma once

#include "Control.hpp"
#include "nil/gate/edges/Compatible.hpp"
#include "nil/gate/edges/ReadOnly.hpp"

#include <nil/dev.hpp>
#include <nil/gate.hpp>
#include <nil/gate/api/uniform.hpp>
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
        std::array<float, 4> color = {};
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
            template <typename T>
            RelaxedEdge(nil::gate::edges::ReadOnly<T>* init_edge)
                : edge(init_edge)
                , identity(nil::utils::traits::identity_v<T>)
            {
            }

            nil::gate::IEdge* edge;
            const void* identity;

            template <typename T>
            operator nil::gate::edges::Mutable<T>*() const
            {
                if (nil::utils::traits::identity_v<T> != identity)
                {
                    throw std::runtime_error("incompatible types");
                }
                return static_cast<nil::gate::edges::Mutable<T>*>(edge);
            }

            template <typename T>
            operator nil::gate::edges::ReadOnly<T>*() const
            {
                if (nil::utils::traits::identity_v<T> != identity)
                {
                    throw std::runtime_error("incompatible types");
                }
                return static_cast<nil::gate::edges::ReadOnly<T>*>(edge);
            }

            template <typename T>
            operator nil::gate::edges::Compatible<T>() const
            {
                if (nil::utils::traits::identity_v<T> != identity)
                {
                    throw std::runtime_error("incompatible types");
                }
                return static_cast<nil::gate::edges::ReadOnly<T>*>(edge);
            }

            template <typename T>
            void set_value(T value)
            {
                if (nil::utils::traits::identity_v<T> != identity)
                {
                    throw std::runtime_error("incompatible types");
                }
                static_cast<nil::gate::edges::Mutable<T>*>(edge)->set_value(value);
            }
        };

        std::unique_ptr<nil::gate::Core> core;
        std::unordered_map<std::uint64_t, RelaxedEdge> control_edges;
        std::unordered_map<std::uint64_t, RelaxedEdge> internal_edges;
        std::function<void(std::uint64_t)> activate;
        std::function<void(std::uint64_t)> deactivate;
        std::shared_ptr<bool> paused = std::make_shared<bool>(false);

        std::function<void(const void*, float, std::function<void()>)> post;
    };

    struct AppState
    {
        using IDs = std::vector<std::uint64_t>;
        using NodeFactory = std::function<
            void(GraphState&, std::uint64_t, std::uint64_t, const IDs&, const IDs&, const IDs&)>;
        using EdgeFactory = std::function<void(GraphState&, std::uint64_t)>;

        nil::nedit::proto::State info;
        std::vector<NodeFactory> node_factories;
        std::vector<NodeFactory> feedback_node_factories;
        std::vector<NodeFactory> delay_node_factories;
        std::vector<EdgeFactory> edge_factories;
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
                nil::gate::detail::traits::types<Inputs...> /* unused */,
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
                nil::gate::detail::traits::types<Outputs...> /* unused */
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

                [[maybe_unused]] const auto output_edges = nil::gate::api::uniform::add_node(
                    T(args...),
                    *state.core,
                    std::move(a),
                    typename nil::gate::detail::traits::node<T>::inputs::edges(
                        state.internal_edges.at(i.at(i_indices))...,
                        state.control_edges.at(c.at(c_indices))...
                    )
                );
                ( //
                    state.internal_edges.emplace(o.at(o_indices), RE{get<o_indices>(output_edges)}),
                    ...
                );
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
                using input_t = typename nil::gate::detail::traits::node<T>::inputs;
                using output_t = typename nil::gate::detail::traits::node<T>::outputs;

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
            AppState::NodeFactory factory(
                const Node<T, Controls...>& node,
                std::tuple<A...> a,
                const Args&... args
            )
            {
                using input_t = typename nil::gate::detail::traits::node<T>::inputs;
                using output_t = typename nil::gate::detail::traits::node<T>::outputs;

                const auto controls = get_controls(node);
                return                                             //
                    [controls, a = std::move(a), ... args = args]( //
                        GraphState& state,
                        std::uint64_t id,
                        std::uint64_t /* alias */,
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
                    detail::creation::create_node<nil::gate::nodes::Scoped<T>>(
                        state,
                        a,
                        input_ids,
                        output_ids,
                        control_ids,
                        std::make_index_sequence<input_count>(),
                        std::make_index_sequence<output_t::size>(),
                        std::make_index_sequence<sizeof...(Controls)>(),
                        [&state, id]() { state.activate(id); },
                        [&state, id]() { state.deactivate(id); },
                        args...
                    );
                };
            }
        }
    }

    class App final
    {
    public:
        App(AppState& init_state)
            : state(init_state)
        {
            struct Any
            {
                bool operator==(const Any& /* unused */) const
                {
                    return true;
                }
            };

            add_pin<Any>({"any", {0.5f, 0.5f, 0.5f, 1.0f}});

            {
                struct N
                {
                    Any operator()(Any, bool) const;
                };

                auto& node_message = *state.info.mutable_types()->add_nodes();
                detail::api::to_message(
                    node_message,
                    Node<N, Value<bool>>{"feedback", {{.value = true}}},
                    state.type_to_pin_index
                );
                state.node_factories.emplace_back( //
                    [this](
                        GraphState& graph_state,
                        std::uint64_t id,
                        std::uint64_t alias,
                        const std::vector<std::uint64_t>& i_ids,
                        const std::vector<std::uint64_t>& s_ids,
                        const std::vector<std::uint64_t>& c_ids
                    ) {
                        state.feedback_node_factories[alias](
                            graph_state,
                            id,
                            alias,
                            i_ids,
                            s_ids,
                            c_ids
                        );
                    }
                );
            }
            {
                struct N
                {
                    void operator()(nil::gate::async_outputs<Any>, Any, float) const;
                };

                auto& node_message = *state.info.mutable_types()->add_nodes();
                detail::api::to_message(
                    node_message,
                    Node<N, MinMax<float>>{"delay", {{.value = 0.1f, .min = 0.0f, .max = 5.f}}},
                    state.type_to_pin_index
                );
                state.node_factories.emplace_back( //
                    [this](
                        GraphState& graph_state,
                        std::uint64_t id,
                        std::uint64_t alias,
                        const std::vector<std::uint64_t>& i_ids,
                        const std::vector<std::uint64_t>& s_ids,
                        const std::vector<std::uint64_t>& c_ids
                    ) {
                        state.delay_node_factories[alias](
                            graph_state,
                            id,
                            alias,
                            i_ids,
                            s_ids,
                            c_ids
                        );
                    }
                );
            }
        }

        ~App() = default;

        App(App&&) noexcept = delete;
        App& operator=(App&&) noexcept = delete;

        App(const App&) = delete;
        App& operator=(const App&) = delete;

        template <typename T>
        App& add_pin(const Pin<T>& pin)
        {
            constexpr auto identity = nil::utils::traits::identity_v<T>;
            state.type_to_pin_index.emplace(identity, state.info.types().pins_size());
            detail::api::to_message(*state.info.mutable_types()->add_pins(), pin);

            state.edge_factories.push_back(
                [](GraphState& graph_state, std::uint64_t id)
                {
                    graph_state         //
                        .internal_edges //
                        .emplace(id, ext::GraphState::RelaxedEdge{graph_state.core->edge(T())});
                }
            );
            state.feedback_node_factories.push_back(
                [](GraphState& graph_state,
                   std::uint64_t id,
                   std::uint64_t alias,
                   const std::vector<std::uint64_t>& i_ids,
                   const std::vector<std::uint64_t>& s_ids,
                   const std::vector<std::uint64_t>& c_ids)
                {
                    struct FeedbackNode
                    {
                        void operator()(const T& v, bool enabled) const
                        {
                            if (output == nullptr)
                            {
                                output = graph_state->internal_edges.at(s_id);
                            }
                            if (enabled)
                            {
                                if (output->value() != v)
                                {
                                    output->set_value(v);
                                    graph_state->core->commit();
                                }
                            }
                        }

                        GraphState* graph_state;
                        std::uint64_t s_id;
                        mutable nil::gate::edges::Mutable<T>* output;
                    };

                    auto factory = detail::api::factory(
                        Node<FeedbackNode, Value<bool>>{"feedback", {{.value = true}}},
                        std::tuple<>(),
                        &graph_state,
                        s_ids.at(0),
                        nullptr
                    );
                    factory(graph_state, id, alias, i_ids, s_ids, c_ids);
                }
            );
            state.delay_node_factories.push_back(
                [](GraphState& graph_state,
                   std::uint64_t id,
                   std::uint64_t alias,
                   const std::vector<std::uint64_t>& i_ids,
                   const std::vector<std::uint64_t>& s_ids,
                   const std::vector<std::uint64_t>& c_ids)
                {
                    struct Delay
                    {
                        void operator()(const T& v, float time) const
                        {
                            graph_state->post(
                                this,
                                time,
                                [this, v]() mutable
                                {
                                    if (async_output->value() != v)
                                    {
                                        async_output->set_value(std::move(v));
                                        graph_state->core->commit();
                                    }
                                }
                            );
                        }

                        GraphState* graph_state;
                        nil::gate::edges::Mutable<T>* async_output;
                    };

                    auto e = graph_state.core->edge(T());
                    graph_state         //
                        .internal_edges //
                        .emplace(id, ext::GraphState::RelaxedEdge{e});
                    auto factory = detail::api::factory(
                        Node<Delay, MinMax<float>>{
                            "delay",
                            {{.value = 0.1f, .min = 0.0f, .max = 5.f}}
                        },
                        std::tuple<>(),
                        &graph_state,
                        e
                    );
                    factory(graph_state, id, alias, i_ids, s_ids, c_ids);
                }
            );
            return *this;
        }

        template <typename T, typename... Controls, typename... Args>
        std::enable_if_t<!nil::gate::detail::traits::node<T>::has_async, App&> add_node(
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
        std::enable_if_t<nil::gate::detail::traits::node<T>::has_async, App&> add_node(
            const Node<T, Controls...>& node,
            nil::gate::detail::traits::node<T>::async_outputs::tuple a,
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
