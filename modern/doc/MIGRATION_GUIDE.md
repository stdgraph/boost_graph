# BGL Modern Migration Guide

## Migrating from Boost.Graph to BGL Modern

This guide helps you migrate existing Boost.Graph code to the new BGL Modern API with C++20 features.

---

## Overview of Changes

### Key Improvements
- **C++20 Concepts**: Type-safe graph concepts replace SFINAE
- **Ranges**: All graph operations return C++20 ranges
- **Structured Returns**: Algorithms return structured results, not output parameters
- **Lambda Visitors**: Modern callback interface with lambdas
- **Named Parameters**: Clearer parameter passing with designated initializers
- **Strong Typing**: Type-safe vertex/edge descriptors

---

## Graph Construction

### Old API (Boost.Graph)
```cpp
#include <boost/graph/adjacency_list.hpp>

typedef boost::adjacency_list<boost::vecS, boost::vecS, 
                               boost::directedS> Graph;
Graph g(5);
boost::add_edge(0, 1, g);
boost::add_edge(1, 2, g);
```

### New API (BGL Modern)
```cpp
#include <bgl/modern/adjacency_list.hpp>

using namespace bgl;
adjacency_list<directed_tag> g(5);
g.add_edge(0, 1);
g.add_edge(1, 2);
```

**Changes:**
- Simpler template parameters (only directionality required)
- Member function `add_edge()` instead of free function
- No selector tags needed (vecS, etc.)

---

## Algorithm Invocation

### Breadth-First Search

#### Old API
```cpp
std::vector<int> distances(num_vertices(g));
std::vector<Vertex> predecessors(num_vertices(g));

boost::breadth_first_search(g, start,
    boost::visitor(
        boost::make_bfs_visitor(
            boost::record_distances(distances.data(), 
                                   boost::on_tree_edge())
        )
    )
);
```

#### New API
```cpp
auto result = breadth_first_search(g, start);

// Access results directly
auto distance_to_v = result.distance_map()(v);
auto predecessor_of_v = result.predecessor_map()(v);
```

**Changes:**
- No output parameters - results are returned
- Structured result with named accessor methods
- No need to pre-allocate storage
- Much simpler invocation

---

### Dijkstra's Algorithm

#### Old API
```cpp
std::vector<double> distances(num_vertices(g));
std::vector<Vertex> predecessors(num_vertices(g));

auto weight_map = boost::get(&EdgeProps::weight, g);

boost::dijkstra_shortest_paths(g, start,
    boost::distance_map(&distances[0])
           .predecessor_map(&predecessors[0])
           .weight_map(weight_map)
);
```

#### New API
```cpp
// Lambda weight map
auto weight_map = [&g](const auto& e) { return g[e].weight; };

auto result = dijkstra_shortest_paths(g, start, weight_map);

auto distance_to_v = result.distance_map()(v);
auto predecessor_of_v = result.predecessor_map()(v);
```

**Changes:**
- Weight map is a simple lambda
- No named parameter library needed
- Results returned, not passed as parameters
- Cleaner, more readable code

---

### Depth-First Search

#### Old API
```cpp
std::vector<int> discover_times(num_vertices(g));
std::vector<int> finish_times(num_vertices(g));

boost::depth_first_search(g,
    boost::visitor(
        boost::make_dfs_visitor(
            boost::record_timestamps(
                discover_times.data(), finish_times.data(),
                boost::on_discover_vertex()
            )
        )
    )
);
```

#### New API
```cpp
auto result = depth_first_search(g, start);

auto discover_time = result.discovery_time_map()(v);
auto finish_time = result.finish_time_map()(v);
```

---

## Visitors and Callbacks

### Old API - Custom Visitor
```cpp
struct MyVisitor : public boost::default_bfs_visitor {
    void discover_vertex(Vertex v, const Graph& g) {
        std::cout << "Discovered: " << v << "\n";
    }
    
    void examine_edge(Edge e, const Graph& g) {
        std::cout << "Edge: " << source(e, g) 
                  << "->" << target(e, g) << "\n";
    }
};

boost::breadth_first_search(g, start, boost::visitor(MyVisitor()));
```

### New API - Lambda Callbacks
```cpp
auto callbacks = on_discover_vertex([](auto v, const auto& g) {
    std::cout << "Discovered: " << v << "\n";
});

breadth_first_search(g, start, callbacks);
```

**Or multiple callbacks:**
```cpp
bfs_callbacks callbacks{
    .on_discover_vertex = [](auto v, const auto& g) {
        std::cout << "Discovered: " << v << "\n";
    },
    .on_examine_edge = [](auto e, const auto& g) {
        std::cout << "Edge: " << source(e, g) 
                  << "->" << target(e, g) << "\n";
    }
};

breadth_first_search(g, start, callbacks);
```

**Changes:**
- No class inheritance required
- Use lambdas directly
- Designated initializers for multiple callbacks
- Type-safe, concise

---

## Property Maps

### Bundled Properties

#### Old API
```cpp
struct VertexProps {
    int value;
};

struct EdgeProps {
    double weight;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS,
                               boost::directedS,
                               VertexProps, EdgeProps> Graph;
Graph g;

// Access
auto weight_map = boost::get(&EdgeProps::weight, g);
double w = boost::get(weight_map, e);
```

#### New API
```cpp
struct VertexProps {
    int value;
};

struct EdgeProps {
    double weight;
};

adjacency_list<directed_tag, VertexProps, EdgeProps> g(5);

// Direct access
g[v].value = 42;
double w = g[e].weight;
```

**Changes:**
- Direct property access with `operator[]`
- No property map accessors needed
- Simpler, more intuitive

---

### External Property Maps

#### Old API
```cpp
std::vector<double> distances(num_vertices(g));
auto dist_map = boost::make_iterator_property_map(
    distances.begin(),
    boost::get(boost::vertex_index, g)
);
```

#### New API
```cpp
std::vector<double> distances(num_vertices(g));

// Use vector directly or create property map
auto dist_map = [&distances](auto v) -> double& {
    return distances[v];
};
```

**Changes:**
- Simple lambdas replace complex property map construction
- Direct vector access when appropriate
- More flexible and readable

---

## Iteration and Ranges

### Old API
```cpp
// Iterate vertices
auto [vi, vi_end] = boost::vertices(g);
for (; vi != vi_end; ++vi) {
    auto v = *vi;
    // ...
}

// Iterate edges
auto [ei, ei_end] = boost::out_edges(v, g);
for (; ei != ei_end; ++ei) {
    auto e = *ei;
    auto target_v = boost::target(e, g);
    // ...
}
```

### New API
```cpp
// C++20 range-based iteration
for (auto v : vertices(g)) {
    // ...
}

// Works with range algorithms
std::ranges::for_each(vertices(g), [](auto v) {
    // ...
});

// Edge iteration
for (auto e : out_edges(v, g)) {
    auto target_v = target(e, g);
    // ...
}

// Range pipelines
auto targets = out_edges(v, g)
    | std::views::transform([&g](auto e) { return target(e, g); })
    | std::views::filter([](auto t) { return t > 10; });
```

**Changes:**
- Direct range-based for loops
- Full C++20 ranges support
- Composable with standard library algorithms and views
- No iterator pairs needed

---

## Type Safety

### Old API
```cpp
// Vertex and edge descriptors are often just integers
Graph::vertex_descriptor v = 0;
Graph::edge_descriptor e = add_edge(0, 1, g).first;

// Easy to mix up vertex indices and descriptors
```

### New API
```cpp
// Strong typing available
adjacency_list<directed_tag> g(5);

auto v = g.vertices()[0];  // Type-safe vertex descriptor
auto [e, inserted] = g.add_edge(0, 1);  // Structured binding

// Or use simple size_t when appropriate
for (std::size_t v = 0; v < num_vertices(g); ++v) {
    // ...
}
```

---

## Common Migration Patterns

### Pattern 1: Distance Map Result

**Old:**
```cpp
std::vector<double> distances(num_vertices(g));
dijkstra_shortest_paths(g, s, distance_map(&distances[0]));
return distances[target];
```

**New:**
```cpp
auto result = dijkstra_shortest_paths(g, s, weight_map);
return result.distance_map()(target);
```

---

### Pattern 2: Predecessor Path Reconstruction

**Old:**
```cpp
std::vector<Vertex> predecessors(num_vertices(g));
dijkstra_shortest_paths(g, s, predecessor_map(&predecessors[0]));

// Reconstruct path
std::vector<Vertex> path;
for (Vertex v = target; v != s; v = predecessors[v]) {
    path.push_back(v);
}
```

**New:**
```cpp
auto result = dijkstra_shortest_paths(g, s, weight_map);

// Reconstruct path
std::vector<std::size_t> path;
for (auto v = target; v != s; v = result.predecessor_map()(v)) {
    path.push_back(v);
}
```

---

### Pattern 3: Custom Vertex Properties

**Old:**
```cpp
struct VertexData {
    std::string name;
    int id;
};

typedef adjacency_list<vecS, vecS, directedS, VertexData> Graph;
Graph g;
auto v = add_vertex(g);
g[v].name = "Node1";
```

**New:**
```cpp
struct VertexData {
    std::string name;
    int id;
};

adjacency_list<directed_tag, VertexData> g(10);
g[0].name = "Node1";
g[0].id = 42;
```

---

## Concept Checking

### Old API
```cpp
BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<Graph>));
BOOST_CONCEPT_ASSERT((boost::IncidenceGraphConcept<Graph>));
```

### New API
```cpp
// Compile-time concept checking
static_assert(VertexListGraph<adjacency_list<directed_tag>>);
static_assert(IncidenceGraph<adjacency_list<directed_tag>>);

// Or constrained templates
template<VertexListGraph G>
void my_algorithm(const G& g) {
    // ...
}
```

**Changes:**
- C++20 concepts instead of BOOST_CONCEPT_ASSERT
- Cleaner template constraints
- Better error messages

---

## Summary of Benefits

| Feature | Old API | New API |
|---------|---------|---------|
| Return Style | Output parameters | Structured returns |
| Visitors | Class inheritance | Lambdas |
| Ranges | Iterator pairs | C++20 ranges |
| Concepts | Macro-based | C++20 concepts |
| Property Access | Property maps | Direct `operator[]` |
| Type Safety | Weak | Strong (when desired) |
| Verbosity | High | Low |
| Composability | Limited | Full (ranges/algorithms) |

---

## Quick Reference

### Header Mapping

| Old Header | New Header |
|------------|------------|
| `boost/graph/adjacency_list.hpp` | `bgl/modern/adjacency_list.hpp` |
| `boost/graph/breadth_first_search.hpp` | `bgl/modern/breadth_first_search.hpp` |
| `boost/graph/depth_first_search.hpp` | `bgl/modern/depth_first_search.hpp` |
| `boost/graph/dijkstra_shortest_paths.hpp` | `bgl/modern/dijkstra_shortest_paths.hpp` |
| `boost/graph/graph_traits.hpp` | `bgl/modern/graph_traits.hpp` |

### Function Mapping

| Old Function | New Function |
|--------------|--------------|
| `boost::add_edge(u, v, g)` | `g.add_edge(u, v)` |
| `boost::vertices(g)` | `vertices(g)` |
| `boost::out_edges(v, g)` | `out_edges(v, g)` |
| `boost::source(e, g)` | `source(e, g)` |
| `boost::target(e, g)` | `target(e, g)` |
| `boost::num_vertices(g)` | `num_vertices(g)` |

---

## Need Help?

- See `examples/modern/` for complete working examples
- Check `test/` directory for comprehensive usage examples
- All major Boost.Graph algorithms have been modernized

**Happy migrating!** 🚀
