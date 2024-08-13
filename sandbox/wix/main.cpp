#include <nil/service.hpp>
#include <nil/service/ws/Server.hpp>

#include <gen/wix/messages/sample.pb.h>

#include "serve.hpp"

#include <iostream>
#include <string>
#include <thread>

int main()
{
    std::thread http(serve);

    nil::service::ws::Server service({.port = 1101});
    service.on_connect(                        //
        [&service](const nil::service::ID& id) //
        {
            std::cout << "connect: " << id.text << std::endl;
            nil::wix::proto::Sample msg;
            msg.set_nil_bool(true);
            msg.set_nil_number(100);
            msg.set_nil_string("hello from server");
            const auto str = msg.SerializeAsString();
            std::cout << "msg: " << str.size() << std::endl;
            service.send(id, str);
        }
    );
    service.on_disconnect(             //
        [](const nil::service::ID& id) //
        { std::cout << "disconnect: " << id.text << std::endl; }
    );
    service.on_message(
        [](const nil::service::ID& id, const void* data, std::uint64_t size)
        {
            const auto msg = nil::service::type_cast<std::string>(data, size);
            std::cout << "message: " << id.text << ":" << msg << std::endl;
        }
    );
    service.run();
    return 0;
}
