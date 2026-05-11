# Glossary

Back to [overview.md](../overview.md)

Terms used throughout BGL documentation and source code, listed
alphabetically. Cross-references point to the subdocument with the
primary treatment of each concept.

---

**archetype**  
A minimal class that models exactly one concept and nothing more. Used to
verify that an algorithm template does not accidentally rely on
capabilities outside its advertised concept requirements. BGL provides
graph archetypes in
[`graph_archetypes.hpp`](../../../include/boost/graph/graph_archetypes.hpp).
See [concepts.md](concepts.md) §4.

---

**event point**  
A named hook in a traversal algorithm where user-supplied code (a
*visitor* method) is called. Examples: `examine_vertex`,
`tree_edge`, `finish_vertex`. Event points are fixed per algorithm and
documented in each algorithm's visitor concept. See
[architecture.md](architecture.md) § Visitor pattern.

---

**exterior property**  
Property data stored *outside* the graph object, in a separate container
(typically a `std::vector`) indexed by a vertex or edge index. Exterior
properties are expressed as property maps and passed to algorithms as
arguments. Contrast *interior property*. See
[`exterior_property.hpp`](../../../include/boost/graph/exterior_property.hpp)
and [extending.md](extending.md) §3.

---

**graph descriptor** (vertex descriptor / edge descriptor)  
An opaque handle that uniquely identifies a vertex or edge within a
particular graph instance. Descriptors are returned by `add_vertex`,
`add_edge`, `vertices(g)`, `edges(g)`, etc. They must be
`DefaultConstructible`, `EqualityComparable`, and `Assignable`. They
should be treated as opaque: do not assume they are integers (unless
using `vecS`-backed storage). See [concepts.md](concepts.md) §2.1 and
[architecture.md](architecture.md) § `graph_traits<G>`.

---

**interior property**  
Property data stored *inside* the graph object, attached to each vertex
or edge at graph-construction time via a property type parameter (e.g.
`adjacency_list<…, property<vertex_color_t, int>, …>`). Interior
properties are accessed via `get(vertex_color, g, v)`. Contrast
*exterior property*. See [architecture.md](architecture.md) § Property
maps and [graph-types.md](graph-types.md).

---

**named parameter**  
A call convention (implemented via Boost.Parameter) that allows passing
optional algorithm arguments by keyword rather than position. Callers
omit parameters they don't need; the algorithm fills in defaults.
Example: `dijkstra_shortest_paths(g, s, predecessor_map(pred)
    .distance_map(dist).weight_map(wt))`. Documented in
[`named_function_params.hpp`](../../../include/boost/graph/named_function_params.hpp)
and [`doc/bgl_named_params.html`](../../../doc/bgl_named_params.html).
See [architecture.md](architecture.md) § Named parameters.

---

**property map**  
An object that maps keys (vertex/edge descriptors, or other types) to
values. The map concept is defined by Boost.PropertyMap
(`readable_property_map_tag`, `read_write_property_map_tag`,
`lvalue_property_map_tag`). Algorithms use property maps for weights,
colors, distances, predecessors, and similar per-element data. See
[architecture.md](architecture.md) § Property maps.

---

**selector tag** (container selector)  
A tag type passed as a template parameter to `adjacency_list` to choose
the underlying container for vertex and edge storage. Examples: `vecS`
(vector), `listS` (list), `setS` (ordered set), `hash_setS` (unordered
set), `multisetS`. The choice affects iterator stability, edge-parallel
policy, and whether vertex descriptors are indices or iterators. See
[graph-types.md](graph-types.md) § `adjacency_list`.

---

**traversal category**  
A tag type (or combination of tag types) nested in `graph_traits<G>` as
`traversal_category` that declares which traversal-concept tags a graph
type supports. Algorithms use `boost::is_convertible<traversal_category,
some_graph_tag>` to detect capabilities at compile time. See
[concepts.md](concepts.md) §3.

---

**vertex/edge bundle**  
A user-defined struct stored at each vertex or edge, used as an
alternative to the `property<Tag, Value>` chain for attaching multiple
properties. Accessed directly via `g[v]` or `g[e]` when using
`adjacency_list` with bundle types. See
[`doc/bundles.html`](../../../doc/bundles.html) and
[graph-types.md](graph-types.md) § `adjacency_list`.

---

**visitor**  
A user-supplied object with methods called at algorithm *event points*.
Visitors allow injecting custom logic (logging, early termination,
recording) without modifying the algorithm. Each BGL algorithm defines a
*visitor concept* specifying which methods it calls and when. Default
no-op visitors (e.g. `bfs_visitor<>`, `dfs_visitor<>`) are provided for
use as base classes. See [architecture.md](architecture.md) § Visitor
pattern.
