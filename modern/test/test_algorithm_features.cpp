// BGL Modern - Algorithm Features Tests
// Tests for structured returns, lambda visitors, and named parameters
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <bgl/modern/adjacency_list.hpp>
#include <bgl/modern/breadth_first_search.hpp>
#include <bgl/modern/depth_first_search.hpp>
#include <bgl/modern/dijkstra_shortest_paths.hpp>
#include <bgl/modern/bellman_ford_shortest_paths.hpp>

#include <iostream>
#include <cassert>
#include <vector>

using namespace bgl;

// =============================================================================
// Test Structured Returns
// =============================================================================

void test_bfs_structured_return() {
    std::cout << "Testing BFS structured return...\n";
    
    adjacency_list<directed_tag> g(6);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    g.add_edge(2, 4);
    g.add_edge(3, 5);
    
    // Structured return with named accessors
    auto result = breadth_first_search(g, 0);
    
    // Access distance map
    assert(result.distance_map()(0) == 0);
    assert(result.distance_map()(1) == 1);
    assert(result.distance_map()(3) == 2);
    
    // Access predecessor map
    assert(result.predecessor_map()(1) == 0);
    assert(result.predecessor_map()(2) == 0);
    
    std::cout << "  ✓ BFS structured return works\n";
}

void test_dijkstra_structured_return() {
    std::cout << "Testing Dijkstra structured return...\n";
    
    struct EdgeProps {
        double weight;
    };
    adjacency_list<directed_tag, no_property, EdgeProps> g(4);
    
    g.add_edge(0, 1, {.weight = 1.0});
    g.add_edge(1, 2, {.weight = 2.0});
    g.add_edge(0, 2, {.weight = 5.0});
    g.add_edge(2, 3, {.weight = 1.0});
    
    auto weight_map = [&g](const auto& e) { return g[e].weight; };
    auto result = dijkstra_shortest_paths(g, 0, weight_map);
    
    // Verify distances
    assert(result.distance_map()(0) == 0.0);
    assert(result.distance_map()(1) == 1.0);
    assert(result.distance_map()(2) == 3.0);
    assert(result.distance_map()(3) == 4.0);
    
    // Verify predecessors
    assert(result.predecessor_map()(1) == 0);
    assert(result.predecessor_map()(2) == 1);
    
    std::cout << "  ✓ Dijkstra structured return works\n";
}

void test_dfs_structured_return() {
    std::cout << "Testing DFS structured return...\n";
    
    adjacency_list<directed_tag> g(5);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    g.add_edge(2, 4);
    
    auto result = depth_first_search(g, 0);
    
    // Verify discovery and finish times exist and are ordered
    for (auto v : vertices(g)) {
        assert(result.discovery_time_map()(v) < result.finish_time_map()(v));
    }
    
    std::cout << "  ✓ DFS structured return works\n";
}

// =============================================================================
// Test Lambda Visitors
// =============================================================================

void test_bfs_lambda_visitor() {
    std::cout << "Testing BFS with lambda visitor...\n";
    
    adjacency_list<directed_tag> g(5);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    g.add_edge(2, 4);
    
    std::vector<std::size_t> discover_order;
    
    auto callbacks = on_discover_vertex([&](auto v, const auto&) {
        discover_order.push_back(v);
    });
    
    breadth_first_search(g, 0, callbacks);
    
    assert(discover_order.size() == 5);
    assert(discover_order[0] == 0);  // Source first
    
    std::cout << "  ✓ BFS lambda visitor works\n";
}

void test_dfs_lambda_visitor() {
    std::cout << "Testing DFS with lambda visitor...\n";
    
    adjacency_list<directed_tag> g(5);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 0);  // Back edge
    g.add_edge(2, 3);
    g.add_edge(3, 4);
    
    std::vector<std::size_t> back_edge_targets;
    
    auto callbacks = on_back_edge([&](auto e, const auto& graph) {
        back_edge_targets.push_back(target(e, graph));
    });
    
    depth_first_search(g, 0, callbacks);
    
    assert(back_edge_targets.size() == 1);  // One back edge (2->0)
    assert(back_edge_targets[0] == 0);
    
    std::cout << "  ✓ DFS lambda visitor works\n";
}

void test_dijkstra_lambda_visitor() {
    std::cout << "Testing Dijkstra with weight map lambda...\n";
    
    struct EdgeProps {
        double weight;
    };
    adjacency_list<directed_tag, no_property, EdgeProps> g(3);
    
    g.add_edge(0, 1, {.weight = 1.0});
    g.add_edge(1, 2, {.weight = 2.0});
    
    // Lambda as weight map
    auto weight_map = [&g](const auto& e) { return g[e].weight; };
    auto result = dijkstra_shortest_paths(g, 0, weight_map);
    
    assert(result.distance_map()(0) == 0.0);
    assert(result.distance_map()(1) == 1.0);
    assert(result.distance_map()(2) == 3.0);
    
    std::cout << "  ✓ Dijkstra weight map lambda works\n";
}

// =============================================================================
// Test Named Parameter Structs
// =============================================================================

void test_dijkstra_params() {
    std::cout << "Testing Dijkstra named parameters...\n";
    
    struct EdgeProps {
        double weight;
    };
    adjacency_list<directed_tag, no_property, EdgeProps> g(4);
    
    g.add_edge(0, 1, {.weight = 2.0});
    g.add_edge(1, 2, {.weight = 3.0});
    g.add_edge(0, 3, {.weight = 10.0});
    
    auto weight_map = [&g](const auto& e) { return g[e].weight; };
    
    // Use weight map directly (named parameter)
    auto result = dijkstra_shortest_paths(g, 0, weight_map);
    
    assert(result.distance_map()(2) == 5.0);
    
    std::cout << "  ✓ Dijkstra named parameters work\n";
}

void test_bfs_params() {
    std::cout << "Testing BFS with callback parameter...\n";
    
    adjacency_list<directed_tag> g(4);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    
    std::vector<std::size_t> visited;
    
    // Use callbacks as parameter
    auto callbacks = on_discover_vertex([&](auto v, const auto&) { 
        visited.push_back(v); 
    });
    
    breadth_first_search(g, 0, callbacks);
    
    assert(visited.size() == 4);
    
    std::cout << "  ✓ BFS callback parameter works\n";
}

// =============================================================================
// Test API Compatibility
// =============================================================================

void test_return_value_usage() {
    std::cout << "Testing return value direct usage...\n";
    
    adjacency_list<directed_tag> g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    
    // Use result directly without storing
    assert(breadth_first_search(g, 0).distance_map()(2) == 2);
    
    std::cout << "  ✓ Direct return value usage works\n";
}

void test_chained_algorithms() {
    std::cout << "Testing chained algorithm usage...\n";
    
    struct EdgeProps {
        double weight;
    };
    adjacency_list<directed_tag, no_property, EdgeProps> g(4);
    
    g.add_edge(0, 1, {.weight = 1.0});
    g.add_edge(1, 2, {.weight = 1.0});
    g.add_edge(2, 3, {.weight = 1.0});
    
    auto weight_map = [&g](const auto& e) { return g[e].weight; };
    
    // Run multiple algorithms on same graph
    auto bfs_result = breadth_first_search(g, 0);
    auto dijk_result = dijkstra_shortest_paths(g, 0, weight_map);
    
    // Both should give same distances for unit weights
    assert(bfs_result.distance_map()(3) == 3);
    assert(dijk_result.distance_map()(3) == 3.0);
    
    std::cout << "  ✓ Chained algorithms work\n";
}

// =============================================================================
// Main Test Driver
// =============================================================================

int main() {
    std::cout << "=== BGL Modern Algorithm Features Tests ===\n\n";
    
    try {
        // Structured returns
        test_bfs_structured_return();
        test_dijkstra_structured_return();
        test_dfs_structured_return();
        
        // Lambda visitors
        test_bfs_lambda_visitor();
        test_dfs_lambda_visitor();
        test_dijkstra_lambda_visitor();
        
        // Named parameters
        test_dijkstra_params();
        test_bfs_params();
        
        // API compatibility
        test_return_value_usage();
        test_chained_algorithms();
        
        std::cout << "\n✓ All algorithm features tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << "\n";
        return 1;
    }
}
