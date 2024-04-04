#include "Service.hpp"
#include "Info.hpp"

#include <nil/dev.hpp>
#include <nil/gate/runners/boost_asio.hpp>

#include <iostream>
#include <thread>
#include <utility>

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

Service::Service()
{
    struct Any
    {
        bool operator==(const Any& /* unused */) const
        {
            return true;
        }
    };

    core.set_runner(std::make_unique<nil::gate::runners::Asio>(10));

    add_type(Any());

    node_factories.emplace_back(std::make_unique<BuiltInNode>(&Service::make_feedback));
    node_factories.emplace_back(std::make_unique<BuiltInNode>(&Service::make_delay));
}

void Service::populate(const GraphInfo& graph)
{
    for (const auto& score : graph.scores)
    {
        const auto& node = score.second->second;
        if (node.type == 0)
        {
            type_factories[graph.pin_to_link.at(node.outputs[0]).it->second.type] //
                ->create_edge(*this, graph.pin_to_link.at(node.outputs[0]).it->second.input);
        }
    }
    // populate feedback edge here.
    for (const auto& score : graph.scores)
    {
        const auto& node = score.second->second;
        this->node_factories[node.type]->create_node(*this, graph, node);
    }
}

void Service::post(const void* id, float time, std::function<void()> cb)
{
    boost::asio::post(
        context,
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
                    context,
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

void Service::run()
{
    auto work = boost::asio::make_work_guard(context);

    core.set_commit(
        [&](auto&)
        {
            boost::asio::post(
                context,
                [this]()
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    core.run();
                }
            );
        }
    );
    core.commit();
    context.run();
}

void Service::make_feedback(const GraphInfo& graph, const NodeInfo& node)
{
    type_factories
        .at(graph.pin_to_link.at(node.inputs[0]).it->second.type) //
        ->create_feedback(*this, graph, node);
}

void Service::make_delay(const GraphInfo& graph, const NodeInfo& node)
{
    type_factories
        .at(graph.pin_to_link.at(node.inputs[0]).it->second.type) //
        ->create_delay(*this, graph, node);
}
