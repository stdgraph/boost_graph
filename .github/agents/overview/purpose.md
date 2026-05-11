# Purpose

Back to [overview.md](../overview.md)

---

## What problems BGL solves

Writing graph algorithms from scratch requires simultaneously solving two
unrelated problems: the algorithmic logic (how to traverse, relax, merge,
etc.) and the data-structure mechanics (how edges are stored, how vertices
are iterated, how properties are associated). These concerns cross-cut
each other: the same Dijkstra logic should work regardless of whether
vertices are integers in a vector, node pointers in a list, or keys in a
hash map.

BGL separates them cleanly:

- **Algorithms** are written once, in terms of abstract *concepts*
  (`IncidenceGraph`, `VertexListGraph`, `WeightMap`, …).
- **Data structures** implement those concepts and can be swapped in
  without changing algorithm code.
- **Property maps** carry per-element data (weights, colors, distances)
  between the two without hard-wiring storage decisions into either.

The library ships a set of production-quality algorithm implementations
(shortest paths, spanning trees, network flow, planarity, layout, …) and a
set of general-purpose graph data structures (`adjacency_list`,
`adjacency_matrix`, `compressed_sparse_row_graph`, …) that can be combined
freely.

---

## Why this design matters

The alternative — writing algorithms that accept only one graph
representation — forces either code duplication (one Dijkstra per graph
type) or adaptation overhead (convert-then-run). Either approach breaks
down as the number of algorithm/graph combinations grows.

BGL's concept-based approach means:

- A user-defined graph type gets access to the full algorithm library the
  moment it satisfies the appropriate concept.
- Algorithm authors do not need to know anything about storage; they rely
  only on the operations the concept guarantees (and they can verify this
  at compile time via archetypes).
- Compile-time checking of concept requirements produces diagnostics that
  name the missing expression, not a cascade of template errors from deep
  inside the algorithm body.

---

## Primary audience

BGL is aimed at **C++ library and application developers** who need graph
algorithms embedded in a larger system:

- Developers of scientific computing, EDA, network analysis, or
  computational geometry libraries who want to use or customise graph
  algorithms without writing them from scratch.
- Application developers who have an existing graph representation (e.g.
  a scene graph, a dependency graph, a road network) and want to plug it
  directly into standard algorithms by writing a thin concept adaptor.
- Researchers and students who want a reference implementation of
  published algorithms with a documented interface.

BGL is *not* a stand-alone graph application or visualisation tool. It
provides no I/O beyond GraphViz and GraphML parsing, and it requires
familiarity with C++ templates.

---

## Relationship to the C++ Standard Library

BGL applies the same philosophy that the STL applies to sequences:

| STL | BGL analogue |
|-----|--------------|
| Iterator concept | Graph traversal concept (`IncidenceGraph`, etc.) |
| Algorithm (e.g. `std::sort`) | Graph algorithm (e.g. `dijkstra_shortest_paths`) |
| Container (e.g. `std::vector`) | Graph data structure (e.g. `adjacency_list`) |
| Iterator adaptor | Graph adaptor (`filtered_graph`, `reverse_graph`, …) |
| `std::function_output_iterator` | Visitor event-point callback |

Just as STL algorithms work on any range satisfying the iterator concept,
BGL algorithms work on any graph satisfying the appropriate graph concept.
BGL depends on the STL for iterators, containers, and utilities; it is
built on top of the STL, not as a replacement.

---

## Where to go next

| You want to | Read |
|-------------|------|
| Understand the generic-programming model | [architecture.md](architecture.md) |
| See the concept definitions | [concepts.md](concepts.md) |
| Choose a graph data structure | [graph-types.md](graph-types.md) |
| Find an algorithm | [algorithms.md](algorithms.md) |
