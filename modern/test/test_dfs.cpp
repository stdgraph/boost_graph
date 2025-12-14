// BGL Modern - Depth-First Search Tests
// Verify the C++20 DFS implementation works correctly
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <bgl/modern/depth_first_search.hpp>

#include <iostream>
#include <cassert>
#include <vector>
#include <ranges>
#include <algorithm>

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

struct simple_graph {
    using vertex_descriptor = std::size_t;
    using edge_descriptor = edge_desc;
    using directed_category = bgl::directed_tag;
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
        adj_[u].push_back({v, edge_count_++});
    }
};

inline std::size_t num_vertices(const simple_graph& g) {
    return g.num_vertices_;
}

inline auto vertices(const simple_graph& g) {
    return std::views::iota(std::size_t{0}, g.num_vertices_);
}

struct out_edge_iterator {
    using value_type = edge_desc;
    using difference_type = std::ptrdiff_t;
    using pointer = const edge_desc*;
    using reference = edge_desc;
    using iterator_category = std::forward_iterator_tag;
    
    std::size_t source_{};
    std::vector<simple_graph::stored_edge>::const_iterator it_{};
    
    out_edge_iterator() = default;
    out_edge_iterator(std::size_t src, std::vector<simple_graph::stored_edge>::const_iterator it)
        : source_(src), it_(it) {}
    
    edge_desc operator*() const { return {source_, it_->target, it_->index}; }
    out_edge_iterator& operator++() { ++it_; return *this; }
    out_edge_iterator operator++(int) { auto tmp = *this; ++it_; return tmp; }
    bool operator==(const out_edge_iterator& o) const { return it_ == o.it_; }
    bool operator!=(const out_edge_iterator& o) const { return it_ != o.it_; }
};

struct out_edge_range {
    std::size_t source_;
    const std::vector<simple_graph::stored_edge>* adj_;
    out_edge_iterator begin() const { return {source_, adj_->begin()}; }
    out_edge_iterator end() const { return {source_, adj_->end()}; }
};

inline out_edge_range out_edges(simple_graph::vertex_descriptor v, const simple_graph& g) {
    return {v, &g.adj_[v]};
}

inline std::size_t out_degree(simple_graph::vertex_descriptor v, const simple_graph& g) {
    return g.adj_[v].size();
}

inline simple_graph::vertex_descriptor source(edge_desc e, const simple_graph&) {
    return e.source;
}

inline simple_graph::vertex_descriptor target(edge_desc e, const simple_graph&) {
    return e.target;
}

} // namespace test

template<>
struct bgl::graph_traits<test::simple_graph> {
    using vertex_descriptor = test::simple_graph::vertex_descriptor;
    using edge_descriptor = test::edge_desc;
    using directed_category = bgl::directed_tag;
    using edge_parallel_category = bgl::allow_parallel_edge_tag;
    using traversal_category = void;
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
};

// =============================================================================
// Tests
// =============================================================================

namespace {

void test_simple_dfs() {
    // Linear graph: 0 -> 1 -> 2 -> 3
    test::simple_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    
    auto result = bgl::depth_first_search(g, std::size_t{0});
    
    // All vertices should be discovered
    assert(result.is_discovered(0));
    assert(result.is_discovered(1));
    assert(result.is_discovered(2));
    assert(result.is_discovered(3));
    
    // Discovery order should be 0, 1, 2, 3
    auto& order = result.discovered_vertices();
    assert(order.size() == 4);
    assert(order[0] == 0);
    assert(order[1] == 1);
    assert(order[2] == 2);
    assert(order[3] == 3);
    
    // No cycle in linear graph
    assert(!result.has_cycle());
    
    std::cout << "  Simple DFS: PASSED\n";
}

void test_dfs_times() {
    // Linear graph: 0 -> 1 -> 2
    test::simple_graph g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    
    auto result = bgl::depth_first_search(g, std::size_t{0});
    
    // Discovery times: 0, 1, 2
    // Finish times: 5, 4, 3 (last discovered finishes first)
    auto [d0, f0] = result.times(0);
    auto [d1, f1] = result.times(1);
    auto [d2, f2] = result.times(2);
    
    assert(d0 < d1 && d1 < d2);  // Discovery order
    assert(f2 < f1 && f1 < f0);  // Finish order (reverse)
    
    // Verify parenthesis theorem: for ancestor u and descendant v,
    // d[u] < d[v] < f[v] < f[u]
    assert(d0 < d2 && d2 < f2 && f2 < f0);
    
    std::cout << "  DFS times: PASSED\n";
}

void test_dfs_cycle_detection() {
    // Graph with cycle: 0 -> 1 -> 2 -> 0
    test::simple_graph g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 0);  // Back edge
    
    auto result = bgl::depth_first_search(g, std::size_t{0});
    
    assert(result.has_cycle());
    
    std::cout << "  DFS cycle detection: PASSED\n";
}

void test_dfs_no_cycle() {
    // DAG: 0 -> 1, 0 -> 2, 1 -> 2
    test::simple_graph g(3);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 2);
    
    auto result = bgl::depth_first_search(g, std::size_t{0});
    
    assert(!result.has_cycle());
    
    std::cout << "  DFS no cycle (DAG): PASSED\n";
}

void test_dfs_topological_order() {
    // DAG for topological sort:
    // 0 -> 1 -> 3
    // 0 -> 2 -> 3
    test::simple_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    g.add_edge(2, 3);
    
    auto result = bgl::depth_first_search(g, std::size_t{0});
    
    assert(!result.has_cycle());
    
    auto topo = result.topological_order();
    assert(topo.size() == 4);
    
    // Verify topological order: for each edge u->v, u appears before v
    auto pos = [&topo](std::size_t v) {
        return std::find(topo.begin(), topo.end(), v) - topo.begin();
    };
    
    assert(pos(0) < pos(1));
    assert(pos(0) < pos(2));
    assert(pos(1) < pos(3));
    assert(pos(2) < pos(3));
    
    std::cout << "  DFS topological order: PASSED\n";
}

void test_dfs_ancestors() {
    // Tree: 0 -> 1 -> 2
    test::simple_graph g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    
    auto result = bgl::depth_first_search(g, std::size_t{0});
    
    assert(result.is_ancestor(0, 1));
    assert(result.is_ancestor(0, 2));
    assert(result.is_ancestor(1, 2));
    
    assert(result.is_descendant(2, 0));
    assert(result.is_descendant(2, 1));
    assert(result.is_descendant(1, 0));
    
    assert(!result.is_ancestor(2, 0));
    assert(!result.is_descendant(0, 2));
    
    std::cout << "  DFS ancestors: PASSED\n";
}

void test_dfs_unreachable() {
    test::simple_graph g(4);
    g.add_edge(0, 1);
    // Vertices 2, 3 are unreachable from 0
    
    auto result = bgl::depth_first_search(g, std::size_t{0});
    
    assert(result.is_discovered(0));
    assert(result.is_discovered(1));
    assert(!result.is_discovered(2));
    assert(!result.is_discovered(3));
    
    std::cout << "  DFS unreachable (single source): PASSED\n";
}

void test_dfs_all_vertices() {
    // Disconnected graph
    test::simple_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(2, 3);
    
    // DFS from all vertices (forest)
    auto result = bgl::depth_first_search(g);
    
    // All vertices should be discovered
    assert(result.is_discovered(0));
    assert(result.is_discovered(1));
    assert(result.is_discovered(2));
    assert(result.is_discovered(3));
    
    assert(result.discovered_vertices().size() == 4);
    
    std::cout << "  DFS all vertices (forest): PASSED\n";
}

void test_dfs_path_reconstruction() {
    test::simple_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    
    auto result = bgl::depth_first_search(g, std::size_t{0});
    
    auto path = result.path_to(3);
    assert(path.size() == 4);
    assert(path[0] == 0);
    assert(path[1] == 1);
    assert(path[2] == 2);
    assert(path[3] == 3);
    
    std::cout << "  DFS path reconstruction: PASSED\n";
}

void test_dfs_single_vertex() {
    test::simple_graph g(1);
    
    auto result = bgl::depth_first_search(g, std::size_t{0});
    
    assert(result.is_discovered(0));
    assert(result.is_finished(0));
    assert(!result.has_cycle());
    assert(result.discovered_vertices().size() == 1);
    
    std::cout << "  DFS single vertex: PASSED\n";
}

void test_dfs_self_loop() {
    test::simple_graph g(2);
    g.add_edge(0, 1);
    g.add_edge(1, 1);  // Self-loop
    
    auto result = bgl::depth_first_search(g, std::size_t{0});
    
    // Self-loop is a cycle
    assert(result.has_cycle());
    
    std::cout << "  DFS self-loop: PASSED\n";
}

void test_dfs_colors() {
    test::simple_graph g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    
    auto result = bgl::depth_first_search(g, std::size_t{0});
    
    // All visited vertices should be black after DFS completes
    assert(result.color_of(0) == bgl::vertex_color::black);
    assert(result.color_of(1) == bgl::vertex_color::black);
    assert(result.color_of(2) == bgl::vertex_color::black);
    
    std::cout << "  DFS colors: PASSED\n";
}

} // anonymous namespace

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "BGL Modern Depth-First Search Tests\n";
    std::cout << "====================================\n\n";
    
    std::cout << "Basic Tests:\n";
    test_simple_dfs();
    test_dfs_times();
    test_dfs_single_vertex();
    
    std::cout << "\nCycle Tests:\n";
    test_dfs_cycle_detection();
    test_dfs_no_cycle();
    test_dfs_self_loop();
    
    std::cout << "\nTopological Sort Tests:\n";
    test_dfs_topological_order();
    
    std::cout << "\nTree Structure Tests:\n";
    test_dfs_ancestors();
    test_dfs_path_reconstruction();
    
    std::cout << "\nConnectivity Tests:\n";
    test_dfs_unreachable();
    test_dfs_all_vertices();
    
    std::cout << "\nProperty Tests:\n";
    test_dfs_colors();
    
    std::cout << "\n====================================\n";
    std::cout << "All DFS tests passed!\n";
    
    return 0;
}
