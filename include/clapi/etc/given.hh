#pragma once

#include "clapi/etc/premise.hh"

#include <concepts>
#include <functional>
#include <ranges>
#include <type_traits>

//----------------------------------------------------------------------------------------

namespace clapi::_detail::inline etc
{

//----------------------------------------------------------------------------------------
// _cond_invocable - iff condition result of invocable_r<void> otherwise true
//----------------------------------------------------------------------------------------

template <bool Cond_, typename FnTy_>
concept _cond_invocable =
  (Cond_ and std::is_invocable_r_v<void, FnTy_>) or (not Cond_);

//----------------------------------------------------------------------------------------
// _unless_cond_invocable - iff not condition result of invocable_r<void> otherwise true
//----------------------------------------------------------------------------------------

template <bool Cond_, typename FnTy_>
concept _unless_cond_invocable =
  (not Cond_ and std::is_invocable_r_v<void, FnTy_>) or Cond_;

//----------------------------------------------------------------------------------------
// _cond_noexcept_invocable - as _cond_invocable but nothrow for noexcept check
//----------------------------------------------------------------------------------------

template <bool Cond_, typename FnTy_>
concept _cond_noexcept_invocable =
  (Cond_ and std::is_nothrow_invocable_r_v<void, FnTy_>) or (not Cond_);

//----------------------------------------------------------------------------------------
// _unless_cond_noexcept_invocable - negated cond in _cond_noexcept_invocable
//----------------------------------------------------------------------------------------

template <bool Cond_, typename FnTy_>
concept _unless_cond_noexcept_invocable =
  (not Cond_ and std::is_nothrow_invocable_r_v<void, FnTy_>) or Cond_;

} // namespace clapi::_detail::inline etc

namespace clapi::inline etc::inline causality
{


//----------------------------------------------------------------------------------------
// reasoning - four-boolean nondeterministic conservative combinatory logic effect.
//----------------------------------------------------------------------------------------
//
// Note: Strongly over-generalized for clapi purpouses only consequence and repersussion
//       are used, as there is no real need for reasoning about equivalence of unknown,
//       nor inconsistent propositions.
//       Thus one won't ever see fallacy or superstiton in context of given_t.
enum class reasoning
{
//----------------------------------------------------------------------------------------
// consequece - a detremination made base on both confimed and not refuted `premise_t`
//----------------------------------------------------------------------------------------
  consequence,
//----------------------------------------------------------------------------------------
// repercussion - the determination made base on refuted and not confimed `premise_t`
//----------------------------------------------------------------------------------------
  repercussion,
//----------------------------------------------------------------------------------------
// fallacy - the detrmination made base on neither confirmed nor refuted `premise_t`
//----------------------------------------------------------------------------------------
  fallacy,
//----------------------------------------------------------------------------------------
// superstition - the determination made base on both confimed and refuted `permise_t`
//----------------------------------------------------------------------------------------
  superstition,
};

//----------------------------------------------------------------------------------------
// reason_about [PremiseType] - evaluates premise to actionable/non-actionable categories
//----------------------------------------------------------------------------------------

template <typename Premise_>
static consteval auto reason_about() -> reasoning
{
  using enum reasoning;

  if constexpr (Premise_::confimed and not Premise_::refuted) return consequence;
  if constexpr (Premise_::refuted and not Premise_::confimed) return repercussion;
  if constexpr (not Premise_::refuted and not Premise_::confimed) return fallacy;
  if constexpr (Premise_::refuted and Premise_::confimed) return superstition;
}

constexpr bool is_decided(reasoning r)
{
  using enum reasoning;
  return (r == consequence) || (r == repercussion);
}

//----------------------------------------------------------------------------------------
// given_t - an assumption made under verity of `permise_t` determinig casual actions.
//----------------------------------------------------------------------------------------

template <reasoning Reason_>
struct given_t
{
  using enum reasoning;
  using reasoning_t = constant<Reason_>;

  static constexpr reasoning_t destiny{};

  static constexpr bool consequential_fate = (destiny == consequence);
  static constexpr bool repercussional_fate = (destiny == repercussion);

  static constexpr bool actionable_fate = is_decided(destiny);

  explicit constexpr given_t() = default;

  template <bool Verity_, auto Basis_>
  constexpr given_t(premise_t<Verity_, Basis_>) {}

  consteval operator bool() const noexcept
     requires (is_decided(destiny))
  { return destiny == consequence; }

  static constexpr auto then(std::invocable auto &&choice)
    noexcept(_detail::_cond_noexcept_invocable<consequential_fate,
                                               decltype(choice)>) -> given_t
    requires _detail::_cond_invocable<consequential_fate, decltype(choice)>
  {
     if constexpr (consequential_fate)
       std::invoke_r<void>(std::forward<decltype(choice)>(choice));

     return given_t{};
  }

  constexpr auto or_else(std::invocable auto &&choice)
    noexcept(_detail::_cond_noexcept_invocable<repercussional_fate,
                                               decltype(choice)>) -> given_t
    requires _detail::_cond_invocable<repercussional_fate, decltype(choice)>
  {
    if constexpr (destiny == repercussion)
      std::invoke_r<void>(std::forward<decltype(choice)>(choice));

    return given_t{};
  }

  constexpr auto irregardless(std::invocable auto &&necessity)
    noexcept(std::is_nothrow_invocable_r_v<void, decltype(necessity)>) -> given_t
    requires std::is_invocable_r_v<void, decltype(necessity)>
  {
    std::invoke_r<void>(std::forward<decltype(necessity)>(necessity));

    return given_t{};
  }
};

// given_t - CTAD
template <bool V_, auto B_>
given_t(premise_t<V_, B_>) -> given_t<reason_about<premise_t<V_, B_>>()>;

} // namespace clapi::inline etc

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
