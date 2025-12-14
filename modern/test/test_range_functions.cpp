// BGL Modern - Range Functions Test
// Tests for range-returning free functions
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <bgl/modern/range_functions.hpp>
#include <bgl/modern/concepts.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <ranges>
#include <vector>
#include <utility>

namespace {

// =============================================================================
// Test Graph Implementation
// =============================================================================

/// A simple adjacency list graph for testing range functions.
/// Supports vertices, edges, out_edges, in_edges, and adjacent_vertices.
class test_graph {
public:
    // Required type aliases for graph_traits
    using vertex_descriptor = std::size_t;
    using edge_descriptor = std::pair<vertex_descriptor, vertex_descriptor>;
    using directed_category = bgl::bidirectional_tag;
    using edge_parallel_category = bgl::allow_parallel_edge_tag;
    using traversal_category = void;  // Full traversal support
    
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
    
private:
    std::size_t num_verts_ = 0;
    std::vector<edge_descriptor> edges_;
    // Adjacency lists (out-edges for each vertex)
    std::vector<std::vector<edge_descriptor>> out_edges_;
    // In-edge adjacency lists
    std::vector<std::vector<edge_descriptor>> in_edges_;
    // Adjacent vertices cache
    std::vector<std::vector<vertex_descriptor>> adj_verts_;

public:
    test_graph() = default;
    
    explicit test_graph(std::size_t num_vertices) 
        : num_verts_(num_vertices)
        , out_edges_(num_vertices)
        , in_edges_(num_vertices)
        , adj_verts_(num_vertices)
    {}
    
    void add_edge(vertex_descriptor u, vertex_descriptor v) {
        edge_descriptor e{u, v};
        edges_.push_back(e);
        out_edges_[u].push_back(e);
        in_edges_[v].push_back(e);
        adj_verts_[u].push_back(v);
    }
    
    // Vertex range interface
    auto vertex_begin() const { 
        return std::views::iota(vertex_descriptor{0}, num_verts_).begin(); 
    }
    auto vertex_end() const { 
        return std::views::iota(vertex_descriptor{0}, num_verts_).end(); 
    }
    std::size_t num_vertices() const { return num_verts_; }
    
    // Edge range interface
    auto edge_begin() const { return edges_.begin(); }
    auto edge_end() const { return edges_.end(); }
    std::size_t num_edges() const { return edges_.size(); }
    
    // Out-edge interface
    auto out_edge_begin(vertex_descriptor v) const { return out_edges_[v].begin(); }
    auto out_edge_end(vertex_descriptor v) const { return out_edges_[v].end(); }
    std::size_t out_degree(vertex_descriptor v) const { return out_edges_[v].size(); }
    
    // In-edge interface
    auto in_edge_begin(vertex_descriptor v) const { return in_edges_[v].begin(); }
    auto in_edge_end(vertex_descriptor v) const { return in_edges_[v].end(); }
    std::size_t in_degree(vertex_descriptor v) const { return in_edges_[v].size(); }
    
    // Adjacent vertices interface
    auto adjacent_vertex_begin(vertex_descriptor v) const { return adj_verts_[v].begin(); }
    auto adjacent_vertex_end(vertex_descriptor v) const { return adj_verts_[v].end(); }
    
    // Edge endpoint accessors
    vertex_descriptor source(edge_descriptor e) const { return e.first; }
    vertex_descriptor target(edge_descriptor e) const { return e.second; }
};

// =============================================================================
// Concept Verification
// =============================================================================

// Verify test_graph satisfies all range concepts
static_assert(bgl::HasVertexRange<test_graph>);
static_assert(bgl::HasEdgeRange<test_graph>);
static_assert(bgl::HasOutEdgeRange<test_graph>);
static_assert(bgl::HasInEdgeRange<test_graph>);
static_assert(bgl::HasAdjacentVertexRange<test_graph>);

// Verify graph concepts from concepts.hpp
static_assert(bgl::Graph<test_graph>);
static_assert(bgl::VertexListGraph<test_graph>);
static_assert(bgl::EdgeListGraph<test_graph>);
static_assert(bgl::IncidenceGraph<test_graph>);
static_assert(bgl::BidirectionalGraph<test_graph>);
static_assert(bgl::AdjacencyGraph<test_graph>);

// =============================================================================
// Test Functions
// =============================================================================

void test_vertices_range() {
    std::cout << "Testing vertices(g)..." << std::endl;
    
    test_graph g(5);
    
    // Test basic iteration
    std::vector<std::size_t> verts;
    for (auto v : bgl::vertices(g)) {
        verts.push_back(v);
    }
    assert(verts.size() == 5);
    assert(verts == (std::vector<std::size_t>{0, 1, 2, 3, 4}));
    
    // Test with std::ranges::for_each
    std::size_t sum = 0;
    std::ranges::for_each(bgl::vertices(g), [&sum](auto v) {
        sum += v;
    });
    assert(sum == 0 + 1 + 2 + 3 + 4);
    
    // Test with std::views::filter
    auto even_vertices = bgl::vertices(g) | std::views::filter([](auto v) {
        return v % 2 == 0;
    });
    std::vector<std::size_t> evens;
    for (auto v : even_vertices) {
        evens.push_back(v);
    }
    assert(evens == (std::vector<std::size_t>{0, 2, 4}));
    
    // Test num_vertices
    assert(bgl::num_vertices(g) == 5);
    
    std::cout << "  PASSED" << std::endl;
}

void test_edges_range() {
    std::cout << "Testing edges(g)..." << std::endl;
    
    test_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    g.add_edge(0, 3);
    
    // Test basic iteration
    std::size_t edge_count = 0;
    for ([[maybe_unused]] auto e : bgl::edges(g)) {
        ++edge_count;
    }
    assert(edge_count == 4);
    
    // Test with std::ranges::count_if
    auto count = std::ranges::count_if(bgl::edges(g), [](auto e) {
        return e.first == 0;  // edges from vertex 0
    });
    assert(count == 2);  // (0,1) and (0,3)
    
    // Test with std::views::transform
    auto targets = bgl::edges(g) | std::views::transform([](auto e) {
        return e.second;
    });
    std::vector<std::size_t> target_vec;
    for (auto t : targets) {
        target_vec.push_back(t);
    }
    assert(target_vec == (std::vector<std::size_t>{1, 2, 3, 3}));
    
    // Test num_edges
    assert(bgl::num_edges(g) == 4);
    
    std::cout << "  PASSED" << std::endl;
}

void test_out_edges_range() {
    std::cout << "Testing out_edges(v, g)..." << std::endl;
    
    test_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(0, 3);
    g.add_edge(1, 2);
    
    // Test out_edges from vertex 0
    std::size_t count = 0;
    for (auto e : bgl::out_edges(std::size_t{0}, g)) {
        assert(bgl::source(e, g) == 0);
        ++count;
    }
    assert(count == 3);
    
    // Test out_degree
    assert(bgl::out_degree(std::size_t{0}, g) == 3);
    assert(bgl::out_degree(std::size_t{1}, g) == 1);
    assert(bgl::out_degree(std::size_t{2}, g) == 0);
    
    // Test with std::views::transform to get targets
    auto neighbors = bgl::out_edges(std::size_t{0}, g) 
        | std::views::transform([&g](auto e) { return bgl::target(e, g); });
    std::vector<std::size_t> neighbor_vec;
    for (auto n : neighbors) {
        neighbor_vec.push_back(n);
    }
    assert(neighbor_vec == (std::vector<std::size_t>{1, 2, 3}));
    
    std::cout << "  PASSED" << std::endl;
}

void test_in_edges_range() {
    std::cout << "Testing in_edges(v, g)..." << std::endl;
    
    test_graph g(4);
    g.add_edge(0, 3);
    g.add_edge(1, 3);
    g.add_edge(2, 3);
    
    // Test in_edges to vertex 3
    std::size_t count = 0;
    for (auto e : bgl::in_edges(std::size_t{3}, g)) {
        assert(bgl::target(e, g) == 3);
        ++count;
    }
    assert(count == 3);
    
    // Test in_degree
    assert(bgl::in_degree(std::size_t{3}, g) == 3);
    assert(bgl::in_degree(std::size_t{0}, g) == 0);
    
    // Test degree (in + out)
    g.add_edge(3, 0);  // Add an out-edge from vertex 3
    assert(bgl::degree(std::size_t{3}, g) == 4);  // 3 in + 1 out
    
    std::cout << "  PASSED" << std::endl;
}

void test_adjacent_vertices_range() {
    std::cout << "Testing adjacent_vertices(v, g)..." << std::endl;
    
    test_graph g(5);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(0, 3);
    g.add_edge(1, 4);
    
    // Test adjacent_vertices from vertex 0
    std::vector<std::size_t> adj;
    for (auto v : bgl::adjacent_vertices(std::size_t{0}, g)) {
        adj.push_back(v);
    }
    assert(adj == (std::vector<std::size_t>{1, 2, 3}));
    
    // Test with std::views::take
    auto first_two = bgl::adjacent_vertices(std::size_t{0}, g) | std::views::take(2);
    std::vector<std::size_t> first_two_vec;
    for (auto v : first_two) {
        first_two_vec.push_back(v);
    }
    assert(first_two_vec == (std::vector<std::size_t>{1, 2}));
    
    // Test vertex with no neighbors
    std::vector<std::size_t> no_adj;
    for (auto v : bgl::adjacent_vertices(std::size_t{4}, g)) {
        no_adj.push_back(v);
    }
    assert(no_adj.empty());
    
    std::cout << "  PASSED" << std::endl;
}

void test_out_neighbors_helper() {
    std::cout << "Testing out_neighbors(v, g)..." << std::endl;
    
    test_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(0, 3);
    
    // Test out_neighbors - directly get target vertices
    std::vector<std::size_t> neighbors;
    for (auto v : bgl::out_neighbors(std::size_t{0}, g)) {
        neighbors.push_back(v);
    }
    assert(neighbors == (std::vector<std::size_t>{1, 2, 3}));
    
    std::cout << "  PASSED" << std::endl;
}

void test_in_neighbors_helper() {
    std::cout << "Testing in_neighbors(v, g)..." << std::endl;
    
    test_graph g(4);
    g.add_edge(0, 3);
    g.add_edge(1, 3);
    g.add_edge(2, 3);
    
    // Test in_neighbors - directly get source vertices
    std::vector<std::size_t> predecessors;
    for (auto v : bgl::in_neighbors(std::size_t{3}, g)) {
        predecessors.push_back(v);
    }
    assert(predecessors == (std::vector<std::size_t>{0, 1, 2}));
    
    std::cout << "  PASSED" << std::endl;
}

void test_ranges_algorithms_compatibility() {
    std::cout << "Testing compatibility with std::ranges algorithms..." << std::endl;
    
    test_graph g(6);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    g.add_edge(2, 4);
    g.add_edge(3, 5);
    g.add_edge(4, 5);
    
    // std::ranges::find
    auto verts = bgl::vertices(g);
    auto found = std::ranges::find(verts, std::size_t{3});
    assert(found != verts.end());
    assert(*found == 3);
    
    // std::ranges::count_if on edges
    auto high_source_edges = std::ranges::count_if(bgl::edges(g), [](auto e) {
        return e.first >= 2;
    });
    assert(high_source_edges == 3);  // (2,4), (3,5), (4,5)
    
    // std::ranges::any_of
    bool has_vertex_5 = std::ranges::any_of(bgl::vertices(g), [](auto v) {
        return v == 5;
    });
    assert(has_vertex_5);
    
    // std::ranges::all_of
    bool all_valid = std::ranges::all_of(bgl::vertices(g), [](auto v) {
        return v < 10;
    });
    assert(all_valid);
    
    // std::ranges::none_of
    bool no_negative = std::ranges::none_of(bgl::vertices(g), [](auto v) {
        return v > 100;  // All are small
    });
    assert(no_negative);
    
    // Chained views: filter + transform
    auto filtered_neighbors = bgl::vertices(g) 
        | std::views::filter([](auto v) { return v < 3; })
        | std::views::transform([&g](auto v) { 
            return bgl::out_degree(v, g);
        });
    
    std::vector<std::size_t> degrees;
    for (auto d : filtered_neighbors) {
        degrees.push_back(d);
    }
    assert(degrees == (std::vector<std::size_t>{2, 1, 1}));  // degrees of v0, v1, v2
    
    std::cout << "  PASSED" << std::endl;
}

} // anonymous namespace

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "=== BGL Modern Range Functions Tests ===" << std::endl;
    
    test_vertices_range();
    test_edges_range();
    test_out_edges_range();
    test_in_edges_range();
    test_adjacent_vertices_range();
    test_out_neighbors_helper();
    test_in_neighbors_helper();
    test_ranges_algorithms_compatibility();
    
    std::cout << "\n=== All range function tests passed! ===" << std::endl;
    return 0;
}
