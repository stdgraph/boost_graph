# Build and Test

Back to [overview.md](../overview.md)

---

## 1. Prerequisites

| Requirement | Detail |
|-------------|--------|
| C++ standard | C++14 or later |
| Build system | Boost.Build (b2) **or** CMake — see §3 |
| Superproject | A full `boostorg/boost` superproject checkout is required for building and testing. Standalone builds of `libs/graph` alone are **not supported**. |
| Dependencies | All declared in [CMakeLists.txt](../../../CMakeLists.txt) (`target_link_libraries`); the same set applies under Boost.Build. See §2 for the dependency list. |

---

## 2. Build-file overview

The following files in this repository define how BGL is built. They are
not standalone entry points — they are consumed by the superproject
orchestration.

| File | Role |
|------|------|
| [`CMakeLists.txt`](../../../CMakeLists.txt) | CMake library target (`boost_graph`); declares all `Boost::*` dependencies and the two compiled sources |
| [`build.jam`](../../../build.jam) | Top-level Boost.Build project hint consumed by the superproject |
| [`build/Jamfile.v2`](../../../build/Jamfile.v2) | Boost.Build library target; compiles `src/read_graphviz_new.cpp` and `src/graphml.cpp` |

### Compiled components

Only two files produce object code; everything else is header-only.

| Source file | Purpose | Requires |
|-------------|---------|---------|
| [`src/read_graphviz_new.cpp`](../../../src/read_graphviz_new.cpp) | GraphViz `.dot` file parser (Spirit-based) | `Boost::spirit`, `Boost::regex` |
| [`src/graphml.cpp`](../../../src/graphml.cpp) | GraphML XML file parser | `Boost::property_tree`, `Boost::regex` |

### Boost dependencies (from CMakeLists.txt)

Public: `algorithm`, `any`, `array`, `assert`, `bimap`, `concept_check`,
`config`, `container_hash`, `conversion`, `core`, `detail`, `foreach`,
`function`, `integer`, `iterator`, `lexical_cast`, `math`, `move`, `mpl`,
`multi_index`, `multiprecision`, `optional`, `parameter`, `preprocessor`,
`property_map`, `property_tree`, `random`, `range`, `serialization`,
`smart_ptr`, `spirit`, `throw_exception`, `tti`, `tuple`, `type_traits`,
`typeof`, `unordered`, `utility`, `xpressive`.  
Private: `regex`.

---

## 3. Building with a Boost superproject

### 3a. Boost.Build (recommended)

Clone the superproject and all libraries, then:

```sh
# From the top-level boostorg/boost checkout:
./bootstrap.sh          # or bootstrap.bat on Windows
./b2 headers            # stage all headers; required before anything else
./b2 libs/graph/build   # build boost_graph (the compiled library)
```

Run the test suite:

```sh
./b2 libs/graph/test    # runs all graph tests via test/Jamfile.v2
```

Run a subset matching a pattern:

```sh
./b2 libs/graph/test //graph_test_regular
```

The test Jamfile entry-point is [`test/Jamfile.v2`](../../../test/Jamfile.v2).
`test/CMakeLists.txt` is **not present** in this checkout; testing via
CMake requires it to be generated separately.

### 3b. CMake (superproject-driven)

From the superproject root:

```sh
cmake -S . -B build -DBOOST_INCLUDE_LIBRARIES=graph -DCMAKE_CXX_STANDARD=14
cmake --build build
```

To enable tests, add `-DBUILD_TESTING=ON`. BGL's `CMakeLists.txt` will
conditionally add the test subdirectory only if `test/CMakeLists.txt`
exists at configure time.

---

## 4. Examples

Example sources live in [`example/`](../../../example/). They are not built
automatically; build them as part of the test suite or manually:

```sh
# Boost.Build (from superproject root):
./b2 libs/graph/example

# Or compile a single example with your own toolchain (assuming headers staged):
c++ -std=c++14 -I<boost-root> libs/graph/example/dijkstra-example.cpp \
    -o dijkstra-example
```

Most examples are header-only and need no library link. Examples that
parse GraphViz or GraphML files (`read_graphviz`-based) additionally
require linking against `boost_graph`.

---

## 5. Building the documentation

HTML documentation is pre-generated in [`doc/`](../../../doc/). To
regenerate it locally:

```sh
cd doc
bash BUILD_DOCS.sh
```

The script requires a local Quickbook/BoostBook/xsltproc toolchain; see
the Boost documentation build system for setup details.

---

## 6. Where to go next

| Task | Read |
|------|------|
| Adding or modifying code | [extending.md](extending.md) |
| Understanding which files exist in this repo | [organization.md](organization.md) |
| Full Boost superproject workflow | [Boost.Build docs](https://www.boost.org/doc/tools/build/) |
