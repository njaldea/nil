#pragma once

#include "Info.hpp"

#include <nil/gate.hpp>
#include <nil/utils/traits/identity.hpp>

#include <boost/asio.hpp>

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

    virtual void create_edge(
        Service& service,
        std::uint64_t id //
    ) = 0;
    virtual void create_feedback(
        Service& service,
        const GraphInfo& info,
        const NodeInfo& node_info //
    ) = 0;
    virtual void create_delay(
        Service& service,
        const GraphInfo& info,
        const NodeInfo& node_info //
    ) = 0;
};

struct INode
{
    INode() = default;
    virtual ~INode() = default;
    INode(INode&&) = delete;
    INode(const INode&) = delete;
    INode& operator=(INode&&) = delete;
    INode& operator=(const INode&) = delete;

    virtual void create_node(
        Service& service,
        const GraphInfo& graph,
        const NodeInfo& node_info //
    ) = 0;
};

class RelaxedEdge
{
public:
    template <typename T>
    explicit RelaxedEdge(nil::gate::edges::ReadOnly<T>* init_edge)
        : edge(init_edge)
        , identity(nil::utils::traits::identity_v<T>)
    {
    }

    template <typename T>
    void set_value(T value)
    {
        if (nil::utils::traits::identity_v<T> != identity)
        {
            throw std::runtime_error("incompatible types");
        }
        static_cast<nil::gate::edges::Mutable<T>*>(edge)->set_value(std::move(value));
    }

    template <typename T>
    operator nil::gate::edges::Mutable<T>*() const // NOLINT(hicpp-explicit-conversions)
    {
        if (nil::utils::traits::identity_v<T> != identity)
        {
            throw std::runtime_error("incompatible types");
        }
        return static_cast<nil::gate::edges::Mutable<T>*>(edge);
    }

    template <typename T>
    operator nil::gate::edges::Compatible<T>() const // NOLINT(hicpp-explicit-conversions)
    {
        if (nil::utils::traits::identity_v<T> != identity)
        {
            throw std::runtime_error("incompatible types");
        }
        return static_cast<nil::gate::edges::ReadOnly<T>*>(edge);
    }

private:
    nil::gate::IEdge* edge;
    const void* identity;
};

struct Service
{
    Service();
    ~Service() noexcept = default;
    Service(Service&&) = delete;
    Service(const Service&) = delete;
    Service& operator=(Service&&) = delete;
    Service& operator=(const Service&) = delete;

    template <typename T>
        requires std::is_same_v<T, std::decay_t<T>>
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
                    void operator()(const nil::gate::Core& c, const T& v, bool enabled) const
                    {
                        if (enabled)
                        {
                            auto [e] = c.batch(output);
                            e->set_value(v);
                        }
                    }

                    nil::gate::edges::Mutable<T>* output;
                };

                auto* edge = service.core.edge(true);
                service.edges.emplace(node_info.controls.at(0), RelaxedEdge(edge));
                service.core.node(
                    FeedbackNode{service.edges.at(node_info.outputs.at(0))},
                    {service.edges.at(info.opposite_output_pin(node_info.inputs.at(0))), edge}
                );
            }

            void create_delay(Service& service, const GraphInfo& info, const NodeInfo& node_info)
                override
            {
                struct Delay
                {
                    void operator()(
                        const nil::gate::Core& c,
                        nil::gate::async_outputs<T> asyncs,
                        const T& v,
                        float time
                    ) const
                    {
                        if (time == 0)
                        {
                            auto [e] = c.batch(asyncs);
                            e->set_value(std::move(v));
                        }
                        else
                        {
                            service->post(
                                this,
                                time,
                                nil::gate::make_callable(
                                    [&c, v, asyncs]() mutable
                                    {
                                        auto [e] = c.batch(asyncs);
                                        e->set_value(std::move(v));
                                    }
                                )
                            );
                        }
                    }

                    Service* service;
                };

                auto* time = service.core.edge(0.f);
                service.edges.emplace(node_info.controls.at(0), RelaxedEdge(time));

                const auto [o] = service.core.node(
                    Delay{&service},
                    std::tuple<T>(),
                    {service.edges.at(info.opposite_output_pin(node_info.inputs.at(0))), time}
                );
                service.edges.emplace(node_info.outputs.at(0), RelaxedEdge(o));
            }
        };

        type_factories.emplace_back(std::make_unique<Type>(std::move(default_value)));
    }

    template <typename T, typename... C>
        requires nil::gate::detail::traits::node<T>::is_valid
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
                using node_t = nil::gate::detail::traits::node<T>;
                constexpr auto input_count = node_t::inputs::size;
                constexpr auto output_count = node_t::outputs::size;
                constexpr auto control_count = sizeof...(C);

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

        node_factories.emplace_back(
            std::make_unique<Node>(std::move(instance), std::move(controls)...)
        );
    }

    void instantiate(const GraphInfo& graph);

    void set_control_value(std::uint64_t id, bool value);
    void set_control_value(std::uint64_t id, int value);
    void set_control_value(std::uint64_t id, float value);
    void set_control_value(std::uint64_t id, const std::string& value);

    void start(std::uint64_t threads = 1);
    void stop();
    void wait();

private:
    nil::gate::Core core;
    std::unordered_map<std::uint64_t, RelaxedEdge> edges;
    std::vector<std::unique_ptr<IType>> type_factories;
    std::vector<std::unique_ptr<INode>> node_factories;

    std::unique_ptr<boost::asio::io_context> context;
    std::unordered_map<const void*, std::unique_ptr<boost::asio::deadline_timer>> timers;
    std::thread thread;

    template <typename... C, std::size_t... I>
    void make_controls(
        const NodeInfo& node,
        const std::tuple<C...>& controls,
        std::index_sequence<I...> /* unused */
    )
    {
        (..., edges.emplace(node.controls.at(I), RelaxedEdge(core.edge(get<I>(controls)))));
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
                {
                    edges.at(graph.opposite_output_pin(node.inputs.at(I)))...,
                    edges.at(node.controls.at(C))... //
                }
            );
        }
        else
        {
            const auto output_edges = core.node(
                std::move(instance),
                {
                    edges.at(graph.opposite_output_pin(node.inputs.at(I)))...,
                    edges.at(node.controls.at(C))... //
                }
            );
            (..., edges.emplace(node.outputs.at(O), RelaxedEdge(get<O>(output_edges))));
        }
    }

    void make_feedback(const GraphInfo& graph, const NodeInfo& node);
    void make_delay(const GraphInfo& graph, const NodeInfo& node);

    void post(const void*, float, std::unique_ptr<nil::gate::ICallable<void()>>);
};
