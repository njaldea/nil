#pragma once

#include <nil/xit/gtest.hpp>

#include <nil/xit/buffer_type.hpp>

#include <nlohmann/json.hpp>

namespace nil::xit
{
    // this is necessary when publishing a custom data through the network going to the UI
    template <>
    struct buffer_type<nlohmann::json>
    {
        static nlohmann::json deserialize(const void* data, std::uint64_t size);
        static std::vector<std::uint8_t> serialize(const nlohmann::json& value);
    };
}

struct Ranges
{
    std::int64_t v1;
    std::int64_t v2;
    std::int64_t v3;

    bool operator==(const Ranges& o) const;
};

nlohmann::json as_json(std::istream& iss);
Ranges as_range(std::istream& iss);

template <typename T = nlohmann::json>
auto from_json_ptr(const std::string& json_ptr)
{
    struct Accessor
    {
        T get(const nlohmann::json& data) const
        {
            return data[json_ptr];
        }

        void set(nlohmann::json& data, T new_data) const
        {
            data[json_ptr] = std::move(new_data);
        }

        nlohmann::json::json_pointer json_ptr;
    };

    return Accessor{nlohmann::json::json_pointer(json_ptr)};
}

using nil::xit::gtest::from_data;
using nil::xit::gtest::from_file;
using nil::xit::gtest::from_member;
