// BGL Modern - Container Selectors
// C++20 container selector tags and container_gen mechanism
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_CONTAINER_SELECTORS_HPP
#define BGL_MODERN_CONTAINER_SELECTORS_HPP

#include <bgl/modern/version.hpp>

#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <type_traits>
#include <concepts>
#include <iterator>
#include <functional>
#include <cstddef>

namespace bgl {

// =============================================================================
// Container Selector Tags
// =============================================================================

/// Selector for std::vector - O(1) random access, O(n) removal, index-based descriptors
struct vecS {
    static constexpr bool is_random_access = true;
    static constexpr bool has_stable_iterators = false;
    static constexpr bool is_ordered = false;
    static constexpr bool is_unique = false;
};

/// Selector for std::list - O(1) insertion/removal, stable iterators, iterator-based descriptors
struct listS {
    static constexpr bool is_random_access = false;
    static constexpr bool has_stable_iterators = true;
    static constexpr bool is_ordered = false;
    static constexpr bool is_unique = false;
};

/// Selector for std::set - O(log n) operations, ordered unique elements
struct setS {
    static constexpr bool is_random_access = false;
    static constexpr bool has_stable_iterators = true;
    static constexpr bool is_ordered = true;
    static constexpr bool is_unique = true;
};

/// Selector for std::map - O(log n) operations, key-value storage (used for vertex->edge mapping)
struct mapS {
    static constexpr bool is_random_access = false;
    static constexpr bool has_stable_iterators = true;
    static constexpr bool is_ordered = true;
    static constexpr bool is_unique = true;
};

/// Selector for std::multiset - O(log n) operations, ordered with duplicates allowed
struct multisetS {
    static constexpr bool is_random_access = false;
    static constexpr bool has_stable_iterators = true;
    static constexpr bool is_ordered = true;
    static constexpr bool is_unique = false;
};

/// Selector for std::multimap - O(log n) operations, key-value with duplicate keys
struct multimapS {
    static constexpr bool is_random_access = false;
    static constexpr bool has_stable_iterators = true;
    static constexpr bool is_ordered = true;
    static constexpr bool is_unique = false;
};

/// Selector for std::unordered_set - O(1) average operations, hash-based unique elements
struct hash_setS {
    static constexpr bool is_random_access = false;
    static constexpr bool has_stable_iterators = false;  // rehashing invalidates
    static constexpr bool is_ordered = false;
    static constexpr bool is_unique = true;
};

/// Selector for std::unordered_map - O(1) average operations, hash-based key-value
struct hash_mapS {
    static constexpr bool is_random_access = false;
    static constexpr bool has_stable_iterators = false;
    static constexpr bool is_ordered = false;
    static constexpr bool is_unique = true;
};

/// Selector for std::unordered_multiset - O(1) average operations, hash-based with duplicates
struct hash_multisetS {
    static constexpr bool is_random_access = false;
    static constexpr bool has_stable_iterators = false;
    static constexpr bool is_ordered = false;
    static constexpr bool is_unique = false;
};

/// Selector for std::unordered_multimap - O(1) average operations, hash-based with duplicate keys
struct hash_multimapS {
    static constexpr bool is_random_access = false;
    static constexpr bool has_stable_iterators = false;
    static constexpr bool is_ordered = false;
    static constexpr bool is_unique = false;
};

// =============================================================================
// Selector Concepts
// =============================================================================

/// Concept for all valid container selectors
template<typename S>
concept ContainerSelector = 
    std::same_as<S, vecS> ||
    std::same_as<S, listS> ||
    std::same_as<S, setS> ||
    std::same_as<S, mapS> ||
    std::same_as<S, multisetS> ||
    std::same_as<S, multimapS> ||
    std::same_as<S, hash_setS> ||
    std::same_as<S, hash_mapS> ||
    std::same_as<S, hash_multisetS> ||
    std::same_as<S, hash_multimapS>;

/// Concept for sequence-like selectors (vecS, listS)
template<typename S>
concept SequenceSelector = 
    std::same_as<S, vecS> ||
    std::same_as<S, listS>;

/// Concept for associative selectors (set, map variants)
template<typename S>
concept AssociativeSelector =
    std::same_as<S, setS> ||
    std::same_as<S, mapS> ||
    std::same_as<S, multisetS> ||
    std::same_as<S, multimapS>;

/// Concept for unordered/hash-based selectors
template<typename S>
concept UnorderedSelector =
    std::same_as<S, hash_setS> ||
    std::same_as<S, hash_mapS> ||
    std::same_as<S, hash_multisetS> ||
    std::same_as<S, hash_multimapS>;

/// Concept for selectors with stable iterators (safe for removal during iteration)
template<typename S>
concept StableIteratorSelector = S::has_stable_iterators;

/// Concept for selectors with random access
template<typename S>
concept RandomAccessSelector = S::is_random_access;

// =============================================================================
// container_gen - Map Selector Tags to Container Types
// =============================================================================

/// Primary template - generates container type from selector
template<typename Selector, typename ValueType>
struct container_gen;

/// vecS -> std::vector
template<typename ValueType>
struct container_gen<vecS, ValueType> {
    using type = std::vector<ValueType>;
    
    static constexpr auto push_back(type& c, const ValueType& v) {
        c.push_back(v);
        return c.end() - 1;
    }
    
    static constexpr auto push_back(type& c, ValueType&& v) {
        c.push_back(std::move(v));
        return c.end() - 1;
    }
};

/// listS -> std::list
template<typename ValueType>
struct container_gen<listS, ValueType> {
    using type = std::list<ValueType>;
    
    static constexpr auto push_back(type& c, const ValueType& v) {
        c.push_back(v);
        return std::prev(c.end());
    }
    
    static constexpr auto push_back(type& c, ValueType&& v) {
        c.push_back(std::move(v));
        return std::prev(c.end());
    }
};

/// setS -> std::set
template<typename ValueType>
struct container_gen<setS, ValueType> {
    using type = std::set<ValueType>;
    
    static constexpr auto insert(type& c, const ValueType& v) {
        return c.insert(v).first;
    }
    
    static constexpr auto insert(type& c, ValueType&& v) {
        return c.insert(std::move(v)).first;
    }
};

/// mapS -> std::set (for edge lists, maps target vertex to edge)
/// Note: For edge containers, mapS is used with set semantics to prevent parallel edges
template<typename ValueType>
struct container_gen<mapS, ValueType> {
    using type = std::set<ValueType>;  // Maps vertex->edge, use set for edge list
    
    static constexpr auto insert(type& c, const ValueType& v) {
        return c.insert(v).first;
    }
    
    static constexpr auto insert(type& c, ValueType&& v) {
        return c.insert(std::move(v)).first;
    }
};

/// multisetS -> std::multiset
template<typename ValueType>
struct container_gen<multisetS, ValueType> {
    using type = std::multiset<ValueType>;
    
    static constexpr auto insert(type& c, const ValueType& v) {
        return c.insert(v);
    }
    
    static constexpr auto insert(type& c, ValueType&& v) {
        return c.insert(std::move(v));
    }
};

/// multimapS -> std::multiset (for edge lists)
template<typename ValueType>
struct container_gen<multimapS, ValueType> {
    using type = std::multiset<ValueType>;
    
    static constexpr auto insert(type& c, const ValueType& v) {
        return c.insert(v);
    }
    
    static constexpr auto insert(type& c, ValueType&& v) {
        return c.insert(std::move(v));
    }
};

/// hash_setS -> std::unordered_set
template<typename ValueType>
struct container_gen<hash_setS, ValueType> {
    using type = std::unordered_set<ValueType>;
    
    static constexpr auto insert(type& c, const ValueType& v) {
        return c.insert(v).first;
    }
    
    static constexpr auto insert(type& c, ValueType&& v) {
        return c.insert(std::move(v)).first;
    }
};

/// hash_mapS -> std::unordered_set (for edge lists)
template<typename ValueType>
struct container_gen<hash_mapS, ValueType> {
    using type = std::unordered_set<ValueType>;
    
    static constexpr auto insert(type& c, const ValueType& v) {
        return c.insert(v).first;
    }
    
    static constexpr auto insert(type& c, ValueType&& v) {
        return c.insert(std::move(v)).first;
    }
};

/// hash_multisetS -> std::unordered_multiset
template<typename ValueType>
struct container_gen<hash_multisetS, ValueType> {
    using type = std::unordered_multiset<ValueType>;
    
    static constexpr auto insert(type& c, const ValueType& v) {
        return c.insert(v);
    }
    
    static constexpr auto insert(type& c, ValueType&& v) {
        return c.insert(std::move(v));
    }
};

/// hash_multimapS -> std::unordered_multiset (for edge lists)
template<typename ValueType>
struct container_gen<hash_multimapS, ValueType> {
    using type = std::unordered_multiset<ValueType>;
    
    static constexpr auto insert(type& c, const ValueType& v) {
        return c.insert(v);
    }
    
    static constexpr auto insert(type& c, ValueType&& v) {
        return c.insert(std::move(v));
    }
};

/// Helper alias for container type
template<typename Selector, typename ValueType>
using container_t = typename container_gen<Selector, ValueType>::type;

// =============================================================================
// Vertex List Container Generation
// =============================================================================

/// Generate vertex storage container from selector
template<typename VertexListS, typename VertexData>
struct vertex_list_gen {
    using container_type = container_t<VertexListS, VertexData>;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;
};

// =============================================================================
// Out-Edge List Container Generation
// =============================================================================

/// Generate out-edge storage container from selector
template<typename OutEdgeListS, typename StoredEdge>
struct out_edge_list_gen {
    using container_type = container_t<OutEdgeListS, StoredEdge>;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;
};

// =============================================================================
// Descriptor Types Based on Selector
// =============================================================================

namespace detail {

/// Determine vertex descriptor type based on vertex list selector
template<typename VertexListS, typename VertexData>
struct vertex_descriptor_selector {
    // Default: use iterator as descriptor for stable containers
    using container_type = container_t<VertexListS, VertexData>;
    using type = typename container_type::iterator;
};

/// Specialization for vecS - use index as descriptor
template<typename VertexData>
struct vertex_descriptor_selector<vecS, VertexData> {
    using type = std::size_t;
};

/// Helper alias
template<typename VertexListS, typename VertexData>
using vertex_descriptor_for = typename vertex_descriptor_selector<VertexListS, VertexData>::type;

/// Determine if vertex descriptor is an index type
template<typename VertexListS>
constexpr bool is_index_based_descriptor = std::same_as<VertexListS, vecS>;

} // namespace detail

// =============================================================================
// Container Utility Functions
// =============================================================================

namespace detail {

/// Get size of container
template<typename Container>
constexpr auto container_size(const Container& c) {
    return c.size();
}

/// Clear container
template<typename Container>
constexpr void container_clear(Container& c) {
    c.clear();
}

/// Add element to container (sequence-style)
template<SequenceSelector S, typename Container, typename T>
constexpr auto container_add(Container& c, T&& value) {
    return container_gen<S, std::remove_cvref_t<T>>::push_back(c, std::forward<T>(value));
}

/// Add element to container (associative-style)
template<typename S, typename Container, typename T>
    requires (AssociativeSelector<S> || UnorderedSelector<S>)
constexpr auto container_add(Container& c, T&& value) {
    return container_gen<S, std::remove_cvref_t<T>>::insert(c, std::forward<T>(value));
}

/// Find element in container
template<typename Container, typename T>
constexpr auto container_find(Container& c, const T& value) {
    if constexpr (requires { c.find(value); }) {
        return c.find(value);
    } else {
        return std::find(c.begin(), c.end(), value);
    }
}

/// Erase element from container by iterator
template<typename Container, typename Iterator>
constexpr auto container_erase(Container& c, Iterator pos) {
    return c.erase(pos);
}

/// Erase element from container by value
template<typename Container, typename T>
constexpr auto container_erase_value(Container& c, const T& value) {
    if constexpr (requires { c.erase(value); }) {
        return c.erase(value);
    } else {
        auto it = std::find(c.begin(), c.end(), value);
        if (it != c.end()) {
            return c.erase(it);
        }
        return c.end();
    }
}

} // namespace detail

// =============================================================================
// Parallel Edge Category Based on Edge List Selector
// =============================================================================

namespace detail {

/// Determine edge parallel category based on out-edge list selector
template<typename OutEdgeListS>
struct parallel_edge_category_selector {
    // Default: allow parallel edges for non-unique containers
    using type = std::conditional_t<
        OutEdgeListS::is_unique,
        disallow_parallel_edge_tag,
        allow_parallel_edge_tag
    >;
};

template<typename OutEdgeListS>
using parallel_edge_category_for = typename parallel_edge_category_selector<OutEdgeListS>::type;

} // namespace detail

} // namespace bgl

#endif // BGL_MODERN_CONTAINER_SELECTORS_HPP
