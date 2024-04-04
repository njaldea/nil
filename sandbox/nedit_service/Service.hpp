#pragma once

#include "Info.hpp"

#include <nil/gate.hpp>
#include <nil/utils/traits/identity.hpp>

#include <boost/asio.hpp>

#include <functional>
#include <unordered_map>

struct Service;

struct IType
{
    IType() = default;
    virtual ~IType() = default;
    IType(IType&&) = delete;
    IType(const IType&) = delete;
    IType& operator=(IType&&) = delete;
    IType& operator=(const IType&) = delete;

    virtual void create_edge(Service& service, std::uint64_t id1) = 0;
    virtual void create_feedback(Service& service, const GraphInfo& info, const NodeInfo& node_info)
        = 0;
    virtual void create_delay(Service& service, const GraphInfo& info, const NodeInfo& node_info)
        = 0;
};

struct INode
{
    INode() = default;
    virtual ~INode() = default;
    INode(INode&&) = delete;
    INode(const INode&) = delete;
    INode& operator=(INode&&) = delete;
    INode& operator=(const INode&) = delete;
    virtual void create_node(Service& service, const GraphInfo& graph, const NodeInfo& node_info)
        = 0;
};

struct RelaxedEdge
{
    nil::gate::IEdge* edge;
    const void* identity;

    template <typename T>
    explicit RelaxedEdge(nil::gate::edges::ReadOnly<T>* init_edge)
        : edge(init_edge)
        , identity(nil::utils::traits::identity_v<T>)
    {
    }

    template <typename T>
    operator nil::gate::edges::Mutable<T>*() const // NOLINT
    {
        if (nil::utils::traits::identity_v<T> != identity)
        {
            throw std::runtime_error("incompatible types");
        }
        return static_cast<nil::gate::edges::Mutable<T>*>(edge);
    }

    template <typename T>
    operator nil::gate::edges::Compatible<T>() const // NOLINT
    {
        if (nil::utils::traits::identity_v<T> != identity)
        {
            throw std::runtime_error("incompatible types");
        }
        return static_cast<nil::gate::edges::ReadOnly<T>*>(edge);
    }
};

struct Service
{
    Service();
    ~Service() noexcept = default;
    Service(Service&&) = delete;
    Service(const Service&) = delete;
    Service& operator=(Service&&) = delete;
    Service& operator=(const Service&) = delete;

    //  TODO:
    //      Probably move populate out and simply add an interface to
    //  instantiate a node one by one and let outside utility functions
    //  handle it.
    void populate(const GraphInfo& graph);

    template <typename T>
    void add_type(T default_value)
    {
        struct Type: IType
        {
            T value;

            explicit Type(T init_value)
                : value(std::move(init_value))
            {
            }

            void create_edge(Service& service, std::uint64_t id) override
            {
                service.edges.emplace(id, RelaxedEdge(service.core.edge(value)));
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
                service.core.node(
                    FeedbackNode{&service, service.edges.at(node_info.outputs[0])},
                    {service.edges.at(info.pin_to_link.at(node_info.inputs[0]).it->second.input),
                     edge}
                );
            }

            void create_delay(Service& service, const GraphInfo& info, const NodeInfo& node_info)
                override
            {
                struct Delay
                {
                    void operator()(
                        nil::gate::async_outputs<T> asyncs,
                        const nil::gate::Core& c,
                        const T& v,
                        float time
                    ) const
                    {
                        service->post(
                            this,
                            time,
                            [&c, v, asyncs]() mutable
                            {
                                if (get<0>(asyncs)->value() != v)
                                {
                                    get<0>(asyncs)->set_value(std::move(v));
                                    c.commit();
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
                    {service.edges.at(info.pin_to_link.at(node_info.inputs[0]).it->second.input),
                     time}
                );
                service.edges.emplace(node_info.outputs[0], RelaxedEdge(o));
            }
        };

        type_factories.emplace_back(std::make_unique<Type>(std::move(default_value)));
    }

    template <typename T, typename... C>
    void add_node(T instance, C... controls)
    {
        struct Node: INode
        {
            explicit Node(T init_instance, C... init_c)
                : instance(std::move(init_instance))
                , controls(init_c...)
            {
            }

            void create_node(Service& service, const GraphInfo& graph, const NodeInfo& node)
                override
            {
                constexpr auto control_count = sizeof...(C);
                constexpr auto input_count = nil::gate::detail::traits::node<T>::inputs::size;
                constexpr auto output_count = nil::gate::detail::traits::node<T>::outputs::size;

                service.make_controls(
                    node,
                    controls,
                    std::make_index_sequence<control_count>() //
                );

                service.make_node(
                    instance,
                    graph,
                    node,
                    std::make_index_sequence<input_count - control_count>(),
                    std::make_index_sequence<control_count>(),
                    std::make_index_sequence<output_count>() //
                );
            }

            T instance;
            std::tuple<C...> controls;
        };

        node_factories.emplace_back( //
            std::make_unique<Node>(std::move(instance), std::move(controls)...)
        );
    }

    void post(const void*, float, std::function<void()>);
    void run();

private:
    nil::gate::Core core;
    std::unordered_map<std::uint64_t, RelaxedEdge> edges;
    std::vector<std::unique_ptr<IType>> type_factories;
    std::vector<std::unique_ptr<INode>> node_factories;

    boost::asio::io_context context;
    std::unordered_map<const void*, std::unique_ptr<boost::asio::deadline_timer>> timers;

    template <typename... C, std::size_t... I>
    void make_controls(
        const NodeInfo& node,
        const std::tuple<C...>& controls,
        std::index_sequence<I...> /* unused */
    )
    {
        ( //
            ...,
            edges.emplace(node.controls.at(I), RelaxedEdge(core.edge(get<I>(controls))))
        );
    }

    template <typename T, std::size_t... I, std::size_t... C, std::size_t... O>
    void make_node(
        T instance,
        const GraphInfo& graph,
        const NodeInfo& node,
        std::index_sequence<I...> /* unused */,
        std::index_sequence<C...> /* unused */,
        std::index_sequence<O...> /* unused */
    )
    {
        if constexpr (sizeof...(O) == 0)
        {
            core.node(
                std::move(instance),
                {edges.at(graph.pin_to_link.at(node.inputs.at(I)).it->second.input)...,
                 edges.at(node.controls.at(C))...}
            );
        }
        else
        {
            const auto output_edges = core.node(
                std::move(instance),
                {edges.at(graph.pin_to_link.at(node.inputs.at(I)).it->second.input)...,
                 edges.at(node.controls.at(C))...}
            );
            ( //
                ...,
                edges.emplace(node.outputs.at(O), RelaxedEdge(get<O>(output_edges)))
            );
        }
    }

    void make_feedback(const GraphInfo& graph, const NodeInfo& node);
    void make_delay(const GraphInfo& graph, const NodeInfo& node);
};
