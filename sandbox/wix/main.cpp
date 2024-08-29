#include <nil/service/http/Server.hpp>

#include "http_install.hpp"
#include "ws_install.hpp"

#include <string>

int main()
{
    std::int32_t next_id = 0;
    Block block1 = {"top block", {}};
    install(block1, next_id);
    install(add_block(block1, "bottom block"), next_id);

    nil::service::http::Server server({.port = 1101});
    http_install(server);
    ws_install(server.use_ws("/ws"), block1);
    server.run();
    return 0;
}
