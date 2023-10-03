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

    template <typename T>
    using Subscriber = std::function<void(const T&)>;
    using Unsubscriber = std::function<void()>;

    /**
     * @brief A reactive object
     *
     * @tparam T                data type
     * @tparam ThreadSafety     type to use to guarantee thread safety
     */
    template <typename T, typename ThreadSafety = NoMutex>
    class Data final: public std::enable_shared_from_this<Data<T>>
    {
    private:
        using Subscribers = std::list<Subscriber<T>>;

        Data(T value)
            : value(std::move(value))
        {
        }

    public:
        using ptr_t = std::shared_ptr<Data<T>>;

        /**
         * @brief Creates a new Data<T>
         *
         * @param value
         * @return std::shared_ptr<Data<T>>
         */
        static ptr_t create(T value)
        {
            return ptr_t(new Data<T>(std::move(value)));
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
        Unsubscriber subscribe(Subscriber<T> call)
        {
            call(value);

            struct Unsub
            {
                Unsub(std::weak_ptr<Data<T>> parent, Subscribers::iterator it)
                    : parent(std::move(parent))
                    , it(std::move(it))
                {
                }

                void exec()
                {
                    if (auto ptr = parent.lock(); ptr)
                    {
                        auto lock = std::unique_lock(ptr->mutex);
                        ptr->subscribers.erase(it);
                        parent = {};
                    }
                }

                std::weak_ptr<Data<T>> parent;
                Subscribers::iterator it;
            };

            // This is done using shared_ptr to allow copying of std::function<void()>
            // to prevent issues of double unsub when it is copied.
            auto unsub = std::make_shared<Unsub>(
                this->weak_from_this(),
                subscribers.insert(subscribers.end(), std::move(call))
            );
            return [unsub]() mutable
            {
                unsub->exec();
                unsub = {};
            };
        }

    private:
        T value;
        Subscribers subscribers;
        ThreadSafety mutex;
    };
}
