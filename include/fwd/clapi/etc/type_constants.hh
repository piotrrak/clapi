#pragma once

#include <cstddef>
#include <utility>

namespace clapi::inline type_constants
{

using std::integral_constant;

template <auto V_>
using constant = integral_constant<decltype(V_), V_>;

template <bool V_>
using boolean = constant<V_>;

template <std::size_t V_>
using size = constant<V_>;

template <auto V_>
inline constexpr auto constant_ = constant<V_>{};

template <bool V_>
inline constexpr auto boolean_ = boolean<V_>{};

template <std::size_t V_>
constexpr inline auto size_ = size<V_>{};

using ney = boolean<false>; // Note: it is an alias of std::false_type
using aye = boolean<true>;  // Note: it is an alias of std::true_type

} // namespace clapi

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
