// BGL Modern - Graph Traits
// Type traits and helper aliases for graph types
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_GRAPH_TRAITS_HPP
#define BGL_MODERN_GRAPH_TRAITS_HPP

#include <bgl/modern/version.hpp>
#include <type_traits>
#include <concepts>
#include <utility>
#include <tuple>
#include <cstddef>

namespace bgl {

// =============================================================================
// Direction Tags
// =============================================================================

/// Tag indicating a directed graph
struct directed_tag {};

/// Tag indicating an undirected graph  
struct undirected_tag {};

/// Tag indicating a bidirectional graph (directed with in-edge access)
struct bidirectional_tag : directed_tag {};

// =============================================================================
// Edge Parallel Category Tags
// =============================================================================

/// Tag indicating graph allows parallel edges (multigraph)
struct allow_parallel_edge_tag {};

/// Tag indicating graph disallows parallel edges (simple graph)
struct disallow_parallel_edge_tag {};

// =============================================================================
// Property Tag
// =============================================================================

/// Empty property type for graphs without vertex/edge/graph properties
struct no_property {};

// =============================================================================
// Graph Traits Primary Template
// =============================================================================

/// Primary template for graph_traits - specializations provide graph-specific types
/// 
/// Graph types should either:
/// 1. Provide nested type aliases (vertex_descriptor, edge_descriptor, etc.)
/// 2. Specialize graph_traits<G> for their type
///
template<typename G>
struct graph_traits {
    using vertex_descriptor      = typename G::vertex_descriptor;
    using edge_descriptor        = typename G::edge_descriptor;
    using directed_category      = typename G::directed_category;
    using edge_parallel_category = typename G::edge_parallel_category;
    using traversal_category     = typename G::traversal_category;
    
    using vertices_size_type     = typename G::vertices_size_type;
    using edges_size_type        = typename G::edges_size_type;
    using degree_size_type       = typename G::degree_size_type;
};

// =============================================================================
// Helper Type Aliases
// =============================================================================

/// Extract vertex_descriptor from a graph type
template<typename G>
using vertex_descriptor_t = typename graph_traits<G>::vertex_descriptor;

/// Extract edge_descriptor from a graph type
template<typename G>
using edge_descriptor_t = typename graph_traits<G>::edge_descriptor;

/// Extract directed_category from a graph type
template<typename G>
using directed_category_t = typename graph_traits<G>::directed_category;

/// Extract edge_parallel_category from a graph type
template<typename G>
using edge_parallel_category_t = typename graph_traits<G>::edge_parallel_category;

/// Extract traversal_category from a graph type
template<typename G>
using traversal_category_t = typename graph_traits<G>::traversal_category;

/// Extract vertices_size_type from a graph type
template<typename G>
using vertices_size_type_t = typename graph_traits<G>::vertices_size_type;

/// Extract edges_size_type from a graph type
template<typename G>
using edges_size_type_t = typename graph_traits<G>::edges_size_type;

/// Extract degree_size_type from a graph type
template<typename G>
using degree_size_type_t = typename graph_traits<G>::degree_size_type;

// =============================================================================
// Direction Type Traits
// =============================================================================

/// Check if a graph is directed
template<typename G>
inline constexpr bool is_directed_v = 
    std::is_base_of_v<directed_tag, directed_category_t<G>>;

/// Check if a graph is undirected
template<typename G>
inline constexpr bool is_undirected_v = 
    std::is_same_v<undirected_tag, directed_category_t<G>>;

/// Check if a graph is bidirectional (has in-edge access)
template<typename G>
inline constexpr bool is_bidirectional_v = 
    std::is_same_v<bidirectional_tag, directed_category_t<G>>;

/// Check if a graph allows parallel edges
template<typename G>
inline constexpr bool allows_parallel_edges_v = 
    std::is_same_v<allow_parallel_edge_tag, edge_parallel_category_t<G>>;

// =============================================================================
// Conditional Type Selection (replaces Boost.MPL)
// =============================================================================

/// Select type based on graph direction (replaces mpl::if_)
/// Usage: directed_select_t<G, DirectedType, UndirectedType>
template<typename G, typename DirectedType, typename UndirectedType>
using directed_select_t = std::conditional_t<
    is_directed_v<G>,
    DirectedType,
    UndirectedType
>;

/// Select type based on bidirectional property
template<typename G, typename BidirectionalType, typename OtherType>
using bidirectional_select_t = std::conditional_t<
    is_bidirectional_v<G>,
    BidirectionalType,
    OtherType
>;

/// Select type based on parallel edge support
template<typename G, typename ParallelType, typename SimpleType>
using parallel_edge_select_t = std::conditional_t<
    allows_parallel_edges_v<G>,
    ParallelType,
    SimpleType
>;

// =============================================================================
// Property Type Detection (SFINAE-free with concepts)
// =============================================================================

namespace detail {

/// Detect if type has vertex_property_type
template<typename G>
concept has_vertex_property = requires {
    typename G::vertex_property_type;
};

/// Detect if type has edge_property_type
template<typename G>
concept has_edge_property = requires {
    typename G::edge_property_type;
};

/// Detect if type has graph_property_type
template<typename G>
concept has_graph_property = requires {
    typename G::graph_property_type;
};

/// Detect if graph provides null_vertex()
template<typename G>
concept has_null_vertex = requires {
    { G::null_vertex() } -> std::same_as<typename G::vertex_descriptor>;
};

} // namespace detail

/// Extract vertex_property_type if available, otherwise void
template<typename G>
using vertex_property_t = std::conditional_t<
    detail::has_vertex_property<G>,
    typename G::vertex_property_type,
    void
>;

/// Extract edge_property_type if available, otherwise void
template<typename G>
using edge_property_t = std::conditional_t<
    detail::has_edge_property<G>,
    typename G::edge_property_type,
    void
>;

/// Extract graph_property_type if available, otherwise void
template<typename G>
using graph_property_t = std::conditional_t<
    detail::has_graph_property<G>,
    typename G::graph_property_type,
    void
>;

// =============================================================================
// Null Vertex/Edge Helpers
// =============================================================================

/// Returns a null vertex descriptor (graph-specific)
template<typename G>
constexpr vertex_descriptor_t<G> null_vertex() {
    if constexpr (detail::has_null_vertex<G>) {
        return G::null_vertex();
    } else if constexpr (std::is_integral_v<vertex_descriptor_t<G>>) {
        return static_cast<vertex_descriptor_t<G>>(-1);
    } else {
        return vertex_descriptor_t<G>{};
    }
}

// =============================================================================
// Graph Traits Specialization Helpers
// =============================================================================

/// Helper base for defining graph_traits specializations
/// 
/// Usage:
///   template<>
///   struct graph_traits<MyGraph> : graph_traits_base<MyGraph> {
///       // Override specific types as needed
///   };
///
template<typename G>
struct graph_traits_base {
    using vertex_descriptor      = std::size_t;
    using edge_descriptor        = std::pair<std::size_t, std::size_t>;
    using directed_category      = directed_tag;
    using edge_parallel_category = disallow_parallel_edge_tag;
    using traversal_category     = void;
    using vertices_size_type     = std::size_t;
    using edges_size_type        = std::size_t;
    using degree_size_type       = std::size_t;
};

// =============================================================================
// Type Identity (for deferred type evaluation)
// =============================================================================

/// Type identity wrapper (C++20 has std::type_identity)
template<typename T>
struct type_identity {
    using type = T;
};

template<typename T>
using type_identity_t = typename type_identity<T>::type;

// =============================================================================
// Compile-Time Graph Property Queries
// =============================================================================

/// Check if vertex_descriptor is integral (enables vector-based property maps)
template<typename G>
inline constexpr bool has_integral_vertex_descriptor_v = 
    std::is_integral_v<vertex_descriptor_t<G>>;

/// Check if edge_descriptor is a pair type
template<typename G>
inline constexpr bool has_pair_edge_descriptor_v = requires {
    typename std::tuple_size<edge_descriptor_t<G>>::type;
    requires std::tuple_size_v<edge_descriptor_t<G>> == 2;
};

} // namespace bgl

#endif // BGL_MODERN_GRAPH_TRAITS_HPP
