#include "clapi/etc/basic.hh"
#include "clapi/deduced/function_pointer.hh"

namespace test_funptr_concepts_sanity
{

static void foo() {}

using clapi::nontype_t, clapi::nontype;

using clapi::deduced::function_pointer, clapi::deduced::function_pointer_type;

static_assert(function_pointer<foo>);
static_assert(function_pointer<&foo>); // Same as above - different syntax

static_assert(function_pointer<nontype<foo>>);
static_assert(function_pointer<nontype<&foo>>); // Same as above - different syntax

static_assert(function_pointer_type<nontype_t<foo>>);
static_assert(function_pointer_type<auto (*) (void) -> void>);

static_assert(not function_pointer_type<auto (void) -> void>);
static_assert(not function_pointer_type<auto (&) (void) -> void>);

using clapi::deduced::plain_function_pointer, clapi::deduced::plain_function_pointer_type;

static_assert(plain_function_pointer<foo>);
static_assert(plain_function_pointer<&foo>); // Same as above - different syntax

static_assert(not plain_function_pointer<nontype<foo>>);
static_assert(not plain_function_pointer<nontype<&foo>>); // Same as above - different syntax

static_assert(not plain_function_pointer_type<nontype_t<foo>>);
static_assert(plain_function_pointer_type<auto (*) (void) -> void>);

static_assert(not plain_function_pointer_type<auto (void) -> void>);
static_assert(not plain_function_pointer_type<auto (&) (void) -> void>);

using clapi::deduced::nontype_function_pointer, clapi::deduced::nontype_function_pointer_type;

static_assert(not nontype_function_pointer<foo>);
static_assert(not nontype_function_pointer<&foo>); // Same as above - different syntax

static_assert(nontype_function_pointer<nontype<foo>>);
static_assert(nontype_function_pointer<nontype<&foo>>); // Same as above - different syntax

static_assert(nontype_function_pointer_type<nontype_t<foo>>);
static_assert(not nontype_function_pointer_type<auto (*) (void) -> void>);

static_assert(not nontype_function_pointer_type<auto (void) -> void>);
static_assert(not nontype_function_pointer_type<auto (&) (void) -> void>);

}
