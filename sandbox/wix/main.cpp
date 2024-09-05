// #include <nil/service/http/Server.hpp>
#include <nil/service/ws/Server.hpp>

#include "ws_install.hpp"

int main()
{
    nil::service::ws::Server server({.port = 1101});
    ws_install(server);
    server.run();
    return 0;
}
