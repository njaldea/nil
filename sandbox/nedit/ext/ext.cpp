#include "ext.hpp"
#include "../codec.hpp"

#include <nil/service.hpp>
#include <nil/utils/traits/identity.hpp>

#include "app/install.hpp"

#include <array>
#include <iostream>
#include <sstream>
#include <thread>

#include <gen/nedit/messages/control_update.pb.h>
#include <gen/nedit/messages/graph_update.pb.h>
#include <gen/nedit/messages/node_state.pb.h>
#include <gen/nedit/messages/state.pb.h>
#include <gen/nedit/messages/type.pb.h>

#include <boost/asio.hpp>

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
            context.post([this]() { run(); });
        }
    }
};

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
    ext::install(app);
    const std::string types = app_state.info.types().SerializeAsString();

    nil::service::TypedService service(                        //
        nil::service::make_service<nil::service::tcp::Server>( //
            {.port = std::uint16_t(options.number("port"))}
        )
    );

    const auto make_state = [&service]()
    {
        ext::GraphState state;
        state.core = std::make_unique<nil::gate::Core>();

        state.activate = [&service](std::uint64_t id)
        {
            nil::nedit::proto::NodeState message;
            message.set_id(id);
            message.set_active(true);
            service.publish(
                nil::nedit::proto::message_type::NodeState,
                message.SerializeAsString()
            );
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        };

        state.deactivate = [&service](std::uint64_t id)
        {
            nil::nedit::proto::NodeState message;
            message.set_id(id);
            message.set_active(false);
            service.publish(
                nil::nedit::proto::message_type::NodeState,
                message.SerializeAsString()
            );
        };

        return state;
    };
    ext::GraphState graph_state = make_state();

    const auto send_state //
        = [&service, &app_state](const std::string& id)
    { service.send(id, nil::nedit::proto::message_type::State, app_state.info); };

    service.on_connect(send_state);

    const auto control_update = [&graph_state, &executor](const auto& message)
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
                        executor.posts.emplace_back([&]() { graph_state.core->run(); });
                    }
                }
            }
        );
        executor.context.post([&]() { executor.run(); });
    };

    service.on_message(
        nil::nedit::proto::message_type::ControlUpdateB,
        [&control_update](const std::string&, const nil::nedit::proto::ControlUpdateB& message)
        { control_update(message); }
    );

    service.on_message(
        nil::nedit::proto::message_type::ControlUpdateI,
        [&control_update](const std::string&, const nil::nedit::proto::ControlUpdateI& message)
        { control_update(message); }
    );

    service.on_message(
        nil::nedit::proto::message_type::ControlUpdateF,
        [&control_update](const std::string&, const nil::nedit::proto::ControlUpdateF& message)
        { control_update(message); }
    );

    service.on_message(
        nil::nedit::proto::message_type::ControlUpdateS,
        [&control_update](const std::string&, const nil::nedit::proto::ControlUpdateS& message)
        { control_update(message); }
    );

    const auto load_state                                 //
        = [&graph_state, &app_state, &types, &make_state] //
        (const nil::nedit::proto::State& info)
    {
        if (info.types().SerializeAsString() != types)
        {
            return false;
        }

        app_state.info = info;

        graph_state = make_state();

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

            // convert id of input from id of the pin to the id of the port it is connected to.
            for (auto& i : nodes.back().inputs)
            {
                if (i_to_o.contains(i))
                {
                    i = i_to_o.at(i);
                }
                else
                {
                    std::cout << "incomplete input... not supported" << std::endl;
                    return true;
                }
            }
        }

        for (const auto& node : ext::sort_by_score(nodes))
        {
            auto& factory = app_state.node_factories[node.type];
            factory(graph_state, node.id, node.inputs, node.outputs, node.controls);
        }

        return true;
    };

    service.on_message(
        nil::nedit::proto::message_type::State,
        [&](const std::string& id, const nil::nedit::proto::State& info)
        {
            const std::lock_guard _(executor.mutex);
            executor.tasks.emplace_back(
                [&, info]()
                {
                    if (load_state(info))
                    {
                        send_state(id);
                    }
                    else
                    {
                        std::cout << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__
                                  << std::endl;
                        std::cout << "state is not compatible to types" << std::endl;
                    }
                }
            );
            executor.context.post([&]() { executor.run(); });
        }
    );

    service.on_message(
        nil::nedit::proto::message_type::Run,
        [&]()
        {
            const std::lock_guard _(executor.mutex);
            if (executor.posts.empty())
            {
                executor.posts.emplace_back(
                    [&]()
                    {
                        try
                        {
                            graph_state.core->run();
                        }
                        catch (const std::exception& ex)
                        {
                            std::cout << ex.what() << std::endl;
                        }
                    }
                );
                executor.context.post([&]() { executor.run(); });
            }
        }
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
