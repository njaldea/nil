#pragma once

#include <nil/service/IService.hpp>

#include <filesystem>
#include <functional>
#include <string>

namespace nil::xit
{
    template <typename T>
    struct Binding;

    struct Frame;
    struct Core;

    std::unique_ptr<Core, void (*)(Core*)> make_core(nil::service::IService& service);

    Frame& add_frame(Core& core, std::string id, std::filesystem::path path);

    template <typename CorePtr>
        requires requires(CorePtr arg) {
            { *arg } -> std::same_as<Core&>;
        }
    Frame& add_frame(CorePtr& core_ptr, std::string id, std::filesystem::path path)
    {
        return add_frame(*core_ptr, std::move(id), std::move(path));
    }

    Binding<std::int64_t>& bind(
        Frame& frame,
        std::string tag,
        std::int64_t value,
        std::function<void(std::int64_t)> on_change = {}
    );
    Binding<std::string>& bind(
        Frame& frame,
        std::string tag,
        std::string value,
        std::function<void(const std::string&)> on_change = {}
    );

    void post(Binding<std::int64_t>& binding, std::int64_t value);
    void post(Binding<std::string>& binding, std::string value);
}
