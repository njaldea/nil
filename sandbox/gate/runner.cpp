#include "nil/gate/IRunner.hpp"
#include "nil/gate/types.hpp"
#include <boost/asio/executor_work_guard.hpp>
#include <nil/dev.hpp>
#include <nil/gate.hpp>

#include <nil/gate/bias/nil.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include <iostream>
#include <set>
#include <sstream>
#include <thread>

class Runner final: public nil::gate::IRunner
{
public:
    Runner(boost::asio::io_context& init_main_context, std::size_t count)
        : main_context(init_main_context)
        , work(boost::asio::make_work_guard(exec_context))
    {
        for (auto i = 0u; i < count; ++i)
        {
            exec_th.emplace_back([this]() { exec_context.run(); });
        }
    }

    ~Runner() noexcept override
    {
        work.reset();

        for (auto& t : exec_th)
        {
            t.join();
        }
    }

    Runner(Runner&&) = delete;
    Runner(const Runner&) = delete;
    Runner& operator=(Runner&&) = delete;
    Runner& operator=(const Runner&) = delete;

    void flush(std::vector<std::unique_ptr<nil::gate::ICallable<void()>>> diffs) override
    {
        boost::asio::post(
            main_context,
            [this, diffs = std::move(diffs)]() mutable
            {
                all_diffs.push_back(std::move(diffs));
                if (running_list.empty())
                {
                    for (const auto& dd : std::exchange(all_diffs, {}))
                    {
                        for (const auto& d : dd)
                        {
                            d->call();
                        }
                    }
                }
            }
        );
    }

    void run(const std::vector<std::unique_ptr<nil::gate::INode>>& nodes) override
    {
        boost::asio::post(
            main_context,
            [this, &nodes]()
            {
                if (!running_list.empty())
                {
                    deferred_nodes = &nodes;
                    return;
                }
                for (const auto& node : nodes)
                {
                    if (node->is_pending())
                    {
                        if (node->is_ready())
                        {
                            run_node(node.get());
                        }
                        else
                        {
                            waiting_list.emplace(node.get());
                        }
                    }
                }
            }
        );
    }

    void run_node(nil::gate::INode* node)
    {
        running_list.emplace(node);
        boost::asio::post(
            exec_context,
            [this, node]()
            {
                node->exec();
                mark_done(node);
            }
        );
    }

    void mark_done(nil::gate::INode* node)
    {
        boost::asio::post(
            main_context,
            [this, node]()
            {
                node->done();
                running_list.erase(node);

                if (nullptr == deferred_nodes)
                {
                    for (auto* n : waiting_list)
                    {
                        if (n->is_ready())
                        {
                            waiting_list.erase(n);
                            run_node(n);
                            break;
                        }
                    }
                }
                else
                {
                    if (running_list.empty())
                    {
                        waiting_list.clear();
                        flush({});
                        run(*deferred_nodes);
                    }
                }
            }
        );
    }

private:
    std::set<nil::gate::INode*> running_list;
    std::set<nil::gate::INode*> waiting_list;

    const std::vector<std::unique_ptr<nil::gate::INode>>* deferred_nodes = nullptr;
    std::vector<std::vector<std::unique_ptr<nil::gate::ICallable<void()>>>> all_diffs;

    boost::asio::io_context& main_context;

    boost::asio::io_context exec_context;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work;
    std::vector<std::thread> exec_th;
};

int main()
{
    nil::gate::Core core;

    auto* e1_i = core.edge(0);

    auto [e2_i] = core.node(
        [](int v)
        {
            std::cout << "Main: " << v << std::endl;
            return v + 10;
        },
        {e1_i}
    );

    core.node(
        [](int b1, int b2)
        {
            std::stringstream ss;
            ss << "First: " << b1 << ":" << b2 << '\n';
            std::cout << ss.str() << std::flush;
        },
        {e1_i, e2_i}
    );

    auto [r] = core.node(
        [](nil::gate::async_outputs<int> a, const nil::gate::Core& c, int b1, int b2)
        {
            std::stringstream ss;
            ss << "Second: " << b1 << ":" << b2 << '\n';
            std::cout << ss.str() << std::flush;
            if (b1 % 2 == 0)
            {
                auto [aa] = c.batch(a);
                aa->set_value(aa->value() + 20);
            }
        },
        {3000},
        {e1_i, e2_i}
    );

    core.node(
        [](int i)
        {
            std::stringstream ss;
            ss << "printer: " << i << '\n';
            std::cout << ss.str() << std::flush;
        },
        {r}
    );

    boost::asio::io_context context;
    auto _ = boost::asio::make_work_guard(context);

    core.set_runner(std::make_unique<Runner>(context, 10));
    core.set_commit([](const nil::gate::Core& c) { c.run(); });

    std::thread mutator_th(
        [e1_i, &core]()
        {
            while (true)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                e1_i->set_value(e1_i->value() + 1);
                core.run();
            }
        }
    );

    context.run();
}
