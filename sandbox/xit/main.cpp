// #include <nil/service/http/Server.hpp>
#include <nil/service/concat.hpp>
#include <nil/service/ws/Server.hpp>

#include "codec.hpp"
#include "ws_install.hpp"

#include <xit/messages/message.pb.h>

#include <thread>

int main()
{
    nil::service::ws::Server server({.port = 1101});
    ws_install(server);

    std::thread th(
        [&]()
        {
            std::string line;
            bool first = true;
            do // NOLINT
            {
                if (!first)
                {
                    nil::xit::proto::BindingUpdate msg;
                    msg.set_group(0);
                    auto* binding = msg.mutable_binding();
                    binding->set_tag("binding_0_1");
                    binding->set_value_str(line);
                    server.publish(
                        nil::service::concat(nil::xit::proto::MessageType_BindingUpdate, msg)
                    );
                }
                first = false;

                std::cout << "input here: ";
            } while (std::getline(std::cin, line));
        }
    );

    server.run();
    return 0;
}
