#pragma once

#include "codec.hpp"

#include <nil/utils/traits/callable.hpp>

#include <functional>
#include <string>
#include <unordered_map>

namespace nil::service
{
    namespace detail
    {
        template <typename T>
        struct traits_impl
        {
            // TODO:
            // fix for msvc. error coming from if constexpr code below
            // using type = char;
        };

        template <typename T>
        struct traits_impl<nil::utils::traits::types<>(const std::string&, T)>
        {
            using type = std::decay_t<T>;
        };

        template <typename T>
        struct traits: traits_impl<typename nil::utils::traits::callable<T>::type>
        {
        };

        template <typename T>
        using traits_t = typename traits<T>::type;
    }

    template <typename Indexer>
    class TypedHandler
    {
    public:
        template <typename Message>
        TypedHandler& add(Indexer type, Message handler)
        {
            handlers.emplace(
                type,
                [h = std::move(handler)] //
                (const std::string& id, const void* data, std::uint64_t size)
                {
                    if constexpr (nil::utils::traits::callable<Message>::inputs::size == 2)
                    {
                        using T = detail::traits_t<Message>;
                        h(id, codec<T>::deserialize(data, size));
                    }
                    else if constexpr (nil::utils::traits::callable<Message>::inputs::size == 1)
                    {
                        (void)data;
                        (void)size;
                        h(id);
                    }
                    else
                    {
                        (void)id;
                        (void)data;
                        (void)size;
                        h();
                    }
                }
            );
            return *this;
        }

        void operator()(const std::string& id, const void* data, std::uint64_t size) const
        {
            const auto m = static_cast<const std::uint8_t*>(data);
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
