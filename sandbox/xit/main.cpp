#include <nil/xit.hpp>

#include <nlohmann/json.hpp>

#include <iostream>
#include <unordered_map>

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

struct TagInfo
{
    nlohmann::json input;
};

using Data = std::unordered_map<std::string, TagInfo, transparent::Hash, transparent::Equal>;

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

    struct App
    {
        Data data;

        nlohmann::json get_value(std::string_view tag) const
        {
            if (const auto it = data.find(tag); it != data.end())
            {
                return it->second.input;
            }
            return nlohmann::json::object();
        }

        void set_value(std::string_view tag, nlohmann::json value)
        {
            if (const auto it = data.find(tag); it != data.end())
            {
                it->second.input = std::move(value);
            }
            else
            {
                data.emplace(tag, std::move(value));
            }
        }
    };

    auto app = App{.data=Data{
        {"a", {nlohmann::json::array({nlohmann::json::object({
            {"x", nlohmann::json::array({"giraffes", "orangutans", "monkeys"})},
            {"y", nlohmann::json::array({20, 14, 23})},
            {"type", "bar"}
        })})}},
        {"b", {nlohmann::json::array({nlohmann::json::object({
            {"x", nlohmann::json::array({"giraffes", "orangutans", "monkeys"})},
            {"y", nlohmann::json::array({20, 14, 23})},
            {"type", "bar"}
        })})}} //
    }};

    auto& view_frame = add_tagged_frame(core, "view_frame", "gui/ViewFrame.svelte");
    auto& editor_frame = add_tagged_frame(core, "editor_frame", "gui/EditorFrame.svelte");
    auto& value = add_value(
        view_frame,
        "scene",
        [&](auto tag) { return app.get_value(tag); },
        [](auto tag, const nlohmann::json& v)
        { std::cout << "tag: " << tag << " - " << v.dump() << std::endl; } //
    );
    add_value(
        editor_frame,
        "scene",
        [&](auto tag) { return app.get_value(tag); },
        [&](std::string_view tag, nlohmann::json new_data)
        {
            post(tag, value, new_data);
            app.set_value(tag, std::move(new_data));
        } //
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
