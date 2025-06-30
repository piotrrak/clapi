#pragma once

namespace clapi::inline etc
{

template <auto V_> struct nontype_t;

template <typename Ty_> struct itstype_t;

template <int = 0> struct empty;

} // namespace clapi::inline etc

namespace clapi::detail::inline etc
{
//
//
template <bool Cond_, typename Ty_, typename = void>
constexpr inline auto _cond = itstype_t<Ty_>{};

template <bool Cond_, typename If_, typename Else_>
  requires (!Cond_)
constexpr inline auto _cond<Cond_, If_, Else_> = itstype_t<Else_>{};

template <bool Cond_, typename If_, typename Else_>
using _cond_t = typename decltype(_cond<Cond_, If_, Else_>)::type;

} // namespace clapi::detail::inline etc

namespace clapi::inline etc
{

template <int N = 0>
using empty_t = empty<N>;

template <bool Cond_, typename Ty_, int N_ = 0>
using else_empty_t = detail::_cond_t<Cond_, Ty_, empty_t<N_>>;

} // namespace clapi::inline etc

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
