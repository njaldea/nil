#include <nil/dev.hpp>
#include <nil/gate.hpp>

#include <nil/gate/bias/nil.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include <iostream>
#include <thread>

float deferred(nil::gate::async_outputs<int> z, const nil::gate::Core& core, bool a)
{
    std::cout << "deferred: " << a << std::endl;
    if (a)
    {
        auto [zz] = core.batch(z);
        // this will be triggered on next core.run()
        zz->set_value(zz->value() + 100);
        return 321.0f;
    }
    return 432.0f;
}

std::reference_wrapper<const float> switcher(bool a, const float& l, const float& b)
{
    return a ? std::ref(l) : std::ref(b);
}

void foo(int v)
{
    std::cout << "foo: " << v << std::endl;
}

int main()
{
    nil::gate::Core core;

    const auto printer_i = [](int v) { std::cout << "printer<int>: " << v << std::endl; };
    const auto printer_f = [](float v) { std::cout << "printer<float>: " << v << std::endl; };

    auto ref = 100.f;
    auto* a = core.edge(false);
    auto* l = core.edge(std::cref(ref));
    auto* r = core.edge(200.f);
    const auto [f, x] = core.node(&deferred, {9000}, {a});
    const auto [fs] = core.node(&switcher, {a, l, r});
    core.node(printer_i, {x});

    core.node(printer_f, {fs});
    core.node(&foo, {x});

    boost::asio::io_context context;
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
