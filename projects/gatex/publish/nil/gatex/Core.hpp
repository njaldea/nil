#pragma once

#include "INode.hpp"
#include "IType.hpp"
#include "RelaxedEdge.hpp"

#include <nil/gate/Core.hpp>
#include <nil/gate/detail/traits/node.hpp>
#include <nil/gate/nodes/Scoped.hpp>

#include <nil/gatex_proto/identity.pb.h>

#include <boost/asio.hpp>
#include <utility>

namespace nil::gatex
{
    class Core final
    {
    public:
        Core();
        ~Core() noexcept = default;
        Core(Core&&) = delete;
        Core(const Core&) = delete;
        Core& operator=(Core&&) = delete;
        Core& operator=(const Core&) = delete;

        template <typename T>
            requires std::is_same_v<T, std::decay_t<T>>
        void add_type(T default_value)
        {
            struct Type: IType
            {
                T value;

                explicit Type(T init_value)
                    : value(std::move(init_value))
                {
                }

                void create_edge(Core& xcore, std::uint64_t id) override
                {
                    xcore.edges.emplace(id, RelaxedEdge(xcore.core.edge(value)));
                }

                void create_feedback(Core& xcore, const GraphInfo& info, const NodeInfo& node_info)
                    override
                {
                    struct FeedbackNode
                    {
                        void operator()(const nil::gate::Core& c, const T& v, bool enabled) const
                        {
                            if (enabled)
                            {
                                auto [e] = c.batch(output);
                                e->set_value(v);
                            }
                        }

                        nil::gate::edges::Mutable<T>* output;
                    };

                    auto* edge = xcore.core.edge(true);
                    xcore.edges.emplace(node_info.controls.at(0), RelaxedEdge(edge));
                    xcore.core.node(
                        nil::gate::nodes::Scoped<FeedbackNode>(
                            [&xcore, id = node_info.id]() { xcore.activate(id); },
                            FeedbackNode(xcore.edges.at(node_info.outputs.at(0))),
                            [&xcore, id = node_info.id]() { xcore.deactivate(id); }
                        ),
                        {xcore.edges.at(info.opposite_output_pin(node_info.inputs.at(0))), edge}
                    );
                }

                void create_delay(Core& xcore, const GraphInfo& info, const NodeInfo& node_info)
                    override
                {
                    struct Delay
                    {
                        void operator()(
                            const nil::gate::Core& c,
                            nil::gate::async_outputs<T> asyncs,
                            const T& v,
                            float time
                        ) const
                        {
                            if (time == 0)
                            {
                                auto [e] = c.batch(asyncs);
                                e->set_value(std::move(v));
                            }
                            else
                            {
                                xcore->post(
                                    this,
                                    time,
                                    nil::gate::make_callable(
                                        [&c, v, asyncs]() mutable
                                        {
                                            auto [e] = c.batch(asyncs);
                                            e->set_value(std::move(v));
                                        }
                                    )
                                );
                            }
                        }

                        Core* xcore;
                    };

                    constexpr auto DELAY_INITIAL_TIMEOUT = 0.1f;
                    auto* time = xcore.core.edge(DELAY_INITIAL_TIMEOUT);
                    xcore.edges.emplace(node_info.controls.at(0), RelaxedEdge(time));

                    const auto [o] = xcore.core.node(
                        nil::gate::nodes::Scoped<Delay>(
                            [&xcore, id = node_info.id]() { xcore.activate(id); },
                            Delay(&xcore),
                            [&xcore, id = node_info.id]() { xcore.deactivate(id); }
                        ),
                        std::tuple<T>(),
                        {xcore.edges.at(info.opposite_output_pin(node_info.inputs.at(0))), time}
                    );
                    xcore.edges.emplace(node_info.outputs.at(0), RelaxedEdge(o));
                }
            };

            type_factories.emplace_back(std::make_unique<Type>(std::move(default_value)));
            identity.set_types(type_factories.size());
        }

        template <typename T, typename... C>
            requires nil::gate::detail::traits::node<T>::is_valid
        void add_node(T instance, C... controls)
        {
            using node_t = nil::gate::detail::traits::node<T>;
            constexpr auto input_count = node_t::inputs::size;
            constexpr auto output_count = node_t::outputs::size;
            constexpr auto control_count = sizeof...(C);

            struct Node: INode
            {
                explicit Node(T init_instance, C... init_c)
                    : instance(std::move(init_instance))
                    , controls(init_c...)
                {
                }

                void create_node(Core& xcore, const GraphInfo& graph, const NodeInfo& node) override
                {
                    xcore.make_controls(
                        node,
                        controls,
                        std::make_index_sequence<control_count>() //
                    );

                    xcore.make_node(
                        instance,
                        graph,
                        node,
                        std::make_index_sequence<input_count - control_count>(),
                        std::make_index_sequence<control_count>(),
                        std::make_index_sequence<output_count>() //
                    );
                }

                T instance;
                std::tuple<C...> controls;
            };

            node_factories.emplace_back(
                std::make_unique<Node>(std::move(instance), std::move(controls)...)
            );

            add_node_identity<T>(
                typename node_t::inputs::type(),
                typename node_t::outputs::type(),
                nil::gate::detail::traits::types<C...>()
            );
        }

        void instantiate(const GraphInfo& graph);

        void set_control_value(std::uint64_t id, bool value);
        void set_control_value(std::uint64_t id, int value);
        void set_control_value(std::uint64_t id, float value);
        void set_control_value(std::uint64_t id, const std::string& value);

        template <typename T>
        void set_before_hook(T cb)
        {
            before_hook = nil::gate::make_callable<T, std::uint64_t>(std::move(cb));
        }

        template <typename T>
        void set_after_hook(T cb)
        {
            after_hook = nil::gate::make_callable<T, std::uint64_t>(std::move(cb));
        }

        // temporary bypass
        void set_metadata(std::string new_metadata);
        bool is_compatible(const std::string& data) const;

        void start();
        void resume();
        void stop();
        void wait();

    private:
        nil::gate::Core core;
        std::unordered_map<std::uint64_t, RelaxedEdge> edges;
        std::vector<std::unique_ptr<IType>> type_factories;
        std::vector<std::unique_ptr<INode>> node_factories;
        std::unordered_map<const void*, std::uint64_t> type_to_index;

        std::unique_ptr<nil::gate::ICallable<void(std::uint64_t)>> before_hook;
        std::unique_ptr<nil::gate::ICallable<void(std::uint64_t)>> after_hook;

        std::unique_ptr<boost::asio::io_context> context;
        std::unique_ptr<std::thread> thread;
        std::unordered_map<const void*, std::unique_ptr<boost::asio::deadline_timer>> timers;

        std::string metadata;

        nil::gatex_proto::Identity identity;

        template <typename T, typename... IT, typename... OT, typename... CT>
        void add_node_identity(
            nil::gate::detail::traits::types<IT...> /* unused */,
            nil::gate::detail::traits::types<OT...> /* unused */,
            nil::gate::detail::traits::types<CT...> /* unused */
        )
        {
            auto* node = identity.add_nodes();
            const auto add_inputs = [&](std::uint64_t index)
            {
                if (node->inputs_size() < int(sizeof...(IT) - sizeof...(CT)))
                {
                    node->add_inputs(index);
                }
            };
            const auto mapping = [](const void* id)
            {
                if (identity_v<bool> == id)
                {
                    return nil::gatex_proto::Identity_Node_ControlType_Bool;
                }
                if (identity_v<int> == id)
                {
                    return nil::gatex_proto::Identity_Node_ControlType_Int;
                }
                if (identity_v<float> == id)
                {
                    return nil::gatex_proto::Identity_Node_ControlType_Float;
                }
                if (identity_v<std::string> == id)
                {
                    return nil::gatex_proto::Identity_Node_ControlType_Text;
                }
                return nil::gatex_proto::Identity_Node_ControlType_Int;
            };
            (..., add_inputs(type_to_index.at(identity_v<IT>)));
            (..., node->add_outputs(type_to_index.at(identity_v<OT>)));
            (..., node->add_controls(mapping(identity_v<CT>)));
        }

        template <typename... C, std::size_t... I>
        void make_controls(
            const NodeInfo& node,
            const std::tuple<C...>& controls,
            std::index_sequence<I...> /* unused */
        )
        {
            (..., edges.emplace(node.controls.at(I), RelaxedEdge(core.edge(get<I>(controls)))));
        }

        template <typename T, std::size_t... I, std::size_t... C, std::size_t... O>
        void make_node(
            T instance,
            const GraphInfo& graph,
            const NodeInfo& node,
            std::index_sequence<I...> /* unused */,
            std::index_sequence<C...> /* unused */,
            std::index_sequence<O...> /* unused */
        )
        {
            if constexpr (sizeof...(O) == 0)
            {
                core.node(
                    nil::gate::nodes::Scoped<T>(
                        [this, id = node.id]() { activate(id); },
                        std::move(instance),
                        [this, id = node.id]() { deactivate(id); }
                    ),
                    {
                        edges.at(graph.opposite_output_pin(node.inputs.at(I)))...,
                        edges.at(node.controls.at(C))... //
                    }
                );
            }
            else
            {
                const auto output_edges = core.node(
                    nil::gate::nodes::Scoped<T>(
                        [this, id = node.id]() { activate(id); },
                        std::move(instance),
                        [this, id = node.id]() { deactivate(id); }
                    ),
                    {
                        edges.at(graph.opposite_output_pin(node.inputs.at(I)))...,
                        edges.at(node.controls.at(C))... //
                    }
                );
                (..., edges.emplace(node.outputs.at(O), RelaxedEdge(get<O>(output_edges))));
            }
        }

        void make_feedback(const GraphInfo& graph, const NodeInfo& node);
        void make_delay(const GraphInfo& graph, const NodeInfo& node);

        void activate(std::uint64_t id);
        void deactivate(std::uint64_t id);

        void post(const void*, float, std::unique_ptr<nil::gate::ICallable<void()>>);
    };
}
