#include <nil/cli.hpp>
#include <nil/cli/nodes/Help.hpp>
#include <nil/gate.hpp>

#include <iostream>
#include <unordered_map>

struct A
{
    void operator()(int i, bool b) const
    {
        std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
        std::cout << i << ":" << b << std::endl;
    }
};

struct V
{
    V(std::string v)
    {
        std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
        std::cout << v << std::endl;
    }

    std::tuple<int, bool> operator()() const
    {
        std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
        return {100, false};
    }
};

int main()
{
    nil::gate::Core core;

    const auto edges = core.node<V>({}, "abcd");
    const auto edgei = core.edge<int>();
    const auto edgeo = core.edge<bool>();
    core.node<A>({edgei, std::get<1>(edges)});

    std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
    edgei->set_value(123);
    edgeo->set_value(false);
    core.run();
    edgei->set_value(321);
    std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
    core.run();
    std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
}
