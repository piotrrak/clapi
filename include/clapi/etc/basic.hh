#pragma once

#include "fwd/clapi/etc/basic.hh"

#include "clapi/etc/type_constants.hh"

#include <concepts>
#include <type_traits>

namespace clapi::inline etc
{

// nontype_t and itstype_t - are duals identity of type and value in the type world.
//
// nontype and itstype are corresponding duals of above in the value world.
//
// nottype_t - represents not-type as type at same time being value identity.
// It is the gateway for non-types (literal values) to the type world.
//
// itstype_t - represents type of type
// It's the gateway for types to the value world.
//
// Both of them should be considered two flavours identities.
// The nottype_t is value identity, while itstype_t is type identity.

template <auto Value_>
struct nontype_t
{
  static constexpr inline auto value = Value_;

  explicit nontype_t() = default;
};

template <typename Type_>
struct itstype_t
{
  using type = Type_;

  explicit itstype_t() = default;
};

template <auto Value_>
constexpr nontype_t<Value_> nontype{};

template <typename Ty_>
constexpr itstype_t<Ty_> itstype{};

template <int>
struct empty {};

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

template <typename Ty_>
constexpr Ty_* nullptr_v = nullptr;

} // namespace clapi::inline etc

namespace clapi::detail::inline etc::_fptr_traits
{

// Below is std::is_function_v conforming implementation.
//
// It's here to make it bit more obvious what's going on, with regards type-categories
// queries that are performed.
//
// This fallback implementation strategy deserves the brief explaination.
//
// According to [dcl.fct]p10 and directly in note to example 5 to
//
// [Note 5: A function type that has a cv-qualifier-seq is not a cv-qualified type;
//          there are no cv-qualified function types.  — end note]
//
// Please do note: checking `!is_const_v<const T>` or `!is_volatile_v<volatile T>`
// would have suffice, but one can also do both all together with single template alias.
//
// (Please see _isnt_cv_qualified below).
//
// The pointer to function is in pointer type primary category, while pointers to member
// are in their own separate member-function-pointer and member-object-pointer
// primary type categories.
//
// Those three primary type categories can be cv-qualified,
// thus are also excluded by above constraint.
//
// In addition the lvalue/rvalue references to function type belong correspondingly to
// the primary type categories:
//
// - lvalue-reference, (is_lvalue_reference_v<> == true) or
// - rvalue-reference, (is_rvalue_reference_v<> == true)
//
// That two belong to the compound type category of references (is_reference_v<> == true).
//
// One ought exclude those:

template <typename _Type>
consteval bool _is_reference(int = 0) noexcept
{
  return std::is_lvalue_reference_v<_Type>
    or std::is_rvalue_reference_v<_Type>;
}

// NB: Unless used exactly as in _is_function() _isnt_cv_qualified is wrong!
// Beware that the only correct usage of _isnt_cv_qualified is:
//
// (_isnt_cv_qualified<const volatile Ty_> == true)
//
// and only in context of Ty_ belonging to function-type type category!
template <typename>
constexpr inline bool _isnt_cv_qualified = true;

template <typename _Type>
constexpr inline bool _isnt_cv_qualified<const volatile _Type> = false;

template <typename _Type>
[[nodiscard]] consteval bool _is_function() noexcept
{
  return _isnt_cv_qualified<const volatile _Type>
    and not _is_reference<_Type>(0);
}

template <typename Ty_>
constexpr inline auto _is_function_ptr_type =
  boolean_<std::is_pointer_v<Ty_> and _is_function<unptr<Ty_>>()>;

template <auto Value_>
constexpr inline auto _is_function_ptr =
  _is_function_ptr_type<decltype(Value_)>;

template <auto Value_>
constexpr inline auto _is_function_ptr_type<nontype_t<Value_>> =
  _is_function_ptr<Value_>;

} // namespace clapi::detail::inline etc

namespace clapi::inline etc::inline concepts
{

using std::integral, std::signed_integral, std::unsigned_integral;

using std::same_as, std::convertible_to;

template <typename Ty1_, typename Ty2_>
concept different_from = not same_as<uncvref<Ty1_>, uncvref<Ty1_>>;

// function_pointer_type
//
// true iff type Ty_ is function pointer or nontype_t of thereof.
template <typename Ty_>
concept function_pointer_type =
  detail::_fptr_traits::_is_function_ptr_type<Ty_>();

// plain_function_pointer_type
//
// true iff type Ty_ is function pointer but not nontype_t of thereof.
template <typename Ty_>
concept plain_function_pointer_type = std::is_pointer_v<Ty_>
  and function_pointer_type<Ty_>;

// nontype_function_pointer_type
//
// true iff type Ty_ is nontype_t of function pointer,
//      but not function pointer type itself.
template <typename Ty_>
concept nontype_function_pointer_type = function_pointer_type<Ty_>
  and not plain_function_pointer_type<Ty_>;

template <auto Value_>
concept function_pointer = detail::_fptr_traits::_is_function_ptr<Value_>();

template <auto Value_>
concept plain_function_pointer = std::is_pointer_v<decltype(Value_)>
  and function_pointer<Value_>;

template <auto Value_>
concept nontype_function_pointer = function_pointer<Value_>
  and not plain_function_pointer<Value_>;

template <typename Ty_>
concept qualified = different_from<uncvref<Ty_>, Ty_>;

template <typename Ty_>
concept unqualified = same_as<uncvref<Ty_>, Ty_>;

template <typename Ty_>
concept mutable_type = same_as<unc<Ty_>, Ty_>
  and not std::is_function_v<Ty_>;

} // namespace clapi::inline etc::inline concepts

namespace clapi::inline etc
{

template <unqualified Derived_>
struct immovable
{
public:
  immovable() = default;

  immovable(const immovable &) = delete;
  immovable(immovable &&) = delete;

  template <mutable_type D_>
    requires std::is_pointer_interconvertible_base_of_v<immovable, D_>
  auto operator=(this D_ &&, uncv<D_> &&) -> immovable & = delete;

  ~immovable() = default;
};

} // namespace clapi::inline etc

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
