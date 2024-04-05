#include <nil/dev.hpp>
#include <nil/gate.hpp>
#include <nil/gate/runners/boost_asio.hpp>
#include <nil/gate/types.hpp>

#include <nil/gate/bias/nil.hpp>

#include <boost/asio/executor_work_guard.hpp>

#include <format>
#include <iostream>
#include <thread>

int main()
{
    nil::gate::Core core;

    auto* e1_i = core.edge(0);

    auto [e2_i] = core.node(
        [](int v)
        {
            std::cout << std::format("Main: {}\n", v) << std::flush;
            return v + 10;
        },
        {e1_i}
    );

    core.node(
        [](int b1, int b2) { std::cout << std::format("First: {} : {}\n", b1, b2) << std::flush; },
        {e1_i, e2_i}
    );

    auto [r] = core.node(
        [](const nil::gate::Core& c, nil::gate::async_outputs<int> a, int b1, int b2)
        {
            std::cout << std::format("Second: {} : {}\n", b1, b2) << std::flush;
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

    core.set_runner(std::make_unique<nil::gate::runners::Asio>(10));
    core.set_commit([](const nil::gate::Core& c) { c.run(); });

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        auto [b] = core.batch(e1_i);
        b->set_value(b->value() + 1);
    }
}
