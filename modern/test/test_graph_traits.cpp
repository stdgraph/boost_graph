// BGL Modern - Graph Traits Tests
// Test type traits, helper aliases, and user-defined graph types
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <bgl/modern/graph_traits.hpp>
#include <bgl/modern/concepts.hpp>

#include <vector>
#include <list>
#include <utility>
#include <iostream>
#include <cstddef>

// =============================================================================
// User-Defined Graph Type 1: Simple Directed Graph
// =============================================================================

namespace user {

/// A user-defined directed graph with nested type aliases
struct directed_graph {
    using vertex_descriptor      = std::size_t;
    using edge_descriptor        = std::pair<std::size_t, std::size_t>;
    using directed_category      = bgl::directed_tag;
    using edge_parallel_category = bgl::disallow_parallel_edge_tag;
    using traversal_category     = void;
    using vertices_size_type     = std::size_t;
    using edges_size_type        = std::size_t;
    using degree_size_type       = std::size_t;
    using vertex_property_type   = double;
    using edge_property_type     = int;
    
    std::vector<std::vector<vertex_descriptor>> adj_;
    std::vector<double> vertex_weights_;
    
    explicit directed_graph(std::size_t n) : adj_(n), vertex_weights_(n, 0.0) {}
    
    static vertex_descriptor null_vertex() { return static_cast<vertex_descriptor>(-1); }
};

// Free functions for directed_graph
inline auto vertices(const directed_graph& g) {
    return std::views::iota(std::size_t{0}, g.adj_.size());
}

inline std::size_t num_vertices(const directed_graph& g) {
    return g.adj_.size();
}

inline auto out_edges(directed_graph::vertex_descriptor v, const directed_graph& g) {
    std::vector<directed_graph::edge_descriptor> result;
    for (auto u : g.adj_[v]) {
        result.emplace_back(v, u);
    }
    return result;
}

inline std::size_t out_degree(directed_graph::vertex_descriptor v, const directed_graph& g) {
    return g.adj_[v].size();
}

inline directed_graph::vertex_descriptor source(
    directed_graph::edge_descriptor e, const directed_graph&) {
    return e.first;
}

inline directed_graph::vertex_descriptor target(
    directed_graph::edge_descriptor e, const directed_graph&) {
    return e.second;
}

} // namespace user

// =============================================================================
// User-Defined Graph Type 2: Undirected Graph via Specialization
// =============================================================================

namespace user {

/// A user-defined undirected graph that uses graph_traits specialization
struct undirected_graph {
    std::vector<std::list<std::size_t>> adj_;
    
    explicit undirected_graph(std::size_t n) : adj_(n) {}
    
    void add_edge(std::size_t u, std::size_t v) {
        adj_[u].push_back(v);
        adj_[v].push_back(u);
    }
};

} // namespace user

// Specialize graph_traits for undirected_graph
namespace bgl {

template<>
struct graph_traits<user::undirected_graph> {
    using vertex_descriptor      = std::size_t;
    using edge_descriptor        = std::pair<std::size_t, std::size_t>;
    using directed_category      = undirected_tag;
    using edge_parallel_category = allow_parallel_edge_tag;
    using traversal_category     = void;
    using vertices_size_type     = std::size_t;
    using edges_size_type        = std::size_t;
    using degree_size_type       = std::size_t;
};

} // namespace bgl

namespace user {

// Free functions for undirected_graph
inline auto vertices(const undirected_graph& g) {
    return std::views::iota(std::size_t{0}, g.adj_.size());
}

inline std::size_t num_vertices(const undirected_graph& g) {
    return g.adj_.size();
}

} // namespace user

// =============================================================================
// User-Defined Graph Type 3: Bidirectional Graph
// =============================================================================

namespace user {

/// A bidirectional graph with both in-edges and out-edges
struct bidirectional_graph {
    using vertex_descriptor      = std::size_t;
    using edge_descriptor        = std::pair<std::size_t, std::size_t>;
    using directed_category      = bgl::bidirectional_tag;
    using edge_parallel_category = bgl::disallow_parallel_edge_tag;
    using traversal_category     = void;
    using vertices_size_type     = std::size_t;
    using edges_size_type        = std::size_t;
    using degree_size_type       = std::size_t;
    
    std::vector<std::vector<vertex_descriptor>> out_adj_;
    std::vector<std::vector<vertex_descriptor>> in_adj_;
    
    explicit bidirectional_graph(std::size_t n) : out_adj_(n), in_adj_(n) {}
    
    void add_edge(vertex_descriptor u, vertex_descriptor v) {
        out_adj_[u].push_back(v);
        in_adj_[v].push_back(u);
    }
};

// Free functions for bidirectional_graph
inline auto vertices(const bidirectional_graph& g) {
    return std::views::iota(std::size_t{0}, g.out_adj_.size());
}

inline std::size_t num_vertices(const bidirectional_graph& g) {
    return g.out_adj_.size();
}

inline auto out_edges(bidirectional_graph::vertex_descriptor v, const bidirectional_graph& g) {
    std::vector<bidirectional_graph::edge_descriptor> result;
    for (auto u : g.out_adj_[v]) {
        result.emplace_back(v, u);
    }
    return result;
}

inline std::size_t out_degree(bidirectional_graph::vertex_descriptor v, const bidirectional_graph& g) {
    return g.out_adj_[v].size();
}

inline auto in_edges(bidirectional_graph::vertex_descriptor v, const bidirectional_graph& g) {
    std::vector<bidirectional_graph::edge_descriptor> result;
    for (auto u : g.in_adj_[v]) {
        result.emplace_back(u, v);
    }
    return result;
}

inline std::size_t in_degree(bidirectional_graph::vertex_descriptor v, const bidirectional_graph& g) {
    return g.in_adj_[v].size();
}

inline bidirectional_graph::vertex_descriptor source(
    bidirectional_graph::edge_descriptor e, const bidirectional_graph&) {
    return e.first;
}

inline bidirectional_graph::vertex_descriptor target(
    bidirectional_graph::edge_descriptor e, const bidirectional_graph&) {
    return e.second;
}

} // namespace user

// =============================================================================
// Static Assertions - Type Aliases
// =============================================================================

// Test vertex_descriptor_t
static_assert(std::is_same_v<
    bgl::vertex_descriptor_t<user::directed_graph>,
    std::size_t
>, "vertex_descriptor_t should be std::size_t");

// Test edge_descriptor_t
static_assert(std::is_same_v<
    bgl::edge_descriptor_t<user::directed_graph>,
    std::pair<std::size_t, std::size_t>
>, "edge_descriptor_t should be pair");

// Test directed_category_t
static_assert(std::is_same_v<
    bgl::directed_category_t<user::directed_graph>,
    bgl::directed_tag
>, "directed_category_t should be directed_tag");

// Test undirected graph via specialization
static_assert(std::is_same_v<
    bgl::directed_category_t<user::undirected_graph>,
    bgl::undirected_tag
>, "undirected_graph should have undirected_tag");

// =============================================================================
// Static Assertions - Direction Traits
// =============================================================================

static_assert(bgl::is_directed_v<user::directed_graph>,
    "directed_graph should be directed");

static_assert(!bgl::is_undirected_v<user::directed_graph>,
    "directed_graph should not be undirected");

static_assert(bgl::is_undirected_v<user::undirected_graph>,
    "undirected_graph should be undirected");

static_assert(!bgl::is_directed_v<user::undirected_graph>,
    "undirected_graph should not be directed");

static_assert(bgl::is_bidirectional_v<user::bidirectional_graph>,
    "bidirectional_graph should be bidirectional");

static_assert(bgl::is_directed_v<user::bidirectional_graph>,
    "bidirectional_graph should also be directed (inheritance)");

// =============================================================================
// Static Assertions - Parallel Edge Traits
// =============================================================================

static_assert(!bgl::allows_parallel_edges_v<user::directed_graph>,
    "directed_graph should not allow parallel edges");

static_assert(bgl::allows_parallel_edges_v<user::undirected_graph>,
    "undirected_graph should allow parallel edges");

// =============================================================================
// Static Assertions - Conditional Type Selection
// =============================================================================

// Test directed_select_t
static_assert(std::is_same_v<
    bgl::directed_select_t<user::directed_graph, int, double>,
    int
>, "directed_select_t should select int for directed graph");

static_assert(std::is_same_v<
    bgl::directed_select_t<user::undirected_graph, int, double>,
    double
>, "directed_select_t should select double for undirected graph");

// Test bidirectional_select_t
static_assert(std::is_same_v<
    bgl::bidirectional_select_t<user::bidirectional_graph, int, double>,
    int
>, "bidirectional_select_t should select int for bidirectional graph");

static_assert(std::is_same_v<
    bgl::bidirectional_select_t<user::directed_graph, int, double>,
    double
>, "bidirectional_select_t should select double for non-bidirectional graph");

// =============================================================================
// Static Assertions - Property Detection
// =============================================================================

static_assert(bgl::detail::has_vertex_property<user::directed_graph>,
    "directed_graph should have vertex_property_type");

static_assert(bgl::detail::has_edge_property<user::directed_graph>,
    "directed_graph should have edge_property_type");

static_assert(!bgl::detail::has_vertex_property<user::undirected_graph>,
    "undirected_graph should not have vertex_property_type");

// =============================================================================
// Static Assertions - Descriptor Properties
// =============================================================================

static_assert(bgl::has_integral_vertex_descriptor_v<user::directed_graph>,
    "directed_graph should have integral vertex_descriptor");

static_assert(bgl::has_pair_edge_descriptor_v<user::directed_graph>,
    "directed_graph should have pair edge_descriptor");

// =============================================================================
// Static Assertions - Null Vertex
// =============================================================================

static_assert(bgl::detail::has_null_vertex<user::directed_graph>,
    "directed_graph should have null_vertex()");

static_assert(!bgl::detail::has_null_vertex<user::undirected_graph>,
    "undirected_graph should not have null_vertex()");

// =============================================================================
// Static Assertions - Graph Concepts
// =============================================================================

static_assert(bgl::Graph<user::directed_graph>,
    "directed_graph should satisfy Graph concept");

static_assert(bgl::VertexListGraph<user::directed_graph>,
    "directed_graph should satisfy VertexListGraph concept");

static_assert(bgl::IncidenceGraph<user::directed_graph>,
    "directed_graph should satisfy IncidenceGraph concept");

static_assert(bgl::Graph<user::undirected_graph>,
    "undirected_graph should satisfy Graph concept via specialization");

static_assert(bgl::VertexListGraph<user::undirected_graph>,
    "undirected_graph should satisfy VertexListGraph concept");

static_assert(bgl::BidirectionalGraph<user::bidirectional_graph>,
    "bidirectional_graph should satisfy BidirectionalGraph concept");

// =============================================================================
// Runtime Tests
// =============================================================================

void test_null_vertex() {
    auto null_v = bgl::null_vertex<user::directed_graph>();
    if (null_v != static_cast<std::size_t>(-1)) {
        std::cerr << "null_vertex failed for directed_graph" << std::endl;
        std::exit(1);
    }
    
    // undirected_graph uses fallback (integral type)
    auto null_v2 = bgl::null_vertex<user::undirected_graph>();
    if (null_v2 != static_cast<std::size_t>(-1)) {
        std::cerr << "null_vertex fallback failed for undirected_graph" << std::endl;
        std::exit(1);
    }
}

void test_directed_graph_operations() {
    user::directed_graph g(5);
    
    // Add some edges
    g.adj_[0].push_back(1);
    g.adj_[0].push_back(2);
    g.adj_[1].push_back(2);
    
    if (num_vertices(g) != 5) {
        std::cerr << "num_vertices failed" << std::endl;
        std::exit(1);
    }
    
    if (out_degree(0, g) != 2) {
        std::cerr << "out_degree failed" << std::endl;
        std::exit(1);
    }
}

void test_undirected_graph_operations() {
    user::undirected_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    
    if (num_vertices(g) != 4) {
        std::cerr << "num_vertices failed for undirected_graph" << std::endl;
        std::exit(1);
    }
}

void test_bidirectional_graph_operations() {
    user::bidirectional_graph g(4);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    
    if (out_degree(0, g) != 2) {
        std::cerr << "out_degree failed for bidirectional_graph" << std::endl;
        std::exit(1);
    }
    
    if (in_degree(1, g) != 1) {
        std::cerr << "in_degree failed for bidirectional_graph" << std::endl;
        std::exit(1);
    }
    
    if (in_degree(3, g) != 1) {
        std::cerr << "in_degree failed for vertex 3" << std::endl;
        std::exit(1);
    }
}

void test_conditional_dispatch() {
    // Demonstrate compile-time dispatch based on graph direction
    auto process_graph = []<typename G>(const G& g) {
        if constexpr (bgl::is_bidirectional_v<G>) {
            // Can use in_edges
            for (auto v : vertices(g)) {
                [[maybe_unused]] auto in_deg = in_degree(v, g);
            }
            return "bidirectional";
        } else if constexpr (bgl::is_directed_v<G>) {
            // Only out_edges available
            return "directed";
        } else {
            // Undirected
            return "undirected";
        }
    };
    
    user::directed_graph dg(3);
    user::undirected_graph ug(3);
    user::bidirectional_graph bg(3);
    
    if (std::string(process_graph(dg)) != "directed") {
        std::cerr << "Conditional dispatch failed for directed_graph" << std::endl;
        std::exit(1);
    }
    
    if (std::string(process_graph(ug)) != "undirected") {
        std::cerr << "Conditional dispatch failed for undirected_graph" << std::endl;
        std::exit(1);
    }
    
    if (std::string(process_graph(bg)) != "bidirectional") {
        std::cerr << "Conditional dispatch failed for bidirectional_graph" << std::endl;
        std::exit(1);
    }
}

int main() {
    std::cout << "Testing BGL Modern Graph Traits..." << std::endl;
    std::cout << std::endl;
    
    std::cout << "Static assertions passed:" << std::endl;
    std::cout << "  [PASS] Type aliases (vertex_descriptor_t, edge_descriptor_t, etc.)" << std::endl;
    std::cout << "  [PASS] Direction traits (is_directed_v, is_undirected_v, is_bidirectional_v)" << std::endl;
    std::cout << "  [PASS] Parallel edge traits (allows_parallel_edges_v)" << std::endl;
    std::cout << "  [PASS] Conditional type selection (directed_select_t, bidirectional_select_t)" << std::endl;
    std::cout << "  [PASS] Property detection (has_vertex_property, has_edge_property)" << std::endl;
    std::cout << "  [PASS] Descriptor properties (has_integral_vertex_descriptor_v)" << std::endl;
    std::cout << "  [PASS] graph_traits specialization for user::undirected_graph" << std::endl;
    std::cout << "  [PASS] Graph concepts satisfied by user-defined types" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Runtime tests:" << std::endl;
    
    test_null_vertex();
    std::cout << "  [PASS] null_vertex()" << std::endl;
    
    test_directed_graph_operations();
    std::cout << "  [PASS] Directed graph operations" << std::endl;
    
    test_undirected_graph_operations();
    std::cout << "  [PASS] Undirected graph operations (via specialization)" << std::endl;
    
    test_bidirectional_graph_operations();
    std::cout << "  [PASS] Bidirectional graph operations" << std::endl;
    
    test_conditional_dispatch();
    std::cout << "  [PASS] Conditional dispatch (if constexpr)" << std::endl;
    
    std::cout << std::endl;
    std::cout << "All graph traits tests passed!" << std::endl;
    
    return 0;
}
