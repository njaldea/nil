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

    /**
     * TODO:
     *  -   move Tree to gate
     *  -   rewrite Tree to remove score out
     *  -   make Tree emit TN without score but already sorted
     *  -   make Tree not depend on graph
     *  -   move GraphBuilder to gate
     */

    class Tree
    {
    public:
        struct TN
        {
            std::uint32_t score;
            std::uint64_t t;
            std::vector<std::uint64_t> i;
            std::vector<std::uint64_t> o;
        };

    private:
        struct TE
        {
            TN* input = nullptr;
            std::vector<TN*> outputs;
        };

        struct State
        {
            std::vector<TN> nodes;
            std::unordered_map<std::uint64_t, TE> edges;
        };

    public:
        std::vector<TN> populate(const nil::nedit::proto::Graph& g)
        {
            State state;

            for (const auto& node : g.nodes())
            {
                add_node(
                    state,
                    node.type(),
                    {node.inputs().begin(), node.inputs().end()},
                    {node.outputs().begin(), node.outputs().end()} //
                );
            }
            calculate_score(state);
            sort(state);

            return std::move(state.nodes);
        }

    private:
        void add_node(
            State& state,
            std::uint64_t type,
            std::vector<std::uint64_t> inputs,
            std::vector<std::uint64_t> outputs
        )
        {
            state.nodes.push_back({0u, type, std::move(inputs), std::move(outputs)});
            for (const auto& i : state.nodes.back().i)
            {
                state.edges[i].outputs.push_back(&state.nodes.back());
            }
            for (const auto& o : state.nodes.back().o)
            {
                state.edges[o].input = &state.nodes.back();
            }
        }

        void calculate_score(State& state)
        {
            for (auto& node : state.nodes)
            {
                node.score = recurse_score(state, &node);
            }
        }

        // TODO:
        //  optimize for larger graph.
        //  find a way to early return when a score is already calculated.
        std::uint32_t recurse_score(const State& state, const TN* current_node)
        {
            auto score = 0u;
            for (const auto& i : current_node->i)
            {
                score = std::max(recurse_score(state, state.edges.at(i).input) + 1, score);
            }
            return score;
        }

        void sort(State& state)
        {
            std::sort(
                state.nodes.begin(),
                state.nodes.end(),
                [](const auto& l, const auto& r) { return l.score < r.score; }
            );
        }
    };

    class GraphBuilder
    {
    private:
        using EdgeIDs = std::vector<std::uint64_t>;
        using NodeFactory
            = std::function<void(GraphBuilder&, nil::gate::Core&, const EdgeIDs&, const EdgeIDs&)>;

    public:
        template <typename T>
        std::enable_if_t<nil::gate::detail::traits<T>::is_valid, GraphBuilder&> add_node()
        {
            node_factories.push_back(
                [](                        //
                    GraphBuilder& self,    //
                    nil::gate::Core& core, //
                    const EdgeIDs& inputs, //
                    const EdgeIDs& outputs //
                )
                {
                    using traits = nil::gate::detail::traits<T>;
                    const auto i_seq = typename traits::i::make_sequence();
                    const auto o_seq = typename traits::o::make_sequence();
                    const auto result = self.create<T>(core, inputs, i_seq);
                    if constexpr (traits::o::size > 0)
                    {
                        self.store<T>(outputs, result, o_seq);
                    }
                }
            );
            return *this;
        }

        void instantiate(
            nil::gate::Core& core,
            std::uint64_t type,
            const EdgeIDs& inputs,
            const EdgeIDs& outputs
        )
        {
            node_factories[type](*this, core, inputs, outputs);
        }

    private:
        struct RelaxedEdge
        {
            template <typename T>
            operator nil::gate::ReadOnlyEdge<T>*() const
            {
                return static_cast<nil::gate::ReadOnlyEdge<T>*>(value);
            }

            nil::gate::IEdge* value;
        };

        std::vector<NodeFactory> node_factories;
        std::unordered_map<std::uint64_t, RelaxedEdge> edges;

        template <typename T, std::size_t... indices>
        auto create(
            nil::gate::Core& core,
            const EdgeIDs& inputs,
            std::index_sequence<indices...> //
        )
        {
            using edges_t = typename nil::gate::detail::traits<T>::i::readonly_edges;
            return core.node<T>(edges_t(edges.at(inputs[indices])...));
        }

        template <typename T, std::size_t... indices>
        void store(
            const EdgeIDs& outputs,
            const nil::gate::detail::traits<T>::o::readonly_edges& result,
            std::index_sequence<indices...> //
        )
        {
            (..., edges.emplace(outputs[indices], std::get<indices>(result)));
        }
    };

    struct App
    {
        const std::array<nil::nedit::proto::PinInfo, 2> pins_info = {
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

        const std::array<nil::nedit::proto::NodeInfo, 8> nodes_info = { //
            []()
            {
                nil::nedit::proto::NodeInfo info;
                info.add_outputs(0);
                info.set_label("Input_b<false>");
                return info;
            }(),
            []()
            {
                nil::nedit::proto::NodeInfo info;
                info.add_outputs(0);
                info.set_label("Input_b<true>");
                return info;
            }(),
            []()
            {
                nil::nedit::proto::NodeInfo info;
                info.add_outputs(1);
                info.set_label("Input_i<5>");
                return info;
            }(),
            []()
            {
                nil::nedit::proto::NodeInfo info;
                info.add_outputs(1);
                info.set_label("Input_i<10>");
                return info;
            }(),
            []()
            {
                nil::nedit::proto::NodeInfo info;
                info.add_inputs(0);
                info.add_inputs(1);
                info.add_outputs(1);
                info.set_label("Inverter");
                return info;
            }(),
            []()
            {
                nil::nedit::proto::NodeInfo info;
                info.add_inputs(1);
                info.add_inputs(1);
                info.add_outputs(1);
                info.set_label("Add");
                return info;
            }(),
            []()
            {
                nil::nedit::proto::NodeInfo info;
                info.add_inputs(1);
                info.add_inputs(1);
                info.add_outputs(1);
                info.set_label("Mul");
                return info;
            }(),
            []()
            {
                nil::nedit::proto::NodeInfo info;
                info.add_inputs(1);
                info.set_label("Consume");
                return info;
            }()
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

auto make_builder()
{
    return GraphBuilder()
        .add_node<Input_b<false>>()
        .add_node<Input_b<true>>()
        .add_node<Input_i<5>>()
        .add_node<Input_i<10>>()
        .add_node<Inverter>()
        .add_node<Add>()
        .add_node<Mul>()
        .add_node<Consume>();
}

int EXT::run(const nil::cli::Options& options) const
{
    if (options.flag("help"))
    {
        options.help(std::cout);
        return 0;
    }

    const auto app = App();
    auto core = std::unique_ptr<nil::gate::Core>();

    nil::service::TypedService client( //
        nil::service::tcp::Client::Options{
            .host = "127.0.0.1",
            .port = std::uint16_t(options.number("port")) //
        }
    );

    client.on_connect(
        [&](const std::string& id)
        {
            for (const auto& pin : app.pins_info)
            {
                client.send(id, nil::nedit::proto::type::PinInfo, pin);
            }
            for (const auto& node : app.nodes_info)
            {
                client.send(id, nil::nedit::proto::type::NodeInfo, node);
            }
            {
                client.send(id, nil::nedit::proto::type::Freeze, std::string());
            }
        }
    );

    client.on_message(
        nil::nedit::proto::type::GraphUpdate,
        [&core](const std::string&, const nil::nedit::proto::Graph& graph)
        {
            core = std::make_unique<nil::gate::Core>();
            auto builder = make_builder();

            const auto nodes = Tree().populate(graph);

            for (const auto& node : nodes)
            {
                builder.instantiate(*core, node.t, node.i, node.o);
            }

            try
            {
                core->validate();
                core->run();
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
