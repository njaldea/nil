#include "ext.hpp"
#include "../codec.hpp"

#include <nil/gate.hpp>
#include <nil/service.hpp>

#include <iostream>
#include <sstream>
#include <thread>

#include <gen/nedit/messages/graph_update.pb.h>
#include <gen/nedit/messages/node_info.pb.h>
#include <gen/nedit/messages/pin_info.pb.h>
#include <gen/nedit/messages/type.pb.h>

namespace
{
    template <int v>
    struct Input_i
    {
        std::tuple<int> operator()()
        {
            std::cout << "Input_i<" << v << '>' << std::endl;
            return {v};
        }
    };

    template <bool v>
    struct Input_b
    {
        std::tuple<bool> operator()()
        {
            std::cout << "Input_b<" << v << '>' << std::endl;
            return {v};
        }
    };

    struct Add
    {
        std::tuple<int> operator()(int l, int r)
        {
            std::cout << l << " + " << r << std::endl;
            return {l + r};
        }
    };

    struct Mul
    {
        std::tuple<int> operator()(int l, int r)
        {
            std::cout << l << " * " << r << std::endl;
            return {l * r};
        }
    };

    struct Inverter
    {
        std::tuple<int> operator()(bool l, int r)
        {
            std::cout << l << " ! " << r << std::endl;
            return {l ? -r : r};
        }
    };

    struct Consume
    {
        void operator()(int v)
        {
            std::cout << "Consume: " << v << std::endl;
        }
    };

    struct App
    {
        std::unique_ptr<nil::gate::Core> core;
        std::unordered_map<std::uint32_t, nil::gate::IEdge*> edges;

        using Factory = std::function<
            void(const std::vector<std::uint32_t>&, const std::vector<std::uint32_t>&)>;

        struct Pin
        {
            struct Color
            {
                float r;
                float g;
                float b;
                float a;
            } color;

            std::string label;
        };

        const std::vector<nil::nedit::proto::PinInfo> pins_info = {
            []()
            {
                nil::nedit::proto::PinInfo info;
                info.mutable_color()->set_r(0.0);
                info.mutable_color()->set_g(1.0);
                info.mutable_color()->set_b(0.0);
                info.mutable_color()->set_a(1.0);
                info.set_label("bool");
                return info;
            }(),
            []()
            {
                nil::nedit::proto::PinInfo info;
                info.mutable_color()->set_r(1.0);
                info.mutable_color()->set_g(0.0);
                info.mutable_color()->set_b(0.0);
                info.mutable_color()->set_a(1.0);
                info.set_label("int");
                return info;
            }(),
        };

        struct Node
        {
            nil::nedit::proto::NodeInfo info;
            Factory factory;
        };

        std::vector<Node> nodes_info = { //
            {
                []()
                {
                    nil::nedit::proto::NodeInfo info;
                    info.add_outputs(0);
                    info.set_label("Input_b<false>");
                    return info;
                }(),
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    (void)input;
                    const auto [result] = core->node<Input_b<false>>({});
                    edges[output[0]] = result;
                } //
            },
            {
                []()
                {
                    nil::nedit::proto::NodeInfo info;
                    info.add_outputs(0);
                    info.set_label("Input_b<true>");
                    return info;
                }(),
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    (void)input;
                    const auto [result] = core->node<Input_b<true>>({});
                    edges[output[0]] = result;
                } //
            },
            {
                []()
                {
                    nil::nedit::proto::NodeInfo info;
                    info.add_outputs(1);
                    info.set_label("Input_i<5>");
                    return info;
                }(),
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    (void)input;
                    const auto [result] = core->node<Input_i<5>>({});
                    edges[output[0]] = result;
                } //
            },
            {
                []()
                {
                    nil::nedit::proto::NodeInfo info;
                    info.add_outputs(1);
                    info.set_label("Input_i<10>");
                    return info;
                }(),
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    (void)input;
                    const auto [result] = core->node<Input_i<10>>({});
                    edges[output[0]] = result;
                } //
            },
            {
                []()
                {
                    nil::nedit::proto::NodeInfo info;
                    info.add_inputs(0);
                    info.add_inputs(1);
                    info.add_outputs(1);
                    info.set_label("Inverter");
                    return info;
                }(),
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    const auto [result] = core->node<Inverter>({
                        static_cast<nil::gate::ReadOnlyEdge<bool>*>(edges[input[0]]),
                        static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[input[1]]) //
                    });
                    edges[output[0]] = result;
                } //
            },
            {
                []()
                {
                    nil::nedit::proto::NodeInfo info;
                    info.add_inputs(1);
                    info.add_inputs(1);
                    info.add_outputs(1);
                    info.set_label("Add");
                    return info;
                }(),
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    const auto [result] = core->node<Add>({
                        static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[input[0]]),
                        static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[input[1]]) //
                    });
                    edges[output[0]] = result;
                } //
            },
            {
                []()
                {
                    nil::nedit::proto::NodeInfo info;
                    info.add_inputs(1);
                    info.add_inputs(1);
                    info.add_outputs(1);
                    info.set_label("Mul");
                    return info;
                }(),
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    const auto [result] = core->node<Mul>({
                        static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[input[0]]),
                        static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[input[1]]) //
                    });
                    edges[output[0]] = result;
                } //
            },
            {
                []()
                {
                    nil::nedit::proto::NodeInfo info;
                    info.add_inputs(1);
                    info.set_label("Consume");
                    return info;
                }(),
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    (void)output;
                    core->node<Consume>({
                        static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[input[0]]) //
                    });
                    return true;
                } //
            }
        };
    };
}

nil::cli::OptionInfo EXT::options() const
{
    return nil::cli::Builder()
        .flag("help", {.skey = 'h', .msg = "this help"})
        .number("port", {.skey = 'p', .msg = "port", .fallback = 1101})
        .build();
}

int EXT::run(const nil::cli::Options& options) const
{
    if (options.flag("help"))
    {
        options.help(std::cout);
        return 0;
    }

    App app;

    nil::service::TypedService client(nil::service::tcp::Client::Options{
        .host = "127.0.0.1",                          //
        .port = std::uint16_t(options.number("port")) //
    });

    client.on_connect(
        [&](const std::string& id)
        {
            for (const auto& pin : app.pins_info)
            {
                client.send(id, nil::nedit::proto::type::PinInfo, pin);
            }
            for (const auto& node : app.nodes_info)
            {
                client.send(id, nil::nedit::proto::type::NodeInfo, node.info);
            }
            {
                client.send(id, nil::nedit::proto::type::Freeze, std::string());
            }
        }
    );

    client.on_message(
        nil::nedit::proto::type::GraphUpdate,
        [&app](const std::string&, const nil::nedit::proto::Graph& graph)
        {
            struct TN
            {
                std::uint32_t score;
                std::uint64_t t;
                std::vector<std::uint32_t> i;
                std::vector<std::uint32_t> o;
            };

            std::vector<TN> items;
            {
                std::unordered_map<std::uint32_t, TN*> edge_to_node;

                for (const auto& node : graph.nodes())
                {
                    items.push_back({
                        0u,
                        node.type(),
                        {node.inputs().begin(), node.inputs().end()},
                        {node.outputs().begin(), node.outputs().end()} //
                    });
                    for (const auto& o : node.outputs())
                    {
                        edge_to_node.emplace(o, &items.back());
                    }
                }
                std::unordered_map<std::uint32_t, std::uint32_t> edge_score;

                for (const auto& [edge_id, node] : edge_to_node)
                {
                    auto final_score = 0u;
                    for (const auto i : node->i)
                    {
                        auto current_score = 0u;
                        auto current_node = node;
                        while (current_node != nullptr)
                        {
                            current_score += 1;
                            current_node = edge_to_node[i];
                        }
                        if (final_score < current_score)
                        {
                            final_score = current_score;
                        }
                    }
                    edge_score[edge_id] = final_score;
                }

                for (auto& node : items)
                {
                    for (const auto& i : node.i)
                    {
                        node.score = std::max(i, node.score);
                    }
                }
            }

            std::sort(
                items.begin(),
                items.end(),
                [](const auto& l, const auto& r) { return l.score < r.score; }
            );

            app.edges.clear();
            app.core = std::make_unique<nil::gate::Core>();

            for (const auto& item : items)
            {
                app.nodes_info[item.t].factory(item.i, item.o);
            }

            try
            {
                app.core->validate();
                app.core->run();
            }
            catch (const std::exception& ex)
            {
                std::cout << ex.what() << std::endl;
            }
        }
    );

    client.run();
    return 0;
}
