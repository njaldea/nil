#include <nil/dev.hpp>
#include <nil/gate.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include <iostream>
#include <thread>

struct Deferred
{
    std::tuple<float> operator()(nil::gate::async_edges<int> z, bool a)
    {
        std::cout << __FILE__ << ':' << __LINE__ << ':' << (const char*)(__FUNCTION__) << std::endl;
        std::cout << a << std::endl;
        if (a)
        {
            auto* zz = get<0>(z);
            // this will be triggered on next core.run()
            zz->set_value(zz->value() + 100);
        }
        return {a ? 321.0f : 432.0f};
    }
};

template <typename T>
struct Printer
{
    void operator()(const T& v)
    {
        std::cout << __FILE__ << ':' << __LINE__ << ':' << (const char*)(__FUNCTION__) << std::endl;
        std::cout << v << std::endl;
    }
};

int main()
{
    boost::asio::io_context context;

    nil::gate::Core core;
    const auto run = [&core]() { core.run(); };

    auto* a = core.edge(false);
    const auto [f, x] = core.node<Deferred>({9000}, {a});
    core.node<Printer<int>>({x});
    core.node<Printer<float>>({f});

    core.run();

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
        a->set_value(!a->value());
        boost::asio::post(context, run);
    }

    gate_thread.join();
}
