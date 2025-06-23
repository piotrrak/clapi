#pragma once

#include "fwd/clapi/etc/seq.hh"

#include "clapi/etc/basic.hh"
#include "clapi/etc/compiler.hh"

namespace clapi::detail::inline sequences
{

//----------------------------------------------------------------------------------------
// _sized_seq - base class for sequences
//----------------------------------------------------------------------------------------
template <std::size_t Size_>
struct _sized_seq
{
  static constexpr inline auto size = size_<Size_>;
  static constexpr inline auto empty = boolean_<Size_ == 0>;

  template <typename IndexedTy_ = unsigned>
  static consteval auto in_range(unsigned_integral auto idx) noexcept -> bool
  {
    return std::in_range<IndexedTy_>(idx) && (idx < size());
  }

  template <typename IndexedTy_ = unsigned>
  static consteval auto in_range(signed_integral auto idx) noexcept -> bool
  {
    constexpr decltype(idx) Zero{0};
    if (idx < Zero)
      return std::in_range<IndexedTy_>(size + idx) and (size() <= -idx);
    else
      return std::in_range<IndexedTy_>(idx) and (idx < size());
  }
};

// The brief explaination of implementation: {{{
//
// Somewhat in spirit of [P2098R1](https://wg21.link/P2098R1)
// This implementiation is very limited specialized for templates with signle nontype
// parameter.
//
// Dealing with variadic mix of nontype and typename template parameters has no proper
// solution as of today.
//
// That caused this faciltiy to never be integrated into c++ standard library.
// Main reason being, that nontype template parameters are in few aspects second category.
//
// Yet, in such limited usecases such specialized facility is useful.
// In this case, it's prefered solution, as it leaves concept of sequence more open.
// This could have been handled with different strategy.
// }}}

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

template <typename Ty_>
  requires requires {
    {Ty_::size } -> convertible_to<std::size_t>;
    {Ty_::size() } -> same_as<std::size_t>;
  }
constexpr std::size_t _size_v_impl = Ty_::size();

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

template <typename Ty_>
constexpr empty<> _size_v{};

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

template <typename Ty_>
  requires requires { {_size_v_impl<Ty_>}; }
constexpr size_t _size_v<Ty_> = _size_v_impl<Ty_>;

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

template <std::size_t... Idxs_>
constexpr size_t _size_v<iseq<Idxs_...>> = sizeof...(Idxs_);

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

template <typename TypeOfNontype_,
          typename Ty_,
          template <TypeOfNontype_> class Primary_>
constexpr inline bool _is_nontype1_specialization_of = false;

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

template <typename TypeOfNontype_,
          template <TypeOfNontype_> class Primary_,
          TypeOfNontype_ Value_>
constexpr inline bool _is_nontype1_specialization_of<TypeOfNontype_,
                                                     Primary_<Value_>,
                                                     Primary_> = true;

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

template <typename Ty_>
concept _not_sized_seq =
  not _is_nontype1_specialization_of<std::size_t, Ty_, _sized_seq>;

using std::add_cv_t;

template <typename>
consteval bool _chk_pinterconv_sized_seq_base(add_cv_t<void>*)
{
  return false;
}

template <typename Ty_,
          unsigned N_ = _size_v<Ty_>>
  requires _not_sized_seq<Ty_>
consteval bool _chk_pinterconv_sized_seq_base(add_cv_t<_sized_seq<N_>>*)
{
  return std::is_pointer_interconvertible_base_of_v<_sized_seq<N_>, Ty_>;
}

template <typename Ty_>
concept _derived_from_sized_seq = _chk_pinterconv_sized_seq_base<Ty_>(nullptr_v<Ty_>);

}
namespace clapi::inline sequences::inline concepts
{

//----------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------

template <typename Ty_>
constexpr bool opt_in_sized_sequence = false;

//----------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------
template <typename Ty_>
concept sized_sequence = opt_in_sized_sequence<Ty_>
  or detail::_derived_from_sized_seq<Ty_>;

} // namespace clapi::inline sequences inline concepts

namespace clapi::inline sequences
{

//  Note: Those specialization serves as an aid to SFINAE-friendliness {{{
//
//    clang SFINAE-fails in concept can_deduce_with for an empty an empty `tseq<>`
//    g++ bails trying instantiate `tseq<>::template type_at` which hard fails.
//    Below gives it a hand and provide tseq<> without type_at
//    causing it can SFINAE-fail earlier, rather than hard fail.
//
//  }}}

//----------------------------------------------------------------------------------------
// tseq<> - specialization of empty `tseq<>`
//----------------------------------------------------------------------------------------
template <>
struct tseq<> : detail::_sized_seq<0> {};

//----------------------------------------------------------------------------------------
// vseq<> - specialization of empty `vseq<>`
//----------------------------------------------------------------------------------------

template <>
struct vseq<> : detail::_sized_seq<0> {};

// We'll use pack indexing, clang/g++ would warn.
// While it's doable without since c++, but just harder and slower to compile.
//
// c++26 for pack-indexing
_clapi_BEGIN_ALLOW_CPP26()

//----------------------------------------------------------------------------------------
// tseq<...> - type repersentation of sequence of types (without storage)
//----------------------------------------------------------------------------------------

template <typename... Types_>
struct tseq : detail::_sized_seq<sizeof...(Types_)>
{
  template <unsigned Idx_>
    requires (tseq::in_range(Idx_))
  using type_at = Types_...[Idx_];

  // NB: boxed as value in `itstype<>`
  template <unsigned Idx_>
    requires (tseq::in_range(Idx_))
  static constexpr inline auto get_v = itstype<Types_...[Idx_]>;

  template <unsigned_integral UIdxTy_, UIdxTy_ Idx_>
    requires (tseq::in_range(Idx_))
  [[nodiscard]] static consteval auto operator[](constant<Idx_>) noexcept
  {
    return tseq::get_v<Idx_>;
  }

  template <signed_integral SIdxTy_, SIdxTy_ Idx_>
    requires (tseq::in_range(Idx_))
  [[nodiscard]] static consteval auto operator[](constant<Idx_>) noexcept
  {
    if constexpr(Idx_ >= 0)
      return tseq::get_v<unsigned(Idx_)>;
    else
      return tseq::get_v<unsigned(tseq::size + Idx_)>; // NB: negative wrap-around
  }
};

//----------------------------------------------------------------------------------------
// vseq<...> - the type repersentation of sequence of literals (without storage)
//----------------------------------------------------------------------------------------

template <auto... Values_>
struct vseq : detail::_sized_seq<sizeof...(Values_)>
{
  template <unsigned Idx_>
    requires (vseq::in_range(Idx_))
  static constexpr inline auto get_v = Values_...[Idx_];

  template <unsigned Idx_>
    requires (vseq::in_range(Idx_))
  using type_at = nontype_t<Values_...[Idx_]>;

  template <unsigned_integral UIdxTy_, UIdxTy_ Idx_>
    requires (vseq::in_range(Idx_))
  static consteval auto operator[](constant<Idx_>) noexcept
  {
    return get_v<Idx_>;
  }

  template <signed_integral SIndexTy_, SIndexTy_ Idx_>
    requires (vseq::in_range(Idx_))
  static consteval auto operator[](constant<Idx_>) noexcept
  {
    if constexpr(Idx_ >= 0)
      return vseq::get_v<unsigned(Idx_)>;
    else
      return vseq::get_v<unsigned(vseq::size + Idx_)>; // NB: negative wrap-around
  }
};

//----------------------------------------------------------------------------------------
// get<I>(seq) - returns value at index I [value or itstype<T>]
//----------------------------------------------------------------------------------------

template <std::size_t Idx_,
          typename Seq_>
  requires detail::_derived_from_sized_seq<Seq_>
consteval auto get(Seq_) noexcept -> decltype(Seq_::template get_v<Idx_>)
{
  return Seq_::template get_v<Idx_>;
}

//----------------------------------------------------------------------------------------
// tseq_gather - `tseq<>` of pack-params at indices given in `iseq<>` [typename ::type]
//----------------------------------------------------------------------------------------
//
//  Note: the specialization of `tseq_gather<index_sequence, typename...>` {{{
//
//  ``` c++
//    template <index_sequence Seq_, typename...>
//    struct tseq_gather;
//  ```
//  }}}

template <std::size_t... Idxs_,
          typename... Types_>
  requires (tseq<Types_>::in_range(Idxs_) && ... && true)
struct tseq_gather<iseq<Idxs_...>, Types_...>
{
  using type = tseq<Types_...[Idxs_]...>;
};

//----------------------------------------------------------------------------------------
// vseq_gather - `vseq<...>` of pack-params at indices given in `iseq<>` [typename ::type]
//----------------------------------------------------------------------------------------
//
//  Note: the specialization of `vseq_gather<index_sequence, auto...>` {{{
//
//  ``` c++
//    template <index_sequence Seq_, auto...>
//    struct vseq_gather;
//  ```
//  }}}

template <std::size_t... Idxs_,
          auto... Values_>
  requires (vseq<Values_...>::in_range(Idxs_) && ... && true)
struct vseq_gather<iseq<Idxs_...>, Values_...>
{
  using type = vseq<Values_...[Idxs_]...>;
};

//----------------------------------------------------------------------------------------
// iseq_gather - `iseq<...>` of pack-params at indices given in `iseq<>` [typename ::type]
//----------------------------------------------------------------------------------------
//
//  Note: the specialization of: `iseq_gather<index_sequence, std::size_t...>` {{{
//
//  ``` c++
//    template <index_sequence Seq_, std::size_t...>
//    struct iseq_gather;
//  ```
//  }}}

template <std::size_t... Idxs_,
          std::size_t... Values_>
  requires (vseq<Values_...>::in_range(Idxs_) && ... && true)
struct iseq_gather<iseq<Idxs_...>, Values_...>
{
  using type = vseq<Values_...[Idxs_]...>;
};

_clapi_END_ALLOW_CPP26()

} // namespace clapi::inline sequences

// Asserts: {{{
namespace clapi::detail::inline sequences
{

static_assert(not _derived_from_sized_seq<_sized_seq<0>>);
static_assert(_derived_from_sized_seq<tseq<>>);
static_assert(_derived_from_sized_seq<vseq<>>);
static_assert(not _derived_from_sized_seq<iseq<>>);

}
// }}}

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
