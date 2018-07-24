//把元组展开为不定参数
#pragma once
#include <tuple>

template <class _Callable, class _Tuple, size_t... _Indices>
constexpr decltype(auto) _Apply_impl(_Callable &&_Obj, _Tuple &&_Tpl, std::index_sequence<_Indices...>)
{
    return std::forward<_Callable>(_Obj)(std::get<_Indices>(std::forward<_Tuple>(_Tpl))...);
}

template <class _Callable, class _Tuple>
constexpr decltype(auto) apply(_Callable &&_Obj, _Tuple &&_Tpl)
{
    return _Apply_impl(std::forward<_Callable>(_Obj), std::forward<_Tuple>(_Tpl), std::make_index_sequence<std::tuple_size<std::decay_t<_Tuple>>::value>());
}
