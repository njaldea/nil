#include <nil/xit.hpp>

#include <unordered_map>

struct JSON
{
    std::string data = "{}";
};

namespace nil::xit
{
    template <>
    struct buffer_type<JSON>
    {
        static JSON deserialize(const void* data, std::uint64_t size)
        {
            return {.data = std::string(static_cast<const char*>(data), size)};
        }

        static std::vector<std::uint8_t> serialize(const JSON& value)
        {
            return {value.data.begin(), value.data.end()};
        }
    };
}

int main()
{
    const auto source_path = std::filesystem::path(__FILE__).parent_path();

    auto http_server = nil::xit::make_server({
        .source_path = source_path,
        .port = 1101,
        .buffer_size = 1024ul * 1024ul * 100ul //
    });
    auto core = nil::xit::make_core(http_server);
    set_relative_directory(core, source_path);

    const auto tmp_dir = std::filesystem::temp_directory_path() / "nil-xit-gtest";
    std::filesystem::remove_all(tmp_dir);
    set_cache_directory(core, tmp_dir);

    {
        auto& frame = add_unique_frame(core, "demo", "gui/Main.svelte");
        add_value(frame, "scenes", JSON{R"({ "scenes": ["a", "b"] })"});
    }

    std::unordered_map<std::string, JSON> data = {
        {"a", JSON(R"({ "hello": true })")},
        {"b", JSON(R"({ "world": true })")} //
    };

    auto& test_frame = add_tagged_frame(core, "test_frame", "gui/TestFrame.svelte");
    auto& value = add_value(
        test_frame,
        "scene",
        [&](std::string_view tag) { return data[std::string(tag)]; }
    );

    auto& editor_frame = add_tagged_frame(core, "editor_frame", "gui/EditorFrame.svelte");
    add_value(
        editor_frame,
        "scene",
        [&](std::string_view tag) -> JSON { return data[std::string(tag)]; },
        [&](std::string_view tag, JSON new_data)
        {
            post(tag, value, new_data);
            data[std::string(tag)] = std::move(new_data);
        }
    );

    start(http_server);
    return 0;
}
