#pragma once

#include "fwd/clapi/etc/basic.hh"
#include "fwd/clapi/etc/type_constants.hh"

namespace clapi::_detail::inline sequences
{

//----------------------------------------------------------------------------------------
// _sized_seq - provides seq-like types with the basic size-related members
//----------------------------------------------------------------------------------------

template <std::size_t Size_>
struct _sized_seq;

using std::add_cv_t;

template <typename>
consteval bool _chk_pinterconv_sized_seq_base(add_cv_t<void>*)
{
  return false;
}

template <typename Ty_>
concept _derived_from_sized_seq = _chk_pinterconv_sized_seq_base<Ty_>(nullptr_v<Ty_>);

} // namespace clapi::_detail::inline sequences

namespace clapi::inline sequences
{

//----------------------------------------------------------------------------------------
// iseq<> - the type representing pack of indices (of std::size_t)
//----------------------------------------------------------------------------------------
//
//  Note: just an alias to `std::index_sequence<...>`;
//  TODO: iseq with wrapper API like tseq/vseq and rename?

template <std::size_t...Idxs_>
using iseq = std::index_sequence<Idxs_...>;

//----------------------------------------------------------------------------------------
// tseq<...> - the type representing pack of types.
//----------------------------------------------------------------------------------------

template <typename... Types_>
struct tseq;

//----------------------------------------------------------------------------------------
// vseq<...> - the type representing pack of values.
//----------------------------------------------------------------------------------------

template <auto... Values_>
struct vseq;

template <typename>
constexpr inline ney is_some_tseq = {};

template <typename... Types_>
constexpr inline aye is_some_tseq<tseq<Types_...>>{};

//----------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------

template <typename>
constexpr inline ney is_some_vseq = {};

template <auto ... Values_>
constexpr inline aye is_some_vseq<vseq<Values_...>>{};

//----------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------

template <typename>
constexpr inline ney is_some_iseq = {};

template <std::size_t... Idxs_>
constexpr inline aye is_some_iseq<iseq<Idxs_...>>{};

} // clapi::sequences

namespace clapi::inline sequences::inline concepts
{

//----------------------------------------------------------------------------------------
// index_sequence
//----------------------------------------------------------------------------------------

template <typename Type_>
concept index_sequence = is_some_iseq<Type_>();

//----------------------------------------------------------------------------------------
// type_sequence
//----------------------------------------------------------------------------------------
template <typename Type_>
concept type_sequence = is_some_tseq<Type_>();

//----------------------------------------------------------------------------------------
// value_sequence
//----------------------------------------------------------------------------------------
template <typename Type_>
concept value_sequence = is_some_vseq<Type_>();

//----------------------------------------------------------------------------------------
// basic_sized_sequence -
//----------------------------------------------------------------------------------------
template <typename Type_>
concept basic_sized_sequence = copyable<Type_> and default_initializable<Type_>
  and requires {
  // check for constexpr size/size()
  { clapi::as_constexpr(Type_::size())   } noexcept -> same_as<std::size_t>;
  { clapi::as_constexpr(Type_::size)     }          -> convertible_to<std::size_t>;

  // check for constexpr empty/empty()
  { clapi::as_constexpr(Type_::empty())  } noexcept -> convertible_to<bool>;
  { clapi::as_constexpr(Type_::empty)    }          -> convertible_to<bool>;

  // check for constexpr negated empty/empty()
  { clapi::as_constexpr(!Type_::empty()) } noexcept -> convertible_to<bool>;
  { clapi::as_constexpr(!Type_::empty)   }          -> convertible_to<bool>;
};

template <typename Type_>
concept adl_sized_sequence = copyable<Type_> and requires (Type_ && s) {
  { clapi::as_constexpr(sequence_size(s)) } noexcept -> convertible_to<std::size_t>;
};

template <typename Type_>
concept basic_non_empty_sequence = basic_sized_sequence<Type_> and not (Type_::empty);

} // clapi::inline sequences::inline concepts

namespace clapi::inline sequences
{

//----------------------------------------------------------------------------------------
// opt_in_sized_sequence - enables `sized_sequences` related facilities for the type
//----------------------------------------------------------------------------------------
//
// Note: This requires to provide implemention of the `adl_sequence_size()` in enclosing
// namespace to the type declation of type.
//
// See the example and `concept adl_sized_sequence` definition above:

/* Example code {{{

``` c++

#include <cstddef>
#include <type_traits>

namespace example
{

struct seven_sequence {};

constexpr auto adl_sized_sequence(seven_sequence) noexcept -> std::size_t
{ return 7; }

struct not_sequence {};

} // namespace example

namespace clapi::inline sequences
{

template<>
constexpr inline bool opt_in_sized_sequence = true;

} // clapi::inline sequences

static_assert(clapi::sized_sequences<example::seven_sequence>);
static_assert(not clapi::sized_sequences<example::not_sequence>);

```
}}} */

template <typename Ty_>
constexpr bool opt_in_sized_sequence = false;

//----------------------------------------------------------------------------------------
// adl_sequence_size - asserting fallback sequence_size
//----------------------------------------------------------------------------------------
//
// Note: It might be overriden for external type to enable `sized_sequence` facilities.
consteval inline auto adl_sequence_size(auto s) noexcept
  requires (not basic_sized_sequence<decltype(s)>)
{
  using seq_t = decltype(s);

  static_assert(always_false_v<seq_t>
                or not opt_in_sized_sequence<seq_t>,
                "Missing the user specialization for adl_sequence_size()\n"
                "Please see the note about opt_in_sized_sequence_above.");
  static_assert(always_false_v<seq_t>,
                "missing specialization for adl_sequence_size");

  // NB: Forced type-dependant expression, to avoid premature empty_t<> instantiation.
  return empty_t<42 - int(always_false_v<seq_t>)>{};
};

//----------------------------------------------------------------------------------------
// adl_sequence_size - specialization for `basic_sized_sequence` concept
//----------------------------------------------------------------------------------------
constexpr inline auto adl_sequence_size(basic_sized_sequence auto s) noexcept
{
  return decltype(s)::size;
}

consteval auto sequence_size(auto s) noexcept
  requires (basic_sized_sequence<decltype(s)> or adl_sized_sequence<decltype(s)>)
{
  if constexpr (basic_sized_sequence<decltype(s)>)
    return decltype(s)::size;
  else
    return size_<adl_sequence_size(s)>;
}

consteval auto sequence_empty(auto s) noexcept
  requires (basic_sized_sequence<decltype(s)> or adl_sized_sequence<decltype(s)>)
{
  if constexpr (basic_sized_sequence<decltype(s)>)
    return decltype(s)::empty;
  else
    return boolean_<std::size_t{0} == adl_sequence_size(s)>;
}


//----------------------------------------------------------------------------------------
// iseq_for<>
//----------------------------------------------------------------------------------------
//
// TODO: Might use compiler builtin if it makes compilation any faster.

template <typename... Types_>
using iseq_for = std::index_sequence_for<Types_...>;

//----------------------------------------------------------------------------------------
// iseq_for_n<>
//----------------------------------------------------------------------------------------

template <std::size_t N_>
using iseq_for_n = std::make_index_sequence<N_>;

//----------------------------------------------------------------------------------------
// tseq_for<>
//----------------------------------------------------------------------------------------

template <auto... Values_>
using tseq_for = tseq<nontype_t<Values_>...>;

//----------------------------------------------------------------------------------------
// vseq_for<>
//----------------------------------------------------------------------------------------

template <typename... Types_>
using vseq_for = vseq<itstype_t<Types_>{}...>;

//----------------------------------------------------------------------------------------
// tseq_gather<>
//----------------------------------------------------------------------------------------

template <index_sequence Seq_, typename...>
struct tseq_gather;

//----------------------------------------------------------------------------------------
// vseq_gather<>
//----------------------------------------------------------------------------------------

template <index_sequence Seq_, auto...>
struct vseq_gather;

//----------------------------------------------------------------------------------------
// iseq_gather<>
//----------------------------------------------------------------------------------------

template <index_sequence Seq_, std::size_t...>
struct iseq_gather;

} // clapi::sequences

namespace clapi::inline sequences::inline concepts
{

//----------------------------------------------------------------------------------------
// sized_sequence -
//----------------------------------------------------------------------------------------
template <typename Ty_>
concept sized_sequence = opt_in_sized_sequence<Ty_>
  or _detail::_derived_from_sized_seq<Ty_>
  or basic_sized_sequence<Ty_>;

template <typename Type_>
concept non_empty_sequence = basic_non_empty_sequence<Type_>
  or (not sequence_empty(_detail::_unprotected_declval<Type_>()));


//----------------------------------------------------------------------------------------
// basic_indexable_sequence -
//----------------------------------------------------------------------------------------
//
// TODO: finish, seems we'll need non-empty-sequence<> and size_sequence from header
// When at it, it should probably be good to allow ADL injection of size and empty checks
// (by the free functions) and also hook it up with std::tuple_size std::get machinery of
// standard library.
//
// That would allow using seq's with structure bindings, which can introduce a pack since
// c++26. Could be still useful in the c++23 dialect - if fixed number of elements in the
// sequence is expected.

// XXX: g++ has problem parsing correctly requires expression.
//      Thus here's version it will accept.
//
//  Considering following minimal case highlighting
//
// [[expr.prim.req.general]] 7.5.8.1 p1, p4
// production of parameter-declaration-clause should be: `parameter-declaration-clause`
// and should be accepted as `foo` above ([dcl.fct]).
// g++-15.1 incorrectly rejects it.

/*
{{{
``` c++

template <typename T, T V>
struct int_cst
{
  constexpr static inline T value = V;
};

template <typename Type>
void foo(int_cst<decltype(Type::value), Type::value>) {};

template <typename Type>
concept Foo = requires (int_cst<decltype(Type::value), Type::value> t) {
  {Type::value };
};

```

}}}
*/

template <typename Type_>
concept basic_indexable_sequence_gcc = basic_non_empty_sequence<Type_>
  and requires {

  typename Type_::template type_at<unsigned(0)>;

  // checks if typename type_at<I> is a type for all valid values of `I` (indices)
  {
    []<auto... idxs> (iseq<idxs...>) consteval {
      (void)((void)itstype_t<typename Type_::template type_at<idxs>>{}, ..., 0);
    }(iseq_for_n<Type_::size>{})
  } -> same_as<void>;

  // checks if get_v<I> is a value for all valid values of `I`
  {
    []<auto... idxs> (iseq<idxs...>) consteval {
      (void)((void)Type_::template get_v<idxs>, ..., 0);
    }(iseq_for_n<Type_::size>{})

  } -> same_as<void>;

  // checks if operator[] (constant_<I>> is a value for all valid values of `I`
  {
    []<auto... idxs> (iseq<idxs...>) consteval {
      (void)((void)Type_{}[size_<idxs>], ...);
    }(iseq_for_n<Type_::size>{})

  } -> same_as<void>;
};

template <typename Type_>
concept basic_rev_indexable_sequence_gcc = basic_indexable_sequence_gcc<Type_>
  and requires {

  // checks if operator[] is of the same type when indexing from end (negative)
  {
    []<auto... idxs> (iseq<idxs...>) consteval {
      constexpr static auto N = size_<sizeof...(idxs)>;
      constexpr static auto are_symetric =
        boolean_<(same_as<
                  decltype(Type_{}[size_<N-1 - idxs>]),
                  decltype(Type_{}[ssize_<signed(-N+1 + idxs)>])> && ...  && true)>;

      return are_symetric;
    }(iseq_for_n<Type_::size>{})

  } -> same_as<aye>;
};

#if defined(__clang__)

// XXX: Rejected by g++
template <typename Type_>
concept basic_indexable_sequence_clang = basic_non_empty_sequence<Type_>
  and requires (Type_ && seq, size<Type_::size> N, iseq_for_n<Type_::size> rng) {

  typename Type_::template type_at<unsigned(0)>;

  // checks if typename type_at<I> is a type for all valid values of `I` (indices)
  {
    []<auto... idxs> (iseq<idxs...>) consteval {
      (void)((void)itstype_t<typename Type_::template type_at<idxs>>{}, ..., 0);
    }(rng)
  } -> same_as<void>;

  // checks if get_v<I> is a value for all valid values of `I`
  {
    []<auto... idxs> (iseq<idxs...>) consteval {
      (void)((void)Type_::template get_v<idxs>, ... , 0);
    }(rng)

  } -> same_as<void>;

  // checks if operator[] (constant_<I>> is a value for all valid values of `I`
  {
    []<auto... idxs> (iseq<idxs...>) consteval {
      (void)((void)seq[size_<idxs>], ..., 0);
    }(rng)

  } -> same_as<void>;
};

template <typename Type_>
concept basic_rev_indexable_sequence_clang = basic_indexable_sequence_clang<Type_>
  and requires (Type_ && seq, size<Type_::size> N, iseq_for_n<Type_::size> rng) {
  // checks if operator[] is of the same type when indexing from end (negative)
  {
    []<auto... idxs> (iseq<idxs...>) consteval {
      constexpr static boolean are_symetric =
        boolean_<(same_as<decltype(seq[size_<N-1 - idxs>]),
                          decltype(seq[ssize_<signed(-N+1 + idxs)>])> && ... && true)>;

      return are_symetric;
    }(rng)
  } -> same_as<aye>;
};

#endif

#if defined(__clang__)

template <typename Type_>
concept basic_indexable_sequence = basic_indexable_sequence_clang<Type_>;

template <typename Type_>
concept basic_rev_indexable_sequence = basic_rev_indexable_sequence_clang<Type_>;

#elif defined(__GNUG__)

template <typename Type_>
concept basic_indexable_sequence = basic_indexable_sequence_gcc<Type_>;

template <typename Type_>
concept basic_rev_indexable_sequence = basic_rev_indexable_sequence_gcc<Type_>;

#endif


} // clapi::inline sequences::inline concepts

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
