# Extending BGL

Back to [overview.md](../overview.md)

BGL is designed to be extended by library users without modifying the
library itself. There are three main extension points: new algorithms,
new graph types, and new property maps. Each follows a documented pattern
that relies on the concept/archetype machinery described in
[concepts.md](concepts.md).

---

## 1. Adding a new algorithm

### 1.1 Choose the right concept requirements

Declare the minimum set of concepts your algorithm needs. Prefer weaker
concepts (e.g. `IncidenceGraph` over `BidirectionalGraph`) if the
algorithm does not use in-edges. This maximises the set of graph types
your algorithm can work with.

Use `BOOST_CONCEPT_ASSERT` (or the older `function_requires`) at the top
of the implementation to enforce them at compile time:

```cpp
#include <boost/graph/graph_concepts.hpp>

template <typename Graph, typename WeightMap, typename DistanceMap>
void my_algorithm(const Graph& g, WeightMap weight, DistanceMap dist)
{
    BOOST_CONCEPT_ASSERT((boost::concepts::IncidenceGraphConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::concepts::VertexListGraphConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyMapConcept<
        WeightMap, typename boost::graph_traits<Graph>::edge_descriptor>));
    // ...
}
```

Verify against the archetypes in
[`graph_archetypes.hpp`](../../../include/boost/graph/graph_archetypes.hpp)
to make sure you rely on nothing outside the declared concept set.

### 1.2 Named parameters

Algorithms expose optional parameters via the Boost.Parameter mechanism.
Include [`named_function_params.hpp`](../../../include/boost/graph/named_function_params.hpp)
to access the pre-declared parameter keywords (`boost::graph::keywords`
namespace in newer code, legacy `bgl_named_params` in older code).

A typical pattern:

```cpp
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/detail/d_ary_heap.hpp>

// Core implementation accepting fully-resolved types:
template <typename Graph, typename WeightMap, typename DistanceMap,
          typename PredecessorMap>
void my_algorithm_impl(const Graph& g, WeightMap weight,
                       DistanceMap dist, PredecessorMap pred);

// Named-parameter entry point — defaults filled in here:
template <typename Graph, typename P, typename T, typename R>
void my_algorithm(const Graph& g,
    const boost::bgl_named_params<P, T, R>& params
        = boost::no_named_parameters())
{
    using boost::graph::keywords::_weight_map;
    using boost::graph::keywords::_distance_map;
    // resolve defaults ...
    my_algorithm_impl(g, ...);
}
```

Study `dijkstra_shortest_paths.hpp` as a well-documented reference for
the complete pattern including default-filling and parameter-map
construction.

### 1.3 Visitor event-point pattern

If your algorithm has customisation hooks (e.g. "do something when a
vertex is discovered"), expose them as a *visitor* with named event-point
methods:

```cpp
template <typename Visitor>
struct MyAlgorithmVisitorConcept {
    void constraints() {
        Visitor vis;
        boost::graph_traits<G>::vertex_descriptor u;
        vis.initialize_vertex(u, g);
        vis.examine_vertex(u, g);
        // ... other event points
    }
};

// Default no-op visitor:
struct my_algorithm_default_visitor {
    template <typename V, typename G> void initialize_vertex(V, const G&) {}
    template <typename V, typename G> void examine_vertex(V, const G&) {}
    // ...
};
```

See [`visitors.hpp`](../../../include/boost/graph/visitors.hpp) for the
shared visitor utility machinery used across BGL, and
[architecture.md](architecture.md) § Visitor pattern for the event-point
catalogue.

### 1.4 Testing a new algorithm

Add a `.cpp` file under `test/` and register it in
[`test/Jamfile.v2`](../../../test/Jamfile.v2):

```
[ run my_algorithm_test.cpp ]
```

Run from the superproject root:

```sh
./b2 libs/graph/test//my_algorithm_test
```

---

## 2. Adding a new graph type

### 2.1 Declare `graph_traits` specialisation

Every BGL graph type must specialise (or provide ADL-found overloads for)
`boost::graph_traits<G>`. The required nested types depend on which
concepts your graph will model. At minimum for `IncidenceGraph`:

```cpp
namespace boost {
template <>
struct graph_traits<MyGraph> {
    using vertex_descriptor   = /* ... */;
    using edge_descriptor     = /* ... */;
    using directed_category   = directed_tag;         // or undirected_tag
    using edge_parallel_category = allow_parallel_edge_tag; // or disallow_parallel_edge_tag
    using traversal_category  = incidence_graph_tag;  // or combination tag
    using out_edge_iterator   = /* ... */;
    using degree_size_type    = std::size_t;
};
} // namespace boost
```

Then provide the required free functions (`out_edges`, `out_degree`,
`source`, `target`, …) in `MyGraph`'s namespace so ADL finds them.

### 2.2 Choose which concepts to model

More concepts = compatible with more algorithms. A read-only implicit
graph (like `grid_graph`) models `IncidenceGraph + VertexListGraph +
EdgeListGraph` without mutability. A full mutable container graph
(like `adjacency_list`) adds `MutableBidirectionalGraph`.

Consult the full hierarchy in [concepts.md](concepts.md) §2 to decide
which refinement chain to implement.

### 2.3 Verify against archetypes

Instantiate your algorithm template against
`incidence_graph_archetype<…>` or `vertex_list_graph_archetype<…>` from
[`graph_archetypes.hpp`](../../../include/boost/graph/graph_archetypes.hpp)
to confirm you haven't accidentally narrowed the supported concept set.

### 2.4 Interior properties (optional)

To support `get(vertex_color, g)` / `get(edge_weight, g)` style access,
specialise `vertex_property_type<G>` and `edge_property_type<G>` and
implement the `PropertyGraph` overloads. See
[`adjacency_list.hpp`](../../../include/boost/graph/adjacency_list.hpp)
for a comprehensive example.

---

## 3. Adding a new property map

Property maps are the BGL-standard way to associate values with
vertex/edge descriptors without coupling them to the graph.

### 3.1 Implement the property map concept

A property map must provide `boost::property_traits<Map>` with:
- `value_type` — type returned by `get`
- `key_type` — type of the key (typically a vertex or edge descriptor)
- `category` — `readable_property_map_tag`, `read_write_property_map_tag`,
  or `lvalue_property_map_tag`

And the free functions `get(map, key)` and (for writable) `put(map, key, value)`.

### 3.2 Use `exterior_property` for vector-backed maps

For the common case of a vector indexed by a contiguous vertex/edge
index, use the helper in
[`exterior_property.hpp`](../../../include/boost/graph/exterior_property.hpp):

```cpp
#include <boost/graph/exterior_property.hpp>

// Declare a per-vertex double property:
using DistanceProperty = boost::exterior_vertex_property<Graph, double>;
using DistanceMatrix   = typename DistanceProperty::matrix_type;
using DistanceMap      = typename DistanceProperty::map_type;

DistanceMatrix dist_matrix(num_vertices(g));
DistanceMap    dist(dist_matrix, g);
```

This produces a property map compatible with all BGL distance-related
algorithms.

### 3.3 Bundled and interior properties

If you own the graph type and want properties stored inside graph
structures, use the bundled-property mechanism from
[`adjacency_list.hpp`](../../../include/boost/graph/adjacency_list.hpp)
and document the bundle struct. See
[`doc/bundles.html`](../../../doc/bundles.html) for the full recipe.

---

## 4. Testing conventions

| Convention | Detail |
|------------|--------|
| Test file location | `test/<feature_name>_test.cpp` or `test/<feature_name>.cpp` |
| Build registration | Add a `[ run ... ]` entry in [`test/Jamfile.v2`](../../../test/Jamfile.v2) under the appropriate `alias` |
| Concept-check tests | Use `[ compile ... ]` entries for headers that should compile against archetypes |
| Run command | `./b2 libs/graph/test` from the superproject root |

---

## 5. Where to go next

| Topic | Read |
|-------|------|
| Concept requirements reference | [concepts.md](concepts.md) |
| Graph types shipped with BGL | [graph-types.md](graph-types.md) |
| Algorithm catalogue | [algorithms.md](algorithms.md) |
| Building and running tests | [build-and-test.md](build-and-test.md) |
| Named-parameter mechanism | [architecture.md](architecture.md) § Named parameters |
