// BGL Modern - Range-Returning Free Functions
// C++20 range-based graph traversal functions
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_RANGE_FUNCTIONS_HPP
#define BGL_MODERN_RANGE_FUNCTIONS_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/graph_traits.hpp>

#include <ranges>
#include <iterator>
#include <cstddef>

namespace bgl {

// =============================================================================
// Range-Returning Free Function Concepts
// =============================================================================

/// Concept for types that can provide a vertex range
template<typename G>
concept HasVertexRange = requires(const G& g) {
    { g.vertex_begin() } -> std::input_or_output_iterator;
    { g.vertex_end() } -> std::sentinel_for<decltype(g.vertex_begin())>;
};

/// Concept for types that can provide an edge range
template<typename G>
concept HasEdgeRange = requires(const G& g) {
    { g.edge_begin() } -> std::input_or_output_iterator;
    { g.edge_end() } -> std::sentinel_for<decltype(g.edge_begin())>;
};

/// Concept for types that can provide out-edge range
template<typename G>
concept HasOutEdgeRange = requires(const G& g, vertex_descriptor_t<G> v) {
    { g.out_edge_begin(v) } -> std::input_or_output_iterator;
    { g.out_edge_end(v) } -> std::sentinel_for<decltype(g.out_edge_begin(v))>;
};

/// Concept for types that can provide in-edge range
template<typename G>
concept HasInEdgeRange = requires(const G& g, vertex_descriptor_t<G> v) {
    { g.in_edge_begin(v) } -> std::input_or_output_iterator;
    { g.in_edge_end(v) } -> std::sentinel_for<decltype(g.in_edge_begin(v))>;
};

/// Concept for types that can provide adjacent vertices range
template<typename G>
concept HasAdjacentVertexRange = requires(const G& g, vertex_descriptor_t<G> v) {
    { g.adjacent_vertex_begin(v) } -> std::input_or_output_iterator;
    { g.adjacent_vertex_end(v) } -> std::sentinel_for<decltype(g.adjacent_vertex_begin(v))>;
};

// =============================================================================
// vertices(g) - Return range of all vertices
// =============================================================================

/// Returns a range of all vertices in the graph.
/// 
/// Usage:
///   for (auto v : vertices(g)) { ... }
///   auto count = std::ranges::distance(vertices(g));
///   auto filtered = vertices(g) | std::views::filter(pred);
///
template<HasVertexRange G>
[[nodiscard]] constexpr auto vertices(const G& g) noexcept(
    noexcept(g.vertex_begin()) && noexcept(g.vertex_end())
) {
    return std::ranges::subrange(g.vertex_begin(), g.vertex_end());
}

/// Returns the number of vertices in the graph.
template<typename G>
    requires requires(const G& g) { { g.num_vertices() } -> std::convertible_to<std::size_t>; }
[[nodiscard]] constexpr auto num_vertices(const G& g) noexcept(
    noexcept(g.num_vertices())
) -> vertices_size_type_t<G> {
    return g.num_vertices();
}

// =============================================================================
// edges(g) - Return range of all edges
// =============================================================================

/// Returns a range of all edges in the graph.
///
/// Usage:
///   for (auto e : edges(g)) { ... }
///   auto heavy = edges(g) | std::views::filter([&](auto e) { 
///       return weight(e) > threshold; 
///   });
///
template<HasEdgeRange G>
[[nodiscard]] constexpr auto edges(const G& g) noexcept(
    noexcept(g.edge_begin()) && noexcept(g.edge_end())
) {
    return std::ranges::subrange(g.edge_begin(), g.edge_end());
}

/// Returns the number of edges in the graph.
template<typename G>
    requires requires(const G& g) { { g.num_edges() } -> std::convertible_to<std::size_t>; }
[[nodiscard]] constexpr auto num_edges(const G& g) noexcept(
    noexcept(g.num_edges())
) -> edges_size_type_t<G> {
    return g.num_edges();
}

// =============================================================================
// out_edges(v, g) - Return range of outgoing edges from vertex v
// =============================================================================

/// Returns a range of outgoing edges from vertex v.
///
/// Usage:
///   for (auto e : out_edges(v, g)) { 
///       auto target_v = target(e, g);
///       ...
///   }
///
template<HasOutEdgeRange G>
[[nodiscard]] constexpr auto out_edges(
    vertex_descriptor_t<G> v, 
    const G& g
) noexcept(
    noexcept(g.out_edge_begin(v)) && noexcept(g.out_edge_end(v))
) {
    return std::ranges::subrange(g.out_edge_begin(v), g.out_edge_end(v));
}

/// Returns the number of outgoing edges from vertex v.
template<typename G>
    requires requires(const G& g, vertex_descriptor_t<G> v) { 
        { g.out_degree(v) } -> std::convertible_to<std::size_t>; 
    }
[[nodiscard]] constexpr auto out_degree(
    vertex_descriptor_t<G> v,
    const G& g
) noexcept(
    noexcept(g.out_degree(v))
) -> degree_size_type_t<G> {
    return g.out_degree(v);
}

// =============================================================================
// in_edges(v, g) - Return range of incoming edges to vertex v
// =============================================================================

/// Returns a range of incoming edges to vertex v.
/// Only available for bidirectional graphs.
///
/// Usage:
///   for (auto e : in_edges(v, g)) {
///       auto source_v = source(e, g);
///       ...
///   }
///
template<HasInEdgeRange G>
[[nodiscard]] constexpr auto in_edges(
    vertex_descriptor_t<G> v,
    const G& g
) noexcept(
    noexcept(g.in_edge_begin(v)) && noexcept(g.in_edge_end(v))
) {
    return std::ranges::subrange(g.in_edge_begin(v), g.in_edge_end(v));
}

/// Returns the number of incoming edges to vertex v.
template<typename G>
    requires requires(const G& g, vertex_descriptor_t<G> v) { 
        { g.in_degree(v) } -> std::convertible_to<std::size_t>; 
    }
[[nodiscard]] constexpr auto in_degree(
    vertex_descriptor_t<G> v,
    const G& g
) noexcept(
    noexcept(g.in_degree(v))
) -> degree_size_type_t<G> {
    return g.in_degree(v);
}

/// Returns total degree (in + out) for bidirectional graphs.
template<typename G>
    requires requires(const G& g, vertex_descriptor_t<G> v) { 
        { g.in_degree(v) } -> std::convertible_to<std::size_t>;
        { g.out_degree(v) } -> std::convertible_to<std::size_t>;
    }
[[nodiscard]] constexpr auto degree(
    vertex_descriptor_t<G> v,
    const G& g
) noexcept(
    noexcept(g.in_degree(v)) && noexcept(g.out_degree(v))
) -> degree_size_type_t<G> {
    return g.in_degree(v) + g.out_degree(v);
}

// =============================================================================
// adjacent_vertices(v, g) - Return range of vertices adjacent to v
// =============================================================================

/// Returns a range of vertices adjacent to vertex v.
///
/// Usage:
///   for (auto neighbor : adjacent_vertices(v, g)) { ... }
///   auto neighbors = adjacent_vertices(v, g) | std::views::take(5);
///
template<HasAdjacentVertexRange G>
[[nodiscard]] constexpr auto adjacent_vertices(
    vertex_descriptor_t<G> v,
    const G& g
) noexcept(
    noexcept(g.adjacent_vertex_begin(v)) && noexcept(g.adjacent_vertex_end(v))
) {
    return std::ranges::subrange(g.adjacent_vertex_begin(v), g.adjacent_vertex_end(v));
}

// =============================================================================
// source(e, g) and target(e, g) - Edge endpoint accessors
// =============================================================================

/// Returns the source vertex of edge e.
template<typename G>
    requires requires(const G& g, edge_descriptor_t<G> e) {
        { g.source(e) } -> std::convertible_to<vertex_descriptor_t<G>>;
    }
[[nodiscard]] constexpr auto source(
    edge_descriptor_t<G> e,
    const G& g
) noexcept(
    noexcept(g.source(e))
) -> vertex_descriptor_t<G> {
    return g.source(e);
}

/// Returns the target vertex of edge e.
template<typename G>
    requires requires(const G& g, edge_descriptor_t<G> e) {
        { g.target(e) } -> std::convertible_to<vertex_descriptor_t<G>>;
    }
[[nodiscard]] constexpr auto target(
    edge_descriptor_t<G> e,
    const G& g
) noexcept(
    noexcept(g.target(e))
) -> vertex_descriptor_t<G> {
    return g.target(e);
}

// =============================================================================
// Range Adaptor Helpers
// =============================================================================

/// Get targets of all out-edges from a vertex as a range.
/// Combines out_edges with target extraction.
///
/// Usage:
///   for (auto neighbor : out_neighbors(v, g)) { ... }
///
template<HasOutEdgeRange G>
    requires requires(const G& g, edge_descriptor_t<G> e) {
        { g.target(e) } -> std::convertible_to<vertex_descriptor_t<G>>;
    }
[[nodiscard]] constexpr auto out_neighbors(
    vertex_descriptor_t<G> v,
    const G& g
) {
    return out_edges(v, g) | std::views::transform([&g](auto e) {
        return g.target(e);
    });
}

/// Get sources of all in-edges to a vertex as a range.
/// Combines in_edges with source extraction.
///
/// Usage:
///   for (auto predecessor : in_neighbors(v, g)) { ... }
///
template<HasInEdgeRange G>
    requires requires(const G& g, edge_descriptor_t<G> e) {
        { g.source(e) } -> std::convertible_to<vertex_descriptor_t<G>>;
    }
[[nodiscard]] constexpr auto in_neighbors(
    vertex_descriptor_t<G> v,
    const G& g
) {
    return in_edges(v, g) | std::views::transform([&g](auto e) {
        return g.source(e);
    });
}

} // namespace bgl

#endif // BGL_MODERN_RANGE_FUNCTIONS_HPP
