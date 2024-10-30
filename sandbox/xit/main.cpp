
#include <nil/xit.hpp>

#include <nil/gate.hpp>
#include <nil/gate/bias/nil.hpp>
#include <nil/gate/runners/NonBlocking.hpp>

#include <nlohmann/json.hpp>

#include <iostream>

namespace transparent
{
    struct Hash
    {
        using is_transparent = void;

        std::size_t operator()(std::string_view txt) const
        {
            return std::hash<std::string_view>()(txt);
        }

        std::size_t operator()(const std::string& txt) const
        {
            return this->operator()(std::string_view(txt));
        }
    };

    struct Equal
    {
        using is_transparent = void;

        bool operator()(const std::string& lhs, const std::string& rhs) const
        {
            return lhs == rhs;
        }

        bool operator()(std::string_view lhs, const std::string& rhs) const
        {
            return lhs == rhs;
        }
    };
}

namespace nil::xit
{
    template <>
    struct buffer_type<nlohmann::json>
    {
        static nlohmann::json deserialize(const void* data, std::uint64_t size)
        {
            return nlohmann::json::parse(std::string_view(static_cast<const char*>(data), size));
        }

        static std::vector<std::uint8_t> serialize(const nlohmann::json& value)
        {
            auto s = value.dump();
            return {s.begin(), s.end()};
        }
    };
}

int main()
{
    const auto source_path = std::filesystem::path(__FILE__).parent_path();
    const auto http_server = nil::xit::make_server({
        .source_path = source_path,
        .port = 1101,
        .buffer_size = 1024ul * 1024ul * 100ul //
    });
    const auto core = nil::xit::make_core(http_server);
    set_relative_directory(core, source_path);

    const auto tmp_dir = std::filesystem::temp_directory_path() / "nil-xit-gtest";
    std::filesystem::remove_all(tmp_dir);
    set_cache_directory(core, tmp_dir);

    auto& main_frame = add_unique_frame(core, "demo", "gui/Main.svelte");
    add_value(main_frame, "scenes", nlohmann::json::parse(R"({ "scenes": ["", "a", "b"] })"));
    auto& selected_view = add_value(main_frame, "selected", 0L);

    // TODO: 2 copies needed for thread safety
    //  -   one for the messaging layer
    //  -   one for nil::gate
    // an alternative would be to write a logic to lock the resource for both threads
    // or just use an immediate (but debounced) runner for nil::gate

    nil::gate::Core gate_core;
    gate_core.set_runner<nil::gate::runners::NonBlocking>();

    const auto add_test_data = [&]()
    {
        auto data = nlohmann::json::array({nlohmann::json::object(
            {{"x", nlohmann::json::array({"giraffes", "orangutans", "monkeys"})},
             {"y", nlohmann::json::array({20, 14, 23})},
             {"type", "bar"}}
        )});
        auto* edge = gate_core.edge(data);
        return std::make_tuple(std::move(data), edge);
    };
    auto [d1, e1] = add_test_data();
    auto [d2, e2] = add_test_data();

    auto& view_frame = add_tagged_frame(core, "view_frame", "gui/ViewFrame.svelte");
    auto& editor_frame = add_tagged_frame(core, "editor_frame", "gui/EditorFrame.svelte");

    auto get_value = [&](auto tag)
    {
        if (tag == "a")
        {
            return d1;
        }
        if (tag == "b")
        {
            return d2;
        }
        return nlohmann::json::array();
    };

    auto& value = add_value(view_frame, "scene", get_value);

    gate_core.node(
        [&, tag = "a"](const nlohmann::json& data)
        {
            std::cout << "run (test) " << tag << std::endl;
            post(tag, value, data);
        },
        {e1}
    );
    gate_core.node(
        [&, tag = "b"](const nlohmann::json& data)
        {
            std::cout << "run (test) " << tag << std::endl;
            post(tag, value, data);
        },
        {e2}
    );

    add_value(
        editor_frame,
        "scene",
        get_value,
        [&](std::string_view tag, nlohmann::json new_data)
        {
            std::cout << "set_value " << tag << std::endl;
            if (tag == "a")
            {
                d1 = new_data;
                e1->set_value(std::move(new_data));
                gate_core.commit();
            }
            else if (tag == "b")
            {
                d2 = new_data;
                e2->set_value(std::move(new_data));
                gate_core.commit();
            }
        }
    );

    {
        auto& frame = add_unique_frame(core, "cli", "gui/CLI.svelte");
        add_signal(
            frame,
            "message",
            [&](std::string_view message)
            {
                if (message == "unload")
                {
                    post(selected_view, 0);
                }
                else if (message == "load a")
                {
                    post(selected_view, 1);
                }
                else if (message == "load b")
                {
                    post(selected_view, 2);
                }
            }
        );
    }

    start(http_server);
    return 0;
}
