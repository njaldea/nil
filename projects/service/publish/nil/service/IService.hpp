#pragma once

#include <nil/service/types.hpp>

namespace nil::service
{
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
         * @brief stop the service.
         */
        virtual void stop() = 0;

        /**
         * @brief Prepare the service.
         *  Should be called once before running.
         *  Call before calling other methods.
         */
        virtual void restart() = 0;

        /**
         * @brief Add a message handler for a specific message type
         *
         * @param type
         * @param handler
         */
        virtual void on(
            std::uint32_t type,
            MsgHandler handler //
        ) = 0;

        /**
         * @brief Add an event handler for service events.
         *
         * @param event
         * @param handler
         */
        virtual void on(
            Event event,
            EventHandler handler //
        ) = 0;

        /**
         * @brief Send a message to a specific id.
         *  Does nothing if id is unknown.
         *
         * @param id    - identifier
         * @param type  - message type
         * @param data  - payload
         * @param size  - payload size
         */
        virtual void send(
            const std::string& id,
            std::uint32_t type,
            const void* data,
            std::uint64_t size //
        ) = 0;

        /**
         * @brief Broadcast a message to all listeners
         *
         * @param type  - message type
         * @param data  - payload
         * @param size  - payload size
         */
        virtual void publish(
            std::uint32_t type,
            const void* data,
            std::uint64_t size //
        ) = 0;
    };
}
