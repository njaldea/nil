#include "ext.hpp"
#include "../codec.hpp"

#include <nil/gate.hpp>
#include <nil/service.hpp>
#include <nil/utils/traits/type.hpp>

#include <iostream>
#include <sstream>

#include <gen/nedit/messages/graph_update.pb.h>
#include <gen/nedit/messages/node_info.pb.h>
#include <gen/nedit/messages/pin_info.pb.h>
#include <gen/nedit/messages/type.pb.h>

namespace
{
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

    struct Consume
    {
        void operator()(int v)
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
    public:
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

        template <typename T, typename... Args>
        App& add_node(std::string label, Args&&... args)
        {
            nodes.push_back(
                [&]()
                {
                    nil::nedit::proto::NodeInfo info;
                    add_inputs<T>(
                        info,
                        typename nil::gate::detail::traits<std::decay_t<T>>::i::type()
                    );
                    add_outputs<T>(
                        info,
                        typename nil::gate::detail::traits<std::decay_t<T>>::o::type()
                    );
                    info.set_label(std::move(label));
                    return info;
                }()
            );
            builder.add_node_type<T>(std::forward<Args>(args)...);
            return *this;
        }

        std::vector<nil::nedit::proto::PinInfo> pins;
        std::vector<nil::nedit::proto::NodeInfo> nodes;
        nil::gate::CoreBuilder builder;

    private:
        std::unordered_map<const void*, std::uint64_t> type_to_pin_index;

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

    auto app = //
        App()  //
            .add_pin<bool>("bool", {0.0f, 1.0f, 0.0f, 1.0f})
            .add_pin<int>("int", {1.0f, 0.0f, 0.0f, 1.0f})
            .add_node<Input<bool>>("Input_b<false>", "b", false)
            .add_node<Input<bool>>("Input_b<true>", "b", true)
            .add_node<Input<int>>("Input_i<5>", "i", 5)
            .add_node<Input<int>>("Input_i<10>", "i", 10)
            .add_node<Inverter>("Inverter")
            .add_node<Add>("Add")
            .add_node<Mul>("Mul")
            .add_node<Consume>("Consume");

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
        nil::nedit::proto::message_type::GraphUpdate,
        [&core, &app](const std::string&, const nil::nedit::proto::Graph& graph)
        {
            auto builder = app.builder;
            for (const auto& node : graph.nodes())
            {
                builder.add_node(
                    node.type(),
                    {node.inputs().begin(), node.inputs().end()},
                    {node.outputs().begin(), node.outputs().end()}
                );
            }

            core = builder.build();
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
