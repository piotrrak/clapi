#pragma once

#include "clapi/etc/basic.hh"
#include "clapi/deduced/function_pointer.hh"

namespace clapi::deduced::inline concepts
{

//----------------------------------------------------------------------------------------
// can_deduce_with - SFINAE based - deduced::metafns helper for function literals
//----------------------------------------------------------------------------------------

template <template <auto> class Deduce_t_, auto Fn_>
concept can_deduce_with = function_pointer<Fn_> and requires {
  typename Deduce_t_<Fn_>;
};

} // namespace clapi::deduced::inline concepts

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
