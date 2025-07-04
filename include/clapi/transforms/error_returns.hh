#pragma once

#include "clapi/deduced/error_returns.hh"
#include "clapi/transforms/skip_last_param.hh"
#include "clapi/etc/param_optimization.hh"

// TODO: Is it layering volation??
// Q: should the clapi be lowered in heiarchy or abstracted?
#include "clapi/api_error.hh"
#include "clapi/diagnostics.hh"

namespace clapi::_detail::transforms
{

using namespace clapi::deduced;

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
template <auto Fn_, typename... Params_>
  requires deduced::plain_function_pointer<Fn_>
struct _returning_value_api
{
  using api_fn_t = nontype_t<Fn_>;

  template <typename... OtherParams_>
  using rebind_t = _returning_value_api<Fn_, OtherParams_...>;

  using return_type = result_of<Fn_>;
  using tracking_t = sloc_tracking<>;

  static constexpr inline auto API = api_fn_t{};
  static constexpr inline auto API_fn = Fn_;

  template <clapi::LoggingPolicy OnErrorPolicy_>
  [[nodiscard]] inline static auto
    invoke(constant<OnErrorPolicy_>,
           Params_... args,
           tracking_t diag) noexcept -> error_or<return_type>
  {
    using enum clapi::LoggingPolicy;

    auto policy_log_call = [&] {log_API(API, std::move(diag));};

    given_policy<Always>.then(policy_log_call);
    {
			::cl_int out_error;
      if (auto ret = std::invoke_r<return_type>(API_fn,
                                                clapi::fwd_opt<Params_>>(args)...,
                                                &out_error);
          out_error == CLAPI_API_SUCCESS_VALUE) [[likely]]
      {
        return {ret};
      }
      else [[unlikely]]
      {
        // Already logged above
        if constexpr (OnErrorPolicy_ != Always)
          given_policy<OnErrorPolicy_>.then(policy_log_call);

        return clapi::to_error(out_error);
      }
    }

    std::unreachable();
  }

  [[nodiscard]] static auto
    operator() (Params_... args,
                tracking_t diag = clapi::here()) noexcept -> error_or<return_type>
  {
    using enum clapi::LoggingPolicy;

    return _returning_value_api::invoke(constant_<OnError>,
                                        clapi::fwd_opt<Params_>(args)...,
                                        std::move(diag));
  }

  [[nodiscard]] static auto
    operator() (no_log_error_t,
                Params_... args,
                tracking_t diag = clapi::here()) noexcept -> error_or<return_type>
  {
    using enum clapi::LoggingPolicy;

    return _returning_value_api::invoke(constant_<Never>,
                                        clapi::fwd_opt<Params_>(args)...,
                                        std::move(diag));
  }
};

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

template <auto Fn_, typename... Params_>
struct _returning_error_api
{
  using api_fn_t = nontype_t<Fn_>;

  template <typename... OtherParams_>
  using rebind_t = _returning_error_api<Fn_, OtherParams_...>;

  static constexpr inline auto API_fn_v = api_fn_t{};
  static constexpr inline auto API_fn = Fn_;

  using return_type = void;
  using tracking_t = sloc_tracking<>;

  template <clapi::LoggingPolicy OnErrorPolicy_>
  [[nodiscard]] static auto
    invoke(constant<OnErrorPolicy_>,
           Params_... args,
           tracking_t diag = clapi::here()) noexcept -> error_or<return_type>
  {
    using enum clapi::LoggingPolicy;

    auto log_call = [&] {log_API(API_fn_v, std::move(diag));};

    given_policy<Always>.then(log_call);

    if (auto ret_error = std::invoke(API_fn, clapi::fwd_opt<Params_>(args)...);
        ret_error == CLAPI_API_SUCCESS_VALUE) [[likely]]
    {
      return {};
    }
    else [[unlikely]]
    {
      if constexpr (unless_policy<Always>)
        given_policy<OnErrorPolicy_>.then(log_call);

      return clapi::to_error(ret_error);
    }

    std::unreachable();
  }

  [[nodiscard]] static auto
    operator() (Params_... args,
                tracking_t diag = clapi::here()) noexcept -> error_or<return_type>
  {
    using enum clapi::LoggingPolicy;

    return invoke(constant_<OnError>, clapi::fwd_opt<Params_>(args)..., diag);
  }

  [[nodiscard]] static auto
    operator() (no_log_error_t,
                Params_... args,
                tracking_t diag = clapi::here()) noexcept -> error_or<return_type>
  {
    using enum clapi::LoggingPolicy;

    return invoke(constant_<Never>,
                  clapi::fwd_opt<Params_>(args)...,
                  std::move(diag));
  }
};

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

template <auto Fn_,
         typename API_t_ = nontype_t<Fn_>>
consteval auto _select_err_handling()
{
  // Helper returns instance of itstype_t Base_<Fn_> with parameters specified in `tseq<>`
  constexpr auto bind_params = []
    <template <auto, typename...> class Base_, typename ...Params_>
    (Base_<Fn_>, tseq<Params_...>) consteval
  {
    // NB: Please note that parameters are being here conditionally
    // transformed copies by value rather than straightforward std::forward<>
    //
    // See param_opt_t<> and fwd_opt<> for details.
    using base = Base_<Fn_>::template rebind_t<param_opt_t<Params_>...>;

    return itstype<base>;
  };

  constexpr bool isOutErr = is_outerr_clapi_v<API_t_>;

  using params_t = params_of<Fn_>;
  constexpr tseq params = params_t{};

  if constexpr (not isOutErr)
  {
    return bind_params(_returning_error_api<Fn_>{}, params);
  }
  else
  {
    constexpr tseq new_args = _skip_last_param<params_t>{};

    return bind_params(_returning_value_api<Fn_>{}, params);
  }
}

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

template <auto Fn_>
using _checking_base = typename
  std::invoke_result_t<decltype(_select_err_handling<Fn_>)>;

} // namespace clapi::_detail::transforms

namespace clapi::transforms
{

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

template <auto Fn_>
  requires deduced::plain_function_pointer<Fn_> and deduced::core_api<nontype_t<Fn_>>
struct check_fn : _detail::transforms::_checking_base<Fn_>::type
{
  constexpr check_fn() = default;
  constexpr check_fn(nontype_t<Fn_>) noexcept : check_fn() {}
};

template <auto Fn_>
check_fn(nontype_t<Fn_>) -> check_fn<Fn_>;

} // namespace clapi::transforms

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
