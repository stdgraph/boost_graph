// BGL Modern - std::ranges Integration Tests
// Comprehensive tests for C++20 ranges compatibility
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <bgl/modern/adjacency_list.hpp>
#include <bgl/modern/adjacency_matrix.hpp>
#include <bgl/modern/breadth_first_search.hpp>
#include <bgl/modern/depth_first_search.hpp>
#include <bgl/modern/dijkstra_shortest_paths.hpp>

#include <algorithm>
#include <iostream>
#include <ranges>
#include <vector>
#include <cassert>

using namespace bgl;

// =============================================================================
// Test vertices(g) with std::ranges algorithms
// =============================================================================

void test_vertices_with_for_each() {
    std::cout << "Testing vertices(g) with std::ranges::for_each...\n";
    
    adjacency_list<directed_tag> g(6);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    g.add_edge(3, 4);
    g.add_edge(4, 5);
    
    // Test that vertices() works with std::ranges::for_each
    std::vector<std::size_t> visited_vertices;
    std::ranges::for_each(vertices(g), [&](auto v) {
        visited_vertices.push_back(v);
    });
    
    assert(visited_vertices.size() == 6);
    std::vector<std::size_t> expected = {0, 1, 2, 3, 4, 5};
    assert(visited_vertices == expected);
    
    std::cout << "  ✓ vertices(g) works with std::ranges::for_each\n";
}

void test_vertices_with_count_if() {
    std::cout << "Testing vertices(g) with std::ranges::count_if...\n";
    
    adjacency_list<directed_tag> g(10);
    for (std::size_t i = 0; i < 9; ++i) {
        g.add_edge(i, i + 1);
    }
    
    // Count vertices with even indices
    auto count = std::ranges::count_if(vertices(g), [](auto v) {
        return v % 2 == 0;
    });
    
    assert(count == 5);  // 0, 2, 4, 6, 8
    
    std::cout << "  ✓ vertices(g) works with std::ranges::count_if\n";
}

void test_vertices_with_transform() {
    std::cout << "Testing vertices(g) with std::views::transform...\n";
    
    adjacency_list<directed_tag> g(5);
    
    // Transform vertices to their out-degrees
    auto degrees = vertices(g) 
        | std::views::transform([&g](auto v) { return out_degree(v, g); });
    
    std::vector<std::size_t> degree_vec;
    std::ranges::copy(degrees, std::back_inserter(degree_vec));
    
    assert(degree_vec.size() == 5);
    
    std::cout << "  ✓ vertices(g) works with std::views::transform\n";
}

// =============================================================================
// Test out_edges(v, g) with std::ranges algorithms
// =============================================================================

void test_out_edges_with_filter() {
    std::cout << "Testing out_edges(v, g) with std::views::filter...\n";
    
    // Create weighted graph
    struct EdgeProps {
        double weight;
    };
    adjacency_list<directed_tag, no_property, EdgeProps> g(5);
    
    g.add_edge(0, 1, {.weight = 1.0});
    g.add_edge(0, 2, {.weight = 5.0});
    g.add_edge(0, 3, {.weight = 3.0});
    g.add_edge(0, 4, {.weight = 7.0});
    
    // Filter edges with weight > 3.0
    auto heavy_edges = out_edges(0, g) 
        | std::views::filter([&g](auto e) { return g[e].weight > 3.0; });
    
    std::size_t count = std::ranges::distance(heavy_edges);
    assert(count == 2);  // edges to 2 and 4
    
    std::cout << "  ✓ out_edges(v, g) works with std::views::filter\n";
}

void test_out_edges_with_transform() {
    std::cout << "Testing out_edges(v, g) with std::views::transform...\n";
    
    adjacency_list<directed_tag> g(4);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(0, 3);
    
    // Transform edges to their targets
    auto targets = out_edges(0, g)
        | std::views::transform([&g](auto e) { return target(e, g); });
    
    std::vector<std::size_t> target_vec;
    std::ranges::copy(targets, std::back_inserter(target_vec));
    
    assert(target_vec.size() == 3);
    std::vector<std::size_t> expected_targets = {1, 2, 3};
    assert(target_vec == expected_targets);
    
    std::cout << "  ✓ out_edges(v, g) works with std::views::transform\n";
}

void test_out_edges_with_take() {
    std::cout << "Testing out_edges(v, g) with std::views::take...\n";
    
    adjacency_list<directed_tag> g(6);
    for (std::size_t i = 1; i < 6; ++i) {
        g.add_edge(0, i);
    }
    
    // Take only first 3 edges
    auto first_three = out_edges(0, g) | std::views::take(3);
    
    std::size_t count = std::ranges::distance(first_three);
    assert(count == 3);
    
    std::cout << "  ✓ out_edges(v, g) works with std::views::take\n";
}

// =============================================================================
// Test algorithm outputs with std::ranges algorithms
// =============================================================================

void test_bfs_result_with_ranges() {
    std::cout << "Testing BFS result with std::ranges...\n";
    
    adjacency_list<directed_tag> g(6);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    g.add_edge(2, 4);
    g.add_edge(3, 5);
    g.add_edge(4, 5);
    
    auto result = breadth_first_search(g, 0);
    
    // Get distance map as a vector
    std::vector<std::size_t> distances;
    for (auto v : vertices(g)) {
        distances.push_back(result.distance_map()(v));
    }
    
    // Use std::ranges::sort
    std::ranges::sort(distances);
    assert(distances[0] == 0);  // Source vertex
    // Furthest vertex is 3 steps: 0->1->3->5 or 0->2->4->5
    assert(distances[5] == 3);
    
    // Use std::ranges::max_element
    auto max_dist = std::ranges::max_element(distances);
    assert(*max_dist == 3);
    
    std::cout << "  ✓ BFS result works with std::ranges::sort and max_element\n";
}

void test_dijkstra_result_with_ranges() {
    std::cout << "Testing Dijkstra result with std::ranges...\n";
    
    struct EdgeProps {
        double weight;
    };
    adjacency_list<directed_tag, no_property, EdgeProps> g(5);
    
    g.add_edge(0, 1, {.weight = 1.0});
    g.add_edge(0, 2, {.weight = 4.0});
    g.add_edge(1, 2, {.weight = 2.0});
    g.add_edge(1, 3, {.weight = 5.0});
    g.add_edge(2, 3, {.weight = 1.0});
    g.add_edge(3, 4, {.weight = 3.0});
    
    auto weight_map = [&g](const auto& e) { return g[e].weight; };
    auto result = dijkstra_shortest_paths(g, 0, weight_map);
    
    // Collect distances into vector
    std::vector<double> distances;
    for (auto v : vertices(g)) {
        distances.push_back(result.distance_map()(v));
    }
    
    // Use std::ranges::copy to create a sorted copy
    std::vector<double> sorted_distances;
    std::ranges::copy(distances, std::back_inserter(sorted_distances));
    std::ranges::sort(sorted_distances);
    
    assert(sorted_distances[0] == 0.0);  // Source
    assert(sorted_distances[4] == 7.0);  // 0->1->2->3->4 = 1+2+1+3 = 7
    
    std::cout << "  ✓ Dijkstra result works with std::ranges::copy and sort\n";
}

void test_dfs_result_with_ranges() {
    std::cout << "Testing DFS result with std::ranges...\n";
    
    adjacency_list<directed_tag> g(5);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    g.add_edge(2, 4);
    
    auto result = depth_first_search(g, 0);
    
    // Check that all vertices have valid discovery/finish times
    bool all_discovered = true;
    for (auto v : vertices(g)) {
        if (result.discovery_time_map()(v) < 0) {
            all_discovered = false;
            break;
        }
    }
    assert(all_discovered);
    
    // Check finish times are after discovery times
    bool valid_times = true;
    for (auto v : vertices(g)) {
        if (result.finish_time_map()(v) <= result.discovery_time_map()(v)) {
            valid_times = false;
            break;
        }
    }
    assert(valid_times);
    
    std::cout << "  ✓ DFS result works with std::ranges::all_of\n";
}

// =============================================================================
// Test range pipelines
// =============================================================================

void test_complex_range_pipeline() {
    std::cout << "Testing complex range pipeline...\n";
    
    struct EdgeProps {
        double weight;
    };
    adjacency_list<directed_tag, no_property, EdgeProps> g(10);
    
    // Create graph with various edge weights
    for (std::size_t i = 0; i < 9; ++i) {
        g.add_edge(i, i + 1, {.weight = static_cast<double>(i + 1)});
    }
    
    // Complex pipeline: get vertices with out-degree > 0,
    // transform to their out-edges, filter by weight > 5,
    // transform to target vertices
    auto result = vertices(g)
        | std::views::filter([&g](auto v) { return out_degree(v, g) > 0; })
        | std::views::transform([&g](auto v) {
            return out_edges(v, g) 
                | std::views::filter([&g](auto e) { return g[e].weight > 5.0; })
                | std::views::transform([&g](auto e) { return target(e, g); });
        })
        | std::views::join;  // Flatten nested ranges
    
    std::vector<std::size_t> targets;
    std::ranges::copy(result, std::back_inserter(targets));
    
    // Edges with weight > 5: 5->6 (6), 6->7 (7), 7->8 (8), 8->9 (9)
    assert(targets.size() == 4);
    
    std::cout << "  ✓ Complex range pipelines work correctly\n";
}

void test_vertices_range_properties() {
    std::cout << "Testing vertices range properties...\n";
    
    adjacency_list<directed_tag> g(100);
    
    auto verts = vertices(g);
    
    // Test that vertices() satisfies std::ranges::forward_range
    static_assert(std::ranges::forward_range<decltype(verts)>);
    
    // Test that we can iterate multiple times
    std::size_t count1 = std::ranges::distance(verts);
    std::size_t count2 = std::ranges::distance(verts);
    assert(count1 == count2);
    assert(count1 == 100);
    
    // Test that we can use range-based for
    std::size_t count3 = 0;
    for ([[maybe_unused]] auto v : verts) {
        ++count3;
    }
    assert(count3 == 100);
    
    std::cout << "  ✓ Vertices range satisfies forward_range requirements\n";
}

void test_edges_range_properties() {
    std::cout << "Testing edges range properties...\n";
    
    adjacency_list<directed_tag> g(10);
    for (std::size_t i = 0; i < 5; ++i) {
        g.add_edge(0, i + 1);
    }
    
    auto edges = out_edges(0, g);
    
    // Test that out_edges() satisfies std::ranges::forward_range
    static_assert(std::ranges::forward_range<decltype(edges)>);
    
    // Test multiple iterations
    std::size_t count1 = std::ranges::distance(edges);
    std::size_t count2 = std::ranges::distance(edges);
    assert(count1 == count2);
    assert(count1 == 5);
    
    std::cout << "  ✓ Edges range satisfies forward_range requirements\n";
}

// =============================================================================
// Test with std::views compositions
// =============================================================================

void test_views_drop_and_take() {
    std::cout << "Testing std::views::drop and take...\n";
    
    adjacency_list<directed_tag> g(20);
    
    // Take middle 10 vertices (drop 5, take 10)
    auto middle_vertices = vertices(g) 
        | std::views::drop(5) 
        | std::views::take(10);
    
    std::vector<std::size_t> middle;
    std::ranges::copy(middle_vertices, std::back_inserter(middle));
    
    assert(middle.size() == 10);
    assert(middle[0] == 5);
    assert(middle[9] == 14);
    
    std::cout << "  ✓ std::views::drop and take work correctly\n";
}

void test_views_reverse() {
    std::cout << "Testing std::views::reverse...\n";
    
    adjacency_list<directed_tag> g(5);
    
    // Note: reverse requires bidirectional_range, which our vertices() may not support
    // This tests if we can collect and reverse
    std::vector<std::size_t> verts;
    std::ranges::copy(vertices(g), std::back_inserter(verts));
    
    auto reversed = verts | std::views::reverse;
    std::vector<std::size_t> reversed_vec;
    std::ranges::copy(reversed, std::back_inserter(reversed_vec));
    
    assert(reversed_vec[0] == 4);
    assert(reversed_vec[4] == 0);
    
    std::cout << "  ✓ Collected vertices work with std::views::reverse\n";
}

void test_ranges_find_if() {
    std::cout << "Testing std::ranges::find_if...\n";
    
    struct VertexProps {
        int value;
    };
    adjacency_list<directed_tag, VertexProps> g(10);
    
    // Set some vertex values
    g[5].value = 42;
    
    // Find vertex with value 42
    auto it = std::ranges::find_if(vertices(g), [&g](auto v) {
        return g[v].value == 42;
    });
    
    assert(it != std::ranges::end(vertices(g)));
    assert(*it == 5);
    
    std::cout << "  ✓ std::ranges::find_if works with vertices\n";
}

// =============================================================================
// Main Test Driver
// =============================================================================

int main() {
    std::cout << "=== BGL Modern std::ranges Integration Tests ===\n\n";
    
    try {
        // vertices(g) with std::ranges algorithms
        test_vertices_with_for_each();
        test_vertices_with_count_if();
        test_vertices_with_transform();
        
        // out_edges(v, g) with std::ranges algorithms
        test_out_edges_with_filter();
        test_out_edges_with_transform();
        test_out_edges_with_take();
        
        // Algorithm outputs with std::ranges
        test_bfs_result_with_ranges();
        test_dijkstra_result_with_ranges();
        test_dfs_result_with_ranges();
        
        // Complex pipelines
        test_complex_range_pipeline();
        test_vertices_range_properties();
        test_edges_range_properties();
        
        // std::views compositions
        test_views_drop_and_take();
        test_views_reverse();
        test_ranges_find_if();
        
        std::cout << "\n✓ All std::ranges integration tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}
