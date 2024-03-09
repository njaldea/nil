#pragma once

#include "../detail/ICallable.hpp"
#include "../detail/traits/callable.hpp"

namespace nil::gate::nodes
{
    template <typename T, typename Inputs = detail::traits::callable<T>::inputs>
    struct Scoped;

    template <typename T, typename... Inputs>
    struct Scoped<T, nil::gate::detail::traits::types<Inputs...>> final
    {
        template <typename Pre, typename Post, typename... Args>
        Scoped(Pre init_pre, Post init_post, Args&&... args)
            : pre(detail::make_callable(std::move(init_pre)))
            , post(detail::make_callable(std::move(init_post)))
            , node{std::forward<Args>(args)...}
        {
        }

        Scoped() = delete;
        ~Scoped() = default;

        Scoped(Scoped&&) = delete;
        Scoped(const Scoped&) = delete;
        Scoped& operator=(Scoped&&) = delete;
        Scoped& operator=(const Scoped&) = delete;

        auto operator()(const Inputs&... args)
        {
            struct OnDestroy final
            {
                explicit OnDestroy(detail::ICallable<void()>* init_post)
                    : post(init_post)
                {
                }

                OnDestroy() = delete;

                ~OnDestroy()
                {
                    post->call();
                }

                OnDestroy(OnDestroy&&) = delete;
                OnDestroy(const OnDestroy&) = delete;
                OnDestroy& operator=(OnDestroy&&) = delete;
                OnDestroy& operator=(const OnDestroy&) = delete;

                detail::ICallable<void()>* post;
            };

            pre->call();
            OnDestroy _(post.get());

            return node.operator()(args...);
        }

        std::unique_ptr<detail::ICallable<void()>> pre;
        std::unique_ptr<detail::ICallable<void()>> post;
        T node;
    };
}
