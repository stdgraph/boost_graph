# Algorithms

Back to [overview.md](../overview.md)

BGL algorithms are free-function templates parameterized on graph and
property-map types. They require no particular graph class — any type
modeling the stated concept works. All accept optional parameters via the
named-parameter mechanism (see [architecture.md](architecture.md)).

This document surveys the algorithm families, with one representative header
and doc link per family. Full per-algorithm documentation lives in
[`doc/`](../../../doc/).

---

## Traversal and Search

Algorithms that visit every reachable vertex in a defined order. Both accept
a visitor object invoked at event points during traversal.

| Header | Algorithm | Requires | Doc |
|--------|-----------|----------|-----|
| [`breadth_first_search.hpp`](../../../include/boost/graph/breadth_first_search.hpp) | BFS | `IncidenceGraph` + `VertexListGraph` | [`doc/breadth_first_search.html`](../../../doc/breadth_first_search.html) |
| [`depth_first_search.hpp`](../../../include/boost/graph/depth_first_search.hpp) | DFS | `IncidenceGraph` + `VertexListGraph` | [`doc/depth_first_search.html`](../../../doc/depth_first_search.html) |
| [`undirected_dfs.hpp`](../../../include/boost/graph/undirected_dfs.hpp) | Undirected DFS | `IncidenceGraph` + `VertexListGraph` | (see DFS doc) |
| [`neighbor_bfs.hpp`](../../../include/boost/graph/neighbor_bfs.hpp) | Neighbor BFS | `AdjacencyGraph` | — |
| [`astar_search.hpp`](../../../include/boost/graph/astar_search.hpp) | A\* heuristic search | `IncidenceGraph` + `VertexListGraph` | [`doc/astar_search.html`](../../../doc/astar_search.html) |
| [`maximum_adjacency_search.hpp`](../../../include/boost/graph/maximum_adjacency_search.hpp) | Maximum adjacency search | `VertexListGraph` + `IncidenceGraph` | — |

---

## Shortest Paths

Algorithms that compute single-source or all-pairs shortest paths. Property
maps supply edge weights, distance outputs, and predecessor outputs.

| Header | Algorithm | Notes | Doc |
|--------|-----------|-------|-----|
| [`dijkstra_shortest_paths.hpp`](../../../include/boost/graph/dijkstra_shortest_paths.hpp) | Dijkstra (with color map) | Non-negative weights | [`doc/dijkstra_shortest_paths.html`](../../../doc/dijkstra_shortest_paths.html) |
| [`dijkstra_shortest_paths_no_color_map.hpp`](../../../include/boost/graph/dijkstra_shortest_paths_no_color_map.hpp) | Dijkstra (no color map) | Saves memory; same complexity | [`doc/dijkstra_shortest_paths.html`](../../../doc/dijkstra_shortest_paths.html) |
| [`bellman_ford_shortest_paths.hpp`](../../../include/boost/graph/bellman_ford_shortest_paths.hpp) | Bellman-Ford | Handles negative weights; detects negative cycles | [`doc/bellman_ford_shortest.html`](../../../doc/bellman_ford_shortest.html) |
| [`dag_shortest_paths.hpp`](../../../include/boost/graph/dag_shortest_paths.hpp) | DAG shortest paths | Linear time; directed acyclic graphs only | [`doc/dag_shortest_paths.html`](../../../doc/dag_shortest_paths.html) |
| [`astar_search.hpp`](../../../include/boost/graph/astar_search.hpp) | A\* | Heuristic-guided; same interface as Dijkstra | [`doc/astar_search.html`](../../../doc/astar_search.html) |
| [`johnson_all_pairs_shortest.hpp`](../../../include/boost/graph/johnson_all_pairs_shortest.hpp) | Johnson all-pairs | Handles negative weights via reweighting | [`doc/johnson_all_pairs_shortest.html`](../../../doc/johnson_all_pairs_shortest.html) |
| [`floyd_warshall_shortest.hpp`](../../../include/boost/graph/floyd_warshall_shortest.hpp) | Floyd-Warshall all-pairs | O(V³); simple; detects negative cycles | [`doc/floyd_warshall_shortest.html`](../../../doc/floyd_warshall_shortest.html) |
| [`r_c_shortest_paths.hpp`](../../../include/boost/graph/r_c_shortest_paths.hpp) | Resource-constrained shortest paths | Pareto-optimal paths under multiple constraints | [`doc/r_c_shortest_paths.html`](../../../doc/r_c_shortest_paths.html) |

---

## Minimum Spanning Trees

Compute a minimum-weight spanning tree (or forest) of an undirected graph.

| Header | Algorithm | Notes | Doc |
|--------|-----------|-------|-----|
| [`kruskal_min_spanning_tree.hpp`](../../../include/boost/graph/kruskal_min_spanning_tree.hpp) | Kruskal | `EdgeListGraph` + disjoint-sets; outputs an edge iterator range | [`doc/kruskal_min_spanning_tree.html`](../../../doc/kruskal_min_spanning_tree.html) |
| [`prim_minimum_spanning_tree.hpp`](../../../include/boost/graph/prim_minimum_spanning_tree.hpp) | Prim | `VertexListGraph` + `IncidenceGraph`; outputs a predecessor map | [`doc/prim_minimum_spanning_tree.html`](../../../doc/prim_minimum_spanning_tree.html) |
| [`random_spanning_tree.hpp`](../../../include/boost/graph/random_spanning_tree.hpp) | Random spanning tree | Loop-erased random walk (Wilson's algorithm) | [`doc/random_spanning_tree.html`](../../../doc/random_spanning_tree.html) |
| [`two_graphs_common_spanning_trees.hpp`](../../../include/boost/graph/two_graphs_common_spanning_trees.hpp) | Common spanning trees | Enumerates spanning trees common to two graphs | [`doc/two_graphs_common_spanning_trees.html`](../../../doc/two_graphs_common_spanning_trees.html) |

---

## Network Flow

Max-flow and min-cost flow algorithms. All consume a capacity edge
property map and return a flow property map.

| Header | Algorithm | Notes | Doc |
|--------|-----------|-------|-----|
| [`edmonds_karp_max_flow.hpp`](../../../include/boost/graph/edmonds_karp_max_flow.hpp) | Edmonds-Karp max flow | BFS-augmenting path; O(VE²) | [`doc/edmonds_karp_max_flow.html`](../../../doc/edmonds_karp_max_flow.html) |
| [`push_relabel_max_flow.hpp`](../../../include/boost/graph/push_relabel_max_flow.hpp) | Push-relabel max flow | O(V²√E); faster on dense graphs | [`doc/push_relabel_max_flow.html`](../../../doc/push_relabel_max_flow.html) |
| [`boykov_kolmogorov_max_flow.hpp`](../../../include/boost/graph/boykov_kolmogorov_max_flow.hpp) | Boykov-Kolmogorov | Efficient for computer vision / image segmentation problems | [`doc/boykov_kolmogorov_max_flow.html`](../../../doc/boykov_kolmogorov_max_flow.html) |
| [`cycle_canceling.hpp`](../../../include/boost/graph/cycle_canceling.hpp) | Cycle-canceling min-cost flow | Requires an initial feasible flow | [`doc/cycle_canceling.html`](../../../doc/cycle_canceling.html) |
| [`successive_shortest_path_nonnegative_weights.hpp`](../../../include/boost/graph/successive_shortest_path_nonnegative_weights.hpp) | Successive shortest paths | Min-cost flow; requires non-negative reduced costs | [`doc/successive_shortest_path_nonnegative_weights.html`](../../../doc/successive_shortest_path_nonnegative_weights.html) |
| [`find_flow_cost.hpp`](../../../include/boost/graph/find_flow_cost.hpp) | Flow cost query | Returns total cost of a given flow | [`doc/find_flow_cost.html`](../../../doc/find_flow_cost.html) |

Note: `edmunds_karp_max_flow.hpp` (alternate spelling) is a legacy alias for
`edmonds_karp_max_flow.hpp`; use the canonical spelling.

---

## Connected Components

Algorithms that partition a graph's vertices into connected or strongly
connected subsets.

| Header | Algorithm | Notes | Doc |
|--------|-----------|-------|-----|
| [`connected_components.hpp`](../../../include/boost/graph/connected_components.hpp) | Connected components | Undirected graphs; labels each vertex | [`doc/connected_components.html`](../../../doc/connected_components.html) |
| [`strong_components.hpp`](../../../include/boost/graph/strong_components.hpp) | Strongly connected components | Directed graphs; Tarjan / Kosaraju | [`doc/strong_components.html`](../../../doc/strong_components.html) |
| [`biconnected_components.hpp`](../../../include/boost/graph/biconnected_components.hpp) | Biconnected components and articulation points | Also outputs articulation points | [`doc/biconnected_components.html`](../../../doc/biconnected_components.html) |
| [`incremental_components.hpp`](../../../include/boost/graph/incremental_components.hpp) | Incremental connected components | Union-find; efficient for dynamic edge additions | [`doc/incremental_components.html`](../../../doc/incremental_components.html) |
| [`st_connected.hpp`](../../../include/boost/graph/st_connected.hpp) | s-t connectivity | Boolean reachability query | — |
| [`edge_connectivity.hpp`](../../../include/boost/graph/edge_connectivity.hpp) | Edge connectivity | Minimum number of edges whose removal disconnects the graph | — |

---

## Graph Coloring and Matching

Algorithms for coloring vertices/edges and finding maximum matchings.

| Header | Algorithm | Notes | Doc |
|--------|-----------|-------|-----|
| [`sequential_vertex_coloring.hpp`](../../../include/boost/graph/sequential_vertex_coloring.hpp) | Sequential vertex coloring | Greedy; number of colors used depends on vertex ordering | [`doc/graph_coloring.html`](../../../doc/graph_coloring.html) |
| [`edge_coloring.hpp`](../../../include/boost/graph/edge_coloring.hpp) | Edge coloring | Uses at most Δ+1 colors | [`doc/edge_coloring.html`](../../../doc/edge_coloring.html) |
| [`max_cardinality_matching.hpp`](../../../include/boost/graph/max_cardinality_matching.hpp) | Maximum cardinality matching | General (non-bipartite) Edmond's blossom algorithm | — |
| [`maximum_weighted_matching.hpp`](../../../include/boost/graph/maximum_weighted_matching.hpp) | Maximum weighted matching | General weighted matching | — |
| [`bipartite.hpp`](../../../include/boost/graph/bipartite.hpp) | Bipartite testing | Also finds an odd cycle certificate if not bipartite | [`doc/find_odd_cycle.html`](../../../doc/find_odd_cycle.html) |

---

## Ordering and Reordering

Algorithms that reorder vertices to reduce matrix bandwidth, fill-in, or
wavefront. Useful as preprocessing steps for sparse linear solvers.

| Header | Algorithm | Notes | Doc |
|--------|-----------|-------|-----|
| [`topological_sort.hpp`](../../../include/boost/graph/topological_sort.hpp) | Topological sort | DFS-based; requires DAG | [`doc/topological_sort.html`](../../../doc/topological_sort.html) |
| [`cuthill_mckee_ordering.hpp`](../../../include/boost/graph/cuthill_mckee_ordering.hpp) | Cuthill-McKee / reverse Cuthill-McKee | BFS-based bandwidth reduction | [`doc/cuthill_mckee_ordering.html`](../../../doc/cuthill_mckee_ordering.html) |
| [`king_ordering.hpp`](../../../include/boost/graph/king_ordering.hpp) | King ordering | Profile/wavefront reduction | [`doc/bandwidth.html`](../../../doc/bandwidth.html) |
| [`sloan_ordering.hpp`](../../../include/boost/graph/sloan_ordering.hpp) | Sloan ordering | Profile/wavefront reduction | [`doc/sloan_ordering.htm`](../../../doc/sloan_ordering.htm) |
| [`minimum_degree_ordering.hpp`](../../../include/boost/graph/minimum_degree_ordering.hpp) | Minimum degree ordering | Fill-reducing ordering for sparse Cholesky | [`doc/minimum_degree_ordering.html`](../../../doc/minimum_degree_ordering.html) |
| [`smallest_last_ordering.hpp`](../../../include/boost/graph/smallest_last_ordering.hpp) | Smallest-last ordering | Degree-based; useful before sequential coloring | — |
| `bandwidth.hpp`, `profile.hpp`, `wavefront.hpp` | Metric queries | Compute bandwidth, profile, and wavefront of a graph ordering | [`doc/bandwidth.html`](../../../doc/bandwidth.html) |

---

## Planarity

Algorithms for testing planarity, finding planar embeddings, and producing
planar straight-line drawings.

| Header | Algorithm | Notes | Doc |
|--------|-----------|-------|-----|
| [`boyer_myrvold_planar_test.hpp`](../../../include/boost/graph/boyer_myrvold_planar_test.hpp) | Boyer-Myrvold planarity test | Also produces a planar embedding or Kuratowski subgraph | [`doc/boyer_myrvold.html`](../../../doc/boyer_myrvold.html) |
| [`planar_face_traversal.hpp`](../../../include/boost/graph/planar_face_traversal.hpp) | Planar face traversal | Visits faces of a planar embedding | [`doc/planar_face_traversal.html`](../../../doc/planar_face_traversal.html) |
| [`planar_canonical_ordering.hpp`](../../../include/boost/graph/planar_canonical_ordering.hpp) | Canonical ordering | Required input for Chrobak-Payne drawing | [`doc/planar_canonical_ordering.html`](../../../doc/planar_canonical_ordering.html) |
| [`chrobak_payne_drawing.hpp`](../../../include/boost/graph/chrobak_payne_drawing.hpp) | Chrobak-Payne straight-line drawing | Grid drawing; uses canonical ordering | [`doc/straight_line_drawing.html`](../../../doc/straight_line_drawing.html) |
| [`is_straight_line_drawing.hpp`](../../../include/boost/graph/is_straight_line_drawing.hpp) | Drawing verification | Checks that no edges cross | — |
| [`is_kuratowski_subgraph.hpp`](../../../include/boost/graph/is_kuratowski_subgraph.hpp) | Kuratowski subgraph check | Verifies a non-planarity certificate | — |
| [`make_connected.hpp`](../../../include/boost/graph/make_connected.hpp) | Augment to connected | Preprocessing for planar embedding | [`doc/make_connected.html`](../../../doc/make_connected.html) |
| [`make_biconnected_planar.hpp`](../../../include/boost/graph/make_biconnected_planar.hpp) | Augment to biconnected | Preprocessing for planar drawing | [`doc/make_biconnected_planar.html`](../../../doc/make_biconnected_planar.html) |
| [`make_maximal_planar.hpp`](../../../include/boost/graph/make_maximal_planar.hpp) | Augment to maximal planar | Preprocessing for Chrobak-Payne | [`doc/make_maximal_planar.html`](../../../doc/make_maximal_planar.html) |

---

## Graph Layout

Force-directed and other layout algorithms. They assign 2D (or higher-dim)
positions to vertices. Position types use the `PositionMap` property-map
interface; topology (bounding space) is provided via `topology.hpp`.

| Header | Algorithm | Doc |
|--------|-----------|-----|
| [`random_layout.hpp`](../../../include/boost/graph/random_layout.hpp) | Random placement | [`doc/random_layout.html`](../../../doc/random_layout.html) |
| [`circle_layout.hpp`](../../../include/boost/graph/circle_layout.hpp) | Circular layout | [`doc/circle_layout.html`](../../../doc/circle_layout.html) |
| [`kamada_kawai_spring_layout.hpp`](../../../include/boost/graph/kamada_kawai_spring_layout.hpp) | Kamada-Kawai spring layout | [`doc/kamada_kawai_spring_layout.html`](../../../doc/kamada_kawai_spring_layout.html) |
| [`fruchterman_reingold.hpp`](../../../include/boost/graph/fruchterman_reingold.hpp) | Fruchterman-Reingold force-directed | [`doc/fruchterman_reingold.html`](../../../doc/fruchterman_reingold.html) |
| [`gursoy_atun_layout.hpp`](../../../include/boost/graph/gursoy_atun_layout.hpp) | Gursoy-Atun SOM layout | [`doc/gursoy_atun_layout.html`](../../../doc/gursoy_atun_layout.html) |

---

## Centrality and Clustering

Algorithms that measure vertex/edge importance or group vertices into
communities.

| Header | Algorithm | Notes | Doc |
|--------|-----------|-------|-----|
| [`betweenness_centrality.hpp`](../../../include/boost/graph/betweenness_centrality.hpp) | Betweenness centrality | Vertex and edge betweenness; Brandes algorithm | [`doc/betweenness_centrality.html`](../../../doc/betweenness_centrality.html) |
| [`bc_clustering.hpp`](../../../include/boost/graph/bc_clustering.hpp) | Betweenness-centrality clustering | Iterative edge removal | [`doc/bc_clustering.html`](../../../doc/bc_clustering.html) |
| [`closeness_centrality.hpp`](../../../include/boost/graph/closeness_centrality.hpp) | Closeness centrality | Requires precomputed distances | — |
| [`degree_centrality.hpp`](../../../include/boost/graph/degree_centrality.hpp) | Degree centrality | Normalized degree | — |
| [`eccentricity.hpp`](../../../include/boost/graph/eccentricity.hpp) | Eccentricity / diameter / radius | Requires all-pairs distances | — |
| [`clustering_coefficient.hpp`](../../../include/boost/graph/clustering_coefficient.hpp) | Clustering coefficient | Local and mean clustering coefficient | — |
| [`core_numbers.hpp`](../../../include/boost/graph/core_numbers.hpp) | Core decomposition (k-core) | — | — |
| [`page_rank.hpp`](../../../include/boost/graph/page_rank.hpp) | PageRank | Iterative; parameterized on damping factor | — |
| [`louvain_clustering.hpp`](../../../include/boost/graph/louvain_clustering.hpp) | Louvain community detection | Modularity-maximizing partitioning | [`doc/louvain_clustering.html`](../../../doc/louvain_clustering.html) |

---

## Cycles, Cliques, and Subgraph Isomorphism

Enumeration algorithms that find cycles, cliques, or subgraph matches.

| Header | Algorithm | Notes | Doc |
|--------|-----------|-------|-----|
| [`tiernan_all_cycles.hpp`](../../../include/boost/graph/tiernan_all_cycles.hpp) | All simple cycles (Tiernan) | Visitor-based enumeration | — |
| [`hawick_circuits.hpp`](../../../include/boost/graph/hawick_circuits.hpp) | All circuits (Hawick-James) | More efficient than Tiernan for directed graphs | [`doc/hawick_circuits.html`](../../../doc/hawick_circuits.html) |
| [`howard_cycle_ratio.hpp`](../../../include/boost/graph/howard_cycle_ratio.hpp) | Min/max cycle ratio | Howard's policy-iteration algorithm | [`doc/howard_cycle_ratio.html`](../../../doc/howard_cycle_ratio.html) |
| [`bron_kerbosch_all_cliques.hpp`](../../../include/boost/graph/bron_kerbosch_all_cliques.hpp) | All cliques (Bron-Kerbosch) | Visitor-based; reports each maximal clique | — |
| [`isomorphism.hpp`](../../../include/boost/graph/isomorphism.hpp) | Graph isomorphism | Ullmann backtracking | [`doc/isomorphism.html`](../../../doc/isomorphism.html) |
| [`vf2_sub_graph_iso.hpp`](../../../include/boost/graph/vf2_sub_graph_iso.hpp) | VF2 subgraph isomorphism | Faster than Ullmann for large graphs; also graph isomorphism | [`doc/vf2_sub_graph_iso.html`](../../../doc/vf2_sub_graph_iso.html) |
| [`mcgregor_common_subgraphs.hpp`](../../../include/boost/graph/mcgregor_common_subgraphs.hpp) | McGregor common subgraphs | Maximum common subgraph enumeration | [`doc/mcgregor_common_subgraphs.html`](../../../doc/mcgregor_common_subgraphs.html) |

---

## Graph Generators

Algorithms that construct synthetic graphs with specified statistical or
structural properties.

| Header | Generator | Doc |
|--------|-----------|-----|
| [`erdos_renyi_generator.hpp`](../../../include/boost/graph/erdos_renyi_generator.hpp) | Erdős-Rényi random graph | [`doc/erdos_renyi_generator.html`](../../../doc/erdos_renyi_generator.html) |
| [`plod_generator.hpp`](../../../include/boost/graph/plod_generator.hpp) | Power-law out-degree (PLOD) | [`doc/plod_generator.html`](../../../doc/plod_generator.html) |
| [`rmat_graph_generator.hpp`](../../../include/boost/graph/rmat_graph_generator.hpp) | R-MAT recursive matrix | — |
| [`mesh_graph_generator.hpp`](../../../include/boost/graph/mesh_graph_generator.hpp) | Mesh / grid | — |
| [`small_world_generator.hpp`](../../../include/boost/graph/small_world_generator.hpp) | Watts-Strogatz small world | [`doc/small_world_generator.html`](../../../doc/small_world_generator.html) |
| [`ssca_graph_generator.hpp`](../../../include/boost/graph/ssca_graph_generator.hpp) | SSCA benchmark graph | — |
| [`random.hpp`](../../../include/boost/graph/random.hpp) | Random graph utilities | Random spanning trees, random vertex/edge selection | — |

---

## Graph Utility and Transformation

Algorithms that copy, transform, or derive one graph from another.

| Header | Purpose | Doc |
|--------|---------|-----|
| [`copy.hpp`](../../../include/boost/graph/copy.hpp) | Graph copy with property-map translation | — |
| [`transpose_graph.hpp`](../../../include/boost/graph/transpose_graph.hpp) | Produce an edge-reversed copy | [`doc/transpose_graph.html`](../../../doc/transpose_graph.html) |
| [`transitive_closure.hpp`](../../../include/boost/graph/transitive_closure.hpp) | Transitive closure | [`doc/transitive_closure.html`](../../../doc/transitive_closure.html) |
| [`transitive_reduction.hpp`](../../../include/boost/graph/transitive_reduction.hpp) | Transitive reduction | — |
| [`create_condensation_graph.hpp`](../../../include/boost/graph/create_condensation_graph.hpp) | SCC condensation graph | — |
| [`dominator_tree.hpp`](../../../include/boost/graph/dominator_tree.hpp) | Dominator tree (Lengauer-Tarjan) | — |
| [`stoer_wagner_min_cut.hpp`](../../../include/boost/graph/stoer_wagner_min_cut.hpp) | Stoer-Wagner global min cut | [`doc/stoer_wagner_min_cut.html`](../../../doc/stoer_wagner_min_cut.html) |
| [`metric_tsp_approx.hpp`](../../../include/boost/graph/metric_tsp_approx.hpp) | Metric TSP approximation | [`doc/metric_tsp_approx.html`](../../../doc/metric_tsp_approx.html) |
| [`loop_erased_random_walk.hpp`](../../../include/boost/graph/loop_erased_random_walk.hpp) | Loop-erased random walk | — |

---

## Where to go next

| You want to | Read |
|-------------|------|
| Understand how algorithms accept parameters | [architecture.md](architecture.md) (§ Named parameters, § Visitor pattern) |
| Choose a graph type to run algorithms on | [graph-types.md](graph-types.md) |
| Understand algorithm concept requirements | [concepts.md](concepts.md) |
| Add a new algorithm | [extending.md](extending.md) |
