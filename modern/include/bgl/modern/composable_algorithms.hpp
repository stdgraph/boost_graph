// BGL Modern - Composable Graph Algorithms
// C++20 ranges-based algorithm composition
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_COMPOSABLE_ALGORITHMS_HPP
#define BGL_MODERN_COMPOSABLE_ALGORITHMS_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>
#include <bgl/modern/range_functions.hpp>
#include <bgl/modern/connected_components.hpp>
#include <bgl/modern/breadth_first_search.hpp>

#include <ranges>
#include <vector>
#include <algorithm>
#include <functional>
#include <concepts>
#include <map>

namespace bgl {

// =============================================================================
// Graph Range Adaptors
// =============================================================================

/// Range adaptor that filters vertices by a predicate
///
/// Usage:
/// @code
///     for (auto v : vertices(g) | filter_vertices([](auto v) { return v % 2 == 0; })) {
///         // Process even-numbered vertices
///     }
/// @endcode
///
namespace views {

template<typename Predicate>
struct filter_vertices_fn {
    Predicate pred;
    
    template<std::ranges::input_range R>
    auto operator()(R&& range) const {
        return std::views::filter(std::forward<R>(range), pred);
    }
};

/// Filter vertices by predicate
template<typename Predicate>
auto filter_vertices(Predicate&& pred) {
    return filter_vertices_fn<std::decay_t<Predicate>>{
        std::forward<Predicate>(pred)
    };
}

/// Range adaptor for piping syntax
template<std::ranges::input_range R, typename Predicate>
auto operator|(R&& range, filter_vertices_fn<Predicate> fn) {
    return fn(std::forward<R>(range));
}

} // namespace views

// =============================================================================
// Component-Based Operations
// =============================================================================

/// Result of component analysis that can be composed
template<typename G>
class component_view {
public:
    using graph_type = G;
    using vertex_descriptor = vertex_descriptor_t<G>;
    using component_type = std::size_t;
    
    component_view(const G& g, component_result<G> result)
        : graph_(g), result_(std::move(result))
    {}
    
    const G& graph() const { return graph_; }
    const component_result<G>& components() const { return result_; }
    
    /// Get range of all component IDs
    auto component_ids() const {
        return std::views::iota(std::size_t{0}, result_.num_components());
    }
    
    /// Get vertices in a specific component
    auto vertices_in(component_type comp) const {
        return vertices(graph_)
            | std::views::filter([this, comp](vertex_descriptor v) {
                return result_.component_of(v) == comp;
              });
    }
    
    /// Get component sizes
    auto component_sizes() const {
        std::vector<std::size_t> sizes(result_.num_components(), 0);
        for (auto v : vertices(graph_)) {
            sizes[result_.component_of(v)]++;
        }
        return sizes;
    }
    
private:
    const G& graph_;
    component_result<G> result_;
};

/// Compute connected components and return composable view
///
/// Usage:
/// @code
///     auto components = find_components(g);
///     for (auto comp_id : components.component_ids()) {
///         std::cout << "Component " << comp_id << " has "
///                   << std::ranges::distance(components.vertices_in(comp_id))
///                   << " vertices\n";
///     }
/// @endcode
///
template<Graph G>
auto find_components(const G& g) {
    return component_view<G>(g, connected_components(g));
}

// =============================================================================
// BFS Distance-Based Operations
// =============================================================================

/// Result of BFS that can be composed
template<typename G>
class bfs_view {
public:
    using graph_type = G;
    using vertex_descriptor = vertex_descriptor_t<G>;
    using distance_type = std::size_t;
    
    bfs_view(const G& g, bfs_result<G> result)
        : graph_(g), result_(std::move(result))
    {}
    
    const G& graph() const { return graph_; }
    const bfs_result<G>& bfs_result_data() const { return result_; }
    
    /// Get vertices reachable from source
    auto reachable_vertices() const {
        return vertices(graph_)
            | std::views::filter([this](vertex_descriptor v) {
                return result_.is_reachable(v);
              });
    }
    
    /// Get vertices at a specific distance
    auto vertices_at_distance(distance_type dist) const {
        return vertices(graph_)
            | std::views::filter([this, dist](vertex_descriptor v) {
                return result_.distance_to(v) == dist;
              });
    }
    
    /// Get vertices within a distance range
    auto vertices_within_distance(distance_type max_dist) const {
        return vertices(graph_)
            | std::views::filter([this, max_dist](vertex_descriptor v) {
                return result_.is_reachable(v) && result_.distance_to(v) <= max_dist;
              });
    }
    
    /// Get maximum distance (eccentricity from source)
    distance_type max_distance() const {
        distance_type max_d = 0;
        for (auto v : reachable_vertices()) {
            max_d = std::max(max_d, result_.distance_to(v));
        }
        return max_d;
    }
    
private:
    const G& graph_;
    bfs_result<G> result_;
};

/// Compute BFS and return composable view
///
/// Usage:
/// @code
///     auto bfs = find_distances_from(g, source);
///     for (auto v : bfs.vertices_within_distance(2)) {
///         // Vertices within 2 hops
///     }
/// @endcode
///
template<Graph G>
auto find_distances_from(const G& g, vertex_descriptor_t<G> source) {
    return bfs_view<G>(g, breadth_first_search(g, source));
}

// =============================================================================
// Composable Filters
// =============================================================================

namespace filters {

/// Filter components by size
struct by_size {
    std::size_t min_size;
    std::size_t max_size = std::numeric_limits<std::size_t>::max();
    
    template<typename G>
    auto operator()(const component_view<G>& view) const {
        // Capture sizes by value to avoid dangling reference
        auto sizes = view.component_sizes();
        return view.component_ids()
            | std::views::filter([sizes, min = min_size, max = max_size](std::size_t comp_id) {
                return sizes[comp_id] >= min && sizes[comp_id] <= max;
              });
    }
};

/// Filter components keeping only large ones
inline auto large_components(std::size_t min_size) {
    return by_size{min_size};
}

/// Filter components keeping only small ones
inline auto small_components(std::size_t max_size) {
    return by_size{0, max_size};
}

/// Filter to keep only the largest component
template<typename G>
auto largest_component(const component_view<G>& view) {
    auto sizes = view.component_sizes();
    auto max_it = std::max_element(sizes.begin(), sizes.end());
    
    std::vector<std::size_t> result;
    if (max_it != sizes.end()) {
        std::size_t largest = std::distance(sizes.begin(), max_it);
        result.push_back(largest);
    }
    return result;
}

} // namespace filters

// =============================================================================
// Composable Transformations
// =============================================================================

namespace transforms {

/// Transform range of component IDs to ranges of vertices
template<typename G>
struct to_vertices_fn {
    const component_view<G>& view;
    
    auto operator()(std::size_t comp_id) const {
        return view.vertices_in(comp_id);
    }
};

template<typename G>
auto to_vertices(const component_view<G>& view) {
    return to_vertices_fn<G>{view};
}

/// Collect vertices from multiple components
template<typename G, std::ranges::input_range R>
    requires std::same_as<std::ranges::range_value_t<R>, std::size_t>
auto collect_vertices(const component_view<G>& view, R&& comp_ids) {
    std::vector<vertex_descriptor_t<G>> result;
    for (auto comp_id : comp_ids) {
        for (auto v : view.vertices_in(comp_id)) {
            result.push_back(v);
        }
    }
    return result;
}

} // namespace transforms

// =============================================================================
// Pipeline Utilities
// =============================================================================

/// Pipeline helper for chaining operations
template<typename T>
class pipeline {
public:
    explicit pipeline(T value) : value_(std::move(value)) {}
    
    const T& get() const { return value_; }
    T& get() { return value_; }
    
    // Allow piping with range adaptors
    template<typename F>
    auto operator|(F&& fn) const & {
        return pipeline{fn(value_)};
    }
    
    template<typename F>
    auto operator|(F&& fn) && {
        return pipeline{fn(std::move(value_))};
    }
    
private:
    T value_;
};

/// Create a pipeline from a value
template<typename T>
auto make_pipeline(T&& value) {
    return pipeline<std::decay_t<T>>{std::forward<T>(value)};
}

// =============================================================================
// High-Level Composable Algorithms
// =============================================================================

/// Find k-core of graph (vertices with degree >= k)
///
/// Usage:
/// @code
///     auto core = find_k_core(g, 3);
///     std::cout << "Vertices in 3-core: " << std::ranges::distance(core) << "\n";
/// @endcode
///
template<Graph G>
auto find_k_core(const G& g, std::size_t k) {
    return vertices(g)
        | std::views::filter([&g, k](auto v) {
            return out_degree(v, g) >= k;
          });
}

/// Find neighbors of a vertex
///
/// Usage:
/// @code
///     for (auto neighbor : find_neighbors(g, v)) {
///         // Process each neighbor
///     }
/// @endcode
///
template<Graph G>
auto find_neighbors(const G& g, vertex_descriptor_t<G> v) {
    return out_edges(v, g)
        | std::views::transform([&g](auto e) { return target(e, g); });
}

/// Find common neighbors of two vertices
///
template<Graph G>
auto find_common_neighbors(const G& g, vertex_descriptor_t<G> u, vertex_descriptor_t<G> v) {
    std::vector<vertex_descriptor_t<G>> u_neighbors;
    for (auto n : find_neighbors(g, u)) {
        u_neighbors.push_back(n);
    }
    std::ranges::sort(u_neighbors);
    
    return find_neighbors(g, v)
        | std::views::filter([&u_neighbors](auto w) {
            return std::ranges::binary_search(u_neighbors, w);
          });
}

/// Count vertices satisfying a predicate (composable)
///
template<Graph G, typename Predicate>
std::size_t count_vertices_if(const G& g, Predicate&& pred) {
    return static_cast<std::size_t>(std::ranges::count_if(vertices(g), std::forward<Predicate>(pred)));
}

/// Find degree distribution
///
/// Returns a map from degree to count of vertices with that degree
///
template<Graph G>
auto degree_distribution(const G& g) {
    std::map<std::size_t, std::size_t> dist;
    for (auto v : vertices(g)) {
        dist[out_degree(v, g)]++;
    }
    return dist;
}

} // namespace bgl

#endif // BGL_MODERN_COMPOSABLE_ALGORITHMS_HPP
