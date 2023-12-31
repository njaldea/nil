#include <iostream>

#include <nil/pulse.hpp>

int main()
{
    std::vector<std::function<void()>> unsubscribes;
    std::cout << "setup\n";
    {
        auto d = nil::pulse::Data<bool>::create(false);
        std::cout << "data create\n";
        {
            unsubscribes.push_back(d->subscribe( //
                [](bool v) { std::cout << "subscribe 1: " << v << std::endl; }
            ));
            std::cout << "unsub 1\n";
            unsubscribes.push_back(d->subscribe( //
                [](bool v) { std::cout << "subscribe 2: " << v << std::endl; }
            ));
            std::cout << "unsub 2\n";
            auto unsub3 = d->subscribe( //
                [](bool v) { std::cout << "subscribe 3: " << v << std::endl; }
            );
            std::cout << "unsub 3 weak\n";
            d->set(true);
            std::cout << "set true\n";
            unsubscribes[0]();
            std::cout << "unsub 1 done\n";
            auto copy = unsub3;
            unsub3();
            copy();
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
