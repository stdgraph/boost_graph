# Architecture

Back to [overview.md](../overview.md)

This document explains how Boost.Graph is put together: the design idioms
that recur across every algorithm and graph type, and the small set of core
abstractions that bind them. It is intended to give an agent or new
contributor enough mental model to read any header in
[`include/boost/graph/`](../../../include/boost/graph/) and understand its
role.

The detailed concept hierarchy lives in [concepts.md](concepts.md); this
document only sketches it.

---

## 1. Generic programming model

BGL is built on the same generic-programming idiom as the C++ Standard
Library:

- **Algorithms are templates** parameterized on the graph type, the
  iterator/property-map types they operate on, and policy parameters
  (visitors, comparators, color maps).
- **Concepts** specify the syntactic and semantic requirements that a graph
  type must satisfy to be usable with a given algorithm.
- **Models** are the concrete types (e.g. `adjacency_list`) that satisfy
  one or more concepts.
- **User types** can be made into models without touching the BGL source by
  specializing `graph_traits` and providing free functions matching the
  required signatures.

Authoritative reference: [`doc/constructing_algorithms.html`](../../../doc/constructing_algorithms.html).

The practical consequence: the same Dijkstra implementation runs against
`adjacency_list`, `compressed_sparse_row_graph`, `grid_graph`, or any
user-defined type that models `IncidenceGraph` and `VertexListGraph`.

---

## 2. `graph_traits<G>` — the universal adapter

[`graph_traits.hpp`](../../../include/boost/graph/graph_traits.hpp) defines
the trait class that every algorithm uses to discover the associated types
of a graph:

| Trait member | Role |
|--------------|------|
| `vertex_descriptor` | Opaque handle for a vertex |
| `edge_descriptor` | Opaque handle for an edge |
| `vertex_iterator`, `edge_iterator` | Iteration over all vertices / edges |
| `out_edge_iterator`, `in_edge_iterator` | Per-vertex edge iteration |
| `adjacency_iterator` | Per-vertex neighbor iteration |
| `directed_category` | `directed_tag`, `undirected_tag`, or `bidirectional_tag` |
| `edge_parallel_category` | Whether parallel edges are allowed |
| `traversal_category` | Which traversal interfaces the graph supports |
| `vertices_size_type`, `edges_size_type`, `degree_size_type` | Unsigned size types |

`graph_traits` is the single point of customization. Adapting a foreign
graph type to BGL means providing a `graph_traits` specialization plus the
free functions (`vertices`, `edges`, `out_edges`, `source`, `target`, etc.)
that the relevant concept requires.

Three category tags drive overload resolution:

- `directed_tag`, `undirected_tag`, `bidirectional_tag` (the last refines
  `directed_tag`).
- The `traversal_category` tag composes flags such as
  `incidence_graph_tag`, `bidirectional_graph_tag`, `adjacency_graph_tag`,
  `vertex_list_graph_tag`, `edge_list_graph_tag`, `adjacency_matrix_tag`.

---

## 3. Concept hierarchy (sketch)

Concepts are declared in
[`graph_concepts.hpp`](../../../include/boost/graph/graph_concepts.hpp)
using the Boost.ConceptCheck library and live in `boost::concepts`. The
core refinement structure:

```
Graph
├── IncidenceGraph
│   └── BidirectionalGraph
├── AdjacencyGraph
├── VertexListGraph
│   └── VertexAndEdgeListGraph
├── EdgeListGraph
│   └── VertexAndEdgeListGraph
└── AdjacencyMatrix

EdgeMutableGraph ──┐
                   ├── MutableGraph
VertexMutableGraph ┘
                       └── MutableIncidenceGraph
                              └── MutableBidirectionalGraph
                       └── MutableEdgeListGraph

ReadablePropertyGraph
└── PropertyGraph
       └── LvaluePropertyGraph
```

Plus the index/utility concepts: `VertexIndexGraph`, `EdgeIndexGraph`,
`ColorValue`, `BasicMatrix`, `NumericValue`, `DegreeMeasure`,
`DistanceMeasure`.

For the full list of refinements, required expressions, and complexity
guarantees, see [concepts.md](concepts.md) and
[`doc/graph_concepts.html`](../../../doc/graph_concepts.html).

Concept archetypes for testing live in
[`graph_archetypes.hpp`](../../../include/boost/graph/graph_archetypes.hpp).

---

## 4. Property maps — interior vs exterior

Algorithms read and write per-vertex and per-edge data through the
**property map** abstraction (defined by Boost.PropertyMap, used here).
There are two storage strategies:

- **Interior properties** are stored inside the graph itself. They are
  declared via the `Properties` template parameters of `adjacency_list`
  and accessed with `get(property_tag, g)` returning a property map. The
  built-in tags (`vertex_index_t`, `vertex_color_t`, `edge_weight_t`,
  `vertex_distance_t`, etc.) live in
  [`properties.hpp`](../../../include/boost/graph/properties.hpp).
- **Exterior properties** are stored outside the graph (typically a
  `std::vector` indexed by vertex/edge index). The user wraps the storage
  in an `iterator_property_map` and passes it to the algorithm.
  Helpers for typical exterior-property setups live in
  [`exterior_property.hpp`](../../../include/boost/graph/exterior_property.hpp).

Algorithms accept either kind transparently because both expose the same
property-map interface. The choice is a memory/performance trade-off, not
an API distinction.

Compact specialized property maps for color values are provided by
[`one_bit_color_map.hpp`](../../../include/boost/graph/one_bit_color_map.hpp)
and
[`two_bit_color_map.hpp`](../../../include/boost/graph/two_bit_color_map.hpp).

---

## 5. Named parameters

Most BGL algorithms accept many optional parameters (visitors, color maps,
weight maps, distance maps, predecessor maps, comparators, …). Rather than
overloading on every combination, BGL provides a **named-parameter**
mechanism in
[`named_function_params.hpp`](../../../include/boost/graph/named_function_params.hpp).

Usage idiom:

```cpp
dijkstra_shortest_paths(g, s,
    weight_map(get(edge_weight, g))
        .predecessor_map(pred)
        .distance_map(dist));
```

Each call to a named-parameter setter (`weight_map`, `predecessor_map`,
`distance_map`, `visitor`, …) returns a `bgl_named_params` chain that the
algorithm unpacks via `lookup_named_param`. Parameters not supplied by the
caller fall back to defaults (often constructed from `vertex_index`).

The full set of named parameters is enumerated by the
`BOOST_BGL_DECLARE_NAMED_PARAMS` macro near the top of
`named_function_params.hpp`. See
[`doc/bgl_named_params.html`](../../../doc/bgl_named_params.html) for the
user-facing reference.

---

## 6. Visitor pattern — the event-point model

BGL search and traversal algorithms are instrumented with **event points**.
A visitor is an object that responds to one or more events; the algorithm
calls the visitor at each event point during traversal.

The event tags are defined in
[`visitors.hpp`](../../../include/boost/graph/visitors.hpp) and include
(non-exhaustive):

- `on_initialize_vertex`, `on_start_vertex`
- `on_discover_vertex`, `on_examine_vertex`, `on_finish_vertex`
- `on_examine_edge`, `on_tree_edge`, `on_non_tree_edge`
- `on_back_edge`, `on_forward_or_cross_edge`, `on_finish_edge`
- `on_gray_target`, `on_black_target`
- `on_edge_relaxed`, `on_edge_not_relaxed`,
  `on_edge_minimized`, `on_edge_not_minimized`

Each algorithm exposes a per-algorithm visitor concept that documents which
events it actually fires:

| Visitor concept | Header | Doc |
|-----------------|--------|-----|
| `BFSVisitor` | `breadth_first_search.hpp` | `doc/BFSVisitor.html` |
| `DFSVisitor` | `depth_first_search.hpp` | `doc/DFSVisitor.html` |
| `DijkstraVisitor` | `dijkstra_shortest_paths.hpp` | `doc/DijkstraVisitor.html` |
| `BellmanFordVisitor` | `bellman_ford_shortest_paths.hpp` | `doc/BellmanFordVisitor.html` |
| `AStarVisitor` | `astar_search.hpp` | `doc/AStarVisitor.html` |
| `EventVisitor` | `visitors.hpp` | `doc/EventVisitor.html` |

Reusable visitor adaptors (predecessor recorder, distance recorder,
property writer, time stamper, etc.) are provided directly in
`visitors.hpp`. Multiple visitors can be combined into a single
`EventVisitorList`.

Authoritative reference:
[`doc/visitor_concepts.html`](../../../doc/visitor_concepts.html).

---

## 7. Graph selectors

`adjacency_list` is parameterized on **selector tags** that pick the
underlying STL containers and the directedness:

| Selector | Role | Examples |
|----------|------|----------|
| Directedness | Picks `directed_category` | `directedS`, `undirectedS`, `bidirectionalS` |
| Vertex container | Picks vertex storage | `vecS`, `listS`, `setS`, `hash_setS`, … |
| Edge container | Picks per-vertex edge storage | `vecS`, `listS`, `setS`, … |

The directedness tags are defined in
[`graph_selectors.hpp`](../../../include/boost/graph/graph_selectors.hpp);
the container selectors are defined alongside `adjacency_list`. The
selector you pick determines complexity guarantees (e.g. `vecS` vertex
storage means O(1) `vertex(i, g)` but invalidates descriptors on
`remove_vertex`, while `listS` is the opposite trade-off).

`directed_graph` and `undirected_graph` are convenience wrappers around
`adjacency_list` with a fixed selector configuration.

---

## How the pieces fit together

A typical algorithm call exercises every layer above:

```cpp
adjacency_list<vecS, vecS, directedS,
               no_property,
               property<edge_weight_t, double>> g;     // selectors + interior property
// ... populate g ...
std::vector<vertex_descriptor> pred(num_vertices(g));  // exterior property storage
std::vector<double>            dist(num_vertices(g));

dijkstra_shortest_paths(g, source,
    predecessor_map(make_iterator_property_map(pred.begin(), get(vertex_index, g)))
        .distance_map(make_iterator_property_map(dist.begin(), get(vertex_index, g)))
        .visitor(make_dijkstra_visitor(record_predecessors(/*...*/, on_edge_relaxed()))));
```

This single call uses:

- `graph_traits` (to resolve `vertex_descriptor` and the iterator types),
- the concept system (Dijkstra requires `IncidenceGraph` + `VertexListGraph`),
- interior properties (the edge weight) and exterior properties
  (`pred`, `dist`),
- named parameters (`predecessor_map`, `distance_map`, `visitor`),
- the visitor event-point model (`on_edge_relaxed`),
- selectors (`vecS`, `directedS` chose the storage and directedness).

Every other BGL algorithm follows the same pattern with different concept
requirements, default named parameters, and event points.

---

## Where to go next

| You want to | Read |
|-------------|------|
| See the full concept refinement diagram | [concepts.md](concepts.md) |
| Choose a graph data structure | [graph-types.md](graph-types.md) |
| Find an algorithm by family | [algorithms.md](algorithms.md) |
| Add a new algorithm/graph type/property map | [extending.md](extending.md) |
| Look up a term | [glossary.md](glossary.md) |
