#include "nil/gate/runners/asio.hpp"
#include "nil/gate/types.hpp"
#include <nil/dev.hpp>
#include <nil/gate.hpp>

#include <nil/gate/bias/nil.hpp>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include <iostream>
#include <set>
#include <sstream>
#include <thread>

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
            std::ostringstream ss;
            ss << "First: " << b1 << ":" << b2 << '\n';
            std::cout << ss.str() << std::flush;
        },
        {e1_i, e2_i}
    );

    auto [r] = core.node(
        [](nil::gate::async_outputs<int> a, const nil::gate::Core& c, int b1, int b2)
        {
            std::ostringstream ss;
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

    core.set_runner(std::make_unique<nil::gate::runners::Asio>(10));
    core.set_commit([](const nil::gate::Core& c) { c.run(); });

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        auto [b] = core.batch(e1_i);
        b->set_value(b->value() + 1);
    }
}
