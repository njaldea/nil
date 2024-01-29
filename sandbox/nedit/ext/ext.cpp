#include "ext.hpp"
#include "../codec.hpp"

#include <nil/service.hpp>
#include <nil/utils/traits/identity.hpp>

#include "app/install.hpp"

#include <array>
#include <iostream>
#include <sstream>

#include <gen/nedit/messages/control_update.pb.h>
#include <gen/nedit/messages/graph_update.pb.h>
#include <gen/nedit/messages/type.pb.h>

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

    ext::AppState app_state;
    ext::GraphState graph_state;
    ext::App app(app_state);
    ext::install(app);

    const std::string types = app_state.info.types().SerializeAsString();

    nil::service::TypedService client(                         //
        nil::service::make_service<nil::service::tcp::Client>( //
            {.host = "127.0.0.1", .port = std::uint16_t(options.number("port"))}
        )
    );

    const auto send_state //
        = [&client, &app_state](const std::string& id)
    { client.send(id, nil::nedit::proto::message_type::State, app_state.info); };

    client.on_connect(send_state);

    client.on_message(
        nil::nedit::proto::message_type::ControlUpdate,
        [&graph_state](const std::string&, const nil::nedit::proto::ControlUpdate& message)
        {
            const auto it = graph_state.control_edges.find(message.id());
            if (it != graph_state.control_edges.end())
            {
                switch (message.value_case())
                {
                    case nil::nedit::proto::ControlUpdate::ValueCase::kB:
                    {
                        it->second.set_value(message.b());
                        break;
                    }
                    case nil::nedit::proto::ControlUpdate::ValueCase::kI:
                    {
                        it->second.set_value(message.i());
                        break;
                    }
                    case nil::nedit::proto::ControlUpdate::ValueCase::kF:
                    {
                        it->second.set_value(message.f());
                        break;
                    }
                    case nil::nedit::proto::ControlUpdate::ValueCase::kS:
                    {
                        it->second.set_value(message.s());
                        break;
                    }
                    default:
                        return;
                }
                graph_state.core.run();
            }
        }
    );

    const auto load_state                    //
        = [&graph_state, &app_state, &types] //
        (const nil::nedit::proto::State& info)
    {
        std::string state = info.types().SerializeAsString();

        if (state != types)
        {
            return false;
        }

        app_state.info = info;

        graph_state = {};

        std::vector<ext::NodeData> nodes;
        nodes.reserve(std::size_t(info.graph().nodes().size()));
        for (const auto& node : info.graph().nodes())
        {
            nodes.push_back({
                .type = node.type(),
                .inputs = {node.inputs().begin(), node.inputs().end()},
                .outputs = {node.outputs().begin(), node.outputs().end()},
                .controls = {node.controls().begin(), node.controls().end()} //
            });
        }

        for (const auto& node : ext::sort_by_score(nodes))
        {
            app_state
                .node_factories[node.type](graph_state, node.inputs, node.outputs, node.controls);
        }

        try
        {
            graph_state.core.run();
        }
        catch (const std::exception& ex)
        {
            std::cout << ex.what() << std::endl;
        }

        return true;
    };

    client.on_message(
        nil::nedit::proto::message_type::Load,
        [&](const std::string& id, const nil::nedit::proto::State& info)
        {
            if (load_state(info))
            {
                send_state(id);
            }
            else
            {
                std::cout << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << std::endl;
                std::cout << "state is not compatible to types" << std::endl;
            }
        }
    );

    client.on_message(
        nil::nedit::proto::message_type::Update,
        [&](const std::string&, const nil::nedit::proto::State& info)
        {
            if (!load_state(info))
            {
                std::cout << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << std::endl;
                std::cout << "state is not compatible to types" << std::endl;
                // this should be dead code.
            }
        }
    );

    client.run();
    return 0;
}
