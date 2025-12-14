// BGL Modern - Bellman-Ford Shortest Paths Tests
// Verify the C++20 Bellman-Ford implementation works correctly
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <bgl/modern/bellman_ford_shortest_paths.hpp>

#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <ranges>

// =============================================================================
// Test Graph Implementation (supports EdgeListGraph)
// =============================================================================

namespace test {

// Edge descriptor with source, target, and index
struct edge_desc {
    std::size_t source;
    std::size_t target;
    std::size_t index;
    
    bool operator==(const edge_desc&) const = default;
};

/// A weighted directed graph supporting EdgeListGraph concept
struct bf_graph {
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
    
    struct stored_edge {
        vertex_descriptor source;
        vertex_descriptor target;
        std::size_t index;
        edge_property prop;
    };
    
    std::size_t num_vertices_ = 0;
    std::vector<stored_edge> edges_;
    std::vector<std::vector<std::size_t>> out_edge_indices_;  // vertex -> edge indices
    
    bf_graph() = default;
    explicit bf_graph(std::size_t n) : num_vertices_(n), out_edge_indices_(n) {}
    
    void add_edge(vertex_descriptor u, vertex_descriptor v, double weight = 1.0) {
        std::size_t idx = edges_.size();
        edges_.push_back({u, v, idx, {weight}});
        out_edge_indices_[u].push_back(idx);
    }
    
    const edge_property& operator[](const edge_desc& e) const {
        return edges_[e.index].prop;
    }
};

// --- VertexListGraph functions ---

inline std::size_t num_vertices(const bf_graph& g) {
    return g.num_vertices_;
}

inline auto vertices(const bf_graph& g) {
    return std::views::iota(std::size_t{0}, g.num_vertices_);
}

// --- EdgeListGraph functions ---

struct edge_iterator {
    using value_type = edge_desc;
    using difference_type = std::ptrdiff_t;
    using pointer = const edge_desc*;
    using reference = edge_desc;
    using iterator_category = std::forward_iterator_tag;
    
    std::vector<bf_graph::stored_edge>::const_iterator it_;
    
    edge_iterator() = default;
    edge_iterator(std::vector<bf_graph::stored_edge>::const_iterator it) : it_(it) {}
    
    edge_desc operator*() const { return {it_->source, it_->target, it_->index}; }
    edge_iterator& operator++() { ++it_; return *this; }
    edge_iterator operator++(int) { auto tmp = *this; ++it_; return tmp; }
    bool operator==(const edge_iterator& o) const { return it_ == o.it_; }
    bool operator!=(const edge_iterator& o) const { return it_ != o.it_; }
};

struct edge_range {
    const std::vector<bf_graph::stored_edge>* edges_;
    edge_iterator begin() const { return {edges_->begin()}; }
    edge_iterator end() const { return {edges_->end()}; }
};

inline edge_range edges(const bf_graph& g) {
    return {&g.edges_};
}

inline std::size_t num_edges(const bf_graph& g) {
    return g.edges_.size();
}

// --- IncidenceGraph functions ---

struct out_edge_iterator {
    using value_type = edge_desc;
    using difference_type = std::ptrdiff_t;
    using pointer = const edge_desc*;
    using reference = edge_desc;
    using iterator_category = std::forward_iterator_tag;
    
    const bf_graph* g_;
    std::vector<std::size_t>::const_iterator it_;
    
    out_edge_iterator() = default;
    out_edge_iterator(const bf_graph* g, std::vector<std::size_t>::const_iterator it)
        : g_(g), it_(it) {}
    
    edge_desc operator*() const {
        const auto& e = g_->edges_[*it_];
        return {e.source, e.target, e.index};
    }
    out_edge_iterator& operator++() { ++it_; return *this; }
    out_edge_iterator operator++(int) { auto tmp = *this; ++it_; return tmp; }
    bool operator==(const out_edge_iterator& o) const { return it_ == o.it_; }
    bool operator!=(const out_edge_iterator& o) const { return it_ != o.it_; }
};

struct out_edge_range {
    const bf_graph* g_;
    const std::vector<std::size_t>* indices_;
    out_edge_iterator begin() const { return {g_, indices_->begin()}; }
    out_edge_iterator end() const { return {g_, indices_->end()}; }
};

inline out_edge_range out_edges(bf_graph::vertex_descriptor v, const bf_graph& g) {
    return {&g, &g.out_edge_indices_[v]};
}

inline std::size_t out_degree(bf_graph::vertex_descriptor v, const bf_graph& g) {
    return g.out_edge_indices_[v].size();
}

inline bf_graph::vertex_descriptor source(edge_desc e, const bf_graph&) {
    return e.source;
}

inline bf_graph::vertex_descriptor target(edge_desc e, const bf_graph&) {
    return e.target;
}

} // namespace test

// Specialize graph_traits
template<>
struct bgl::graph_traits<test::bf_graph> {
    using vertex_descriptor = test::bf_graph::vertex_descriptor;
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
    // 0 -> 1 -> 2 -> 3 with weights 1, 2, 3
    test::bf_graph g(4);
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 2, 2.0);
    g.add_edge(2, 3, 3.0);
    
    auto result = bgl::bellman_ford_shortest_paths(g, std::size_t{0});
    
    assert(!result.has_negative_cycle());
    assert(approx_equal(result.distance_to(0), 0.0));
    assert(approx_equal(result.distance_to(1), 1.0));
    assert(approx_equal(result.distance_to(2), 3.0));
    assert(approx_equal(result.distance_to(3), 6.0));
    
    std::cout << "  Simple path: PASSED\n";
}

void test_negative_weights() {
    // Graph with negative weights but no negative cycle
    //     1
    //    /|\
    //   2 | -3
    //  /  |  \
    // 0   1   2
    //  \     /
    //   -1  2
    //    \ /
    //     3
    test::bf_graph g(4);
    g.add_edge(0, 1, 2.0);
    g.add_edge(0, 3, -1.0);
    g.add_edge(1, 2, -3.0);
    g.add_edge(3, 2, 2.0);
    
    auto result = bgl::bellman_ford_shortest_paths(g, std::size_t{0});
    
    assert(!result.has_negative_cycle());
    assert(approx_equal(result.distance_to(0), 0.0));
    assert(approx_equal(result.distance_to(1), 2.0));
    assert(approx_equal(result.distance_to(2), -1.0));  // 0->1->2: 2-3=-1
    assert(approx_equal(result.distance_to(3), -1.0));
    
    std::cout << "  Negative weights: PASSED\n";
}

void test_negative_cycle() {
    // Graph with negative cycle: 0 -> 1 -> 2 -> 0 with total weight -1
    test::bf_graph g(3);
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 2, 1.0);
    g.add_edge(2, 0, -3.0);  // Cycle: 1 + 1 - 3 = -1
    
    auto result = bgl::bellman_ford_shortest_paths(g, std::size_t{0});
    
    assert(result.has_negative_cycle());
    
    std::cout << "  Negative cycle detection: PASSED\n";
}

void test_unreachable_vertices() {
    test::bf_graph g(4);
    g.add_edge(0, 1, 1.0);
    // Vertices 2, 3 are unreachable
    
    auto result = bgl::bellman_ford_shortest_paths(g, std::size_t{0});
    
    assert(!result.has_negative_cycle());
    assert(result.is_reachable(0));
    assert(result.is_reachable(1));
    assert(!result.is_reachable(2));
    assert(!result.is_reachable(3));
    
    std::cout << "  Unreachable vertices: PASSED\n";
}

void test_custom_weight_accessor() {
    test::bf_graph g(3);
    g.add_edge(0, 1, 5.0);  // Stored weight ignored
    g.add_edge(1, 2, 5.0);
    
    // Use custom weight: always 2.0
    auto result = bgl::bellman_ford_shortest_paths(g, std::size_t{0},
        [](test::edge_desc) { return 2.0; });
    
    assert(!result.has_negative_cycle());
    assert(approx_equal(result.distance_to(1), 2.0));
    assert(approx_equal(result.distance_to(2), 4.0));
    
    std::cout << "  Custom weight accessor: PASSED\n";
}

void test_single_vertex() {
    test::bf_graph g(1);
    
    auto result = bgl::bellman_ford_shortest_paths(g, std::size_t{0});
    
    assert(!result.has_negative_cycle());
    assert(approx_equal(result.distance_to(0), 0.0));
    
    std::cout << "  Single vertex: PASSED\n";
}

void test_self_loop_negative() {
    // Self-loop with negative weight = negative cycle
    test::bf_graph g(2);
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 1, -1.0);  // Negative self-loop
    
    auto result = bgl::bellman_ford_shortest_paths(g, std::size_t{0});
    
    assert(result.has_negative_cycle());
    
    std::cout << "  Self-loop negative cycle: PASSED\n";
}

void test_unreachable_negative_cycle() {
    // Negative cycle exists in component {2,3} but not reachable from source 0
    // 0 -> 1 (isolated)
    // 2 -> 3 -> 2 (negative cycle, weight -1)
    test::bf_graph g(4);
    g.add_edge(0, 1, 1.0);
    g.add_edge(2, 3, 1.0);
    g.add_edge(3, 2, -2.0);  // Creates negative cycle in unreachable component
    
    auto result = bgl::bellman_ford_shortest_paths(g, std::size_t{0});
    
    // Should NOT report negative cycle since it's unreachable from source
    assert(!result.has_negative_cycle());
    assert(result.is_reachable(1));
    assert(!result.is_reachable(2));
    assert(!result.is_reachable(3));
    
    std::cout << "  Unreachable negative cycle: PASSED\n";
}

void test_path_reconstruction() {
    // Test that path_to() works with negative weights
    // 0 -> 1 -> 2 with negative edge
    test::bf_graph g(3);
    g.add_edge(0, 1, 2.0);
    g.add_edge(1, 2, -1.0);
    
    auto result = bgl::bellman_ford_shortest_paths(g, std::size_t{0});
    
    assert(!result.has_negative_cycle());
    auto path = result.path_to(2);
    assert(path.size() == 3);
    assert(path[0] == 0);
    assert(path[1] == 1);
    assert(path[2] == 2);
    assert(approx_equal(result.distance_to(2), 1.0));  // 2 + (-1) = 1
    
    std::cout << "  Path reconstruction: PASSED\n";
}

void test_zero_weight_edges() {
    // Graph with zero-weight edges
    test::bf_graph g(3);
    g.add_edge(0, 1, 0.0);
    g.add_edge(1, 2, 0.0);
    
    auto result = bgl::bellman_ford_shortest_paths(g, std::size_t{0});
    
    assert(!result.has_negative_cycle());
    assert(approx_equal(result.distance_to(0), 0.0));
    assert(approx_equal(result.distance_to(1), 0.0));
    assert(approx_equal(result.distance_to(2), 0.0));
    
    std::cout << "  Zero weight edges: PASSED\n";
}

void test_empty_graph() {
    test::bf_graph g(0);
    
    // This should handle gracefully (though source 0 is invalid)
    // The algorithm should just return with no cycle
    auto result = bgl::bellman_ford_shortest_paths(g, std::size_t{0});
    
    assert(!result.has_negative_cycle());
    
    std::cout << "  Empty graph: PASSED\n";
}

void test_integer_distance_type() {
    test::bf_graph g(3);
    g.add_edge(0, 1, 2.0);
    g.add_edge(1, 2, 3.0);
    
    auto result = bgl::bellman_ford_shortest_paths_as<int>(
        g, std::size_t{0}, [](test::edge_desc e) { 
            // Return integer weights
            return e.index == 0 ? 2 : 3; 
        });
    
    assert(!result.has_negative_cycle());
    assert(result.distance_to(0) == 0);
    assert(result.distance_to(1) == 2);
    assert(result.distance_to(2) == 5);
    
    std::cout << "  Integer distance type: PASSED\n";
}

void test_multiple_paths() {
    // Diamond graph with multiple paths
    //       1
    //      / \
    //   1 /   \ 5
    //    /     \
    //   0       3
    //    \     /
    //   2 \   / 1
    //      \ /
    //       2
    test::bf_graph g(4);
    g.add_edge(0, 1, 1.0);
    g.add_edge(0, 2, 2.0);
    g.add_edge(1, 3, 5.0);
    g.add_edge(2, 3, 1.0);
    
    auto result = bgl::bellman_ford_shortest_paths(g, std::size_t{0});
    
    assert(!result.has_negative_cycle());
    // Shortest path to 3: 0->2->3 = 2+1 = 3 (not 0->1->3 = 1+5 = 6)
    assert(approx_equal(result.distance_to(3), 3.0));
    
    auto path = result.path_to(3);
    assert(path.size() == 3);
    assert(path[0] == 0);
    assert(path[1] == 2);
    assert(path[2] == 3);
    
    std::cout << "  Multiple paths: PASSED\n";
}

void test_larger_graph() {
    // 4x4 grid graph with some negative edges
    // Each cell connects right and down
    const std::size_t size = 4;
    test::bf_graph g(size * size);
    
    auto idx = [size](std::size_t r, std::size_t c) { return r * size + c; };
    
    for (std::size_t r = 0; r < size; ++r) {
        for (std::size_t c = 0; c < size; ++c) {
            if (c + 1 < size) {
                // Horizontal edge (some negative)
                double w = (r == 1 && c == 1) ? -0.5 : 1.0;
                g.add_edge(idx(r, c), idx(r, c + 1), w);
            }
            if (r + 1 < size) {
                // Vertical edge
                g.add_edge(idx(r, c), idx(r + 1, c), 1.0);
            }
        }
    }
    
    auto result = bgl::bellman_ford_shortest_paths(g, std::size_t{0});
    
    assert(!result.has_negative_cycle());
    // All vertices should be reachable
    for (std::size_t i = 0; i < size * size; ++i) {
        assert(result.is_reachable(i));
    }
    // Bottom-right corner: shortest path uses negative edge
    // 0,0 -> 0,1 -> 0,2 -> 0,3 -> 1,3 -> 2,3 -> 3,3 = 6
    // Or: 0,0 -> 1,0 -> 1,1 -> 1,2 (-0.5) -> 1,3 -> 2,3 -> 3,3 = 1+1-0.5+1+1+1 = 4.5
    assert(result.distance_to(idx(3, 3)) < 6.0);
    
    std::cout << "  Larger graph (4x4 grid): PASSED\n";
}

} // anonymous namespace

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "BGL Modern Bellman-Ford Shortest Paths Tests\n";
    std::cout << "=============================================\n\n";
    
    std::cout << "Basic Tests:\n";
    test_simple_path();
    test_negative_weights();
    test_unreachable_vertices();
    test_single_vertex();
    test_empty_graph();
    test_zero_weight_edges();
    
    std::cout << "\nNegative Cycle Tests:\n";
    test_negative_cycle();
    test_self_loop_negative();
    test_unreachable_negative_cycle();
    
    std::cout << "\nPath Tests:\n";
    test_path_reconstruction();
    test_multiple_paths();
    
    std::cout << "\nWeight Accessor Tests:\n";
    test_custom_weight_accessor();
    test_integer_distance_type();
    
    std::cout << "\nScalability Tests:\n";
    test_larger_graph();
    
    std::cout << "\n=============================================\n";
    std::cout << "All Bellman-Ford tests passed!\n";
    
    return 0;
}

