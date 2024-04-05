#include "Command.hpp"
#include "Info.hpp"
#include "Service.hpp"

#include <gen/nedit/messages/state.pb.h>

#include <filesystem>
#include <fstream>
#include <iostream>

nil::cli::OptionInfo CMD::options() const
{
    return nil::cli::Builder()
        .flag("help", {.skey = 'h', .msg = "this help"})
        .number("port", {.skey = 'p', .msg = "use port", .fallback = 1101})
        .build();
}

template <typename T>
struct Input
{
    T operator()(const T& value) const
    {
        std::cout << std::format("Input[{}]: <{}>\n", name, value) << std::flush;
        return value;
    }

    std::string name;
};

struct Add
{
    int operator()(int l, int r) const
    {
        std::cout << std::format("Add: {} + {} = {}\n", l, r, l + r) << std::flush;
        return l + r;
    }
};

struct Mul
{
    int operator()(int l, int r) const
    {
        std::cout << std::format("Mul: {} * {} = {}\n", l, r, l * r) << std::flush;
        return l * r;
    }
};

struct Inverter
{
    int operator()(bool l, int r) const
    {
        std::cout << std::format("Inv: {} => {}\n", l ? 'T' : 'F', l ? -r : r) << std::flush;
        return l ? -r : r;
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

int CMD::run(const nil::cli::Options& options) const
{
    if (options.flag("help"))
    {
        options.help(std::cout);
        return 0;
    }

    std::string file = "../sandbox/nedit/state.dump";
    if (!std::filesystem::exists(file))
    {
        return 1;
    }

    nil::nedit::proto::State state;

    {
        std::ifstream fs(file, std::ios::binary);
        state.ParseFromIstream(&fs);
    }

    GraphInfo info;

    {
        const auto& graph = state.graph();

        for (const auto& node : graph.nodes())
        {
            info.add_node(
                node.id(),
                NodeInfo{
                    .type = node.type(),
                    .inputs = {node.inputs().begin(), node.inputs().end()},
                    .outputs = {node.outputs().begin(), node.outputs().end()},
                    .controls = {node.controls().begin(), node.controls().end()} //
                }
            );
        }

        for (const auto& link : graph.links())
        {
            info.add_link(
                link.id(),
                LinkInfo{
                    .type = link.type(),
                    .input = link.input(),
                    .output = link.output() //
                }
            );
        }

        info.score();
    }

    Service service;

    {
        service.add_type(false);
        service.add_type(0);
        service.add_type(0.0f);
        service.add_type(std::string());

        service.add_node(Input<bool>("b"), false);
        service.add_node(Input<int>("i"), 1);
        service.add_node(Input<float>("f"), 0.0f);
        service.add_node(Input<std::string>("i"), std::string());

        service.add_node(Inverter());

        service.add_node(Add());
        service.add_node(Add(), 0);
        service.add_node(Mul());
        service.add_node(Mul(), 0);

        service.add_node(Consume<bool>());
        service.add_node(Consume<int>());
        service.add_node(Consume<float>());
        service.add_node(Consume<std::string>());
    }

    service.instantiate(info);
    service.start();
    service.wait();

    return 0;
}
