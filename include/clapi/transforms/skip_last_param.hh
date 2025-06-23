#pragma once

#include "clapi/etc/seq.hh"

namespace clapi::detail::transforms
{
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

template <type_sequence> struct _skip_last_param;

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

template <typename AtLeastOne_,
          typename... Types_>
struct _skip_last_param<tseq<AtLeastOne_, Types_...>> :
  tseq_gather<iseq_for<Types_...>, AtLeastOne_, Types_...>
{
};
} // namespace clapi::detail::transforms

namespace clapi::transforms
{

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
template <type_sequence TSeq_>
using skip_last_param = typename detail::transforms::_skip_last_param<TSeq_>::type;

} // namespace clapi::transforms

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
