//将一个成员函数与一个成员对象绑定，使其看起来像一个全局函数。
#pragma once

template <typename _Tp, typename _Class, typename... _Args>
constexpr decltype(auto) mem_fn_bind(_Class (_Tp::*__f)(_Args...), _Tp* __t)
{
    return [=](_Args... args) { return (__t->*__f)(args...); };
}
