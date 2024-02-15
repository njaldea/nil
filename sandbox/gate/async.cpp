#include <nil/dev.hpp>
#include <nil/gate.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include <iostream>
#include <thread>

struct Deferred
{
    void operator()(bool a)
    {
        std::cout << __FILE__ << ':' << __LINE__ << ':' << (const char*)(__FUNCTION__) << std::endl;
        std::cout << a << std::endl;
        if (a)
        {
            x->set_value(x->value() + 100);
        }
    }

    nil::gate::MutableEdge<int>* x;
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

//  TODO:
//   -  Deferred should receive all of the inputs + a tuple of outputs (MutableEdge)
//   -  Core should have that is different from node. node_async will be responsible in doing the
//   following:
//
int main()
{
    boost::asio::io_context context;
    auto g = boost::asio::make_work_guard(context);

    nil::gate::Core core;

    auto* a = core.edge(false);
    auto* x = core.edge(200);
    core.node<Deferred>({a}, x);
    core.node<Printer<int>>({x});

    auto gate_thread = std::thread([&context]() { context.run(); });
    boost::asio::post(context, [&core]() { core.run(); });

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        a->set_value(!a->value());
        boost::asio::post(context, [&core, a]() { core.run(); });
    }
}
