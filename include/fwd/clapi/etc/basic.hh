#pragma once

// Foundational definitions {{{
//
// While technically those are definitions rather then forward declarations,
// those were placed here in oreder to help constrain our forward declarations
// with the concepts.
//
// Thus, those should be considered as foundational for other foward-declarations.
// One ought to try to minimize such cases.
// }}}

  // TODO: reconsider/check if providing forward includes is of any real value.
  // The modules based compilation, might be some lead.
#include <concepts> // won't get away without that, since we really want constain template
                    // declarations.
#include <type_traits>

namespace clapi::detail
{

// E. Niebler's? std::declval<T>() implementation by overload resolution
template <typename Ty_>
constexpr auto _declval(int) -> Ty_ &&; // return is only well-formed iff: `_is_referencable_v<Ty_>`
                              // Note: normal reference collapsing rules apply here
                              //       implementing `std::add_rvalue_reference_t<Ty_>` for
                              //       the return type

template <typename Ty_>
constexpr auto _declval(long) -> Ty_;   // fallback case when `not _is_referencable_v<_T>` holds
                              // this *worse* overload is selected as second
                              // due to widening cast from ^int to ^long

// Unprotected for use in non-unevaluated context declval<T>() implementation.
template <typename T_>
constexpr decltype(_declval<T_>(0)) _unprotected_declval() noexcept
{
  return _declval<T_>(0);
}

} // namespace clapi::detail

// Prepare for clapi::detail rename to clapi::_detail
namespace clapi
{
namespace _detail = detail;
}

namespace clapi::inline etc
{

consteval auto as_constexpr(auto cst)
  noexcept(noexcept(auto(cst))) -> decltype(auto(cst))
  requires requires { {auto(cst)}; }
{ return cst; }

template <auto V_> struct nontype_t;

template <typename Ty_> struct itstype_t;

template <int = 0> struct empty;

template <typename Type_>
using unc = std::remove_const_t<Type_>;

template <typename Type_>
using unv = std::remove_volatile_t<Type_>;

template <typename Type_>
using uncv = std::remove_cv_t<Type_>;

template <typename Type_>
using unptr = std::remove_pointer_t<Type_>;

template <typename Type_>
using unref = std::remove_reference_t<Type_>;

template <typename Type_>
using uncvref = std::remove_reference_t<Type_>;

template <typename... Types_>
constexpr inline bool always_false_v = false;

template <typename Ty_>
constexpr inline Ty_* nullptr_v = nullptr;

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
using else_empty_t = _detail::_cond_t<Cond_, Ty_, empty_t<N_>>;

} // namespace clapi::inline etc

namespace clapi::inline etc::inline concepts
{

using std::integral, std::signed_integral, std::unsigned_integral;

using std::same_as, std::convertible_to;

using std::copyable, std::movable;
using std::default_initializable, std::destructible;

template <typename Ty1_, typename Ty2_>
concept different_from = not same_as<uncvref<Ty1_>, uncvref<Ty2_>>;

template <typename Ty_>
concept unqualified = same_as<uncvref<Ty_>, Ty_>;

template <typename Ty_>
concept qualified = not unqualified<Ty_>;

template <typename Ty_>
concept mutable_type = same_as<unc<Ty_>, Ty_>
  and not std::is_function_v<Ty_>;

} // namespace clapi::inline etc::inline concepts

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
