#pragma once

#include "codec.hpp"

#include <array>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace nil::service
{
    namespace detail
    {
        template <typename... Args>
        struct ICallable
        {
            ICallable() = default;
            virtual ~ICallable() noexcept = default;

            ICallable(ICallable&&) noexcept = delete;
            ICallable& operator=(ICallable&&) noexcept = delete;

            ICallable(const ICallable&) = delete;
            ICallable& operator=(const ICallable&) = delete;

            virtual void call(const std::string&, Args... args) = 0;
        };

        template <typename Options>
        struct Storage final
        {
            Options options;
            std::unique_ptr<ICallable<const void*, std::uint64_t>> msg;
            std::unique_ptr<ICallable<>> connect;
            std::unique_ptr<ICallable<>> disconnect;
        };

        template <typename Handler, typename... Args>
        std::unique_ptr<ICallable<Args...>> make_callable(Handler&& handler)
        {
            struct Callable final: detail::ICallable<Args...>
            {
                explicit Callable(Handler init_handler)
                    : handler(std::move(init_handler))
                {
                }

                ~Callable() noexcept override = default;

                Callable(Callable&&) noexcept = delete;
                Callable(const Callable&) = delete;
                Callable& operator=(Callable&&) noexcept = delete;
                Callable& operator=(const Callable&) = delete;

                void call(const std::string& id, Args... args) override
                {
                    if constexpr (std::is_invocable_v<Handler, const std::string&, Args...>)
                    {
                        handler(id, args...);
                    }
                    else
                    {
                        handler(args...);
                    }
                }

                Handler handler;
            };

            return std::make_unique<Callable>(std::forward<Handler>(handler));
        }
    }

    using MessageHandler //
        = std::unique_ptr<detail::ICallable<const void*, std::uint64_t>>;
    using ConnectHandler //
        = std::unique_ptr<detail::ICallable<>>;
    using DisconnectHandler //
        = std::unique_ptr<detail::ICallable<>>;

    class IService
    {
    public:
        virtual ~IService() noexcept = default;

        IService() = default;
        IService(IService&&) noexcept = delete;
        IService(const IService&) = delete;
        IService& operator=(IService&&) noexcept = delete;
        IService& operator=(const IService&) = delete;

        /**
         * @brief run the service. blocking.
         */
        virtual void run() = 0;

        /**
         * @brief stop the service. non-blocking.
         */
        virtual void stop() = 0;

        /**
         * @brief Prepare the service.
         *  Should be called once after stopping and before running.
         *  Call before calling other methods.
         */
        virtual void restart() = 0;

        /**
         * @brief Broadcast a message to all listeners
         *
         * @param type  message type
         * @param data  payload
         * @param size  payload size
         */
        virtual void publish(std::vector<std::uint8_t> message) = 0;

        /**
         * @brief Send a message to a specific id.
         *  Does nothing if id is unknown.
         *
         * @param id    identifier
         * @param data  data
         */
        virtual void send(const std::string& id, std::vector<std::uint8_t> message) = 0;

        template <typename... T>
        void publish(const T&... data)
        {
            this->publish(serialize(data...));
        }

        /**
         * @brief Broadcast a message to all listeners
         *
         * @param type  message type
         * @param data  payload
         * @param size  payload size
         */
        void publish_raw(const void* data, std::uint64_t size)
        {
            const auto* ptr = static_cast<const std::uint8_t*>(data);
            publish(std::vector<std::uint8_t>(ptr, ptr + size));
        }

        template <typename... T>
        void send(const std::string& id, T&&... data)
        {
            this->send(id, serialize(data...));
        }

        /**
         * @brief Send a message to a specific id.
         *  Does nothing if id is unknown.
         *
         * @param id    identifier
         * @param data  payload
         * @param size  payload size
         */
        void send_raw(const std::string& id, const void* data, std::uint64_t size)
        {
            const auto* ptr = static_cast<const std::uint8_t*>(data);
            send(id, std::vector<std::uint8_t>(ptr, ptr + size));
        }

        /**
         * @brief Add a message handler
         *
         * @param handler
         */
        template <typename Handler>
        void on_message(Handler handler)
        {
            on_message_impl(
                detail::make_callable<Handler, const void*, std::uint64_t>(std::move(handler))
            );
        }

        /**
         * @brief Add a connect handler for service events.
         *
         * @param handler
         */
        template <typename Handler>
        void on_connect(Handler handler)
        {
            on_connect_impl(detail::make_callable<Handler>(std::move(handler)));
        }

        /**
         * @brief Add a disconnect handler for service events.
         *
         * @param handler
         */
        template <typename Handler>
        void on_disconnect(Handler handler)
        {
            on_disconnect_impl(detail::make_callable<Handler>(std::move(handler)));
        }

    private:
        // [TODO] refactor and make this better;
        template <typename... T>
        std::vector<std::uint8_t> serialize(const T&... data)
        {
            const std::array<std::vector<std::uint8_t>, sizeof...(T)> buffers
                = {codec<T>::serialize(data)...};

            std::vector<std::uint8_t> message;
            message.reserve(
                [&]()
                {
                    auto c = 0ul;
                    for (const auto& b : buffers)
                    {
                        c += b.size();
                    }
                    return c;
                }()
            );
            for (const auto& buffer : buffers)
            {
                for (const auto& item : buffer)
                {
                    message.push_back(item);
                }
            }
            return message;
        }

        /**
         * @brief Add a message handler
         *
         * @param handler
         */
        virtual void on_message_impl(MessageHandler handler) = 0;

        /**
         * @brief Add a connect handler for service events.
         *
         * @param handler
         */
        virtual void on_connect_impl(ConnectHandler handler) = 0;

        /**
         * @brief Add a disconnect handler for service events.
         *
         * @param handler
         */
        virtual void on_disconnect_impl(DisconnectHandler handler) = 0;
    };

    template <typename T>
    std::unique_ptr<IService> make_service(typename T::Options options)
    {
        return std::make_unique<T>(std::move(options));
    }
}
