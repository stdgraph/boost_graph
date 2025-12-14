// BGL Modern - Property Maps
// C++20 concepts and utilities for property maps
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// =============================================================================
// PROPERTY MAP OVERVIEW
// =============================================================================
//
// Property maps provide a uniform abstraction for accessing properties
// associated with graph vertices and edges. This header provides:
//
// 1. CONCEPTS (defined in concepts.hpp):
//    - PropertyMap<F, Key, Value>: Base concept (invocable Key -> Value)
//    - ReadablePropertyMap<F, Key, Value>: Readable property access
//    - WritablePropertyMap<PM, Key, Value>: Writable property access
//    - ReadWritePropertyMap<PM, Key, Value>: Both read and write
//    - LvaluePropertyMap<PMap, Key>: Returns lvalue reference
//
// 2. FREE FUNCTIONS:
//    - get(pmap, key) -> value: Read from property map
//    - put(pmap, key, value): Write to property map
//
// 3. PROPERTY MAP ADAPTERS:
//    - vector_property_map<T>: Vector-backed for integral keys
//    - iterator_property_map<Iter, IndexMap>: Iterator + index map
//    - identity_property_map: Returns key as value
//
// 4. DEFAULT ACCESSORS:
//    - default_weight_accessor(g): Access g[e].weight
//    - default_distance_accessor(g): Access g[v].distance
//    - vertex_property_accessor(g, &T::member): Access member pointer
//    - edge_property_accessor(g, &T::member): Access member pointer
//
// =============================================================================
// DESCRIPTOR TYPES AND CONTAINER CHOICES
// =============================================================================
//
// Graph descriptors can be implemented in several ways, each with different
// implications for property map design:
//
// 1. INTEGRAL INDICES (e.g., size_t):
//    - vertex_descriptor = std::size_t
//    - Direct array indexing: container[descriptor]
//    - Best container: std::vector (O(1) access)
//    - Example:
//        std::vector<Color> colors(num_vertices(g));
//        auto color_map = [&colors](size_t v) -> Color& { return colors[v]; };
//
// 2. ITERATOR-BASED DESCRIPTORS:
//    - vertex_descriptor = some_iterator_type
//    - May need conversion to index via vertex_index map
//    - Use iterator_property_map adapter
//    - Example:
//        auto pmap = make_iterator_property_map(colors.begin(), vertex_index_map);
//
// 3. OPAQUE HANDLES (pointers, smart handles):
//    - vertex_descriptor = void*, node*, handle<T>
//    - Cannot use direct array indexing
//    - Best container: std::unordered_map (O(1) average)
//    - Example:
//        std::unordered_map<vertex_descriptor, Color> colors;
//        auto color_map = [&colors](vertex_descriptor v) -> Color& { 
//            return colors[v]; 
//        };
//
// CONTAINER CHOICE SUMMARY:
// +-----------------------+---------------------------+------------------------+
// | Descriptor Type       | Recommended Container     | Property Map Pattern   |
// +-----------------------+---------------------------+------------------------+
// | size_t (integral)     | std::vector               | vector_property_map    |
// | iterator              | std::vector + index_map   | iterator_property_map  |
// | pointer/handle        | std::unordered_map        | Lambda with map        |
// | sparse (few have it)  | std::unordered_map        | Lambda with map        |
// +-----------------------+---------------------------+------------------------+
//
// =============================================================================
// THE GRAPH-CAPTURING LAMBDA PATTERN
// =============================================================================
//
// The recommended pattern for creating property maps that access bundled
// properties is to use a graph-capturing lambda:
//
//     auto weight = [&g](edge_descriptor e) { return g[e].weight; };
//
// This pattern:
// - Captures the graph by reference
// - Returns the property value (or reference for mutable access)
// - Works uniformly whether properties are bundled or external
// - Requires no separate property map library
//
// For mutable access, return a reference:
//
//     auto color = [&g](vertex_descriptor v) -> Color& { return g[v].color; };
//     color(v) = Color::black;  // Direct assignment
//
// =============================================================================

#ifndef BGL_MODERN_PROPERTY_MAP_HPP
#define BGL_MODERN_PROPERTY_MAP_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>  // Includes graph_traits.hpp and property map concepts

#include <concepts>
#include <type_traits>
#include <functional>
#include <utility>
#include <vector>

namespace bgl {

// =============================================================================
// Property Map Value Type Deduction
// =============================================================================

/// Deduce the value type of a property map from its invocation result
template<typename PMap, typename Key>
using property_value_t = std::remove_cvref_t<std::invoke_result_t<PMap, Key>>;

// =============================================================================
// LvaluePropertyMap Concept
// =============================================================================

/// An LvaluePropertyMap returns an lvalue reference, allowing direct modification.
///
/// Requirements:
/// - pmap(key) returns a reference (lvalue or rvalue) to Value
///
/// Example:
/// @code
///     auto color_map = [&colors](vertex_descriptor v) -> Color& {
///         return colors[v];
///     };
///     color_map(v) = Color::white;  // Direct assignment via reference
/// @endcode
///
template<typename PMap, typename Key, typename Value = property_value_t<PMap, Key>>
concept LvaluePropertyMap = 
    std::invocable<PMap, Key> &&
    std::is_lvalue_reference_v<std::invoke_result_t<PMap, Key>>;

// =============================================================================
// put() and get() Free Functions
// =============================================================================

/// Read a property value from a readable property map
/// 
/// @param pmap The property map
/// @param key The key to look up
/// @return The property value associated with key
///
template<typename PMap, typename Key>
    requires std::invocable<PMap, Key>
constexpr decltype(auto) get(PMap&& pmap, Key&& key) {
    return std::invoke(std::forward<PMap>(pmap), std::forward<Key>(key));
}

/// Write a property value to a property map that returns an lvalue reference
///
/// @param pmap The property map (must return lvalue reference)
/// @param key The key to write to
/// @param value The value to write
///
template<typename PMap, typename Key, typename Value>
    requires std::invocable<std::remove_cvref_t<PMap>&, std::remove_cvref_t<Key>>
constexpr void put(PMap&& pmap, Key&& key, Value&& value) {
    std::invoke(std::forward<PMap>(pmap), std::forward<Key>(key)) = std::forward<Value>(value);
}

// =============================================================================
// Property Map Adapters
// =============================================================================

/// An iterator property map that uses a random-access iterator and an index map.
///
/// This is useful for creating property maps backed by contiguous storage
/// (vectors, arrays) when the key is not directly an index.
///
/// @tparam Iterator Random access iterator type
/// @tparam IndexMap Property map from Key -> Index
///
/// Descriptor Types:
/// -----------------
/// Graph descriptors can be:
/// - Integral indices (size_t): Direct array indexing is possible
/// - Iterator-based: Requires conversion to index or iterator arithmetic
/// - Opaque handles: Requires an index map to convert to array index
///
/// For simple cases where vertex_descriptor is size_t, use make_vector_property_map
/// which directly indexes into the backing container.
///
template<std::random_access_iterator Iterator, typename IndexMap>
class iterator_property_map {
public:
    using key_type = property_value_t<IndexMap, typename std::iterator_traits<Iterator>::difference_type>;
    using value_type = typename std::iterator_traits<Iterator>::value_type;
    using reference = typename std::iterator_traits<Iterator>::reference;
    
    iterator_property_map() = default;
    
    iterator_property_map(Iterator iter, IndexMap index_map)
        : iter_(iter), index_map_(std::move(index_map)) {}
    
    reference operator()(const key_type& key) const {
        return iter_[get(index_map_, key)];
    }
    
private:
    Iterator iter_;
    IndexMap index_map_;
};

/// Create an iterator property map
template<std::random_access_iterator Iterator, typename IndexMap>
auto make_iterator_property_map(Iterator iter, IndexMap index_map) {
    return iterator_property_map<Iterator, IndexMap>(iter, std::move(index_map));
}

/// A simple vector-backed property map for integral key types.
///
/// This is the simplest property map: directly indexes into a vector.
/// Suitable when vertex_descriptor or edge_descriptor is size_t.
///
/// Container Choices:
/// ------------------
/// - std::vector: Best for dense property storage, O(1) access
/// - std::unordered_map: Best for sparse properties or non-integral keys
/// - std::array: Best when size is known at compile time
///
/// @tparam Container The underlying container type (default: std::vector)
///
template<typename T, typename Container = std::vector<T>>
class vector_property_map {
public:
    using key_type = typename Container::size_type;
    using value_type = T;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;
    
    vector_property_map() = default;
    
    explicit vector_property_map(std::size_t size) : data_(size) {}
    explicit vector_property_map(std::size_t size, const T& value) : data_(size, value) {}
    explicit vector_property_map(Container data) : data_(std::move(data)) {}
    
    reference operator()(key_type key) { return data_[key]; }
    const_reference operator()(key_type key) const { return data_[key]; }
    
    /// Access underlying container
    Container& data() { return data_; }
    const Container& data() const { return data_; }
    
    /// Resize the backing storage
    void resize(std::size_t size) { data_.resize(size); }
    void resize(std::size_t size, const T& value) { data_.resize(size, value); }
    
private:
    Container data_;
};

/// Create a vector property map with given size
template<typename T>
auto make_vector_property_map(std::size_t size) {
    return vector_property_map<T>(size);
}

/// Create a vector property map with given size and default value
template<typename T>
auto make_vector_property_map(std::size_t size, const T& default_value) {
    return vector_property_map<T>(size, default_value);
}

// =============================================================================
// Identity Property Map
// =============================================================================

/// A property map that returns the key itself as the value.
///
/// Useful as a default index map when vertex_descriptor is already an index.
///
struct identity_property_map {
    template<typename Key>
    constexpr Key operator()(Key key) const noexcept {
        return key;
    }
};

// =============================================================================
// Default Property Accessors
// =============================================================================

/// Create a default weight accessor for a graph.
///
/// This is the recommended pattern for algorithm defaults:
/// - Captures the graph by reference
/// - Returns edge weight via g[e].weight (assumes bundled properties)
///
/// Example:
/// @code
///     adjacency_list<...> g;
///     auto weight = default_weight_accessor(g);
///     for (auto e : edges(g)) {
///         std::cout << weight(e) << "\n";
///     }
/// @endcode
///
/// @param g The graph (must have bundled edge properties with .weight member)
/// @return A lambda that extracts edge weights
///
template<typename G>
    requires requires(const G& g, edge_descriptor_t<G> e) { { g[e].weight } -> std::convertible_to<double>; }
auto default_weight_accessor(const G& g) {
    return [&g](edge_descriptor_t<G> e) -> decltype(auto) {
        return g[e].weight;
    };
}

/// Create a default distance accessor for a graph.
///
/// Similar to default_weight_accessor but for vertex distances.
///
/// @param g The graph (must have bundled vertex properties with .distance member)
/// @return A lambda that extracts vertex distances
///
template<typename G>
    requires requires(const G& g, vertex_descriptor_t<G> v) { { g[v].distance } -> std::convertible_to<double>; }
auto default_distance_accessor(const G& g) {
    return [&g](vertex_descriptor_t<G> v) -> decltype(auto) {
        return g[v].distance;
    };
}

/// Create a mutable distance accessor for a graph.
///
/// Returns a reference to allow both reading and writing.
///
/// @param g The graph (mutable, must have bundled vertex properties with .distance member)
/// @return A lambda that returns a reference to vertex distances
///
template<typename G>
    requires requires(G& g, vertex_descriptor_t<G> v) { { g[v].distance } -> std::convertible_to<double>; }
auto mutable_distance_accessor(G& g) {
    return [&g](vertex_descriptor_t<G> v) -> decltype(auto) {
        return (g[v].distance);  // Parentheses preserve reference
    };
}

// =============================================================================
// Bundled Property Access Helpers
// =============================================================================

/// Create a property accessor for a specific member of bundled vertex properties.
///
/// @param g The graph
/// @param member Pointer to member of vertex property type
/// @return A lambda that extracts the specified member
///
template<typename G, typename Property, typename Member>
auto vertex_property_accessor(const G& g, Member Property::* member) {
    return [&g, member](vertex_descriptor_t<G> v) -> decltype(auto) {
        return g[v].*member;
    };
}

/// Create a mutable property accessor for a specific member of bundled vertex properties.
template<typename G, typename Property, typename Member>
auto mutable_vertex_property_accessor(G& g, Member Property::* member) {
    return [&g, member](vertex_descriptor_t<G> v) -> Member& {
        return g[v].*member;
    };
}

/// Create a property accessor for a specific member of bundled edge properties.
///
/// @param g The graph
/// @param member Pointer to member of edge property type
/// @return A lambda that extracts the specified member
///
template<typename G, typename Property, typename Member>
auto edge_property_accessor(const G& g, Member Property::* member) {
    return [&g, member](edge_descriptor_t<G> e) -> decltype(auto) {
        return g[e].*member;
    };
}

/// Create a mutable property accessor for a specific member of bundled edge properties.
template<typename G, typename Property, typename Member>
auto mutable_edge_property_accessor(G& g, Member Property::* member) {
    return [&g, member](edge_descriptor_t<G> e) -> Member& {
        return g[e].*member;
    };
}

} // namespace bgl

#endif // BGL_MODERN_PROPERTY_MAP_HPP
