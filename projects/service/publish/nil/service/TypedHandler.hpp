#pragma once

#include "IService.hpp"
#include "codec.hpp"

#include <string>
#include <unordered_map>

namespace nil::service
{
    template <typename Indexer>
    class TypedHandler final
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

        template <typename T>
        struct Callable final: detail::ICallable<const void*, std::uint64_t>
        {
            Callable() = delete;

            Callable(Callable&&) noexcept = delete;
            Callable& operator=(Callable&&) noexcept = delete;

            Callable(const Callable&) = delete;
            Callable& operator=(const Callable&) = delete;

            explicit Callable(T init_handler)
                : handler(std::move(init_handler))
            {
            }

            ~Callable() noexcept override = default;

            void call(const std::string& id, const void* data, std::uint64_t size) override
            {
                if constexpr (std::is_invocable_v<T>)
                {
                    handler();
                }
                else if constexpr (std::is_invocable_v<T, const std::string&>)
                {
                    handler(id);
                }
                else
                {
                    handler(id, AutoCast(data, &size));
                }
            }

            T handler;
        };

    public:
        TypedHandler() = default;
        ~TypedHandler() noexcept = default;

        TypedHandler(TypedHandler&&) = default;
        TypedHandler& operator=(TypedHandler&&) = default;

        TypedHandler(const TypedHandler&) = delete;
        TypedHandler& operator=(const TypedHandler&) = delete;

        template <typename Handler>
        TypedHandler add(Indexer type, Handler handler) &&
        {
            handlers.emplace(type, std::make_unique<Callable<Handler>>(std::move(handler)));
            return std::move(*this);
        }

        template <typename Handler>
        TypedHandler& add(Indexer type, Handler handler) &
        {
            handlers.emplace(type, std::make_unique<Callable<Handler>>(std::move(handler)));
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
                it->second->call(id, m + o_size - size, size);
            }
        }

    private:
        std::unordered_map<Indexer, MessageHandler> handlers;
    };
}
