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
            , node(std::forward<Args>(args)...)
        {
        }

        auto operator()(const Inputs&... args) const
        {
            struct OnDestroy
            {
                OnDestroy(detail::ICallable& init_post)
                    : post(init_post)
                {
                }

                ~OnDestroy()
                {
                    post.call();
                }

                detail::ICallable& post;
            };

            pre->call();
            OnDestroy _(*post);

            return node.operator()(args...);
        }

        std::unique_ptr<detail::ICallable> pre;
        std::unique_ptr<detail::ICallable> post;
        T node;
    };
}
