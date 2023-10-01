#pragma once

#include <functional>
#include <list>
#include <memory>
#include <set>

template <typename T>
class Data: public std::enable_shared_from_this<Data<T>>
{
    Data(T data)
        : data(std::move(data))
    {
    }

public:
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
            for (const auto& subscription : subscriptions)
            {
                subscription(data);
            }
        }
    }

    std::function<void()> subscribe(std::function<void(const T&)> call, Mode mode = Mode::Strong)
    {
        call(data);
        subscriptions.emplace_back(std::move(call));

        struct Unsubsciber
        {
            Unsubsciber(
                std::weak_ptr<Data<T>> parent,
                std::list<std::function<void(const T&)>>::iterator it,
                Mode mode
            )
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
                    ptr->subscriptions.erase(it);
                    parent = {};
                }
            }

            std::weak_ptr<Data<T>> parent;
            std::list<std::function<void(const T&)>>::iterator it;
            Mode mode;
        };

        return Unsubsciber(this->weak_from_this(), --subscriptions.end(), mode);
    }

private:
    T data;
    std::list<std::function<void(const T&)>> subscriptions;
};
