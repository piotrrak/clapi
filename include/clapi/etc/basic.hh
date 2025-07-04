#pragma once

#include "fwd/clapi/etc/basic.hh"

#include "clapi/etc/type_constants.hh"

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

} // namespace clapi::inline etc


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
