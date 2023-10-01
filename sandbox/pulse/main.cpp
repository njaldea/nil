#include <iostream>

#include <nil/pulse.hpp>

int main()
{
    std::vector<std::function<void()>> unsubscribes;
    std::cout << "setup\n";
    {
        auto d = Data<bool>::create(false);
        std::cout << "data create\n";
        {
            unsubscribes.emplace_back(
                d->subscribe([](bool v) { std::cout << "subscribe 1: " << v << std::endl; })
            );
            std::cout << "unsub 1\n";
            unsubscribes.emplace_back(
                d->subscribe([](bool v) { std::cout << "subscribe 2: " << v << std::endl; })
            );
            std::cout << "unsub 2\n";
            auto unsub3 = d->subscribe(
                [](bool v) { std::cout << "subscribe 3: " << v << std::endl; },
                Data<bool>::Mode::Weak
            );
            std::cout << "unsub 3 weak\n";
            d->set(true);
            std::cout << "set true\n";
            unsubscribes[0]();
            std::cout << "unsub 1 done\n";
            d->set(false);
            std::cout << "set false\n";
        }
        std::cout << "unsub 3 destroyed\n";
        d->set(true);
        std::cout << "set true\n";
        std::cout << d->get() << std::endl;
    }
    std::cout << "data destroyed\n";
}
