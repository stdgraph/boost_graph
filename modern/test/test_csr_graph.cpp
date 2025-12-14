// BGL Modern - Compressed Sparse Row Graph Tests
// Tests for compressed_sparse_row_graph container
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <bgl/modern/compressed_sparse_row_graph.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <span>
#include <utility>

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
    std::cout << "Testing CSR basic construction...\n";
    
    bgl::compressed_sparse_row_graph<bgl::directed_tag> g(10);
    
    check(g.num_vertices() == 10, "num_vertices() returns 10");
    check(g.num_edges() == 0, "num_edges() returns 0 for empty graph");
}

// =============================================================================
// Test: Construction from Edge List
// =============================================================================

void test_edge_list_construction() {
    std::cout << "Testing CSR construction from edge list...\n";
    
    std::vector<std::pair<std::size_t, std::size_t>> edges = {
        {0, 1}, {0, 2}, {1, 2}, {2, 3}, {3, 0}
    };
    
    bgl::compressed_sparse_row_graph<bgl::directed_tag> g(4, edges);
    
    check(g.num_vertices() == 4, "num_vertices() is 4");
    check(g.num_edges() == 5, "num_edges() is 5");
}

// =============================================================================
// Test: Std::span Access
// =============================================================================

void test_span_access() {
    std::cout << "Testing CSR std::span access...\n";
    
    std::vector<std::pair<std::size_t, std::size_t>> edges = {
        {0, 1}, {0, 2}, {0, 3}, {1, 2}
    };
    
    bgl::compressed_sparse_row_graph<bgl::directed_tag> g(4, edges);
    
    std::span<const std::size_t> adj0 = g.adjacent_vertices(0);
    check(adj0.size() == 3, "Vertex 0 has 3 neighbors");
    
    std::span<const std::size_t> adj1 = g.adjacent_vertices(1);
    check(adj1.size() == 1, "Vertex 1 has 1 neighbor");
    
    std::span<const std::size_t> adj3 = g.adjacent_vertices(3);
    check(adj3.empty(), "Vertex 3 has no neighbors");
    
    // Check row_start span
    std::span<const std::size_t> row = g.row_start();
    check(row.size() == 5, "row_start has num_vertices+1 elements");
}

// =============================================================================
// Test: Vertex Properties
// =============================================================================

void test_vertex_properties() {
    std::cout << "Testing CSR vertex properties...\n";
    
    std::vector<std::pair<std::size_t, std::size_t>> edges = {{0, 1}};
    bgl::compressed_sparse_row_graph<bgl::directed_tag, std::string> g(3, edges);
    
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
    std::cout << "Testing CSR edge properties...\n";
    
    std::vector<std::pair<std::size_t, std::size_t>> edges = {
        {0, 1}, {0, 2}, {1, 2}
    };
    std::vector<double> weights = {1.0, 2.0, 3.0};
    
    bgl::compressed_sparse_row_graph<bgl::directed_tag, bgl::no_property, double> 
        g(3, edges, weights);
    
    std::span<const double> props = g.adjacent_edge_properties(0);
    check(props.size() == 2, "Vertex 0 has 2 edge properties");
    check(props[0] == 1.0, "First edge weight is 1.0");
    check(props[1] == 2.0, "Second edge weight is 2.0");
}

// =============================================================================
// Test: Out Edges
// =============================================================================

void test_out_edges() {
    std::cout << "Testing CSR out_edges...\n";
    
    std::vector<std::pair<std::size_t, std::size_t>> edges = {
        {0, 1}, {0, 2}, {0, 3}
    };
    
    bgl::compressed_sparse_row_graph<bgl::directed_tag> g(4, edges);
    
    std::vector<std::size_t> targets;
    for (auto e : bgl::out_edges(std::size_t{0}, g)) {
        targets.push_back(bgl::target(e, g));
    }
    
    check(targets.size() == 3, "out_edges returns 3 edges");
    check(targets[0] == 1 || targets[0] == 2 || targets[0] == 3, "Valid target");
}

// =============================================================================
// Test: Degree
// =============================================================================

void test_degree() {
    std::cout << "Testing CSR degree...\n";
    
    std::vector<std::pair<std::size_t, std::size_t>> edges = {
        {0, 1}, {0, 2}, {0, 3}, {1, 2}
    };
    
    bgl::compressed_sparse_row_graph<bgl::directed_tag> g(4, edges);
    
    check(bgl::out_degree(std::size_t{0}, g) == 3, "out_degree(0) = 3");
    check(bgl::out_degree(std::size_t{1}, g) == 1, "out_degree(1) = 1");
    check(bgl::out_degree(std::size_t{2}, g) == 0, "out_degree(2) = 0");
    check(bgl::out_degree(std::size_t{3}, g) == 0, "out_degree(3) = 0");
}

// =============================================================================
// Test: Undirected Graph
// =============================================================================

void test_undirected_graph() {
    std::cout << "Testing CSR undirected graph...\n";
    
    std::vector<std::pair<std::size_t, std::size_t>> edges = {
        {0, 1}, {1, 2}
    };
    
    bgl::compressed_sparse_row_graph<bgl::undirected_tag> g(3, edges);
    
    // Undirected stores edges in both directions
    check(g.num_edges() == 2, "Undirected: num_edges is logical count");
    
    // Check both directions accessible
    std::span<const std::size_t> adj0 = g.adjacent_vertices(0);
    std::span<const std::size_t> adj1 = g.adjacent_vertices(1);
    
    check(!adj0.empty(), "Vertex 0 has neighbors");
    check(!adj1.empty(), "Vertex 1 has neighbors");
}

// =============================================================================
// Test: Vertex Iteration
// =============================================================================

void test_vertex_iteration() {
    std::cout << "Testing CSR vertex iteration...\n";
    
    bgl::compressed_sparse_row_graph<bgl::directed_tag> g(5);
    
    std::vector<std::size_t> verts;
    for (auto v : bgl::vertices(g)) {
        verts.push_back(v);
    }
    
    check(verts.size() == 5, "vertices() returns 5 vertices");
    check(verts[0] == 0 && verts[4] == 4, "Vertices are 0..4");
}

// =============================================================================
// Test: Raw Data Access
// =============================================================================

void test_raw_data_access() {
    std::cout << "Testing CSR raw data access...\n";
    
    std::vector<std::pair<std::size_t, std::size_t>> edges = {
        {0, 1}, {0, 2}, {1, 2}
    };
    
    bgl::compressed_sparse_row_graph<bgl::directed_tag> g(3, edges);
    
    auto row_start = g.row_start();
    auto targets = g.targets();
    
    check(row_start.size() == 4, "row_start has 4 elements");
    check(targets.size() == 3, "targets has 3 elements");
    
    // Verify CSR structure: row_start[v+1] - row_start[v] = out_degree(v)
    check(row_start[1] - row_start[0] == 2, "Vertex 0 has 2 out-edges");
    check(row_start[2] - row_start[1] == 1, "Vertex 1 has 1 out-edge");
    check(row_start[3] - row_start[2] == 0, "Vertex 2 has 0 out-edges");
}

} // anonymous namespace

int main() {
    std::cout << "=== BGL Modern: CSR Graph Container Tests ===\n\n";
    
    test_basic_construction();
    test_edge_list_construction();
    test_span_access();
    test_vertex_properties();
    test_edge_properties();
    test_out_edges();
    test_degree();
    test_undirected_graph();
    test_vertex_iteration();
    test_raw_data_access();
    
    std::cout << "\n=== Results: " << tests_passed << "/" << tests_run << " tests passed ===\n";
    
    return (tests_passed == tests_run) ? 0 : 1;
}
