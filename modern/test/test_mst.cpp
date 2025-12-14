// BGL Modern - Minimum Spanning Tree Tests
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <bgl/modern/kruskal_minimum_spanning_tree.hpp>
#include <bgl/modern/prim_minimum_spanning_tree.hpp>

#include <vector>
#include <iostream>
#include <cassert>
#include <cmath>

// Weighted undirected graph for MST testing
namespace test {

struct weighted_graph {
    using vertex_descriptor = std::size_t;
    // Edge descriptor includes source context for correct target() behavior
    struct edge_descriptor {
        std::size_t edge_idx;
        vertex_descriptor from;  // The vertex we're iterating from
    };
    using directed_category = bgl::undirected_tag;
    using edge_parallel_category = bgl::allow_parallel_edge_tag;
    using traversal_category = void;
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
    
    struct edge_data {
        vertex_descriptor u, v;
        double weight;
    };
    
    std::size_t num_verts_ = 0;
    std::vector<edge_data> edges_;
    std::vector<std::vector<std::pair<std::size_t, vertex_descriptor>>> adj_;
    
    weighted_graph() = default;
    explicit weighted_graph(std::size_t n) : num_verts_(n), adj_(n) {}
    
    void add_edge(vertex_descriptor u, vertex_descriptor v, double w) {
        std::size_t e = edges_.size();
        edges_.push_back({u, v, w});
        adj_[u].emplace_back(e, v);
        adj_[v].emplace_back(e, u);  // Undirected
    }
    
    double weight(edge_descriptor e) const { return edges_[e.edge_idx].weight; }
    double weight(std::size_t e) const { return edges_[e].weight; }
};

inline std::size_t num_vertices(const weighted_graph& g) { return g.num_verts_; }
inline std::size_t num_edges(const weighted_graph& g) { return g.edges_.size(); }

inline auto vertices(const weighted_graph& g) {
    return std::ranges::iota_view(std::size_t{0}, g.num_verts_);
}

// Edge range for Kruskal (just edge indices)
struct edge_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = weighted_graph::edge_descriptor;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type;
    
    std::size_t idx = 0;
    
    edge_iterator() = default;
    explicit edge_iterator(std::size_t i) : idx(i) {}
    
    value_type operator*() const { return {idx, 0}; }
    edge_iterator& operator++() { ++idx; return *this; }
    edge_iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }
    bool operator==(const edge_iterator& o) const { return idx == o.idx; }
};

inline auto edges(const weighted_graph& g) {
    return std::ranges::subrange(edge_iterator{0}, edge_iterator{g.edges_.size()});
}

// Out-edge range for Prim
struct out_edge_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = weighted_graph::edge_descriptor;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type;
    
    const weighted_graph* g = nullptr;
    std::size_t u = 0;
    std::size_t idx = 0;
    
    out_edge_iterator() = default;
    out_edge_iterator(const weighted_graph* g_, std::size_t u_, std::size_t idx_)
        : g(g_), u(u_), idx(idx_) {}
    
    value_type operator*() const { 
        return {g->adj_[u][idx].first, u};  // Include source context
    }
    out_edge_iterator& operator++() { ++idx; return *this; }
    out_edge_iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }
    bool operator==(const out_edge_iterator& o) const { return u == o.u && idx == o.idx; }
};

inline auto out_edges(weighted_graph::vertex_descriptor u, const weighted_graph& g) {
    return std::ranges::subrange(
        out_edge_iterator{&g, u, 0},
        out_edge_iterator{&g, u, g.adj_[u].size()}
    );
}

inline std::size_t out_degree(weighted_graph::vertex_descriptor u, const weighted_graph& g) {
    return g.adj_[u].size();
}

inline weighted_graph::vertex_descriptor source(weighted_graph::edge_descriptor e, const weighted_graph& g) {
    // For edges from edges() iterator: from field is unused, use edge_data.u
    // For edges from out_edges() iterator: from field indicates the source
    // We use edge_data.u as the canonical source for consistency
    return g.edges_[e.edge_idx].u;
}

inline weighted_graph::vertex_descriptor target(weighted_graph::edge_descriptor e, const weighted_graph& g) {
    // For edges from edges() iterator: return edge_data.v
    // For edges from out_edges() iterator: return the "other" endpoint
    const auto& ed = g.edges_[e.edge_idx];
    // If from is set and matches one endpoint, return the other
    if (e.from == ed.u) return ed.v;
    if (e.from == ed.v) return ed.u;
    // Otherwise (from=0 from edges() iterator), just return ed.v
    return ed.v;
}

} // namespace test

// =============================================================================
// Kruskal's Algorithm Tests
// =============================================================================

namespace {

void test_kruskal_simple_triangle() {
    // Triangle graph: 0 -- 1 -- 2 -- 0
    // Weights: (0,1)=1, (1,2)=2, (0,2)=3
    // MST should include edges with weight 1 and 2
    test::weighted_graph g(3);
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 2, 2.0);
    g.add_edge(0, 2, 3.0);
    
    auto result = bgl::kruskal_minimum_spanning_tree(g, [&](auto e) {
        return g.weight(e.edge_idx);
    });
    
    std::cout << "    num_edges=" << result.num_edges() 
              << " total_weight=" << result.total_weight() << std::endl;
    
    if (result.num_edges() != 2) {
        std::cerr << "FAIL: expected 2 edges, got " << result.num_edges() << std::endl;
    }
    if (std::abs(result.total_weight() - 3.0) >= 0.001) {
        std::cerr << "FAIL: expected weight 3.0, got " << result.total_weight() << std::endl;
    }
    
    assert(result.num_edges() == 2);
    assert(result.is_spanning_tree());
    assert(std::abs(result.total_weight() - 3.0) < 0.001);  // 1 + 2 = 3
    
    std::cout << "  kruskal_simple_triangle: PASSED\n";
}

void test_kruskal_square() {
    // Square with diagonal:
    //   0 --- 1
    //   |  \  |
    //   3 --- 2
    // Weights: (0,1)=1, (1,2)=4, (2,3)=2, (0,3)=3, (0,2)=5
    // MST: (0,1)=1, (2,3)=2, (0,3)=3 -> total = 6
    test::weighted_graph g(4);
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 2, 4.0);
    g.add_edge(2, 3, 2.0);
    g.add_edge(0, 3, 3.0);
    g.add_edge(0, 2, 5.0);
    
    auto result = bgl::kruskal_minimum_spanning_tree(g, [&](auto e) {
        return g.weight(e.edge_idx);
    });
    
    assert(result.num_edges() == 3);  // V-1 = 4-1 = 3
    assert(result.is_spanning_tree());
    assert(std::abs(result.total_weight() - 6.0) < 0.001);
    
    std::cout << "  kruskal_square: PASSED\n";
}

void test_kruskal_disconnected() {
    // Two disconnected components
    test::weighted_graph g(4);
    g.add_edge(0, 1, 1.0);
    g.add_edge(2, 3, 2.0);
    
    auto result = bgl::kruskal_minimum_spanning_tree(g, [&](auto e) {
        return g.weight(e.edge_idx);
    });
    
    assert(result.num_edges() == 2);
    assert(!result.is_spanning_tree());  // Not connected
    assert(result.num_components() == 2);
    assert(std::abs(result.total_weight() - 3.0) < 0.001);
    
    std::cout << "  kruskal_disconnected: PASSED\n";
}

void test_kruskal_single_vertex() {
    test::weighted_graph g(1);
    
    auto result = bgl::kruskal_minimum_spanning_tree(g, [&](auto e) {
        return g.weight(e.edge_idx);
    });
    
    assert(result.num_edges() == 0);
    assert(result.is_spanning_tree());  // Single vertex is trivially spanning
    assert(result.total_weight() == 0.0);
    
    std::cout << "  kruskal_single_vertex: PASSED\n";
}

} // anonymous namespace

// =============================================================================
// Prim's Algorithm Tests
// =============================================================================

namespace {

void test_prim_simple_triangle() {
    test::weighted_graph g(3);
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 2, 2.0);
    g.add_edge(0, 2, 3.0);
    
    auto result = bgl::prim_minimum_spanning_tree(g, std::size_t{0}, [&](auto e) {
        return g.weight(e.edge_idx);
    });
    
    assert(result.num_edges() == 2);
    assert(result.is_spanning_tree());
    assert(std::abs(result.total_weight() - 3.0) < 0.001);
    
    std::cout << "  prim_simple_triangle: PASSED\n";
}

void test_prim_square() {
    test::weighted_graph g(4);
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 2, 4.0);
    g.add_edge(2, 3, 2.0);
    g.add_edge(0, 3, 3.0);
    g.add_edge(0, 2, 5.0);
    
    auto result = bgl::prim_minimum_spanning_tree(g, std::size_t{0}, [&](auto e) {
        return g.weight(e.edge_idx);
    });
    
    assert(result.num_edges() == 3);
    assert(result.is_spanning_tree());
    assert(std::abs(result.total_weight() - 6.0) < 0.001);
    
    std::cout << "  prim_square: PASSED\n";
}

void test_prim_predecessors() {
    // Verify predecessor tree is correct
    test::weighted_graph g(4);
    g.add_edge(0, 1, 1.0);  // 0-1 is cheapest from 0
    g.add_edge(0, 2, 3.0);
    g.add_edge(1, 2, 1.0);  // 1-2 is cheapest from 1
    g.add_edge(2, 3, 1.0);  // 2-3 is cheapest from 2
    
    auto result = bgl::prim_minimum_spanning_tree(g, std::size_t{0}, [&](auto e) {
        return g.weight(e.edge_idx);
    });
    
    // MST: 0-1 (w=1), 1-2 (w=1), 2-3 (w=1), total=3
    assert(result.num_edges() == 3);
    assert(std::abs(result.total_weight() - 3.0) < 0.001);
    
    // Check predecessor relationships
    assert(result.predecessor_of(0) == 0);  // Source is its own predecessor
    assert(result.predecessor_of(1) == 0);  // 1's parent is 0
    assert(result.predecessor_of(2) == 1);  // 2's parent is 1
    assert(result.predecessor_of(3) == 2);  // 3's parent is 2
    
    std::cout << "  prim_predecessors: PASSED\n";
}

void test_prim_single_vertex() {
    test::weighted_graph g(1);
    
    auto result = bgl::prim_minimum_spanning_tree(g, std::size_t{0}, [&](auto e) {
        return g.weight(e.edge_idx);
    });
    
    assert(result.num_edges() == 0);
    assert(result.is_spanning_tree());
    assert(result.total_weight() == 0.0);
    
    std::cout << "  prim_single_vertex: PASSED\n";
}

} // anonymous namespace

int main() {
    std::cout << "BGL Modern MST Tests\n";
    std::cout << "====================\n\n";
    
    std::cout << "Kruskal's Algorithm:\n";
    test_kruskal_simple_triangle();
    test_kruskal_square();
    test_kruskal_disconnected();
    test_kruskal_single_vertex();
    
    std::cout << "\nPrim's Algorithm:\n";
    test_prim_simple_triangle();
    test_prim_square();
    test_prim_predecessors();
    test_prim_single_vertex();
    
    std::cout << "\n====================\n";
    std::cout << "All MST tests passed!\n";
    
    return 0;
}
