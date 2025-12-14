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
- [ ] Define `PropertyMap` concept (`std::invocable<F, Key>` + `std::convertible_to<Value>`)
- [ ] Define `ReadablePropertyMap`, `WritablePropertyMap`, `ReadWritePropertyMap` concepts
- [ ] Implement `default_weight_accessor(g)` pattern (graph-capturing lambda)
- [ ] Document descriptor types (integral index vs iterator) and container choices
- [ ] Remove Boost.PropertyMap dependency from core headers

---

## Phase 2: Algorithm Modernization

**Goal:** Modernize algorithm interfaces with ranges, structured returns, and lambda visitors.

### 2.1 Simplified Algorithm Interfaces
- [ ] Define `dijkstra_result` struct (distances, predecessors maps)
- [ ] Implement `dijkstra_shortest_paths(g, start)` returning `dijkstra_result`
- [ ] Implement `dijkstra_shortest_paths(g, start, get_weight)` with custom property accessor
- [ ] Apply same pattern to `bellman_ford_shortest_paths`
- [ ] Apply same pattern to `bfs` / `dfs` (return visited order or tree)

### 2.2 Named Parameters via Designated Initializers
- [ ] Define `dijkstra_params` struct with defaulted template members
- [ ] Implement `dijkstra_shortest_paths(g, start, dijkstra_params{...})`
- [ ] Apply pattern to other multi-parameter algorithms (A*, Prim, Kruskal)
- [ ] Remove Boost.Parameter dependency

### 2.3 Lambda-Friendly Visitors
- [ ] Define `bfs_callbacks` struct with `std::function` members
- [ ] Implement `breadth_first_search(g, start, bfs_callbacks{...})`
- [ ] Define `dfs_callbacks` struct
- [ ] Implement `depth_first_search(g, start, dfs_callbacks{...})`
- [ ] Support direct lambda/invocable for single-event use case

### 2.4 Range-Based Algorithm Variants
- [ ] `breadth_first_search(g, initial_vertices_range, visitor)`
- [ ] `depth_first_search(g, initial_vertices_range, visitor)`
- [ ] Ensure algorithms accept `std::views::filter` results as input

### 2.5 Minimum Spanning Tree Algorithms
- [ ] Modernize `kruskal_minimum_spanning_tree` (return edge range)
- [ ] Modernize `prim_minimum_spanning_tree` (return edge range)

### 2.6 Connectivity Algorithms
- [ ] Modernize `connected_components` (return component map or range)
- [ ] Modernize `strong_components`
- [ ] Modernize `topological_sort` (return sorted vertex range)

---

## Phase 3: Graph Container Modernization

**Goal:** Modernize `adjacency_list` and other containers; remove Boost.MPL.

### 3.1 Replace Boost.MPL
- [ ] Replace all `boost::mpl::if_` with `std::conditional_t`
- [ ] Replace `boost::mpl::and_`, `boost::mpl::or_` with `&&`, `||` on `::value`
- [ ] Replace `boost::mpl::bool_` with `std::bool_constant`
- [ ] Remove `#include <boost/mpl/*.hpp>` from all headers

### 3.2 Modernize adjacency_list
- [ ] Reorder template parameters for better defaults
- [ ] Support bundled properties via simple structs (designated initializer friendly)
- [ ] Implement `g[v]` returning vertex property reference
- [ ] Implement `g[e]` returning edge property reference
- [ ] Ensure `add_vertex({.name = "A", .weight = 1.0})` works

### 3.3 Strong Typing for Descriptors
- [ ] Implement `descriptor<Tag>` wrapper with `operator<=>` and hash
- [ ] Define `vertex_tag`, `edge_tag`
- [ ] Typedef `vertex_descriptor = descriptor<vertex_tag>` (opt-in or default)
- [ ] Verify type safety prevents mixing vertex/edge descriptors

### 3.4 Modernize Other Containers
- [ ] Review `adjacency_matrix` for range support
- [ ] Review `compressed_sparse_row_graph` for `std::span` usage
- [ ] Review `grid_graph`, `labeled_graph`, `subgraph`

---

## Phase 4: Advanced Features

**Goal:** Add coroutines, parallel execution, algorithm composition.

### 4.1 Coroutine-Based Traversals (C++20/23)
- [ ] Implement `generator<T>` (or use `std::generator` in C++23)
- [ ] Implement `bfs_traverse(g, start)` as coroutine yielding vertices
- [ ] Implement `dfs_traverse(g, start)` as coroutine yielding vertices
- [ ] Support early termination via `co_return` / breaking out of range-for

### 4.2 Parallel Execution Policies
- [ ] Identify parallelizable algorithms (BFS levels, independent component processing)
- [ ] Add `std::execution::par` overloads for applicable algorithms
- [ ] Benchmark parallel vs sequential performance

### 4.3 Algorithm Composition
- [ ] Design pipeline/composition API (e.g., `g | find_components() | filter_large()`)
- [ ] Prototype with 2-3 composable operations
- [ ] Evaluate integration with `std::ranges` pipelines

### 4.4 Validation Framework
- [ ] Implement `validate_graph(g)` runtime checks
- [ ] Check for invalid descriptors, dangling edges, etc.
- [ ] Return structured error information

---

## Phase 5: Testing & Documentation

**Goal:** Comprehensive test coverage and migration documentation.

### 5.1 Concept Tests
- [ ] `static_assert` tests for all concepts against `adjacency_list`
- [ ] `static_assert` tests for user-defined graph types
- [ ] Negative tests (types that should *not* satisfy concepts)

### 5.2 Range Tests
- [ ] Test `vertices(g)` with `std::ranges::for_each`
- [ ] Test `out_edges(v, g)` with `std::views::filter`
- [ ] Test algorithm outputs with `std::ranges::sort`, `std::ranges::copy`

### 5.3 Algorithm Tests
- [ ] Port existing Boost.Graph test cases to new API
- [ ] Add tests for structured returns
- [ ] Add tests for lambda visitors
- [ ] Add tests for named parameter structs

### 5.4 Performance Tests
- [ ] Benchmark Dijkstra (old vs new)
- [ ] Benchmark BFS/DFS (old vs new)
- [ ] Benchmark graph construction
- [ ] Ensure no performance regression

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
| **Phase 3** | MEDIUM-HIGH | Medium | Container cleanup, strong typing, MPL removal |
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
| Phase 3 | 4-6 weeks | 14-20 weeks |
| Phase 4 | 4-6 weeks | 18-26 weeks |
| Phase 5 | 3-4 weeks | 21-30 weeks |
| Phase 6 | 1-2 weeks | 22-32 weeks |

**Total estimated duration: 5-8 months**
