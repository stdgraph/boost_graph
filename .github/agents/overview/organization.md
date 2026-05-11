# Repository Organization

Back to [overview.md](../overview.md)

This document describes the layout of the `boost/libs/graph` repository and
the role of each directory and top-level file.

---

## Top-level layout

| Path | Role |
|------|------|
| [`include/boost/graph/`](../../../include/boost/graph/) | Public headers — canonical source of truth for the API. ~150 top-level headers plus `detail/`, `planar_detail/`, and `property_maps/` subdirectories. |
| [`src/`](../../../src/) | Compiled (non-header-only) components. Contains exactly two source files; see [Compiled components](#compiled-components) below. |
| [`doc/`](../../../doc/) | Long-form HTML documentation for concepts, algorithms, and graph types. Some pages predate recent header changes; on any discrepancy, the header wins. |
| [`example/`](../../../example/) | ~100 illustrative `.cpp` programs demonstrating idiomatic BGL usage. Built as part of the Boost.Build and CMake test suites. |
| [`test/`](../../../test/) | 300+ regression tests. Both `Jamfile.v2` (Boost.Build) and `CMakeLists.txt` entry points are present. |
| [`build/`](../../../build/) | `Jamfile.v2` for building the compiled components with Boost.Build. |
| [`meta/`](../../../meta/) | `libraries.json` — Boost library metadata (authors, category, C++ standard). |
| [`.github/`](../../../.github/) | GitHub Actions CI workflows and agent overview documents (this directory). |
| [`CMakeLists.txt`](../../../CMakeLists.txt) | CMake entry point for the library and tests. |
| [`build.jam`](../../../build.jam) | Boost.Build top-level entry point. |
| [`README.md`](../../../README.md) | Project description, build/test quick-start, CI badges. |
| [`index.html`](../../../index.html) | Alternate HTML entry point (mirrors README narrative). |

---

## Include directory structure

```
include/boost/graph/
├── *.hpp                  # ~140 public algorithm, graph-type, and utility headers
├── detail/                # Internal implementation headers — not public API
├── planar_detail/         # Planarity algorithm internals — not public API
└── property_maps/         # Property map adaptors (a small set of helpers)
```

Headers under `detail/` and `planar_detail/` are not part of the public API
and should not be cited in user-facing documentation.

---

## Compiled components

The library is almost entirely header-only. The two exceptions live in
[`src/`](../../../src/):

| Source file | Compiled artifact | Purpose |
|-------------|------------------|---------|
| `src/read_graphviz_new.cpp` | part of `libboost_graph` | DOT (GraphViz) format reader |
| `src/graphml.cpp` | part of `libboost_graph` | GraphML format reader/writer |

Both files are compiled into a single library target (`boost_graph` / `Boost::graph`).
All other BGL functionality is header-only and requires no linking beyond
header-only Boost dependencies.

The compiled components link against a number of Boost libraries. The full list
is declared in [`CMakeLists.txt`](../../../CMakeLists.txt) under
`target_link_libraries`. Notable dependencies: `Boost::property_tree` (GraphML),
`Boost::spirit` and `Boost::regex` (DOT reader), `Boost::serialization`
(adjacency list serialization).

---

## Build system entry points

BGL supports two build systems in parallel.

### Boost.Build

| File | Purpose |
|------|---------|
| [`build.jam`](../../../build.jam) | Top-level entry point; delegates to `build/Jamfile.v2` |
| [`build/Jamfile.v2`](../../../build/Jamfile.v2) | Defines the `boost_graph` library target and its compiled sources |
| [`test/Jamfile.v2`](../../../test/Jamfile.v2) | Defines all 300+ test targets |

Boost.Build requires a full `boostorg/boost` superproject checkout. See
[build-and-test.md](build-and-test.md) for invocation details.

### CMake

| File | Purpose |
|------|---------|
| [`CMakeLists.txt`](../../../CMakeLists.txt) | Defines the `boost_graph` / `Boost::graph` target, links dependencies, and conditionally includes `test/` |
| `test/CMakeLists.txt` | Test targets for CTest (not present in this checkout) |

CMake is designed to run as part of the Boost superproject (`boostorg/boost`).
Standalone CMake builds are not currently supported. See
[build-and-test.md](build-and-test.md) for both repo-local and superproject
workflows.

---

## Agent navigation guide

| Task | Go to |
|------|-------|
| Find the header for algorithm X | [`include/boost/graph/`](../../../include/boost/graph/) |
| Read algorithm/concept documentation | [`doc/`](../../../doc/) |
| See a working usage example | [`example/`](../../../example/) |
| Check whether a component is tested | [`test/`](../../../test/) |
| Understand build dependencies | [`CMakeLists.txt`](../../../CMakeLists.txt) |
| Add a new test | [`test/Jamfile.v2`](../../../test/Jamfile.v2) (CMake test file not present in this checkout) |
