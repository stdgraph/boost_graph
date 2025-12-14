// BGL Modern - Dijkstra's Shortest Paths Tests
// Verify the C++20 Dijkstra implementation works correctly
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <bgl/modern/dijkstra_shortest_paths.hpp>

#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <ranges>

// =============================================================================
// Test Graph Implementation
// =============================================================================

namespace test {

// Edge descriptor that includes edge index for unique identification
struct edge_desc {
    std::size_t source;
    std::size_t target;
    std::size_t index;
    
    bool operator==(const edge_desc&) const = default;
};

/// A simple weighted directed graph for testing
struct weighted_graph {
    using vertex_descriptor = std::size_t;
    using edge_descriptor = edge_desc;
    using directed_category = bgl::directed_tag;
    using edge_parallel_category = bgl::allow_parallel_edge_tag;
    using traversal_category = void;
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
    
    struct edge_property {
        double weight = 1.0;
    };
    
    // Store edges with index for proper edge descriptors
    struct stored_edge {
        vertex_descriptor target;
        std::size_t edge_index;  // Global edge index
        edge_property prop;
    };
    
    // Adjacency list: vertex -> [stored_edge, ...]
    std::vector<std::vector<stored_edge>> adj_;
    std::size_t edge_count_ = 0;
    
    // Edge properties indexed by edge_index
    std::vector<edge_property> edge_props_;
    // Edge source/target for lookup
    std::vector<std::pair<vertex_descriptor, vertex_descriptor>> edge_endpoints_;
    
    weighted_graph() = default;
    explicit weighted_graph(std::size_t n) : adj_(n) {}
    
    void add_edge(vertex_descriptor u, vertex_descriptor v, double weight = 1.0) {
        std::size_t idx = edge_count_++;
        edge_property prop{weight};
        adj_[u].push_back({v, idx, prop});
        edge_props_.push_back(prop);
        edge_endpoints_.push_back({u, v});
    }
    
    // Edge property access via edge index
    const edge_property& get_edge_prop(std::size_t edge_idx) const {
        return edge_props_[edge_idx];
    }
    
    // Support bundled property access via operator[] for edges
    const edge_property& operator[](const edge_desc& e) const {
        return edge_props_[e.index];
    }
};

// Free functions for weighted_graph (ADL)

inline std::size_t num_vertices(const weighted_graph& g) {
    return g.adj_.size();
}

inline auto vertices(const weighted_graph& g) {
    return std::views::iota(std::size_t{0}, g.adj_.size());
}

// Out-edge iterator that satisfies forward_iterator
struct out_edge_iterator {
    using value_type = edge_desc;
    using difference_type = std::ptrdiff_t;
    using pointer = const edge_desc*;
    using reference = edge_desc;
    using iterator_category = std::forward_iterator_tag;
    
    std::size_t source_{};
    std::vector<weighted_graph::stored_edge>::const_iterator it_{};
    
    out_edge_iterator() = default;
    out_edge_iterator(std::size_t src, std::vector<weighted_graph::stored_edge>::const_iterator it)
        : source_(src), it_(it) {}
    
    edge_desc operator*() const {
        return {source_, it_->target, it_->edge_index};
    }
    
    out_edge_iterator& operator++() { ++it_; return *this; }
    out_edge_iterator operator++(int) { auto tmp = *this; ++it_; return tmp; }
    
    bool operator==(const out_edge_iterator& other) const { return it_ == other.it_; }
    bool operator!=(const out_edge_iterator& other) const { return it_ != other.it_; }
};

struct out_edge_range {
    std::size_t source_;
    const std::vector<weighted_graph::stored_edge>* adj_;
    
    out_edge_iterator begin() const { return {source_, adj_->begin()}; }
    out_edge_iterator end() const { return {source_, adj_->end()}; }
};

inline out_edge_range out_edges(weighted_graph::vertex_descriptor v, const weighted_graph& g) {
    return {v, &g.adj_[v]};
}

inline std::size_t out_degree(weighted_graph::vertex_descriptor v, const weighted_graph& g) {
    return g.adj_[v].size();
}

inline weighted_graph::vertex_descriptor source(edge_desc e, const weighted_graph&) {
    return e.source;
}

inline weighted_graph::vertex_descriptor target(edge_desc e, const weighted_graph&) {
    return e.target;
}

// Weight accessor that works with edge_desc
inline double get_weight(edge_desc e, const weighted_graph& g) {
    return g.get_edge_prop(e.index).weight;
}

} // namespace test

// Specialize graph_traits for our test graph
template<>
struct bgl::graph_traits<test::weighted_graph> {
    using vertex_descriptor = test::weighted_graph::vertex_descriptor;
    using edge_descriptor = test::edge_desc;
    using directed_category = bgl::directed_tag;
    using edge_parallel_category = bgl::allow_parallel_edge_tag;
    using traversal_category = void;
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
};

// =============================================================================
// Test Helpers
// =============================================================================

namespace {

constexpr double eps = 1e-9;

bool approx_equal(double a, double b) {
    return std::abs(a - b) < eps;
}

// =============================================================================
// Tests
// =============================================================================

void test_simple_path() {
    // Linear graph: 0 -> 1 -> 2 -> 3
    test::weighted_graph g(4);
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 2, 2.0);
    g.add_edge(2, 3, 3.0);
    
    auto result = bgl::dijkstra_shortest_paths(g, std::size_t{0});
    
    assert(result.source() == 0);
    assert(approx_equal(result.distance_to(0), 0.0));
    assert(approx_equal(result.distance_to(1), 1.0));
    assert(approx_equal(result.distance_to(2), 3.0));
    assert(approx_equal(result.distance_to(3), 6.0));
    
    // Check path to vertex 3
    auto path = result.path_to(3);
    assert(path.size() == 4);
    assert(path[0] == 0 && path[1] == 1 && path[2] == 2 && path[3] == 3);
    
    std::cout << "  Simple path: PASSED\n";
}

void test_branching_graph() {
    // Diamond graph:
    //     1
    //    / \
    //   0   3
    //    \ /
    //     2
    test::weighted_graph g(4);
    g.add_edge(0, 1, 1.0);
    g.add_edge(0, 2, 4.0);
    g.add_edge(1, 3, 2.0);
    g.add_edge(2, 3, 1.0);
    
    auto result = bgl::dijkstra_shortest_paths(g, std::size_t{0});
    
    assert(approx_equal(result.distance_to(0), 0.0));
    assert(approx_equal(result.distance_to(1), 1.0));
    assert(approx_equal(result.distance_to(2), 4.0));
    assert(approx_equal(result.distance_to(3), 3.0));  // Via 0->1->3, not 0->2->3
    
    // Path to 3 should go through 1
    auto path = result.path_to(3);
    assert(path.size() == 3);
    assert(path[0] == 0 && path[1] == 1 && path[2] == 3);
    
    std::cout << "  Branching graph: PASSED\n";
}

void test_unreachable_vertices() {
    // Disconnected graph: 0 -> 1, 2 -> 3
    test::weighted_graph g(4);
    g.add_edge(0, 1, 1.0);
    g.add_edge(2, 3, 1.0);
    
    auto result = bgl::dijkstra_shortest_paths(g, std::size_t{0});
    
    assert(result.is_reachable(0));
    assert(result.is_reachable(1));
    assert(!result.is_reachable(2));
    assert(!result.is_reachable(3));
    
    // Path to unreachable vertex should be empty
    auto path = result.path_to(2);
    assert(path.empty());
    
    std::cout << "  Unreachable vertices: PASSED\n";
}

void test_custom_weight_accessor() {
    test::weighted_graph g(3);
    g.add_edge(0, 1, 5.0);
    g.add_edge(1, 2, 3.0);
    
    // Use a lambda weight accessor with the new edge descriptor
    auto result = bgl::dijkstra_shortest_paths(g, std::size_t{0}, 
        [&g](test::edge_desc e) {
            return test::get_weight(e, g);
        });
    
    assert(approx_equal(result.distance_to(0), 0.0));
    assert(approx_equal(result.distance_to(1), 5.0));
    assert(approx_equal(result.distance_to(2), 8.0));
    
    std::cout << "  Custom weight accessor: PASSED\n";
}

void test_unit_weights() {
    // Test with default unit weights (ignoring actual edge weights)
    test::weighted_graph g(4);
    g.add_edge(0, 1, 100.0);  // High weight, but unit_weight ignores it
    g.add_edge(1, 2, 100.0);
    g.add_edge(2, 3, 100.0);
    
    auto result = bgl::dijkstra_shortest_paths(g, std::size_t{0}, bgl::unit_weight{});
    
    // With unit weights, each edge has weight 1.0
    assert(approx_equal(result.distance_to(0), 0.0));
    assert(approx_equal(result.distance_to(1), 1.0));
    assert(approx_equal(result.distance_to(2), 2.0));
    assert(approx_equal(result.distance_to(3), 3.0));
    
    std::cout << "  Unit weights: PASSED\n";
}

void test_single_vertex() {
    test::weighted_graph g(1);
    
    auto result = bgl::dijkstra_shortest_paths(g, std::size_t{0});
    
    assert(result.is_reachable(0));
    assert(approx_equal(result.distance_to(0), 0.0));
    
    auto path = result.path_to(0);
    assert(path.size() == 1);
    assert(path[0] == 0);
    
    std::cout << "  Single vertex: PASSED\n";
}

void test_self_loop() {
    test::weighted_graph g(2);
    g.add_edge(0, 0, 1.0);  // Self-loop
    g.add_edge(0, 1, 2.0);
    
    auto result = bgl::dijkstra_shortest_paths(g, std::size_t{0});
    
    assert(approx_equal(result.distance_to(0), 0.0));  // Self-loop shouldn't change distance
    assert(approx_equal(result.distance_to(1), 2.0));
    
    std::cout << "  Self loop: PASSED\n";
}

void test_parallel_edges() {
    // Two edges from 0 to 1 with different weights
    test::weighted_graph g(2);
    g.add_edge(0, 1, 5.0);
    g.add_edge(0, 1, 2.0);  // Shorter path
    
    // With the new edge descriptor that includes index, we can properly
    // distinguish parallel edges
    auto result = bgl::dijkstra_shortest_paths(g, std::size_t{0},
        [&g](test::edge_desc e) {
            return test::get_weight(e, g);
        });
    
    // Should find the shorter of the two edges
    assert(result.is_reachable(1));
    assert(approx_equal(result.distance_to(1), 2.0));  // Should use shorter edge
    
    std::cout << "  Parallel edges: PASSED\n";
}

void test_larger_graph() {
    // Create a grid-like graph
    //   0 - 1 - 2
    //   |   |   |
    //   3 - 4 - 5
    //   |   |   |
    //   6 - 7 - 8
    test::weighted_graph g(9);
    
    // Horizontal edges
    g.add_edge(0, 1, 1.0); g.add_edge(1, 0, 1.0);
    g.add_edge(1, 2, 1.0); g.add_edge(2, 1, 1.0);
    g.add_edge(3, 4, 1.0); g.add_edge(4, 3, 1.0);
    g.add_edge(4, 5, 1.0); g.add_edge(5, 4, 1.0);
    g.add_edge(6, 7, 1.0); g.add_edge(7, 6, 1.0);
    g.add_edge(7, 8, 1.0); g.add_edge(8, 7, 1.0);
    
    // Vertical edges
    g.add_edge(0, 3, 1.0); g.add_edge(3, 0, 1.0);
    g.add_edge(1, 4, 1.0); g.add_edge(4, 1, 1.0);
    g.add_edge(2, 5, 1.0); g.add_edge(5, 2, 1.0);
    g.add_edge(3, 6, 1.0); g.add_edge(6, 3, 1.0);
    g.add_edge(4, 7, 1.0); g.add_edge(7, 4, 1.0);
    g.add_edge(5, 8, 1.0); g.add_edge(8, 5, 1.0);
    
    auto result = bgl::dijkstra_shortest_paths(g, std::size_t{0}, bgl::unit_weight{});
    
    // Check distances (Manhattan distances)
    assert(approx_equal(result.distance_to(0), 0.0));
    assert(approx_equal(result.distance_to(1), 1.0));
    assert(approx_equal(result.distance_to(2), 2.0));
    assert(approx_equal(result.distance_to(3), 1.0));
    assert(approx_equal(result.distance_to(4), 2.0));
    assert(approx_equal(result.distance_to(5), 3.0));
    assert(approx_equal(result.distance_to(6), 2.0));
    assert(approx_equal(result.distance_to(7), 3.0));
    assert(approx_equal(result.distance_to(8), 4.0));
    
    std::cout << "  Larger graph (3x3 grid): PASSED\n";
}

void test_different_source() {
    test::weighted_graph g(4);
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 2, 1.0);
    g.add_edge(2, 3, 1.0);
    g.add_edge(3, 0, 1.0);  // Cycle back
    
    // Start from vertex 2
    auto result = bgl::dijkstra_shortest_paths(g, std::size_t{2}, bgl::unit_weight{});
    
    assert(approx_equal(result.distance_to(2), 0.0));
    assert(approx_equal(result.distance_to(3), 1.0));
    assert(approx_equal(result.distance_to(0), 2.0));
    assert(approx_equal(result.distance_to(1), 3.0));
    
    std::cout << "  Different source: PASSED\n";
}

void test_integer_weights() {
    test::weighted_graph g(3);
    g.add_edge(0, 1, 2.0);
    g.add_edge(1, 2, 3.0);
    
    // Use explicit integer distance type with dijkstra_shortest_paths_as
    auto result = bgl::dijkstra_shortest_paths_as<int>(
        g, std::size_t{0}, [](test::edge_desc) { return 1; });
    
    assert(result.distance_to(0) == 0);
    assert(result.distance_to(1) == 1);
    assert(result.distance_to(2) == 2);
    
    std::cout << "  Integer weights: PASSED\n";
}

} // anonymous namespace

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "BGL Modern Dijkstra Shortest Paths Tests\n";
    std::cout << "=========================================\n\n";
    
    std::cout << "Basic Tests:\n";
    test_simple_path();
    test_branching_graph();
    test_unreachable_vertices();
    test_single_vertex();
    
    std::cout << "\nWeight Accessor Tests:\n";
    test_custom_weight_accessor();
    test_unit_weights();
    test_integer_weights();
    
    std::cout << "\nEdge Case Tests:\n";
    test_self_loop();
    test_parallel_edges();
    test_different_source();
    
    std::cout << "\nScalability Tests:\n";
    test_larger_graph();
    
    std::cout << "\n=========================================\n";
    std::cout << "All Dijkstra tests passed!\n";
    
    return 0;
}
