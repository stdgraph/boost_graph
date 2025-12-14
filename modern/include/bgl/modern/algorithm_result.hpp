// BGL Modern - Algorithm Results
// Structured return types for graph algorithms
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_ALGORITHM_RESULT_HPP
#define BGL_MODERN_ALGORITHM_RESULT_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/graph_traits.hpp>
#include <bgl/modern/property_map.hpp>

#include <algorithm>
#include <vector>
#include <limits>
#include <optional>
#include <concepts>
#include <type_traits>

namespace bgl {

// =============================================================================
// dijkstra_result
// =============================================================================

/// Result type for Dijkstra's shortest paths algorithm.
///
/// This struct holds the computed distances and predecessors for all vertices
/// reachable from the source vertex. It provides convenient accessors for
/// querying shortest path information.
///
/// Usage:
/// @code
///     auto result = dijkstra_shortest_paths(g, source);
///     
///     // Query distance to a specific vertex
///     double dist = result.distance_to(target);
///     
///     // Check if a vertex is reachable
///     if (result.is_reachable(target)) {
///         // Reconstruct path
///         auto path = result.path_to(target);
///     }
///     
///     // Access the raw property maps
///     for (auto v : vertices(g)) {
///         std::cout << "Distance to " << v << ": " << result.distances(v) << "\n";
///     }
/// @endcode
///
/// @tparam G The graph type
/// @tparam DistanceType The type used for distances (default: double)
///
template<typename G, typename DistanceType = double>
class dijkstra_result {
public:
    using graph_type = G;
    using vertex_descriptor = vertex_descriptor_t<G>;
    using distance_type = DistanceType;
    using size_type = std::size_t;
    
    /// Sentinel value indicating unreachable vertices
    static constexpr distance_type infinity() {
        if constexpr (std::numeric_limits<distance_type>::has_infinity) {
            return std::numeric_limits<distance_type>::infinity();
        } else {
            return std::numeric_limits<distance_type>::max();
        }
    }
    
    /// Sentinel value indicating no predecessor (for source or unreachable vertices)
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
    
    dijkstra_result() = default;
    
    /// Construct with pre-allocated storage for n vertices
    explicit dijkstra_result(size_type n)
        : distances_(n, infinity())
        , predecessors_(n, null_vertex())
        , source_(null_vertex())
    {}
    
    /// Construct with pre-allocated storage and source vertex
    dijkstra_result(size_type n, vertex_descriptor source)
        : distances_(n, infinity())
        , predecessors_(n, null_vertex())
        , source_(source)
    {
        if (source < n) {
            distances_[source] = distance_type{0};
            predecessors_[source] = source;  // Source is its own predecessor
        }
    }
    
    // -------------------------------------------------------------------------
    // Property Map Access (for algorithm use)
    // -------------------------------------------------------------------------
    
    /// Get a mutable property map for distances
    /// Used by the algorithm to update distances during computation
    auto distance_map() {
        return [this](vertex_descriptor v) -> distance_type& {
            return distances_[v];
        };
    }
    
    /// Get a const property map for distances
    auto distance_map() const {
        return [this](vertex_descriptor v) -> const distance_type& {
            return distances_[v];
        };
    }
    
    /// Get a mutable property map for predecessors
    /// Used by the algorithm to record the shortest path tree
    auto predecessor_map() {
        return [this](vertex_descriptor v) -> vertex_descriptor& {
            return predecessors_[v];
        };
    }
    
    /// Get a const property map for predecessors
    auto predecessor_map() const {
        return [this](vertex_descriptor v) -> const vertex_descriptor& {
            return predecessors_[v];
        };
    }
    
    // -------------------------------------------------------------------------
    // Query Interface
    // -------------------------------------------------------------------------
    
    /// Get the source vertex
    vertex_descriptor source() const { return source_; }
    
    /// Set the source vertex
    void set_source(vertex_descriptor s) { 
        source_ = s; 
        if (s < distances_.size()) {
            distances_[s] = distance_type{0};
            predecessors_[s] = s;
        }
    }
    
    /// Get the distance to a vertex
    distance_type distance_to(vertex_descriptor v) const {
        return distances_[v];
    }
    
    /// Get the predecessor of a vertex on the shortest path
    vertex_descriptor predecessor_of(vertex_descriptor v) const {
        return predecessors_[v];
    }
    
    /// Check if a vertex is reachable from the source
    bool is_reachable(vertex_descriptor v) const {
        return distances_[v] != infinity();
    }
    
    /// Check if a vertex is the source
    bool is_source(vertex_descriptor v) const {
        return v == source_;
    }
    
    /// Reconstruct the path from source to target
    /// Returns empty vector if target is unreachable
    std::vector<vertex_descriptor> path_to(vertex_descriptor target) const {
        std::vector<vertex_descriptor> path;
        
        if (!is_reachable(target)) {
            return path;  // Empty path for unreachable vertices
        }
        
        // Trace back from target to source
        vertex_descriptor current = target;
        while (current != source_) {
            path.push_back(current);
            vertex_descriptor pred = predecessors_[current];
            if (pred == current || pred == null_vertex()) {
                // Stuck - shouldn't happen for reachable vertices
                break;
            }
            current = pred;
        }
        path.push_back(source_);
        
        // Reverse to get source -> target order
        std::ranges::reverse(path);
        return path;
    }
    
    /// Get the number of edges in the shortest path to target
    /// Returns nullopt if target is unreachable
    std::optional<size_type> path_length(vertex_descriptor target) const {
        if (!is_reachable(target)) {
            return std::nullopt;
        }
        
        size_type length = 0;
        vertex_descriptor current = target;
        while (current != source_) {
            ++length;
            vertex_descriptor pred = predecessors_[current];
            if (pred == current || pred == null_vertex()) {
                break;
            }
            current = pred;
        }
        return length;
    }
    
    // -------------------------------------------------------------------------
    // Raw Data Access
    // -------------------------------------------------------------------------
    
    /// Direct access to distances vector
    const std::vector<distance_type>& distances() const { return distances_; }
    std::vector<distance_type>& distances() { return distances_; }
    
    /// Direct access to predecessors vector
    const std::vector<vertex_descriptor>& predecessors() const { return predecessors_; }
    std::vector<vertex_descriptor>& predecessors() { return predecessors_; }
    
    /// Resize the result storage
    void resize(size_type n) {
        distances_.resize(n, infinity());
        predecessors_.resize(n, null_vertex());
    }
    
    /// Clear and resize, resetting all values
    void reset(size_type n) {
        distances_.assign(n, infinity());
        predecessors_.assign(n, null_vertex());
        source_ = null_vertex();
    }
    
    /// Reset with a new source vertex
    void reset(size_type n, vertex_descriptor source) {
        reset(n);
        set_source(source);
    }
    
private:
    std::vector<distance_type> distances_;
    std::vector<vertex_descriptor> predecessors_;
    vertex_descriptor source_{null_vertex()};
};

// =============================================================================
// Factory Functions
// =============================================================================

/// Create a dijkstra_result sized for the given graph
template<typename DistanceType = double, typename G>
auto make_dijkstra_result(const G& g) {
    return dijkstra_result<G, DistanceType>(num_vertices(g));
}

/// Create a dijkstra_result sized for the given graph with source vertex
template<typename DistanceType = double, typename G>
auto make_dijkstra_result(const G& g, vertex_descriptor_t<G> source) {
    return dijkstra_result<G, DistanceType>(num_vertices(g), source);
}

} // namespace bgl

#endif // BGL_MODERN_ALGORITHM_RESULT_HPP
