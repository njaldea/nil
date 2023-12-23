#pragma once

#include "IService.hpp"

#include <memory>

namespace nil::service
{
    namespace detail
    {
        template <typename T>
        struct traits: traits<decltype(&T::operator())>
        {
        };

        template <typename T, typename Arg>
        struct traits<void (T::*)(const std::string&, const Arg&) const>
            : traits<void(const std::string&, const Arg&)>
        {
        };

        template <typename T, typename Arg>
        struct traits<void (T::*)(const std::string&, const Arg&)>
            : traits<void(const std::string&, const Arg&)>
        {
        };

        template <typename Arg>
        struct traits<void(const std::string&, const Arg&)>
        {
            using type = Arg;
        };

        template <typename T>
        using traits_t = typename traits<T>::type;
    }

    template <typename T, typename = void>
    struct codec
    {
        static std::vector<std::uint8_t> serialize(const T&);
        static T deserialize(const void* data, std::uint64_t size);
    };

    template <>
    struct codec<std::string>
    {
        static std::vector<std::uint8_t> serialize(const std::string& message)
        {
            return std::vector<std::uint8_t>(message.begin(), message.end());
        }

        static std::string deserialize(const void* data, std::uint64_t size)
        {
            return std::string(static_cast<const char*>(data), size);
        }
    };

    class TypedService final
    {
    public:
        template <typename Options>
        TypedService(Options options)
            : service(std::make_unique<typename Options::Service>(std::move(options)))
        {
            service->on_message(
                [this](const std::string& id, const void* data, std::uint64_t size)
                {
                    const auto m = static_cast<const std::uint8_t*>(data);
                    const auto it = handlers.find(TypedService::type(data));
                    if (it != handlers.end() && it->second)
                    {
                        it->second(id, m + sizeof(std::uint32_t), size - sizeof(std::uint32_t));
                    }
                }
            );
        }

        ~TypedService() noexcept = default;
        TypedService(TypedService&&) = delete;
        TypedService(const TypedService&) = delete;
        TypedService& operator=(TypedService&&) = delete;
        TypedService& operator=(const TypedService&) = delete;

        void run()
        {
            service->run();
        }

        void stop()
        {
            service->stop();
        }

        void restart()
        {
            service->restart();
        }

        template <typename T>
        void on_message(std::uint32_t type, T handler)
        {
            handlers.emplace(
                type,
                [h = std::move(handler)]                                        //
                (const std::string& id, const void* data, std::uint64_t size)   //
                {                                                               //
                    h(id, codec<detail::traits_t<T>>::deserialize(data, size)); //
                }
            );
        }

        void on_connect(ConnectHandler handler)
        {
            service->on_connect(std::move(handler));
        }

        void on_disconnect(DisconnectHandler handler)
        {
            service->on_disconnect(std::move(handler));
        }

        template <typename T>
        void send(const std::string& id, std::uint32_t type, const T& message)
        {
            auto data = TypedService::serialize(type, codec<T>::serialize(message));
            service->send(id, data.data(), data.size());
        }

        template <typename T>
        void publish(std::uint32_t type, const T& message)
        {
            auto data = TypedService::serialize(type, codec<T>::serialize(message));
            service->publish(data.data(), data.size());
        }

    private:
        std::unique_ptr<nil::service::IService> service;
        std::unordered_map<std::uint32_t, MessageHandler> handlers;

        static std::uint32_t type(const void* data);
        static std::vector<std::uint8_t> serialize(
            std::uint32_t type,
            std::vector<std::uint8_t> payload
        );
    };
}
