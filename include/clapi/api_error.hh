#pragma once

#include "clapi/etc/type_constants.hh"

#include <cstdint>

#if __has_include(<CL/cl_platform.h>)
# include <CL/cl_platform.h>
#else
#warning "Missing include <CL/cl_platform.h> assuming type of ::cl_int"
using ::cl_int = std::int32_t;
#endif

#include <compare>
#include <expected>
#include <type_traits>
#include <utility>

namespace clapi::inline errors
{

enum struct clapi_errcode : ::cl_int {};

using error_code_t = clapi_errcode;

constexpr static inline auto CLAPI_API_SUCCESS_VALUE = constant_<::cl_int(0)>;
constexpr static inline auto CLAPI_SUCCESS = constant_<error_code_t(0)>;

template <typename Ty_ = void>
using error_or = std::expected<Ty_, error_code_t>;

inline constexpr auto to_error(::cl_int v) noexcept ->
   decltype(std::unexpected{error_code_t(v)})
{
  // TODO: <contacts>???
  // assert(v != CLAPI_API_SUCCESS_VALUE);
  return std::unexpected{error_code_t(v)};
}

} // namespace clapi::inline errors

/*
 Comparison with clapi_error_code_t with int type is disabled by default.

Given:

In order to enable such comparison one can use
```c++
using namespace clapi::enable_errcode_int_compare;

```
Ie. to allow clapi::error_code_t to meet concept:

```c++
template <typename CLAPI_errcode_t>
concept cl_int_comparable =
  requires (::cl_int cl_err, CLAPI_errcode_t clapi_err) {
  { cl_err == clapi_err } -> std::same_as<bool>;
  { clapi_err == cl_err } -> std::same_as<bool>;
  { cl_err != clapi_err } -> std::same_as<bool>;
  { clapi_err != cl_err } -> std::same_as<bool>;
};

cl_int_comparable<clapi::errors::error_code_t> == true;
```
*/
namespace clapi::enable_errcode_int_compare
{

[[nodiscard]]
inline constexpr auto operator==(error_code_t e1, ::cl_int e2) noexcept -> bool
{
  return std::to_underlying(e1) == e2;
}

[[nodiscard]]
inline constexpr auto operator==(::cl_int e2, error_code_t e1) noexcept -> bool
{
  return std::to_underlying(e1) == e2;
}

static_assert(
requires (::cl_int cl_err, error_code_t clapi_err) {
  { cl_err == clapi_err } -> std::same_as<bool>;
  { clapi_err == cl_err } -> std::same_as<bool>;
  { cl_err != clapi_err } -> std::same_as<bool>;
  { clapi_err != cl_err } -> std::same_as<bool>;
});

} // namespace clapi::enable_errcode_int_compare

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
