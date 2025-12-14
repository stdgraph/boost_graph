// BGL Modern - Topological Sort Algorithm
// C++20 implementation of topological sort
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_TOPOLOGICAL_SORT_HPP
#define BGL_MODERN_TOPOLOGICAL_SORT_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>

#include <vector>
#include <ranges>
#include <concepts>
#include <algorithm>
#include <stdexcept>
#include <optional>

namespace bgl {

// =============================================================================
// topological_sort_result - Result Type
// =============================================================================

/// Result type for topological sort algorithm.
///
/// This struct holds the sorted vertex order and cycle detection information.
///
/// Usage:
/// @code
///     auto result = topological_sort(g);
///     
///     if (result.has_cycle()) {
///         std::cout << "Graph has a cycle!\n";
///     } else {
///         for (auto v : result.sorted_vertices()) {
///             std::cout << v << " ";
///         }
///     }
/// @endcode
///
template<typename G>
class topological_sort_result {
public:
    using graph_type = G;
    using vertex_descriptor = vertex_descriptor_t<G>;
    using size_type = std::size_t;
    
    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------
    
    topological_sort_result() = default;
    
    explicit topological_sort_result(size_type num_vertices) {
        sorted_vertices_.reserve(num_vertices);
    }
    
    // -------------------------------------------------------------------------
    // Building the Result (for algorithm use)
    // -------------------------------------------------------------------------
    
    void add_vertex(vertex_descriptor v) {
        sorted_vertices_.push_back(v);
    }
    
    void set_has_cycle(bool has_cycle) {
        has_cycle_ = has_cycle;
    }
    
    void reverse_order() {
        std::ranges::reverse(sorted_vertices_);
    }
    
    // -------------------------------------------------------------------------
    // Query Interface
    // -------------------------------------------------------------------------
    
    /// Get vertices in topological order
    const std::vector<vertex_descriptor>& sorted_vertices() const {
        return sorted_vertices_;
    }
    
    /// Get a range of sorted vertices
    auto sorted_range() const {
        return std::ranges::subrange(sorted_vertices_.begin(), sorted_vertices_.end());
    }
    
    /// Check if a cycle was detected (no valid topological order exists)
    bool has_cycle() const {
        return has_cycle_;
    }
    
    /// Check if a valid topological order was found
    bool is_valid() const {
        return !has_cycle_;
    }
    
    /// Get the number of vertices in the sorted order
    size_type size() const {
        return sorted_vertices_.size();
    }
    
    /// Get vertex at position i in topological order
    vertex_descriptor operator[](size_type i) const {
        return sorted_vertices_[i];
    }
    
    /// Iterator access
    auto begin() const { return sorted_vertices_.begin(); }
    auto end() const { return sorted_vertices_.end(); }
    
private:
    std::vector<vertex_descriptor> sorted_vertices_;
    bool has_cycle_ = false;
};

// =============================================================================
// Factory Functions
// =============================================================================

template<typename G>
auto make_topological_sort_result(const G& g) {
    return topological_sort_result<G>(num_vertices(g));
}

// =============================================================================
// topological_sort - Core Implementation
// =============================================================================

namespace detail {

/// Vertex color for DFS
enum class topo_color { white, gray, black };

/// DFS visit for topological sort
template<typename G>
bool topo_dfs_visit(
    const G& g,
    vertex_descriptor_t<G> u,
    std::vector<topo_color>& colors,
    topological_sort_result<G>& result
) {
    using vertex_descriptor = vertex_descriptor_t<G>;
    
    colors[u] = topo_color::gray;
    
    for (auto e : out_edges(u, g)) {
        vertex_descriptor v = target(e, g);
        
        if (colors[v] == topo_color::gray) {
            // Back edge detected - cycle exists
            return false;
        }
        
        if (colors[v] == topo_color::white) {
            if (!topo_dfs_visit(g, v, colors, result)) {
                return false;  // Cycle found in subtree
            }
        }
    }
    
    colors[u] = topo_color::black;
    result.add_vertex(u);  // Add to result in reverse finish order
    return true;
}

/// Core topological sort implementation using DFS
template<typename G>
void topological_sort_impl(const G& g, topological_sort_result<G>& result) {
    std::vector<topo_color> colors(num_vertices(g), topo_color::white);
    
    bool success = true;
    for (auto v : vertices(g)) {
        if (colors[v] == topo_color::white) {
            if (!topo_dfs_visit(g, v, colors, result)) {
                success = false;
                result.set_has_cycle(true);
                break;
            }
        }
    }
    
    if (success) {
        result.reverse_order();  // Reverse to get topological order
    }
}

} // namespace detail

// =============================================================================
// topological_sort - Public Interface
// =============================================================================

/// Compute topological sort of a directed acyclic graph (DAG).
///
/// Returns vertices in topological order such that for every directed edge
/// (u, v), vertex u comes before vertex v in the ordering.
///
/// If the graph contains a cycle, the result will have has_cycle() == true
/// and the sorted_vertices() will be incomplete.
///
/// Requirements:
/// - G must satisfy VertexListGraph and IncidenceGraph concepts
/// - Graph should be directed
///
/// Complexity: O(V + E)
///
/// @param g The graph
/// @return topological_sort_result containing sorted vertices
///
/// Example:
/// @code
///     auto result = topological_sort(g);
///     
///     if (result.has_cycle()) {
///         std::cerr << "Error: Graph has a cycle!\n";
///     } else {
///         std::cout << "Topological order: ";
///         for (auto v : result) {
///             std::cout << v << " ";
///         }
///         std::cout << "\n";
///     }
/// @endcode
///
template<typename G>
    requires VertexListGraph<G> && IncidenceGraph<G>
auto topological_sort(const G& g) {
    auto result = make_topological_sort_result(g);
    detail::topological_sort_impl(g, result);
    return result;
}

/// Exception thrown when topological_sort_checked encounters a cycle
class not_a_dag : public std::runtime_error {
public:
    not_a_dag() : std::runtime_error("Graph contains a cycle; not a DAG") {}
};

/// Compute topological sort, throwing if cycle detected.
///
/// Same as topological_sort() but throws not_a_dag if the graph
/// contains a cycle instead of returning an invalid result.
///
/// @throws not_a_dag if the graph contains a cycle
///
template<typename G>
    requires VertexListGraph<G> && IncidenceGraph<G>
auto topological_sort_checked(const G& g) {
    auto result = topological_sort(g);
    if (result.has_cycle()) {
        throw not_a_dag();
    }
    return result;
}

} // namespace bgl

#endif // BGL_MODERN_TOPOLOGICAL_SORT_HPP
