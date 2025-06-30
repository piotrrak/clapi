#pragma once

#include "fwd/clapi/etc/basic.hh"
#include "fwd/clapi/etc/type_constants.hh"

  // TODO: reconsider/check if providing forward includes is of any real value.
  // The modules based compilation, might be some lead.
#include <concepts> // won't get away without that, since we really want constain template
                    // declarations.

namespace clapi::inline sequences
{

//----------------------------------------------------------------------------------------
// iseq<> - the type representing pack of indices (of std::size_t)
//----------------------------------------------------------------------------------------
//
//  Note: an alias to std::index_sequence<...>

template <std::size_t...Idxs_>
using iseq = std::index_sequence<Idxs_...>;

//----------------------------------------------------------------------------------------
// tseq<> - the type representing pack of types.
//----------------------------------------------------------------------------------------

template <typename... Types_>
struct tseq;

//----------------------------------------------------------------------------------------
// vseq<> - the type representing pack of values.
//----------------------------------------------------------------------------------------

template <auto... Values_>
struct vseq;

// Foundational definitions {{{
//
// While technically those are definitions rather then forward declarations,
// those were placed here in oreder to help constrain our forward declarations
// with the concepts.
//
// Thus, those should be considered as foundational for other foward-declarations.
// One ought to try to minimize such cases.
// }}}

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
// indexable_sequence -
//----------------------------------------------------------------------------------------
//
// TODO: finish, seems we'll need non-empty-sequence<> and size_sequence from header
// When at it, it should probably be good to allow ADL injection of size and empty checks
// (by the free functions) and also hook it up with std::tuple_size std::get machinery of
// standard library.
//
// That would allow using seq's with structure bindings, which can introduce a pack since
// c++26. Could be still useful in the c++23 dialect - if fixedn umber of elements in the
// sequence is expected.
template <typename Type_>
concept indexable_sequence = requires {
  { Type_::size      } -> std::convertible_to<std::size_t>;
  { Type_::size()    } noexcept -> std::convertible_to<std::size_t>;
  { Type_::empty     } -> std::convertible_to<bool>;
  { Type_::empty()   } noexcept -> std::convertible_to<bool>;
  { !Type_::empty    } -> std::convertible_to<bool>;
  { !Type_::empty()  } noexcept -> std::convertible_to<bool>;
};

} // clapi::inline sequences::inline concepts

namespace clapi::inline sequences
{

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

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
