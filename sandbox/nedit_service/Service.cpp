#include "Service.hpp"
#include "Info.hpp"

#include <nil/dev.hpp>
#include <nil/gate/edges/ReadOnly.hpp>

#include <format>
#include <iostream>
#include <string>
#include <thread>
#include <utility>

template <typename T>
struct Input
{
    std::tuple<T> operator()(const T& value) const
    {
        std::cout << std::format("Input[{}]: <{}>\n", name, value) << std::flush;
        return {value};
    }

    std::string name;
};

struct Add
{
    std::tuple<int> operator()(int l, int r) const
    {
        std::cout << std::format("Add: {} + {} = {}\n", l, r, l + r) << std::flush;
        return {l + r};
    }
};

struct Mul
{
    std::tuple<int> operator()(int l, int r) const
    {
        std::cout << std::format("Mul: {} * {} = {}\n", l, r, l * r) << std::flush;
        return {l * r};
    }
};

struct Inverter
{
    std::tuple<int> operator()(bool l, int r) const
    {
        std::cout << std::format("Inv: {} => {}\n", l ? 'T' : 'F', l ? -r : r) << std::flush;
        return {l ? -r : r};
    }
};

template <typename T>
struct Consume
{
    void operator()(const T& v) const
    {
        std::cout << std::format("Consume: {}\n", v) << std::flush;
    }
};

template <typename T>
struct Type: IType
{
    nil::gate::IEdge* create_edge(nil::gate::Core& core) override
    {
        return core.edge(T());
    }

    void create_feedback(Service& service, const GraphInfo& info, const NodeInfo& node_info)
        override
    {
        struct FeedbackNode
        {
            void operator()(const T& v, bool enabled) const
            {
                if (enabled && output->value() != v)
                {
                    output->set_value(v);
                    service->core.commit();
                }
            }

            Service* service;
            nil::gate::edges::Mutable<T>* output;
        };

        auto* edge = service.core.edge(true);
        service.edges.emplace(node_info.controls[0], RelaxedEdge(edge));
        auto output = static_cast<nil::gate::edges::Mutable<T>*>( //
            service.edges.at(node_info.outputs[0]).edge
        );
        service.core.node(
            FeedbackNode{&service, output},
            {service.edges.at(info.pin_to_link.at(node_info.inputs[0]).it->second.input), edge}
        );
    }

    void create_delay(Service& service, const GraphInfo& info, const NodeInfo& node_info) override
    {
        struct Delay
        {
            void operator()(
                nil::gate::async_outputs<T> asyncs,
                const nil::gate::Core& core,
                const T& v,
                float time
            ) const
            {
                service->post(
                    this,
                    time,
                    [&core, v, asyncs]() mutable
                    {
                        if (get<0>(asyncs)->value() != v)
                        {
                            get<0>(asyncs)->set_value(std::move(v));
                            core.commit();
                        }
                    }
                );
            }

            Service* service;
        };

        auto* time = service.core.edge(0.f);
        service.edges.emplace(node_info.controls[0], RelaxedEdge(time));

        const auto [o] = service.core.node(
            Delay{&service},
            std::tuple<T>(),
            {service.edges.at(info.pin_to_link.at(node_info.inputs[0]).it->second.input), time}
        );
        service.edges.emplace(node_info.outputs[0], RelaxedEdge(o));
    }
};

struct InverterNode: INode
{
    void create_node(Service& service, const GraphInfo& graph, const NodeInfo& node) override
    {
        const auto [o] = service.core.node(
            Inverter(),
            {service.edges.at(graph.pin_to_link.at(node.inputs[0]).it->second.input),
             service.edges.at(graph.pin_to_link.at(node.inputs[1]).it->second.input)}
        );
        service.edges.emplace(node.outputs[0], RelaxedEdge(o));
    }
};

template <typename T>
struct InputNode: INode
{
    explicit InputNode(T init_control)
        : control(std::move(init_control))
    {
    }

    void create_node(Service& service, const GraphInfo& graph, const NodeInfo& node) override
    {
        (void)graph;
        auto* edge = service.core.edge(control);
        const auto [o] = service.core.node(Input<T>(), {edge});
        service.edges.emplace(node.outputs[0], RelaxedEdge(o));
    }

    T control;
};

template <typename T>
struct ProcNode: INode
{
    void create_node(Service& service, const GraphInfo& graph, const NodeInfo& node) override
    {
        const auto [o] = service.core.node(
            T(),
            {service.edges.at(graph.pin_to_link.at(node.inputs[0]).it->second.input),
             service.edges.at(graph.pin_to_link.at(node.inputs[1]).it->second.input)}
        );
        service.edges.emplace(node.outputs[0], RelaxedEdge(o));
    }
};

template <typename T>
struct ControlledProcNode: INode
{
    void create_node(Service& service, const GraphInfo& graph, const NodeInfo& node) override
    {
        auto* edge = service.core.edge(0);
        service.edges.emplace(node.controls[0], RelaxedEdge(edge));
        const auto [o] = service.core.node(
            Add(),
            {service.edges.at(graph.pin_to_link.at(node.inputs[0]).it->second.input), edge}
        );
        service.edges.emplace(node.outputs[0], RelaxedEdge(o));
    }
};

struct BuiltInNode: INode
{
    using creator_t
        = void (IType::*)(Service& service, const GraphInfo& info, const NodeInfo& node_info);

    explicit BuiltInNode(creator_t init_creator)
        : creator(init_creator)
    {
    }

    void create_node(Service& service, const GraphInfo& graph, const NodeInfo& node) override
    {
        (service.type_factories
             .at(graph.pin_to_link.at(node.inputs[0]).it->second.type) //
             .get()
             ->*creator)(service, graph, node);
    }

    void (IType::*creator)(Service& service, const GraphInfo& info, const NodeInfo& node_info);
};

template <typename T>
struct ConsumeNode: INode
{
    void create_node(Service& service, const GraphInfo& graph, const NodeInfo& node) override
    {
        service.core.node(
            Consume<T>(),
            {service.edges.at(graph.pin_to_link.at(node.inputs[0]).it->second.input)}
        );
    }
};

void Service::install()
{
    using text = std::string;

    struct Any
    {
        bool operator==(const Any& /* unused */) const
        {
            return true;
        }
    };

    type_factories.emplace_back(std::make_unique<Type<Any>>());
    type_factories.emplace_back(std::make_unique<Type<bool>>());
    type_factories.emplace_back(std::make_unique<Type<int>>());
    type_factories.emplace_back(std::make_unique<Type<float>>());
    type_factories.emplace_back(std::make_unique<Type<text>>());

    node_factories.emplace_back(std::make_unique<BuiltInNode>(&IType::create_feedback));
    node_factories.emplace_back(std::make_unique<BuiltInNode>(&IType::create_delay));

    node_factories.emplace_back(std::make_unique<InputNode<bool>>(false));
    node_factories.emplace_back(std::make_unique<InputNode<int>>(1));
    node_factories.emplace_back(std::make_unique<InputNode<float>>(0.0f));
    node_factories.emplace_back(std::make_unique<InputNode<text>>(std::string()));

    node_factories.emplace_back(std::make_unique<InverterNode>());

    node_factories.emplace_back(std::make_unique<ProcNode<Add>>());
    node_factories.emplace_back(std::make_unique<ControlledProcNode<Add>>());
    node_factories.emplace_back(std::make_unique<ProcNode<Mul>>());
    node_factories.emplace_back(std::make_unique<ControlledProcNode<Mul>>());

    node_factories.emplace_back(std::make_unique<ConsumeNode<bool>>());
    node_factories.emplace_back(std::make_unique<ConsumeNode<int>>());
    node_factories.emplace_back(std::make_unique<ConsumeNode<float>>());
    node_factories.emplace_back(std::make_unique<ConsumeNode<text>>());
}

void Service::populate(const GraphInfo& graph)
{
    for (const auto& score : graph.scores)
    {
        const auto& node = score.second->second;
        if (node.type == 0)
        {
            std::cout << "feedback populate: " << score.second->first << ":" << node.type
                      << std::endl;
            edges.emplace(
                graph.pin_to_link.at(node.outputs[0]).it->second.input,
                RelaxedEdge{
                    type_factories.at(graph.pin_to_link.at(node.outputs[0]).it->second.type)
                        ->create_edge(core) //
                }
            );
        }
    }
    // populate feedback edge here.
    for (const auto& score : graph.scores)
    {
        const auto& node = score.second->second;
        std::cout << "node create: " << score.second->first << ":" << node.type << std::endl;
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

    const auto r = [&]()
    {
        boost::asio::post(
            context,
            [this]()
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                core.run();
            }
        );
    };
    core.set_commit([&](auto&) { r(); });

    r();
    context.run();
}
