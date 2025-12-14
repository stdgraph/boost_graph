# BGL Modern - Modernized Boost Graph Library for C++20

A modernized version of the Boost Graph Library using C++20 concepts, ranges, and standard library only.

## Requirements

- **C++20** compiler:
  - GCC 10+ 
  - Clang 13+
  - MSVC 2019 16.10+ (Visual Studio 2019)
  - AppleClang 13+ (Xcode 13+)

- **CMake 3.20+**

## Quick Start

```bash
# Clone and navigate to the modern graph library
cd libs/graph/modern

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --parallel

# Run tests
ctest --test-dir build
```

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BGL_MODERN_BUILD_TESTS` | `ON` | Build unit tests |
| `BGL_MODERN_BUILD_EXAMPLES` | `ON` | Build example programs |
| `BGL_MODERN_BUILD_BENCHMARKS` | `OFF` | Build performance benchmarks |
| `BGL_MODERN_ENABLE_WARNINGS` | `ON` | Enable strict compiler warnings |

## Features

### C++20 Concepts

Graph concepts provide clear compile-time constraints:

```cpp
#include <bgl/modern/concepts.hpp>

template<bgl::IncidenceGraph G>
void my_algorithm(const G& g) {
    for (auto v : bgl::vertices(g)) {
        for (auto e : bgl::out_edges(v, g)) {
            // ...
        }
    }
}
```

### Range-Based API

All graph accessors return ranges:

```cpp
#include <bgl/modern/graph.hpp>

for (auto v : bgl::vertices(g)) {
    // Process vertex
}

for (auto e : bgl::out_edges(v, g)) {
    // Process outgoing edge
}

// Works with std::ranges
auto filtered = bgl::vertices(g) 
    | std::views::filter([&](auto v) { return g[v].weight > 0; });
```

### Standard Library Only

No Boost dependencies. Uses only:
- `<concepts>`
- `<ranges>`
- `<type_traits>`
- `<functional>`
- Standard containers

## Project Structure

```
modern/
├── CMakeLists.txt          # Main build configuration
├── cmake/                  # CMake package config
├── include/
│   └── bgl/
│       └── modern/
│           ├── concepts.hpp      # C++20 graph concepts
│           ├── graph_traits.hpp  # Type traits and aliases
│           ├── adjacency_list.hpp
│           └── ...
├── test/                   # Unit tests
├── examples/               # Example programs
└── benchmarks/             # Performance benchmarks
```

## License

Distributed under the Boost Software License, Version 1.0.
See [LICENSE_1_0.txt](../../../LICENSE_1_0.txt).
