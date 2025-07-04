#pragma once

#include "clapi/etc/basic.hh"

#include <functional>

namespace clapi::_detail::inline etc
{

template <typename Ty_, unsigned FitsNWords_>
consteval auto _param_trail() noexcept
{
  using std::add_rvalue_reference, std::is_trivially_copyable_v;

  constexpr std::size_t byte_threshold = FitsNWords_ * sizeof(void*);
  constexpr bool small_enough = sizeof(Ty_) <= byte_threshold;

#if 0
// XXX: Maybe revisit and enable once semantics of those
// is more useful that it is now.
//
// Better to not mess with it right now.
// Before something useful comes up
#if (__has_builtin(__builtin_is_cpp_trivially_relocatable) \
     && __has_builtin(__builtin_is_replaceable))
  if constexpr (__builtin_is_cpp_trivially_relocatable(Ty_)
                && __builtin_is_replaceable(Ty_)
                && small_enough)
    return type_identity<Ty_>{};
#endif
#endif

  if constexpr (is_trivially_copyable_v<Ty_> && small_enough)
    return itstype<Ty_>;
  else
    return add_rvalue_reference<Ty_>{};
}

} // namespace clapi::_detail::inline etc

namespace clapi::inline etc
{

template <typename Ty_, unsigned WordsSzThresh = 1>
using param_opt_t =
  typename std::invoke_result_t<
    decltype(_detail::etc::_param_trail<Ty_, WordsSzThresh>)>::type;

template <typename Ty_, unsigned WordsSzThresh = 1>
[[nodiscard]]
constexpr auto fwd_opt(std::remove_reference_t<Ty_> &t) noexcept ->
  param_opt_t<Ty_, WordsSzThresh>
{
  return static_cast<param_opt_t<Ty_, WordsSzThresh>>(t);
}

template <typename Ty_, unsigned WordsSzThresh = 1>
[[nodiscard]]
constexpr auto fwd_opt(std::remove_reference_t<Ty_> &&t) noexcept ->
  param_opt_t<Ty_, WordsSzThresh>
{
  return static_cast<param_opt_t<Ty_, WordsSzThresh>>(t);
}

} // namespace clapi::inline etc

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
