#include <nil/cli.hpp>
#include <nil/cli/nodes/Help.hpp>
#include <nil/gate.hpp>

#include <iostream>

template <typename U>
struct T;

template <typename... A>
struct T<void(A...)>: T<std::tuple<>(A...)>
{
    using T<std::tuple<>(A...)>::T;
};

template <typename... R, typename... A>
struct T<std::tuple<R...>(A...)>
{
    T(std::string tag)
        : tag(std::move(tag))
    {
        std::cout << "constructing : " << this->tag << std::endl;
    }

    std::tuple<R...> operator()(A...) const
    {
        std::cout << "calling      : " << this->tag << std::endl;
        return std::make_tuple(R()...);
    }

    std::string tag;
};

int main()
{
    nil::gate::Core core;

    // clang-format off
    // const auto [a1, a2, a3, a4] = core.node<T<std::tuple<bool, int, double, std::string>()>>({}, "a");
    const auto a1 = core.edge<bool>();
    const auto a2 = core.edge<int>();
    const auto a3 = core.edge<double>();
    const auto a4 = core.edge<std::string>();
    const auto [b1] = core.node<T<std::tuple<std::string>(bool)>>({a1}, "b");
    const auto [c1] = core.node<T<std::tuple<double>(int)>>({a2}, "c");
    const auto [d1, d2] = core.node<T<std::tuple<float, int>(double)>>({a3}, "d");
    const auto [e1, e2] = core.node<T<std::tuple<char, bool>(std::string, float)>>({a4, d1}, "e");
    const auto [f1, f2] = core.node<T<std::tuple<bool, char>(std::string, double)>>({b1, c1}, "f");
    core.node<T<void(bool)>>({f1}, "g");
    core.node<T<void(char, char)>>({f2, e1}, "h");
    core.node<T<void(char, int)>>({e1, d2}, "i");
    // clang-format on

    a1->set_value(true);
    a2->set_value(1);
    a3->set_value(1.0);
    a4->set_value("text");

    std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
    core.run();

    a2->set_value(2);

    std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
    core.run();
}
