#include <nil/dev.hpp>
#include <nil/gate.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include <iostream>
#include <thread>

std::tuple<float> deferred(nil::gate::async_outputs<int> z, const nil::gate::Core& core, bool a)
{
    std::cout << "deferred: " << a << std::endl;
    if (a)
    {
        auto [zz] = core.batch(z);
        // this will be triggered on next core.run()
        zz->set_value(zz->value() + 100);
        return {321.0f};
    }
    return {432.0f};
}

void foo(int v)
{
    std::cout << "foo: " << v << std::endl;
}

int main()
{
    boost::asio::io_context context;

    nil::gate::Core core;

    auto* a = core.edge(false);
    const auto [f, x] = core.node({9000}, {a}, &deferred);
    core.node({x}, [](int v) { std::cout << "printer<int>: " << v << std::endl; });
    core.node({f}, [](float v) { std::cout << "printer<int>: " << v << std::endl; });
    core.node({x}, &foo);

    std::thread gate_thread(
        [&context]()
        {
            auto g = boost::asio::make_work_guard(context);
            context.run();
        }
    );

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        boost::asio::post(
            context,
            [a, &core]()
            {
                a->set_value(!a->value());
                core.run();
            }
        );
    }

    gate_thread.join();
}
