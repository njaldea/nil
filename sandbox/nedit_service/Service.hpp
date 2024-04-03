#pragma once

#include "Info.hpp"

#include <nil/gate.hpp>

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

    virtual nil::gate::IEdge* create_edge(nil::gate::Core& core) = 0;
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

    template <typename T>
    operator nil::gate::edges::Compatible<T>() const // NOLINT
    {
        return static_cast<nil::gate::edges::ReadOnly<T>*>(edge);
    }
};

struct Service
{
    void install();
    void populate(const GraphInfo& graph);

    nil::gate::Core core;
    std::unordered_map<std::uint64_t, RelaxedEdge> edges;
    std::vector<std::unique_ptr<IType>> type_factories;
    std::vector<std::unique_ptr<INode>> node_factories;

    void post(const void*, float, std::function<void()>);
    void run();

    boost::asio::io_context context;
    std::unordered_map<const void*, std::unique_ptr<boost::asio::deadline_timer>> timers;
};
