#pragma once

#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <set>

namespace nil::pulse
{
    /**
     * @brief Stub and default handler for threadsafety.
     *  Masks implementation compatible with std::unique_lock.
     */
    struct NoMutex
    {
        void lock()
        {
        }

        void unlock()
        {
        }
    };

    /**
     * @brief A reactive object
     *
     * @tparam T                data type
     * @tparam ThreadSafety     type to use to guarantee thread safety
     */
    template <typename T, typename ThreadSafety = NoMutex>
    class Data: public std::enable_shared_from_this<Data<T>>
    {
    private:
        using Subscriber = std::function<void(const T&)>;
        using Subscribers = std::list<Subscriber>;

        Data(T value)
            : value(std::move(value))
        {
        }

    public:
        using ptr = std::shared_ptr<Data<T>>;

        /**
         * @brief Creates a new Data<T>
         *
         * @param value
         * @return std::shared_ptr<Data<T>>
         */
        static std::shared_ptr<Data<T>> create(T value)
        {
            return std::shared_ptr<Data<T>>(new Data<T>(std::move(value)));
        }

        ~Data() = default;

        Data(Data&&) = default;
        Data(const Data&) = default;
        Data& operator=(Data&&) = default;
        Data& operator=(const Data&) = default;

        /**
         * @brief Returns the current value held by the object.
         *
         * @return const T&
         */
        const T& get() const
        {
            return value;
        }

        /**
         * @brief Updates the current value held by the object.
         *  Calls all the subscribers before the function end.
         *
         * @param value
         */
        void set(T value)
        {
            if (this->value != value)
            {
                this->value = std::move(value);
                auto lock = std::unique_lock(mutex);
                for (const auto& sub : subscribers)
                {
                    sub(this->value);
                }
            }
        }

        /**
         * @brief Registers a callback to be invoked when the data chanes
         *
         * @param call                      callback to be called
         * @return std::function<void()>    unsubscribes the callback
         */
        std::function<void()> subscribe(Subscriber call)
        {
            call(value);
            auto parent = this->weak_from_this();
            auto it = subscribers.insert(subscribers.end(), std::move(call));
            return [parent = std::move(parent), it = std::move(it)]() mutable
            {
                if (auto ptr = parent.lock(); ptr)
                {
                    auto lock = std::unique_lock(ptr->mutex);
                    ptr->subscribers.erase(it);
                    parent = {};
                }
            };
        }

    private:
        T value;
        Subscribers subscribers;
        ThreadSafety mutex;
    };
}
