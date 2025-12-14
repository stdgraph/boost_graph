// BGL Modern - Property Map Tests
// Verify C++20 property map concepts work correctly
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <bgl/modern/property_map.hpp>

#include <vector>
#include <array>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <cassert>
#include <cstddef>
#include <limits>

// =============================================================================
// Test Graph Implementation (reused from test_concepts.cpp)
// =============================================================================

namespace test {

struct simple_graph {
    using vertex_descriptor = std::size_t;
    using edge_descriptor = std::pair<vertex_descriptor, vertex_descriptor>;
    using directed_category = bgl::directed_tag;
    using edge_parallel_category = void;
    using traversal_category = void;
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
    
    struct vertex_property {
        double weight = 0.0;
        double distance = std::numeric_limits<double>::infinity();
        int color = 0;
    };
    
    struct edge_property {
        double weight = 1.0;
        double cost = 1.0;
    };
    
    std::vector<std::vector<std::pair<vertex_descriptor, edge_property>>> adj_;
    std::vector<vertex_property> vertex_props_;
    
    simple_graph() = default;
    explicit simple_graph(std::size_t n) : adj_(n), vertex_props_(n) {}
    
    vertex_property& operator[](vertex_descriptor v) { return vertex_props_[v]; }
    const vertex_property& operator[](vertex_descriptor v) const { return vertex_props_[v]; }
    
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
    
    void add_edge(vertex_descriptor u, vertex_descriptor v, double weight = 1.0) {
        adj_[u].push_back({v, edge_property{weight, weight}});
    }
};

inline std::size_t num_vertices(const simple_graph& g) {
    return g.adj_.size();
}

} // namespace test

// =============================================================================
// Static Concept Checks
// =============================================================================

namespace {

// Test lambda-based property map
void test_lambda_property_map_concepts() {
    test::simple_graph g(5);
    
    // Lambda returning value (ReadablePropertyMap)
    auto get_weight = [&g](test::simple_graph::edge_descriptor e) {
        return g[e].weight;
    };
    static_assert(bgl::ReadablePropertyMap<decltype(get_weight), test::simple_graph::edge_descriptor, double>);
    
    // Lambda returning lvalue reference (LvaluePropertyMap)
    auto get_distance = [&g](test::simple_graph::vertex_descriptor v) -> double& {
        return g[v].distance;
    };
    static_assert(bgl::ReadablePropertyMap<decltype(get_distance), test::simple_graph::vertex_descriptor, double>);
    static_assert(bgl::LvaluePropertyMap<decltype(get_distance), test::simple_graph::vertex_descriptor>);
    
    std::cout << "  Lambda property map concepts: PASSED\n";
}

// Test vector property map
void test_vector_property_map_concepts() {
    bgl::vector_property_map<int> pmap(10, 0);
    
    static_assert(bgl::ReadablePropertyMap<decltype(pmap), std::size_t, int>);
    static_assert(bgl::LvaluePropertyMap<decltype(pmap), std::size_t>);
    
    // Test get/put
    bgl::put(pmap, std::size_t{3}, 42);
    assert(bgl::get(pmap, std::size_t{3}) == 42);
    
    std::cout << "  Vector property map concepts: PASSED\n";
}

// Test identity property map
void test_identity_property_map() {
    bgl::identity_property_map id_map;
    
    static_assert(bgl::ReadablePropertyMap<bgl::identity_property_map, std::size_t, std::size_t>);
    static_assert(bgl::ReadablePropertyMap<bgl::identity_property_map, int, int>);
    
    assert(bgl::get(id_map, 42) == 42);
    assert(bgl::get(id_map, std::size_t{100}) == 100);
    
    std::cout << "  Identity property map: PASSED\n";
}

// Test iterator property map
void test_iterator_property_map() {
    std::vector<double> storage{1.0, 2.0, 3.0, 4.0, 5.0};
    bgl::identity_property_map id_map;
    
    auto pmap = bgl::make_iterator_property_map(storage.begin(), id_map);
    
    static_assert(bgl::ReadablePropertyMap<decltype(pmap), std::size_t, double>);
    
    assert(bgl::get(pmap, std::size_t{0}) == 1.0);
    assert(bgl::get(pmap, std::size_t{2}) == 3.0);
    
    // Test writing
    bgl::put(pmap, std::size_t{2}, 30.0);
    assert(storage[2] == 30.0);
    
    std::cout << "  Iterator property map: PASSED\n";
}

// Test default weight accessor
void test_default_weight_accessor() {
    test::simple_graph g(3);
    g.add_edge(0, 1, 2.5);
    g.add_edge(1, 2, 3.5);
    
    auto weight = bgl::default_weight_accessor(g);
    
    static_assert(bgl::ReadablePropertyMap<decltype(weight), test::simple_graph::edge_descriptor, double>);
    
    test::simple_graph::edge_descriptor e01{0, 1};
    assert(weight(e01) == 2.5);
    
    std::cout << "  Default weight accessor: PASSED\n";
}

// Test default distance accessor
void test_default_distance_accessor() {
    test::simple_graph g(3);
    g[0].distance = 0.0;
    g[1].distance = 10.0;
    g[2].distance = 20.0;
    
    auto dist = bgl::default_distance_accessor(g);
    
    static_assert(bgl::ReadablePropertyMap<decltype(dist), test::simple_graph::vertex_descriptor, double>);
    
    assert(dist(0) == 0.0);
    assert(dist(1) == 10.0);
    assert(dist(2) == 20.0);
    
    std::cout << "  Default distance accessor: PASSED\n";
}

// Test mutable distance accessor
void test_mutable_distance_accessor() {
    test::simple_graph g(3);
    
    auto dist = bgl::mutable_distance_accessor(g);
    
    static_assert(bgl::ReadablePropertyMap<decltype(dist), test::simple_graph::vertex_descriptor, double>);
    static_assert(bgl::LvaluePropertyMap<decltype(dist), test::simple_graph::vertex_descriptor>);
    
    // Write via put
    bgl::put(dist, std::size_t{0}, 5.0);
    bgl::put(dist, std::size_t{1}, 15.0);
    
    assert(g[0].distance == 5.0);
    assert(g[1].distance == 15.0);
    
    // Write via direct assignment
    dist(2) = 25.0;
    assert(g[2].distance == 25.0);
    
    std::cout << "  Mutable distance accessor: PASSED\n";
}

// Test vertex property accessor with member pointer
void test_vertex_property_accessor() {
    test::simple_graph g(3);
    g[0].color = 1;
    g[1].color = 2;
    g[2].color = 3;
    
    auto color_map = bgl::vertex_property_accessor(g, &test::simple_graph::vertex_property::color);
    
    static_assert(bgl::ReadablePropertyMap<decltype(color_map), test::simple_graph::vertex_descriptor, int>);
    
    assert(color_map(0) == 1);
    assert(color_map(1) == 2);
    assert(color_map(2) == 3);
    
    std::cout << "  Vertex property accessor: PASSED\n";
}

// Test mutable vertex property accessor
void test_mutable_vertex_property_accessor() {
    test::simple_graph g(3);
    
    auto color_map = bgl::mutable_vertex_property_accessor(g, &test::simple_graph::vertex_property::color);
    
    static_assert(bgl::ReadablePropertyMap<decltype(color_map), test::simple_graph::vertex_descriptor, int>);
    static_assert(bgl::LvaluePropertyMap<decltype(color_map), test::simple_graph::vertex_descriptor>);
    
    color_map(0) = 10;
    color_map(1) = 20;
    bgl::put(color_map, std::size_t{2}, 30);
    
    assert(g[0].color == 10);
    assert(g[1].color == 20);
    assert(g[2].color == 30);
    
    std::cout << "  Mutable vertex property accessor: PASSED\n";
}

// Test edge property accessor
void test_edge_property_accessor() {
    test::simple_graph g(3);
    g.add_edge(0, 1, 1.5);
    g.add_edge(1, 2, 2.5);
    
    auto weight_map = bgl::edge_property_accessor(g, &test::simple_graph::edge_property::weight);
    
    static_assert(bgl::ReadablePropertyMap<decltype(weight_map), test::simple_graph::edge_descriptor, double>);
    
    test::simple_graph::edge_descriptor e01{0, 1};
    test::simple_graph::edge_descriptor e12{1, 2};
    
    assert(weight_map(e01) == 1.5);
    assert(weight_map(e12) == 2.5);
    
    std::cout << "  Edge property accessor: PASSED\n";
}

// Test that property maps work with std::function
void test_std_function_property_map() {
    std::vector<double> weights{1.0, 2.0, 3.0, 4.0, 5.0};
    
    std::function<double(std::size_t)> weight_fn = [&weights](std::size_t i) {
        return weights[i];
    };
    
    static_assert(bgl::ReadablePropertyMap<decltype(weight_fn), std::size_t, double>);
    
    assert(bgl::get(weight_fn, std::size_t{0}) == 1.0);
    assert(bgl::get(weight_fn, std::size_t{4}) == 5.0);
    
    std::cout << "  std::function property map: PASSED\n";
}

// Test property map with unordered_map backend
void test_unordered_map_property_map() {
    std::unordered_map<std::string, int> props;
    props["a"] = 1;
    props["b"] = 2;
    props["c"] = 3;
    
    auto pmap = [&props](const std::string& key) -> int& {
        return props[key];
    };
    
    static_assert(bgl::ReadablePropertyMap<decltype(pmap), std::string, int>);
    static_assert(bgl::LvaluePropertyMap<decltype(pmap), std::string>);
    
    assert(bgl::get(pmap, std::string{"a"}) == 1);
    bgl::put(pmap, std::string{"d"}, 4);
    assert(props["d"] == 4);
    
    std::cout << "  Unordered map property map: PASSED\n";
}

} // anonymous namespace

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "BGL Modern Property Map Tests\n";
    std::cout << "==============================\n\n";
    
    std::cout << "Concept Tests:\n";
    test_lambda_property_map_concepts();
    test_vector_property_map_concepts();
    test_identity_property_map();
    test_iterator_property_map();
    
    std::cout << "\nDefault Accessor Tests:\n";
    test_default_weight_accessor();
    test_default_distance_accessor();
    test_mutable_distance_accessor();
    
    std::cout << "\nMember Pointer Accessor Tests:\n";
    test_vertex_property_accessor();
    test_mutable_vertex_property_accessor();
    test_edge_property_accessor();
    
    std::cout << "\nFlexible Backend Tests:\n";
    test_std_function_property_map();
    test_unordered_map_property_map();
    
    std::cout << "\n==============================\n";
    std::cout << "All property map tests passed!\n";
    
    return 0;
}
