// BGL Modern - Grid Graph Tests
// Tests for grid_graph container
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <bgl/modern/grid_graph.hpp>

#include <iostream>
#include <vector>
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
// Test: 2D Grid Basic Construction
// =============================================================================

void test_2d_basic_construction() {
    std::cout << "Testing 2D grid basic construction...\n";
    
    bgl::grid_graph<2> g(10, 20);
    
    check(g.num_vertices() == 200, "10x20 grid has 200 vertices");
    check(g.length(0) == 10, "First dimension is 10");
    check(g.length(1) == 20, "Second dimension is 20");
}

// =============================================================================
// Test: Coordinate Conversion
// =============================================================================

void test_coordinate_conversion() {
    std::cout << "Testing coordinate conversion...\n";
    
    bgl::grid_graph<2> g(10, 10);
    
    // Test vertex_at
    auto v = g.vertex_at({5, 7});
    check(v < g.num_vertices(), "vertex_at returns valid vertex");
    
    // Test coordinates
    auto coords = g.coordinates(v);
    check(coords[0] == 5 && coords[1] == 7, "coordinates() inverts vertex_at()");
    
    // Test round-trip for all vertices
    bool all_valid = true;
    for (std::size_t i = 0; i < g.num_vertices(); ++i) {
        auto c = g.coordinates(i);
        auto v2 = g.vertex_at(c);
        if (v2 != i) {
            all_valid = false;
            break;
        }
    }
    check(all_valid, "Round-trip works for all vertices");
}

// =============================================================================
// Test: 3D Grid
// =============================================================================

void test_3d_grid() {
    std::cout << "Testing 3D grid...\n";
    
    bgl::grid_graph<3> g(5, 6, 7);
    
    check(g.num_vertices() == 5 * 6 * 7, "5x6x7 grid has 210 vertices");
    check(g.length(0) == 5, "First dimension is 5");
    check(g.length(1) == 6, "Second dimension is 6");
    check(g.length(2) == 7, "Third dimension is 7");
    
    // Test coordinate conversion
    auto v = g.vertex_at({2, 3, 4});
    auto coords = g.coordinates(v);
    check(coords[0] == 2 && coords[1] == 3 && coords[2] == 4, 
          "3D coordinate conversion works");
}

// =============================================================================
// Test: Neighbor Access
// =============================================================================

void test_neighbor_access() {
    std::cout << "Testing neighbor access...\n";
    
    bgl::grid_graph<2> g(10, 10);
    
    // Interior vertex
    auto v = g.vertex_at({5, 5});
    
    auto n_pos_x = g.neighbor(v, 0, true);
    auto n_neg_x = g.neighbor(v, 0, false);
    auto n_pos_y = g.neighbor(v, 1, true);
    auto n_neg_y = g.neighbor(v, 1, false);
    
    check(n_pos_x.has_value(), "Interior: positive x neighbor exists");
    check(n_neg_x.has_value(), "Interior: negative x neighbor exists");
    check(n_pos_y.has_value(), "Interior: positive y neighbor exists");
    check(n_neg_y.has_value(), "Interior: negative y neighbor exists");
    
    // Corner vertex (0, 0)
    auto corner = g.vertex_at({0, 0});
    auto c_neg_x = g.neighbor(corner, 0, false);
    auto c_neg_y = g.neighbor(corner, 1, false);
    
    check(!c_neg_x.has_value(), "Corner: no negative x neighbor");
    check(!c_neg_y.has_value(), "Corner: no negative y neighbor");
}

// =============================================================================
// Test: Degree Calculation
// =============================================================================

void test_degree_calculation() {
    std::cout << "Testing degree calculation...\n";
    
    bgl::grid_graph<2> g(10, 10);
    
    // Corner: 2 neighbors
    auto corner = g.vertex_at({0, 0});
    check(g.degree(corner) == 2, "Corner has degree 2");
    
    // Edge: 3 neighbors
    auto edge = g.vertex_at({0, 5});
    check(g.degree(edge) == 3, "Edge has degree 3");
    
    // Interior: 4 neighbors
    auto interior = g.vertex_at({5, 5});
    check(g.degree(interior) == 4, "Interior has degree 4");
}

// =============================================================================
// Test: Edge Count
// =============================================================================

void test_edge_count() {
    std::cout << "Testing edge count...\n";
    
    // 3x3 grid: 
    // Horizontal edges: 2 per row * 3 rows = 6
    // Vertical edges: 3 per column * 2 rows = 6
    // Total: 12
    bgl::grid_graph<2> g(3, 3);
    check(g.num_edges() == 12, "3x3 grid has 12 edges");
    
    // 2x2 grid: 4 edges
    bgl::grid_graph<2> g2(2, 2);
    check(g2.num_edges() == 4, "2x2 grid has 4 edges");
}

// =============================================================================
// Test: Out Edges
// =============================================================================

void test_out_edges() {
    std::cout << "Testing out_edges...\n";
    
    bgl::grid_graph<2> g(10, 10);
    
    auto v = g.vertex_at({5, 5});
    auto edges = bgl::out_edges(v, g);
    
    check(edges.size() == 4, "Interior vertex has 4 out-edges");
    
    // Verify all neighbors are valid
    bool all_valid = true;
    for (const auto& e : edges) {
        auto t = bgl::target(e, g);
        if (t >= g.num_vertices()) {
            all_valid = false;
            break;
        }
    }
    check(all_valid, "All edge targets are valid vertices");
}

// =============================================================================
// Test: Wrapped Grid
// =============================================================================

void test_wrapped_grid() {
    std::cout << "Testing wrapped grid (torus)...\n";
    
    bgl::grid_graph<2, true> g(10, 10);  // Wrapped
    
    // Corner now has 4 neighbors (wraps around)
    auto corner = g.vertex_at({0, 0});
    check(g.degree(corner) == 4, "Wrapped corner has degree 4");
    
    // Neighbor at negative x wraps to opposite side
    auto n = g.neighbor(corner, 0, false);
    check(n.has_value(), "Wrapped: negative neighbor exists");
    
    if (n) {
        auto coords = g.coordinates(*n);
        check(coords[0] == 9, "Wrapped: neighbor at x=9");
    }
}

// =============================================================================
// Test: Has Edge
// =============================================================================

void test_has_edge() {
    std::cout << "Testing has_edge...\n";
    
    bgl::grid_graph<2> g(10, 10);
    
    auto v1 = g.vertex_at({5, 5});
    auto v2 = g.vertex_at({5, 6});  // Adjacent
    auto v3 = g.vertex_at({5, 7});  // Not adjacent
    auto v4 = g.vertex_at({6, 6});  // Diagonal, not adjacent
    
    check(g.has_edge(v1, v2), "Adjacent vertices have edge");
    check(g.has_edge(v2, v1), "Edge is symmetric");
    check(!g.has_edge(v1, v3), "Non-adjacent vertices have no edge");
    check(!g.has_edge(v1, v4), "Diagonal vertices have no edge");
}

// =============================================================================
// Test: Vertex Iteration
// =============================================================================

void test_vertex_iteration() {
    std::cout << "Testing vertex iteration...\n";
    
    bgl::grid_graph<2> g(5, 5);
    
    std::vector<std::size_t> verts;
    for (auto v : bgl::vertices(g)) {
        verts.push_back(v);
    }
    
    check(verts.size() == 25, "vertices() returns 25 vertices");
    check(verts[0] == 0, "First vertex is 0");
    check(verts[24] == 24, "Last vertex is 24");
}

// =============================================================================
// Test: 1D Grid (Path Graph)
// =============================================================================

void test_1d_grid() {
    std::cout << "Testing 1D grid (path graph)...\n";
    
    bgl::grid_graph<1> g(std::array<std::size_t, 1>{10});
    
    check(g.num_vertices() == 10, "1D grid has 10 vertices");
    check(g.num_edges() == 9, "1D grid has 9 edges");
    
    // Endpoints have degree 1
    check(g.degree(0) == 1, "First vertex has degree 1");
    check(g.degree(9) == 1, "Last vertex has degree 1");
    
    // Interior has degree 2
    check(g.degree(5) == 2, "Interior vertex has degree 2");
}

// =============================================================================
// Test: Type Aliases
// =============================================================================

void test_type_aliases() {
    std::cout << "Testing type aliases...\n";
    
    bgl::grid_graph_2d g2d(10, 10);
    check(g2d.num_vertices() == 100, "grid_graph_2d works");
    
    bgl::grid_graph_3d g3d(5, 5, 5);
    check(g3d.num_vertices() == 125, "grid_graph_3d works");
}

} // anonymous namespace

int main() {
    std::cout << "=== BGL Modern: Grid Graph Container Tests ===\n\n";
    
    test_2d_basic_construction();
    test_coordinate_conversion();
    test_3d_grid();
    test_neighbor_access();
    test_degree_calculation();
    test_edge_count();
    test_out_edges();
    test_wrapped_grid();
    test_has_edge();
    test_vertex_iteration();
    test_1d_grid();
    test_type_aliases();
    
    std::cout << "\n=== Results: " << tests_passed << "/" << tests_run << " tests passed ===\n";
    
    return (tests_passed == tests_run) ? 0 : 1;
}
