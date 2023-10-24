#include "ext.hpp"

#include <nil/gate.hpp>
#include <nil/service.hpp>

#include <iostream>
#include <sstream>
#include <thread>

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
            bool(const std::vector<std::uint32_t>&, const std::vector<std::uint32_t>&)>;

        struct Node
        {
            std::string info;
            Factory factory;
        };

        std::vector<std::string> edges_info = {
            "pin:1.0,0.0,0.0,1.0:int",
            "pin:0.0,1.0,0.0,1.0:bool" //
        };
        std::vector<Node> nodes_info = { //
            {
                "node:-1:Input_b<false>",
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    (void)input;
                    const auto [result] = core->node<Input_b<false>>({});
                    edges[output[0]] = result;
                    return true;
                } //
            },
            {
                "node:-1:Input_b<true>",
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    (void)input;
                    const auto [result] = core->node<Input_b<true>>({});
                    edges[output[0]] = result;
                    return true;
                } //
            },
            {
                "node:-0:Input_i<5>",
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    (void)input;
                    const auto [result] = core->node<Input_i<5>>({});
                    edges[output[0]] = result;
                    return true;
                } //
            },
            {
                "node:-0:Input_i<10>",
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    (void)input;
                    const auto [result] = core->node<Input_i<10>>({});
                    edges[output[0]] = result;
                    return true;
                } //
            },
            {
                "node:1,0-0:Inverter",
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    if (!edges.contains(input[0]) || !edges.contains(input[1]))
                    {
                        return false;
                    }
                    const auto [result] = core->node<Inverter>({
                        static_cast<nil::gate::ReadOnlyEdge<bool>*>(edges[input[0]]),
                        static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[input[1]]) //
                    });
                    edges[output[0]] = result;
                    return true;
                } //
            },
            {
                "node:0,0-0:Add",
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    if (!edges.contains(input[0]) || !edges.contains(input[1]))
                    {
                        return false;
                    }
                    const auto [result] = core->node<Add>({
                        static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[input[0]]),
                        static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[input[1]]) //
                    });
                    edges[output[0]] = result;
                    return true;
                } //
            },
            {
                "node:0,0-0:Mul",
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    if (!edges.contains(input[0]) || !edges.contains(input[1]))
                    {
                        return false;
                    }
                    const auto [result] = core->node<Mul>({
                        static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[input[0]]),
                        static_cast<nil::gate::ReadOnlyEdge<int>*>(edges[input[1]]) //
                    });
                    edges[output[0]] = result;
                    return true;
                } //
            },
            {
                "node:0-:Consume",
                [this](
                    const std::vector<std::uint32_t>& input,
                    const std::vector<std::uint32_t>& output
                )
                {
                    if (!edges.contains(input[0]))
                    {
                        return false;
                    }
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
        .number("port", {.skey = 'p', .msg = "port"})
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
    nil::service::tcp::Client client({
        .host = "127.0.0.1",                          //
        .port = std::uint16_t(options.number("port")) //
    });
    client.on_connect(
        [&](const std::string& id)
        {
            for (const auto& edge : app.edges_info)
            {
                client.send(id, edge.c_str(), edge.size());
            }
            for (const auto& node : app.nodes_info)
            {
                client.send(id, node.info.c_str(), node.info.size());
            }
            client.send(id, "freeze", sizeof("freeze"));
        }
    );

    client.on_message(
        [&](const std::string&, const void* data, std::uint64_t count)
        {
            const auto message = std::string_view(static_cast<const char*>(data), count);
            if (message == "play")
            {
                if (app.core)
                {
                    app.core->run();
                }
            }
            else if (message.starts_with("graph:"))
            {
                struct N
                {
                    bool done;
                    std::uint64_t t;
                    std::vector<std::uint32_t> i;
                    std::vector<std::uint32_t> o;
                };
                std::vector<N> items;
                auto index = message.find_first_of(":node=");
                while (index != std::string_view::npos && index != message.size())
                {
                    auto next_index = message.find_first_of(":node=", index + sizeof(":node="));
                    const auto item = //
                        (next_index == std::string_view::npos
                             ? message.substr(index, message.size() - index)
                             : message.substr(index, next_index - index));
                    if (item.size() > sizeof(":node="))
                    {
                        const auto i = item.substr(sizeof(":node=") - 1);
                        const auto type_end = i.find_first_of("-");
                        if (type_end == std::string_view::npos)
                        {
                            continue;
                        }
                        const auto type = std::stoull(std::string(i.substr(0, type_end)));
                        const auto input_end = i.find_first_of("-", type_end + 1);

                        std::vector<std::uint32_t> vi;
                        {
                            const auto inputs = i.substr(type_end + 1, input_end - type_end - 1);
                            if (!inputs.empty())
                            {
                                auto i_start = 0ul;
                                while (i_start < inputs.size())
                                {
                                    auto i_end = inputs.find_first_of(",", i_start + 1);
                                    vi.push_back(std::uint32_t(std::stoull(
                                        std::string(inputs.substr(i_start, i_end - i_start))
                                    )));
                                    if (i_end == std::string_view::npos)
                                    {
                                        break;
                                    }
                                    i_start = i_end + 1;
                                }
                            }
                        }
                        std::vector<std::uint32_t> vo;
                        {
                            const auto outputs = i.substr(input_end + 1);
                            if (!outputs.empty())
                            {
                                auto i_start = 0ul;
                                while (i_start < outputs.size())
                                {
                                    auto i_end = outputs.find_first_of(",", i_start + 1);
                                    vo.push_back(std::uint32_t(std::stoull(
                                        std::string(outputs.substr(i_start, i_end - i_start))
                                    )));
                                    if (i_end == std::string_view::npos)
                                    {
                                        break;
                                    }
                                    i_start = i_end + 1;
                                }
                            }
                        }
                        items.push_back({false, type, std::move(vi), std::move(vo)});
                    }
                    index = next_index;
                }

                app.edges.clear();
                app.core = std::make_unique<nil::gate::Core>();

                std::size_t done = 0;
                while (done != items.size())
                {
                    const auto current = done;
                    for (auto& n : items)
                    {
                        if (!n.done)
                        {
                            n.done = app.nodes_info[n.t].factory(n.i, n.o);
                            if (n.done)
                            {
                                done += 1;
                            }
                        }
                    }
                    if (current == done)
                    {
                        std::cout << "error" << std::endl;
                        return;
                    }
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
        }
    );

    while (true)
    {
        std::thread t1([&]() { client.run(); });
        std::string message;
        while (std::getline(std::cin, message))
        {
            if (message == "reconnect")
            {
                break;
            }
            client.publish(message.data(), message.size());
        }
        client.stop();
        t1.join();
        client.restart();
    }
    return 0;
}
