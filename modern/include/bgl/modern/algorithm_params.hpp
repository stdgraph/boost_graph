// BGL Modern - Algorithm Parameters
// Named parameters using designated initializers (C++20)
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// =============================================================================
// NAMED PARAMETERS OVERVIEW
// =============================================================================
//
// This header provides parameter structs for graph algorithms that use
// C++20 designated initializers instead of the legacy Boost.Parameter library.
//
// Example usage:
// @code
//     auto result = dijkstra_shortest_paths(g, source, dijkstra_params{
//         .weight_map = [&g](auto e) { return g[e].cost; },
//         .distance_compare = std::less<int>{},
//         .distance_combine = std::plus<int>{}
//     });
// @endcode
//
// Benefits:
// - Type-safe at compile time
// - Self-documenting parameter names
// - No Boost dependencies
// - IDE autocomplete support
// - Zero runtime overhead (all resolved at compile time)
//
// =============================================================================

#ifndef BGL_MODERN_ALGORITHM_PARAMS_HPP
#define BGL_MODERN_ALGORITHM_PARAMS_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/graph_traits.hpp>
#include <bgl/modern/property_map.hpp>

#include <concepts>
#include <functional>
#include <type_traits>
#include <limits>
#include <optional>

namespace bgl {

// =============================================================================
// Default Sentinel Types
// =============================================================================

/// Sentinel type indicating "use default" for an optional parameter
struct use_default_t {
    constexpr use_default_t() noexcept = default;
};

/// Constant to indicate using default behavior
inline constexpr use_default_t use_default{};

/// Check if a type is the use_default_t sentinel
template<typename T>
concept IsDefault = std::same_as<std::remove_cvref_t<T>, use_default_t>;

// =============================================================================
// Default Functors
// =============================================================================

/// Default weight accessor: returns 1 for all edges (unit weight)
struct default_weight_map {
    template<typename Edge>
    constexpr double operator()(Edge&&) const noexcept {
        return 1.0;
    }
};

/// Default distance compare: std::less
template<typename Distance = double>
struct default_distance_compare {
    constexpr bool operator()(Distance a, Distance b) const noexcept {
        return a < b;
    }
};

/// Default distance combine: std::plus
template<typename Distance = double>
struct default_distance_combine {
    constexpr Distance operator()(Distance a, Distance b) const noexcept {
        return a + b;
    }
};

/// Default infinity value for distances
template<typename Distance = double>
struct default_distance_infinity {
    constexpr Distance operator()() const noexcept {
        if constexpr (std::numeric_limits<Distance>::has_infinity) {
            return std::numeric_limits<Distance>::infinity();
        } else {
            return std::numeric_limits<Distance>::max();
        }
    }
};

/// Default zero value for distances
template<typename Distance = double>
struct default_distance_zero {
    constexpr Distance operator()() const noexcept {
        return Distance{0};
    }
};

/// Null visitor that does nothing
struct null_visitor {
    template<typename... Args>
    constexpr void operator()(Args&&...) const noexcept {}
};

// =============================================================================
// dijkstra_params - Named Parameters for Dijkstra's Algorithm
// =============================================================================

/// Named parameters for Dijkstra's shortest paths algorithm.
///
/// All parameters have sensible defaults, so you only need to specify
/// the ones you want to customize.
///
/// Template parameters are automatically deduced from the provided values.
///
/// Example:
/// @code
///     // Use all defaults (unit weights)
///     auto r1 = dijkstra_shortest_paths(g, source, dijkstra_params{});
///
///     // Custom weight map
///     auto r2 = dijkstra_shortest_paths(g, source, dijkstra_params{
///         .weight_map = [&g](auto e) { return g[e].weight; }
///     });
///
///     // Custom comparator for max-path instead of min-path
///     auto r3 = dijkstra_shortest_paths(g, source, dijkstra_params{
///         .weight_map = [&g](auto e) { return g[e].weight; },
///         .distance_compare = std::greater<double>{}
///     });
///
///     // With visitor callbacks
///     auto r4 = dijkstra_shortest_paths(g, source, dijkstra_params{
///         .weight_map = my_weights,
///         .visitor = my_dijkstra_visitor{}
///     });
/// @endcode
///
template<
    typename WeightMap = default_weight_map,
    typename DistanceCompare = use_default_t,
    typename DistanceCombine = use_default_t,
    typename DistanceInfinity = use_default_t,
    typename DistanceZero = use_default_t,
    typename Visitor = null_visitor
>
struct dijkstra_params {
    /// Function to get edge weight: (edge_descriptor) -> weight
    WeightMap weight_map = {};
    
    /// Comparison function for distances (default: std::less)
    DistanceCompare distance_compare = {};
    
    /// Function to combine distances (default: std::plus)
    DistanceCombine distance_combine = {};
    
    /// Function returning the infinity value for distances
    DistanceInfinity distance_infinity = {};
    
    /// Function returning the zero value for distances
    DistanceZero distance_zero = {};
    
    /// Visitor for algorithm events (discover_vertex, examine_edge, etc.)
    Visitor visitor = {};
};

// Deduction guide for dijkstra_params
template<typename... Args>
dijkstra_params(Args...) -> dijkstra_params<Args...>;

// =============================================================================
// bellman_ford_params - Named Parameters for Bellman-Ford Algorithm  
// =============================================================================

/// Named parameters for Bellman-Ford shortest paths algorithm.
///
/// Bellman-Ford handles negative edge weights (unlike Dijkstra) and can
/// detect negative cycles.
///
/// Example:
/// @code
///     auto result = bellman_ford_shortest_paths(g, source, bellman_ford_params{
///         .weight_map = [&g](auto e) { return g[e].weight; }
///     });
///     
///     if (!result.has_negative_cycle()) {
///         // Process shortest paths
///     }
/// @endcode
///
template<
    typename WeightMap = default_weight_map,
    typename DistanceCompare = use_default_t,
    typename DistanceCombine = use_default_t,
    typename DistanceInfinity = use_default_t,
    typename DistanceZero = use_default_t,
    typename Visitor = null_visitor
>
struct bellman_ford_params {
    /// Function to get edge weight: (edge_descriptor) -> weight
    WeightMap weight_map = {};
    
    /// Comparison function for distances (default: std::less)
    DistanceCompare distance_compare = {};
    
    /// Function to combine distances (default: std::plus)
    DistanceCombine distance_combine = {};
    
    /// Function returning the infinity value for distances
    DistanceInfinity distance_infinity = {};
    
    /// Function returning the zero value for distances
    DistanceZero distance_zero = {};
    
    /// Visitor for algorithm events
    Visitor visitor = {};
};

template<typename... Args>
bellman_ford_params(Args...) -> bellman_ford_params<Args...>;

// =============================================================================
// bfs_params - Named Parameters for Breadth-First Search
// =============================================================================

/// Named parameters for Breadth-First Search algorithm.
///
/// Example:
/// @code
///     auto result = breadth_first_search(g, source, bfs_params{
///         .visitor = my_bfs_visitor{}
///     });
/// @endcode
///
template<
    typename ColorMap = use_default_t,
    typename Visitor = null_visitor
>
struct bfs_params {
    /// Color map for tracking vertex state (default: internal vector)
    ColorMap color_map = {};
    
    /// Visitor for algorithm events
    Visitor visitor = {};
};

template<typename... Args>
bfs_params(Args...) -> bfs_params<Args...>;

// =============================================================================
// dfs_params - Named Parameters for Depth-First Search
// =============================================================================

/// Named parameters for Depth-First Search algorithm.
///
/// Example:
/// @code
///     auto result = depth_first_search(g, source, dfs_params{
///         .visitor = my_dfs_visitor{}
///     });
/// @endcode
///
template<
    typename ColorMap = use_default_t,
    typename Visitor = null_visitor
>
struct dfs_params {
    /// Color map for tracking vertex state (default: internal vector)
    ColorMap color_map = {};
    
    /// Visitor for algorithm events
    Visitor visitor = {};
};

template<typename... Args>
dfs_params(Args...) -> dfs_params<Args...>;

// =============================================================================
// prim_params - Named Parameters for Prim's MST Algorithm
// =============================================================================

/// Named parameters for Prim's Minimum Spanning Tree algorithm.
///
/// Example:
/// @code
///     auto mst = prim_minimum_spanning_tree(g, source, prim_params{
///         .weight_map = [&g](auto e) { return g[e].weight; }
///     });
/// @endcode
///
template<
    typename WeightMap = default_weight_map,
    typename Visitor = null_visitor
>
struct prim_params {
    /// Function to get edge weight
    WeightMap weight_map = {};
    
    /// Visitor for algorithm events
    Visitor visitor = {};
};

template<typename... Args>
prim_params(Args...) -> prim_params<Args...>;

// =============================================================================
// kruskal_params - Named Parameters for Kruskal's MST Algorithm
// =============================================================================

/// Named parameters for Kruskal's Minimum Spanning Tree algorithm.
///
/// Example:
/// @code
///     auto mst = kruskal_minimum_spanning_tree(g, kruskal_params{
///         .weight_map = [&g](auto e) { return g[e].weight; }
///     });
/// @endcode
///
template<
    typename WeightMap = default_weight_map,
    typename Visitor = null_visitor
>
struct kruskal_params {
    /// Function to get edge weight
    WeightMap weight_map = {};
    
    /// Visitor for algorithm events
    Visitor visitor = {};
};

template<typename... Args>
kruskal_params(Args...) -> kruskal_params<Args...>;

// =============================================================================
// Parameter Resolution Helpers
// =============================================================================

namespace detail {

/// Resolve a parameter: if it's use_default_t, return the default; otherwise return the value
template<typename T, typename Default>
constexpr decltype(auto) resolve_param(T&& value, Default&& default_value) {
    if constexpr (IsDefault<T>) {
        return std::forward<Default>(default_value);
    } else {
        return std::forward<T>(value);
    }
}

/// Get weight type from a weight map
template<typename WeightMap, typename EdgeDescriptor>
using weight_type_t = std::conditional_t<
    std::same_as<std::remove_cvref_t<WeightMap>, default_weight_map>,
    double,
    std::invoke_result_t<WeightMap, EdgeDescriptor>
>;

} // namespace detail

} // namespace bgl

#endif // BGL_MODERN_ALGORITHM_PARAMS_HPP
