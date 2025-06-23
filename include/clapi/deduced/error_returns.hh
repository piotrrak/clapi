#pragma once

#include "clapi/etc/deduced_sfinae.hh"
#include "clapi/deduced/api_signature.hh"

#include <CL/cl_platform.h>

namespace clapi::deduced::inline opencl
{

template <typename Ty_>
constexpr inline bool is_reterr_clapi_v = false;

template <typename Ty_>
constexpr inline bool is_outerr_clapi_v = false;

template <auto Fn_>
  requires can_deduce_with<deduced::result_of, Fn_>
constexpr inline
bool is_reterr_clapi_v<nontype_t<Fn_>> = [] consteval {
  using ret = deduced::result_of<Fn_>;

  return same_as<ret, ::cl_int>;
}();

template <auto Fn_>
  requires can_deduce_with<deduced::result_of, Fn_>
           and can_deduce_with<deduced::last_param_of, Fn_>
constexpr inline
bool is_outerr_clapi_v<nontype_t<Fn_>> = [] consteval {

  using ret = deduced::result_of<Fn_>;
  using outret = deduced::last_param_of<Fn_>;

  if constexpr (same_as<::cl_int, ret>)
    return false;

  return same_as<outret, ::cl_int*>;
}();

template <typename Ty_>
concept api_call = is_reterr_clapi_v<Ty_> or is_outerr_clapi_v<Ty_>;

template <typename Ty_>
concept core_api = api_call<Ty_>;

} // clapi::deduced

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
