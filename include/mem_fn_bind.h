//将一个成员函数与一个成员对象绑定，使其看起来像一个全局函数。
//需要C++14以上。
#pragma once
#include <functional>

namespace std
{
#if __cplusplus >= 201402L
template <typename _Tp, typename _Class, typename... _Args, size_t... _Idx>
constexpr decltype(auto) __mem_fn_bind_impl(_Class (_Tp::*__f)(_Args...), _Tp* __t, index_sequence<_Idx...>)
{
    return bind(mem_fn(__f), __t, _Placeholder<_Idx + 1>()...);
}

template <typename _Tp, typename _Class, typename... _Args>
constexpr decltype(auto) mem_fn_bind(_Class (_Tp::*__f)(_Args...), _Tp* __t)
{
    return __mem_fn_bind_impl(__f, __t, make_index_sequence<sizeof...(_Args)>());
}
#endif // C++14
} // namespace std
