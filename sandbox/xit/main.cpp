// #include <nil/service/http/Server.hpp>
#include <nil/service/IService.hpp>
#include <nil/service/concat.hpp>
#include <nil/service/ws/Server.hpp>

#include "codec.hpp"
#include "xit.hpp"

#include <xit/messages/message.pb.h>

#include <thread>

int main()
{
    nil::service::ws::Server server({.port = 1101});
    auto x = nil::xit::make_xit(server);
    auto& ff = frame(*x, "id-1", "/home/njaldea/repo/cpp/nil/sandbox/xit/gui/Markup.svelte");
    auto& str_bind = bind(
        ff,
        "binding_0_1",
        "hello world",
        [](const std::string& value) { std::cout << "value changed: " << value << std::endl; }
    );

    std::thread th(
        [&]()
        {
            std::string line;
            std::cout << "input here: ";
            while (std::getline(std::cin, line))
            {
                post(str_bind, line);
                std::cout << "input here: ";
            }
        }
    );

    server.run();
    return 0;
}
