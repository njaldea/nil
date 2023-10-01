#pragma once

#include <functional>
#include <list>
#include <memory>
#include <set>

namespace nil::pulse
{
    enum class Mode
    {
        // automatically
        // unsubscribe
        // upon destruction
        Strong,
        // will not unsubscribe
        // when destroyed
        Weak
    };

    template <typename T>
    class Data: public std::enable_shared_from_this<Data<T>>
    {
    private:
        using Subscriber = std::function<void(const T&)>;
        using Subscribers = std::list<Subscriber>;

        Data(T data)
            : data(std::move(data))
        {
        }

    public:
        using ptr = std::shared_ptr<Data<T>>;

        static std::shared_ptr<Data<T>> create(T data)
        {
            return std::shared_ptr<Data<T>>(new Data<T>(std::move(data)));
        }

        ~Data() = default;

        Data(Data&&) = default;
        Data(const Data&) = default;
        Data& operator=(Data&&) = default;
        Data& operator=(const Data&) = default;

        const T& get() const
        {
            return data;
        }

        void set(T value)
        {
            if (data != value)
            {
                data = std::move(value);
                for (const auto& sub : subscribers)
                {
                    sub(data);
                }
            }
        }

        std::function<void()> subscribe(Subscriber call, Mode mode = Mode::Strong)
        {
            call(data);

            struct Unsubsciber
            {
                Unsubsciber(std::weak_ptr<Data<T>> parent, Subscribers::iterator it, Mode mode)
                    : parent(std::move(parent))
                    , it(std::move(it))
                    , mode(mode)
                {
                }

                Unsubsciber(Unsubsciber&&) = default;
                Unsubsciber(const Unsubsciber&) = default;
                Unsubsciber& operator=(Unsubsciber&&) = default;
                Unsubsciber& operator=(const Unsubsciber&) = default;

                ~Unsubsciber()
                {
                    if (mode == Mode::Strong)
                    {
                        (*this)();
                    }
                }

                void operator()()
                {
                    auto ptr = parent.lock();
                    if (ptr)
                    {
                        ptr->subscribers.erase(it);
                        parent = {};
                    }
                }

                std::weak_ptr<Data<T>> parent;
                Subscribers::iterator it;
                Mode mode;
            };

            return Unsubsciber(
                this->weak_from_this(),
                subscribers.insert(subscribers.end(), std::move(call)),
                mode
            );
        }

    private:
        T data;
        Subscribers subscribers;
    };
}
