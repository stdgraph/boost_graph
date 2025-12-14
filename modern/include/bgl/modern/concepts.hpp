// BGL Modern - Graph Concepts
// C++20 concepts for graph types
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_CONCEPTS_HPP
#define BGL_MODERN_CONCEPTS_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/graph_traits.hpp>

#include <concepts>
#include <ranges>
#include <type_traits>
#include <cstddef>

namespace bgl {

// =============================================================================
// Graph Concept
// =============================================================================

/// Base concept for all graph types.
/// 
/// A Graph must provide:
/// - vertex_descriptor type (via graph_traits)
/// - edge_descriptor type (via graph_traits)
/// - directed_category type (via graph_traits)
///
/// This is the minimal requirement for any graph type.
///
template<typename G>
concept Graph = requires {
    typename graph_traits<G>::vertex_descriptor;
    typename graph_traits<G>::edge_descriptor;
    typename graph_traits<G>::directed_category;
};

// =============================================================================
// IncidenceGraph Concept
// =============================================================================

/// Concept for graphs that provide access to outgoing edges.
///
/// An IncidenceGraph must provide:
/// - out_edges(v, g) -> range of edge descriptors
/// - out_degree(v, g) -> size_t (number of outgoing edges)
/// - source(e, g) -> vertex_descriptor (source vertex of edge)
/// - target(e, g) -> vertex_descriptor (target vertex of edge)
///
template<typename G>
concept IncidenceGraph = Graph<G> && requires(
    const G& g,
    vertex_descriptor_t<G> v,
    edge_descriptor_t<G> e
) {
    { out_edges(v, g) } -> std::ranges::forward_range;
    { out_degree(v, g) } -> std::convertible_to<std::size_t>;
    { source(e, g) } -> std::same_as<vertex_descriptor_t<G>>;
    { target(e, g) } -> std::same_as<vertex_descriptor_t<G>>;
};

// =============================================================================
// BidirectionalGraph Concept
// =============================================================================

/// Concept for graphs that provide access to both incoming and outgoing edges.
///
/// A BidirectionalGraph extends IncidenceGraph with:
/// - in_edges(v, g) -> range of edge descriptors
/// - in_degree(v, g) -> size_t (number of incoming edges)
///
template<typename G>
concept BidirectionalGraph = IncidenceGraph<G> && requires(
    const G& g,
    vertex_descriptor_t<G> v
) {
    { in_edges(v, g) } -> std::ranges::forward_range;
    { in_degree(v, g) } -> std::convertible_to<std::size_t>;
};

// =============================================================================
// VertexListGraph Concept
// =============================================================================

/// Concept for graphs that can enumerate all vertices.
///
/// A VertexListGraph must provide:
/// - vertices(g) -> range of vertex descriptors
/// - num_vertices(g) -> size_t (total number of vertices)
///
template<typename G>
concept VertexListGraph = Graph<G> && requires(const G& g) {
    { vertices(g) } -> std::ranges::forward_range;
    { num_vertices(g) } -> std::convertible_to<std::size_t>;
};

// =============================================================================
// EdgeListGraph Concept
// =============================================================================

/// Concept for graphs that can enumerate all edges.
///
/// An EdgeListGraph must provide:
/// - edges(g) -> range of edge descriptors
/// - num_edges(g) -> size_t (total number of edges)
///
template<typename G>
concept EdgeListGraph = Graph<G> && requires(const G& g) {
    { edges(g) } -> std::ranges::forward_range;
    { num_edges(g) } -> std::convertible_to<std::size_t>;
};

// =============================================================================
// AdjacencyGraph Concept
// =============================================================================

/// Concept for graphs that provide direct access to adjacent vertices.
///
/// An AdjacencyGraph must provide:
/// - adjacent_vertices(v, g) -> range of vertex descriptors
///
template<typename G>
concept AdjacencyGraph = Graph<G> && requires(
    const G& g,
    vertex_descriptor_t<G> v
) {
    { adjacent_vertices(v, g) } -> std::ranges::forward_range;
};

// =============================================================================
// MutableGraph Concept
// =============================================================================

/// Concept for graphs that support adding and removing vertices and edges.
///
/// A MutableGraph must provide:
/// - add_vertex(g) -> vertex_descriptor
/// - remove_vertex(v, g) -> void
/// - add_edge(u, v, g) -> pair<edge_descriptor, bool> or edge_descriptor
/// - remove_edge(u, v, g) -> void
///
template<typename G>
concept MutableGraph = Graph<G> && requires(
    G& g,
    vertex_descriptor_t<G> u,
    vertex_descriptor_t<G> v
) {
    { add_vertex(g) } -> std::same_as<vertex_descriptor_t<G>>;
    { remove_vertex(u, g) } -> std::same_as<void>;
    { add_edge(u, v, g) };  // Return type varies (pair or edge_descriptor)
    { remove_edge(u, v, g) } -> std::same_as<void>;
};

// =============================================================================
// MutableIncidenceGraph Concept
// =============================================================================

/// Concept for mutable graphs with edge removal by edge descriptor.
///
/// Extends MutableGraph with:
/// - remove_edge(e, g) -> void (remove specific edge)
///
template<typename G>
concept MutableIncidenceGraph = MutableGraph<G> && IncidenceGraph<G> && requires(
    G& g,
    edge_descriptor_t<G> e
) {
    { remove_edge(e, g) } -> std::same_as<void>;
};

// =============================================================================
// PropertyGraph Concept
// =============================================================================

/// Concept for graphs with property access via operator[].
///
/// A PropertyGraph must provide:
/// - g[v] -> vertex property reference
/// - g[e] -> edge property reference
///
template<typename G>
concept VertexPropertyGraph = Graph<G> && requires(
    G& g,
    const G& cg,
    vertex_descriptor_t<G> v
) {
    { g[v] };   // Mutable access
    { cg[v] };  // Const access
};

template<typename G>
concept EdgePropertyGraph = Graph<G> && requires(
    G& g,
    const G& cg,
    edge_descriptor_t<G> e
) {
    { g[e] };   // Mutable access
    { cg[e] };  // Const access
};

template<typename G>
concept PropertyGraph = VertexPropertyGraph<G> && EdgePropertyGraph<G>;

// =============================================================================
// Compound Concepts
// =============================================================================

/// A graph that can enumerate vertices and access outgoing edges
template<typename G>
concept VertexListIncidenceGraph = VertexListGraph<G> && IncidenceGraph<G>;

/// A graph with full traversal capabilities
template<typename G>
concept TraversableGraph = VertexListGraph<G> && IncidenceGraph<G>;

/// A fully-featured mutable graph
template<typename G>
concept MutableVertexListGraph = MutableGraph<G> && VertexListGraph<G>;

// =============================================================================
// PropertyMap Concepts
// =============================================================================

/// Concept for a readable property map (callable that maps Key -> Value)
template<typename F, typename Key, typename Value>
concept ReadablePropertyMap = std::invocable<F, Key> &&
    std::convertible_to<std::invoke_result_t<F, Key>, Value>;

/// Concept for a writable property map
template<typename PM, typename Key, typename Value>
concept WritablePropertyMap = requires(PM& pm, Key k, Value v) {
    { pm(k) = v };
};

/// Concept for a read-write property map
template<typename PM, typename Key, typename Value>
concept ReadWritePropertyMap = 
    ReadablePropertyMap<PM, Key, Value> &&
    WritablePropertyMap<PM, Key, Value>;

} // namespace bgl

#endif // BGL_MODERN_CONCEPTS_HPP
