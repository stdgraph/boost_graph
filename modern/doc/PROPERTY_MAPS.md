# Property Map Patterns in BGL Modern

## Overview

BGL Modern provides multiple ways to work with graph properties, from simple bundled properties to external property maps.

---

## Pattern 1: Bundled Properties (Recommended)

### Vertex Properties

The simplest and most efficient approach - store properties directly in the graph.

```cpp
#include <bgl/modern/adjacency_list.hpp>
#include <string>

struct VertexProps {
    std::string name;
    int id;
    double value;
};

using namespace bgl;
adjacency_list<directed_tag, VertexProps> g(10);

// Direct access
g[0].name = "Start";
g[0].id = 1;
g[0].value = 3.14;

// Read
std::cout << "Vertex 0: " << g[0].name << "\n";
```

**Advantages:**
- Fast - no indirection
- Type-safe
- Simple syntax
- Cache-friendly

**Use when:**
- Properties are integral to the graph
- All vertices need the same properties
- Performance matters

---

### Edge Properties

```cpp
struct EdgeProps {
    double weight;
    std::string label;
    bool visited = false;
};

adjacency_list<directed_tag, no_property, EdgeProps> g(5);

auto [e, inserted] = g.add_edge(0, 1, EdgeProps{.weight = 2.5, .label = "edge1"});

// Access
g[e].weight = 3.0;
g[e].visited = true;

std::cout << "Edge weight: " << g[e].weight << "\n";
```

---

### Combined Vertex and Edge Properties

```cpp
struct VertexProps {
    std::string name;
};

struct EdgeProps {
    double weight;
};

adjacency_list<directed_tag, VertexProps, EdgeProps> g(5);

g[0].name = "A";
g[1].name = "B";

auto [e, inserted] = g.add_edge(0, 1, EdgeProps{.weight = 10.0});
g[e].weight = 15.0;
```

---

## Pattern 2: External Property Maps

### Vector-Based Property Map

Use external vectors when:
- Properties are temporary/algorithm-specific
- Not all vertices need the property
- Properties are computed on-demand

```cpp
adjacency_list<directed_tag> g(100);

// External distance map
std::vector<double> distances(num_vertices(g), 
                              std::numeric_limits<double>::infinity());

distances[0] = 0.0;

// Use in algorithms
for (auto v : vertices(g)) {
    if (distances[v] < std::numeric_limits<double>::infinity()) {
        std::cout << "Distance to " << v << ": " << distances[v] << "\n";
    }
}
```

---

### Lambda Property Map

Create property maps on-the-fly using lambdas.

```cpp
adjacency_list<directed_tag> g(10);
std::vector<int> colors(num_vertices(g));

// Read-only lambda
auto color_map = [&colors](std::size_t v) {
    return colors[v];
};

// Read-write lambda
auto color_map_rw = [&colors](std::size_t v) -> int& {
    return colors[v];
};

// Usage
color_map_rw(0) = 1;
int color = color_map(0);
```

**Advantages:**
- Flexible
- No class needed
- Can add logic (caching, validation, etc.)

---

### Map-Based Property Map

Use `std::map` for sparse properties.

```cpp
adjacency_list<directed_tag> g(1000);

// Only some vertices have this property
std::map<std::size_t, std::string> labels;

labels[0] = "Start";
labels[999] = "End";

// Lambda wrapper
auto label_map = [&labels](std::size_t v) -> std::string {
    auto it = labels.find(v);
    return it != labels.end() ? it->second : "";
};

std::cout << "Vertex 0: " << label_map(0) << "\n";
std::cout << "Vertex 5: " << label_map(5) << "\n";  // Returns ""
```

**Use when:**
- Only a few vertices have the property
- Memory is constrained
- Properties are set dynamically

---

## Pattern 3: Computed Properties

### On-Demand Calculation

```cpp
struct EdgeProps {
    double weight;
};

adjacency_list<directed_tag, no_property, EdgeProps> g(5);
g.add_edge(0, 1, EdgeProps{.weight = 1.5});
g.add_edge(1, 2, EdgeProps{.weight = 2.5});

// Computed property: edge weight squared
auto squared_weight = [&g](const auto& e) {
    return g[e].weight * g[e].weight;
};

// Use as weight map
auto result = dijkstra_shortest_paths(g, 0, squared_weight);
```

---

### Derived Properties

```cpp
struct VertexProps {
    double x, y;  // Coordinates
};

adjacency_list<directed_tag, VertexProps> g(10);
g[0].x = 1.0; g[0].y = 2.0;
g[1].x = 4.0; g[1].y = 6.0;

// Computed property: distance from origin
auto distance_from_origin = [&g](std::size_t v) {
    return std::sqrt(g[v].x * g[v].x + g[v].y * g[v].y);
};

for (auto v : vertices(g)) {
    std::cout << "Vertex " << v << " distance: " 
              << distance_from_origin(v) << "\n";
}
```

---

## Pattern 4: Weight Maps for Algorithms

### Simple Lambda Weight Map

```cpp
struct EdgeProps {
    double weight;
};

adjacency_list<directed_tag, no_property, EdgeProps> g(5);
// ... add edges ...

auto weight_map = [&g](const auto& e) {
    return g[e].weight;
};

auto result = dijkstra_shortest_paths(g, 0, weight_map);
```

---

### Conditional Weight Map

```cpp
struct EdgeProps {
    double weight;
    bool blocked;
};

adjacency_list<directed_tag, no_property, EdgeProps> g(5);
// ... add edges ...

// Infinite weight for blocked edges
auto weight_map = [&g](const auto& e) {
    return g[e].blocked ? std::numeric_limits<double>::infinity() 
                        : g[e].weight;
};

auto result = dijkstra_shortest_paths(g, 0, weight_map);
```

---

### Combined Property Weight Map

```cpp
struct EdgeProps {
    double distance;
    double time;
};

adjacency_list<directed_tag, no_property, EdgeProps> g(5);
// ... add edges ...

// Use distance as weight
auto distance_weight = [&g](const auto& e) { return g[e].distance; };

// Or use time as weight
auto time_weight = [&g](const auto& e) { return g[e].time; };

// Different shortest paths!
auto by_distance = dijkstra_shortest_paths(g, 0, distance_weight);
auto by_time = dijkstra_shortest_paths(g, 0, time_weight);
```

---

## Pattern 5: Property Map Adapters

### Transformation Adapter

```cpp
struct VertexProps {
    int value;
};

adjacency_list<directed_tag, VertexProps> g(10);
for (auto v : vertices(g)) {
    g[v].value = static_cast<int>(v) * 2;
}

// Transform property on read
auto doubled_value = [&g](std::size_t v) {
    return g[v].value * 2;
};

for (auto v : vertices(g)) {
    std::cout << "Original: " << g[v].value 
              << ", Doubled: " << doubled_value(v) << "\n";
}
```

---

### Caching Adapter

```cpp
#include <unordered_map>

struct ExpensiveComputation {
    std::unordered_map<std::size_t, double> cache;
    
    double operator()(std::size_t v) {
        auto it = cache.find(v);
        if (it != cache.end()) {
            return it->second;
        }
        
        // Expensive computation
        double result = /* ... */;
        cache[v] = result;
        return result;
    }
};

adjacency_list<directed_tag> g(100);
ExpensiveComputation compute;

// First access is slow, subsequent accesses are fast
double val1 = compute(5);  // Computes
double val2 = compute(5);  // From cache
```

---

## Pattern 6: Algorithm Results as Property Maps

BGL Modern algorithms return results with property map interfaces.

### Using Algorithm Results

```cpp
adjacency_list<directed_tag> g(10);
// ... build graph ...

auto bfs_result = breadth_first_search(g, 0);

// Result acts as property map
auto distance_map = bfs_result.distance_map();
auto predecessor_map = bfs_result.predecessor_map();

for (auto v : vertices(g)) {
    std::cout << "Vertex " << v 
              << ": distance=" << distance_map(v)
              << ", pred=" << predecessor_map(v) << "\n";
}
```

---

### Composing Results

```cpp
auto bfs = breadth_first_search(g, 0);
auto dfs = depth_first_search(g, 0);

// Compare results
for (auto v : vertices(g)) {
    if (bfs.distance_map()(v) != dfs.discovery_time_map()(v)) {
        std::cout << "Different traversal order at " << v << "\n";
    }
}
```

---

## Best Practices

### 1. Choose the Right Pattern

```cpp
// ✓ GOOD: Bundled properties for core graph data
struct VertexProps {
    std::string name;
    double x, y;
};
adjacency_list<directed_tag, VertexProps> g(10);

// ✓ GOOD: External vectors for algorithm temporaries
std::vector<bool> visited(num_vertices(g), false);

// ✗ BAD: External map for all vertices
std::map<std::size_t, std::string> names;  // Use bundled instead!
```

---

### 2. Use Lambdas for Flexibility

```cpp
// ✓ GOOD: Lambda weight map
auto weight = [&g](const auto& e) { return g[e].weight; };

// ✗ BAD: Unnecessary class
struct WeightMap {
    const Graph& g;
    double operator()(const Edge& e) const { return g[e].weight; }
};
```

---

### 3. Prefer const for Read-Only Maps

```cpp
// ✓ GOOD: const lambda for read-only
auto weight = [&g](const auto& e) -> double {
    return g[e].weight;
};

// ✓ GOOD: mutable lambda for read-write
auto color = [&colors](std::size_t v) -> int& {
    return colors[v];
};
```

---

### 4. Document Property Requirements

```cpp
/// Finds shortest paths using custom weight function
/// @tparam G Graph type (IncidenceGraph)
/// @tparam WeightMap Callable: edge_descriptor -> double
template<IncidenceGraph G, typename WeightMap>
auto custom_dijkstra(const G& g, vertex_descriptor_t<G> start, 
                     WeightMap weight);
```

---

## Performance Considerations

### Bundled Properties
- **Access**: O(1), direct
- **Memory**: Contiguous, cache-friendly
- **Best for**: Always-present properties

### External Vectors
- **Access**: O(1), one indirection
- **Memory**: Separate allocation
- **Best for**: Temporary algorithm data

### Map-Based
- **Access**: O(log n) or O(1) average
- **Memory**: Sparse, overhead per entry
- **Best for**: Rare properties

### Lambda/Computed
- **Access**: Varies (can be expensive)
- **Memory**: None (computed on demand)
- **Best for**: Derived properties

---

## Complete Example

```cpp
#include <bgl/modern/adjacency_list.hpp>
#include <bgl/modern/dijkstra_shortest_paths.hpp>
#include <iostream>
#include <vector>
#include <map>

using namespace bgl;

struct VertexProps {
    std::string name;
    double x, y;
};

struct EdgeProps {
    double weight;
};

int main() {
    // Bundled properties
    adjacency_list<directed_tag, VertexProps, EdgeProps> g(5);
    
    g[0] = {"Start", 0.0, 0.0};
    g[1] = {"A", 1.0, 1.0};
    g[2] = {"B", 2.0, 2.0};
    
    g.add_edge(0, 1, EdgeProps{1.5});
    g.add_edge(1, 2, EdgeProps{2.5});
    
    // External property: visit count
    std::map<std::size_t, int> visit_count;
    
    // Lambda weight map
    auto weight_map = [&g](const auto& e) {
        return g[e].weight;
    };
    
    // Run algorithm
    auto result = dijkstra_shortest_paths(g, 0, weight_map);
    
    // Process results
    for (auto v : vertices(g)) {
        std::cout << g[v].name << ": distance = " 
                  << result.distance_map()(v) << "\n";
        visit_count[v]++;
    }
    
    return 0;
}
```

---

## Summary

| Pattern | When to Use | Performance |
|---------|-------------|-------------|
| Bundled Properties | Core graph data | ★★★★★ |
| External Vectors | Algorithm temps | ★★★★☆ |
| Maps (sparse) | Rare properties | ★★★☆☆ |
| Lambda/Computed | Derived values | ★★☆☆☆ to ★★★★★ |
| Algorithm Results | Reusing output | ★★★★★ |

Choose the pattern that best fits your use case for optimal performance and clarity.
