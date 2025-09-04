/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2016-2025 scc

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef BUSTACHE_CONFIG_HPP_INCLUDED
#define BUSTACHE_CONFIG_HPP_INCLUDED

// Require C++20
#if __cplusplus < 202002L
#  error "Bustache requires C++20 or later"
#endif

// Header detection helper
#ifdef __has_include
#  define BUSTACHE_HAS_INCLUDE(x) __has_include(x)
#else
#  define BUSTACHE_HAS_INCLUDE(x) 0
#endif

// Format library selection (std::format or fmt)
#if defined(BUSTACHE_USE_FMT) || !BUSTACHE_HAS_INCLUDE(<format>)
#  define BUSTACHE_USE_FMT_IMPL 1
#  if BUSTACHE_HAS_INCLUDE(<fmt/format.h>)
#    include <fmt/format.h>
namespace bustache {
    namespace format_lib = ::fmt;
    using format_lib::format;
    using format_lib::format_to;
    using format_lib::format_to_n;
    using format_lib::vformat;
}
#  else
#    error "fmt library requested but not found"
#  endif
#else
#  include <format>
namespace bustache {
    namespace format_lib = ::std;
    using std::format;
    using std::format_to;
    using std::format_to_n;
    using std::vformat;
}
#endif


// Compiler-specific optimizations
#ifdef __GNUC__
#  define BUSTACHE_FORCE_INLINE __attribute__((always_inline)) inline
#  define BUSTACHE_NO_INLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#  define BUSTACHE_FORCE_INLINE __forceinline
#  define BUSTACHE_NO_INLINE __declspec(noinline)
#else
#  define BUSTACHE_FORCE_INLINE inline
#  define BUSTACHE_NO_INLINE
#endif

// Export/Import macros for shared library
#ifdef BUSTACHE_EXPORT
#  ifdef _WIN32
#    define BUSTACHE_API __declspec(dllexport)
#  else
#    define BUSTACHE_API __attribute__((visibility("default")))
#  endif
#elif defined(BUSTACHE_SHARED)
#  ifdef _WIN32
#    define BUSTACHE_API __declspec(dllimport)
#  else
#    define BUSTACHE_API
#  endif
#else
#  define BUSTACHE_API
#endif

#endif // BUSTACHE_CONFIG_HPP_INCLUDED