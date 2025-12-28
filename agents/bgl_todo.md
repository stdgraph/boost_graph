# BGL Modernization Implementation Plan

This document outlines the phased implementation plan for modernizing the Boost Graph Library to C++20, based on the analysis in [bgl_overview.md](bgl_overview.md).

---

## Phase 1: Foundation — C++20 Core Infrastructure

**Goal:** Establish C++20 build infrastructure and define core concepts/traits.

### 1.1 Build System & Compiler Setup
- [x] Configure CMakeLists.txt for C++20 (`-std=c++20`)
- [x] Verify compilation on GCC 10+, Clang 13+, MSVC 2019+
- [x] Remove Boost.Build dependency; use standard CMake
- [x] Set up CI pipeline for multi-compiler testing

### 1.2 Core Concepts (replace Boost.ConceptCheck)
- [x] Define `Graph` concept (`vertex_descriptor`, `edge_descriptor`, `directed_category`)
- [x] Define `IncidenceGraph` concept (`out_edges`, `out_degree`, `source`, `target`)
- [x] Define `BidirectionalGraph` concept (`in_edges`, `in_degree`)
- [x] Define `VertexListGraph` concept (`vertices`, `num_vertices`)
- [x] Define `EdgeListGraph` concept (`edges`, `num_edges`)
- [x] Define `AdjacencyGraph` concept (`adjacent_vertices`)
- [x] Define `MutableGraph` concept (`add_vertex`, `remove_vertex`, `add_edge`, `remove_edge`)
- [x] Add `static_assert` tests verifying `adjacency_list` satisfies each concept

### 1.3 Graph Traits Modernization
- [x] Define `vertex_descriptor_t<G>`, `edge_descriptor_t<G>` helper aliases
- [x] Ensure `graph_traits<G>` works with user-defined graph types
- [x] Remove Boost.MPL usage; use `<type_traits>` (`std::conditional_t`, etc.)

### 1.4 Range-Returning Free Functions
- [x] Implement `vertices(g)` returning `std::ranges::subrange`
- [x] Implement `edges(g)` returning `std::ranges::subrange`
- [x] Implement `out_edges(v, g)` returning `std::ranges::subrange`
- [x] Implement `in_edges(v, g)` returning `std::ranges::subrange`
- [x] Implement `adjacent_vertices(v, g)` returning `std::ranges::subrange`
- [x] Verify compatibility with `std::ranges::for_each`, `std::views::filter`, etc.

### 1.5 Property Map Foundation
- [x] Define `PropertyMap` concept (`std::invocable<F, Key>` + `std::convertible_to<Value>`)
- [x] Define `ReadablePropertyMap`, `WritablePropertyMap`, `ReadWritePropertyMap` concepts
- [x] Implement `default_weight_accessor(g)` pattern (graph-capturing lambda)
- [x] Document descriptor types (integral index vs iterator) and container choices
- [x] Remove Boost.PropertyMap dependency from core headers

libs/headers---

## Phase 2: Algorithm Modernization

**Goal:** Modernize algorithm interfaces with ranges, structured returns, and lambda visitors.

### 2.1 Simplified Algorithm Interfaces
- [x] Define `dijkstra_result` struct (distances, predecessors maps)
- [x] Implement `dijkstra_shortest_paths(g, start)` returning `dijkstra_result`
- [x] Implement `dijkstra_shortest_paths(g, start, get_weight)` with custom property accessor
- [x] Apply same pattern to `bellman_ford_shortest_paths`
- [x] Apply same pattern to `bfs` / `dfs` (return visited order or tree)

### 2.2 Named Parameters via Designated Initializers
- [x] Define `dijkstra_params` struct with defaulted template members
- [x] Implement `dijkstra_shortest_paths(g, start, dijkstra_params{...})`
- [x] Apply pattern to other multi-parameter algorithms (A*, Prim, Kruskal)
- [x] Remove Boost.Parameter dependency

### 2.3 Lambda-Friendly Visitors
- [x] Define `bfs_callbacks` struct with `std::function` members
- [x] Implement `breadth_first_search(g, start, bfs_callbacks{...})`
- [x] Define `dfs_callbacks` struct
- [x] Implement `depth_first_search(g, start, dfs_callbacks{...})`
- [x] Support direct lambda/invocable for single-event use case

### 2.4 Range-Based Algorithm Variants
- [x] `breadth_first_search(g, initial_vertices_range, visitor)`
- [x] `depth_first_search(g, initial_vertices_range, visitor)`
- [x] Ensure algorithms accept `std::views::filter` results as input

### 2.5 Minimum Spanning Tree Algorithms
- [x] Modernize `kruskal_minimum_spanning_tree` (return edge range)
- [x] Modernize `prim_minimum_spanning_tree` (return edge range)

### 2.6 Connectivity Algorithms
- [x] Modernize `connected_components` (return component map or range)
- [x] Modernize `strong_components`
- [x] Modernize `topological_sort` (return sorted vertex range)

---

## Phase 3: Graph Container Modernization

**Goal:** Modernize `adjacency_list` and other containers; remove Boost.MPL.

### 3.1 Replace Boost.MPL
- [x] Replace all `boost::mpl::if_` with `std::conditional_t`
- [x] Replace `boost::mpl::and_`, `boost::mpl::or_` with `&&`, `||` on `::value`
- [x] Replace `boost::mpl::bool_` with `std::bool_constant`
- [x] Remove `#include <boost/mpl/*.hpp>` from all headers
- **Note:** The modern/ directory was designed from scratch using C++20 features,
  so it has no Boost.MPL dependencies. Uses std::conditional_t, concepts, etc.

### 3.2 Modernize adjacency_list
- [x] Reorder template parameters for better defaults
- [x] Support bundled properties via simple structs (designated initializer friendly)
- [x] Implement `g[v]` returning vertex property reference
- [x] Implement `g[e]` returning edge property reference
- [x] Ensure `add_vertex({.name = "A", .weight = 1.0})` works

### 3.2.1 Container Selector Implementation ✅ COMPLETE
**Status:** Full container selector infrastructure implemented with vecS, listS, setS support.

**Completed:**
- [x] Create `container_selectors.hpp` with all selector tags and `container_gen` mechanism
- [x] Implement `vecS` selector with `std::vector` backing (index-based descriptors)
- [x] Implement `listS` selector with `std::list` backing (iterator-based descriptors)
- [x] Implement `setS` selector with `std::set` backing (iterator-based descriptors)
- [x] Implement `container_gen` template specializations for all selectors
- [x] Update adjacency_list template parameters: `adjacency_list<OutEdgeListS, VertexListS, DirectedS, VP, EP, GP>`
- [x] Implement vertex removal for stable containers (listS, setS)
- [x] Add `simple_adjacency_list` backward compatibility alias for old API
- [x] Add comprehensive tests (`test_container_selectors.cpp`) covering:
  - [x] Selector properties (is_sequence, is_associative, is_unique, etc.)
  - [x] Container generation for all selector types
  - [x] All selector combinations (vecS/listS/setS for both vertices and edges)
  - [x] Direction variants (directed, undirected, bidirectional)
  - [x] Iterator-based vs index-based descriptor handling

**Selector Tags Implemented:**
- `vecS` → `std::vector<T>` (sequence, not unique, index-based)
- `listS` → `std::list<T>` (sequence, not unique, iterator-based)
- `setS` → `std::set<T>` (associative, unique, iterator-based)
- `mapS` → `std::map<size_t, T>` (mapped, unique)
- `multisetS` → `std::multiset<T>` (associative, not unique)
- `multimapS` → `std::multimap<size_t, T>` (mapped, not unique)
- `hash_setS` → `std::unordered_set<T>` (unordered, unique)
- `hash_mapS` → `std::unordered_map<size_t, T>` (unordered mapped, unique)
- `hash_multisetS` → `std::unordered_multiset<T>` (unordered, not unique)
- `hash_multimapS` → `std::unordered_multimap<size_t, T>` (unordered mapped, not unique)

**Files Created/Modified:**
- `include/bgl/modern/container_selectors.hpp` (NEW - 460 lines)
- `include/bgl/modern/adjacency_list.hpp` (REWRITTEN - new template structure)
- `test/test_container_selectors.cpp` (NEW - comprehensive tests)
- All existing tests updated to use new API

### 3.3 Strong Typing for Descriptors
- [x] Implement `descriptor<Tag>` wrapper with `operator<=>` and hash
- [x] Define `vertex_tag`, `edge_tag`
- [x] Typedef `vertex_descriptor = descriptor<vertex_tag>` (opt-in or default)
- [x] Verify type safety prevents mixing vertex/edge descriptors

### 3.4 Modernize Other Containers
- [x] Implement `adjacency_matrix` with range support, O(1) edge lookup, bundled properties
- [x] Implement `compressed_sparse_row_graph` with `std::span` for zero-copy adjacency access
- [x] Implement `grid_graph` for N-dimensional implicit grid graphs

---

## Phase 4: Advanced Features

**Goal:** Add coroutines, parallel execution, algorithm composition.

### 4.1 Coroutine-Based Traversals (C++20/23)
- [x] Implement `generator<T>` (or use `std::generator` in C++23)
- [x] Implement `bfs_traverse(g, start)` as coroutine yielding vertices
- [x] Implement `dfs_traverse(g, start)` as coroutine yielding vertices
- [x] Support early termination via `co_return` / breaking out of range-for

### 4.2 Parallel Execution Policies
- [x] Identify parallelizable algorithms (BFS levels, independent component processing)
- [x] Add `std::execution::par` overloads for applicable algorithms
- [x] Benchmark parallel vs sequential performance

### 4.3 Algorithm Composition
- [x] Design pipeline/composition API (e.g., `g | find_components() | filter_large()`)
- [x] Prototype with 2-3 composable operations
- [x] Evaluate integration with `std::ranges` pipelines

### 4.4 Validation Framework
- [x] Implement `validate_graph(g)` runtime checks
- [x] Check for invalid descriptors, dangling edges, etc.
- [x] Return structured error information

---

## Phase 5: Testing & Documentation

**Goal:** Comprehensive test coverage and migration documentation.

### 5.1 Concept Tests
- [x] `static_assert` tests for all concepts against `adjacency_list`
- [x] `static_assert` tests for user-defined graph types
- [x] Negative tests (types that should *not* satisfy concepts)

### 5.2 Range Tests
- [x] Test `vertices(g)` with `std::ranges::for_each`
- [x] Test `out_edges(v, g)` with `std::views::filter`
- [x] Test algorithm outputs with `std::ranges::sort`, `std::ranges::copy`
- [x] Test complex range pipelines with multiple views
- [x] Test range property concepts (forward_range, etc.)

### 5.3 Algorithm Tests
- [x] Port existing Boost.Graph test cases to new API
- [x] Add tests for structured returns
- [x] Add tests for lambda visitors
- [x] Add tests for named parameter structs

### 5.4 Performance Tests
- [x] Benchmark Dijkstra (old vs new)
- [x] Benchmark BFS/DFS (old vs new)
- [x] Benchmark graph construction
- [x] Ensure no performance regression

### 5.5 Documentation
- [ ] Write migration guide: old API → new API
- [ ] Document all concepts with examples
- [ ] Document property map patterns (bundled vs external)
- [ ] Provide example programs:
  - [ ] `examples/modern/dijkstra_ranges.cpp`
  - [ ] `examples/modern/bfs_lambda_visitor.cpp`
  - [ ] `examples/modern/custom_graph_type.cpp`

---

## Phase 6: Release & Cleanup

**Goal:** Final cleanup, release preparation.

### 6.1 Dependency Audit
- [ ] Verify zero Boost dependencies in public headers
- [ ] List any remaining internal Boost usage (if any)
- [ ] Document standard library requirements (C++20 minimum)

### 6.2 Header Cleanup
- [ ] Remove dead code / unused legacy paths
- [ ] Organize headers into logical structure
- [ ] Ensure header-only or minimal linking

### 6.3 Release Preparation
- [ ] Update README with new API overview
- [ ] Tag release version
- [ ] Publish release notes highlighting:
  - C++20 concepts
  - Range-based API
  - No Boost dependencies
  - Simplified property maps
  - Lambda visitors

---

## Priority Summary

| Phase | Priority | Effort | Key Deliverables |
|-------|----------|--------|------------------|
| **Phase 1** | HIGH | Medium | Concepts, ranges, property map foundation |
| **Phase 2** | HIGH | High | Modern algorithm APIs, visitors, named params |
| **Phase 3** | MEDIUM-HIGH | High | Container selectors (listS/setS/mapS), strong typing, MPL removal |
| **Phase 4** | MEDIUM | High | Coroutines, parallelism, composition |
| **Phase 5** | HIGH | Medium | Tests, docs, migration guide |
| **Phase 6** | MEDIUM | Low | Cleanup, release |

---

## Dependencies Between Phases

```
Phase 1 (Foundation)
    │
    ├──► Phase 2 (Algorithms) ──► Phase 4 (Advanced)
    │
    └──► Phase 3 (Containers)
                │
                └──► Phase 4 (Advanced)

Phase 5 (Testing) runs in parallel with Phases 2-4
Phase 6 (Release) after all others complete
```

---

## Estimated Timeline

| Phase | Duration | Cumulative |
|-------|----------|------------|
| Phase 1 | 4-6 weeks | 4-6 weeks |
| Phase 2 | 6-8 weeks | 10-14 weeks |
| Phase 3 | 6-8 weeks | 16-22 weeks |
| Phase 4 | 4-6 weeks | 20-28 weeks |
| Phase 5 | 3-4 weeks | 23-32 weeks |
| Phase 6 | 1-2 weeks | 24-34 weeks |

**Total estimated duration: 6-8.5 months**

*Note: Phase 3 duration increased to account for container selector implementation (listS, setS, mapS, multisetS, hash_* variants).*
