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
    struct Xit;

    struct Deleter
    {
        void operator()(Xit*) const;
    };

    std::unique_ptr<Xit, Deleter> make_xit(nil::service::IService& service);

    Frame& frame(Xit& x, std::string, std::filesystem::path);
    Binding<std::int64_t>& bind(
        Frame& f,
        std::string,
        std::int64_t,
        std::function<void(std::int64_t)> = {}
    );
    Binding<std::string>& bind(
        Frame& f,
        std::string,
        std::string,
        std::function<void(const std::string&)> = {}
    );

    void post(Binding<std::int64_t>& b, std::int64_t v);
    void post(Binding<std::string>& b, std::string v);
}
