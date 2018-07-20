//把元组展开为不定参数
#pragma once
#include <tuple>

template <size_t N>
struct Apply
{
    template <typename TFunc, typename TTuple, typename... TArg>
    static inline auto apply(TFunc &&f, TTuple &&t, TArg &&... a)
    {
        return Apply<N - 1>::apply(std::forward<TFunc>(f), std::forward<TTuple>(t), std::get<N - 1>(std::forward<TTuple>(t)), std::forward<TArg>(a)...);
    }
};

template <>
struct Apply<0>
{
    template <typename TFunc, typename TTuple, typename... TArg>
    static inline auto apply(TFunc &&f, TTuple &&, TArg &&... a)
    {
        return std::forward<TFunc>(f)(std::forward<TArg>(a)...);
    }
};

template <typename TFunc, typename TTuple>
inline auto apply(TFunc &&f, TTuple &&t)
{
    return Apply<std::tuple_size<typename std::decay<TTuple>::type>::value>::apply(std::forward<TFunc>(f), std::forward<TTuple>(t));
}