// BGL Modern - Breadth-First Search Algorithm
// C++20 implementation returning structured results
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_BREADTH_FIRST_SEARCH_HPP
#define BGL_MODERN_BREADTH_FIRST_SEARCH_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>

#include <vector>
#include <queue>
#include <limits>
#include <optional>
#include <algorithm>
#include <cstdint>

namespace bgl {

// =============================================================================
// Vertex Colors for BFS
// =============================================================================

enum class vertex_color : std::uint8_t {
    white = 0,  // Undiscovered
    gray  = 1,  // Discovered, in queue
    black = 2   // Finished (all neighbors examined)
};

// =============================================================================
// bfs_result
// =============================================================================

/// Result type for breadth-first search algorithm.
///
/// Contains discovery order, distances from source, predecessors, and
/// provides convenient accessors for querying BFS tree information.
///
/// Usage:
/// @code
///     auto result = breadth_first_search(g, source);
///     
///     // Get BFS distance (number of edges) to a vertex
///     auto dist = result.distance_to(target);
///     
///     // Get vertices in discovery order
///     for (auto v : result.discovered_vertices()) {
///         std::cout << v << " ";
///     }
///     
///     // Reconstruct path
///     auto path = result.path_to(target);
/// @endcode
///
template<typename G>
class bfs_result {
public:
    using graph_type = G;
    using vertex_descriptor = vertex_descriptor_t<G>;
    using size_type = std::size_t;
    using distance_type = size_type;
    
    static constexpr distance_type infinity() {
        return std::numeric_limits<distance_type>::max();
    }
    
    static constexpr vertex_descriptor null_vertex() {
        if constexpr (std::is_integral_v<vertex_descriptor>) {
            return static_cast<vertex_descriptor>(-1);
        } else {
            return vertex_descriptor{};
        }
    }
    
    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------
    
    bfs_result() = default;
    
    explicit bfs_result(size_type n)
        : distances_(n, infinity())
        , predecessors_(n, null_vertex())
        , colors_(n, vertex_color::white)
        , source_(null_vertex())
    {
        discovery_order_.reserve(n);
    }
    
    bfs_result(size_type n, vertex_descriptor source)
        : bfs_result(n)
    {
        set_source(source);
    }
    
    // -------------------------------------------------------------------------
    // Property Map Access (for algorithm use)
    // -------------------------------------------------------------------------
    
    auto distance_map() {
        return [this](vertex_descriptor v) -> distance_type& {
            return distances_[v];
        };
    }
    
    auto distance_map() const {
        return [this](vertex_descriptor v) -> const distance_type& {
            return distances_[v];
        };
    }
    
    auto predecessor_map() {
        return [this](vertex_descriptor v) -> vertex_descriptor& {
            return predecessors_[v];
        };
    }
    
    auto predecessor_map() const {
        return [this](vertex_descriptor v) -> const vertex_descriptor& {
            return predecessors_[v];
        };
    }
    
    auto color_map() {
        return [this](vertex_descriptor v) -> vertex_color& {
            return colors_[v];
        };
    }
    
    auto color_map() const {
        return [this](vertex_descriptor v) -> const vertex_color& {
            return colors_[v];
        };
    }
    
    // -------------------------------------------------------------------------
    // Discovery Tracking
    // -------------------------------------------------------------------------
    
    void discover_vertex(vertex_descriptor v) {
        discovery_order_.push_back(v);
    }
    
    // -------------------------------------------------------------------------
    // Query Interface
    // -------------------------------------------------------------------------
    
    vertex_descriptor source() const { return source_; }
    
    void set_source(vertex_descriptor s) {
        source_ = s;
        if (s < distances_.size()) {
            distances_[s] = 0;
            predecessors_[s] = s;
            colors_[s] = vertex_color::gray;
        }
    }
    
    /// Get the BFS distance (number of edges) to a vertex
    distance_type distance_to(vertex_descriptor v) const {
        return distances_[v];
    }
    
    /// Get the predecessor of a vertex in the BFS tree
    vertex_descriptor predecessor_of(vertex_descriptor v) const {
        return predecessors_[v];
    }
    
    /// Get the color of a vertex
    vertex_color color_of(vertex_descriptor v) const {
        return colors_[v];
    }
    
    /// Check if a vertex was discovered (reachable from source)
    bool is_reachable(vertex_descriptor v) const {
        return distances_[v] != infinity();
    }
    
    /// Check if a vertex is the source
    bool is_source(vertex_descriptor v) const {
        return v == source_;
    }
    
    /// Get vertices in discovery order
    const std::vector<vertex_descriptor>& discovered_vertices() const {
        return discovery_order_;
    }
    
    /// Reconstruct path from source to target
    std::vector<vertex_descriptor> path_to(vertex_descriptor target) const {
        std::vector<vertex_descriptor> path;
        
        if (!is_reachable(target)) {
            return path;
        }
        
        vertex_descriptor current = target;
        while (current != source_) {
            path.push_back(current);
            vertex_descriptor pred = predecessors_[current];
            if (pred == current || pred == null_vertex()) {
                break;
            }
            current = pred;
        }
        path.push_back(source_);
        
        std::ranges::reverse(path);
        return path;
    }
    
    /// Get number of edges in shortest path to target
    std::optional<size_type> path_length(vertex_descriptor target) const {
        if (!is_reachable(target)) {
            return std::nullopt;
        }
        return distances_[target];
    }
    
    // -------------------------------------------------------------------------
    // Raw Data Access
    // -------------------------------------------------------------------------
    
    const std::vector<distance_type>& distances() const { return distances_; }
    const std::vector<vertex_descriptor>& predecessors() const { return predecessors_; }
    const std::vector<vertex_color>& colors() const { return colors_; }
    
private:
    std::vector<distance_type> distances_;
    std::vector<vertex_descriptor> predecessors_;
    std::vector<vertex_color> colors_;
    std::vector<vertex_descriptor> discovery_order_;
    vertex_descriptor source_{null_vertex()};
};

// =============================================================================
// Factory Functions
// =============================================================================

template<typename G>
auto make_bfs_result(const G& g) {
    return bfs_result<G>(num_vertices(g));
}

template<typename G>
auto make_bfs_result(const G& g, vertex_descriptor_t<G> source) {
    return bfs_result<G>(num_vertices(g), source);
}

// =============================================================================
// breadth_first_search - Core Implementation
// =============================================================================

namespace detail {

template<typename G>
void bfs_impl(
    const G& g,
    vertex_descriptor_t<G> start,
    bfs_result<G>& result
) {
    using vertex_descriptor = vertex_descriptor_t<G>;
    
    std::queue<vertex_descriptor> queue;
    
    auto dist = result.distance_map();
    auto pred = result.predecessor_map();
    auto color = result.color_map();
    
    result.set_source(start);
    result.discover_vertex(start);
    queue.push(start);
    
    while (!queue.empty()) {
        vertex_descriptor u = queue.front();
        queue.pop();
        
        for (auto e : out_edges(u, g)) {
            vertex_descriptor v = target(e, g);
            
            if (color(v) == vertex_color::white) {
                // Discovered a new vertex
                color(v) = vertex_color::gray;
                dist(v) = dist(u) + 1;
                pred(v) = u;
                result.discover_vertex(v);
                queue.push(v);
            }
        }
        
        color(u) = vertex_color::black;
    }
}

} // namespace detail

// =============================================================================
// breadth_first_search - Public Interface
// =============================================================================

/// Perform breadth-first search from a source vertex.
///
/// Returns a bfs_result containing:
/// - Discovery order of vertices
/// - Distances (number of edges) from source
/// - Predecessor tree for path reconstruction
/// - Color map showing vertex states
///
/// Requirements:
/// - G must satisfy VertexListGraph and IncidenceGraph concepts
///
/// Complexity: O(V + E)
///
/// @param g The graph
/// @param source The source vertex
/// @return bfs_result containing BFS tree information
///
/// Example:
/// @code
///     auto result = breadth_first_search(g, 0);
///     
///     // Print vertices in BFS order
///     for (auto v : result.discovered_vertices()) {
///         std::cout << v << " at distance " << result.distance_to(v) << "\n";
///     }
/// @endcode
///
template<typename G>
    requires VertexListGraph<G> && IncidenceGraph<G>
auto breadth_first_search(const G& g, vertex_descriptor_t<G> source) {
    auto result = make_bfs_result(g, source);
    detail::bfs_impl(g, source, result);
    return result;
}

} // namespace bgl

#endif // BGL_MODERN_BREADTH_FIRST_SEARCH_HPP
