#include "ext.hpp"
#include "../codec.hpp" // IWYU pragma: keep
#include "app/install.hpp"
#include "app/sort_nodes.hpp"

#include <nil/dev.hpp>
#include <nil/service/TypedHandler.hpp>
#include <nil/service/tcp/Server.hpp>
#include <nil/utils/traits/identity.hpp>

#include <gen/nedit/messages/control_update.pb.h>
#include <gen/nedit/messages/node_state.pb.h>
#include <gen/nedit/messages/state.pb.h>
#include <gen/nedit/messages/type.pb.h>

#include <boost/asio.hpp>

#include <array>
#include <iostream>
#include <thread>

nil::cli::OptionInfo EXT::options() const
{
    return nil::cli::Builder()
        .flag("help", {.skey = 'h', .msg = "this help"})
        .number("port", {.skey = 'p', .msg = "port", .fallback = 1101})
        .build();
}

enum class EPriority : std::uint8_t
{
    State,
    ControlUpdateB,
    ControlUpdateI,
    ControlUpdateF,
    ControlUpdateS,
    Run
};

// first layer of io_context is to debounce changes. second layer is for gate execution.
// this is to allow cancellation in cases where there is a asnyc node that has posted changes.
class Executor
{
public:
    void push(std::tuple<EPriority, std::uint64_t> key, std::function<void()> exec)
    {
        const std::lock_guard _(mutex);
        execs[key] = std::move(exec);
        if (!flushing)
        {
            flushing = true;
            boost::asio::post(*context, [this]() { flush(); });
        }
    }

    // should i debounce this?
    void post(const void* id, float time, std::function<void()> cb)
    {
        const std::lock_guard _(mutex);
        boost::asio::post(
            *context,
            [this, id, time, cb = std::move(cb)]()
            {
                auto& timer = timers[id];

                if (timer)
                {
                    timer->cancel();
                }
                else
                {
                    timer = std::make_unique<boost::asio::deadline_timer>(
                        *context,
                        boost::posix_time::microsec(int(time * 1000000))
                    );
                }

                timer->async_wait(
                    [this, id, cb](const boost::system::error_code& er)
                    {
                        if (er)
                        {
                            if (er != boost::asio::error::operation_aborted)
                            {
                                std::cout << er.what() << std::endl;
                            }
                            return;
                        }
                        timers.erase(id);
                        cb();
                    }
                );
            }
        );
    }

    void reset()
    {
        if (context && thread)
        {
            cancelled = true;
            context->stop();
            thread->join();
            cancelled = false;
            flushing = false;
        }
        const std::lock_guard _(mutex);
        context = std::make_unique<boost::asio::io_context>();
        thread = std::make_unique<std::thread>(
            [this]()
            {
                const auto guard = boost::asio::make_work_guard(*context);
                context->run();
            }
        );
    }

private:
    std::mutex mutex;
    std::map<std::tuple<EPriority, std::uint64_t>, std::function<void()>> execs;
    std::unique_ptr<boost::asio::io_context> context;
    std::unique_ptr<std::thread> thread;
    std::atomic_bool flushing = false;
    std::atomic_bool cancelled = false;

    std::unordered_map<const void*, std::unique_ptr<boost::asio::deadline_timer>> timers;

    void flush()
    {
        const auto ss = [this]()
        {
            const std::lock_guard _(mutex);
            flushing = false;
            return std::exchange(execs, {});
        }();
        for (const auto& [key, value] : ss)
        {
            if (cancelled)
            {
                return;
            }
            if (value)
            {
                value();
            }
        }
    }
};

ext::GraphState make_state(nil::service::IService& service, Executor& executor)
{
    ext::GraphState state;
    state.core = {};
    state.core.set_commit( //
        [&executor, is_paused = state.paused](auto& core)
        {
            executor.push(
                {EPriority::Run, 0},
                [&core, is_paused]()
                {
                    try
                    {
                        if (!*is_paused)
                        {
                            core.run();
                        }
                    }
                    catch (const std::exception& ex)
                    {
                        std::cout << ex.what() << std::endl;
                    }
                }
            );
        }
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

    state.post = [&executor](const void* id, float time, std::function<void()> cb)
    { executor.post(id, time, std::move(cb)); };

    return state;
}

template <typename T>
auto make_control_update(ext::GraphState& graph_state, Executor& executor, EPriority priority)
{
    return [&graph_state, &executor, priority](const std::string&, const T& message)
    {
        executor.push(
            {priority, message.id()},
            [&graph_state, message]()
            {
                const auto it = graph_state.control_edges.find(message.id());
                if (it != graph_state.control_edges.end())
                {
                    it->second.set_value(message.value());
                    graph_state.core.commit();
                }
            }
        );
    };
}

int EXT::run(const nil::cli::Options& options) const
{
    if (options.flag("help"))
    {
        options.help(std::cout);
        return 0;
    }

    ext::AppState app_state;
    ext::App app(app_state);

    nil::service::tcp::Server service({.port = std::uint16_t(options.number("port"))});

    Executor executor;
    executor.reset();

    ext::GraphState graph_state = make_state(service, executor);

    ext::install(app);
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
                make_control_update<proto::ControlUpdateB>(
                    graph_state,
                    executor,
                    EPriority::ControlUpdateB
                )
            )
            .add(
                proto::message_type::ControlUpdateI,
                make_control_update<proto::ControlUpdateI>(
                    graph_state,
                    executor,
                    EPriority::ControlUpdateI
                )
            )
            .add(
                proto::message_type::ControlUpdateF,
                make_control_update<proto::ControlUpdateF>(
                    graph_state,
                    executor,
                    EPriority::ControlUpdateF
                )
            )
            .add(
                proto::message_type::ControlUpdateS,
                make_control_update<proto::ControlUpdateS>(
                    graph_state,
                    executor,
                    EPriority::ControlUpdateS
                )
            )
            .add(proto::message_type::Play, [&graph_state]() { *graph_state.paused = false; })
            .add(proto::message_type::Pause, [&graph_state]() { *graph_state.paused = true; })
            .add(
                proto::message_type::State,
                [&graph_state, &app_state, &types, &service, &executor](
                    const std::string& id,
                    const proto::State& info
                )
                {
                    if (info.types().SerializeAsString() != types)
                    {
                        nil::log();
                        std::cout << "state is not compatible to types" << std::endl;
                        return;
                    }

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
                            .alias = node.alias(),
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
                                std::cout << "incomplete input... not supported" << std::endl;
                                return;
                            }
                        }
                    }

                    executor.reset();
                    executor.push(
                        {EPriority::State, 0u},
                        [&graph_state,
                         &app_state,
                         &service,
                         &executor,
                         id,
                         info,
                         nodes = std::move(nodes)]()
                        {
                            app_state.info = info;
                            graph_state = make_state(service, executor);

                            for (const auto& node : nodes)
                            {
                                if (node.type == 0)
                                {
                                    app_state.edge_factories[node.alias](
                                        graph_state,
                                        node.outputs[0]
                                    );
                                }
                            }

                            for (const auto& node : ext::sort_by_score(nodes))
                            {
                                auto& factory = app_state.node_factories[node.type];
                                factory(
                                    graph_state,
                                    node.id,
                                    node.alias,
                                    node.inputs,
                                    node.outputs,
                                    node.controls
                                );
                            }

                            service.send(id, proto::message_type::State, app_state.info);
                        }
                    );
                }
            )
            .add(
                proto::message_type::Run,
                [&graph_state, &executor]() {
                    executor.push(
                        {EPriority::Run, 0},
                        [&graph_state]() { graph_state.core.commit(); }
                    );
                }
            )
    );

    service.run();
    return 0;
}
