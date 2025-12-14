// BGL Modern - Parallel Algorithms Tests
// Tests for parallel BFS, connected components, and utilities
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <bgl/modern/parallel_algorithms.hpp>
#include <bgl/modern/adjacency_list.hpp>
#include <bgl/modern/breadth_first_search.hpp>
#include <bgl/modern/connected_components.hpp>
#include <bgl/modern/range_functions.hpp>

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <chrono>
#include <execution>

using namespace bgl;

namespace {

// =============================================================================
// Test Helpers
// =============================================================================

template<typename T>
void assert_eq(const T& a, const T& b, const char* msg) {
    if (a != b) {
        std::cerr << "FAIL: " << msg << " (expected " << b << ", got " << a << ")\n";
        std::exit(1);
    }
}

void assert_true(bool cond, const char* msg) {
    if (!cond) {
        std::cerr << "FAIL: " << msg << "\n";
        std::exit(1);
    }
}

// =============================================================================
// Test: Parallel BFS Correctness
// =============================================================================

void test_parallel_bfs_correctness() {
    std::cout << "Testing parallel BFS correctness...\n";
    
    // Create a simple graph:
    //     0
    //    / \
    //   1   2
    //  / \   \
    // 3   4   5
    //
    adjacency_list<undirected_tag> g(6);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    g.add_edge(1, 4);
    g.add_edge(2, 5);
    
    // Run parallel BFS
    auto par_result = parallel_bfs(std::execution::par, g, std::size_t{0});
    
    // Run sequential BFS for comparison
    auto seq_result = breadth_first_search(g, std::size_t{0});
    
    // Check that distances match
    for (std::size_t v = 0; v < 6; ++v) {
        assert_eq(par_result.distance_to(v), seq_result.distance_to(v),
                  "Parallel BFS distances match sequential");
    }
    
    // Verify expected distances
    assert_eq(par_result.distance_to(0), std::size_t{0}, "Distance to source is 0");
    assert_eq(par_result.distance_to(1), std::size_t{1}, "Distance to v1 is 1");
    assert_eq(par_result.distance_to(2), std::size_t{1}, "Distance to v2 is 1");
    assert_eq(par_result.distance_to(3), std::size_t{2}, "Distance to v3 is 2");
    assert_eq(par_result.distance_to(4), std::size_t{2}, "Distance to v4 is 2");
    assert_eq(par_result.distance_to(5), std::size_t{2}, "Distance to v5 is 2");
    
    std::cout << "  PASS: Parallel BFS distances correct\n";
    
    // Check all vertices discovered
    auto discovered = par_result.discovered_vertices();
    assert_eq(discovered.size(), std::size_t{6}, "All vertices discovered");
    
    std::cout << "  PASS: All vertices discovered\n";
}

// =============================================================================
// Test: Parallel BFS on Disconnected Graph
// =============================================================================

void test_parallel_bfs_disconnected() {
    std::cout << "Testing parallel BFS on disconnected graph...\n";
    
    // Create disconnected graph: 0-1-2  3-4
    adjacency_list<undirected_tag> g(5);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(3, 4);
    
    auto result = parallel_bfs(std::execution::par, g, std::size_t{0});
    
    // Component with source
    assert_eq(result.distance_to(0), std::size_t{0}, "Distance to v0");
    assert_eq(result.distance_to(1), std::size_t{1}, "Distance to v1");
    assert_eq(result.distance_to(2), std::size_t{2}, "Distance to v2");
    
    // Unreachable component
    assert_eq(result.distance_to(3), bfs_result<adjacency_list<undirected_tag>>::infinity(),
              "v3 is unreachable");
    assert_eq(result.distance_to(4), bfs_result<adjacency_list<undirected_tag>>::infinity(),
              "v4 is unreachable");
    
    std::cout << "  PASS: Disconnected graph handled correctly\n";
}

// =============================================================================
// Test: Parallel Connected Components Correctness
// =============================================================================

void test_parallel_components_correctness() {
    std::cout << "Testing parallel connected components correctness...\n";
    
    // Create graph with 3 components:
    // Component 0: 0-1-2
    // Component 1: 3-4
    // Component 2: 5
    adjacency_list<undirected_tag> g(6);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(3, 4);
    
    auto par_result = parallel_connected_components(std::execution::par, g);
    auto seq_result = connected_components(g);
    
    // Check number of components
    assert_eq(par_result.num_components(), std::size_t{3}, "3 components");
    assert_eq(par_result.num_components(), seq_result.num_components(),
              "Parallel matches sequential component count");
    
    std::cout << "  PASS: Correct number of components\n";
    
    // Verify component membership (same component iff connected)
    auto same_component_par = [&](std::size_t u, std::size_t v) {
        return par_result.component_of(u) == par_result.component_of(v);
    };
    
    // Component 0: vertices 0, 1, 2
    assert_true(same_component_par(0, 1), "0 and 1 in same component");
    assert_true(same_component_par(1, 2), "1 and 2 in same component");
    assert_true(same_component_par(0, 2), "0 and 2 in same component");
    
    // Component 1: vertices 3, 4
    assert_true(same_component_par(3, 4), "3 and 4 in same component");
    
    // Different components
    assert_true(!same_component_par(0, 3), "0 and 3 in different components");
    assert_true(!same_component_par(0, 5), "0 and 5 in different components");
    assert_true(!same_component_par(3, 5), "3 and 5 in different components");
    
    std::cout << "  PASS: Component membership correct\n";
}

// =============================================================================
// Test: Parallel Components on Fully Connected Graph
// =============================================================================

void test_parallel_components_fully_connected() {
    std::cout << "Testing parallel components on fully connected graph...\n";
    
    const std::size_t n = 10;
    adjacency_list<undirected_tag> g(n);
    
    // Create complete graph
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = i + 1; j < n; ++j) {
            g.add_edge(i, j);
        }
    }
    
    auto result = parallel_connected_components(std::execution::par, g);
    
    assert_eq(result.num_components(), std::size_t{1}, "Single component");
    
    // All vertices should be in the same component
    std::size_t comp0 = result.component_of(0);
    for (std::size_t v = 1; v < n; ++v) {
        assert_eq(result.component_of(v), comp0, "All vertices in same component");
    }
    
    std::cout << "  PASS: Fully connected graph handled correctly\n";
}

// =============================================================================
// Test: Parallel For Each Vertex
// =============================================================================

void test_parallel_for_each_vertex() {
    std::cout << "Testing parallel_for_each_vertex...\n";
    
    adjacency_list<undirected_tag> g(100);
    std::vector<std::atomic<int>> counts(100);
    
    // Initialize counts
    for (auto& c : counts) {
        c.store(0, std::memory_order_relaxed);
    }
    
    // Increment each vertex count in parallel
    parallel_for_each_vertex(std::execution::par, g,
        [&](std::size_t v) {
            counts[v].fetch_add(1, std::memory_order_relaxed);
        });
    
    // Check each vertex was processed exactly once
    for (std::size_t v = 0; v < 100; ++v) {
        assert_eq(counts[v].load(std::memory_order_relaxed), 1,
                  "Each vertex processed once");
    }
    
    std::cout << "  PASS: All vertices processed\n";
}

// =============================================================================
// Test: Parallel For Each Edge
// =============================================================================

void test_parallel_for_each_edge() {
    std::cout << "Testing edge iteration in parallel...\n";
    
    adjacency_list<undirected_tag> g(10);
    
    // Add edges in a chain
    for (std::size_t i = 0; i < 9; ++i) {
        g.add_edge(i, i + 1);
    }
    
    // Manually count edges by iterating vertices
    std::atomic<std::size_t> edge_count{0};
    
    parallel_for_each_vertex(std::execution::par, g,
        [&](std::size_t v) {
            for (auto e : out_edges(v, g)) {
                edge_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    
    // For undirected graph, each edge is counted twice
    assert_eq(edge_count.load(std::memory_order_relaxed), std::size_t{18},
              "All edges counted (twice for undirected)");
    
    std::cout << "  PASS: Edge counting works\n";
}

// =============================================================================
// Test: Sequential Policy Fallback
// =============================================================================

void test_sequential_fallback() {
    std::cout << "Testing sequential policy fallback...\n";
    
    adjacency_list<undirected_tag> g(5);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    g.add_edge(3, 4);
    
    // Test parallel_bfs with sequential policy
    auto bfs_result = parallel_bfs(std::execution::seq, g, std::size_t{0});
    assert_eq(bfs_result.distance_to(4), std::size_t{4}, "Sequential BFS works");
    
    std::cout << "  PASS: Sequential BFS fallback works\n";
    
    // Test parallel_connected_components with sequential policy
    auto cc_result = parallel_connected_components(std::execution::seq, g);
    assert_eq(cc_result.num_components(), std::size_t{1}, "Sequential CC works");
    
    std::cout << "  PASS: Sequential CC fallback works\n";
}

// =============================================================================
// Test: Large Graph Performance
// =============================================================================

void test_large_graph_performance() {
    std::cout << "Testing large graph performance...\n";
    
    const std::size_t n = 10000;
    const std::size_t degree = 10;
    
    adjacency_list<undirected_tag> g(n);
    
    // Create random graph with ~degree edges per vertex
    std::cout << "  Building graph with " << n << " vertices...\n";
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < degree && i + j + 1 < n; ++j) {
            g.add_edge(i, i + j + 1);
        }
    }
    
    std::cout << "  Graph has " << num_edges(g) << " edges\n";
    
    // Benchmark sequential BFS
    auto seq_start = std::chrono::high_resolution_clock::now();
    auto seq_result = breadth_first_search(g, std::size_t{0});
    auto seq_end = std::chrono::high_resolution_clock::now();
    auto seq_time = std::chrono::duration<double, std::milli>(seq_end - seq_start).count();
    
    std::cout << "  Sequential BFS: " << seq_time << " ms\n";
    
    // Benchmark parallel BFS
    auto par_start = std::chrono::high_resolution_clock::now();
    auto par_result = parallel_bfs(std::execution::par, g, std::size_t{0});
    auto par_end = std::chrono::high_resolution_clock::now();
    auto par_time = std::chrono::duration<double, std::milli>(par_end - par_start).count();
    
    std::cout << "  Parallel BFS: " << par_time << " ms\n";
    std::cout << "  Speedup: " << (seq_time / par_time) << "x\n";
    
    // Verify results match
    for (std::size_t v = 0; v < n; ++v) {
        assert_eq(par_result.distance_to(v), seq_result.distance_to(v),
                  "Parallel and sequential BFS results match");
    }
    
    std::cout << "  PASS: Results match\n";
    
    // Benchmark sequential connected components
    adjacency_list<undirected_tag> g2(n);
    for (std::size_t i = 0; i < n - 1; i += 2) {
        g2.add_edge(i, i + 1);  // n/2 components
    }
    
    seq_start = std::chrono::high_resolution_clock::now();
    auto seq_cc = connected_components(g2);
    seq_end = std::chrono::high_resolution_clock::now();
    seq_time = std::chrono::duration<double, std::milli>(seq_end - seq_start).count();
    
    std::cout << "  Sequential CC: " << seq_time << " ms\n";
    
    // Benchmark parallel connected components
    par_start = std::chrono::high_resolution_clock::now();
    auto par_cc = parallel_connected_components(std::execution::par, g2);
    par_end = std::chrono::high_resolution_clock::now();
    par_time = std::chrono::duration<double, std::milli>(par_end - par_start).count();
    
    std::cout << "  Parallel CC: " << par_time << " ms\n";
    std::cout << "  Speedup: " << (seq_time / par_time) << "x\n";
    
    assert_eq(par_cc.num_components(), seq_cc.num_components(),
              "Component counts match");
    
    std::cout << "  PASS: Performance test complete\n";
}

// =============================================================================
// Test: Edge Cases
// =============================================================================

void test_edge_cases() {
    std::cout << "Testing edge cases...\n";
    
    // Empty graph
    adjacency_list<undirected_tag> empty(0);
    auto empty_cc = parallel_connected_components(std::execution::par, empty);
    assert_eq(empty_cc.num_components(), std::size_t{0}, "Empty graph has 0 components");
    
    std::cout << "  PASS: Empty graph\n";
    
    // Single vertex
    adjacency_list<undirected_tag> single(1);
    auto single_bfs = parallel_bfs(std::execution::par, single, std::size_t{0});
    assert_eq(single_bfs.distance_to(0), std::size_t{0}, "Single vertex distance");
    
    auto single_cc = parallel_connected_components(std::execution::par, single);
    assert_eq(single_cc.num_components(), std::size_t{1}, "Single vertex component");
    
    std::cout << "  PASS: Single vertex\n";
    
    // Self-loop (if supported)
    adjacency_list<undirected_tag> loop(1);
    loop.add_edge(0, 0);
    auto loop_cc = parallel_connected_components(std::execution::par, loop);
    assert_eq(loop_cc.num_components(), std::size_t{1}, "Self-loop component");
    
    std::cout << "  PASS: Self-loop\n";
}

} // anonymous namespace

// =============================================================================
// Main Test Runner
// =============================================================================

int main() {
    std::cout << "=== BGL Modern: Parallel Algorithms Tests ===\n\n";
    
    try {
        test_parallel_bfs_correctness();
        test_parallel_bfs_disconnected();
        test_parallel_components_correctness();
        test_parallel_components_fully_connected();
        test_parallel_for_each_vertex();
        test_parallel_for_each_edge();
        test_sequential_fallback();
        test_edge_cases();
        test_large_graph_performance();
        
        std::cout << "\n=== All Tests Passed ===\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
}
