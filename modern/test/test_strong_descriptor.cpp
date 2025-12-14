// BGL Modern - Strong Descriptor Tests
// Verify type-safe vertex/edge descriptors work correctly
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <bgl/modern/strong_descriptor.hpp>

#include <cassert>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <cstdint>

using namespace bgl;

// =============================================================================
// Test Construction
// =============================================================================

void test_default_construction() {
    std::cout << "  Testing default construction... ";
    
    strong_vertex_descriptor v;
    assert(v.null());
    assert(!v.valid());
    assert(v.value() == strong_vertex_descriptor::null_value);
    
    std::cout << "PASSED\n";
}

void test_value_construction() {
    std::cout << "  Testing value construction... ";
    
    strong_vertex_descriptor v{42};
    assert(v.valid());
    assert(!v.null());
    assert(v.value() == 42);
    
    // Implicit conversion to index
    std::size_t idx = v;
    assert(idx == 42);
    
    std::cout << "PASSED\n";
}

void test_null_descriptor() {
    std::cout << "  Testing null_descriptor... ";
    
    auto v = strong_vertex_descriptor::null_descriptor();
    assert(v.null());
    assert(!v.valid());
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Comparison Operators
// =============================================================================

void test_equality() {
    std::cout << "  Testing equality... ";
    
    strong_vertex_descriptor v1{5};
    strong_vertex_descriptor v2{5};
    strong_vertex_descriptor v3{10};
    
    assert(v1 == v2);
    assert(!(v1 == v3));
    assert(v1 != v3);
    
    std::cout << "PASSED\n";
}

void test_ordering() {
    std::cout << "  Testing ordering (operator<=>)... ";
    
    strong_vertex_descriptor v1{5};
    strong_vertex_descriptor v2{10};
    strong_vertex_descriptor v3{5};
    
    assert(v1 < v2);
    assert(v2 > v1);
    assert(v1 <= v2);
    assert(v1 <= v3);
    assert(v2 >= v1);
    assert(v1 >= v3);
    
    // Spaceship operator
    assert((v1 <=> v2) < 0);
    assert((v2 <=> v1) > 0);
    assert((v1 <=> v3) == 0);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Type Safety
// =============================================================================

void test_type_safety_compile_time() {
    std::cout << "  Testing type safety (compile-time)... ";
    
    // These should compile:
    strong_vertex_descriptor v1{1};
    strong_vertex_descriptor v2{2};
    [[maybe_unused]] bool same = (v1 == v2);
    
    strong_edge_index e1{1};
    strong_edge_index e2{2};
    [[maybe_unused]] bool same_e = (e1 == e2);
    
    // The following would NOT compile (different types):
    // bool mixed = (v1 == e1);  // Error: no operator== for different types
    // bool mixed2 = (v1 < e1);  // Error: no operator< for different types
    
    // This is the key type safety feature!
    static_assert(!std::is_same_v<strong_vertex_descriptor, strong_edge_index>,
                  "Vertex and edge descriptors must be different types");
    
    std::cout << "PASSED\n";
}

void test_type_traits() {
    std::cout << "  Testing type traits... ";
    
    // is_descriptor
    static_assert(is_descriptor_v<strong_vertex_descriptor>);
    static_assert(is_descriptor_v<strong_edge_index>);
    static_assert(!is_descriptor_v<int>);
    static_assert(!is_descriptor_v<std::size_t>);
    
    // is_vertex_descriptor
    static_assert(is_vertex_descriptor_v<strong_vertex_descriptor>);
    static_assert(!is_vertex_descriptor_v<strong_edge_index>);
    static_assert(!is_vertex_descriptor_v<int>);
    
    // is_edge_descriptor
    static_assert(is_edge_descriptor_v<strong_edge_index>);
    static_assert(!is_edge_descriptor_v<strong_vertex_descriptor>);
    static_assert(!is_edge_descriptor_v<int>);
    
    std::cout << "PASSED\n";
}

void test_concepts() {
    std::cout << "  Testing concepts... ";
    
    static_assert(Descriptor<strong_vertex_descriptor>);
    static_assert(Descriptor<strong_edge_index>);
    
    static_assert(VertexDescriptor<strong_vertex_descriptor>);
    static_assert(!VertexDescriptor<strong_edge_index>);
    
    static_assert(EdgeDescriptor<strong_edge_index>);
    static_assert(!EdgeDescriptor<strong_vertex_descriptor>);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Hash Support
// =============================================================================

void test_hash() {
    std::cout << "  Testing hash... ";
    
    strong_vertex_descriptor v1{5};
    strong_vertex_descriptor v2{5};
    strong_vertex_descriptor v3{10};
    
    std::hash<strong_vertex_descriptor> hasher;
    
    // Same value should have same hash
    assert(hasher(v1) == hasher(v2));
    
    // Different values may have different hashes (not guaranteed, but likely)
    // Just verify it doesn't crash
    [[maybe_unused]] auto h3 = hasher(v3);
    
    std::cout << "PASSED\n";
}

void test_unordered_set() {
    std::cout << "  Testing std::unordered_set... ";
    
    std::unordered_set<strong_vertex_descriptor> vertices;
    vertices.insert(strong_vertex_descriptor{0});
    vertices.insert(strong_vertex_descriptor{1});
    vertices.insert(strong_vertex_descriptor{2});
    vertices.insert(strong_vertex_descriptor{1});  // Duplicate
    
    assert(vertices.size() == 3);
    assert(vertices.count(strong_vertex_descriptor{1}) == 1);
    assert(vertices.count(strong_vertex_descriptor{5}) == 0);
    
    std::cout << "PASSED\n";
}

void test_unordered_map() {
    std::cout << "  Testing std::unordered_map... ";
    
    std::unordered_map<strong_vertex_descriptor, std::string> names;
    names[strong_vertex_descriptor{0}] = "Alice";
    names[strong_vertex_descriptor{1}] = "Bob";
    names[strong_vertex_descriptor{2}] = "Charlie";
    
    assert(names[strong_vertex_descriptor{0}] == "Alice");
    assert(names[strong_vertex_descriptor{1}] == "Bob");
    assert(names.size() == 3);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Ordered Containers (uses operator<=>)
// =============================================================================

void test_ordered_set() {
    std::cout << "  Testing std::set... ";
    
    std::set<strong_vertex_descriptor> vertices;
    vertices.insert(strong_vertex_descriptor{2});
    vertices.insert(strong_vertex_descriptor{0});
    vertices.insert(strong_vertex_descriptor{1});
    
    assert(vertices.size() == 3);
    
    // Should be in order
    auto it = vertices.begin();
    assert(it->value() == 0); ++it;
    assert(it->value() == 1); ++it;
    assert(it->value() == 2);
    
    std::cout << "PASSED\n";
}

void test_ordered_map() {
    std::cout << "  Testing std::map... ";
    
    std::map<strong_vertex_descriptor, int> distances;
    distances[strong_vertex_descriptor{2}] = 20;
    distances[strong_vertex_descriptor{0}] = 0;
    distances[strong_vertex_descriptor{1}] = 10;
    
    assert(distances.size() == 3);
    assert(distances[strong_vertex_descriptor{1}] == 10);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Array Indexing
// =============================================================================

void test_array_indexing() {
    std::cout << "  Testing array indexing... ";
    
    std::vector<std::string> names = {"Alice", "Bob", "Charlie"};
    
    strong_vertex_descriptor v{1};
    
    // Implicit conversion allows array indexing
    assert(names[v] == "Bob");
    
    // Modify through index
    names[v] = "Robert";
    assert(names[v] == "Robert");
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Helper Functions
// =============================================================================

void test_make_descriptors() {
    std::cout << "  Testing make_vertex_descriptor/make_edge_descriptor... ";
    
    auto v = make_vertex_descriptor(42);
    auto e = make_edge_descriptor(10);
    
    static_assert(is_vertex_descriptor_v<decltype(v)>);
    static_assert(is_edge_descriptor_v<decltype(e)>);
    
    assert(v.value() == 42);
    assert(e.value() == 10);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Constexpr Support
// =============================================================================

void test_constexpr() {
    std::cout << "  Testing constexpr support... ";
    
    // All these should work at compile time
    constexpr strong_vertex_descriptor v1{5};
    constexpr strong_vertex_descriptor v2{10};
    
    static_assert(v1.value() == 5);
    static_assert(v2.value() == 10);
    static_assert(v1 < v2);
    static_assert(v1 != v2);
    static_assert(v1.valid());
    static_assert(!v1.null());
    
    constexpr auto null = strong_vertex_descriptor::null_descriptor();
    static_assert(null.null());
    static_assert(!null.valid());
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Different Index Types
// =============================================================================

void test_custom_index_type() {
    std::cout << "  Testing custom index types... ";
    
    // Use uint32_t instead of size_t
    using small_vertex = descriptor<vertex_tag, std::uint32_t>;
    
    small_vertex v{100};
    assert(v.value() == 100);
    assert(v.valid());
    
    // Verify it's a different type from the default
    static_assert(!std::is_same_v<small_vertex, strong_vertex_descriptor>);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Test Strong Edge Descriptor
// =============================================================================

void test_strong_edge_descriptor() {
    std::cout << "  Testing strong_edge_descriptor... ";
    
    strong_edge_descriptor<> e1{
        strong_vertex_descriptor{0},
        strong_vertex_descriptor{1},
        42
    };
    
    assert(e1.source.value() == 0);
    assert(e1.target.value() == 1);
    assert(e1.edge_index == 42);
    assert(e1.index() == 42);
    
    // Comparison
    strong_edge_descriptor<> e2{
        strong_vertex_descriptor{0},
        strong_vertex_descriptor{1},
        42
    };
    assert(e1 == e2);
    
    strong_edge_descriptor<> e3{
        strong_vertex_descriptor{0},
        strong_vertex_descriptor{2},
        42
    };
    assert(e1 != e3);
    
    std::cout << "PASSED\n";
}

void test_strong_edge_descriptor_hash() {
    std::cout << "  Testing strong_edge_descriptor hash... ";
    
    std::unordered_set<strong_edge_descriptor<>> edges;
    
    edges.insert({strong_vertex_descriptor{0}, strong_vertex_descriptor{1}, 0});
    edges.insert({strong_vertex_descriptor{1}, strong_vertex_descriptor{2}, 1});
    edges.insert({strong_vertex_descriptor{0}, strong_vertex_descriptor{1}, 0}); // Duplicate
    
    assert(edges.size() == 2);
    
    std::cout << "PASSED\n";
}

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "Running BGL Modern Strong Descriptor Tests\n";
    std::cout << "==========================================\n\n";
    
    std::cout << "Construction Tests:\n";
    test_default_construction();
    test_value_construction();
    test_null_descriptor();
    
    std::cout << "\nComparison Tests:\n";
    test_equality();
    test_ordering();
    
    std::cout << "\nType Safety Tests:\n";
    test_type_safety_compile_time();
    test_type_traits();
    test_concepts();
    
    std::cout << "\nHash Tests:\n";
    test_hash();
    test_unordered_set();
    test_unordered_map();
    
    std::cout << "\nOrdered Container Tests:\n";
    test_ordered_set();
    test_ordered_map();
    
    std::cout << "\nArray Indexing Tests:\n";
    test_array_indexing();
    
    std::cout << "\nHelper Function Tests:\n";
    test_make_descriptors();
    
    std::cout << "\nConstexpr Tests:\n";
    test_constexpr();
    
    std::cout << "\nCustom Type Tests:\n";
    test_custom_index_type();
    
    std::cout << "\nStrong Edge Descriptor Tests:\n";
    test_strong_edge_descriptor();
    test_strong_edge_descriptor_hash();
    
    std::cout << "\n==========================================\n";
    std::cout << "All strong descriptor tests passed!\n";
    
    return 0;
}
