#pragma once

#include "codec.hpp"

#include <nil/utils/traits/callable.hpp>

#include <array>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

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

    using MessageHandler = std::function<void(const std::string&, const void*, std::uint64_t)>;
    using ConnectHandler = std::function<void(const std::string&)>;
    using DisconnectHandler = std::function<void(const std::string&)>;

    namespace detail
    {
        template <typename Options>
        struct Storage final
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

    private:
        // TODO: refactor and make this better;
        template <typename... T>
        std::vector<std::uint8_t> serialize(const T&... data)
        {
            const std::array<std::vector<std::uint8_t>, sizeof...(T)> buffers
                = {codec<T>::serialize(data)...};
            const auto size = [&]()
            {
                auto c = 0ul;
                for (const auto& b : buffers)
                {
                    c += b.size();
                }
                return c;
            }();

            std::vector<std::uint8_t> message;
            message.reserve(size);
            auto inserter = std::back_inserter(message);
            for (const auto& b : buffers)
            {
                std::copy(std::begin(b), std::end(b), inserter);
            }
            return message;
        }
    };

    template <typename T>
    std::unique_ptr<IService> make_service(typename T::Options options)
    {
        return std::make_unique<T>(std::move(options));
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
        std::unordered_map<
            Indexer,
            std::function<void(const std::string&, const void*, std::uint64_t)>>
            handlers;
    };
}
