// BGL Modern - Breadth-First Search Tests
// Verify the C++20 BFS implementation works correctly
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <bgl/modern/breadth_first_search.hpp>

#include <iostream>
#include <cassert>
#include <vector>
#include <ranges>

// =============================================================================
// Test Graph Implementation
// =============================================================================

namespace test {

struct edge_desc {
    std::size_t source;
    std::size_t target;
    std::size_t index;
    bool operator==(const edge_desc&) const = default;
};

struct simple_graph {
    using vertex_descriptor = std::size_t;
    using edge_descriptor = edge_desc;
    using directed_category = bgl::directed_tag;
    using edge_parallel_category = bgl::allow_parallel_edge_tag;
    using traversal_category = void;
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
    
    struct stored_edge {
        vertex_descriptor target;
        std::size_t index;
    };
    
    std::size_t num_vertices_ = 0;
    std::vector<std::vector<stored_edge>> adj_;
    std::size_t edge_count_ = 0;
    
    simple_graph() = default;
    explicit simple_graph(std::size_t n) : num_vertices_(n), adj_(n) {}
    
    void add_edge(vertex_descriptor u, vertex_descriptor v) {
        adj_[u].push_back({v, edge_count_++});
    }
};

inline std::size_t num_vertices(const simple_graph& g) {
    return g.num_vertices_;
}

inline auto vertices(const simple_graph& g) {
    return std::views::iota(std::size_t{0}, g.num_vertices_);
}

struct out_edge_iterator {
    using value_type = edge_desc;
    using difference_type = std::ptrdiff_t;
    using pointer = const edge_desc*;
    using reference = edge_desc;
    using iterator_category = std::forward_iterator_tag;
    
    std::size_t source_{};
    std::vector<simple_graph::stored_edge>::const_iterator it_{};
    
    out_edge_iterator() = default;
    out_edge_iterator(std::size_t src, std::vector<simple_graph::stored_edge>::const_iterator it)
        : source_(src), it_(it) {}
    
    edge_desc operator*() const { return {source_, it_->target, it_->index}; }
    out_edge_iterator& operator++() { ++it_; return *this; }
    out_edge_iterator operator++(int) { auto tmp = *this; ++it_; return tmp; }
    bool operator==(const out_edge_iterator& o) const { return it_ == o.it_; }
    bool operator!=(const out_edge_iterator& o) const { return it_ != o.it_; }
};

struct out_edge_range {
    std::size_t source_;
    const std::vector<simple_graph::stored_edge>* adj_;
    out_edge_iterator begin() const { return {source_, adj_->begin()}; }
    out_edge_iterator end() const { return {source_, adj_->end()}; }
};

inline out_edge_range out_edges(simple_graph::vertex_descriptor v, const simple_graph& g) {
    return {v, &g.adj_[v]};
}

inline std::size_t out_degree(simple_graph::vertex_descriptor v, const simple_graph& g) {
    return g.adj_[v].size();
}

inline simple_graph::vertex_descriptor source(edge_desc e, const simple_graph&) {
    return e.source;
}

inline simple_graph::vertex_descriptor target(edge_desc e, const simple_graph&) {
    return e.target;
}

} // namespace test

template<>
struct bgl::graph_traits<test::simple_graph> {
    using vertex_descriptor = test::simple_graph::vertex_descriptor;
    using edge_descriptor = test::edge_desc;
    using directed_category = bgl::directed_tag;
    using edge_parallel_category = bgl::allow_parallel_edge_tag;
    using traversal_category = void;
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
};

// =============================================================================
// Tests
// =============================================================================

namespace {

void test_simple_bfs() {
    // Linear graph: 0 -> 1 -> 2 -> 3
    test::simple_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    
    auto result = bgl::breadth_first_search(g, std::size_t{0});
    
    assert(result.source() == 0);
    assert(result.distance_to(0) == 0);
    assert(result.distance_to(1) == 1);
    assert(result.distance_to(2) == 2);
    assert(result.distance_to(3) == 3);
    
    // Check discovery order
    auto& order = result.discovered_vertices();
    assert(order.size() == 4);
    assert(order[0] == 0);
    assert(order[1] == 1);
    assert(order[2] == 2);
    assert(order[3] == 3);
    
    std::cout << "  Simple BFS: PASSED\n";
}

void test_bfs_tree() {
    // Tree structure:
    //       0
    //      / \
    //     1   2
    //    / \
    //   3   4
    test::simple_graph g(5);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    g.add_edge(1, 4);
    
    auto result = bgl::breadth_first_search(g, std::size_t{0});
    
    assert(result.distance_to(0) == 0);
    assert(result.distance_to(1) == 1);
    assert(result.distance_to(2) == 1);
    assert(result.distance_to(3) == 2);
    assert(result.distance_to(4) == 2);
    
    // Check predecessors
    assert(result.predecessor_of(1) == 0);
    assert(result.predecessor_of(2) == 0);
    assert(result.predecessor_of(3) == 1);
    assert(result.predecessor_of(4) == 1);
    
    std::cout << "  BFS tree: PASSED\n";
}

void test_bfs_path_reconstruction() {
    test::simple_graph g(5);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    g.add_edge(3, 4);
    
    auto result = bgl::breadth_first_search(g, std::size_t{0});
    
    auto path = result.path_to(4);
    assert(path.size() == 5);
    assert(path[0] == 0);
    assert(path[1] == 1);
    assert(path[2] == 2);
    assert(path[3] == 3);
    assert(path[4] == 4);
    
    std::cout << "  BFS path reconstruction: PASSED\n";
}

void test_bfs_unreachable() {
    test::simple_graph g(4);
    g.add_edge(0, 1);
    // Vertices 2, 3 are unreachable
    
    auto result = bgl::breadth_first_search(g, std::size_t{0});
    
    assert(result.is_reachable(0));
    assert(result.is_reachable(1));
    assert(!result.is_reachable(2));
    assert(!result.is_reachable(3));
    
    auto& order = result.discovered_vertices();
    assert(order.size() == 2);
    
    std::cout << "  BFS unreachable: PASSED\n";
}

void test_bfs_cycle() {
    // Cycle: 0 -> 1 -> 2 -> 0
    test::simple_graph g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 0);
    
    auto result = bgl::breadth_first_search(g, std::size_t{0});
    
    // All vertices should be discovered
    assert(result.is_reachable(0));
    assert(result.is_reachable(1));
    assert(result.is_reachable(2));
    
    // Distances should be shortest paths
    assert(result.distance_to(0) == 0);
    assert(result.distance_to(1) == 1);
    assert(result.distance_to(2) == 2);
    
    std::cout << "  BFS with cycle: PASSED\n";
}

void test_bfs_diamond() {
    // Diamond: 0 -> 1 -> 3
    //          0 -> 2 -> 3
    test::simple_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    g.add_edge(2, 3);
    
    auto result = bgl::breadth_first_search(g, std::size_t{0});
    
    assert(result.distance_to(3) == 2);  // Shortest path is 2 edges
    
    std::cout << "  BFS diamond: PASSED\n";
}

void test_bfs_single_vertex() {
    test::simple_graph g(1);
    
    auto result = bgl::breadth_first_search(g, std::size_t{0});
    
    assert(result.is_reachable(0));
    assert(result.distance_to(0) == 0);
    assert(result.discovered_vertices().size() == 1);
    
    std::cout << "  BFS single vertex: PASSED\n";
}

void test_bfs_colors() {
    test::simple_graph g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    
    auto result = bgl::breadth_first_search(g, std::size_t{0});
    
    // All visited vertices should be black after BFS completes
    assert(result.color_of(0) == bgl::vertex_color::black);
    assert(result.color_of(1) == bgl::vertex_color::black);
    assert(result.color_of(2) == bgl::vertex_color::black);
    
    std::cout << "  BFS colors: PASSED\n";
}

void test_bfs_path_length() {
    test::simple_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    
    auto result = bgl::breadth_first_search(g, std::size_t{0});
    
    auto len0 = result.path_length(0);
    auto len3 = result.path_length(3);
    
    assert(len0.has_value() && *len0 == 0);
    assert(len3.has_value() && *len3 == 3);
    
    std::cout << "  BFS path length: PASSED\n";
}

} // anonymous namespace

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "BGL Modern Breadth-First Search Tests\n";
    std::cout << "======================================\n\n";
    
    std::cout << "Basic Tests:\n";
    test_simple_bfs();
    test_bfs_tree();
    test_bfs_single_vertex();
    
    std::cout << "\nPath Tests:\n";
    test_bfs_path_reconstruction();
    test_bfs_path_length();
    
    std::cout << "\nGraph Structure Tests:\n";
    test_bfs_unreachable();
    test_bfs_cycle();
    test_bfs_diamond();
    
    std::cout << "\nProperty Tests:\n";
    test_bfs_colors();
    
    std::cout << "\n======================================\n";
    std::cout << "All BFS tests passed!\n";
    
    return 0;
}
