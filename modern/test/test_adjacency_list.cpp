// BGL Modern - Adjacency List Tests
// Verify the C++20 adjacency_list implementation
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <bgl/modern/adjacency_list.hpp>

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <ranges>

using namespace bgl;

// =============================================================================
// Test Basic Construction
// =============================================================================

void test_default_construction() {
    std::cout << "  Testing default construction... ";
    
    adjacency_list<directed_tag> g;
    assert(num_vertices(g) == 0);
    assert(num_edges(g) == 0);
    
    std::cout << "PASSED\n";
}

void test_sized_construction() {
    std::cout << "  Testing sized construction... ";
    
    adjacency_list<directed_tag> g(5);
    assert(num_vertices(g) == 5);
    assert(num_edges(g) == 0);
    
    // Verify all vertices are accessible
    std::size_t count = 0;
    for ([[maybe_unused]] auto v : vertices(g)) {
        ++count;
    }
    assert(count == 5);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Vertex Operations
// =============================================================================

void test_add_vertex() {
    std::cout << "  Testing add_vertex... ";
    
    adjacency_list<directed_tag> g;
    auto v0 = g.add_vertex();
    auto v1 = g.add_vertex();
    auto v2 = g.add_vertex();
    
    assert(v0 == 0);
    assert(v1 == 1);
    assert(v2 == 2);
    assert(num_vertices(g) == 3);
    
    std::cout << "PASSED\n";
}

void test_vertex_properties() {
    std::cout << "  Testing vertex properties... ";
    
    struct VertexData {
        std::string name;
        int id = 0;
    };
    
    adjacency_list<directed_tag, VertexData> g;
    auto v0 = g.add_vertex({.name = "Alice", .id = 1});
    auto v1 = g.add_vertex({.name = "Bob", .id = 2});
    
    assert(g[v0].name == "Alice");
    assert(g[v0].id == 1);
    assert(g[v1].name == "Bob");
    assert(g[v1].id == 2);
    
    // Modify property
    g[v0].name = "Updated";
    assert(g[v0].name == "Updated");
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Edge Operations
// =============================================================================

void test_add_edge() {
    std::cout << "  Testing add_edge... ";
    
    adjacency_list<directed_tag> g(3);
    
    auto [e01, added1] = g.add_edge(0, 1);
    auto [e12, added2] = g.add_edge(1, 2);
    auto [e02, added3] = g.add_edge(0, 2);
    
    assert(added1 && added2 && added3);
    assert(num_edges(g) == 3);
    
    assert(source(e01, g) == 0);
    assert(target(e01, g) == 1);
    assert(source(e12, g) == 1);
    assert(target(e12, g) == 2);
    
    std::cout << "PASSED\n";
}

void test_edge_properties() {
    std::cout << "  Testing edge properties... ";
    
    struct EdgeData {
        double weight = 0.0;
        std::string label;
    };
    
    adjacency_list<directed_tag, no_property, EdgeData> g(3);
    
    auto [e01, _1] = g.add_edge(0, 1, {.weight = 1.5, .label = "e01"});
    auto [e12, _2] = g.add_edge(1, 2, {.weight = 2.5, .label = "e12"});
    
    assert(g[e01].weight == 1.5);
    assert(g[e01].label == "e01");
    assert(g[e12].weight == 2.5);
    
    // Modify property
    g[e01].weight = 3.0;
    assert(g[e01].weight == 3.0);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Out-Edge Iteration
// =============================================================================

void test_out_edges() {
    std::cout << "  Testing out_edges... ";
    
    adjacency_list<directed_tag> g(4);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(0, 3);
    g.add_edge(1, 2);
    
    // Vertex 0 has 3 out-edges
    assert(out_degree(0, g) == 3);
    
    std::vector<std::size_t> targets;
    for (auto e : out_edges(0, g)) {
        targets.push_back(target(e, g));
    }
    assert(targets.size() == 3);
    assert(targets[0] == 1);
    assert(targets[1] == 2);
    assert(targets[2] == 3);
    
    // Vertex 1 has 1 out-edge
    assert(out_degree(1, g) == 1);
    
    // Vertex 3 has no out-edges
    assert(out_degree(3, g) == 0);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Undirected Graph
// =============================================================================

void test_undirected_graph() {
    std::cout << "  Testing undirected graph... ";
    
    adjacency_list<undirected_tag> g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    
    // In undirected, edges appear in both directions
    assert(out_degree(0, g) == 1);  // 0-1
    assert(out_degree(1, g) == 2);  // 1-0, 1-2
    assert(out_degree(2, g) == 1);  // 2-1
    
    // Verify we can traverse from both ends
    bool found_1_from_0 = false;
    for (auto e : out_edges(0, g)) {
        if (target(e, g) == 1) found_1_from_0 = true;
    }
    assert(found_1_from_0);
    
    bool found_0_from_1 = false;
    for (auto e : out_edges(1, g)) {
        if (target(e, g) == 0) found_0_from_1 = true;
    }
    assert(found_0_from_1);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Bidirectional Graph
// =============================================================================

void test_bidirectional_graph() {
    std::cout << "  Testing bidirectional graph... ";
    
    adjacency_list<bidirectional_tag> g(3);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 2);
    
    // Check out-degrees
    assert(out_degree(0, g) == 2);
    assert(out_degree(1, g) == 1);
    assert(out_degree(2, g) == 0);
    
    // Check in-degrees
    assert(in_degree(0, g) == 0);
    assert(in_degree(1, g) == 1);
    assert(in_degree(2, g) == 2);
    
    // Check in-edges
    std::vector<std::size_t> in_sources;
    for (auto e : in_edges(2, g)) {
        in_sources.push_back(source(e, g));
    }
    assert(in_sources.size() == 2);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Graph with All Properties
// =============================================================================

void test_full_properties() {
    std::cout << "  Testing full properties (vertex, edge, graph)... ";
    
    struct Vertex {
        std::string name;
        double x = 0.0, y = 0.0;
    };
    
    struct Edge {
        double weight = 1.0;
        std::string type;
    };
    
    struct Graph {
        std::string title;
        int version = 1;
    };
    
    adjacency_list<directed_tag, Vertex, Edge, Graph> g;
    
    auto v0 = g.add_vertex({.name = "Node A", .x = 0.0, .y = 0.0});
    auto v1 = g.add_vertex({.name = "Node B", .x = 1.0, .y = 0.0});
    auto [e, _] = g.add_edge(v0, v1, {.weight = 2.5, .type = "highway"});
    
    g.graph_property() = {.title = "Test Graph", .version = 2};
    
    assert(g[v0].name == "Node A");
    assert(g[v1].x == 1.0);
    assert(g[e].weight == 2.5);
    assert(g[e].type == "highway");
    assert(g.graph_property().title == "Test Graph");
    assert(g.graph_property().version == 2);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Range Compatibility
// =============================================================================

void test_range_compatibility() {
    std::cout << "  Testing range compatibility... ";
    
    adjacency_list<directed_tag> g(5);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    g.add_edge(3, 4);
    
    // vertices() is a range
    auto vertex_count = std::ranges::distance(vertices(g));
    assert(vertex_count == 5);
    
    // out_edges() is a range
    auto out_count = std::ranges::distance(out_edges(0, g));
    assert(out_count == 2);
    
    // Can use with algorithms
    auto even_vertices = vertices(g) | std::views::filter([](auto v) { return v % 2 == 0; });
    std::size_t even_count = 0;
    for ([[maybe_unused]] auto v : even_vertices) {
        ++even_count;
    }
    assert(even_count == 3);  // 0, 2, 4
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Concept Satisfaction
// =============================================================================

void test_concepts() {
    std::cout << "  Testing concept satisfaction... ";
    
    // Verify adjacency_list satisfies our concepts
    using G = adjacency_list<directed_tag>;
    
    static_assert(Graph<G>, "adjacency_list must satisfy Graph concept");
    static_assert(VertexListGraph<G>, "adjacency_list must satisfy VertexListGraph");
    static_assert(IncidenceGraph<G>, "adjacency_list must satisfy IncidenceGraph");
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "Running BGL Modern adjacency_list Tests\n";
    std::cout << "========================================\n\n";
    
    std::cout << "Basic Construction Tests:\n";
    test_default_construction();
    test_sized_construction();
    
    std::cout << "\nVertex Operation Tests:\n";
    test_add_vertex();
    test_vertex_properties();
    
    std::cout << "\nEdge Operation Tests:\n";
    test_add_edge();
    test_edge_properties();
    
    std::cout << "\nIteration Tests:\n";
    test_out_edges();
    
    std::cout << "\nDirected Type Tests:\n";
    test_undirected_graph();
    test_bidirectional_graph();
    
    std::cout << "\nProperty Tests:\n";
    test_full_properties();
    
    std::cout << "\nRange Compatibility Tests:\n";
    test_range_compatibility();
    
    std::cout << "\nConcept Tests:\n";
    test_concepts();
    
    std::cout << "\n========================================\n";
    std::cout << "All adjacency_list tests passed!\n";
    
    return 0;
}
