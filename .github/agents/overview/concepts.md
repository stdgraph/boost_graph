# Concepts

Back to [overview.md](../overview.md)

BGL is built on a system of *graph concepts* — sets of syntactic and
semantic requirements that any graph type must satisfy to be used with
particular algorithms. Concepts are declared in
[`graph_concepts.hpp`](../../../include/boost/graph/graph_concepts.hpp)
using Boost.ConceptCheck, and minimal *archetypes* (the smallest possible
models) are provided in
[`graph_archetypes.hpp`](../../../include/boost/graph/graph_archetypes.hpp).

For the high-level discussion of the generic-programming model and where
concepts fit in the algorithm/data-structure design, see
[architecture.md](architecture.md). The full prose definitions and complexity
guarantees live in [`doc/graph_concepts.html`](../../../doc/graph_concepts.html).
This document is the structured catalogue: refinement hierarchy, required
expressions, traversal-category tag, and complexity expectations.

---

## 1. The refinement hierarchy

A concept *X refines* concept *Y* (notation `X : Y`) if every model of *X*
is also a model of *Y*. The arrows below all point from refined-from to
refining concept (read upward to find weaker requirements).

### 1.1 Structure-access concepts

```
                       Graph
                         |
        +----------------+----------------+
        |                |                |
   IncidenceGraph   AdjacencyGraph   VertexListGraph   EdgeListGraph
        |                                              /
   BidirectionalGraph                                 /
                                                     /
                       VertexAndEdgeListGraph  =  VertexListGraph + EdgeListGraph

   AdjacencyMatrix   :  Graph        (random-access edge(u,v,g))
```

### 1.2 Mutability concepts

```
   EdgeMutableGraph        VertexMutableGraph
        \                       /
         +---------+-----------+
                   |
              MutableGraph
                   |
        +----------+----------+
        |                     |
   MutableIncidenceGraph   MutableEdgeListGraph
        |
   MutableBidirectionalGraph

   VertexMutablePropertyGraph : VertexMutableGraph
   EdgeMutablePropertyGraph   : EdgeMutableGraph
```

### 1.3 Property and index concepts

```
   ReadablePropertyGraph
        |
        +-----------+
        |           |
   PropertyGraph   LvaluePropertyGraph

   VertexIndexGraph     (get(vertex_index, g))
   EdgeIndexGraph       (get(edge_index,   g))
```

### 1.4 Auxiliary value concepts

`ColorValue`, `BasicMatrix`, `NumericValue`, `DegreeMeasure`,
`DistanceMeasure` are not graph concepts but appear in algorithm
requirements.

---

## 2. Concept reference

Each entry below lists: refines, traversal-category tag (where
applicable), required expressions, and the expected complexity. Algorithms
specify their required concept(s); a graph type must model all of them to
be passed to a given algorithm.

### 2.1 `Graph<G>`

Refines: *(none)*
Required types (via `graph_traits<G>`):
`vertex_descriptor`, `edge_descriptor`, `directed_category`,
`edge_parallel_category`, `traversal_category`.

Required expressions: descriptors must be `DefaultConstructible`,
`EqualityComparable`, `Assignable`. No traversal operations are required
at this level.

### 2.2 `IncidenceGraph<G> : Graph<G>`

Traversal tag: convertible to `incidence_graph_tag`.

| Expression | Returns | Complexity |
|------------|---------|------------|
| `out_edges(u, g)` | `pair<out_edge_iterator, out_edge_iterator>` | amortized constant |
| `out_degree(u, g)` | `degree_size_type` | constant |
| `source(e, g)` | `vertex_descriptor` | constant |
| `target(e, g)` | `vertex_descriptor` | constant |

`out_edge_iterator` must be a `MultiPassInputIterator`.

### 2.3 `BidirectionalGraph<G> : IncidenceGraph<G>`

Traversal tag: convertible to `bidirectional_graph_tag`.

| Expression | Returns | Complexity |
|------------|---------|------------|
| `in_edges(v, g)` | `pair<in_edge_iterator, in_edge_iterator>` | amortized constant |
| `in_degree(v, g)` | `degree_size_type` | constant |
| `degree(v, g)` | `degree_size_type` | constant |

### 2.4 `AdjacencyGraph<G> : Graph<G>`

Traversal tag: convertible to `adjacency_graph_tag`.

| Expression | Returns | Complexity |
|------------|---------|------------|
| `adjacent_vertices(u, g)` | `pair<adjacency_iterator, adjacency_iterator>` | amortized constant |

### 2.5 `VertexListGraph<G> : Graph<G>`

Traversal tag: convertible to `vertex_list_graph_tag`.

| Expression | Returns | Complexity |
|------------|---------|------------|
| `vertices(g)` | `pair<vertex_iterator, vertex_iterator>` | constant |
| `num_vertices(g)` | `vertices_size_type` | constant |

### 2.6 `EdgeListGraph<G> : Graph<G>`

Traversal tag: convertible to `edge_list_graph_tag`.

| Expression | Returns | Complexity |
|------------|---------|------------|
| `edges(g)` | `pair<edge_iterator, edge_iterator>` | constant |
| `num_edges(g)` | `edges_size_type` | constant |
| `source(e, g)`, `target(e, g)` | `vertex_descriptor` | constant |

### 2.7 `VertexAndEdgeListGraph<G>`

Refines both `VertexListGraph<G>` and `EdgeListGraph<G>`. Adds no new
expressions.

### 2.8 `AdjacencyMatrix<G> : Graph<G>`

| Expression | Returns | Complexity |
|------------|---------|------------|
| `edge(u, v, g)` | `pair<edge_descriptor, bool>` | constant |

The defining property: edge lookup between any two vertices is O(1). Only
[`adjacency_matrix`](../../../include/boost/graph/adjacency_matrix.hpp)
satisfies this in BGL.

### 2.9 Mutability concepts

`EdgeMutableGraph<G>`:

| Expression | Effect |
|------------|--------|
| `add_edge(u, v, g)` | returns `pair<edge_descriptor, bool>` |
| `remove_edge(u, v, g)` | removes any edge between `u` and `v` |
| `remove_edge(e, g)` | removes the specified edge |
| `clear_vertex(v, g)` | removes all edges incident on `v` |

`VertexMutableGraph<G>`:

| Expression | Effect |
|------------|--------|
| `add_vertex(g)` | returns `vertex_descriptor` |
| `remove_vertex(v, g)` | removes `v` (`v` must have no incident edges) |

`MutableGraph<G>` = `EdgeMutableGraph<G>` ∧ `VertexMutableGraph<G>`.

`MutableIncidenceGraph<G> : MutableGraph<G>` adds:
`remove_edge(iter, g)`, `remove_out_edge_if(u, predicate, g)`.

`MutableBidirectionalGraph<G> : MutableIncidenceGraph<G>` adds:
`remove_in_edge_if(u, predicate, g)`.

`MutableEdgeListGraph<G> : EdgeMutableGraph<G>` adds:
`remove_edge_if(predicate, g)`.

`VertexMutablePropertyGraph<G> : VertexMutableGraph<G>` adds:
`add_vertex(vp, g)` where `vp` is a `vertex_property_type<G>::type`.

`EdgeMutablePropertyGraph<G> : EdgeMutableGraph<G>` adds:
`add_edge(u, v, ep, g)` where `ep` is an `edge_property_type<G>::type`.

### 2.10 Property-graph concepts

`ReadablePropertyGraph<G, X, Property>`:

| Expression | Effect |
|------------|--------|
| `get(Property(), g)` | returns a const property map |
| `get(Property(), g, x)` | returns the property value at descriptor `x` |

`PropertyGraph<G, X, Property> : ReadablePropertyGraph<G, X, Property>` adds:

| Expression | Effect |
|------------|--------|
| `get(Property(), g)` | returns a read-write property map |
| `put(Property(), g, x, value)` | writes a property value |

`LvaluePropertyGraph<G, X, Property> : ReadablePropertyGraph<G, X, Property>`
strengthens to require an `LvaluePropertyMap` — `get`/`put` resolve to a
reference into storage.

### 2.11 Index concepts

`VertexIndexGraph<G>`: `get(vertex_index, g)` returns a property map from
vertex descriptors to a contiguous unsigned-integer index.
`renumber_vertex_indices(g)` reassigns indices after structural change.

`EdgeIndexGraph<G>`: same shape for edges.

These concepts are *semantic*: any graph that exposes a `vertex_index` /
`edge_index` property map is implicitly a model. They underpin algorithms
that use exterior property maps keyed on vector-backed storage.

### 2.12 Auxiliary value concepts

`ColorValue<C>`: `EqualityComparable` and `DefaultConstructible` with
`color_traits<C>::white()`, `gray()`, `black()`. Used by traversal
algorithms for the color map.

`BasicMatrix<M, I, V>`: `M` supports `A[i][j]` returning a `V&` and
`const V&` for const matrices. Used by Floyd-Warshall and similar.

`NumericValue<N>`: `DefaultConstructible`, `CopyConstructible`, with
`numeric_values<N>::zero()` and `infinity()`.

`DegreeMeasure<Measure, Graph>` and `DistanceMeasure<Measure, Graph>` are
function-object concepts used by centrality and ordering algorithms.

---

## 3. Traversal-category tags

A graph's `graph_traits<G>::traversal_category` is a tag type that may be
*convertible to* one or more of the following empty tag classes (defined in
[`graph_traits.hpp`](../../../include/boost/graph/graph_traits.hpp)). The
algorithm checks the convertibility to verify the concept at compile time.

| Tag | Concept it asserts |
|-----|--------------------|
| `incidence_graph_tag` | `IncidenceGraph` |
| `bidirectional_graph_tag` | `BidirectionalGraph` |
| `adjacency_graph_tag` | `AdjacencyGraph` |
| `vertex_list_graph_tag` | `VertexListGraph` |
| `edge_list_graph_tag` | `EdgeListGraph` |
| `adjacency_matrix_tag` | `AdjacencyMatrix` |

A graph type combines these by deriving its `traversal_category` from
multiple base tags. `adjacency_list` for example yields
`bidirectional_graph_tag, adjacency_graph_tag, vertex_list_graph_tag,
edge_list_graph_tag` (when configured for bidirectionality).

---

## 4. Concept archetypes

Archetypes are minimal-implementation classes that model exactly one
concept and nothing more. They serve two purposes:

1. **Algorithm authors** instantiate their algorithm against an archetype
   to confirm they are not silently relying on requirements outside the
   advertised concept set.
2. **Documentation** — an archetype is the executable spelling of a
   concept's required expressions.

The archetypes provided in
[`graph_archetypes.hpp`](../../../include/boost/graph/graph_archetypes.hpp):

| Archetype | Concept it models |
|-----------|-------------------|
| `incidence_graph_archetype<V, D, P>` | `IncidenceGraph` |
| `adjacency_graph_archetype<V, D, P>` | `AdjacencyGraph` |
| `vertex_list_graph_archetype<V, D, P>` | `IncidenceGraph` + `AdjacencyGraph` + `VertexListGraph` |
| `property_graph_archetype<G, Property, ValueArch>` | adds `PropertyGraph` to a base archetype |
| `color_value_archetype` | `ColorValue` |
| `buffer_archetype<T>` | `Buffer` (for queue-like inputs to BFS-style algorithms) |

Template parameters `V`, `D`, `P` select vertex descriptor type,
`directed_category`, and `edge_parallel_category`.

---

## 5. Concept usage in algorithms

Each algorithm header begins with `BOOST_CONCEPT_ASSERT` invocations on
the graph and property-map types it receives. A representative pattern from
`dijkstra_shortest_paths.hpp`:

```cpp
function_requires< VertexListGraphConcept<Graph> >();
function_requires< IncidenceGraphConcept<Graph> >();
function_requires< ReadablePropertyMapConcept<WeightMap, Edge> >();
function_requires< ReadWritePropertyMapConcept<DistanceMap, Vertex> >();
```

These checks fire at instantiation time, producing diagnostics naming the
missing requirement. To find an algorithm's exact concept set, search for
`function_requires` (older form) or `BOOST_CONCEPT_ASSERT` (newer form) in
its implementation header.

---

## 6. Models — which graph types satisfy which concepts

| Graph type | `IncidenceGraph` | `Bidirectional` | `AdjacencyGraph` | `VertexListGraph` | `EdgeListGraph` | `AdjacencyMatrix` | Mutable |
|------------|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| `adjacency_list<…, directedS, …>` | ✓ | | ✓ | ✓ | ✓ | | full `MutableIncidenceGraph` |
| `adjacency_list<…, undirectedS, …>` | ✓ | ✓ (via `degree`) | ✓ | ✓ | ✓ | | full |
| `adjacency_list<…, bidirectionalS, …>` | ✓ | ✓ | ✓ | ✓ | ✓ | | `MutableBidirectionalGraph` |
| `adjacency_matrix<…>` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | yes |
| `compressed_sparse_row_graph<…>` | ✓ | optional | ✓ | ✓ | ✓ | | no |
| `grid_graph<N>` | ✓ | ✓ | ✓ | ✓ | ✓ | | no (implicit) |
| `edge_list<…>` | | | | | ✓ | | no |
| `filtered_graph<G, …>` | inherits subset of `G` (see [graph-types.md](graph-types.md)) | | | | | | no |
| `reverse_graph<G>` | inherits with edges flipped | | | | | | no |

For a full discussion of each graph type and its property/index support,
see [graph-types.md](graph-types.md).

---

## 7. Where to go next

| You want to | Read |
|-------------|------|
| The algorithm-side view (which concepts an algorithm needs) | [algorithms.md](algorithms.md) |
| The high-level generic-programming explanation | [architecture.md](architecture.md) |
| The data structures that model these concepts | [graph-types.md](graph-types.md) |
| The original prose specification with examples | [`doc/graph_concepts.html`](../../../doc/graph_concepts.html) |
| Visitor concepts (a separate concept family) | [architecture.md](architecture.md) § Visitor pattern |
