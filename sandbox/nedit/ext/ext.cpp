#include "ext.hpp"
#include "../codec.hpp"

#include <nil/gate.hpp>
#include <nil/service.hpp>
#include <nil/utils/traits/type.hpp>

#include <array>
#include <iostream>
#include <sstream>

#include <gen/nedit/messages/control_update.pb.h>
#include <gen/nedit/messages/graph_update.pb.h>
#include <gen/nedit/messages/node_info.pb.h>
#include <gen/nedit/messages/pin_info.pb.h>
#include <gen/nedit/messages/type.pb.h>

namespace
{
    template <typename T>
    struct Control
    {
        using type = T;

        static T value()
        {
            return T();
        }

        // description how to populate nodeinfo (message)
    };

    template <typename T>
    struct InputWithControl
    {
        InputWithControl(std::string init_name)
            : name(std::move(init_name))
        {
        }

        std::tuple<T> operator()(const T& value)
        {
            std::cout << "InputWithControl_" << name << '<' << value << '>' << std::endl;
            return {value};
        }

        std::string name;
    };

    template <typename T>
    struct Input
    {
        Input(std::string init_name, T init_value)
            : name(std::move(init_name))
            , value(std::move(init_value))
        {
        }

        std::tuple<T> operator()()
        {
            std::cout << "Input_" << name << '<' << value << '>' << std::endl;
            return {value};
        }

        std::string name;
        T value;
    };

    struct Add
    {
        std::tuple<int> operator()(int l, int r)
        {
            std::cout << l << " + " << r << std::endl;
            return {l + r};
        }
    };

    struct Mul
    {
        std::tuple<int> operator()(int l, int r)
        {
            std::cout << l << " * " << r << std::endl;
            return {l * r};
        }
    };

    struct Inverter
    {
        std::tuple<int> operator()(bool l, int r)
        {
            std::cout << l << " ! " << r << std::endl;
            return {l ? -r : r};
        }
    };

    template <typename T>
    struct Consume
    {
        void operator()(T v)
        {
            std::cout << "Consume: " << v << std::endl;
        }
    };

    // [TODO]
    //  -  find a way to abstract out the message type
    //  -  create a library. if possible, gate can take ownership of this, only if the message
    //  handling is abstracted out.
    class App
    {
        using IDs = std::vector<std::uint64_t>;
        using Factory = std::function<void(nil::gate::Core&, const IDs&, const IDs&, const IDs&)>;

    public:
        App()
        {
            std::cout << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << std::endl;
        }

        App(const App&) = delete;
        App(App&&) = delete;
        App& operator=(const App&) = delete;
        App& operator=(App&&) = delete;

        template <typename T>
        App& add_pin(std::string label, std::array<float, 4> color)
        {
            type_to_pin_index.emplace(nil::utils::traits::type<T>::value, pins.size());
            pins.push_back(
                [&]()
                {
                    nil::nedit::proto::PinInfo info;
                    info.mutable_color()->set_r(color[0]);
                    info.mutable_color()->set_g(color[1]);
                    info.mutable_color()->set_b(color[2]);
                    info.mutable_color()->set_a(color[3]);
                    info.set_label(std::move(label));
                    return info;
                }()
            );
            return *this;
        }

        template <typename T, typename... Controls, typename... Args>
        App& add_node(std::string label, Args&&... args)
        {
            nodes.push_back(
                [&]()
                {
                    nil::nedit::proto::NodeInfo info;
                    using input_t = typename nil::gate::detail::traits<std::decay_t<T>>::i::type;
                    add_inputs<T>(info, input_t());
                    using output_t = typename nil::gate::detail::traits<std::decay_t<T>>::o::type;
                    add_outputs<T>(info, output_t());
                    add_controls<T, Controls...>(info);
                    info.set_label(std::move(label));
                    return info;
                }()
            );
            // builder.add_node_type<T>(std::forward<Args>(args)...);
            node_factories.push_back(
                [&, ... args = args](
                    nil::gate::Core& core,
                    const std::vector<std::uint64_t>& inputs,
                    const std::vector<std::uint64_t>& outputs,
                    const std::vector<std::uint64_t>& controls
                )
                {
                    if (sizeof...(Controls) > 0)
                    {
                        // controls.size() == sizeof...(Controls)
                        const auto exposed_edges = core.edges<typename Controls::type...>();
                        this->create_control_edges<T, Controls...>(
                            exposed_edges,
                            controls,
                            std::make_index_sequence<sizeof...(Controls)>()
                        );
                        this->create_node<T, Controls...>(
                            core,
                            exposed_edges,
                            inputs,
                            outputs,
                            args...
                        );
                    }
                    else
                    {
                        this->create_node<T>(core, {}, inputs, outputs, args...);
                    }
                }
            );
            return *this;
        }

        // TODO: for fixing
        // - control_edges
        // - internal_edges
        // - node_factories
        // - create_control_edges
        // - create_control_edges
        void reset()
        {
            control_edges.clear();
            internal_edges.clear();
        }

        std::vector<Factory> node_factories;
        std::vector<nil::nedit::proto::PinInfo> pins;   // messages
        std::vector<nil::nedit::proto::NodeInfo> nodes; // messages
        std::unordered_map<std::uint64_t, nil::gate::IEdge*> control_edges;

    private:
        std::unordered_map<std::uint64_t, nil::gate::IEdge*> internal_edges;
        std::unordered_map<const void*, std::uint64_t> type_to_pin_index;

        template <typename T, typename... C, std::size_t... indices>
        void create_control_edges(
            std::tuple<nil::gate::MutableEdge<typename C::type>*...> mutable_edges,
            std::vector<std::uint64_t> ids,
            std::index_sequence<indices...> //
        )
        {
            if constexpr (sizeof...(C) > 0)
            {
                (create_control_edge<T, C>(std::get<indices>(mutable_edges), ids[indices]), ...);
            }
        }

        template <typename T, typename C>
        void create_control_edge(nil::gate::MutableEdge<typename C::type>* edge, std::uint64_t id)
        {
            edge->set_value(C::value()); // default value
            std::cout << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << std::endl;
            std::cout << id << std::endl;
            std::cout << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << std::endl;
            for (const auto& i : control_edges)
            {
                std::cout << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << std::endl;
                std::cout << i.first << std::endl;
            }
            control_edges.emplace(id, edge);
            std::cout << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << std::endl;
            for (const auto& i : control_edges)
            {
                std::cout << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << std::endl;
                std::cout << i.first << std::endl;
            }
        }

        template <
            typename T,
            typename... C,
            std::size_t... input_indices,
            std::size_t... output_indices,
            std::size_t... control_indices,
            typename... Args>
        void create_node(
            nil::gate::Core& core,
            std::tuple<nil::gate::MutableEdge<typename C::type>*...> controls,
            std::vector<std::uint64_t> inputs,
            std::vector<std::uint64_t> outputs,
            Args&&... args
        )
        {
            if constexpr (sizeof...(input_indices) > 0 || sizeof...(control_indices))
            {
                const auto result = core.node<T>(
                    {inputs[input_indices]..., std::get<control_indices>(controls)...},
                    args...
                );
                (this->internal_edges
                     .emplace(outputs[output_indices], std::get<output_indices>(result)),
                 ...);
            }
            else
            {
                const auto result = core.node<T>({}, args...);
                (this->internal_edges
                     .emplace(outputs[output_indices], std::get<output_indices>(result)),
                 ...);
            }
        }

        template <typename T, typename... Inputs>
        void add_inputs(
            nil::nedit::proto::NodeInfo& info,
            nil::utils::traits::types<Inputs...> /* unused */
        )
        {
            (info.add_inputs(type_to_pin_index.at(nil::utils::traits::type<Inputs>::value)), ...);
        }

        template <typename T, typename... Outputs>
        void add_outputs(
            nil::nedit::proto::NodeInfo& info,
            nil::utils::traits::types<Outputs...> /* unused */
        )
        {
            (info.add_outputs(type_to_pin_index.at(nil::utils::traits::type<Outputs>::value)), ...);
        }

        template <typename T, typename... Controls>
        void add_controls(nil::nedit::proto::NodeInfo& info)
        {
            (add_control<T>(info, nil::utils::traits::types<Controls>()), ...);
        }

        template <typename T>
        void add_control(
            nil::nedit::proto::NodeInfo& info,
            nil::utils::traits::types<Control<float>> /* unused */
        )
        {
            auto* s = info.add_controls()->mutable_slider();
            s->set_value(1.0f);
            s->set_min(0.0f);
            s->set_max(2.0f);
        }

        template <typename T>
        void add_control(
            nil::nedit::proto::NodeInfo& info,
            nil::utils::traits::types<Control<std::string>> /* unused */
        )
        {
            info.add_controls() //
                ->mutable_text()
                ->set_value("<sample text>");
        }
    };
}

nil::cli::OptionInfo EXT::options() const
{
    return nil::cli::Builder()
        .flag("help", {.skey = 'h', .msg = "this help"})
        .number("port", {.skey = 'p', .msg = "port", .fallback = 1101})
        .build();
}

int EXT::run(const nil::cli::Options& options) const
{
    if (options.flag("help"))
    {
        options.help(std::cout);
        return 0;
    }

    App app;
    app //
        .add_pin<bool>("bool", {0.0f, 1.0f, 0.0f, 1.0f})
        .add_pin<int>("int", {1.0f, 0.0f, 0.0f, 1.0f})
        .add_pin<float>("float", {0.0f, 0.0f, 1.0f, 1.0f})
        .add_pin<std::string>("string", {0.0f, 1.0f, 1.0f, 1.0f})
        .add_node<Input<float>, Control<float>>("Input_f<1.0f>", "f", 1.0f)
        .add_node<Input<std::string>, Control<std::string>>(
            "Input_s<sample_text>",
            "s",
            "sample_text"
        )
        .add_node<Input<bool>>("Input_b<false>", "b", false)
        .add_node<Input<bool>>("Input_b<true>", "b", true)
        .add_node<Input<int>>("Input_i<5>", "i", 5)
        .add_node<Input<int>>("Input_i<10>", "i", 10)
        .add_node<Inverter>("Inverter")
        .add_node<Add>("Add")
        .add_node<Mul>("Mul")
        .add_node<Consume<int>>("Consume<i>")
        .add_node<Consume<float>>("Consume<f>");

    nil::gate::Core core;

    nil::service::TypedService client(                         //
        nil::service::make_service<nil::service::tcp::Client>( //
            {.host = "127.0.0.1", .port = std::uint16_t(options.number("port"))}
        )
    );

    client.on_connect(
        [&](const std::string& id)
        {
            for (const auto& pin : app.pins)
            {
                client.send(id, nil::nedit::proto::message_type::PinInfo, pin);
            }
            for (const auto& node : app.nodes)
            {
                client.send(id, nil::nedit::proto::message_type::NodeInfo, node);
            }
            client.send(id, nil::nedit::proto::message_type::Freeze, std::string());
        }
    );

    std::unordered_map<std::uint64_t, nil::gate::IEdge*> control_edges;
    client.on_message(
        nil::nedit::proto::message_type::ControlUpdate,
        [&core, &control_edges](const std::string&, const nil::nedit::proto::ControlUpdate& message)
        {
            if (message.has_f())
            {
                auto* edge = control_edges[message.id()];
                if (edge != nullptr)
                {
                    static_cast<nil::gate::MutableEdge<float>*>(edge)->set_value(message.f());
                    core.run();
                }
            }
        }
    );

    client.on_message(
        nil::nedit::proto::message_type::GraphUpdate,
        [&core, &app, &control_edges](const std::string&, const nil::nedit::proto::Graph& graph)
        {
            core = {};
            control_edges = {};

            std::unordered_map<std::uint64_t, nil::gate::IEdge*> edges;
            std::vector<nil::gate::builder::Node> nodes;
            for (const auto& node : graph.nodes())
            {
                nodes.push_back({
                    node.type(),
                    {node.inputs().begin(), node.inputs().end()},
                    {node.outputs().begin(), node.outputs().end()},
                    {node.controls().begin(), node.controls().end()} //
                });
            }
            for (const auto& node : nil::gate::builder::sort_by_score(nodes))
            {
                switch (node.type)
                {
                    case 0u:
                    {
                        const auto control = core.edge<float>();
                        control->set_value(1.0f);
                        control_edges.emplace(node.controls[0], control);
                        const auto [result] = core.node<InputWithControl<float>>({control}, "f");
                        edges.emplace(node.outputs[0], result);
                        break;
                    }
                    case 1u:
                    {
                        const auto control = core.edge<std::string>();
                        control->set_value("sample_text");
                        control_edges.emplace(node.controls[0], control);
                        const auto [result] = core.node<InputWithControl<std::string>>({}, "s");
                        edges.emplace(node.outputs[0], result);
                        break;
                    }
                    case 2u:
                    {
                        const auto [result] = core.node<Input<bool>>({}, "b", false);
                        edges.emplace(node.outputs[0], result);
                        break;
                    }
                    case 3u:
                    {
                        const auto [result] = core.node<Input<bool>>({}, "b", true);
                        edges.emplace(node.outputs[0], result);
                        break;
                    }
                    case 4u:
                    {
                        const auto [result] = core.node<Input<int>>({}, "i", 5);
                        edges.emplace(node.outputs[0], result);
                        break;
                    }
                    case 5u:
                    {
                        const auto [result] = core.node<Input<int>>({}, "i", 10);
                        edges.emplace(node.outputs[0], result);
                        break;
                    }
                    case 6u:
                    {
                        const auto [result] = core.node<Inverter>({
                            static_cast<nil::gate::ReadOnlyEdge<bool>*>(edges[node.inputs[0]]),
                            static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[node.inputs[1]]) //
                        });
                        edges.emplace(node.outputs[0], result);
                        break;
                    }
                    case 7u:
                    {
                        const auto [result] = core.node<Add>({
                            static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[node.inputs[0]]),
                            static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[node.inputs[1]]) //
                        });
                        edges.emplace(node.outputs[0], result);
                        break;
                    }
                    case 8u:
                    {
                        const auto [result] = core.node<Mul>({
                            static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[node.inputs[0]]),
                            static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[node.inputs[1]]) //
                        });
                        edges.emplace(node.outputs[0], result);
                        break;
                    }
                    case 9u:
                    {
                        core.node<Consume<int>>(
                            {static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[node.inputs[0]])}
                        );
                        break;
                    }
                    case 10u:
                    {
                        core.node<Consume<float>>(
                            {static_cast<nil::gate::ReadOnlyEdge<float>*>(edges[node.inputs[0]])}
                        );
                        break;
                    }
                }
            }

            try
            {
                core.run();
            }
            catch (const std::exception& ex)
            {
                std::cout << ex.what() << std::endl;
            }
        }
    );

    client.run();
    return 0;
}
