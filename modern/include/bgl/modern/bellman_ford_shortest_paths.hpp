// BGL Modern - Bellman-Ford Shortest Paths Algorithm
// C++20 implementation of the Bellman-Ford algorithm
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_BELLMAN_FORD_SHORTEST_PATHS_HPP
#define BGL_MODERN_BELLMAN_FORD_SHORTEST_PATHS_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>
#include <bgl/modern/property_map.hpp>
#include <bgl/modern/algorithm_result.hpp>
#include <bgl/modern/algorithm_params.hpp>
#include <bgl/modern/dijkstra_shortest_paths.hpp>  // For WeightAccessor, make_edge_weight_accessor

#include <vector>
#include <concepts>
#include <type_traits>
#include <optional>

namespace bgl {

// =============================================================================
// bellman_ford_result
// =============================================================================

/// Result type for Bellman-Ford shortest paths algorithm.
///
/// Extends the basic shortest path result with negative cycle detection.
/// If a negative cycle is reachable from the source, has_negative_cycle()
/// returns true and distance/predecessor values may be invalid.
///
/// Usage:
/// @code
///     auto result = bellman_ford_shortest_paths(g, source, get_weight);
///     
///     if (result.has_negative_cycle()) {
///         std::cerr << "Graph contains negative cycle!\n";
///     } else {
///         double dist = result.distance_to(target);
///     }
/// @endcode
///
/// @tparam G The graph type
/// @tparam DistanceType The type used for distances (default: double)
///
template<typename G, typename DistanceType = double>
class bellman_ford_result : public dijkstra_result<G, DistanceType> {
public:
    using base = dijkstra_result<G, DistanceType>;
    using vertex_descriptor = typename base::vertex_descriptor;
    using distance_type = typename base::distance_type;
    using size_type = typename base::size_type;
    
    // Inherit constructors
    using base::base;
    
    // -------------------------------------------------------------------------
    // Negative Cycle Detection
    // -------------------------------------------------------------------------
    
    /// Check if a negative cycle was detected
    bool has_negative_cycle() const { return has_negative_cycle_; }
    
    /// Mark that a negative cycle was detected
    void set_negative_cycle(bool has_cycle) { has_negative_cycle_ = has_cycle; }
    
    /// Get the edges involved in the negative cycle (if detected)
    /// Returns empty if no negative cycle or cycle not traced
    const std::vector<vertex_descriptor>& negative_cycle_vertices() const {
        return negative_cycle_vertices_;
    }
    
    /// Record a vertex involved in the negative cycle
    void add_negative_cycle_vertex(vertex_descriptor v) {
        negative_cycle_vertices_.push_back(v);
    }
    
private:
    bool has_negative_cycle_ = false;
    std::vector<vertex_descriptor> negative_cycle_vertices_;
};

// =============================================================================
// Factory Functions
// =============================================================================

/// Create a bellman_ford_result sized for the given graph
template<typename DistanceType = double, typename G>
auto make_bellman_ford_result(const G& g) {
    return bellman_ford_result<G, DistanceType>(num_vertices(g));
}

/// Create a bellman_ford_result sized for the given graph with source vertex
template<typename DistanceType = double, typename G>
auto make_bellman_ford_result(const G& g, vertex_descriptor_t<G> source) {
    return bellman_ford_result<G, DistanceType>(num_vertices(g), source);
}

// =============================================================================
// bellman_ford_shortest_paths - Core Implementation
// =============================================================================

namespace detail {

/// Core Bellman-Ford implementation
/// Returns true if no negative cycle is detected, false otherwise
template<typename G, typename DistanceType, typename WeightFunc>
bool bellman_ford_impl(
    const G& g,
    vertex_descriptor_t<G> start,
    bellman_ford_result<G, DistanceType>& result,
    WeightFunc&& get_weight
) {
    using vertex_descriptor = vertex_descriptor_t<G>;
    using distance_type = DistanceType;
    
    const auto n = num_vertices(g);
    if (n == 0) {
        return true;  // Empty graph has no negative cycles
    }
    
    // Get property maps from result
    auto dist = result.distance_map();
    auto pred = result.predecessor_map();
    
    // Initialize source
    result.set_source(start);
    
    // Relax all edges |V| - 1 times
    for (std::size_t i = 0; i < n - 1; ++i) {
        bool changed = false;
        
        // Iterate over all edges
        for (auto e : edges(g)) {
            vertex_descriptor u = source(e, g);
            vertex_descriptor v = target(e, g);
            
            if (dist(u) == result.infinity()) {
                continue;  // Source vertex not yet reached
            }
            
            distance_type weight = static_cast<distance_type>(get_weight(e));
            distance_type new_dist = dist(u) + weight;
            
            if (new_dist < dist(v)) {
                dist(v) = new_dist;
                pred(v) = u;
                changed = true;
            }
        }
        
        // Early termination if no changes
        if (!changed) {
            return true;
        }
    }
    
    // Check for negative cycles (|V|th iteration)
    for (auto e : edges(g)) {
        vertex_descriptor u = source(e, g);
        vertex_descriptor v = target(e, g);
        
        if (dist(u) == result.infinity()) {
            continue;
        }
        
        distance_type weight = static_cast<distance_type>(get_weight(e));
        distance_type new_dist = dist(u) + weight;
        
        if (new_dist < dist(v)) {
            // Negative cycle detected!
            result.set_negative_cycle(true);
            result.add_negative_cycle_vertex(v);
            return false;
        }
    }
    
    return true;  // No negative cycle
}

} // namespace detail

// =============================================================================
// bellman_ford_shortest_paths - Public Interface
// =============================================================================

/// Compute shortest paths from a source vertex using Bellman-Ford algorithm.
///
/// Unlike Dijkstra's algorithm, Bellman-Ford can handle negative edge weights
/// and will detect negative cycles. However, it has O(V*E) complexity.
///
/// This overload uses the default edge weight accessor (unit weights if 
/// bundled properties not available).
///
/// Requirements:
/// - G must satisfy VertexListGraph and EdgeListGraph concepts
/// - G must satisfy IncidenceGraph for source/target functions
///
/// Complexity: O(V * E)
///
/// @param g The graph
/// @param source The source vertex
/// @return bellman_ford_result containing distances, predecessors, and cycle info
///
/// Example:
/// @code
///     auto result = bellman_ford_shortest_paths(g, 0);
///     if (!result.has_negative_cycle()) {
///         std::cout << "Distance to vertex 5: " << result.distance_to(5) << "\n";
///     }
/// @endcode
///
template<typename G>
    requires VertexListGraph<G> && EdgeListGraph<G> && IncidenceGraph<G>
auto bellman_ford_shortest_paths(const G& g, vertex_descriptor_t<G> source) {
    auto result = make_bellman_ford_result<double>(g, source);
    detail::bellman_ford_impl(g, source, result, make_edge_weight_accessor(g));
    return result;
}

/// Compute shortest paths using Bellman-Ford with a custom weight accessor.
///
/// This overload allows specifying a custom function to extract edge weights,
/// which is useful for graphs with complex edge properties or computed weights.
///
/// Requirements:
/// - G must satisfy VertexListGraph, EdgeListGraph, and IncidenceGraph concepts
/// - WeightFunc must be invocable with edge_descriptor and return a numeric type
/// - Negative weights are allowed
///
/// Complexity: O(V * E)
///
/// @param g The graph
/// @param source The source vertex
/// @param get_weight Function to extract weight from an edge
/// @return bellman_ford_result containing distances, predecessors, and cycle info
///
/// Example:
/// @code
///     auto result = bellman_ford_shortest_paths(g, 0, 
///         [&g](auto e) { return g[e].cost; });  // cost may be negative
///     
///     if (result.has_negative_cycle()) {
///         std::cerr << "Warning: negative cycle detected\n";
///     }
/// @endcode
///
template<typename G, typename WeightFunc>
    requires VertexListGraph<G> && EdgeListGraph<G> && IncidenceGraph<G> && 
             WeightAccessor<WeightFunc, G>
auto bellman_ford_shortest_paths(
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
    
    auto result = make_bellman_ford_result<distance_type>(g, source);
    detail::bellman_ford_impl(g, source, result, std::forward<WeightFunc>(get_weight));
    return result;
}

/// Compute shortest paths with explicit distance type specification.
///
/// Use this overload when you need precise control over the distance type.
///
/// @tparam DistanceType The type to use for distance calculations
/// @param g The graph
/// @param source The source vertex
/// @param get_weight Function to extract weight from an edge
/// @return bellman_ford_result<G, DistanceType> with distances and cycle info
///
/// Example:
/// @code
///     auto result = bellman_ford_shortest_paths_as<int>(g, 0,
///         [&g](auto e) { return g[e].cost; });
/// @endcode
///
template<typename DistanceType, typename G, typename WeightFunc>
    requires VertexListGraph<G> && EdgeListGraph<G> && IncidenceGraph<G> &&
             WeightAccessor<WeightFunc, G>
auto bellman_ford_shortest_paths_as(
    const G& g,
    vertex_descriptor_t<G> source,
    WeightFunc&& get_weight
) {
    auto result = make_bellman_ford_result<DistanceType>(g, source);
    detail::bellman_ford_impl(g, source, result, std::forward<WeightFunc>(get_weight));
    return result;
}

// =============================================================================
// bellman_ford_shortest_paths - Named Parameters Overload
// =============================================================================

/// Compute shortest paths from a source vertex using named parameters.
///
/// This overload uses C++20 designated initializers for flexible parameter
/// specification. All parameters have sensible defaults.
///
/// Requirements:
/// - G must satisfy VertexListGraph, EdgeListGraph, and IncidenceGraph concepts
///
/// Complexity: O(V * E)
///
/// @param g The graph
/// @param source The source vertex
/// @param params Named parameters (see bellman_ford_params)
/// @return bellman_ford_result containing distances, predecessors, and negative cycle info
///
/// Example:
/// @code
///     // Use all defaults (unit weights)
///     auto r1 = bellman_ford_shortest_paths(g, source, bellman_ford_params{});
///
///     // Custom weight map only
///     auto r2 = bellman_ford_shortest_paths(g, source, bellman_ford_params{
///         .weight_map = [&g](auto e) { return g[e].cost; }
///     });
/// @endcode
///
template<typename G, typename... ParamTypes>
    requires VertexListGraph<G> && EdgeListGraph<G> && IncidenceGraph<G>
auto bellman_ford_shortest_paths(
    const G& g,
    vertex_descriptor_t<G> source,
    const bellman_ford_params<ParamTypes...>& params
) {
    using edge_descriptor = edge_descriptor_t<G>;
    
    // Get the weight map
    auto weight_map = params.weight_map;
    
    // Determine weight/distance type from weight map
    using weight_type = detail::weight_type_t<decltype(weight_map), edge_descriptor>;
    using distance_type = std::conditional_t<
        std::is_floating_point_v<weight_type>,
        weight_type,
        double
    >;
    
    auto result = make_bellman_ford_result<distance_type>(g, source);
    detail::bellman_ford_impl(g, source, result, weight_map);
    return result;
}

} // namespace bgl

#endif // BGL_MODERN_BELLMAN_FORD_SHORTEST_PATHS_HPP
