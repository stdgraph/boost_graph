// BGL Modern - Concept Tests
// Verify C++20 graph concepts work correctly
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>

#include <vector>
#include <ranges>
#include <utility>
#include <iostream>
#include <cstddef>

// =============================================================================
// Test Graph Implementation
// =============================================================================

namespace test {

/// A simple adjacency list graph for testing concepts
struct simple_graph {
    using vertex_descriptor = std::size_t;
    using edge_descriptor = std::pair<vertex_descriptor, vertex_descriptor>;
    using directed_category = bgl::directed_tag;
    using edge_parallel_category = void;  // placeholder
    using traversal_category = void;      // placeholder
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
    
    struct vertex_property {
        double weight = 0.0;
    };
    
    struct edge_property {
        double cost = 1.0;
    };
    
    // Adjacency list: for each vertex, list of (target, edge_property)
    std::vector<std::vector<std::pair<vertex_descriptor, edge_property>>> adj_;
    std::vector<vertex_property> vertex_props_;
    
    simple_graph() = default;
    explicit simple_graph(std::size_t n) : adj_(n), vertex_props_(n) {}
    
    // Property access
    vertex_property& operator[](vertex_descriptor v) { return vertex_props_[v]; }
    const vertex_property& operator[](vertex_descriptor v) const { return vertex_props_[v]; }
    
    // Edge property access (simplified - returns mutable reference)
    edge_property& operator[](edge_descriptor e) {
        for (auto& [target, prop] : adj_[e.first]) {
            if (target == e.second) return prop;
        }
        static edge_property dummy;
        return dummy;
    }
    const edge_property& operator[](edge_descriptor e) const {
        for (const auto& [target, prop] : adj_[e.first]) {
            if (target == e.second) return prop;
        }
        static const edge_property dummy;
        return dummy;
    }
};

// =============================================================================
// Free Functions for simple_graph (ADL-findable)
// =============================================================================

// VertexListGraph requirements
inline auto vertices(const simple_graph& g) {
    return std::views::iota(std::size_t{0}, g.adj_.size());
}

inline std::size_t num_vertices(const simple_graph& g) {
    return g.adj_.size();
}

// EdgeListGraph requirements
inline auto edges(const simple_graph& g) {
    std::vector<simple_graph::edge_descriptor> result;
    for (std::size_t u = 0; u < g.adj_.size(); ++u) {
        for (const auto& [v, prop] : g.adj_[u]) {
            result.emplace_back(u, v);
        }
    }
    return result;
}

inline std::size_t num_edges(const simple_graph& g) {
    std::size_t count = 0;
    for (const auto& adj : g.adj_) {
        count += adj.size();
    }
    return count;
}

// IncidenceGraph requirements
inline auto out_edges(simple_graph::vertex_descriptor v, const simple_graph& g) {
    std::vector<simple_graph::edge_descriptor> result;
    for (const auto& [target, prop] : g.adj_[v]) {
        result.emplace_back(v, target);
    }
    return result;
}

inline std::size_t out_degree(simple_graph::vertex_descriptor v, const simple_graph& g) {
    return g.adj_[v].size();
}

inline simple_graph::vertex_descriptor source(
    simple_graph::edge_descriptor e, 
    [[maybe_unused]] const simple_graph& g
) {
    return e.first;
}

inline simple_graph::vertex_descriptor target(
    simple_graph::edge_descriptor e, 
    [[maybe_unused]] const simple_graph& g
) {
    return e.second;
}

// AdjacencyGraph requirements
inline auto adjacent_vertices(simple_graph::vertex_descriptor v, const simple_graph& g) {
    std::vector<simple_graph::vertex_descriptor> result;
    for (const auto& [target, prop] : g.adj_[v]) {
        result.push_back(target);
    }
    return result;
}

// MutableGraph requirements
inline simple_graph::vertex_descriptor add_vertex(simple_graph& g) {
    auto v = g.adj_.size();
    g.adj_.emplace_back();
    g.vertex_props_.emplace_back();
    return v;
}

inline void remove_vertex(simple_graph::vertex_descriptor v, simple_graph& g) {
    // Simple implementation: just clear adjacency (doesn't compact)
    g.adj_[v].clear();
    // Remove edges pointing to v
    for (auto& adj : g.adj_) {
        std::erase_if(adj, [v](const auto& p) { return p.first == v; });
    }
}

inline std::pair<simple_graph::edge_descriptor, bool> add_edge(
    simple_graph::vertex_descriptor u,
    simple_graph::vertex_descriptor v,
    simple_graph& g
) {
    g.adj_[u].emplace_back(v, simple_graph::edge_property{});
    return {{u, v}, true};
}

inline void remove_edge(
    simple_graph::vertex_descriptor u,
    simple_graph::vertex_descriptor v,
    simple_graph& g
) {
    std::erase_if(g.adj_[u], [v](const auto& p) { return p.first == v; });
}

inline void remove_edge(simple_graph::edge_descriptor e, simple_graph& g) {
    remove_edge(e.first, e.second, g);
}

} // namespace test

// =============================================================================
// Concept Static Assertions
// =============================================================================

// Verify simple_graph satisfies all concepts
static_assert(bgl::Graph<test::simple_graph>, 
    "simple_graph must satisfy Graph concept");

static_assert(bgl::VertexListGraph<test::simple_graph>, 
    "simple_graph must satisfy VertexListGraph concept");

static_assert(bgl::EdgeListGraph<test::simple_graph>, 
    "simple_graph must satisfy EdgeListGraph concept");

static_assert(bgl::IncidenceGraph<test::simple_graph>, 
    "simple_graph must satisfy IncidenceGraph concept");

static_assert(bgl::AdjacencyGraph<test::simple_graph>, 
    "simple_graph must satisfy AdjacencyGraph concept");

static_assert(bgl::MutableGraph<test::simple_graph>, 
    "simple_graph must satisfy MutableGraph concept");

static_assert(bgl::MutableIncidenceGraph<test::simple_graph>, 
    "simple_graph must satisfy MutableIncidenceGraph concept");

static_assert(bgl::VertexPropertyGraph<test::simple_graph>, 
    "simple_graph must satisfy VertexPropertyGraph concept");

static_assert(bgl::EdgePropertyGraph<test::simple_graph>, 
    "simple_graph must satisfy EdgePropertyGraph concept");

static_assert(bgl::PropertyGraph<test::simple_graph>, 
    "simple_graph must satisfy PropertyGraph concept");

static_assert(bgl::TraversableGraph<test::simple_graph>, 
    "simple_graph must satisfy TraversableGraph concept");

// =============================================================================
// Runtime Tests
// =============================================================================

void test_graph_construction() {
    test::simple_graph g(5);
    
    if (num_vertices(g) != 5) {
        std::cerr << "Graph construction failed: expected 5 vertices" << std::endl;
        std::exit(1);
    }
    
    if (num_edges(g) != 0) {
        std::cerr << "Graph construction failed: expected 0 edges" << std::endl;
        std::exit(1);
    }
}

void test_add_edges() {
    test::simple_graph g(4);
    
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(1, 2, g);
    add_edge(2, 3, g);
    
    if (num_edges(g) != 4) {
        std::cerr << "Add edges failed: expected 4 edges, got " << num_edges(g) << std::endl;
        std::exit(1);
    }
    
    if (out_degree(0, g) != 2) {
        std::cerr << "out_degree failed: expected 2, got " << out_degree(0, g) << std::endl;
        std::exit(1);
    }
}

void test_vertex_iteration() {
    test::simple_graph g(5);
    
    int count = 0;
    for (auto v : vertices(g)) {
        (void)v;
        ++count;
    }
    
    if (count != 5) {
        std::cerr << "Vertex iteration failed: expected 5, got " << count << std::endl;
        std::exit(1);
    }
}

void test_edge_iteration() {
    test::simple_graph g(3);
    add_edge(0, 1, g);
    add_edge(1, 2, g);
    add_edge(0, 2, g);
    
    int count = 0;
    for (auto e : edges(g)) {
        (void)e;
        ++count;
    }
    
    if (count != 3) {
        std::cerr << "Edge iteration failed: expected 3, got " << count << std::endl;
        std::exit(1);
    }
}

void test_out_edges_iteration() {
    test::simple_graph g(4);
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(0, 3, g);
    
    int count = 0;
    for (auto e : out_edges(0, g)) {
        if (source(e, g) != 0) {
            std::cerr << "out_edges source mismatch" << std::endl;
            std::exit(1);
        }
        ++count;
    }
    
    if (count != 3) {
        std::cerr << "out_edges iteration failed: expected 3, got " << count << std::endl;
        std::exit(1);
    }
}

void test_adjacent_vertices() {
    test::simple_graph g(4);
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(0, 3, g);
    
    int count = 0;
    for (auto v : adjacent_vertices(0, g)) {
        (void)v;
        ++count;
    }
    
    if (count != 3) {
        std::cerr << "adjacent_vertices failed: expected 3, got " << count << std::endl;
        std::exit(1);
    }
}

void test_mutable_operations() {
    test::simple_graph g;
    
    auto v0 = add_vertex(g);
    auto v1 = add_vertex(g);
    auto v2 = add_vertex(g);
    
    if (num_vertices(g) != 3) {
        std::cerr << "add_vertex failed" << std::endl;
        std::exit(1);
    }
    
    add_edge(v0, v1, g);
    add_edge(v1, v2, g);
    
    if (num_edges(g) != 2) {
        std::cerr << "add_edge failed" << std::endl;
        std::exit(1);
    }
    
    remove_edge(v0, v1, g);
    
    if (num_edges(g) != 1) {
        std::cerr << "remove_edge failed" << std::endl;
        std::exit(1);
    }
}

void test_property_access() {
    test::simple_graph g(3);
    
    g[0].weight = 1.5;
    g[1].weight = 2.5;
    g[2].weight = 3.5;
    
    if (g[0].weight != 1.5 || g[1].weight != 2.5 || g[2].weight != 3.5) {
        std::cerr << "Vertex property access failed" << std::endl;
        std::exit(1);
    }
    
    add_edge(0, 1, g);
    test::simple_graph::edge_descriptor e{0, 1};
    g[e].cost = 99.0;
    
    if (g[e].cost != 99.0) {
        std::cerr << "Edge property access failed" << std::endl;
        std::exit(1);
    }
}

void test_ranges_integration() {
    test::simple_graph g(10);
    for (std::size_t i = 0; i < 9; ++i) {
        add_edge(i, i + 1, g);
        g[i].weight = static_cast<double>(i);
    }
    g[9].weight = 9.0;
    
    // Filter vertices with weight > 5
    auto filtered = vertices(g) 
        | std::views::filter([&](auto v) { return g[v].weight > 5.0; });
    
    int count = 0;
    for (auto v : filtered) {
        (void)v;
        ++count;
    }
    
    if (count != 4) {  // vertices 6, 7, 8, 9
        std::cerr << "Ranges filter failed: expected 4, got " << count << std::endl;
        std::exit(1);
    }
}

int main() {
    std::cout << "Testing BGL Modern Concepts..." << std::endl;
    std::cout << std::endl;
    
    std::cout << "Static assertions passed:" << std::endl;
    std::cout << "  [PASS] Graph concept" << std::endl;
    std::cout << "  [PASS] VertexListGraph concept" << std::endl;
    std::cout << "  [PASS] EdgeListGraph concept" << std::endl;
    std::cout << "  [PASS] IncidenceGraph concept" << std::endl;
    std::cout << "  [PASS] AdjacencyGraph concept" << std::endl;
    std::cout << "  [PASS] MutableGraph concept" << std::endl;
    std::cout << "  [PASS] MutableIncidenceGraph concept" << std::endl;
    std::cout << "  [PASS] VertexPropertyGraph concept" << std::endl;
    std::cout << "  [PASS] EdgePropertyGraph concept" << std::endl;
    std::cout << "  [PASS] PropertyGraph concept" << std::endl;
    std::cout << "  [PASS] TraversableGraph concept" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Runtime tests:" << std::endl;
    
    test_graph_construction();
    std::cout << "  [PASS] Graph construction" << std::endl;
    
    test_add_edges();
    std::cout << "  [PASS] Add edges" << std::endl;
    
    test_vertex_iteration();
    std::cout << "  [PASS] Vertex iteration" << std::endl;
    
    test_edge_iteration();
    std::cout << "  [PASS] Edge iteration" << std::endl;
    
    test_out_edges_iteration();
    std::cout << "  [PASS] Out-edges iteration" << std::endl;
    
    test_adjacent_vertices();
    std::cout << "  [PASS] Adjacent vertices" << std::endl;
    
    test_mutable_operations();
    std::cout << "  [PASS] Mutable operations" << std::endl;
    
    test_property_access();
    std::cout << "  [PASS] Property access" << std::endl;
    
    test_ranges_integration();
    std::cout << "  [PASS] Ranges integration" << std::endl;
    
    std::cout << std::endl;
    std::cout << "All concept tests passed!" << std::endl;
    
    return 0;
}
