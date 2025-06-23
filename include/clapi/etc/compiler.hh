#pragma once

#if defined(__clang__)
#define _clapi_PragmaClang(p) _Pragma(p)
#else
#define _clapi_PragmaClang(p)
#endif

#if !defined(__clang__) && defined(__GNUG__)
#define _clapi_PragmaGCC(p) _Pragma(p)
#else
#define _clapi_PragmaGCC(p)
#endif

#if __has_cpp_attribute(clang::always_inline)
# define clapi_inline_stmt clang::always_inline
#else
# define clapi_inline_stmt likely
#endif

// Compiler can do that, so use it, things like pack indexing
// are much faster to compile when not emulated.
#define _clapi_BEGIN_ALLOW_CPP26() \
  _clapi_PragmaClang("clang diagnostic push") \
  _clapi_PragmaClang("clang diagnostic ignored \"-Wc++26-extensions\"") \
    _clapi_PragmaGCC("GCC diagnostic push")\
    _clapi_PragmaGCC("GCC diagnostic ignored \"-Wc++26-extensions\"")

#define _clapi_END_ALLOW_CPP26() \
  _clapi_PragmaClang("clang diagnostic pop") \
    _clapi_PragmaGCC("GCC diagnostic pop")

/* Best read in VIM {{{
 * vim: noai : et : fdm=marker :
 * }}} */
