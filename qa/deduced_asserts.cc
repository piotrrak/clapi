#include "clapi/deduced/error_returns.hh"

namespace tst_deduced_traits_sanity
{

namespace deduced = clapi::deduced;

using clapi::etc::nontype_t, clapi::etc::nontype;

cl_int dummy_cl();
cl_int dummy_cl_nx() noexcept;

void dummy_cl_oret(cl_int*);
void dummy_cl_oret_nx(cl_int*) noexcept;

void dummy_cl_not_oret_nx(cl_uint*) noexcept;

using deduced::can_deduce_with;

static_assert(can_deduce_with<deduced::result_of, dummy_cl>);
static_assert(can_deduce_with<deduced::result_of, dummy_cl_nx>);

static_assert(deduced::is_reterr_clapi_v<nontype_t<dummy_cl>>);
static_assert(deduced::is_reterr_clapi_v<nontype_t<dummy_cl_nx>>);

static_assert(deduced::is_outerr_clapi_v<nontype_t<dummy_cl_oret>>);
static_assert(deduced::is_outerr_clapi_v<nontype_t<dummy_cl_oret_nx>>);

static_assert(not deduced::is_outerr_clapi_v<nontype_t<dummy_cl_not_oret_nx>>);
static_assert(not deduced::is_outerr_clapi_v<nontype_t<dummy_cl>>);
static_assert(not deduced::is_outerr_clapi_v<nontype_t<dummy_cl_nx>>);

}

