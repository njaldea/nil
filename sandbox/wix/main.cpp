// #include <nil/service/http/Server.hpp>
#include <nil/service/ws/Server.hpp>

// #include "http_install.hpp"
// #include "impl.hpp"
#include "ws_install.hpp"

int main()
{
    // Block block1 = {"top block", {}};
    // Mutator mutator;
    // std::int32_t next_id = 0;
    // install(mutator, block1, next_id);
    // auto& block2 = add_block(mutator, block1, "bottom block");
    // install(mutator, block2, next_id);

    // nil::service::http::Server server({.port = 1101});
    // http_install(server);
    // ws_install(server.use_ws("/ws"), mutator, block1);
    nil::service::ws::Server server({.port = 1101});
    ws_install(server);
    server.run();
    return 0;
}
