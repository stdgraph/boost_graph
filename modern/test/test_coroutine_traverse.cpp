// BGL Modern - Coroutine Traversal Tests
// Tests for generator<T>, bfs_traverse, dfs_traverse
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <bgl/modern/generator.hpp>
#include <bgl/modern/coroutine_traverse.hpp>
#include <bgl/modern/adjacency_list.hpp>

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

namespace {

int tests_run = 0;
int tests_passed = 0;

void check(bool condition, const char* test_name) {
    ++tests_run;
    if (condition) {
        ++tests_passed;
        std::cout << "  PASS: " << test_name << "\n";
    } else {
        std::cout << "  FAIL: " << test_name << "\n";
    }
}

// Simple test graph:
//     0 --- 1 --- 2
//     |     |
//     3 --- 4

bgl::simple_adjacency_list<bgl::undirected_tag> make_test_graph() {
    bgl::simple_adjacency_list<bgl::undirected_tag> g;
    for (int i = 0; i < 5; ++i) g.add_vertex();
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(0, 3);
    g.add_edge(1, 4);
    g.add_edge(3, 4);
    return g;
}

// =============================================================================
// Test: Basic Generator
// =============================================================================

bgl::generator<int> count_to(int n) {
    for (int i = 0; i < n; ++i) {
        co_yield i;
    }
}

void test_basic_generator() {
    std::cout << "Testing basic generator...\n";
    
    std::vector<int> values;
    for (int x : count_to(5)) {
        values.push_back(x);
    }
    
    check(values.size() == 5, "Generator yields 5 values");
    check(values == std::vector<int>{0, 1, 2, 3, 4}, "Values are 0..4");
}

// =============================================================================
// Test: Generator Early Termination
// =============================================================================

void test_generator_early_termination() {
    std::cout << "Testing generator early termination...\n";
    
    std::vector<int> values;
    for (int x : count_to(100)) {
        values.push_back(x);
        if (x >= 2) break;  // Early termination
    }
    
    check(values.size() == 3, "Early termination after 3 values");
    check(values == std::vector<int>{0, 1, 2}, "Values are 0..2");
}

// =============================================================================
// Test: Empty Generator
// =============================================================================

bgl::generator<int> empty_gen() {
    co_return;  // Yield nothing
}

void test_empty_generator() {
    std::cout << "Testing empty generator...\n";
    
    std::vector<int> values;
    for (int x : empty_gen()) {
        values.push_back(x);
    }
    
    check(values.empty(), "Empty generator yields nothing");
}

// =============================================================================
// Test: BFS Traverse
// =============================================================================

void test_bfs_traverse() {
    std::cout << "Testing bfs_traverse...\n";
    
    auto g = make_test_graph();
    
    std::vector<std::size_t> order;
    for (auto v : bgl::bfs_traverse(g, std::size_t{0})) {
        order.push_back(v);
    }
    
    check(order.size() == 5, "BFS visits all 5 vertices");
    check(order[0] == 0, "BFS starts at vertex 0");
    
    // All vertices should be visited exactly once
    std::set<std::size_t> unique(order.begin(), order.end());
    check(unique.size() == 5, "Each vertex visited exactly once");
}

// =============================================================================
// Test: BFS Traverse Early Termination
// =============================================================================

void test_bfs_early_termination() {
    std::cout << "Testing BFS early termination...\n";
    
    auto g = make_test_graph();
    
    std::vector<std::size_t> order;
    for (auto v : bgl::bfs_traverse(g, std::size_t{0})) {
        order.push_back(v);
        if (order.size() >= 3) break;
    }
    
    check(order.size() == 3, "BFS stops after 3 vertices");
}

// =============================================================================
// Test: BFS Traverse Events
// =============================================================================

void test_bfs_events() {
    std::cout << "Testing bfs_traverse_events...\n";
    
    auto g = make_test_graph();
    
    int discover_count = 0;
    int examine_count = 0;
    int finish_count = 0;
    
    for (auto step : bgl::bfs_traverse_events(g, std::size_t{0})) {
        switch (step.event) {
            case bgl::bfs_event::discover_vertex: ++discover_count; break;
            case bgl::bfs_event::examine_vertex: ++examine_count; break;
            case bgl::bfs_event::finish_vertex: ++finish_count; break;
            default: break;
        }
    }
    
    check(discover_count == 5, "5 discover events");
    check(examine_count == 5, "5 examine events");
    check(finish_count == 5, "5 finish events");
}

// =============================================================================
// Test: DFS Traverse
// =============================================================================

void test_dfs_traverse() {
    std::cout << "Testing dfs_traverse...\n";
    
    auto g = make_test_graph();
    
    std::vector<std::size_t> order;
    for (auto v : bgl::dfs_traverse(g, std::size_t{0})) {
        order.push_back(v);
    }
    
    check(order.size() == 5, "DFS visits all 5 vertices");
    check(order[0] == 0, "DFS starts at vertex 0");
    
    std::set<std::size_t> unique(order.begin(), order.end());
    check(unique.size() == 5, "Each vertex visited exactly once");
}

// =============================================================================
// Test: DFS Traverse Early Termination
// =============================================================================

void test_dfs_early_termination() {
    std::cout << "Testing DFS early termination...\n";
    
    auto g = make_test_graph();
    
    std::vector<std::size_t> order;
    for (auto v : bgl::dfs_traverse(g, std::size_t{0})) {
        order.push_back(v);
        if (order.size() >= 3) break;
    }
    
    check(order.size() == 3, "DFS stops after 3 vertices");
}

// =============================================================================
// Test: DFS Traverse Events
// =============================================================================

void test_dfs_events() {
    std::cout << "Testing dfs_traverse_events...\n";
    
    auto g = make_test_graph();
    
    int discover_count = 0;
    int finish_count = 0;
    
    for (auto step : bgl::dfs_traverse_events(g, std::size_t{0})) {
        switch (step.event) {
            case bgl::dfs_event::discover_vertex: ++discover_count; break;
            case bgl::dfs_event::finish_vertex: ++finish_count; break;
            default: break;
        }
    }
    
    check(discover_count == 5, "5 discover events");
    check(finish_count == 5, "5 finish events");
}

// =============================================================================
// Test: BFS Traverse All (Multiple Components)
// =============================================================================

void test_bfs_traverse_all() {
    std::cout << "Testing bfs_traverse_all with disconnected graph...\n";
    
    // Create disconnected graph: 0-1-2 and 3-4
    bgl::simple_adjacency_list<bgl::undirected_tag> g;
    for (int i = 0; i < 5; ++i) g.add_vertex();
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(3, 4);
    
    std::vector<std::size_t> order;
    for (auto v : bgl::bfs_traverse_all(g)) {
        order.push_back(v);
    }
    
    check(order.size() == 5, "BFS all visits all 5 vertices");
    
    std::set<std::size_t> unique(order.begin(), order.end());
    check(unique.size() == 5, "All vertices visited");
}

// =============================================================================
// Test: DFS Traverse All (Multiple Components)
// =============================================================================

void test_dfs_traverse_all() {
    std::cout << "Testing dfs_traverse_all with disconnected graph...\n";
    
    bgl::simple_adjacency_list<bgl::undirected_tag> g;
    for (int i = 0; i < 5; ++i) g.add_vertex();
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(3, 4);
    
    std::vector<std::size_t> order;
    for (auto v : bgl::dfs_traverse_all(g)) {
        order.push_back(v);
    }
    
    check(order.size() == 5, "DFS all visits all 5 vertices");
    
    std::set<std::size_t> unique(order.begin(), order.end());
    check(unique.size() == 5, "All vertices visited");
}

// =============================================================================
// Test: Multi-source BFS
// =============================================================================

void test_bfs_multi_source() {
    std::cout << "Testing bfs_traverse_multi...\n";
    
    auto g = make_test_graph();
    
    std::vector<std::size_t> starts = {0, 2};
    std::vector<std::size_t> order;
    for (auto v : bgl::bfs_traverse_multi(g, starts)) {
        order.push_back(v);
    }
    
    check(order.size() == 5, "Multi-source BFS visits all 5 vertices");
    
    std::set<std::size_t> unique(order.begin(), order.end());
    check(unique.size() == 5, "All vertices visited");
}

// =============================================================================
// Test: Directed Graph Traversal
// =============================================================================

void test_directed_traversal() {
    std::cout << "Testing traversal on directed graph...\n";
    
    // Directed: 0 -> 1 -> 2
    //           |
    //           v
    //           3
    bgl::simple_adjacency_list<bgl::directed_tag> g;
    for (int i = 0; i < 4; ++i) g.add_vertex();
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(0, 3);
    
    std::vector<std::size_t> bfs_order;
    for (auto v : bgl::bfs_traverse(g, std::size_t{0})) {
        bfs_order.push_back(v);
    }
    
    check(bfs_order.size() == 4, "BFS visits all 4 reachable vertices");
    check(bfs_order[0] == 0, "BFS starts at 0");
    
    std::vector<std::size_t> dfs_order;
    for (auto v : bgl::dfs_traverse(g, std::size_t{0})) {
        dfs_order.push_back(v);
    }
    
    check(dfs_order.size() == 4, "DFS visits all 4 reachable vertices");
    check(dfs_order[0] == 0, "DFS starts at 0");
}

// =============================================================================
// Test: Single Vertex Graph
// =============================================================================

void test_single_vertex() {
    std::cout << "Testing traversal on single vertex graph...\n";
    
    bgl::simple_adjacency_list<bgl::directed_tag> g;
    g.add_vertex();
    
    std::vector<std::size_t> bfs_order;
    for (auto v : bgl::bfs_traverse(g, std::size_t{0})) {
        bfs_order.push_back(v);
    }
    
    check(bfs_order.size() == 1, "BFS visits single vertex");
    check(bfs_order[0] == 0, "BFS yields vertex 0");
    
    std::vector<std::size_t> dfs_order;
    for (auto v : bgl::dfs_traverse(g, std::size_t{0})) {
        dfs_order.push_back(v);
    }
    
    check(dfs_order.size() == 1, "DFS visits single vertex");
}

// =============================================================================
// Test: Linear Graph (Path)
// =============================================================================

void test_linear_graph() {
    std::cout << "Testing traversal on linear graph...\n";
    
    // 0 -> 1 -> 2 -> 3 -> 4
    bgl::simple_adjacency_list<bgl::directed_tag> g;
    for (int i = 0; i < 5; ++i) g.add_vertex();
    for (int i = 0; i < 4; ++i) g.add_edge(i, i + 1);
    
    std::vector<std::size_t> bfs_order;
    for (auto v : bgl::bfs_traverse(g, std::size_t{0})) {
        bfs_order.push_back(v);
    }
    
    check(bfs_order == std::vector<std::size_t>{0, 1, 2, 3, 4}, 
          "BFS on path gives sequential order");
    
    std::vector<std::size_t> dfs_order;
    for (auto v : bgl::dfs_traverse(g, std::size_t{0})) {
        dfs_order.push_back(v);
    }
    
    check(dfs_order == std::vector<std::size_t>{0, 1, 2, 3, 4}, 
          "DFS on path gives sequential order");
}

} // anonymous namespace

int main() {
    std::cout << "=== BGL Modern: Coroutine Traversal Tests ===\n\n";
    
    test_basic_generator();
    test_generator_early_termination();
    test_empty_generator();
    test_bfs_traverse();
    test_bfs_early_termination();
    test_bfs_events();
    test_dfs_traverse();
    test_dfs_early_termination();
    test_dfs_events();
    test_bfs_traverse_all();
    test_dfs_traverse_all();
    test_bfs_multi_source();
    test_directed_traversal();
    test_single_vertex();
    test_linear_graph();
    
    std::cout << "\n=== Results: " << tests_passed << "/" << tests_run << " tests passed ===\n";
    
    return (tests_passed == tests_run) ? 0 : 1;
}
