// BGL Modern Example: Custom Graph Type
// Demonstrates how to create a custom graph type that works with BGL Modern
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <bgl/modern/breadth_first_search.hpp>
#include <bgl/modern/graph_traits.hpp>
#include <bgl/modern/concepts.hpp>
#include <iostream>
#include <vector>
#include <ranges>

// Custom graph implementation: Simple adjacency list
class SimpleGraph {
public:
    // Required type definitions for BGL Modern
    using vertex_descriptor = std::size_t;
    using edge_descriptor = std::pair<std::size_t, std::size_t>;
    using directed_category = bgl::directed_tag;
    using edge_parallel_category = bgl::allow_parallel_edge_tag;
    using traversal_category = void;  // Can be omitted
    
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
    
private:
    std::vector<std::vector<std::size_t>> adjacency_;
    
public:
    explicit SimpleGraph(std::size_t num_vertices) 
        : adjacency_(num_vertices) {}
    
    void add_edge(std::size_t u, std::size_t v) {
        adjacency_[u].push_back(v);
    }
    
    std::size_t num_vertices() const {
        return adjacency_.size();
    }
    
    const std::vector<std::size_t>& neighbors(std::size_t v) const {
        return adjacency_[v];
    }
};

// Free functions required by BGL Modern concepts
// These are found via ADL (Argument-Dependent Lookup)

inline std::size_t num_vertices(const SimpleGraph& g) {
    return g.num_vertices();
}

inline auto vertices(const SimpleGraph& g) {
    return std::ranges::iota_view(std::size_t{0}, g.num_vertices());
}

inline std::size_t out_degree(std::size_t v, const SimpleGraph& g) {
    return g.neighbors(v).size();
}

inline auto out_edges(std::size_t v, const SimpleGraph& g) {
    return g.neighbors(v) 
        | std::views::transform([v](std::size_t u) {
            return SimpleGraph::edge_descriptor{v, u};
        });
}

inline std::size_t source(const SimpleGraph::edge_descriptor& e, const SimpleGraph&) {
    return e.first;
}

inline std::size_t target(const SimpleGraph::edge_descriptor& e, const SimpleGraph&) {
    return e.second;
}

// Verify that SimpleGraph satisfies BGL Modern concepts
static_assert(bgl::Graph<SimpleGraph>);
static_assert(bgl::IncidenceGraph<SimpleGraph>);
static_assert(bgl::VertexListGraph<SimpleGraph>);

int main() {
    std::cout << "=== Custom Graph Type Example ===\n\n";
    
    // Create a simple graph
    SimpleGraph g(7);
    
    // Build a binary tree
    g.add_edge(0, 1);  // root -> left
    g.add_edge(0, 2);  // root -> right
    g.add_edge(1, 3);  // left -> left-left
    g.add_edge(1, 4);  // left -> left-right
    g.add_edge(2, 5);  // right -> right-left
    g.add_edge(2, 6);  // right -> right-right
    
    std::cout << "Graph structure:\n";
    for (auto v : vertices(g)) {
        std::cout << "  Vertex " << v << " -> ";
        for (auto e : out_edges(v, g)) {
            std::cout << target(e, g) << " ";
        }
        std::cout << "\n";
    }
    
    // Demonstrate that BGL Modern algorithms work with custom graph
    std::cout << "\nBFS traversal from vertex 0:\n";
    
    std::vector<std::size_t> discovery_order;
    
    auto callbacks = bgl::on_discover_vertex([&](auto v, const auto&) {
        discovery_order.push_back(v);
        std::cout << "  Discovered: " << v << "\n";
    });
    
    bgl::breadth_first_search(g, 0, callbacks);
    
    std::cout << "\nDiscovery order: ";
    for (auto v : discovery_order) {
        std::cout << v << " ";
    }
    std::cout << "\n";
    
    // Get BFS result
    auto result = bgl::breadth_first_search(g, 0);
    
    std::cout << "\nDistances from root:\n";
    for (auto v : vertices(g)) {
        std::cout << "  Vertex " << v << ": distance = " 
                  << result.distance_map()(v) << "\n";
    }
    
    // Demonstrate range operations
    std::cout << "\nLeaf nodes (out-degree = 0):\n";
    auto leaf_nodes = vertices(g)
        | std::views::filter([&g](auto v) {
            return out_degree(v, g) == 0;
        });
    
    for (auto v : leaf_nodes) {
        std::cout << "  Vertex " << v << "\n";
    }
    
    std::cout << "\n✓ Example complete!\n";
    std::cout << "\nThis demonstrates that any type satisfying BGL Modern\n";
    std::cout << "concepts can use the full algorithm library!\n";
    
    return 0;
}
