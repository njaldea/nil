#include "ext.hpp"
#include "app/install.hpp"
#include "app/sort_nodes.hpp"

#include "../codec.hpp" // IWYU pragma: keep

#include <nil/dev.hpp>
#include <nil/gate/runners/boost_asio/Parallel.hpp>
#include <nil/service.hpp>

#include <gen/nedit/messages/control_update.pb.h>
#include <gen/nedit/messages/node_state.pb.h>
#include <gen/nedit/messages/state.pb.h>
#include <gen/nedit/messages/type.pb.h>

#include <boost/asio.hpp>

#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <thread>

namespace
{
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

        ~Executor()
        {
            if (context && thread)
            {
                cancelled = true;
                context->stop();
                thread->join();
                cancelled = false;
                flushing = false;
            }
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

    ext::GraphState make_state(nil::service::MessagingService& service, Executor& executor)
    {
        ext::GraphState state;
        state.core = std::make_unique<nil::gate::Core>();
        state.core->set_runner<nil::gate::runners::boost_asio::Parallel>(20);
        // TODO: fix pausing. it should prevent the runner to execute the graph

        state.node_activate = [&service](std::uint64_t id)
        {
            nil::nedit::proto::NodeState message;
            message.set_id(id);
            message.set_active(true);
            publish(
                service,
                nil::service::concat(nil::nedit::proto::message_type::NodeState, message)
            );
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        };

        state.node_deactivate = [&service](std::uint64_t id)
        {
            nil::nedit::proto::NodeState message;
            message.set_id(id);
            message.set_active(false);
            publish(
                service,
                nil::service::concat(nil::nedit::proto::message_type::NodeState, message)
            );
        };

        state.post = [&executor](const void* id, float time, std::function<void()> cb)
        { executor.post(id, time, std::move(cb)); };

        return state;
    }

    template <typename T>
    auto control_update(
        ext::GraphState& graph_state,
        Executor& executor,
        EPriority priority,
        const T& message
    )
    {
        return executor.push(
            {priority, message.id()},
            [&graph_state, message]()
            {
                const auto it = graph_state.control_edges.find(message.id());
                if (it != graph_state.control_edges.end())
                {
                    it->second.set_value(message.value());
                    graph_state.core->commit();
                }
            }
        );
    }

    int run(const nil::clix::Options& options)
    {
        if (flag(options, "help"))
        {
            help(options, std::cout);
            return 0;
        }

        ext::AppState app_state;
        ext::App app(app_state);

        auto service
            = nil::service::tcp::server::create({.port = std::uint16_t(number(options, "port"))});

        Executor executor;
        executor.reset();

        ext::GraphState graph_state = make_state(service, executor);

        ext::install(app);
        const std::string types = app_state.info.types().SerializeAsString();

        namespace proto = nil::nedit::proto;
        on_connect(
            service,
            [&service, &app_state](const nil::service::ID& id)
            { send(service, id, nil::service::concat(proto::message_type::State, app_state.info)); }
        );
        on_disconnect(service, [&]() { stop(service); });

        on_message(
            service,
            nil::service::map(
                nil::service::mapping(
                    nil::nedit::proto::message_type::ControlUpdateB,
                    [&](const proto::ControlUpdateB& msg)
                    { control_update(graph_state, executor, EPriority::ControlUpdateB, msg); }
                ),
                nil::service::mapping(
                    nil::nedit::proto::message_type::ControlUpdateI,
                    [&](const proto::ControlUpdateI& msg)
                    { control_update(graph_state, executor, EPriority::ControlUpdateI, msg); }
                ),
                nil::service::mapping(
                    nil::nedit::proto::message_type::ControlUpdateF,
                    [&](const proto::ControlUpdateF& msg)
                    { control_update(graph_state, executor, EPriority::ControlUpdateF, msg); }
                ),
                nil::service::mapping(
                    nil::nedit::proto::message_type::ControlUpdateS,
                    [&](const proto::ControlUpdateS& msg)
                    { control_update(graph_state, executor, EPriority::ControlUpdateS, msg); }
                ),
                nil::service::mapping(
                    nil::nedit::proto::message_type::Play,
                    [&]() { *graph_state.paused = false; }
                ),
                nil::service::mapping(
                    nil::nedit::proto::message_type::Pause,
                    [&]() { *graph_state.paused = true; }
                ),
                nil::service::mapping(
                    nil::nedit::proto::message_type::State,
                    [&](const auto& id, const proto::State& info)
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
                                    std::cout << "incomplete input... not supported\n"
                                              << std::flush;
                                    send(
                                        service,
                                        id,
                                        nil::service::concat(proto::message_type::State, info)
                                    );
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

                                send(
                                    service,
                                    id,
                                    nil::service::concat(proto::message_type::State, app_state.info)
                                );
                            }
                        );
                    }
                ),
                nil::service::mapping(
                    nil::nedit::proto::message_type::Run,
                    [&]() {
                        executor.push(
                            {EPriority::Run, 0},
                            [&graph_state]() { graph_state.core->commit(); }
                        );
                    }
                )
            )
        );

        start(service);
        return 0;
    }
}

namespace EXT
{
    void apply(nil::clix::Node& node)
    {
        flag(node, "help", {.skey = 'h', .msg = "this help"});
        number(node, "port", {.skey = 'p', .msg = "port", .fallback = 1101});
        use(node, run);
    }
}
