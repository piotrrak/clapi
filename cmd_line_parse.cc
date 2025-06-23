#include "cmd_arg_parse.hh"


#include <cassert>
#include <filesystem>
#include <format>
#include <print>
#include <ranges>
#include <span>
#include <vector>

// Mix and match with ranges_v3
#if _clapi_MISSING_RANGES_CONCAT
#include <range/v3/all.hpp>
#endif

using namespace cmdline;

namespace cmdline { namespace {

#if _clapi_MISSING_RANGES_CONCAT == 1
using namespace ranges::views;
namespace rng = ::ranges;
#else
using namespace std::views;
namespace rng = std::ranges;
#endif

template <auto N>
using array_sv = std::array<std::string_view, N>;

constexpr auto as_string_view =
  [] [[nodiscard]]
  (auto &s) static noexcept
{
  using std::string_view, std::same_as;

  if constexpr (requires { {s.c_str()} -> std::same_as<const char*>; })
    return string_view{s.c_str()};
  else
    return string_view(s);
};

// Helper to join range elements with space,
// since we don't have range_formatters yet
constexpr static inline auto
join_str =
  [] [[nodiscard]]
  (std::ranges::viewable_range auto &&r) static -> std::string
{
  using std::string;

  return std::forward<decltype(r)>(r)
         | std::ranges::views::join_with(", "sv)
         | std::ranges::to<string>();
};

[[nodiscard]]
auto _verify_allowed(const std::string &full_cmdline,
                     const args_set_t &switches,
                     auto &err_fns) -> expected_or_str<>
{
  using std::string_view, std::vector;

  auto & [err, take_err] = err_fns;

  array_sv allowed_args = allowed_cmd_switches();
  // We will use sorted set operations
  rng::sort(allowed_args);

  // Calculate set of unknown_args args, ie:
  //  set_difference of sorted invocation arguments and sorted allowed arguments
  {
    vector<string_view> unknown_args;
    rng::set_difference(switches, allowed_args, back_inserter(unknown_args));

    if (not unknown_args.empty())
    {
      err("ERROR: In invocation: `{}'", full_cmdline);
      err(" Found the unknown argument(s):");
      err(" {}", join_str(unknown_args));
      err(" while the allowed arguments are:");
      err(" {}", join_str(allowed_args));

      return take_err();
    }
  }

  return {};
}

template <std::size_t N_> [[nodiscard]]
auto _verify_excl_group(const std::string &full_cmdline,
                       args_set_t &switches,
                       array_sv<N_> switch_group,
                       auto &err_fns) -> expected_or_str<>
{
  using std::string_view, std::vector;

  static_assert(not switch_group.empty());

  auto group_default = switch_group[0];

  auto & [err, err_return] = err_fns;
  rng::sort(switch_group);

  {
    vector<string_view> colliding_args;

    rng::set_intersection(switches,
                          switch_group,
                          back_inserter(colliding_args));

    if (colliding_args.size() > 1)
    {
      err("ERROR: In invocation: `{}'", full_cmdline);
      err(" Found the mutually exclusive argument(s):");
      err("  {}", join_str(colliding_args));
      err(" Please specify at most one of:");
      err(" {}", join_str(switch_group));

      return err_return();
    }

    if (colliding_args.empty())
      switches.emplace(group_default);
  }

  return {};
}

[[nodiscard]]auto
_verify_arguments(const std::string &full_cmdline,
                           args_set_t & switches) -> expected_or_str<>
{
  using std::format_string, std::println, std::ostringstream;
  using std::unexpected;

  // If an error occurs format to string
  ostringstream err;

  // formatter helper
  auto format_to_err =
    [&err] <typename... Args_>
    [[gnu::always_inline]]

    (format_string<Args_...> fmt, Args_ &&...a) -> void
  {
    err << ">> "sv;
    println(err, fmt, std::forward<Args_>(a)...);
  };


  auto take_error_str = [&err]
  [[nodiscard]] noexcept {
    return unexpected{std::move(err).str()};
  };

  // Callbacks passed dot verify functions
  auto err_fns = std::pair{
    std::move(format_to_err),
    std::move(take_error_str)
  };

  auto result = _verify_allowed(full_cmdline,
                                switches,
                                err_fns);

  if (!result)
    return unexpected{std::move(result).error()};

  result = _verify_excl_group(full_cmdline,
                              switches,
                              excl_group_dev_type(),
                              err_fns);

  if (!result)
    return unexpected{std::move(result).error()};

  return {};
}

auto _all_argv_view(int argc, const char** argv)
{
  using std::span, rng::views::all, rng::views::transform;

  return all(span<const char*>(argv, argc)) | transform(as_string_view);
}

}}

auto cmdline::parse_args(int argc, const char ** argv) -> args_parse_result_t
{
  namespace fs = std::filesystem;

#if _clapi_MISSING_RANGES_CONCAT == 1
using namespace ranges::views;
namespace rng = ::ranges;
#else
using namespace std::views;
namespace rng = std::ranges;
#endif

  using std::string, std::flat_set, std::unexpected;

  // View of all (including cmd argv[0]) as string_view
  // This way we can easily:
  // - compare them
  // - sort them
  auto argv_strings = _all_argv_view(argc, argv);

  // At front argv[0] - parse path and get just filename
  auto app_name = fs::path{argv_strings.front()}.filename();

  // Specified switches dropping argv[0]
  auto args = argv_strings | drop(1);

  // Full cmdline string joining with space app_name and args
  string full_cmdline = [&] {
    auto name_view = single(app_name) | transform(as_string_view);

    auto tmp = concat(std::move(name_view), args) | rng::to<std::vector>();
    return std::ranges::views::all(tmp) | std::ranges::views::join_with(' ')
           | std::ranges::to<string>();
  }();

  // Specified switches as sorted vector for set operations.
  auto switches = args | std::ranges::to<flat_set>();

  auto valid = _verify_arguments(full_cmdline, switches);

  if (not valid) return unexpected{std::move(valid).error()};

  return std::pair{std::move(app_name).string(),
                   std::move(switches)};
}


