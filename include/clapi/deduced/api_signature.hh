#pragma once

#include "clapi/etc/seq.hh"
#include "clapi/deduced/function_pointer.hh"

namespace clapi::_detail::deduced
{

using clapi::deduced::function_pointer;
using clapi::deduced::function_pointer_type;

using clapi::deduced::plain_function_pointer;
using clapi::deduced::plain_function_pointer_type;

//----------------------------------------------------------------------------------------
// {{{ trait _signature_query<> and auxiliaries - Main Work-Horse function type deduction
//----------------------------------------------------------------------------------------
//
// More detail on usage and inner-workings: {{{
//
// Since we're dealing only with function pointer primary type category,
// the task relatively easy, and one is not concerned about qualifiers like:
// - ref-qualifiers (&&, &)
// - const
// - volatile
// and the combination of thereof.
//
// Thus one can get just get away this only one partial specialiazation.
// Here, this role has `struct _fptr_type_query::_query_function_type`
//
// Second template argument is template-template type of form:
//
//   <typenama ReturnType_,
//    bool IsNoexcept,
//    typename...Parmaters >
// later on to be referred as SigQueryType
//
// This enables one to query specific aspects of function pointer type.
//
// `typename _fptr_type_query::type` becomes type of form
// `std::type_identity` (type_traits - type)
//
// In fasion similiar to standard library type traits like:
// - std::remove_cv,
// - std::add_const
// - std::underlying_type
// - etc...
//
// It was modeled as such traits, with additional point of customization
// namely `SigQueryType` template-template type parameter
//
// of type inherited from an instantion of `SigQueryType`
// with function pointer type:
// - the return type
// - bool true if function is noexcept qualified, false otherwise
// - the pack of all function type parameters
//
// }}}
//
// Here are queries (`SigQueryType<>`) interesting using this facility:

// Queries: {{{
namespace _q
{

// _q::only_params - for function parameters types - `tseq<...>` of such parameters.
//
// Note: Aids implementation of `clapi::deduced::params_of`
template <typename, bool, typename... Params_>
using only_params = tseq<Params_...>;

// _q::param_count - for function parameters types - number of such parameters.
// (Ie. function n-arity)
//
// Note: Aids implementation of `clapi::deduced::param_count_of`
template <typename, bool, typename... Params_>
using param_count = size<sizeof...(Params_)>;

// _q::only_return - for function parameters types - number of such parameters.
//
// Note: Aids implementation of `clapi::deduced::result_of`
template <typename RetTy_, bool, typename...>
using only_return = RetTy_;

// _q::noexcept_qual - clapi::boolean type on wheather function type is
// noexcept(true) qualified.
//
// NB: In this problem domain ie. OpenCL C API always true.
// Reasoning behind: {{{
//   One can simply disregard noexcept qualification of function type
//   since C functions should never throw.
//   At same time such assumption shall be carried throughout rest of this codebase.
// }}}
//
// Note: Aids implementation of `clapi::deduced::noexcept_qual_of`
template <typename, bool /*Disregardless_*/, typename...>
using noexcept_qual = clapi::aye;

} // namespace _q }}}

//----------------------------------------------------------------------------------------
// _fptr_type_query - generalized helper trait for pointer of function type
//----------------------------------------------------------------------------------------
template <plain_function_pointer_type FnPtrTy_,
          template <typename, bool, typename...> class SigQueryTy_>
struct _fptr_type_query
{
private:
  // Helper for wrapping of the base `SigQueryTy_<>` instantiation
  template <typename RetTy_, bool NoExc_, typename... Params_>
  using _apply_query = itstype_t<SigQueryTy_<RetTy_, NoExc_, Params_...>>;

  // Primary template specialized for function type deduction
  template <typename Ty_> requires std::is_function_v<Ty_>
  struct _query_function_type;

  // Specialization for type/noexcept qual deduction
  template <typename RetTy_, bool NoExc_, typename... Params_>
  struct _query_function_type<RetTy_ (Params_...) noexcept(NoExc_)> :
    _apply_query<RetTy_, NoExc_, Params_...> {};

  using _function_type = unptr<FnPtrTy_>;

public:
  using type = _query_function_type<_function_type>;
};

//----------------------------------------------------------------------------------------
// _signature_query - helper alias to apply `SigQueryType<>` on the function literal
//----------------------------------------------------------------------------------------
//
//  Note: Repacks _fptr_type_query::type::type to box `itstype_t<>` decoupling it from
//        the `_fptr_type_query`
//
//  Also: `_fptr_type_query::type::type` is the `Query_<>` instantiation (expected result)
//
//  Examples: {{{
//   * namespace _detail::_q - for queries
//   * _detail::_arity_of, _detail::_params_of, etc. - for application
//  }}}

template <auto Fn_,
          template <typename, bool, typename...> class Query_>
  requires function_pointer<Fn_>
using _signature_query =
  itstype_t<typename _fptr_type_query<decltype(Fn_), Query_>::type::type>;

/// }}}

//----------------------------------------------------------------------------------------
// _arity_of - arity of function literal [`clapi::size`]
//----------------------------------------------------------------------------------------
//
// Number of function literal parameters.
//
// Note: Aids implementation of `clapi::deduced::arity_of`

template <auto Fn_>
  requires function_pointer<Fn_>
constexpr inline _signature_query<Fn_, _q::param_count>::type _arity_of{};

//----------------------------------------------------------------------------------------
// _noexcept_qual_of - function literal qualifier noexcept(true|false) [clapi::boolean]
//----------------------------------------------------------------------------------------
//
// Note: Aids implementation of `clapi::deduced::noexcept_qual_of`

template <auto Fn_>
  requires function_pointer<Fn_>
constexpr inline _signature_query<Fn_, _q::noexcept_qual>::type _noexcept_qual_of{};

//----------------------------------------------------------------------------------------
// _params_of -  function literal parameters [typename = `clapi::tseq<...>`]
//----------------------------------------------------------------------------------------
//
// Note: Aids implementation of `clapi::deduced::_params_of`

template <auto Fn_>
  requires function_pointer<Fn_>
struct _params_of : _signature_query<Fn_, _q::only_params> {};

//----------------------------------------------------------------------------------------
// _result_of - type of function literal return type [typename]
//----------------------------------------------------------------------------------------
//
// Note: Aids implementation of `clapi::deduced::result_of`

template <auto Fn_>
  requires function_pointer<Fn_>
struct _result_of : _signature_query<Fn_, _q::only_return> {};

//----------------------------------------------------------------------------------------
// _last_param_impl - primary (SFINAE-PASS-CASE: iff arity > 0: on typename ::type)
//----------------------------------------------------------------------------------------
//
//  Note: Aids implementation of `clapi::deduced::last_param_of`

template <auto Fn_,
          unsigned FnArity_ = _arity_of<Fn_>>
  requires function_pointer<Fn_>
struct _last_param_impl
{
private:
  using _params = typename _params_of<Fn_>::type;

public:
  using type = _params::template type_at<FnArity_ - 1>;
};

//----------------------------------------------------------------------------------------
// _last_param_impl<*, 0> - specialization (SFINAE-FAIL-CASE: on typename ::type)
//----------------------------------------------------------------------------------------
//
// Note: Aids implementation of `clapi::deduced::last_param_of`

template <auto Fn_>
  requires function_pointer<Fn_>
struct _last_param_impl<Fn_, 0> {};

//----------------------------------------------------------------------------------------
// _last_param_of - (SFINAE-PASS: iff arity > 0 on typename ::type)
//----------------------------------------------------------------------------------------
//
// Note: Aids implementation of `clapi::deduced::last_param_of`

template <auto Fn_>
  requires function_pointer<Fn_>
struct _last_param_of : _last_param_impl<Fn_> {};

} // namespace clapi::_detail::deduced

namespace clapi::deduced
{

//----------------------------------------------------------------------------------------
// params_of - function literal parameters [typename = `tseq<...>`]
//----------------------------------------------------------------------------------------
template <auto Fn_>
  requires function_pointer<Fn_>
           and /* TODO: for now */ plain_function_pointer<Fn_>
using params_of = typename _detail::deduced::_params_of<Fn_>::type;

//----------------------------------------------------------------------------------------
// result_of - type of function literal return type (modeled after std::result_of)
//----------------------------------------------------------------------------------------
//
// NB: It is an implementation of to be removed `std::result_of<>`
// `std::result_of` has been deprecated in favour of `std::invoke_result_t`;
//
// The context and explaination: {{{
//
// We do limit it to function pointers and in context of function types
// primary category, is right tool for a job.
// Since dealing with function literals representing single function type
// that aren't associated with overload set.
//
// Therefore deficiencies of `std::result_of` are not our concern, while much more
// convinient and readable to use.
//
// semantics of `clapi::result_of` could have been represented in terms of the
//  `std::invoke_result<>` trait type as below:
//
/* code {{{
``` c++

  // Helper template - for return type deduction
  template <auto Fn_, typename = clapi::deduced::params_of<Fn_>>
    requires clapi::plain_function_pointer<Fn_>
  struct _result_of_helper;

  template <auto Fn_, typename... Params_>
  struct _result_of_helper<Fn_, clapi::tseq<Params_...>> :
    std::invoke_result<decltype(Fn_), Params_> {};

  template <auto Fn_>
    requires clapi::plain_function_pointer<Fn_>
  using result_of = typename _result_of_helper<Fn_>::type;

```
endcode }}} */
//
// Since one is required to go through and effort of deducing the function-type
// parameters anyway - the other (more generic) way was choosen to be utilized.
// }}}

template <auto Fn_>
  requires function_pointer<Fn_>
           and /* TODO: for now */ plain_function_pointer<Fn_>
using result_of = typename _detail::deduced::_result_of<Fn_>::type;

//----------------------------------------------------------------------------------------
// last_param_of - type of function literal last param (SFINAE-fail if the arity == 0)
//----------------------------------------------------------------------------------------
template <auto Fn_>
  requires function_pointer<Fn_>
           and /* TODO: for now */ plain_function_pointer<Fn_>
using last_param_of = typename _detail::deduced::_last_param_of<Fn_>::type;

//----------------------------------------------------------------------------------------
// param_count_of - function literal arity (number of function literal parameters)
//----------------------------------------------------------------------------------------
template <auto Fn_>
  requires function_pointer<Fn_>
           and /* TODO: for now */ plain_function_pointer<Fn_>
[[deprecated("use clapi::deduced::arity_of")]]
constexpr inline auto param_count_of = _detail::deduced::_arity_of<Fn_>;

//----------------------------------------------------------------------------------------
// arity_of - function literal arity (number of function literal parameters)
//----------------------------------------------------------------------------------------
template <auto Fn_>
  requires function_pointer<Fn_>
  and /* TODO: for now */ plain_function_pointer<Fn_>
constexpr inline auto arity_of = _detail::deduced::_arity_of<Fn_>;

//----------------------------------------------------------------------------------------
// noexcept_qual_of - noexcept qualifier of function literal [clapi::boolean]
//----------------------------------------------------------------------------------------
template <auto Fn_>
  requires function_pointer<Fn_>
           and /* TODO: for now */ plain_function_pointer<Fn_>
[[maybe_unused]]
constexpr inline auto noexcept_qual_of = [] /*ASSERT()*/ consteval static -> clapi::aye
{
/* Always assumed to be true! {{{ */
  static_assert(_detail::deduced::_noexcept_qual_of<Fn_> == true,
                "Should be assumed for C-functions,"
                " regardless specified qualifier");

  return {};
// }}}
}();

} // namespace clapi::deduced

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
