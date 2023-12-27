#pragma once

#include <functional>
#include <memory>
#include <string>

namespace nil::service
{
    using MessageHandler = std::function<void(const std::string&, const void*, std::uint64_t)>;
    using ConnectHandler = std::function<void(const std::string&)>;
    using DisconnectHandler = std::function<void(const std::string&)>;

    namespace detail
    {
        template <typename Options>
        struct Storage
        {
            Options options;
            MessageHandler msg = {};
            ConnectHandler connect = {};
            DisconnectHandler disconnect = {};
        };
    }

    class IService
    {
    public:
        virtual ~IService() noexcept = default;

        IService() = default;
        IService(IService&&) = delete;
        IService(const IService&) = delete;
        IService& operator=(IService&&) = delete;
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
         * @brief Add a message handler
         *
         * @param handler
         */
        virtual void on_message(MessageHandler handler) = 0;

        /**
         * @brief Add a connect handler for service events.
         *
         * @param handler
         */
        virtual void on_connect(ConnectHandler handler) = 0;

        /**
         * @brief Add a disconnect handler for service events.
         *
         * @param handler
         */
        virtual void on_disconnect(DisconnectHandler handler) = 0;

        /**
         * @brief Send a message to a specific id.
         *  Does nothing if id is unknown.
         *
         * @param id    identifier
         * @param data  payload
         * @param size  payload size
         */
        virtual void send(const std::string& id, const void* data, std::uint64_t size) = 0;

        /**
         * @brief Broadcast a message to all listeners
         *
         * @param type  message type
         * @param data  payload
         * @param size  payload size
         */
        virtual void publish(const void* data, std::uint64_t size) = 0;
    };

    template <typename T>
    std::unique_ptr<IService> make_service(typename T::Options options)
    {
        return std::make_unique<T>(std::move(options));
    }
}
