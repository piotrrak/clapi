#pragma once

#include "clapi/etc/seq.hh"

#include <algorithm>
#include <array>
#include <expected>
#include <flat_set>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>


namespace cmdline
{

template <typename Ty_= void>
using expected_or_str = std::expected<Ty_, std::string>;

using args_set_t = std::flat_set<std::string_view>;
using args_parse_result_t =
  expected_or_str<std::pair<std::string, args_set_t>>;

auto parse_args(int argc, const char ** argv) -> args_parse_result_t;

using namespace std::literals::string_view_literals;

consteval auto allowed_cmd_switches() {
  static constexpr std::array switches = {
    "--cpu-only"sv,
    "--gpu-only"sv,
    "--all-types"sv,
    "--want-legacy"sv,
    "--just-first"sv,
//   "--help"sv,
  };
  return auto(switches);
}

template <typename Ty_>
  requires requires { {std::tuple_size_v<Ty_>}; }
constexpr inline auto indices_of = []
consteval {
  constexpr auto Sz = std::tuple_size_v<Ty_>;
  return clapi::iseq_for_n<Sz>{};
};

consteval auto excl_group_dev_type() {
  static constexpr std::array group = {
    "--all-types"sv,
    "--gpu-only"sv,
    "--cpu-only"sv,
  };

  [] <auto... Idx_> (clapi::iseq<Idx_...>&& _)
  consteval -> void {
    static constexpr auto switches = allowed_cmd_switches();

    auto has_switch = [] (auto sw)
      consteval { return std::ranges::contains(switches, sw); };

    static_assert((has_switch(group[Idx_]) && ... && true),
                  "group switch not in allowed");
  }(indices_of<decltype(group)>());

  return auto(group);
}

} // namespace cmdline
