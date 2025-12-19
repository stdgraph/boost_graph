# BGL Modern Concepts Reference

## Graph Concepts in C++20

BGL Modern uses C++20 concepts to provide compile-time checking of graph types and clear template constraints.

---

## Core Graph Concepts

### `Graph<G>`

The most basic graph concept. Requires vertex and edge descriptor types.

```cpp
template<typename G>
concept Graph = requires {
    typename graph_traits<G>::vertex_descriptor;
    typename graph_traits<G>::edge_descriptor;
    typename graph_traits<G>::directed_category;
};
```

**Example:**
```cpp
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/adjacency_list.hpp>

using namespace bgl;

static_assert(Graph<adjacency_list<directed_tag>>);
static_assert(Graph<adjacency_list<undirected_tag>>);

// Use in templates
template<Graph G>
void process_graph(const G& g) {
    // G is guaranteed to be a Graph
}
```

---

### `IncidenceGraph<G>`

A graph where you can iterate over out-edges of a vertex.

```cpp
template<typename G>
concept IncidenceGraph = Graph<G> && requires(const G& g, vertex_descriptor_t<G> v) {
    { out_edges(v, g) } -> std::ranges::forward_range;
    { source(edge_descriptor_t<G>{}, g) } -> std::convertible_to<vertex_descriptor_t<G>>;
    { target(edge_descriptor_t<G>{}, g) } -> std::convertible_to<vertex_descriptor_t<G>>;
    { out_degree(v, g) } -> std::convertible_to<std::size_t>;
};
```

**Required Operations:**
- `out_edges(v, g)` - range of edges leaving vertex v
- `source(e, g)` - source vertex of edge e
- `target(e, g)` - target vertex of edge e  
- `out_degree(v, g)` - number of out-edges from v

**Example:**
```cpp
template<IncidenceGraph G>
void print_neighbors(const G& g, vertex_descriptor_t<G> v) {
    for (auto e : out_edges(v, g)) {
        std::cout << "Edge to: " << target(e, g) << "\n";
    }
}

adjacency_list<directed_tag> g(5);
g.add_edge(0, 1);
g.add_edge(0, 2);

print_neighbors(g, 0);  // Prints: Edge to: 1, Edge to: 2
```

---

### `BidirectionalGraph<G>`

An incidence graph with in-edges as well.

```cpp
template<typename G>
concept BidirectionalGraph = IncidenceGraph<G> && requires(const G& g, vertex_descriptor_t<G> v) {
    { in_edges(v, g) } -> std::ranges::forward_range;
    { in_degree(v, g) } -> std::convertible_to<std::size_t>;
};
```

**Additional Operations:**
- `in_edges(v, g)` - range of edges entering vertex v
- `in_degree(v, g)` - number of in-edges to v

**Example:**
```cpp
template<BidirectionalGraph G>
std::size_t total_degree(const G& g, vertex_descriptor_t<G> v) {
    return out_degree(v, g) + in_degree(v, g);
}
```

---

### `VertexListGraph<G>`

A graph where you can iterate over all vertices.

```cpp
template<typename G>
concept VertexListGraph = Graph<G> && requires(const G& g) {
    { vertices(g) } -> std::ranges::forward_range;
    { num_vertices(g) } -> std::convertible_to<std::size_t>;
};
```

**Required Operations:**
- `vertices(g)` - range of all vertices
- `num_vertices(g)` - total vertex count

**Example:**
```cpp
template<VertexListGraph G>
void print_all_vertices(const G& g) {
    std::cout << "Graph has " << num_vertices(g) << " vertices:\n";
    for (auto v : vertices(g)) {
        std::cout << "  Vertex: " << v << "\n";
    }
}
```

---

### `EdgeListGraph<G>`

A graph where you can iterate over all edges.

```cpp
template<typename G>
concept EdgeListGraph = Graph<G> && requires(const G& g) {
    { edges(g) } -> std::ranges::forward_range;
    { num_edges(g) } -> std::convertible_to<std::size_t>;
};
```

**Required Operations:**
- `edges(g)` - range of all edges
- `num_edges(g)` - total edge count

**Example:**
```cpp
template<EdgeListGraph G>
std::size_t count_self_loops(const G& g) {
    std::size_t count = 0;
    for (auto e : edges(g)) {
        if (source(e, g) == target(e, g)) {
            ++count;
        }
    }
    return count;
}
```

---

### `AdjacencyGraph<G>`

A graph where you can iterate over adjacent vertices directly.

```cpp
template<typename G>
concept AdjacencyGraph = Graph<G> && requires(const G& g, vertex_descriptor_t<G> v) {
    { adjacent_vertices(v, g) } -> std::ranges::forward_range;
};
```

**Required Operations:**
- `adjacent_vertices(v, g)` - range of vertices adjacent to v

**Example:**
```cpp
template<AdjacencyGraph G>
bool has_neighbor(const G& g, vertex_descriptor_t<G> v, vertex_descriptor_t<G> target) {
    for (auto u : adjacent_vertices(v, g)) {
        if (u == target) return true;
    }
    return false;
}
```

---

## Property Map Concepts

### `ReadablePropertyMap<PM, Key>`

A property map that can be read.

```cpp
template<typename PM, typename Key>
concept ReadablePropertyMap = requires(const PM& pmap, Key key) {
    { pmap(key) };  // Must be callable with key
};
```

**Example:**
```cpp
template<typename G, ReadablePropertyMap<vertex_descriptor_t<G>> DistanceMap>
void print_distances(const G& g, DistanceMap distance_map) {
    for (auto v : vertices(g)) {
        std::cout << "Distance to " << v << ": " << distance_map(v) << "\n";
    }
}

// Usage
auto result = dijkstra_shortest_paths(g, 0, weight_map);
print_distances(g, result.distance_map());
```

---

### `WritablePropertyMap<PM, Key, Value>`

A property map that can be written.

```cpp
template<typename PM, typename Key, typename Value>
concept WritablePropertyMap = requires(PM& pmap, Key key, Value value) {
    { pmap(key) = value };
};
```

**Example:**
```cpp
template<typename G, WritablePropertyMap<vertex_descriptor_t<G>, int> ColorMap>
void initialize_colors(const G& g, ColorMap color_map) {
    for (auto v : vertices(g)) {
        color_map(v) = 0;  // White
    }
}
```

---

### `ReadWritePropertyMap<PM, Key, Value>`

A property map that supports both reading and writing.

```cpp
template<typename PM, typename Key, typename Value>
concept ReadWritePropertyMap = 
    ReadablePropertyMap<PM, Key> && 
    WritablePropertyMap<PM, Key, Value>;
```

---

## Weight Accessor Concepts

### `WeightAccessor<WA, G>`

A callable that returns edge weights.

```cpp
template<typename WA, typename G>
concept WeightAccessor = requires(const WA& wa, edge_descriptor_t<G> e) {
    { wa(e) } -> std::convertible_to<double>;
};
```

**Example:**
```cpp
struct EdgeProps {
    double weight;
};

adjacency_list<directed_tag, no_property, EdgeProps> g(5);

// Lambda weight accessor
auto weight_map = [&g](const auto& e) { return g[e].weight; };

static_assert(WeightAccessor<decltype(weight_map), decltype(g)>);

dijkstra_shortest_paths(g, 0, weight_map);
```

---

## Property Graph Concepts

### `PropertyGraph<G, Property>`

A graph with bundled properties accessible via `operator[]`.

```cpp
template<typename G, typename Property>
concept PropertyGraph = Graph<G> && requires(G& g, vertex_descriptor_t<G> v) {
    { g[v] } -> std::convertible_to<Property&>;
};
```

**Example:**
```cpp
struct VertexProps {
    std::string name;
    int value;
};

adjacency_list<directed_tag, VertexProps> g(10);

static_assert(PropertyGraph<decltype(g), VertexProps>);

g[0].name = "Start";
g[0].value = 42;
```

---

## Using Concepts in Your Code

### Template Constraints

```cpp
// Old way (C++17)
template<typename G>
std::enable_if_t<is_vertex_list_graph_v<G>, void>
process_graph(const G& g) {
    // ...
}

// New way (C++20)
template<VertexListGraph G>
void process_graph(const G& g) {
    // ...
}
```

### Concept Combination

```cpp
// Require multiple concepts
template<typename G>
    requires VertexListGraph<G> && IncidenceGraph<G>
void analyze_graph(const G& g) {
    for (auto v : vertices(g)) {
        std::cout << "Vertex " << v << " has " 
                  << out_degree(v, g) << " out-edges\n";
    }
}
```

### Concept-Based Overloading

```cpp
// Different implementations based on concepts
template<BidirectionalGraph G>
void traverse(const G& g, vertex_descriptor_t<G> v) {
    std::cout << "Using bidirectional traversal\n";
    // Can use both in_edges and out_edges
}

template<IncidenceGraph G>
    requires (!BidirectionalGraph<G>)
void traverse(const G& g, vertex_descriptor_t<G> v) {
    std::cout << "Using forward-only traversal\n";
    // Only out_edges available
}
```

---

## Custom Graph Types

### Implementing Graph Concepts

To make your custom type satisfy BGL Modern concepts:

```cpp
struct MyGraph {
    using vertex_descriptor = std::size_t;
    using edge_descriptor = std::pair<std::size_t, std::size_t>;
    using directed_category = bgl::directed_tag;
    
    std::vector<std::vector<std::size_t>> adjacency;
    
    MyGraph(std::size_t n) : adjacency(n) {}
};

// Free functions in same namespace (ADL)
inline std::size_t num_vertices(const MyGraph& g) {
    return g.adjacency.size();
}

inline auto vertices(const MyGraph& g) {
    return std::ranges::iota_view(std::size_t{0}, g.adjacency.size());
}

inline auto out_edges(std::size_t v, const MyGraph& g) {
    return g.adjacency[v] 
        | std::views::transform([v](std::size_t u) {
            return std::make_pair(v, u);
        });
}

inline std::size_t source(const MyGraph::edge_descriptor& e, const MyGraph&) {
    return e.first;
}

inline std::size_t target(const MyGraph::edge_descriptor& e, const MyGraph&) {
    return e.second;
}

inline std::size_t out_degree(std::size_t v, const MyGraph& g) {
    return g.adjacency[v].size();
}

// Now MyGraph satisfies VertexListGraph and IncidenceGraph!
static_assert(bgl::VertexListGraph<MyGraph>);
static_assert(bgl::IncidenceGraph<MyGraph>);
```

---

## Concept Checking Best Practices

### 1. Use Static Assertions

```cpp
template<typename G>
void my_algorithm(const G& g) {
    static_assert(VertexListGraph<G>, 
                  "G must be a VertexListGraph");
    static_assert(IncidenceGraph<G>,
                  "G must be an IncidenceGraph");
    // ...
}
```

### 2. Prefer Concept Constraints

```cpp
// Better: Use requires clause
template<typename G>
    requires VertexListGraph<G> && IncidenceGraph<G>
void my_algorithm(const G& g) {
    // ...
}

// Best: Use concept directly
template<VertexListGraph G>
    requires IncidenceGraph<G>
void my_algorithm(const G& g) {
    // ...
}
```

### 3. Document Concept Requirements

```cpp
/// Performs graph analysis
/// @tparam G Must satisfy VertexListGraph and IncidenceGraph
/// @param g The graph to analyze
template<typename G>
    requires VertexListGraph<G> && IncidenceGraph<G>
void analyze(const G& g);
```

---

## Summary

| Concept | Key Requirement | Example Use |
|---------|----------------|-------------|
| `Graph` | Basic type info | Type traits access |
| `IncidenceGraph` | Out-edge iteration | BFS, DFS, Dijkstra |
| `BidirectionalGraph` | In-edge iteration | Reverse traversals |
| `VertexListGraph` | Vertex iteration | All-vertices algorithms |
| `EdgeListGraph` | Edge iteration | Edge-based algorithms |
| `AdjacencyGraph` | Adjacent vertex iteration | Direct neighbor access |
| `PropertyGraph` | Bundled properties | Property-based algorithms |

All BGL Modern algorithms use these concepts for clear, compile-time checked interfaces.
