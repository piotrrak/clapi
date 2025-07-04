#pragma once

#include "clapi/etc/seq.hh"
#include "clapi/etc/type_constants.hh"

#include <compare>

namespace clapi::inline etc::inline causality
{

//----------------------------------------------------------------------------------------
//
// Note: This is a little framework an unfinished 4-boolean logic
//   For purpouse of clapi only little part of it is of any usefullness. {{{
//
//   Curiously development of it lead to superposition and entanglement ideas naturally
//   occuring as an solution to requirement of preseving consistency.
//
//   When considering contradiction of equivalnce of unknown lead to idea of super-axiom
//   of belief, that needs to be in superposition for equivalence and its contradiction.
//
//   Analogous superpositions seem to occur when dealing with self-contradictions that
//   together with unique_unknowns hinted the need of represanting believe.
//
//   Inablity to decide which belief (that true or that false) should side on lead to
//   analog of superposition of those two due to need of consistent negation
//   (contradiction) of such.
//
//   In hinsight it seams obvious that an attempt to reason about those lead to
//   non-classical (non-deterministic model) model and idea of using superposition
//   (an alterntive of beliefs).
//
//   In such way this theory could be considered conservative, by lifting nondeterminism
//   as representation of (an alternative of could be beliefs).
//
// Also most likely, that act of making judgment should be extended to depend on previous
// apiori pretences to make it of any use for consistent casual reasoning in consistent
// manner (with way back to determinsm).
//
// Beyond that let's let it stay as fun curiosity and little mental excersise of an author
//
// }}}
//----------------------------------------------------------------------------------------

template <bool Veracity_>
struct axiomatic_t : boolean<Veracity_> {};

// TODO: should occur in superposition
template <typename Ty_>
concept belief_type = false;

template <auto Pretence_>
concept belief = belief_type<decltype(Pretence_)>;

template <bool Veracity_>
struct ubiquitous_judgment_t;

template <auto = []{}>
struct unknown_unique_judgment_t;

template <auto = []{}>
struct known_unique_judgment_t; // unimplemented

//----------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------
using ubiq_falsity = ubiquitous_judgment_t<false>;

//----------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------
using ubiq_truth = ubiquitous_judgment_t<true>;

template <typename Ty_>
concept ubiquitous_judgment_type = same_as<Ty_, ubiq_falsity>
  or same_as<Ty_, ubiq_truth>;

template <typename Ty_>
concept unique_judgment_type = not ubiquitous_judgment_type<Ty_>; // Incomplete

// Judgment - for a lack of better word as it turned out to also fit stating
// act of stating an axiom - ie. ubiquitous_judgment_type<>
template <typename Ty_>
concept judgment_type = unique_judgment_type<Ty_> or ubiquitous_judgment_type<Ty_>;

template <auto Jdg_>
concept judgment = judgment_type<decltype(Jdg_)>;

template <typename Ty_>
concept pretence_type = judgment_type<Ty_> or belief_type<Ty_>;

// Act of stating a judgment or belief
template <auto Basis_>
concept pretence = pretence_type<decltype(Basis_)>;

template <bool Veracity_>
struct ubiquitous_judgment_t final : axiomatic_t<Veracity_>
{
  consteval auto contradiction(this ubiquitous_judgment_t j)
  {
    if constexpr (j) return ubiq_falsity{};
    else return ubiq_truth{};
  }
};

template <auto>
struct unknown_unique_judgment_t
{
  // Equivalence
  template <auto Unq_>
  friend consteval bool operator==(unknown_unique_judgment_t,
                                   unknown_unique_judgment_t<Unq_>)
  { return false; } // FIXME: incorrect should be superposition of true/false `belief_t`

  template <auto Unq_>
  friend consteval bool operator!=(unknown_unique_judgment_t,
                                   unknown_unique_judgment_t<Unq_>)
  { return false; } // FIXME: incorrect should be entanlment of superposition of `belief_t`

  consteval auto contradiction(this auto self)-> decltype(self)
  { return {}; }
};

template <auto... Jdgs_>
concept all_unique_judgments =
  std::conjunction_v<boolean<unique_judgment_type<decltype(Jdgs_)>>...>;

template <auto... Jdgs_>
concept any_ubiquitous_judgment =
  std::disjunction_v<boolean<ubiquitous_judgment_type<decltype(Jdgs_)>>...>;

template <bool Verity_, auto Basis_ = unknown_unique_judgment_t<>()>
  requires pretence<Basis_>
struct premise_t;

namespace general
{

template <bool Verity_ = false>
  requires (Verity_ == false)
constexpr inline premise_t<Verity_ and false, ubiq_falsity{}> lie{};

template <bool Verity_ = true>
  requires (Verity_ == true)
constexpr inline premise_t<Verity_ or true, ubiq_truth{}> fact{};

} // namespace general

// Self-contradicting/inconsistent
namespace absurd
{

template <bool Verity_ = false>
constexpr inline premise_t<Verity_ and false, ubiq_falsity{}> fact{};

template <bool Verity_ = true>
constexpr inline premise_t<Verity_ or true, ubiq_falsity{}> lie{};

} // namespace absurd

namespace belived
{
//TODO:: lie and fact repesenting

}

consteval auto contradicting(auto j) noexcept
  requires requires (decltype(j) c) {
    { as_constexpr(c).contradiction() } -> pretence_type;
  }
{
  return j.contradiction();
}

template <bool Verity_, auto Jdg_>
  requires ubiquitous_judgment_type<decltype(Jdg_)>
consteval bool is_self_contradiction(premise_t<Verity_, Jdg_>)
{
  return not premise_t<Verity_, Jdg_>::is_concistent();
}

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

template <bool Verity_, auto Basis_>
  requires pretence<Basis_>
struct premise_t : boolean<Verity_>
{
  using veracity_t = boolean<Verity_>;
  using pretence_t = decltype(Basis_);

  static constexpr inline veracity_t verity{};
  static constexpr inline pretence_t basis{};

  static consteval auto is_concistent()
  {
    // TODO: beliefs
    if constexpr (unique_judgment_type<pretence_t>) return clapi::aye{};
    else return boolean_<verity == basis>;
  }

  // XXX: g++-15 won't CTAD over alias here
  static constexpr inline auto /*boolean*/ self_contradicting =
    boolean_<not premise_t::is_concistent()>;

  // XXX: g++-15 won't CTAD over alias here
  static constexpr inline auto /*boolean*/ confimed =
    boolean_<verity and is_concistent()>;

  // XXX: g++-15 won't CTAD over alias here
  static constexpr inline auto /*boolean*/ refuted =
    boolean_<not verity and is_concistent()>;

  // TODO: incorrect - should be belief_t<false|true> if unknown or superposition
  //       of selfcontradictions if known
  template <bool Verity2_, auto Basis2_>
    requires all_unique_judgments<Basis_, Basis2_>
  friend consteval auto operator== (premise_t, premise_t<Verity2_, Basis2_>) noexcept
  { return absurd::fact<>; }

  template <bool Verity2_, auto Basis2_>
    requires all_unique_judgments<Basis_, Basis2_>
  friend consteval auto operator!= (premise_t, premise_t<Verity2_, Basis2_>) noexcept
  { return not absurd::fact<>; } // TODO: incorrect - should be belief_t<true|false>

  static consteval auto contradiction() noexcept ->
    premise_t<not premise_t::verity, contradicting(Basis_)>
  { return {}; }

  consteval auto operator! () const noexcept -> decltype(premise_t::contradiction())
  { return {}; }
};

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

template <bool Verity_>
constexpr inline premise_t<Verity_> premise;

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

} // namespace clapi::inline etc::inline causality

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
