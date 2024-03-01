#include "ext.hpp"
#include "../codec.hpp"
#include "app/install.hpp"

#include <nil/dev.hpp>
#include <nil/service/TypedHandler.hpp>
#include <nil/service/tcp/Server.hpp>
#include <nil/utils/traits/identity.hpp>

#include <gen/nedit/messages/control_update.pb.h>
#include <gen/nedit/messages/graph_update.pb.h>
#include <gen/nedit/messages/node_state.pb.h>
#include <gen/nedit/messages/state.pb.h>
#include <gen/nedit/messages/type.pb.h>

#include <boost/asio.hpp>

#include <array>
#include <iostream>
#include <sstream>
#include <thread>

nil::cli::OptionInfo EXT::options() const
{
    return nil::cli::Builder()
        .flag("help", {.skey = 'h', .msg = "this help"})
        .number("port", {.skey = 'p', .msg = "port", .fallback = 1101})
        .build();
}

//  TODO:
//      Create a better "priority" executor/debouncer. right now, i have to push everything in.
//  ideally:
//   -  some older tasks can be removed and be overridden by newer tasks
//   -  some task can be registered once and can be "retriggered" easily
//       -  this is what posts do. caller have to check if it is already available befor pushing
//      Maybe possible to add this to service library
struct Executor
{
    std::vector<std::function<void()>> tasks;
    std::vector<std::function<void()>> posts;

    std::mutex mutex;
    boost::asio::io_context context;

    void run()
    {
        const auto [t, p] = [this]()
        {
            const std::lock_guard _(mutex);
            return std::make_tuple(std::exchange(tasks, {}), std::exchange(posts, {}));
        }();
        for (const auto& tt : t)
        {
            tt();
        }
        for (const auto& pp : p)
        {
            pp();
        }
        const std::lock_guard _(mutex);
        if (!tasks.empty() || !posts.empty())
        {
            boost::asio::post(context, [this]() { run(); });
        }
    }
};

ext::GraphState make_state(nil::service::IService& service, Executor& executor)
{
    ext::GraphState state;
    state.core = {};
    state.core.set_commit( //
        [&executor](auto& core) { boost::asio::post(executor.context, [&]() { core.run(); }); }
    );
    state.activate = [&service](std::uint64_t id)
    {
        nil::nedit::proto::NodeState message;
        message.set_id(id);
        message.set_active(true);
        service.publish(nil::nedit::proto::message_type::NodeState, message);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };

    state.deactivate = [&service](std::uint64_t id)
    {
        nil::nedit::proto::NodeState message;
        message.set_id(id);
        message.set_active(false);
        service.publish(nil::nedit::proto::message_type::NodeState, message);
    };

    return state;
}

template <typename T>
auto make_control_update(ext::GraphState& graph_state, Executor& executor)
{
    return [&graph_state, &executor](const std::string&, const T& message)
    {
        const std::lock_guard _(executor.mutex);
        executor.tasks.emplace_back(
            [&graph_state, &executor, message]()
            {
                const auto it = graph_state.control_edges.find(message.id());
                if (it != graph_state.control_edges.end())
                {
                    it->second.set_value(message.value());
                    if (executor.posts.empty())
                    {
                        executor.posts.emplace_back([&]() { graph_state.core.run(); });
                    }
                }
            }
        );
        boost::asio::post(executor.context, [&]() { executor.run(); });
    };
}

int EXT::run(const nil::cli::Options& options) const
{
    if (options.flag("help"))
    {
        options.help(std::cout);
        return 0;
    }

    Executor executor;

    ext::AppState app_state;
    ext::App app(app_state);

    nil::service::tcp::Server service({.port = std::uint16_t(options.number("port"))});

    ext::GraphState graph_state = make_state(service, executor);

    ext::install(
        app,
        [&graph_state, &executor](std::function<void()> cb)
        { boost::asio::post(executor.context, [cb = std::move(cb)]() { cb(); }); }
    );
    const std::string types = app_state.info.types().SerializeAsString();

    namespace proto = nil::nedit::proto;
    service.on_connect( //
        [&service, &app_state](const std::string& id)
        { service.send(id, proto::message_type::State, app_state.info); }
    );

    service.on_message(                                                //
        nil::service::TypedHandler<proto::message_type::MessageType>() //
            .add(
                proto::message_type::ControlUpdateB,
                make_control_update<proto::ControlUpdateB>(graph_state, executor)
            )
            .add(
                proto::message_type::ControlUpdateI,
                make_control_update<proto::ControlUpdateI>(graph_state, executor)
            )
            .add(
                proto::message_type::ControlUpdateF,
                make_control_update<proto::ControlUpdateF>(graph_state, executor)
            )
            .add(
                proto::message_type::ControlUpdateS,
                make_control_update<proto::ControlUpdateS>(graph_state, executor)
            )
            .add(proto::message_type::Play, []() { nil::log(); })
            .add(proto::message_type::Pause, []() { nil::log(); })
            .add(
                proto::message_type::State,
                [&graph_state, &app_state, &types, &service, &executor](
                    const std::string& id,
                    const proto::State& info
                )
                {
                    const std::lock_guard _(executor.mutex);
                    executor.tasks.emplace_back(
                        [&graph_state, &app_state, &types, &service, &executor, id, info]()
                        {
                            if (info.types().SerializeAsString() != types)
                            {
                                nil::log();
                                std::cout << "state is not compatible to types" << std::endl;
                                return;
                            }

                            app_state.info = info;

                            graph_state = make_state(service, executor);

                            std::unordered_map<std::uint64_t, std::uint64_t> i_to_o;
                            for (const auto& link : info.graph().links())
                            {
                                i_to_o.emplace(link.output(), link.input());
                            }

                            std::vector<ext::NodeData> nodes;
                            for (const auto& node : info.graph().nodes())
                            {
                                nodes.push_back({
                                    .id = node.id(),
                                    .type = node.type(),
                                    .inputs = {node.inputs().begin(), node.inputs().end()},
                                    .outputs = {node.outputs().begin(), node.outputs().end()},
                                    .controls = {node.controls().begin(), node.controls().end()} //
                                });

                                // convert id of input from id of the pin to the id of the port it
                                // is connected to.
                                for (auto& i : nodes.back().inputs)
                                {
                                    if (i_to_o.contains(i))
                                    {
                                        i = i_to_o.at(i);
                                    }
                                    else
                                    {
                                        std::cout << "incomplete input... not supported"
                                                  << std::endl;
                                        return;
                                    }
                                }
                            }

                            for (const auto& node : ext::sort_by_score(nodes))
                            {
                                auto& factory = app_state.node_factories[node.type];
                                factory(
                                    graph_state,
                                    node.id,
                                    node.inputs,
                                    node.outputs,
                                    node.controls
                                );
                            }

                            service.send(id, proto::message_type::State, app_state.info);
                        }
                    );
                    boost::asio::post(executor.context, [&]() { executor.run(); });
                }
            )
            .add(
                proto::message_type::Run,
                [&graph_state, &executor]()
                {
                    const std::lock_guard _(executor.mutex);
                    if (executor.posts.empty())
                    {
                        executor.posts.emplace_back(
                            [&]()
                            {
                                try
                                {
                                    graph_state.core.run();
                                }
                                catch (const std::exception& ex)
                                {
                                    std::cout << ex.what() << std::endl;
                                }
                            }
                        );
                        boost::asio::post(executor.context, [&]() { executor.run(); });
                    }
                }
            )
    );

    std::thread ex_thread(
        [&executor]()
        {
            const auto guard = boost::asio::make_work_guard(executor.context);
            executor.context.run();
        }
    );

    service.run();
    ex_thread.join();
    return 0;
}
