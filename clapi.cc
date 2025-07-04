#include "clapi/transforms/error_returns.hh"

using clapi::error_or;
using clapi::etc::nontype_t, clapi::etc::nontype;

using clapi::transforms::check_fn;

template <auto Fn_>
  requires clapi::deduced::plain_function_pointer<Fn_>
           and clapi::deduced::core_api<nontype_t<Fn_>>
constexpr inline auto check = check_fn(nontype<Fn_>);

// An example of using the wrapper `clapi::transforms::check` functor: {{{

/*

``` c++
template <auto Fn_>
  requires clapi::deduced::plain_function_pointer<Fn_>
           and clapi::deduced::core_api<nontype_t<Fn_>>
[[nodiscard]]
auto checked(auto &&...args) noexcept
{
  return check<Fn_>(clapi::fwd_opt<decltype(args)>(args)...);
}
```
}}}
*/

#include "cmd_arg_parse.hh"

#include <CL/cl.h>

static_assert(requires {
  check_fn(nontype<::clGetPlatformIDs>);
});

#include <cassert>
#include <concepts>
#include <coroutine>
#include <ranges>
#include <vector>
#include <version>

// SCARY includes for std::generator, views::concat fallbacks for c++23 dialect
// and missing std::generator<> in standard library

// We're mixing std::ranges/std::ranges::views with ::ranges_v3 library.
// XXX: For some reason ::ranges_v3 doesn't belive that libstdc++ ranges meet
// some of concepts of ranges_v3.
//
// This is odd, thus should not be defined in library dependend manner and should
// freely mix with eachother.
// Those don't, thus we do little flaky dance over which one we use.
//
// It most likely boils down to some non-conforming differences in concept definitions
// while also doesn't ::ranges_v3 doesn't check for existance of
// `std::ranges::range_adaptor_closure` going its own way ignoring existance of
// standard library.
//
// Also ranges_v3 generator never learned about exitance of elements_of thus - which
// surely warrants it's implementation having status of an experimental.
//
// Still better than nothing, one could say.
//
// TODO: Open-source lx6::rgenerator implementation???
// It's bit coupled with rest of lx6, but shouldn't be too hard to decouple or also
// provide the rest of small building blocks.
// Differences seem to boil down to use of some helper traits/aliases to to traits.

// TODO similar incantation over <ranges> lives in command line parsing implementation.
#if !defined(__cpp_coroutines) && defined(__cpp_lib_coroutine)
// required for to pass (RANGES_CXX_COROUTINES > RANGES_CXX_COROUTINES_TS1)
// XXX: no idea why ::ranges_v3 lib depends on that
#define __cpp_coroutines __cpp_lib_coroutine
#endif
#if _clapi_MISSING_RANGES_CONCAT == 1
#include <range/v3/view/concat.hpp>

// XXX: It _IS_ MISSING_ std::ranges::elements_of equivalent
#include <range/v3/experimental/utility/generator.hpp>

using ranges::views::concat;
template <typename Ty_>
using generator = ranges::experimental::generator<Ty_>;
#else
#include <generator>
template <typename Ty_>
using generator = std::generator<Ty_>;
using std::ranges::elements_of;
#endif

namespace rng = std::ranges;

auto enum_platforms() -> generator<cl_platform_id>
{
  using std::vector;

  constexpr auto getPlatformIDs = check<::clGetPlatformIDs>;

  ::cl_uint nplatforms;
  if (error_or<> result = getPlatformIDs(0, nullptr, &nplatforms);
      result.has_value()) [[likely]]
  {
    if (nplatforms == 0) [[unlikely]] co_return;

    vector<cl_platform_id> platforms{nplatforms};
    result = getPlatformIDs(platforms.size(), platforms.data(), nullptr);

    if (!result) [[unlikely]] throw result.error();

    auto ids = rng::views::as_rvalue(std::move(platforms));

#if _clapi_MISSING_RANGES_CONCAT
    // XXX: ranges_v3 experimental generator
    for (auto id : std::move(ids)) { co_yield id; }
#else
    co_yield elements_of(std::move(ids));
#endif
  }
  else
  {
    throw result.error();
  }
}

using full_device_id_t = std::tuple<::cl_platform_id,
                                    ::cl_device_id,
                                    ::cl_device_type>;

// Since thare are multiple templated overloads we make it callable object.
// This allows us to bind such object, what otherwise wouldn't be possible.
//
// Otherwise one would need to to cast it or wrap it in lambda function.
struct enum_platform_devices_fn
{
  static const enum_platform_devices_fn self;

  auto static operator() (::cl_platform_id pid,
                          ::cl_device_type device_type = CL_DEVICE_TYPE_ALL) ->
    generator<full_device_id_t>
  {
    using namespace std::views;
    using std::vector;
    using clapi::ExpectedFailure;

    cl_uint num_dev;

    static constexpr auto getDeviceIDs = check<::clGetDeviceIDs>;
    static constexpr auto getDeviceInfo = check<::clGetDeviceInfo>;

    if (error_or<> ret = getDeviceIDs(ExpectedFailure,
                                      pid, device_type, 0, nullptr, &num_dev);
        not ret)
    {
      using namespace clapi::enable_errcode_int_compare;
      // Will return CL_DEVICE_NOT_FOUND rather than reporting num_dev as 0,
      // whenever no devices of specified type are present.
      //
      // That's not really an error, stop enumeration, yielding an empty range.
      if (ret.error() == CL_DEVICE_NOT_FOUND) co_return;

      throw ret.error();
    }
    else
    {
      if (num_dev == 0) co_return;

      vector<cl_device_id> devs{num_dev};
      // Fill devices
      if (auto ret = getDeviceIDs(pid, device_type, devs.size(), devs.data(), nullptr);
          !ret)
        throw ret.error();

      // We're asked CL_DEVICE_TYPE_ALL, thus we don't know device type,
      // to construct full_device_id_t, and we need to query it.
      if (device_type == CL_DEVICE_TYPE_ALL)
      {
        auto dev_types = devs | transform([](auto id) {
          cl_device_type t;

          if (auto ret = getDeviceInfo(id, CL_DEVICE_TYPE, sizeof(t), &t, nullptr); !ret)
            throw ret.error();

          return t;
        });

        auto pdevs = zip(repeat(pid), devs, dev_types);

#if _clapi_MISSING_RANGES_CONCAT
        for (auto pd : pdevs) { co_yield pd; }
#else
        co_yield elements_of(pdevs);
#endif
      }
      else
      {
        assert(device_type != CL_DEVICE_TYPE_ALL);
        // There's no need to query CL_DEVICE_TYPE
        auto pdevs = zip(repeat(pid), devs, repeat(device_type));

#if _clapi_MISSING_RANGES_CONCAT
      // XXX: ranges_v3 experimental generator
        for (auto pd : pdevs) { co_yield pd; }
#else
        co_yield elements_of(pdevs);
#endif
      }
    }
  }

  static auto operator() (rng::viewable_range auto platforms,
                           cl_device_type devt = CL_DEVICE_TYPE_ALL) ->
    generator<full_device_id_t>
  {
    for (auto p : platforms)
    {
#if _clapi_MISSING_RANGES_CONCAT
      // XXX: ranges_v3 experimental generator
      for (auto pd : self(p, devt)) { co_yield pd; }
#else
      co_yield elements_of(self(p, devt));
#endif
    }

    co_return;
  }

  auto static operator() (cl_device_type devt = CL_DEVICE_TYPE_ALL) ->
    generator<full_device_id_t>
  {
    auto devices = self(enum_platforms(), devt);

#if _clapi_MISSING_RANGES_CONCAT
    // XXX: ranges_v3 experimental generator
    for (auto pd : std::move(devices)) co_yield pd;
#else
    co_yield elements_of(std::move(devices));
#endif
  }
};

constexpr enum_platform_devices_fn enum_platform_devices{};

namespace query_prop
{

template <auto Fn_,
         typename IdTy_,
         typename PropTy_>
concept clapi_query_function =
  std::is_invocable_r_v<::cl_int,
                        decltype(Fn_),
                        IdTy_,
                        PropTy_,
                        ::size_t,
                        void*,
                        ::size_t*>;


template <auto Fn_, typename PropTy_>
auto query_string_property_(auto id, PropTy_ prop) -> std::string
  requires clapi_query_function<Fn_, decltype(id), PropTy_>
{
   using std::string;

   ::size_t req_capacity;

   constexpr check_fn<Fn_> query;
   error_or<> r = query(id, prop, 0, nullptr, &req_capacity);

   if (!r) throw r.error();

   // XXX: C++ requires null-terminated data since c++-11
   // Code around won't work for anything that out-dated.
   // But for sake of backporting...
   static_assert(__cplusplus >= 201103L,
                 "c++11 and later requires std::basic_string<char>::data()"
                 " to have space for '\\0';\n"
                 " This implementation takes it into account");

   string ret(req_capacity-1, '\0'); // Note: -1, please see above for explaination.

   r = query(id, prop, ret.size()+1, ret.data(), nullptr);
   if (!r) [[unlikely]] throw r.error();

   return ret;
}

template <auto Fn_, typename PropTy_>
auto query_string_property_(nontype_t<Fn_>, auto id, PropTy_ prop) -> std::string
  requires clapi_query_function<Fn_, decltype(id), PropTy_>
{
  return query_string_property_<Fn_>(id, prop);
}

template <auto Fn_,
         std::integral Ty_,
         typename ObjTy_,
         typename PropTy_>
  requires clapi_query_function<Fn_, ObjTy_, PropTy_>
auto query_integral_property_(ObjTy_ id, PropTy_ prop) -> Ty_
{
  Ty_ value;

  auto r = check<Fn_>(id, prop, sizeof(value), &value, nullptr);

  if (!r) [[unlikely]] throw r.error();

  return value;
}

template <auto Fn_, std::integral Ty_, typename ObjTy_, typename PropTy_>
  requires clapi_query_function<Fn_, ObjTy_, PropTy_>
auto query_integral_property_(nontype_t<Fn_>, ObjTy_ id, PropTy_ prop) -> Ty_
{
  [[clapi_inline_stmt]]
  return query_integral_property_<Fn_, Ty_>(id, prop);
}

template <auto Fn_, typename ObjTy_, typename PropTy_>
  requires clapi_query_function<Fn_, ObjTy_, ::cl_bool>
[[nodiscard]]
auto query_bool_property_(ObjTy_ o, PropTy_ p) -> bool
{
  [[clapi_inline_stmt]]
  return query_integral_property_<Fn_, cl_bool>(o, p);
}

template <auto Fn_, typename ObjTy_, typename PropTy_>
  requires clapi_query_function<Fn_, ObjTy_, ::cl_bool>
[[nodiscard]]
auto query_bool_property_(nontype_t<Fn_>, ObjTy_ o, PropTy_ p) -> bool
{
  [[clapi_inline_stmt]]
  return query_integral_property_<Fn_, cl_bool>(o, p);
}

template <auto Fn_, typename ObjTy_, typename PropTy_>
  requires clapi_query_function<Fn_, ObjTy_, ::cl_bool>
[[nodiscard]]
auto query_bool_property(nontype_t<Fn_>, ObjTy_ o, PropTy_ p) -> bool
{
  [[clapi_inline_stmt]]
  return query_bool_property_<Fn_>(o, p);
}

} // namespace query_prop

#include <any>

namespace query_prop_any
{

namespace _qp = query_prop;

template <auto Fn_, typename PropTy_>
[[maybe_unused, nodiscard]]
auto query_string_property(auto id, PropTy_ prop) -> std::any
  requires _qp::clapi_query_function<Fn_, decltype(id), PropTy_>
{
  using std::string, std::make_any;
  return make_any<string>(_qp::query_string_property_<Fn_>(id, prop));
}

template <auto Fn_, typename PropTy_>
[[maybe_unused, nodiscard]]
auto query_string_property(nontype_t<Fn_>, auto id, PropTy_ prop) -> std::any
  requires _qp::clapi_query_function<Fn_, decltype(id), PropTy_>
{
  return query_string_property<Fn_>(id, prop);
}

template <auto Fn_, std::integral Ty_, typename ObjTy_, typename PropTy_>
  requires _qp::clapi_query_function<Fn_, ObjTy_, PropTy_>
[[maybe_unused, nodiscard]]
auto query_integral_property(ObjTy_ id, PropTy_ prop) -> std::any
{
  using std::make_any;

  [[clapi_inline_stmt]]
  return make_any<Ty_>(_qp::query_integral_property_<Fn_, Ty_>(id, prop));
}

template <auto Fn_, typename ObjTy_, typename PropTy_>
  requires _qp::clapi_query_function<Fn_, ObjTy_, ::cl_bool>
[[maybe_unused, nodiscard]]
auto query_bool_property(ObjTy_ o, PropTy_ p) -> bool
{
  using std::make_any;

  [[clapi_inline_stmt]]
  return make_any(_qp::query_bool_property_<Fn_>(o, p));
}

} // namespace query_prop_any

using namespace query_prop;

[[maybe_unused]]
static constexpr auto string_any = []
  [[nodiscard]] [[using gnu: always_inline, flatten]]
  (std::any &&a)
  static { return std::any_cast<std::string>(a); };

#include <print>

using namespace std::literals::string_view_literals;

static constexpr auto DevInfo = nontype<clGetDeviceInfo>;
static constexpr auto PlatInfo = nontype<clGetPlatformInfo>;

constexpr static auto check_full_30_profile =
  [] (const full_device_id_t &dev) static -> bool
{
  const auto [p, d, _] = dev;

  auto pprofile = query_string_property_(PlatInfo, p, CL_PLATFORM_PROFILE);
  auto dprofile = query_string_property_(DevInfo, d, CL_DEVICE_PROFILE);

  constexpr auto FULL = "FULL_PROFILE"sv;

  bool ok = std::move(pprofile) == FULL;
  ok &= std::move(dprofile) == FULL;

  cl_version pversion, dversion;

  constexpr auto query_version = [] <auto F_>
    [[nodiscard]] [[using gnu: always_inline, flatten]]
    (nontype_t<F_>, auto... args) static noexcept
  {
    return query_integral_property_<F_, ::cl_version>(/*FIXME: ExpectedFailure*/args...);
  };

  // NB: Was an extension before 3.0 (cl_khr_extended_versioning) promoted to 3.0
  try {
    // FIXME: probably want no-logging version of utility.
    // If we're don't have cl_khr_extended_versioning those will fail and throw
    // Such error is ok, one can deduce from it platform doesn't support OpenCL 3.0
    //
    // One could avoid that by checking for cl_khr_extended_versioning first.
    // It is much simpler to try to use it and possibly fail, since we must do it anyway.
    pversion = query_version(PlatInfo, /*TODO: ExpectedFailure,*/
                             p, CL_PLATFORM_NUMERIC_VERSION);
    dversion = query_version(DevInfo, /*TODO: ExpectedFailure,*/
                             d, CL_DEVICE_NUMERIC_VERSION);
  }
  catch (clapi::error_code_t) {
    return false;
  }

  ok &= (dversion >= CL_MAKE_VERSION(3, 0, 0));
  ok &= (pversion >= CL_MAKE_VERSION(3, 0, 0));

  return ok;
};

static auto has_cmdline_switch(const cmdline::args_set_t &args_set,
                               std::string_view sw)
{
    assert(rng::contains(cmdline::allowed_cmd_switches(), sw));
    return args_set.contains(sw);
};

int main(int argc,
         const char ** argv,
        [[maybe_unused]] const char ** env) try
{
  using namespace std::views;
  using std::vector, std::string;

  auto args_parse_result = cmdline::parse_args(argc, argv);

  if (not args_parse_result.has_value())
  {
    std::println(stderr, "{}", args_parse_result.error());
    return 1;
  }

  auto [_, args_set] = std::move(args_parse_result).value();

  auto platforms = enum_platforms() | rng::to<vector>();

  if (platforms.empty())
  {
    std::println("No OpenCL platforms found"sv);
    return 0;
  }

  vector<full_device_id_t> selected;

  // Returns true if one have parsed particular switch
  auto has_switch = std::bind_front(has_cmdline_switch, std::cref(args_set));

  auto select_devices = [&selected, &has_switch]
    (rng::viewable_range auto &&discovered_dev)
  {
    // true iff device's property CL_DEVICE_AVAILABLE is true
    auto if_avail = [](auto full_dev) static -> bool {
      return query_bool_property_(DevInfo,
                                  std::get<cl_device_id>(full_dev),
                                  CL_DEVICE_AVAILABLE);
    };

    auto remembered = discovered_dev | rng::to<vector>();
    auto available = remembered | filter(if_avail) | rng::to<vector>();

    // By the default select devices supporting full OpenCL 3.0 profile.
    if (not has_switch("--want-legacy"))
    {
      available = all(std::move(available))
                  | filter(check_full_30_profile)
                  | rng::to<vector>();
    }

    // Print all discorvered devices.
    std::println("The OpenCL discovered devices (per-platform) are:");
    for (auto dev : remembered)
    {
      auto [p, d, t] = dev;

      auto pname = query_string_property_(PlatInfo, p, CL_PLATFORM_NAME);
      auto dname = query_string_property_(DevInfo, d, CL_DEVICE_NAME);

      auto type = t == CL_DEVICE_TYPE_GPU? "GPU"sv : "CPU"sv;

      std::println("Platform: {}", std::move(pname));
      std::println("  {} device : {}", type, std::move(dname));
      std::println("      available: {}\n", if_avail(dev));
    }

    if (has_switch("--just-first"sv))
      available = all(available) | take(1) | rng::to<vector>();

    std::println("Selecting {} device(s)", size(available));
    selected = std::move(available);
  };

  auto plat_ref = std::cref(platforms);

  auto gpu_devs = std::bind(enum_platform_devices, plat_ref, CL_DEVICE_TYPE_GPU);
  auto cpu_devs = std::bind(enum_platform_devices, plat_ref, CL_DEVICE_TYPE_CPU);
  auto all_devs = std::bind(enum_platform_devices, plat_ref, CL_DEVICE_TYPE_ALL);

  // Mutualy-exclusive switches `cmdline::excl_group_dev_type`
  //   See: `cmd_arg_parse.hh`
  if (has_switch("--all-types"sv)) select_devices(all_devs());
  else if (has_switch("--cpu-only"sv)) select_devices(cpu_devs());
  else if (has_switch("--gpu-only"sv)) select_devices(gpu_devs());

  // We do nothing with selection, but we did select them...
  // Those are to are likeing.
  // We really have them!
  // We're happy now and feeling acomplished.
  // Aren't we?
}
catch (clapi::error_code_t e)
{
   std::println(stderr, "OCL Error: {}", int(e));
}

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
