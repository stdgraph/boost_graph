// BGL Modern - Dijkstra's Shortest Paths Algorithm
// C++20 implementation of Dijkstra's algorithm
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_DIJKSTRA_SHORTEST_PATHS_HPP
#define BGL_MODERN_DIJKSTRA_SHORTEST_PATHS_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>
#include <bgl/modern/property_map.hpp>
#include <bgl/modern/algorithm_result.hpp>

#include <queue>
#include <vector>
#include <functional>
#include <concepts>
#include <type_traits>

namespace bgl {

// =============================================================================
// Weight Accessor Concept
// =============================================================================

/// Concept for a weight accessor function
/// Must be invocable with an edge descriptor and return a numeric weight
template<typename WeightFunc, typename G>
concept WeightAccessor = 
    std::invocable<WeightFunc, edge_descriptor_t<G>> &&
    std::convertible_to<std::invoke_result_t<WeightFunc, edge_descriptor_t<G>>, double>;

// =============================================================================
// Default Weight Accessor
// =============================================================================

/// Default weight accessor that returns 1.0 for all edges (unweighted graph)
struct unit_weight {
    template<typename Edge>
    constexpr double operator()(Edge&&) const noexcept {
        return 1.0;
    }
};

/// Default weight accessor that accesses g[e].weight for bundled properties
template<typename G>
auto make_edge_weight_accessor(const G& g) {
    if constexpr (requires(const G& g, edge_descriptor_t<G> e) { g[e].weight; }) {
        return [&g](edge_descriptor_t<G> e) { return g[e].weight; };
    } else {
        return unit_weight{};
    }
}

// =============================================================================
// dijkstra_shortest_paths - Core Implementation
// =============================================================================

namespace detail {

/// Core Dijkstra implementation using a priority queue
template<typename G, typename DistanceType, typename WeightFunc>
void dijkstra_impl(
    const G& g,
    vertex_descriptor_t<G> source,
    dijkstra_result<G, DistanceType>& result,
    WeightFunc&& get_weight
) {
    using vertex_descriptor = vertex_descriptor_t<G>;
    using distance_type = DistanceType;
    
    // Priority queue entry: (distance, vertex)
    // Using greater<> for min-heap behavior
    using pq_entry = std::pair<distance_type, vertex_descriptor>;
    std::priority_queue<pq_entry, std::vector<pq_entry>, std::greater<pq_entry>> pq;
    
    // Get property maps from result
    auto dist = result.distance_map();
    auto pred = result.predecessor_map();
    
    // Initialize source
    result.set_source(source);
    pq.push({distance_type{0}, source});
    
    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        
        // Skip if we've already found a better path
        if (d > dist(u)) {
            continue;
        }
        
        // Relax all outgoing edges
        for (auto e : out_edges(u, g)) {
            vertex_descriptor v = target(e, g);
            distance_type weight = static_cast<distance_type>(get_weight(e));
            distance_type new_dist = dist(u) + weight;
            
            if (new_dist < dist(v)) {
                dist(v) = new_dist;
                pred(v) = u;
                pq.push({new_dist, v});
            }
        }
    }
}

} // namespace detail

// =============================================================================
// dijkstra_shortest_paths - Public Interface
// =============================================================================

/// Compute shortest paths from a source vertex using Dijkstra's algorithm.
///
/// This overload uses unit weights (all edges have weight 1.0), making it
/// equivalent to BFS for shortest path counting.
///
/// Requirements:
/// - G must satisfy VertexListGraph and IncidenceGraph concepts
/// - Vertex descriptors must be integral (for indexing into vectors)
///
/// Complexity: O((V + E) log V) with binary heap
///
/// @param g The graph
/// @param source The source vertex
/// @return dijkstra_result containing distances and predecessors
///
/// Example:
/// @code
///     auto result = dijkstra_shortest_paths(g, 0);
///     std::cout << "Distance to vertex 5: " << result.distance_to(5) << "\n";
/// @endcode
///
template<typename G>
    requires VertexListGraph<G> && IncidenceGraph<G>
auto dijkstra_shortest_paths(const G& g, vertex_descriptor_t<G> source) {
    auto result = make_dijkstra_result<double>(g, source);
    detail::dijkstra_impl(g, source, result, make_edge_weight_accessor(g));
    return result;
}

/// Compute shortest paths from a source vertex using Dijkstra's algorithm
/// with a custom weight accessor.
///
/// Requirements:
/// - G must satisfy VertexListGraph and IncidenceGraph concepts
/// - WeightFunc must be invocable with edge_descriptor and return a numeric type
/// - All edge weights must be non-negative
///
/// Complexity: O((V + E) log V) with binary heap
///
/// @param g The graph
/// @param source The source vertex
/// @param get_weight Function to extract weight from an edge
/// @return dijkstra_result containing distances and predecessors
///
/// Example:
/// @code
///     // Using a lambda to access edge weights
///     auto result = dijkstra_shortest_paths(g, 0, 
///         [&g](auto e) { return g[e].weight; });
///
///     // Using a custom weight function
///     auto result = dijkstra_shortest_paths(g, 0,
///         [](auto e) { return 1.0; });  // Unit weights
/// @endcode
///
template<typename G, typename WeightFunc>
    requires VertexListGraph<G> && IncidenceGraph<G> && WeightAccessor<WeightFunc, G>
auto dijkstra_shortest_paths(
    const G& g, 
    vertex_descriptor_t<G> source,
    WeightFunc&& get_weight
) {
    using weight_type = std::invoke_result_t<WeightFunc, edge_descriptor_t<G>>;
    using distance_type = std::conditional_t<
        std::is_floating_point_v<weight_type>,
        weight_type,
        double
    >;
    
    auto result = make_dijkstra_result<distance_type>(g, source);
    detail::dijkstra_impl(g, source, result, std::forward<WeightFunc>(get_weight));
    return result;
}

/// Compute shortest paths with explicit distance type specification.
///
/// Use this overload when you need precise control over the distance type,
/// for example when using integer weights and wanting integer distances.
///
/// @tparam DistanceType The type to use for distance calculations
/// @param g The graph
/// @param source The source vertex  
/// @param get_weight Function to extract weight from an edge
/// @return dijkstra_result<G, DistanceType> containing distances and predecessors
///
/// Example:
/// @code
///     // Use integer distances for integer weights
///     auto result = dijkstra_shortest_paths_as<int>(g, 0,
///         [&g](auto e) { return g[e].cost; });  // cost is int
/// @endcode
///
template<typename DistanceType, typename G, typename WeightFunc>
    requires VertexListGraph<G> && IncidenceGraph<G> && WeightAccessor<WeightFunc, G>
auto dijkstra_shortest_paths_as(
    const G& g,
    vertex_descriptor_t<G> source,
    WeightFunc&& get_weight
) {
    auto result = make_dijkstra_result<DistanceType>(g, source);
    detail::dijkstra_impl(g, source, result, std::forward<WeightFunc>(get_weight));
    return result;
}

} // namespace bgl

#endif // BGL_MODERN_DIJKSTRA_SHORTEST_PATHS_HPP
