#include <nil/gatex/Core.hpp>
#include <nil/gatex/Info.hpp>

#include <nil/gate/runners/boost_asio.hpp>

#include <iostream>
#include <stdexcept>
#include <thread>
#include <utility>

namespace nil::gatex
{
    Core::Core()
    {
        context = std::make_unique<boost::asio::io_context>();
        core.set_runner(std::make_unique<nil::gate::runners::Asio>(10));
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
            using creator_t = void (Core::*)(const GraphInfo& info, const NodeInfo& node_info);

            explicit BuiltInNode(creator_t init_creator)
                : maker(init_creator)
            {
            }

            void create_node(Core& xcore, const GraphInfo& graph, const NodeInfo& node) override
            {
                (xcore.*maker)(graph, node);
            }

            creator_t maker;
        };

        node_factories.emplace_back(std::make_unique<BuiltInNode>(&Core::make_feedback));
        node_factories.emplace_back(std::make_unique<BuiltInNode>(&Core::make_delay));
    }

    void Core::set_metadata(std::string new_metadata)
    {
        metadata = std::move(new_metadata);
    }

    bool Core::is_compatible(const std::string& data) const
    {
        return metadata == data;
    }

    void Core::instantiate(const GraphInfo& graph)
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

    void Core::set_control_value(std::uint64_t id, bool value)
    {
        boost::asio::post(
            *context,
            [this, id, value]()
            {
                edges.at(id).set_value(value);
                core.commit();
            }
        );
    }

    void Core::set_control_value(std::uint64_t id, int value)
    {
        boost::asio::post(
            *context,
            [this, id, value]()
            {
                edges.at(id).set_value(value);
                core.commit();
            }
        );
    }

    void Core::set_control_value(std::uint64_t id, float value)
    {
        boost::asio::post(
            *context,
            [this, id, value]()
            {
                edges.at(id).set_value(value);
                core.commit();
            }
        );
    }

    void Core::set_control_value(std::uint64_t id, const std::string& value)
    {
        boost::asio::post(
            *context,
            [this, id, value]()
            {
                edges.at(id).set_value(value);
                core.commit();
            }
        );
    }

    void Core::post(const void* id, float time, std::unique_ptr<nil::gate::ICallable<void()>> cb)
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

    void Core::start()
    {
        if (!thread)
        {
            timers.clear();
            context = std::make_unique<boost::asio::io_context>();
            resume();
        }
    }

    void Core::resume()
    {
        if (!thread)
        {
            thread = std::make_unique<std::thread>(
                [this]()
                {
                    auto work = boost::asio::make_work_guard(*context);
                    core.commit();
                    context->run();
                }
            );
        }
    }

    void Core::stop()
    {
        if (context)
        {
            context->stop();
        }
    }

    void Core::wait()
    {
        if (thread && thread->joinable())
        {
            thread->join();
            thread.reset();
            context->restart();
        }
    }

    void Core::make_feedback(const GraphInfo& graph, const NodeInfo& node)
    {
        type_factories
            .at(graph.link_type(node.inputs.at(0))) //
            ->create_feedback(*this, graph, node);
    }

    void Core::make_delay(const GraphInfo& graph, const NodeInfo& node)
    {
        type_factories
            .at(graph.link_type(node.inputs.at(0))) //
            ->create_delay(*this, graph, node);
    }

    void Core::activate(std::uint64_t id)
    {
        if (before_hook)
        {
            before_hook->call(id);
        }
    }

    void Core::deactivate(std::uint64_t id)
    {
        if (after_hook)
        {
            after_hook->call(id);
        }
    }

}
