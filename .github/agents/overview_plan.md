# Overview Implementation Plan

**Strategy reference:** [overview_strategy.md](overview_strategy.md)  
**Goal reference:** [overview_goal.md](overview_goal.md)  
**Target branch:** `test_reorg`  
**Baseline commit:** `05e28619` (2026-05-11)  
**C++ standard:** 14 (from [meta/libraries.json](../../meta/libraries.json))  
**Parallel BGL:** out of scope for this plan; handled separately.

---

## Phase A — Reconnaissance

**Output:** one-sentence role for every top-level directory; branch recorded above.

### A.1 — Read top-level narrative sources

| File | Purpose |
|------|---------|
| [README.md](../../README.md) | Project description, build instructions, test commands, CI links |
| [index.html](../../index.html) | Alternate entry point (may duplicate README) |
| [meta/libraries.json](../../meta/libraries.json) | Authors, category, C++ standard; **no `dependencies` key** — list build prerequisites from CMakeLists.txt instead |

Key facts already captured:

- **Name:** Boost Graph Library (BGL)
- **Authors:** Jeremy Siek and University of Notre Dame team
- **Maintainer:** Jeremy W. Murphy
- **Category:** Algorithms, Containers, Iterators
- **C++ std:** 14
- **Build note:** Cannot currently be built outside of the full Boost tree

### A.2 — Skim doc taxonomy

Source: [doc/table_of_contents.html](../../doc/table_of_contents.html)

Documented sections (reuse these headings in overview subdocuments):

1. Introduction / File Dependency Example / Kevin Bacon
2. Concepts (Graph, Vertex/EdgeList, Adjacency, Bidirectional, Mutable graphs)
3. Algorithms — BFS, DFS, Dijkstra, Bellman-Ford, DAG shortest paths, A\*
4. Algorithms — All-pairs shortest paths (Johnson, Floyd-Warshall)
5. Algorithms — Transitive closure, dominator tree, components
6. Algorithms — Flow (Edmonds-Karp, Boykov-Kolmogorov, push-relabel)
7. Algorithms — Min cut (Stoer-Wagner), matching, coloring
8. Algorithms — Ordering (Cuthill-McKee, King, Sloan, minimum degree)
9. Algorithms — Planarity (Boyer-Myrvold, face traversal, straight-line drawing)
10. Algorithms — Layout (random, circle, Kamada-Kawai, Fruchterman-Reingold, Gursoy-Atun)
11. Algorithms — Centrality and clustering (betweenness, Louvain)
12. Graph types — adjacency_list, adjacency_matrix, CSR, grid_graph
13. Visitors and EventVisitor concept
14. Property maps and named parameters
15. IO — GraphViz, GraphML, DIMACS, METIS

### A.3 — Directory role summary

| Directory / File | Role |
|-----------------|------|
| `include/boost/graph/` | Public headers — canonical source of truth for the API |
| `src/` | Compiled components: `graphml.cpp`, `read_graphviz_new.cpp` |
| `doc/` | Long-form HTML documentation |
| `example/` | ~100 illustrative `.cpp` programs |
| `test/` | 300+ regression tests (`Jamfile.v2`) |
| `build/` | Jamfile.v2 for compiled components |
| `meta/` | `libraries.json` — Boost metadata |
| `.github/` | CI workflows and agent documents |
| `CMakeLists.txt` | CMake entry point |
| `build.jam` | Boost.Build entry point |

---

## Phase B — Inventory

**Output:** grouped header list; one representative header per algorithm family.

### B.1 — Group all public headers in `include/boost/graph/`

Exclude from public API presentation: `detail/`, `planar_detail/`,
`iteration_macros_undef.hpp`, `dll_import_export.hpp`, `use_mpi.hpp` (MPI, out of scope),
`overloading.hpp`, `relax.hpp` (internal helpers).

Note: `edmunds_karp_max_flow.hpp` appears to be a legacy spelling alias for
`edmonds_karp_max_flow.hpp` — confirm and mention only the canonical name.

#### Graph Data Structures

| Header | Description |
|--------|-------------|
| `adjacency_list.hpp` | Primary general-purpose graph; parameterized on container selectors |
| `adjacency_matrix.hpp` | Dense matrix-backed graph |
| `compressed_sparse_row_graph.hpp` | Read-optimized CSR format |
| `directed_graph.hpp` | Convenience wrapper for directed adjacency_list |
| `undirected_graph.hpp` | Convenience wrapper for undirected adjacency_list |
| `grid_graph.hpp` | Implicit N-dimensional grid |
| `filtered_graph.hpp` | Filtered view of another graph |
| `reverse_graph.hpp` | Edge-reversed view |
| `subgraph.hpp` | Subgraph with shared property storage |
| `edge_list.hpp` | Edge-sequence–based graph |
| `labeled_graph.hpp` | Graph with label-to-vertex mapping |
| `graph_as_tree.hpp` | Tree view adapter |
| `vector_as_graph.hpp`, `matrix_as_graph.hpp`, `stanford_graph.hpp`, `leda_graph.hpp` | Third-party graph adapters |

#### Traversal and Search

| Header | Algorithm |
|--------|-----------|
| `breadth_first_search.hpp` | BFS |
| `depth_first_search.hpp` | DFS |
| `undirected_dfs.hpp` | Undirected DFS |
| `neighbor_bfs.hpp` | Neighbor BFS |
| `astar_search.hpp` | A\* heuristic search |
| `maximum_adjacency_search.hpp` | Maximum adjacency search |

#### Shortest Paths

| Header | Algorithm |
|--------|-----------|
| `dijkstra_shortest_paths.hpp` | Dijkstra (with color map) |
| `dijkstra_shortest_paths_no_color_map.hpp` | Dijkstra (without color map) |
| `bellman_ford_shortest_paths.hpp` | Bellman-Ford |
| `dag_shortest_paths.hpp` | DAG shortest paths |
| `johnson_all_pairs_shortest.hpp` | Johnson all-pairs |
| `floyd_warshall_shortest.hpp` | Floyd-Warshall all-pairs |
| `r_c_shortest_paths.hpp` | Resource-constrained shortest paths |

#### Minimum Spanning Trees

| Header | Algorithm |
|--------|-----------|
| `kruskal_min_spanning_tree.hpp` | Kruskal MST |
| `prim_minimum_spanning_tree.hpp` | Prim MST |
| `random_spanning_tree.hpp` | Random spanning tree |
| `two_graphs_common_spanning_trees.hpp` | Common spanning trees of two graphs |

#### Network Flow

| Header | Algorithm |
|--------|-----------|
| `edmonds_karp_max_flow.hpp` | Edmonds-Karp max flow |
| `push_relabel_max_flow.hpp` | Push-relabel max flow |
| `boykov_kolmogorov_max_flow.hpp` | Boykov-Kolmogorov max flow |
| `cycle_canceling.hpp` | Cycle-canceling min-cost flow |
| `successive_shortest_path_nonnegative_weights.hpp` | SSP min-cost flow |
| `find_flow_cost.hpp` | Flow cost query |

#### Connected Components

| Header | Algorithm |
|--------|-----------|
| `connected_components.hpp` | Connected components (undirected) |
| `strong_components.hpp` | Strongly connected components (Tarjan/Kosaraju) |
| `biconnected_components.hpp` | Biconnected components and articulation points |
| `incremental_components.hpp` | Incremental connected components (union-find) |
| `st_connected.hpp` | s-t connectivity query |
| `edge_connectivity.hpp` | Edge connectivity |

#### Graph Coloring and Matching

| Header | Algorithm |
|--------|-----------|
| `sequential_vertex_coloring.hpp` | Greedy vertex coloring |
| `edge_coloring.hpp` | Edge coloring |
| `max_cardinality_matching.hpp` | Maximum cardinality matching |
| `maximum_weighted_matching.hpp` | Maximum weighted matching |
| `bipartite.hpp` | Bipartite testing and odd-cycle detection |

#### Ordering and Reordering

| Header | Algorithm |
|--------|-----------|
| `topological_sort.hpp` | Topological sort |
| `cuthill_mckee_ordering.hpp` | Cuthill-McKee bandwidth reduction |
| `king_ordering.hpp` | King ordering |
| `sloan_ordering.hpp` | Sloan ordering |
| `minimum_degree_ordering.hpp` | Minimum degree ordering |
| `smallest_last_ordering.hpp` | Smallest-last ordering |
| `bandwidth.hpp`, `profile.hpp`, `wavefront.hpp` | Bandwidth/profile/wavefront metrics |

#### Planarity

| Header | Purpose |
|--------|---------|
| `boyer_myrvold_planar_test.hpp` | Boyer-Myrvold planarity test |
| `planar_face_traversal.hpp` | Face traversal |
| `planar_canonical_ordering.hpp` | Canonical ordering |
| `chrobak_payne_drawing.hpp` | Straight-line planar drawing |
| `is_straight_line_drawing.hpp` | Drawing verification |
| `is_kuratowski_subgraph.hpp` | Kuratowski subgraph check |
| `make_connected.hpp`, `make_biconnected_planar.hpp`, `make_maximal_planar.hpp` | Graph augmentation |

#### Graph Layout

| Header | Algorithm |
|--------|-----------|
| `random_layout.hpp` | Random placement |
| `circle_layout.hpp` | Circular layout |
| `kamada_kawai_spring_layout.hpp` | Kamada-Kawai spring layout |
| `fruchterman_reingold.hpp` | Fruchterman-Reingold force-directed |
| `gursoy_atun_layout.hpp` | Gursoy-Atun SOM layout |
| `topology.hpp` | Topology primitives (used by layout algorithms) |

#### Centrality and Clustering

| Header | Measure |
|--------|---------|
| `betweenness_centrality.hpp`, `bc_clustering.hpp` | Betweenness centrality and clustering |
| `closeness_centrality.hpp` | Closeness centrality |
| `degree_centrality.hpp` | Degree centrality |
| `eccentricity.hpp`, `geodesic_distance.hpp` | Eccentricity / geodesic distance |
| `clustering_coefficient.hpp` | Clustering coefficient |
| `core_numbers.hpp` | Core decomposition |
| `page_rank.hpp` | PageRank |
| `louvain_clustering.hpp`, `louvain_quality_functions.hpp` | Louvain community detection |

#### Cycles, Cliques, and Subgraph Isomorphism

| Header | Algorithm |
|--------|-----------|
| `tiernan_all_cycles.hpp` | All cycles (Tiernan) |
| `hawick_circuits.hpp` | All circuits (Hawick-James) |
| `howard_cycle_ratio.hpp` | Minimum/maximum cycle ratio |
| `bron_kerbosch_all_cliques.hpp` | All cliques (Bron-Kerbosch) |
| `isomorphism.hpp` | Graph isomorphism |
| `vf2_sub_graph_iso.hpp` | VF2 subgraph isomorphism |
| `mcgregor_common_subgraphs.hpp` | McGregor common subgraphs |

#### Graph Generators

| Header | Generator |
|--------|-----------|
| `erdos_renyi_generator.hpp` | Erdős-Rényi random graph |
| `plod_generator.hpp` | Power-law out-degree |
| `rmat_graph_generator.hpp` | R-MAT |
| `mesh_graph_generator.hpp` | Mesh / grid |
| `small_world_generator.hpp` | Watts-Strogatz small world |
| `ssca_graph_generator.hpp` | SSCA benchmark |
| `random.hpp` | Random graph utilities |

#### Graph Utility and Transformation

| Header | Purpose |
|--------|---------|
| `copy.hpp` | Graph copy |
| `transpose_graph.hpp` | Transpose |
| `transitive_closure.hpp`, `transitive_reduction.hpp` | Closure / reduction |
| `create_condensation_graph.hpp` | SCC condensation |
| `dominator_tree.hpp` | Dominator tree (Lengauer-Tarjan) |
| `stoer_wagner_min_cut.hpp` | Stoer-Wagner global min cut |
| `metric_tsp_approx.hpp` | Metric TSP approximation |
| `loop_erased_random_walk.hpp` | Loop-erased random walk |
| `graph_stats.hpp`, `graph_utility.hpp`, `lookup_edge.hpp` | Statistics and utility queries |

#### Concepts, Traits, and Archetypes

| Header | Purpose |
|--------|---------|
| `graph_concepts.hpp` | Concept definitions and checks |
| `graph_traits.hpp` | `graph_traits<G>` trait class |
| `graph_selectors.hpp` | `directedS`, `undirectedS`, `bidirectionalS` tags |
| `graph_mutability_traits.hpp` | Mutability trait tags |
| `graph_archetypes.hpp` | Concept archetypes for testing |
| `tree_traits.hpp` | Tree-specific traits |
| `buffer_concepts.hpp`, `point_traits.hpp` | Buffer and point concepts |
| `adjacency_iterator.hpp`, `vertex_and_edge_range.hpp` | Iterator utilities |

#### Property Maps and Named Parameters

| Header | Purpose |
|--------|---------|
| `properties.hpp` | Built-in property tags |
| `named_function_params.hpp` | Named parameter mechanism |
| `named_graph.hpp` | Named-vertex graph mixin |
| `exterior_property.hpp` | Exterior property storage helpers |
| `property_iter_range.hpp` | Property iteration |
| `one_bit_color_map.hpp`, `two_bit_color_map.hpp` | Compact color maps |
| `property_maps/` | Property map adaptors subdirectory |

#### Visitors

| Header | Purpose |
|--------|---------|
| `visitors.hpp` | Standard visitor adaptors (predecessor recorder, distance recorder, etc.) |

#### IO

| Header | Format |
|--------|--------|
| `graphviz.hpp` | GraphViz DOT read/write |
| `graphml.hpp` | GraphML read/write (has compiled component in `src/`) |
| `adj_list_serialize.hpp` | Boost.Serialization support |
| `adjacency_list_io.hpp` | Simple text IO for adjacency_list |
| `dimacs.hpp`, `read_dimacs.hpp`, `write_dimacs.hpp` | DIMACS format |
| `metis.hpp` | METIS partitioning format |

### B.2 — Compile components

Only two source files in `src/`:

- `src/graphml.cpp` — GraphML parser (compiled, links against Boost.PropertyTree)
- `src/read_graphviz_new.cpp` — DOT reader (compiled)

All other functionality is header-only.

---

## Phase C — Drafting

**Output:** `overview.md` plus all subdocuments listed in §2 of the strategy.

Work in this order; create each file before starting the next.

### C.1 — Create `overview.md`

File: `.github/agents/overview.md`  
Purpose: thin index — one paragraph per topic, one link per subdocument.  
Length budget: 60–120 lines.

Required sections:

1. One-paragraph description of BGL (from README / meta)
2. C++ standard, build system, build note (outside Boost tree not supported)
3. Table of contents linking to each subdocument
4. "How to update this overview" note pointing to [overview_strategy.md](overview_strategy.md)
5. Last-reviewed stamp: branch `test_reorg`, commit `05e28619`, date 2026-05-11

### C.2 — Create `overview/organization.md`

Cover the directory role table from Phase A.3 above. Mention both build systems
(`CMakeLists.txt` and `build.jam`). Note the two compiled components.

### C.3 — Create `overview/architecture.md`

Cover in order:

1. Generic programming model — policies via template parameters
2. `graph_traits<G>` — the universal adapter
3. Concept hierarchy (brief; link to `concepts.md` for detail)
4. Property maps — interior vs exterior
5. Named parameters (`named_function_params.hpp`)
6. Visitor pattern — event-point model, `visitors.hpp`
7. Graph selectors (`directedS`, `undirectedS`, `bidirectionalS`)

Sources: `include/boost/graph/graph_traits.hpp`,
`include/boost/graph/graph_concepts.hpp`,
`include/boost/graph/named_function_params.hpp`,
`include/boost/graph/visitors.hpp`,
`doc/constructing_algorithms.html`,
`doc/visitor_concepts.html`

### C.4 — Create `overview/graph-types.md`

One row/paragraph per entry in the "Graph Data Structures" group from B.1.
For each entry link to (a) its header and (b) its doc page when one exists.

### C.5 — Create `overview/algorithms.md`

Use the algorithm family groups from B.1 as sections.
For each family: 1–2 sentences of description, representative header link,
representative doc page link.

### C.6 — Create `overview/concepts.md`

Cover the concept refinement hierarchy from `doc/graph_concepts.html`.
Reference `include/boost/graph/graph_concepts.hpp` and
`include/boost/graph/graph_archetypes.hpp`.

### C.7 — Create `overview/build-and-test.md`

Cover:

1. Prerequisites (C++14; repo-local editing is fine, but full build/test usually
      requires a Boost superproject checkout)
2. Repo-local default: describe the files and entry points without promising a
      standalone build. Mention [CMakeLists.txt](../../CMakeLists.txt),
      [build.jam](../../build.jam), and [build/Jamfile.v2](../../build/Jamfile.v2)
      as the relevant build definitions in this repo.
3. Optional superproject workflow: if the user is inside a full `boostorg/boost`
      checkout, document Boost.Build commands such as `./b2 headers`, `./b2`, and
      test execution from `libs/graph/test/`; document the corresponding
      superproject-driven CMake workflow there as an optional path, not the default.
4. Compiled components (`src/`) and what they require
5. Examples (`example/`) — how to build and run a single example
6. How docs are produced (`doc/BUILD_DOCS.sh`)

### C.8 — Create `overview/extending.md`

Cover:

1. Adding a new algorithm: follow visitor event-point pattern; use
   `named_function_params.hpp` for parameters
2. Adding a new graph type: implement the relevant concept set
3. Adding a new property map: use `exterior_property.hpp` or implement
   the property map concept from Boost.PropertyMap
4. Testing conventions (matching test in `test/`, entry in `test/Jamfile.v2`)

### C.9 — Create `overview/glossary.md`

Define at minimum: graph descriptor, property map, visitor, traversal
category, named parameter, interior property, exterior property,
vertex/edge bundle, selector tag, event point, archetype.

### C.10 — Create `overview/purpose.md`

Summarize:

- What problems BGL solves (generic graph algorithms over user-defined graph types)
- Why it is important (decouples algorithm from data structure via concepts)
- Primary audience (C++ library and application developers)
- Relationship to STL (same generic/concept-based philosophy)

---

## Phase D — Verification

For each document created in Phase C:

- [ ] Verify every `include/boost/graph/` link targets a file that actually exists
- [ ] Verify every `doc/` link targets a file that actually exists
- [ ] Check the `edmunds_karp_max_flow.hpp` alias situation and document correctly
- [ ] Re-read `overview.md` as an agent: can you locate the right header for any
      algorithm family in ≤ 2 hops?
- [ ] Re-read `overview.md` as a newcomer: is the first paragraph self-explanatory
      without prior Boost knowledge?
- [ ] Check for duplicated content across `architecture.md` and `concepts.md`;
      consolidate if found
- [ ] Confirm `overview.md` fits on one screen (~60–120 lines)

---

## Phase E — Maintenance Hooks

- [ ] The "How to update" note and last-reviewed stamp are in `overview.md` (done in C.1)
- [ ] Add a trigger note to [overview_strategy.md](overview_strategy.md)
      if any revision trigger fires during production

---

## Status

| Task | Status | Notes |
|------|--------|-------|
| **Phase A — Reconnaissance** | ✅ Complete | Facts captured in this plan |
| A.1 Read README, index.html, meta/libraries.json | ✅ Complete | No `dependencies` key in JSON |
| A.2 Skim doc/table_of_contents.html | ✅ Complete | Taxonomy sections listed above |
| A.3 Directory role summary | ✅ Complete | Table above |
| **Phase B — Inventory** | ✅ Complete | All 150+ headers grouped |
| B.1 Group public headers | ✅ Complete | 14 groups in tables above |
| B.2 Compiled components | ✅ Complete | 2 files in `src/` |
| **Phase C — Drafting** | ✅ Complete | C.1–C.10 all complete |
| C.1 `overview.md` | ✅ Complete | `.github/agents/overview.md` |
| C.2 `organization.md` | ✅ Complete | `.github/agents/overview/organization.md` |
| C.3 `architecture.md` | ✅ Complete | `.github/agents/overview/architecture.md` |
| C.4 `graph-types.md` | ✅ Complete | `.github/agents/overview/graph-types.md` |
| C.5 `algorithms.md` | ✅ Complete | `.github/agents/overview/algorithms.md` |
| C.6 `concepts.md` | ✅ Complete | `.github/agents/overview/concepts.md` |
| C.7 `build-and-test.md` | ✅ Complete | `.github/agents/overview/build-and-test.md` |
| C.8 `extending.md` | ✅ Complete | `.github/agents/overview/extending.md` |
| C.9 `glossary.md` | ✅ Complete | `.github/agents/overview/glossary.md` |
| C.10 `purpose.md` | ✅ Complete | `.github/agents/overview/purpose.md` |
| **Phase D — Verification** | ⬜ Not started | |
| **Phase E — Maintenance Hooks** | ⬜ Not started | |
