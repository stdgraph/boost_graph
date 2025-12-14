// BGL Modern - Algorithm Result Tests
// Verify dijkstra_result and related structures work correctly
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <bgl/modern/algorithm_result.hpp>
#include <bgl/modern/concepts.hpp>

#include <iostream>
#include <cassert>
#include <cmath>

// =============================================================================
// Test Graph Implementation
// =============================================================================

namespace test {

struct simple_graph {
    using vertex_descriptor = std::size_t;
    using edge_descriptor = std::pair<vertex_descriptor, vertex_descriptor>;
    using directed_category = bgl::directed_tag;
    using edge_parallel_category = bgl::disallow_parallel_edge_tag;
    using traversal_category = void;
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
    
    struct edge_property {
        double weight = 1.0;
    };
    
    std::vector<std::vector<std::pair<vertex_descriptor, edge_property>>> adj_;
    
    simple_graph() = default;
    explicit simple_graph(std::size_t n) : adj_(n) {}
    
    void add_edge(vertex_descriptor u, vertex_descriptor v, double weight = 1.0) {
        adj_[u].push_back({v, edge_property{weight}});
    }
};

inline std::size_t num_vertices(const simple_graph& g) {
    return g.adj_.size();
}

inline auto vertices(const simple_graph& g) {
    return std::views::iota(std::size_t{0}, g.adj_.size());
}

} // namespace test

// =============================================================================
// Tests
// =============================================================================

namespace {

void test_construction() {
    // Default construction
    bgl::dijkstra_result<test::simple_graph> result1;
    
    // Construction with size
    bgl::dijkstra_result<test::simple_graph> result2(10);
    assert(result2.distances().size() == 10);
    assert(result2.predecessors().size() == 10);
    
    // All distances should be infinity
    for (std::size_t v = 0; v < 10; ++v) {
        assert(result2.distance_to(v) == result2.infinity());
        assert(!result2.is_reachable(v));
    }
    
    // Construction with size and source
    bgl::dijkstra_result<test::simple_graph> result3(10, 0);
    assert(result3.source() == 0);
    assert(result3.distance_to(0) == 0.0);
    assert(result3.is_reachable(0));
    assert(result3.is_source(0));
    assert(result3.predecessor_of(0) == 0);  // Source is its own predecessor
    
    std::cout << "  Construction: PASSED\n";
}

void test_factory_functions() {
    test::simple_graph g(5);
    
    auto result1 = bgl::make_dijkstra_result(g);
    assert(result1.distances().size() == 5);
    
    auto result2 = bgl::make_dijkstra_result(g, 2);
    assert(result2.source() == 2);
    assert(result2.distance_to(2) == 0.0);
    
    // Custom distance type
    auto result3 = bgl::make_dijkstra_result<int>(g, 0);
    assert(result3.distance_to(0) == 0);
    
    std::cout << "  Factory functions: PASSED\n";
}

void test_property_maps() {
    bgl::dijkstra_result<test::simple_graph> result(5, 0);
    
    // Test distance_map
    auto dist_map = result.distance_map();
    dist_map(1) = 10.0;
    dist_map(2) = 20.0;
    assert(result.distance_to(1) == 10.0);
    assert(result.distance_to(2) == 20.0);
    
    // Test predecessor_map
    auto pred_map = result.predecessor_map();
    pred_map(1) = 0;
    pred_map(2) = 1;
    assert(result.predecessor_of(1) == 0);
    assert(result.predecessor_of(2) == 1);
    
    // Test const versions
    const auto& const_result = result;
    auto const_dist_map = const_result.distance_map();
    assert(const_dist_map(1) == 10.0);
    
    std::cout << "  Property maps: PASSED\n";
}

void test_reachability() {
    bgl::dijkstra_result<test::simple_graph> result(5, 0);
    
    // Source is reachable
    assert(result.is_reachable(0));
    
    // Others not yet reachable
    assert(!result.is_reachable(1));
    assert(!result.is_reachable(4));
    
    // Mark vertex 1 as reachable
    result.distance_map()(1) = 5.0;
    assert(result.is_reachable(1));
    
    std::cout << "  Reachability: PASSED\n";
}

void test_path_reconstruction() {
    // Simulate a shortest path tree:
    // 0 -> 1 -> 2 -> 3
    bgl::dijkstra_result<test::simple_graph> result(4, 0);
    
    auto dist = result.distance_map();
    auto pred = result.predecessor_map();
    
    // Set up distances and predecessors
    dist(1) = 1.0;  pred(1) = 0;
    dist(2) = 2.0;  pred(2) = 1;
    dist(3) = 3.0;  pred(3) = 2;
    
    // Reconstruct path to vertex 3
    auto path = result.path_to(3);
    assert(path.size() == 4);
    assert(path[0] == 0);
    assert(path[1] == 1);
    assert(path[2] == 2);
    assert(path[3] == 3);
    
    // Path length
    auto len = result.path_length(3);
    assert(len.has_value());
    assert(len.value() == 3);
    
    // Path to source
    auto source_path = result.path_to(0);
    assert(source_path.size() == 1);
    assert(source_path[0] == 0);
    
    // Path length to source
    auto source_len = result.path_length(0);
    assert(source_len.has_value());
    assert(source_len.value() == 0);
    
    std::cout << "  Path reconstruction: PASSED\n";
}

void test_unreachable_path() {
    bgl::dijkstra_result<test::simple_graph> result(5, 0);
    
    // Vertex 4 is unreachable (distance is infinity)
    auto path = result.path_to(4);
    assert(path.empty());
    
    auto len = result.path_length(4);
    assert(!len.has_value());
    
    std::cout << "  Unreachable path: PASSED\n";
}

void test_reset() {
    bgl::dijkstra_result<test::simple_graph> result(5, 0);
    
    // Modify some values
    result.distance_map()(1) = 10.0;
    result.predecessor_map()(1) = 0;
    
    // Reset with new source
    result.reset(5, 2);
    
    assert(result.source() == 2);
    assert(result.distance_to(2) == 0.0);
    assert(result.distance_to(0) == result.infinity());
    assert(result.distance_to(1) == result.infinity());
    
    std::cout << "  Reset: PASSED\n";
}

void test_integer_distance() {
    bgl::dijkstra_result<test::simple_graph, int> result(5, 0);
    
    // Check that infinity works for integers
    assert(result.infinity() == std::numeric_limits<int>::max());
    
    result.distance_map()(1) = 10;
    assert(result.distance_to(1) == 10);
    assert(result.is_reachable(1));
    
    std::cout << "  Integer distance type: PASSED\n";
}

void test_branching_path() {
    // Test a branching shortest path tree:
    //     1
    //    /
    //   0 - 2
    //    \
    //     3 - 4
    bgl::dijkstra_result<test::simple_graph> result(5, 0);
    
    auto dist = result.distance_map();
    auto pred = result.predecessor_map();
    
    dist(1) = 1.0;  pred(1) = 0;
    dist(2) = 1.0;  pred(2) = 0;
    dist(3) = 1.0;  pred(3) = 0;
    dist(4) = 2.0;  pred(4) = 3;
    
    // Path to 1
    auto path1 = result.path_to(1);
    assert(path1.size() == 2);
    assert(path1[0] == 0 && path1[1] == 1);
    
    // Path to 4 (goes through 3)
    auto path4 = result.path_to(4);
    assert(path4.size() == 3);
    assert(path4[0] == 0 && path4[1] == 3 && path4[2] == 4);
    
    std::cout << "  Branching path: PASSED\n";
}

} // anonymous namespace

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "BGL Modern Algorithm Result Tests\n";
    std::cout << "==================================\n\n";
    
    std::cout << "dijkstra_result Tests:\n";
    test_construction();
    test_factory_functions();
    test_property_maps();
    test_reachability();
    test_path_reconstruction();
    test_unreachable_path();
    test_reset();
    test_integer_distance();
    test_branching_path();
    
    std::cout << "\n==================================\n";
    std::cout << "All algorithm result tests passed!\n";
    
    return 0;
}
