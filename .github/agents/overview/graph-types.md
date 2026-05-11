# Graph Types

Back to [overview.md](../overview.md)

BGL provides a range of built-in graph data structures, plus adapter layers
for third-party types. All are models of one or more graph concepts and work
with any algorithm that requires those concepts. Choosing the right type is
mainly a trade-off between mutability, memory layout, descriptor stability,
and traversal cost.

---

## Primary general-purpose graphs

### `adjacency_list`

Header: [`include/boost/graph/adjacency_list.hpp`](../../../include/boost/graph/adjacency_list.hpp)  
Doc: [`doc/adjacency_list.html`](../../../doc/adjacency_list.html)

The primary workhorse. Parameterized on container selectors for the edge list
(`OutEdgeList`), the vertex list (`VertexList`), the directedness tag, and
interior vertex/edge properties. The selector choices (`vecS`, `listS`,
`setS`, `hash_setS`, `multisetS`) determine complexity guarantees, descriptor
stability, and whether parallel edges are allowed.

| Selector | Vertex container | Edge container |
|----------|-----------------|----------------|
| `vecS` | `std::vector` — O(1) index access, descriptors invalidated by `add_vertex`/`remove_vertex` | `std::vector` — fastest iteration, allows parallel edges |
| `listS` | `std::list` — stable descriptors, no index-based access | `std::list` — stable descriptors |
| `setS` | `std::set` — no parallel edges, O(log n) edge lookup | — |

Directedness is chosen with `directedS`, `undirectedS`, or
`bidirectionalS` (the last adds `in_edges` support at the cost of extra
storage).

### `adjacency_matrix`

Header: [`include/boost/graph/adjacency_matrix.hpp`](../../../include/boost/graph/adjacency_matrix.hpp)  
Doc: [`doc/adjacency_matrix.html`](../../../doc/adjacency_matrix.html)

A dense bit-matrix–backed graph. O(1) `edge(u, v, g)` lookup (models
`AdjacencyMatrix`). Best for small, dense graphs where edge-existence
queries dominate and memory proportional to V² is acceptable. Vertex count
is fixed at construction.

### `compressed_sparse_row_graph`

Header: [`include/boost/graph/compressed_sparse_row_graph.hpp`](../../../include/boost/graph/compressed_sparse_row_graph.hpp)  
Doc: [`doc/compressed_sparse_row.html`](../../../doc/compressed_sparse_row.html)

A read-optimized CSR (compressed sparse row) format. Very low memory
overhead, excellent cache locality for out-edge traversal. Immutable after
construction — no `add_vertex` or `add_edge`. Best for large static graphs
on which many traversal passes are performed (e.g. BFS over a web graph).
Models `IncidenceGraph`, `AdjacencyGraph`, and `VertexListGraph`.

---

## Convenience wrappers

### `directed_graph` / `undirected_graph`

Headers: [`include/boost/graph/directed_graph.hpp`](../../../include/boost/graph/directed_graph.hpp),
[`include/boost/graph/undirected_graph.hpp`](../../../include/boost/graph/undirected_graph.hpp)  
Docs: [`doc/directed_graph.html`](../../../doc/directed_graph.html),
[`doc/undirected_graph.html`](../../../doc/undirected_graph.html)

Thin wrappers around `adjacency_list` with sane defaults (stable
`listS` vertex storage, `bidirectionalS` or `undirectedS` respectively,
bundled-property support). A good first choice when you want a mutable
graph and do not need to tune the underlying containers.

---

## View / adapter graphs

These types wrap an existing graph and present a transformed view without
copying the data.

### `filtered_graph`

Header: [`include/boost/graph/filtered_graph.hpp`](../../../include/boost/graph/filtered_graph.hpp)  
Doc: [`doc/filtered_graph.html`](../../../doc/filtered_graph.html)

Presents a subset of an existing graph's vertices and/or edges as defined
by predicate function objects. The underlying graph is not modified; the
filter is applied lazily during traversal. Useful for running algorithms
on subgraphs (e.g. the residual graph during flow algorithms).

### `reverse_graph`

Header: [`include/boost/graph/reverse_graph.hpp`](../../../include/boost/graph/reverse_graph.hpp)  
Doc: [`doc/reverse_graph.html`](../../../doc/reverse_graph.html)

Presents the edge-reversed view of a directed graph. `out_edges(u, rg)`
returns the in-edges of `u` in the original. Zero-copy; used internally by
algorithms that require traversal in the reverse direction (e.g. computing
strongly connected components).

### `subgraph`

Header: [`include/boost/graph/subgraph.hpp`](../../../include/boost/graph/subgraph.hpp)  
Doc: [`doc/subgraph.html`](../../../doc/subgraph.html)

A hierarchical subgraph that shares vertex/edge property storage with its
parent. Mutations propagate to the parent; the parent's global vertex/edge
descriptors are accessible from a subgraph. Useful when algorithms must
work on induced subgraphs that remain synchronized with the main graph.

### `labeled_graph`

Header: [`include/boost/graph/labeled_graph.hpp`](../../../include/boost/graph/labeled_graph.hpp)  
Doc: not yet available in this checkout.

A graph wrapper that adds a bidirectional mapping from arbitrary label
values (any hashable or ordered type) to vertex descriptors, so vertices
can be looked up by label in O(1) or O(log n). The underlying graph type
is a template parameter.

---

## Implicit / generated graphs

### `grid_graph`

Header: [`include/boost/graph/grid_graph.hpp`](../../../include/boost/graph/grid_graph.hpp)  
Doc: [`doc/grid_graph.html`](../../../doc/grid_graph.html)

An N-dimensional implicit grid graph. Vertices and edges are computed on
the fly from integer coordinates — no storage for edges or an adjacency
structure. Dimensions, wrapping per-axis, and total size are compile-time
or run-time parameters. Models `IncidenceGraph`, `AdjacencyGraph`,
`VertexListGraph`, `EdgeListGraph`, and `AdjacencyMatrix`.

---

## Edge-sequence graph

### `edge_list`

Header: [`include/boost/graph/edge_list.hpp`](../../../include/boost/graph/edge_list.hpp)  
Doc: [`doc/edge_list.html`](../../../doc/edge_list.html)

A lightweight adapter that wraps a pair of iterators over edge objects and
presents them as a graph. Models `EdgeListGraph` only — no vertex traversal
or adjacency. Useful for algorithms that iterate edges exactly once (e.g.
Kruskal MST from an edge list).

---

## Tree adapter

### `graph_as_tree`

Header: [`include/boost/graph/graph_as_tree.hpp`](../../../include/boost/graph/graph_as_tree.hpp)  
Doc: not yet available in this checkout.

Wraps a BGL graph with a designated root vertex and a predecessor/parent
property map to expose a tree interface (parent, children traversal).

---

## Third-party graph adapters

These headers adapt external graph representations to the BGL interface.
They are provided for interoperability; no BGL-internal algorithms depend
on them.

| Header | Adapts |
|--------|--------|
| [`vector_as_graph.hpp`](../../../include/boost/graph/vector_as_graph.hpp) | `std::vector<std::list<int>>` as an `IncidenceGraph` / `VertexListGraph` |
| [`matrix_as_graph.hpp`](../../../include/boost/graph/matrix_as_graph.hpp) | A boolean matrix type as a `BidirectionalGraph` |
| [`stanford_graph.hpp`](../../../include/boost/graph/stanford_graph.hpp) | Stanford GraphBase `Graph*` as a BGL graph |
| [`leda_graph.hpp`](../../../include/boost/graph/leda_graph.hpp) | LEDA `GRAPH<V,E>` as a BGL graph |

---

## Choosing a graph type

| Requirement | Recommended type |
|-------------|-----------------|
| General-purpose mutable graph | `directed_graph` / `undirected_graph` |
| Tunable containers / parallel edges | `adjacency_list` |
| Dense graph, O(1) edge lookup | `adjacency_matrix` |
| Large static graph, fast traversal | `compressed_sparse_row_graph` |
| Implicit regular structure | `grid_graph` |
| Subgraph synchronized with parent | `subgraph` |
| Filter existing graph | `filtered_graph` |
| Traverse reversed edges | `reverse_graph` |
| Find vertices by label | `labeled_graph` |
| Algorithms over an edge list only | `edge_list` |

---

## Where to go next

| You want to | Read |
|-------------|------|
| Understand the concepts these types model | [concepts.md](concepts.md) |
| See the generic programming model behind the types | [architecture.md](architecture.md) |
| Find algorithms that operate on these types | [algorithms.md](algorithms.md) |
