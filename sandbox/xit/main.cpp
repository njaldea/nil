#include <nil/xit.hpp>

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

    start(http_server);
    return 0;
}
