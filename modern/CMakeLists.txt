# BGL Modern - C++20 Graph Library
# Modernized Boost Graph Library with C++20 concepts, ranges, and standard library only
#
# Copyright 2024 Boost Authors
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.20)

project(bgl_modern
    VERSION 2.0.0
    DESCRIPTION "Modernized Boost Graph Library for C++20"
    LANGUAGES CXX
)

# ==============================================================================
# C++20 Requirement
# ==============================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Verify compiler support
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10.0")
        message(FATAL_ERROR "GCC 10+ required for C++20 concepts and ranges. Found: ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.0")
        message(FATAL_ERROR "Clang 13+ required for C++20 concepts and ranges. Found: ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.29")
        message(FATAL_ERROR "MSVC 2019 16.10+ required for C++20 concepts and ranges. Found: ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
else()
    message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}. C++20 support not verified.")
endif()

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")

# ==============================================================================
# Build Options
# ==============================================================================

option(BGL_MODERN_BUILD_TESTS "Build unit tests" ON)
option(BGL_MODERN_BUILD_EXAMPLES "Build example programs" ON)
option(BGL_MODERN_BUILD_BENCHMARKS "Build performance benchmarks" OFF)
option(BGL_MODERN_ENABLE_WARNINGS "Enable strict compiler warnings" ON)

# ==============================================================================
# Compiler Warnings
# ==============================================================================

if(BGL_MODERN_ENABLE_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wshadow
            -Wno-unused-parameter
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(
            /W4
            /permissive-
        )
    endif()
endif()

# ==============================================================================
# Header-Only Library Target
# ==============================================================================

add_library(bgl_modern INTERFACE)
add_library(bgl::modern ALIAS bgl_modern)

target_include_directories(bgl_modern INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(bgl_modern INTERFACE cxx_std_20)

# ==============================================================================
# Tests
# ==============================================================================

if(BGL_MODERN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# ==============================================================================
# Examples
# ==============================================================================

if(BGL_MODERN_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ==============================================================================
# Installation
# ==============================================================================

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS bgl_modern
    EXPORT bgl_modern_targets
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT bgl_modern_targets
    FILE bgl_modern-targets.cmake
    NAMESPACE bgl::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bgl_modern
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/bgl_modern-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/bgl_modern-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bgl_modern
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/bgl_modern-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/bgl_modern-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/bgl_modern-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bgl_modern
)

# ==============================================================================
# Summary
# ==============================================================================

message(STATUS "")
message(STATUS "BGL Modern ${PROJECT_VERSION} Configuration:")
message(STATUS "  Build tests:      ${BGL_MODERN_BUILD_TESTS}")
message(STATUS "  Build examples:   ${BGL_MODERN_BUILD_EXAMPLES}")
message(STATUS "  Build benchmarks: ${BGL_MODERN_BUILD_BENCHMARKS}")
message(STATUS "  Warnings enabled: ${BGL_MODERN_ENABLE_WARNINGS}")
message(STATUS "")
