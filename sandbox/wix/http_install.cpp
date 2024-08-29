#include "http_install.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

const auto path = std::filesystem::path(__FILE__).parent_path();

void http_install(nil::service::http::Server& server)
{
    server.use(
        "/",
        "text/html",
        [](std::ostream& os) { os << std::fstream(path / "index.html").rdbuf(); }
    );
    server.use(
        "/index.js",
        "application/javascript",
        [](std::ostream& os) { os << std::fstream(path / "front_end/bundle/index.js").rdbuf(); }
    );
    server.use(
        "/wix/message.proto",
        "application/octet-stream",
        [](std::ostream& os) { os << std::fstream(path / "message.proto").rdbuf(); }
    );
    server.on_ready(                                                  //
        [](const auto& id)                                            //
        {                                                             //
            std::cout << "ready: http://localhost:1101" << std::endl; //
            std::cout << "ready: " << id.text << std::endl;           //
        }
    );
}
