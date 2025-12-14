// BGL Modern - C++20 Compiler Version Check
// This file verifies C++20 support at compile time

#ifndef BGL_MODERN_VERSION_HPP
#define BGL_MODERN_VERSION_HPP

// Require C++20
#if !defined(__cplusplus) || __cplusplus < 202002L
#if defined(_MSVC_LANG) && _MSVC_LANG >= 202002L
// MSVC uses _MSVC_LANG for standard version
#else
#error "BGL Modern requires C++20 or later. Compile with -std=c++20 or /std:c++20."
#endif
#endif

// Check for concepts support
#if defined(__cpp_concepts) && __cpp_concepts >= 201907L
#define BGL_MODERN_HAS_CONCEPTS 1
#else
#error "BGL Modern requires C++20 concepts support (__cpp_concepts >= 201907L)."
#endif

// Check for ranges support
// Note: __cpp_lib_ranges may not be defined in all standard library headers
// We check for it but allow compilation to proceed if ranges are available
#if defined(__cpp_lib_ranges)
#define BGL_MODERN_HAS_RANGES 1
#elif __cplusplus >= 202002L
// Assume ranges support in C++20 mode
#define BGL_MODERN_HAS_RANGES 1
#else
#error "BGL Modern requires C++20 ranges support."
#endif

// Compiler identification
#if defined(__GNUC__) && !defined(__clang__)
#define BGL_MODERN_COMPILER_GCC 1
#define BGL_MODERN_COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if BGL_MODERN_COMPILER_VERSION < 100000
#error "BGL Modern requires GCC 10 or later."
#endif

#elif defined(__clang__)
#define BGL_MODERN_COMPILER_CLANG 1
#define BGL_MODERN_COMPILER_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#if BGL_MODERN_COMPILER_VERSION < 130000
#error "BGL Modern requires Clang 13 or later."
#endif

#elif defined(_MSC_VER)
#define BGL_MODERN_COMPILER_MSVC 1
#define BGL_MODERN_COMPILER_VERSION _MSC_VER
#if BGL_MODERN_COMPILER_VERSION < 1929
#error "BGL Modern requires MSVC 2019 16.10 or later."
#endif

#else
#define BGL_MODERN_COMPILER_UNKNOWN 1
#define BGL_MODERN_COMPILER_VERSION 0
#endif

// Library version
#define BGL_MODERN_VERSION_MAJOR 2
#define BGL_MODERN_VERSION_MINOR 0
#define BGL_MODERN_VERSION_PATCH 0
#define BGL_MODERN_VERSION \
    (BGL_MODERN_VERSION_MAJOR * 10000 + BGL_MODERN_VERSION_MINOR * 100 + BGL_MODERN_VERSION_PATCH)

namespace bgl::modern {

/// Library version as a compile-time constant
inline constexpr int version = BGL_MODERN_VERSION;
inline constexpr int version_major = BGL_MODERN_VERSION_MAJOR;
inline constexpr int version_minor = BGL_MODERN_VERSION_MINOR;
inline constexpr int version_patch = BGL_MODERN_VERSION_PATCH;

} // namespace bgl::modern

#endif // BGL_MODERN_VERSION_HPP
