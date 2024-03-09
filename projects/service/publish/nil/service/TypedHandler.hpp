#pragma once

#include "codec.hpp"

#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace nil::service
{
    template <typename Indexer>
    class TypedHandler
    {
        struct AutoCast
        {
            template <typename T>
            operator T() const // NOLINT
            {
                return codec<T>::deserialize(d, *s);
            }

            const void* d;
            std::uint64_t* s;
        };

    public:
        template <typename Handler>
        TypedHandler& add(Indexer type, Handler handler)
        {
            handlers.emplace(
                type,
                [h = std::move(handler)] //
                (                        //
                    [[maybe_unused]] const std::string& id,
                    [[maybe_unused]] const void* data,
                    [[maybe_unused]] std::uint64_t size
                )
                {
                    if constexpr (std::is_invocable_v<Handler>)
                    {
                        h();
                    }
                    else if constexpr (std::is_invocable_v<Handler, const std::string&>)
                    {
                        h(id);
                    }
                    else
                    {
                        h(id, AutoCast(data, &size));
                    }
                }
            );
            return *this;
        }

        void operator()(const std::string& id, const void* data, std::uint64_t size) const
        {
            const auto* const m = static_cast<const std::uint8_t*>(data);
            const auto o_size = size;
            const auto t = codec<Indexer>::deserialize(data, size);
            const auto it = handlers.find(t);
            if (it != handlers.end() && it->second)
            {
                it->second(id, m + o_size - size, size);
            }
        }

    private:
        using Handler = std::function<void(const std::string&, const void*, std::uint64_t)>;
        std::unordered_map<Indexer, Handler> handlers;
    };
}
