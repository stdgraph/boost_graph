// BGL Modern - Algorithm Parameters Tests
// Verify named parameter structs work with designated initializers
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <bgl/modern/dijkstra_shortest_paths.hpp>
#include <bgl/modern/bellman_ford_shortest_paths.hpp>
#include <bgl/modern/breadth_first_search.hpp>
#include <bgl/modern/depth_first_search.hpp>
#include <bgl/modern/algorithm_params.hpp>

#include <vector>
#include <iostream>
#include <cassert>
#include <cmath>
#include <iterator>

// =============================================================================
// Test Graph Implementation
// =============================================================================

namespace test {

struct simple_graph {
    using vertex_descriptor = std::size_t;
    using edge_descriptor = std::pair<vertex_descriptor, vertex_descriptor>;
    using directed_category = bgl::directed_tag;
    using edge_parallel_category = bgl::allow_parallel_edge_tag;
    using traversal_category = void;
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
    
    struct edge_property {
        double weight = 1.0;
        int cost = 1;
    };
    
    std::vector<std::vector<std::pair<vertex_descriptor, edge_property>>> adj_;
    
    simple_graph() = default;
    explicit simple_graph(std::size_t n) : adj_(n) {}
    
    void add_edge(vertex_descriptor u, vertex_descriptor v, double weight = 1.0, int cost = 1) {
        adj_[u].push_back({v, edge_property{weight, cost}});
    }
    
    edge_property& operator[](edge_descriptor e) {
        for (auto& [target, prop] : adj_[e.first]) {
            if (target == e.second) return prop;
        }
        static edge_property dummy;
        return dummy;
    }
    
    const edge_property& operator[](edge_descriptor e) const {
        for (const auto& [target, prop] : adj_[e.first]) {
            if (target == e.second) return prop;
        }
        static const edge_property dummy;
        return dummy;
    }
};

// Graph free functions
inline std::size_t num_vertices(const simple_graph& g) { return g.adj_.size(); }
inline std::size_t num_edges(const simple_graph& g) {
    std::size_t count = 0;
    for (const auto& adj : g.adj_) count += adj.size();
    return count;
}

struct edge_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = simple_graph::edge_descriptor;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type;
    
    const simple_graph* g = nullptr;
    std::size_t u = 0;
    std::size_t idx = 0;
    
    edge_iterator() = default;
    edge_iterator(const simple_graph* g_, std::size_t u_, std::size_t idx_) : g(g_), u(u_), idx(idx_) {}
    
    simple_graph::edge_descriptor operator*() const {
        return {u, g->adj_[u][idx].first};
    }
    
    edge_iterator& operator++() {
        ++idx;
        while (u < g->adj_.size() && idx >= g->adj_[u].size()) {
            ++u;
            idx = 0;
        }
        return *this;
    }
    
    edge_iterator operator++(int) {
        edge_iterator tmp = *this;
        ++(*this);
        return tmp;
    }
    
    bool operator==(const edge_iterator& other) const {
        return u == other.u && idx == other.idx;
    }
};

inline auto edges(const simple_graph& g) {
    edge_iterator begin{&g, 0, 0};
    while (begin.u < g.adj_.size() && g.adj_[begin.u].empty()) ++begin.u;
    edge_iterator end{&g, g.adj_.size(), 0};
    return std::ranges::subrange(begin, end);
}

inline auto vertices(const simple_graph& g) {
    return std::ranges::iota_view(std::size_t{0}, g.adj_.size());
}

struct out_edge_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = simple_graph::edge_descriptor;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type;
    
    const simple_graph* g = nullptr;
    std::size_t u = 0;
    std::size_t idx = 0;
    
    out_edge_iterator() = default;
    out_edge_iterator(const simple_graph* g_, std::size_t u_, std::size_t idx_) : g(g_), u(u_), idx(idx_) {}
    
    simple_graph::edge_descriptor operator*() const {
        return {u, g->adj_[u][idx].first};
    }
    
    out_edge_iterator& operator++() { ++idx; return *this; }
    
    out_edge_iterator operator++(int) {
        out_edge_iterator tmp = *this;
        ++(*this);
        return tmp;
    }
    
    bool operator==(const out_edge_iterator& other) const {
        return u == other.u && idx == other.idx;
    }
};

inline auto out_edges(simple_graph::vertex_descriptor u, const simple_graph& g) {
    return std::ranges::subrange(
        out_edge_iterator{&g, u, 0},
        out_edge_iterator{&g, u, g.adj_[u].size()}
    );
}

inline std::size_t out_degree(simple_graph::vertex_descriptor u, const simple_graph& g) {
    return g.adj_[u].size();
}

inline simple_graph::vertex_descriptor source(simple_graph::edge_descriptor e, const simple_graph&) {
    return e.first;
}

inline simple_graph::vertex_descriptor target(simple_graph::edge_descriptor e, const simple_graph&) {
    return e.second;
}

} // namespace test

// =============================================================================
// Tests
// =============================================================================

namespace {

void test_dijkstra_params_default() {
    test::simple_graph g(4);
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 2, 2.0);
    g.add_edge(2, 3, 1.0);
    g.add_edge(0, 3, 10.0);
    
    // Test with empty params (all defaults)
    auto result = bgl::dijkstra_shortest_paths(g, std::size_t{0}, bgl::dijkstra_params{});
    
    // With default unit weights, shortest path to 3 is directly 0->3 = 1 edge = 1.0
    // (shorter than 0->1->2->3 = 3 edges = 3.0)
    assert(result.distance_to(0) == 0.0);
    assert(result.distance_to(1) == 1.0);
    assert(result.distance_to(2) == 2.0);
    assert(result.distance_to(3) == 1.0);  // Direct path is shortest with unit weights
    
    std::cout << "  dijkstra_params default: PASSED\n";
}

void test_dijkstra_params_custom_weight() {
    test::simple_graph g(4);
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 2, 2.0);
    g.add_edge(2, 3, 1.0);
    g.add_edge(0, 3, 10.0);
    
    // Test with custom weight map
    auto result = bgl::dijkstra_shortest_paths(g, std::size_t{0}, bgl::dijkstra_params{
        .weight_map = [&g](auto e) { return g[e].weight; }
    });
    
    // With actual weights: 0->1->2->3 = 1+2+1 = 4, 0->3 = 10
    assert(result.distance_to(3) == 4.0);
    
    std::cout << "  dijkstra_params custom weight: PASSED\n";
}

void test_dijkstra_params_integer_cost() {
    test::simple_graph g(3);
    g.add_edge(0, 1, 1.0, 5);
    g.add_edge(1, 2, 1.0, 3);
    g.add_edge(0, 2, 1.0, 10);
    
    // Test with integer cost accessor
    auto result = bgl::dijkstra_shortest_paths(g, std::size_t{0}, bgl::dijkstra_params{
        .weight_map = [&g](auto e) { return g[e].cost; }
    });
    
    // Path 0->1->2 costs 5+3=8, path 0->2 costs 10
    assert(result.distance_to(2) == 8.0);
    
    std::cout << "  dijkstra_params integer cost: PASSED\n";
}

void test_bellman_ford_params_default() {
    test::simple_graph g(3);
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 2, 1.0);
    
    // Test with empty params
    auto result = bgl::bellman_ford_shortest_paths(g, std::size_t{0}, bgl::bellman_ford_params{});
    
    assert(!result.has_negative_cycle());
    assert(result.distance_to(2) == 2.0);  // Unit weights: 2 edges
    
    std::cout << "  bellman_ford_params default: PASSED\n";
}

void test_bellman_ford_params_custom_weight() {
    test::simple_graph g(4);
    g.add_edge(0, 1, 2.0);
    g.add_edge(1, 2, -1.0);  // Negative weight
    g.add_edge(2, 3, 1.0);
    g.add_edge(0, 3, 5.0);
    
    auto result = bgl::bellman_ford_shortest_paths(g, std::size_t{0}, bgl::bellman_ford_params{
        .weight_map = [&g](auto e) { return g[e].weight; }
    });
    
    // 0->1->2->3 = 2+(-1)+1 = 2, 0->3 = 5
    assert(!result.has_negative_cycle());
    assert(result.distance_to(3) == 2.0);
    
    std::cout << "  bellman_ford_params custom weight: PASSED\n";
}

void test_bfs_params_default() {
    test::simple_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    g.add_edge(2, 3);
    
    // Test with empty params
    auto result = bgl::breadth_first_search(g, std::size_t{0}, bgl::bfs_params{});
    
    assert(result.distance_to(0) == 0);
    assert(result.distance_to(1) == 1);
    assert(result.distance_to(2) == 1);
    assert(result.distance_to(3) == 2);
    
    std::cout << "  bfs_params default: PASSED\n";
}

void test_dfs_params_default() {
    test::simple_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    
    // Test with empty params (single source)
    auto result = bgl::depth_first_search(g, std::size_t{0}, bgl::dfs_params{});
    
    assert(result.is_discovered(0));
    assert(result.is_discovered(1));
    assert(result.is_discovered(2));
    assert(result.is_discovered(3));
    
    std::cout << "  dfs_params default (single source): PASSED\n";
}

void test_dfs_params_all_vertices() {
    test::simple_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(2, 3);  // Disconnected component
    
    // Test with empty params (all vertices)
    auto result = bgl::depth_first_search(g, bgl::dfs_params{});
    
    assert(result.is_discovered(0));
    assert(result.is_discovered(1));
    assert(result.is_discovered(2));
    assert(result.is_discovered(3));
    
    std::cout << "  dfs_params default (all vertices): PASSED\n";
}

void test_params_type_deduction() {
    // Verify that template parameter deduction works correctly
    
    // Lambda weight map
    auto lambda_weight = [](auto e) { return 1.0; };
    bgl::dijkstra_params params1{.weight_map = lambda_weight};
    static_assert(!std::same_as<decltype(params1.weight_map), bgl::default_weight_map>);
    
    // Default weight map
    bgl::dijkstra_params params2{};
    static_assert(std::same_as<decltype(params2.weight_map), bgl::default_weight_map>);
    
    std::cout << "  params type deduction: PASSED\n";
}

void test_use_default_sentinel() {
    // Test the use_default_t sentinel
    static_assert(bgl::IsDefault<bgl::use_default_t>);
    static_assert(bgl::IsDefault<const bgl::use_default_t&>);
    static_assert(!bgl::IsDefault<int>);
    static_assert(!bgl::IsDefault<bgl::default_weight_map>);
    
    std::cout << "  use_default sentinel: PASSED\n";
}

} // anonymous namespace

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "BGL Modern Algorithm Parameters Tests\n";
    std::cout << "======================================\n\n";
    
    std::cout << "Dijkstra Named Parameters:\n";
    test_dijkstra_params_default();
    test_dijkstra_params_custom_weight();
    test_dijkstra_params_integer_cost();
    
    std::cout << "\nBellman-Ford Named Parameters:\n";
    test_bellman_ford_params_default();
    test_bellman_ford_params_custom_weight();
    
    std::cout << "\nBFS Named Parameters:\n";
    test_bfs_params_default();
    
    std::cout << "\nDFS Named Parameters:\n";
    test_dfs_params_default();
    test_dfs_params_all_vertices();
    
    std::cout << "\nType System Tests:\n";
    test_params_type_deduction();
    test_use_default_sentinel();
    
    std::cout << "\n======================================\n";
    std::cout << "All algorithm parameters tests passed!\n";
    
    return 0;
}
