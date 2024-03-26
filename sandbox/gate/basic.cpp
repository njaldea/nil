#include <nil/dev.hpp>
#include <nil/gate.hpp>

#include <nil/gate/bias/nil.hpp>

#include <iostream>

template <typename U>
struct T;

template <typename... A>
struct T<void(A...)>: T<std::tuple<>(A...)>
{
    using T<std::tuple<>(A...)>::T;
};

template <typename R, typename... A>
struct T<R(A...)>
{
    explicit T(std::string init_tag)
        : tag(std::move(init_tag))
    {
        std::cout << "constructing : " << tag << std::endl;
    }

    R operator()(A... /*unused*/) const
    {
        std::cout << "calling      : " << tag << std::endl;
        return R();
    }

    std::string tag;
};

int main()
{
    nil::gate::Core core;
    core.set_commit([](const nil::gate::Core&) { nil::log(); });

    using A = T<void()>;
    using B = T<std::tuple<std::string>(const std::unique_ptr<const bool>&)>;
    using C = T<std::tuple<double>(int)>;
    using D = T<std::tuple<float>(const double)>;
    using E = T<std::tuple<char>(const std::string&, float)>;
    using F = T<bool(std::string, double)>;
    using G = T<void(bool)>;
    using H = T<void(bool, char)>;
    using I = T<void(char, float)>;

    auto* a1 = core.edge(std::unique_ptr<bool>());
    auto* a2 = core.edge(0);
    auto* a3 = core.edge(0.0);
    auto* a4 = core.edge(std::string());

    core.node(A("a"));
    const auto [b1] = core.node(B("b"), {a1});
    const auto [c1] = core.node(C("c"), {a2});
    const auto [d1] = core.node(D("d"), {a3});
    const auto [e1] = core.node(E("e"), {a4, d1});
    const auto [f1] = core.node(F("f"), {b1, c1});
    core.node(G("g"), {f1});
    core.node(H("h"), {f1, e1});
    core.node(I("i"), {e1, d1});

    {
        auto [b_a2, b_a3] = core.batch(a2, a3);
        b_a2->set_value(1111);
        b_a3->set_value(1332.0);
    }

    nil::log();
    core.run();

    a2->set_value(2);

    nil::log();
    core.run();
}
