// BGL Modern - Adjacency Matrix Container Tests
// Tests for adjacency_matrix graph container
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <bgl/modern/adjacency_matrix.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <ranges>
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

// =============================================================================
// Test: Basic Construction
// =============================================================================

void test_basic_construction() {
    std::cout << "Testing adjacency_matrix basic construction...\n";
    
    bgl::adjacency_matrix<bgl::directed_tag> g(10);
    
    check(g.num_vertices() == 10, "num_vertices() returns 10");
    check(g.num_edges() == 0, "num_edges() returns 0 for empty graph");
}

// =============================================================================
// Test: Edge Operations
// =============================================================================

void test_edge_operations() {
    std::cout << "Testing adjacency_matrix edge operations...\n";
    
    bgl::adjacency_matrix<bgl::directed_tag> g(5);
    
    auto [e1, added1] = g.add_edge(0, 1);
    check(added1, "First edge added successfully");
    check(g.has_edge(0, 1), "has_edge returns true");
    check(!g.has_edge(1, 0), "Directed graph: reverse edge doesn't exist");
    check(g.num_edges() == 1, "num_edges is 1");
    
    auto [e2, added2] = g.add_edge(0, 1);
    check(!added2, "Duplicate edge not added");
    check(g.num_edges() == 1, "num_edges still 1");
    
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    check(g.num_edges() == 3, "num_edges is 3");
    
    g.remove_edge(1, 2);
    check(!g.has_edge(1, 2), "Edge removed");
    check(g.num_edges() == 2, "num_edges is 2");
}

// =============================================================================
// Test: Undirected Graph
// =============================================================================

void test_undirected_graph() {
    std::cout << "Testing adjacency_matrix undirected graph...\n";
    
    bgl::adjacency_matrix<bgl::undirected_tag> g(5);
    
    g.add_edge(0, 1);
    check(g.has_edge(0, 1), "Forward edge exists");
    check(g.has_edge(1, 0), "Reverse edge exists (undirected)");
    check(g.num_edges() == 1, "Undirected: num_edges counts once");
    
    g.remove_edge(1, 0);
    check(!g.has_edge(0, 1), "Forward edge removed");
    check(!g.has_edge(1, 0), "Reverse edge also removed");
}

// =============================================================================
// Test: Vertex Properties
// =============================================================================

void test_vertex_properties() {
    std::cout << "Testing adjacency_matrix vertex properties...\n";
    
    bgl::adjacency_matrix<bgl::directed_tag, std::string> g(3);
    
    g[0] = "Alice";
    g[1] = "Bob";
    g[2] = "Charlie";
    
    check(g[0] == "Alice", "Vertex 0 property");
    check(g[1] == "Bob", "Vertex 1 property");
    check(g[2] == "Charlie", "Vertex 2 property");
}

// =============================================================================
// Test: Edge Properties
// =============================================================================

void test_edge_properties() {
    std::cout << "Testing adjacency_matrix edge properties...\n";
    
    bgl::adjacency_matrix<bgl::directed_tag, bgl::no_property, double> g(3);
    
    auto [e1, _1] = g.add_edge(0, 1, 1.5);
    auto [e2, _2] = g.add_edge(1, 2, 2.5);
    
    check(g[e1] == 1.5, "Edge 0->1 property is 1.5");
    check(g[e2] == 2.5, "Edge 1->2 property is 2.5");
    
    g[e1] = 3.0;
    check(g[e1] == 3.0, "Edge property updated");
}

// =============================================================================
// Test: Vertex Iteration
// =============================================================================

void test_vertex_iteration() {
    std::cout << "Testing adjacency_matrix vertex iteration...\n";
    
    bgl::adjacency_matrix<bgl::directed_tag> g(5);
    
    auto verts = bgl::vertices(g);
    std::vector<std::size_t> v_list;
    for (auto v : verts) {
        v_list.push_back(v);
    }
    
    check(v_list.size() == 5, "vertices() returns 5 vertices");
    check(v_list[0] == 0 && v_list[4] == 4, "Vertices are 0..4");
}

// =============================================================================
// Test: Out Edge Iteration
// =============================================================================

void test_out_edge_iteration() {
    std::cout << "Testing adjacency_matrix out_edges...\n";
    
    bgl::adjacency_matrix<bgl::directed_tag> g(5);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(0, 3);
    
    std::vector<std::size_t> targets;
    for (auto e : bgl::out_edges(std::size_t{0}, g)) {
        targets.push_back(bgl::target(e, g));
    }
    
    check(targets.size() == 3, "Vertex 0 has 3 out-edges");
    check(std::ranges::find(targets, 1) != targets.end(), "Target 1 found");
    check(std::ranges::find(targets, 2) != targets.end(), "Target 2 found");
    check(std::ranges::find(targets, 3) != targets.end(), "Target 3 found");
}

// =============================================================================
// Test: In Edge Iteration
// =============================================================================

void test_in_edge_iteration() {
    std::cout << "Testing adjacency_matrix in_edges...\n";
    
    bgl::adjacency_matrix<bgl::directed_tag> g(5);
    g.add_edge(0, 3);
    g.add_edge(1, 3);
    g.add_edge(2, 3);
    
    std::vector<std::size_t> sources;
    for (auto e : bgl::in_edges(std::size_t{3}, g)) {
        sources.push_back(bgl::source(e, g));
    }
    
    check(sources.size() == 3, "Vertex 3 has 3 in-edges");
    check(std::ranges::find(sources, 0) != sources.end(), "Source 0 found");
    check(std::ranges::find(sources, 1) != sources.end(), "Source 1 found");
    check(std::ranges::find(sources, 2) != sources.end(), "Source 2 found");
}

// =============================================================================
// Test: Degree Queries
// =============================================================================

void test_degree_queries() {
    std::cout << "Testing adjacency_matrix degree queries...\n";
    
    bgl::adjacency_matrix<bgl::directed_tag> g(5);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(3, 0);
    
    check(bgl::out_degree(std::size_t{0}, g) == 2, "out_degree(0) = 2");
    check(bgl::in_degree(std::size_t{0}, g) == 1, "in_degree(0) = 1");
    check(bgl::out_degree(std::size_t{3}, g) == 1, "out_degree(3) = 1");
}

// =============================================================================
// Test: Edge Lookup
// =============================================================================

void test_edge_lookup() {
    std::cout << "Testing adjacency_matrix edge lookup...\n";
    
    bgl::adjacency_matrix<bgl::directed_tag> g(5);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    
    auto e1 = bgl::edge(std::size_t{0}, std::size_t{1}, g);
    auto e2 = bgl::edge(std::size_t{0}, std::size_t{2}, g);
    
    check(e1.has_value(), "Edge 0->1 found");
    check(!e2.has_value(), "Edge 0->2 not found");
    
    if (e1) {
        check(e1->source == 0 && e1->target == 1, "Edge descriptor correct");
    }
}

// =============================================================================
// Test: Self Loops
// =============================================================================

void test_self_loops() {
    std::cout << "Testing adjacency_matrix self-loops...\n";
    
    bgl::adjacency_matrix<bgl::directed_tag> g(5);
    auto [e, added] = g.add_edge(2, 2);
    
    check(added, "Self-loop added");
    check(g.has_edge(2, 2), "Self-loop exists");
    check(g.num_edges() == 1, "Self-loop counts as one edge");
}

} // anonymous namespace

int main() {
    std::cout << "=== BGL Modern: Adjacency Matrix Container Tests ===\n\n";
    
    test_basic_construction();
    test_edge_operations();
    test_undirected_graph();
    test_vertex_properties();
    test_edge_properties();
    test_vertex_iteration();
    test_out_edge_iteration();
    test_in_edge_iteration();
    test_degree_queries();
    test_edge_lookup();
    test_self_loops();
    
    std::cout << "\n=== Results: " << tests_passed << "/" << tests_run << " tests passed ===\n";
    
    return (tests_passed == tests_run) ? 0 : 1;
}
