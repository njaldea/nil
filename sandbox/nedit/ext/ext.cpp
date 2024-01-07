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

        static void to_message(nil::nedit::proto::NodeInfo& info);
    };

    template <>
    void Control<bool>::to_message(nil::nedit::proto::NodeInfo& info)
    {
        auto* s = info.add_controls()->mutable_toggle();
        s->set_value(false);
    }

    template <>
    void Control<int>::to_message(nil::nedit::proto::NodeInfo& info)
    {
        auto* s = info.add_controls()->mutable_spinbox();
        s->set_value(0);
        s->set_min(0);
        s->set_max(10);
    }

    template <>
    void Control<float>::to_message(nil::nedit::proto::NodeInfo& info)
    {
        auto* s = info.add_controls()->mutable_slider();
        s->set_value(1.0f);
        s->set_min(0.0f);
        s->set_max(2.0f);
    }

    template <>
    void Control<std::string>::to_message(nil::nedit::proto::NodeInfo& info)
    {
        auto* s = info.add_controls()->mutable_text();
        s->set_value("<sample text>");
    }

    template <typename T>
    struct Input
    {
        explicit Input(std::string init_name)
            : name(std::move(init_name))
        {
        }

        std::tuple<T> operator()(const T& value)
        {
            std::cout << "Input_" << name << '<' << value << '>' << std::endl;
            return {value};
        }

        std::string name;
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
        void operator()(const T& v)
        {
            std::cout << "Consume: " << v << std::endl;
        }
    };

    // [TODO]
    //  -  find a way to abstract out the message type
    //  -  create a library. if possible, gate can take ownership of this, only if the message
    //  handling is abstracted out.
    class App final
    {
        using IDs = std::vector<std::uint64_t>;
        using Factory = std::function<void(nil::gate::Core&, const IDs&, const IDs&, const IDs&)>;

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
        };

    public:
        App() = default;
        ~App() = default;
        App(const App&) = delete;
        App(App&&) = delete;
        App& operator=(const App&) = delete;
        App& operator=(App&&) = delete;

        template <typename T>
        App& add_pin(std::string label, std::array<float, 4> color)
        {
            type_to_pin_index.emplace(nil::utils::traits::identity_v<T>, pins.size());
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
        App& add_node(std::string label, const Args&... args)
        {
            using input_t = typename nil::gate::detail::traits<std::decay_t<T>>::i;
            using output_t = typename nil::gate::detail::traits<std::decay_t<T>>::o;
            nodes.push_back(
                [&]()
                {
                    nil::nedit::proto::NodeInfo info;
                    add_inputs<T>(
                        info,
                        typename input_t::type(),
                        std::make_index_sequence<input_t::size - sizeof...(Controls)>()
                    );
                    add_outputs<T>(info, typename output_t::type());
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
                        this->create_control_edges<T, Controls...>(
                            core,
                            controls,
                            std::make_index_sequence<sizeof...(Controls)>()
                        );
                    }
                    constexpr auto input_count                //
                        = input_t::size < sizeof...(Controls) //
                        ? 0u
                        : input_t::size - sizeof...(Controls);
                    this->create_node<T>(
                        core,
                        inputs,
                        outputs,
                        controls,
                        std::make_index_sequence<input_count>(),
                        typename output_t::make_index_sequence(),
                        std::make_index_sequence<sizeof...(Controls)>(),
                        args...
                    );
                }
            );
            return *this;
        }

        void reset()
        {
            control_edges.clear();
            internal_edges.clear();
        }

        void create_node(
            nil::gate::Core& core,
            std::uint64_t type,
            const std::vector<std::uint64_t>& inputs,
            const std::vector<std::uint64_t>& outputs,
            const std::vector<std::uint64_t>& controls
        )
        {
            node_factories[type](core, inputs, outputs, controls);
        }

        std::vector<nil::nedit::proto::PinInfo> pins;   // messages
        std::vector<nil::nedit::proto::NodeInfo> nodes; // messages
        std::unordered_map<std::uint64_t, RelaxedEdge> control_edges;

    private:
        std::vector<Factory> node_factories;
        std::unordered_map<std::uint64_t, RelaxedEdge> internal_edges;
        std::unordered_map<const void*, std::uint64_t> type_to_pin_index;

        template <typename T, typename... C, std::size_t... indices>
        void create_control_edges(
            nil::gate::Core& core,
            const std::vector<std::uint64_t>& ids,
            std::index_sequence<indices...> /* unused */ //
        )
        {
            if constexpr (sizeof...(C) > 0)
            {
                const auto mutable_edges = core.edges<typename C::type...>();
                (create_control_edge<T, C>(std::get<indices>(mutable_edges), ids[indices]), ...);
            }
        }

        template <typename T, typename C>
        void create_control_edge(nil::gate::MutableEdge<typename C::type>* edge, std::uint64_t id)
        {
            edge->set_value(C::value());
            control_edges.emplace(id, RelaxedEdge{edge});
        }

        template <
            typename T,
            std::size_t... input_indices,
            std::size_t... output_indices,
            std::size_t... control_indices,
            typename... Args>
        void create_node(
            nil::gate::Core& core,
            const std::vector<std::uint64_t>& inputs,
            const std::vector<std::uint64_t>& outputs,
            const std::vector<std::uint64_t>& controls,
            std::index_sequence<input_indices...> /* unused */,
            std::index_sequence<output_indices...> /* unused */,
            std::index_sequence<control_indices...> /* unused */,
            const Args&... args
        )
        {
            const auto result = core.node<T>(
                {
                    internal_edges[inputs[input_indices]]...,
                    control_edges[controls[control_indices]]... //
                },
                args...
            );
            if constexpr (sizeof...(output_indices) > 0u)
            {
                (                                                     //
                    this->internal_edges.emplace(                     //
                        outputs[output_indices],                      //
                        RelaxedEdge{std::get<output_indices>(result)} //
                    ),                                                //
                    ...                                               //
                );
            }
        }

        template <typename T, typename... Inputs, std::size_t... indices>
        void add_inputs(
            nil::nedit::proto::NodeInfo& info,
            nil::utils::traits::types<Inputs...> /* unused */,
            std::index_sequence<indices...> /* unused */
        )
        {
            ( //
                info.add_inputs(type_to_pin_index.at(
                    nil::utils::traits::identity_v<
                        std::tuple_element_t<indices, std::tuple<Inputs...>>> //
                )),
                ... //
            );
        }

        template <typename T, typename... Outputs>
        void add_outputs(
            nil::nedit::proto::NodeInfo& info,
            nil::utils::traits::types<Outputs...> /* unused */
        )
        {
            (info.add_outputs(type_to_pin_index.at(nil::utils::traits::identity_v<Outputs>)), ...);
        }

        template <typename T, typename... Controls>
        void add_controls(nil::nedit::proto::NodeInfo& info)
        {
            (Controls::to_message(info), ...);
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
        .add_node<Input<bool>, Control<bool>>("Input_b", "b")
        .add_node<Input<int>, Control<int>>("Input_i", "i")
        .add_node<Input<float>, Control<float>>("Input_f", "f")
        .add_node<Input<std::string>, Control<std::string>>("Input_s", "s")
        .add_node<Consume<bool>>("Consume<b>")
        .add_node<Consume<int>>("Consume<i>")
        .add_node<Consume<float>>("Consume<f>")
        .add_node<Consume<std::string>>("Consume<s>")
        .add_node<Inverter>("Inverter")
        .add_node<Add>("Add")
        .add_node<Mul>("Mul");

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

    client.on_message(
        nil::nedit::proto::message_type::ControlUpdate,
        [&core, &app](const std::string&, const nil::nedit::proto::ControlUpdate& message)
        {
            const auto it = app.control_edges.find(message.id());
            if (it != app.control_edges.end())
            {
                auto* edge = it->second.edge;
                if (message.has_b())
                {
                    static_cast<nil::gate::MutableEdge<bool>*>(edge) //
                        ->set_value(message.b());
                }
                else if (message.has_i())
                {
                    static_cast<nil::gate::MutableEdge<std::int32_t>*>(edge) //
                        ->set_value(message.i());
                }
                else if (message.has_f())
                {
                    static_cast<nil::gate::MutableEdge<float>*>(edge) //
                        ->set_value(message.f());
                }
                else if (message.has_s())
                {
                    static_cast<nil::gate::MutableEdge<std::string>*>(edge) //
                        ->set_value(message.s());
                }
                core.run();
            }
        }
    );

    client.on_message(
        nil::nedit::proto::message_type::GraphUpdate,
        [&core, &app](const std::string&, const nil::nedit::proto::Graph& graph)
        {
            core = {};
            app.reset();

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
                app.create_node(core, node.type, node.inputs, node.outputs, node.controls);
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
