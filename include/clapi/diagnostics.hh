#pragma once

#include "clapi/etc/basic.hh"
#include "clapi/etc/given.hh"
#include "clapi/deduced/function_pointer.hh"

#include <source_location>
#include <print>

namespace clapi::inline diag
{

enum struct SLocTrackingPolicy
{
  Never,
  Always,
};

enum struct LoggingPolicy
{
  Never,
  OnError,
  Always,
};

constexpr auto SLocTracking = SLocTrackingPolicy::Always;
constexpr auto CLAPILogging = LoggingPolicy::OnError;

template <auto Unknown, std::equality_comparable = decltype(Unknown)>
constexpr given_t given_policy = general::lie<>;

template <auto Policy_>
constexpr given_t unless_policy = premise<given_policy<Policy_>>.contradiction();

template <LoggingPolicy Policy_>
constexpr given_t given_policy<Policy_> = premise<(Policy_ == CLAPILogging)>;

template <SLocTrackingPolicy Policy_>
constexpr given_t given_policy<Policy_> = premise<(Policy_ == SLocTracking)>;

using SLoc = std::source_location;

consteval inline auto here(SLoc loc = SLoc::current())
{
  return loc;
}

template <bool Enabled_ = unless_policy<SLocTrackingPolicy::Never>>
struct sloc_tracking
{
  constexpr sloc_tracking(SLoc l) noexcept : loc(l) {}

  constexpr operator bool(this auto &&) noexcept { return Enabled_; }

  explicit operator SLoc(this auto && self)
  {
    return std::forward_like<decltype(self)>(self.loc);
  }

  auto get() const & -> SLoc { return *this; }
  auto get() && -> SLoc { return std::move(loc); }

  SLoc loc;
};

template <>
struct sloc_tracking<false>
{
  constexpr sloc_tracking([[maybe_unused]]SLoc _) {}

  consteval operator bool() const & noexcept { return ney{}; }
  consteval operator bool() && noexcept { return ney{}; }
};

template <typename FnTy_>
  requires deduced::nontype_function_pointer_type<FnTy_>
consteval auto TODO_ugly_unportable_name__(FnTy_) noexcept {
  constexpr std::string_view full{
#ifdef __clang__
  here().function_name()
#elifdef __GNUG__
  __PRETTY_FUNCTION__
#else
  "<*unsupported*>"
#endif
  };

  auto match_end = [](auto s, std::string_view pat) consteval static
  { return s.find(pat) + pat.size(); };

#if __clang__
  // clang shows it's a pointer
  constexpr auto pos = match_end(full, "<&");
#elif __GNUG__
  // gcc - does not
  constexpr auto pos = match_end(full, "<");
#endif

#if defined(__clang__) || defined(__GNUG__)
  constexpr auto end = full.find(">", pos);
  constexpr auto ret = full.substr(pos, end-pos);
#else
  constexpr auto ret = full;
#endif

  return ret;
};

template <auto Fn_, bool Enabled_>
auto log_API(nontype_t<Fn_> fn,
             sloc_tracking<Enabled_> diag)
{
  using namespace std::string_view_literals;

  if constexpr (diag)
  {
    constexpr auto name = TODO_ugly_unportable_name__(fn);

    SLoc loc{std::move(diag)};
    std::println(stderr,
                 "*** called {:30} {:>1}{}:{}",
                 name, "at"sv, loc.file_name(), loc.line());
  }

  return clapi::aye{};
}

using no_log_error_t = struct {};

[[maybe_unused]]
constexpr inline no_log_error_t no_log_error{};
constexpr inline no_log_error_t ExpectedFailure{};

} // namespace clapi::inline diag

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
