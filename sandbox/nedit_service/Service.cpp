#include "Service.hpp"
#include "Info.hpp"
#include "nil/gate/runners/immediate.hpp"

#include <nil/dev.hpp>
#include <nil/gate/runners/boost_asio.hpp>

#include <iostream>
#include <thread>
#include <utility>

Service::Service()
{
    core.set_commit([this](auto&) { boost::asio::post(*context, [this]() { core.run(); }); });

    struct Any
    {
        bool operator==(const Any& /* unused */) const
        {
            return true;
        }
    };

    add_type(Any());

    struct BuiltInNode: INode
    {
        using creator_t = void (Service::*)(const GraphInfo& info, const NodeInfo& node_info);

        explicit BuiltInNode(creator_t init_creator)
            : maker(init_creator)
        {
        }

        void create_node(Service& service, const GraphInfo& graph, const NodeInfo& node) override
        {
            (service.*maker)(graph, node);
        }

        creator_t maker;
    };

    node_factories.emplace_back(std::make_unique<BuiltInNode>(&Service::make_feedback));
    node_factories.emplace_back(std::make_unique<BuiltInNode>(&Service::make_delay));
}

void Service::instantiate(const GraphInfo& graph)
{
    edges.clear();
    core.clear();

    for (const auto& score : graph.scores)
    {
        const auto& node = score.second->second;
        if (node.type == 0)
        {
            type_factories
                .at(graph.link_type(node.inputs.at(0))) //
                ->create_edge(*this, node.outputs.at(0));
        }
    }
    // populate feedback edge here.
    for (const auto& score : graph.scores)
    {
        const auto& node = score.second->second;
        node_factories.at(node.type)->create_node(*this, graph, node);
    }
}

void Service::set_control_value(std::uint64_t id, bool value)
{
    if (context)
    {
        boost::asio::post(*context, [this, id, value]() { edges.at(id).set_value(value); });
    }
}

void Service::set_control_value(std::uint64_t id, int value)
{
    if (context)
    {
        boost::asio::post(*context, [this, id, value]() { edges.at(id).set_value(value); });
    }
}

void Service::set_control_value(std::uint64_t id, float value)
{
    if (context)
    {
        boost::asio::post(*context, [this, id, value]() { edges.at(id).set_value(value); });
    }
}

void Service::set_control_value(std::uint64_t id, const std::string& value)
{
    if (context)
    {
        boost::asio::post(*context, [this, id, value]() { edges.at(id).set_value(value); });
    }
}

void Service::post(const void* id, float time, std::unique_ptr<nil::gate::ICallable<void()>> cb)
{
    if (context)
    {
        boost::asio::post(
            *context,
            [this, id, time, cb = std::move(cb)]() mutable
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
                    [this, id, cb = std::move(cb)](const boost::system::error_code& er) mutable
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
                        cb->call();
                    }
                );
            }
        );
    }
}

void Service::start(std::uint64_t threads)
{
    if (threads > 1)
    {
        core.set_runner(std::make_unique<nil::gate::runners::Asio>(threads));
    }
    else
    {
        core.set_runner(std::make_unique<nil::gate::runners::Immediate>());
    }
    context = std::make_unique<boost::asio::io_context>();
    thread = std::thread(
        [this]()
        {
            auto work = boost::asio::make_work_guard(*context);
            core.commit();
            context->run();
        }
    );
}

void Service::stop()
{
    if (context)
    {
        context->stop();
    }
}

void Service::wait()
{
    thread.join();
}

void Service::make_feedback(const GraphInfo& graph, const NodeInfo& node)
{
    type_factories
        .at(graph.link_type(node.inputs.at(0))) //
        ->create_feedback(*this, graph, node);
}

void Service::make_delay(const GraphInfo& graph, const NodeInfo& node)
{
    type_factories
        .at(graph.link_type(node.inputs.at(0))) //
        ->create_delay(*this, graph, node);
}
