#include <nil/service/ws/Server.hpp>

#include <nil/xit.hpp>

#include <iostream>
#include <thread>

struct JSON
{
    std::string buffer;
};

namespace nil::xit
{
    template <>
    struct buffer_type<JSON>
    {
        static JSON deserialize(const void* data, std::uint64_t size)
        {
            return {.buffer = {static_cast<const char*>(data), size}};
        }

        static std::vector<std::uint8_t> serialize(const JSON& value)
        {
            return {value.buffer.begin(), value.buffer.end()};
        }
    };
}

void add_group(nil::xit::Core& core)
{
    add_frame(core, "group", std::filesystem::path(__FILE__).parent_path() / "gui/GroupUp.svelte");
}

void add_json_editor(nil::xit::Core& core)
{
    auto& frame = add_frame(
        core,
        "json_editor", // frame id
        std::filesystem::path(__FILE__).parent_path() / "gui/JsonEditor.svelte"
    );
    bind(frame, "json_binding", JSON{.buffer = R"({ "hello": "hello this is buffer" })"});
}

auto& add_base(nil::xit::Core& core)
{
    auto& frame = add_frame(
        core,
        "id-1", // frame id
        std::filesystem::path(__FILE__).parent_path() / "gui/Markup.svelte"
    );

    return bind(
        frame,
        "binding_0_1",
        "hello world",
        // this is to test gui -> cpp data flow
        [](const std::string& value) { std::cout << "value changed: " << value << std::endl; }
    );
}

int main()
{
    nil::service::ws::Server server({.port = 1101});
    // https://xit-ui.vercel.app/view/{server or ip:port}/{frame id}
    server.on_ready(                                                                      //
        [](const auto& id)                                                                //
        {                                                                                 //
            std::cout << "ready ws      : " << id.text << '\n';                           //
            std::cout << " ui is at     : \n";                                            //
            std::cout << " -  https://xit-ui.vercel.app/view/localhost:1101/group\n";     //
            std::cout << " -  https://xit-ui.vercel.app/view/localhost:1101/id-1\n";      //
            std::cout << " -  https://xit-ui.vercel.app/view/localhost:1101/json_editor"; //
            std::cout << std::endl;                                                       //
        }
    );

    auto core = nil::xit::make_core(server);
    add_group(core);
    add_json_editor(core);
    auto& str_bind = add_base(core);

    std::thread th(
        [&]()
        {
            std::string line;
            std::cout << "input here: ";
            while (std::getline(std::cin, line))
            {
                // will not update the local data inside str_bind
                post(str_bind, line);

                // if local data needs to be overwritten, use `set` instead
                // this will update the local data without waiting for an update
                // from the UI.

                // set(str_bind, line);
                std::cout << "input here: ";
            }
        }
    );

    server.run();
    th.join();
    return 0;
}
