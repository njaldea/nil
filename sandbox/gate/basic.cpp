#include <nil/dev.hpp>
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
    explicit T(std::string init_tag)
        : tag(std::move(init_tag))
    {
        std::cout << "constructing : " << tag << std::endl;
    }

    std::tuple<R...> operator()(A... /*unused*/) const
    {
        std::cout << "calling      : " << tag << std::endl;
        return std::make_tuple(R()...);
    }

    std::string tag;
};

int main()
{
    nil::gate::Core core;
    core.set_commit(
        [](nil::gate::Core&) {
            std::cout << __FILE__ << ':' << __LINE__ << ':' << (const char*)(__FUNCTION__)
                      << std::endl;
        }
    );

    // using A = T<std::tuple<bool, int, double, std::string>()>;
    using B = T<std::tuple<std::string>(const std::unique_ptr<const bool>&)>;
    using C = T<std::tuple<double>(int)>;
    using D = T<std::tuple<float>(const double)>;
    using E = T<std::tuple<char>(const std::string&, float)>;
    using F = T<std::tuple<bool>(std::string, double)>;
    using G = T<void(bool)>;
    using H = T<void(bool, char)>;
    using I = T<void(char, float)>;

    // const auto [a1, a2, a3, a4] = core.node<A>({}, "a");
    auto* a1 = core.edge<std::unique_ptr<const bool>>();
    auto* a2 = core.edge<int>();
    auto* a3 = core.edge<double>();
    auto* a4 = core.edge<std::string>();
    const auto [b1] = core.node<B>({a1}, "b");
    const auto [c1] = core.node<C>({a2}, "c");
    const auto [d1] = core.node<D>({a3}, "d");
    const auto [e1] = core.node<E>({a4, d1}, "e");
    const auto [f1] = core.node<F>({b1, c1}, "f");
    core.node<G>({f1}, "g");
    core.node<H>({f1, e1}, "h");
    core.node<I>({e1, d1}, "i");

    {
        auto batch_o = core.batch(a2, a3);
        auto& [b_a2, b_a3] = batch_o;
        b_a2->set_value(1111);
        b_a3->set_value(1332.0);
    }

    nil::log();
    core.run();

    a2->set_value(2);

    nil::log();
    core.run();
}
