// BGL Modern - Connectivity Algorithms Test Suite
// Tests for connected_components, strong_components, topological_sort
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <bgl/modern/connected_components.hpp>
#include <bgl/modern/strong_components.hpp>
#include <bgl/modern/topological_sort.hpp>

#include <cassert>
#include <iostream>
#include <algorithm>
#include <ranges>
#include <vector>

using namespace bgl;

// =============================================================================
// Test Graph Implementation
// =============================================================================

namespace test {

struct edge_desc {
    std::size_t source;
    std::size_t target;
    std::size_t index;
    bool operator==(const edge_desc&) const = default;
};

template<typename DirectedTag>
struct simple_graph {
    using vertex_descriptor = std::size_t;
    using edge_descriptor = edge_desc;
    using directed_category = DirectedTag;
    using edge_parallel_category = bgl::allow_parallel_edge_tag;
    using traversal_category = void;
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
    
    struct stored_edge {
        vertex_descriptor target;
        std::size_t index;
    };
    
    std::size_t num_vertices_ = 0;
    std::vector<std::vector<stored_edge>> adj_;
    std::size_t edge_count_ = 0;
    
    simple_graph() = default;
    explicit simple_graph(std::size_t n) : num_vertices_(n), adj_(n) {}
    
    void add_edge(vertex_descriptor u, vertex_descriptor v) {
        adj_[u].push_back({v, edge_count_});
        // For undirected graphs, add the reverse edge too
        if constexpr (std::same_as<DirectedTag, bgl::undirected_tag>) {
            adj_[v].push_back({u, edge_count_});
        }
        ++edge_count_;
    }
};

using directed_graph = simple_graph<bgl::directed_tag>;
using undirected_graph = simple_graph<bgl::undirected_tag>;

template<typename DirectedTag>
inline std::size_t num_vertices(const simple_graph<DirectedTag>& g) {
    return g.num_vertices_;
}

template<typename DirectedTag>
inline auto vertices(const simple_graph<DirectedTag>& g) {
    return std::views::iota(std::size_t{0}, g.num_vertices_);
}

template<typename DirectedTag>
struct out_edge_iterator {
    using value_type = edge_desc;
    using difference_type = std::ptrdiff_t;
    using pointer = const edge_desc*;
    using reference = edge_desc;
    using iterator_category = std::forward_iterator_tag;
    using stored_edge = typename simple_graph<DirectedTag>::stored_edge;
    
    std::size_t source_{};
    typename std::vector<stored_edge>::const_iterator it_{};
    
    out_edge_iterator() = default;
    out_edge_iterator(std::size_t src, typename std::vector<stored_edge>::const_iterator it)
        : source_(src), it_(it) {}
    
    edge_desc operator*() const { return {source_, it_->target, it_->index}; }
    out_edge_iterator& operator++() { ++it_; return *this; }
    out_edge_iterator operator++(int) { auto tmp = *this; ++it_; return tmp; }
    bool operator==(const out_edge_iterator& o) const { return it_ == o.it_; }
};

template<typename DirectedTag>
inline auto out_edges(std::size_t u, const simple_graph<DirectedTag>& g) {
    return std::ranges::subrange(
        out_edge_iterator<DirectedTag>(u, g.adj_[u].begin()),
        out_edge_iterator<DirectedTag>(u, g.adj_[u].end())
    );
}

template<typename DirectedTag>
inline std::size_t out_degree(std::size_t u, const simple_graph<DirectedTag>& g) {
    return g.adj_[u].size();
}

template<typename DirectedTag>
inline std::size_t source(edge_desc e, const simple_graph<DirectedTag>&) {
    return e.source;
}

template<typename DirectedTag>
inline std::size_t target(edge_desc e, const simple_graph<DirectedTag>&) {
    return e.target;
}

} // namespace test

// =============================================================================
// Test connected_components
// =============================================================================

void test_connected_components_single_component() {
    std::cout << "  Testing connected_components single component... ";
    
    // Create a connected undirected graph:
    // 0 -- 1 -- 2
    // |         |
    // 3 ------- 4
    test::undirected_graph g(5);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(0, 3);
    g.add_edge(2, 4);
    g.add_edge(3, 4);
    
    auto result = connected_components(g);
    
    assert(result.num_components() == 1);
    assert(result.is_connected());
    
    // All vertices should be in the same component
    for (std::size_t i = 1; i < 5; ++i) {
        assert(result.same_component(0, i));
    }
    
    std::cout << "PASSED\n";
}

void test_connected_components_multiple_components() {
    std::cout << "  Testing connected_components multiple components... ";
    
    // Create a disconnected graph with 3 components:
    // Component 0: 0 -- 1
    // Component 1: 2 -- 3 -- 4
    // Component 2: 5 (isolated)
    test::undirected_graph g(6);
    g.add_edge(0, 1);
    g.add_edge(2, 3);
    g.add_edge(3, 4);
    
    auto result = connected_components(g);
    
    assert(result.num_components() == 3);
    assert(!result.is_connected());
    
    // Check same-component relationships
    assert(result.same_component(0, 1));
    assert(result.same_component(2, 3));
    assert(result.same_component(2, 4));
    assert(result.same_component(3, 4));
    
    // Check different components
    assert(!result.same_component(0, 2));
    assert(!result.same_component(1, 4));
    assert(!result.same_component(0, 5));
    assert(!result.same_component(2, 5));
    
    std::cout << "PASSED\n";
}

void test_connected_components_empty_graph() {
    std::cout << "  Testing connected_components empty graph... ";
    
    test::undirected_graph g(0);
    auto result = connected_components(g);
    
    assert(result.num_components() == 0);
    
    std::cout << "PASSED\n";
}

void test_connected_components_isolated_vertices() {
    std::cout << "  Testing connected_components isolated vertices... ";
    
    // All isolated vertices
    test::undirected_graph g(4);
    auto result = connected_components(g);
    
    assert(result.num_components() == 4);
    assert(!result.is_connected());
    
    // Each vertex is its own component
    for (std::size_t i = 0; i < 4; ++i) {
        for (std::size_t j = i + 1; j < 4; ++j) {
            assert(!result.same_component(i, j));
        }
    }
    
    std::cout << "PASSED\n";
}

void test_connected_components_vertices_in_component() {
    std::cout << "  Testing connected_components vertices_in_component... ";
    
    // Two components: {0, 1, 2} and {3, 4}
    test::undirected_graph g(5);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(3, 4);
    
    auto result = connected_components(g);
    
    assert(result.num_components() == 2);
    
    // Get component ID of vertex 0
    auto comp0 = result.component_of(0);
    auto verts0 = result.vertices_in_component(comp0);
    assert(verts0.size() == 3);
    
    // Get component ID of vertex 3
    auto comp3 = result.component_of(3);
    auto verts3 = result.vertices_in_component(comp3);
    assert(verts3.size() == 2);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test strong_components
// =============================================================================

void test_strong_components_single_scc() {
    std::cout << "  Testing strong_components single SCC... ";
    
    // Create a strongly connected directed graph (cycle):
    // 0 -> 1 -> 2 -> 0
    test::directed_graph g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 0);
    
    auto result = strong_components(g);
    
    assert(result.num_components() == 1);
    assert(result.is_strongly_connected());
    
    std::cout << "PASSED\n";
}

void test_strong_components_multiple_scc() {
    std::cout << "  Testing strong_components multiple SCCs... ";
    
    // Classic SCC example:
    // 0 -> 1, 1 -> 2, 2 -> 0  (SCC: {0, 1, 2})
    // 2 -> 3, 3 -> 4, 4 -> 3  (SCC: {3, 4})
    test::directed_graph g(5);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 0);
    g.add_edge(2, 3);
    g.add_edge(3, 4);
    g.add_edge(4, 3);
    
    auto result = strong_components(g);
    
    assert(result.num_components() == 2);
    assert(!result.is_strongly_connected());
    
    // Check same-component relationships
    assert(result.same_component(0, 1));
    assert(result.same_component(1, 2));
    assert(result.same_component(0, 2));
    assert(result.same_component(3, 4));
    
    // Different components
    assert(!result.same_component(0, 3));
    assert(!result.same_component(2, 4));
    
    std::cout << "PASSED\n";
}

void test_strong_components_dag() {
    std::cout << "  Testing strong_components DAG... ";
    
    // DAG: each vertex is its own SCC
    // 0 -> 1 -> 2 -> 3
    test::directed_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    
    auto result = strong_components(g);
    
    assert(result.num_components() == 4);
    
    // Each vertex is its own component
    for (std::size_t i = 0; i < 4; ++i) {
        for (std::size_t j = i + 1; j < 4; ++j) {
            assert(!result.same_component(i, j));
        }
    }
    
    std::cout << "PASSED\n";
}

void test_strong_components_isolated() {
    std::cout << "  Testing strong_components isolated vertices... ";
    
    test::directed_graph g(3);
    // No edges - 3 isolated vertices
    
    auto result = strong_components(g);
    
    assert(result.num_components() == 3);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test topological_sort
// =============================================================================

void test_topological_sort_linear() {
    std::cout << "  Testing topological_sort linear DAG... ";
    
    // Linear DAG: 0 -> 1 -> 2 -> 3
    test::directed_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    
    auto result = topological_sort(g);
    
    assert(!result.has_cycle());
    assert(result.is_valid());
    assert(result.size() == 4);
    
    // Find positions in sorted order
    auto find_pos = [&](std::size_t v) -> std::size_t {
        for (std::size_t i = 0; i < result.size(); ++i) {
            if (result[i] == v) return i;
        }
        return result.size();
    };
    
    // Verify topological property: u before v if edge u -> v exists
    assert(find_pos(0) < find_pos(1));
    assert(find_pos(1) < find_pos(2));
    assert(find_pos(2) < find_pos(3));
    
    std::cout << "PASSED\n";
}

void test_topological_sort_diamond() {
    std::cout << "  Testing topological_sort diamond DAG... ";
    
    // Diamond DAG:
    //     1
    //    / \
    //   0   3
    //    \ /
    //     2
    test::directed_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    g.add_edge(2, 3);
    
    auto result = topological_sort(g);
    
    assert(!result.has_cycle());
    assert(result.size() == 4);
    
    auto find_pos = [&](std::size_t v) -> std::size_t {
        for (std::size_t i = 0; i < result.size(); ++i) {
            if (result[i] == v) return i;
        }
        return result.size();
    };
    
    // 0 must come before 1 and 2
    assert(find_pos(0) < find_pos(1));
    assert(find_pos(0) < find_pos(2));
    
    // 1 and 2 must come before 3
    assert(find_pos(1) < find_pos(3));
    assert(find_pos(2) < find_pos(3));
    
    std::cout << "PASSED\n";
}

void test_topological_sort_cycle() {
    std::cout << "  Testing topological_sort with cycle... ";
    
    // Graph with cycle: 0 -> 1 -> 2 -> 0
    test::directed_graph g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 0);
    
    auto result = topological_sort(g);
    
    assert(result.has_cycle());
    assert(!result.is_valid());
    
    std::cout << "PASSED\n";
}

void test_topological_sort_checked() {
    std::cout << "  Testing topological_sort_checked exception... ";
    
    // Graph with cycle
    test::directed_graph g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 0);
    
    bool caught = false;
    try {
        auto result = topological_sort_checked(g);
        (void)result;  // Suppress warning
    } catch (const not_a_dag& e) {
        caught = true;
    }
    
    assert(caught);
    
    std::cout << "PASSED\n";
}

void test_topological_sort_disconnected() {
    std::cout << "  Testing topological_sort disconnected DAG... ";
    
    // Two disconnected chains: 0 -> 1 and 2 -> 3
    test::directed_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(2, 3);
    
    auto result = topological_sort(g);
    
    assert(!result.has_cycle());
    assert(result.size() == 4);
    
    auto find_pos = [&](std::size_t v) -> std::size_t {
        for (std::size_t i = 0; i < result.size(); ++i) {
            if (result[i] == v) return i;
        }
        return result.size();
    };
    
    assert(find_pos(0) < find_pos(1));
    assert(find_pos(2) < find_pos(3));
    
    std::cout << "PASSED\n";
}

void test_topological_sort_empty() {
    std::cout << "  Testing topological_sort empty graph... ";
    
    test::directed_graph g(0);
    auto result = topological_sort(g);
    
    assert(!result.has_cycle());
    assert(result.size() == 0);
    
    std::cout << "PASSED\n";
}

void test_topological_sort_iterator() {
    std::cout << "  Testing topological_sort range iteration... ";
    
    test::directed_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    
    auto result = topological_sort(g);
    
    // Test range-based for loop
    std::size_t count = 0;
    for ([[maybe_unused]] auto v : result) {
        ++count;
    }
    assert(count == 4);
    
    // Test sorted_range()
    auto range = result.sorted_range();
    assert(std::ranges::distance(range) == 4);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "Running BGL Modern Connectivity Algorithm Tests\n";
    std::cout << "================================================\n\n";
    
    std::cout << "Connected Components Tests:\n";
    test_connected_components_single_component();
    test_connected_components_multiple_components();
    test_connected_components_empty_graph();
    test_connected_components_isolated_vertices();
    test_connected_components_vertices_in_component();
    
    std::cout << "\nStrong Components Tests:\n";
    test_strong_components_single_scc();
    test_strong_components_multiple_scc();
    test_strong_components_dag();
    test_strong_components_isolated();
    
    std::cout << "\nTopological Sort Tests:\n";
    test_topological_sort_linear();
    test_topological_sort_diamond();
    test_topological_sort_cycle();
    test_topological_sort_checked();
    test_topological_sort_disconnected();
    test_topological_sort_empty();
    test_topological_sort_iterator();
    
    std::cout << "\n================================================\n";
    std::cout << "All connectivity algorithm tests passed!\n";
    
    return 0;
}
