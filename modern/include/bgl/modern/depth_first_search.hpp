// BGL Modern - Depth-First Search Algorithm
// C++20 implementation returning structured results
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_DEPTH_FIRST_SEARCH_HPP
#define BGL_MODERN_DEPTH_FIRST_SEARCH_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>
#include <bgl/modern/breadth_first_search.hpp>  // For vertex_color
#include <bgl/modern/algorithm_params.hpp>
#include <bgl/modern/visitor_callbacks.hpp>

#include <vector>
#include <stack>
#include <limits>
#include <optional>
#include <algorithm>

namespace bgl {

// =============================================================================
// Edge Classification for DFS
// =============================================================================

enum class edge_type : std::uint8_t {
    tree_edge,   // Edge to undiscovered vertex (part of DFS tree)
    back_edge,   // Edge to ancestor (indicates cycle)
    forward_edge, // Edge to descendant (in directed graphs)
    cross_edge   // Edge to non-ancestor/non-descendant
};

// =============================================================================
// dfs_result
// =============================================================================

/// Result type for depth-first search algorithm.
///
/// Contains discovery and finish times, predecessors, and provides
/// convenient accessors for querying DFS tree/forest information.
///
/// Usage:
/// @code
///     auto result = depth_first_search(g, source);
///     
///     // Get discovery and finish times
///     auto [d, f] = result.times(v);
///     
///     // Get vertices in discovery order
///     for (auto v : result.discovered_vertices()) {
///         std::cout << v << " ";
///     }
///     
///     // Check if there's a cycle (back edges exist)
///     if (result.has_cycle()) {
///         std::cout << "Graph has a cycle\n";
///     }
/// @endcode
///
template<typename G>
class dfs_result {
public:
    using graph_type = G;
    using vertex_descriptor = vertex_descriptor_t<G>;
    using size_type = std::size_t;
    using time_type = size_type;
    
    static constexpr time_type no_time() {
        return std::numeric_limits<time_type>::max();
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
    
    dfs_result() = default;
    
    explicit dfs_result(size_type n)
        : discovery_times_(n, no_time())
        , finish_times_(n, no_time())
        , predecessors_(n, null_vertex())
        , colors_(n, vertex_color::white)
    {
        discovery_order_.reserve(n);
        finish_order_.reserve(n);
    }
    
    // -------------------------------------------------------------------------
    // Property Map Access (for algorithm use)
    // -------------------------------------------------------------------------
    
    auto discovery_time_map() {
        return [this](vertex_descriptor v) -> time_type& {
            return discovery_times_[v];
        };
    }
    
    auto finish_time_map() {
        return [this](vertex_descriptor v) -> time_type& {
            return finish_times_[v];
        };
    }
    
    auto predecessor_map() {
        return [this](vertex_descriptor v) -> vertex_descriptor& {
            return predecessors_[v];
        };
    }
    
    auto color_map() {
        return [this](vertex_descriptor v) -> vertex_color& {
            return colors_[v];
        };
    }
    
    // -------------------------------------------------------------------------
    // Time Tracking
    // -------------------------------------------------------------------------
    
    void discover_vertex(vertex_descriptor v, time_type t) {
        discovery_times_[v] = t;
        discovery_order_.push_back(v);
    }
    
    void finish_vertex(vertex_descriptor v, time_type t) {
        finish_times_[v] = t;
        finish_order_.push_back(v);
    }
    
    void record_back_edge() {
        has_cycle_ = true;
    }
    
    // -------------------------------------------------------------------------
    // Query Interface
    // -------------------------------------------------------------------------
    
    /// Get discovery time of a vertex
    time_type discovery_time(vertex_descriptor v) const {
        return discovery_times_[v];
    }
    
    /// Get finish time of a vertex
    time_type finish_time(vertex_descriptor v) const {
        return finish_times_[v];
    }
    
    /// Get both discovery and finish times
    std::pair<time_type, time_type> times(vertex_descriptor v) const {
        return {discovery_times_[v], finish_times_[v]};
    }
    
    /// Get the predecessor of a vertex in the DFS tree
    vertex_descriptor predecessor_of(vertex_descriptor v) const {
        return predecessors_[v];
    }
    
    /// Get the color of a vertex
    vertex_color color_of(vertex_descriptor v) const {
        return colors_[v];
    }
    
    /// Check if a vertex was discovered
    bool is_discovered(vertex_descriptor v) const {
        return discovery_times_[v] != no_time();
    }
    
    /// Check if a vertex is finished
    bool is_finished(vertex_descriptor v) const {
        return finish_times_[v] != no_time();
    }
    
    /// Check if the graph has a cycle (back edge was found)
    bool has_cycle() const {
        return has_cycle_;
    }
    
    /// Get vertices in discovery order
    const std::vector<vertex_descriptor>& discovered_vertices() const {
        return discovery_order_;
    }
    
    /// Get vertices in finish order (useful for topological sort)
    const std::vector<vertex_descriptor>& finished_vertices() const {
        return finish_order_;
    }
    
    /// Get topological order (reverse of finish order)
    /// Only valid if !has_cycle()
    std::vector<vertex_descriptor> topological_order() const {
        auto result = finish_order_;
        std::ranges::reverse(result);
        return result;
    }
    
    /// Check if u is an ancestor of v in the DFS tree
    bool is_ancestor(vertex_descriptor u, vertex_descriptor v) const {
        return discovery_times_[u] < discovery_times_[v] &&
               finish_times_[u] > finish_times_[v];
    }
    
    /// Check if u is a descendant of v in the DFS tree
    bool is_descendant(vertex_descriptor u, vertex_descriptor v) const {
        return is_ancestor(v, u);
    }
    
    /// Reconstruct path from tree root to vertex
    std::vector<vertex_descriptor> path_to(vertex_descriptor target) const {
        std::vector<vertex_descriptor> path;
        
        if (!is_discovered(target)) {
            return path;
        }
        
        vertex_descriptor current = target;
        while (predecessors_[current] != current && 
               predecessors_[current] != null_vertex()) {
            path.push_back(current);
            current = predecessors_[current];
        }
        path.push_back(current);  // Add the root
        
        std::ranges::reverse(path);
        return path;
    }
    
    // -------------------------------------------------------------------------
    // Raw Data Access
    // -------------------------------------------------------------------------
    
    const std::vector<time_type>& discovery_times() const { return discovery_times_; }
    const std::vector<time_type>& finish_times() const { return finish_times_; }
    const std::vector<vertex_descriptor>& predecessors() const { return predecessors_; }
    const std::vector<vertex_color>& colors() const { return colors_; }
    
private:
    std::vector<time_type> discovery_times_;
    std::vector<time_type> finish_times_;
    std::vector<vertex_descriptor> predecessors_;
    std::vector<vertex_color> colors_;
    std::vector<vertex_descriptor> discovery_order_;
    std::vector<vertex_descriptor> finish_order_;
    bool has_cycle_ = false;
};

// =============================================================================
// Factory Functions
// =============================================================================

template<typename G>
auto make_dfs_result(const G& g) {
    return dfs_result<G>(num_vertices(g));
}

// =============================================================================
// depth_first_search - Core Implementation
// =============================================================================

namespace detail {

/// DFS visit with callbacks
template<typename G, typename Callbacks>
void dfs_visit_with_callbacks(
    const G& g,
    vertex_descriptor_t<G> u,
    dfs_result<G>& result,
    std::size_t& time,
    Callbacks&& callbacks
) {
    using vertex_descriptor = vertex_descriptor_t<G>;
    
    auto color = result.color_map();
    auto pred = result.predecessor_map();
    
    // Discover u
    color(u) = vertex_color::gray;
    result.discover_vertex(u, time++);
    
    // Discover vertex callback
    if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(callbacks.on_discover_vertex)>, null_callback>) {
        callbacks.on_discover_vertex(u, g);
    }
    
    // Explore edges
    for (auto e : out_edges(u, g)) {
        vertex_descriptor v = target(e, g);
        
        // Examine edge callback
        if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(callbacks.on_examine_edge)>, null_callback>) {
            callbacks.on_examine_edge(e, g);
        }
        
        if (color(v) == vertex_color::white) {
            // Tree edge callback
            if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(callbacks.on_tree_edge)>, null_callback>) {
                callbacks.on_tree_edge(e, g);
            }
            
            pred(v) = u;
            dfs_visit_with_callbacks(g, v, result, time, callbacks);
        } else if (color(v) == vertex_color::gray) {
            // Back edge (cycle detected)
            result.record_back_edge();
            
            // Back edge callback
            if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(callbacks.on_back_edge)>, null_callback>) {
                callbacks.on_back_edge(e, g);
            }
        } else {
            // Forward or cross edge (black vertex)
            if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(callbacks.on_forward_or_cross_edge)>, null_callback>) {
                callbacks.on_forward_or_cross_edge(e, g);
            }
        }
        
        // Finish edge callback
        if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(callbacks.on_finish_edge)>, null_callback>) {
            callbacks.on_finish_edge(e, g);
        }
    }
    
    // Finish u
    color(u) = vertex_color::black;
    result.finish_vertex(u, time++);
    
    // Finish vertex callback
    if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(callbacks.on_finish_vertex)>, null_callback>) {
        callbacks.on_finish_vertex(u, g);
    }
}

/// DFS implementation with callbacks (single source)
template<typename G, typename Callbacks>
void dfs_impl(
    const G& g,
    vertex_descriptor_t<G> start,
    dfs_result<G>& result,
    Callbacks&& callbacks
) {
    auto color = result.color_map();
    auto pred = result.predecessor_map();
    std::size_t time = 0;
    
    // Initialize all vertices
    for (auto v : vertices(g)) {
        if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(callbacks.on_initialize_vertex)>, null_callback>) {
            callbacks.on_initialize_vertex(v, g);
        }
    }
    
    // Start vertex callback
    if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(callbacks.on_start_vertex)>, null_callback>) {
        callbacks.on_start_vertex(start, g);
    }
    
    // Start from the specified vertex
    pred(start) = start;  // Root of tree
    dfs_visit_with_callbacks(g, start, result, time, callbacks);
}

/// DFS implementation without callbacks
template<typename G>
void dfs_impl(
    const G& g,
    vertex_descriptor_t<G> start,
    dfs_result<G>& result
) {
    dfs_impl(g, start, result, dfs_callbacks{});
}

/// Original dfs_visit without callbacks (for backward compatibility)
template<typename G>
void dfs_visit(
    const G& g,
    vertex_descriptor_t<G> u,
    dfs_result<G>& result,
    std::size_t& time
) {
    dfs_visit_with_callbacks(g, u, result, time, dfs_callbacks{});
}

/// DFS implementation with callbacks (all vertices)
template<typename G, typename Callbacks>
void dfs_impl_all(
    const G& g,
    dfs_result<G>& result,
    Callbacks&& callbacks
) {
    auto color = result.color_map();
    auto pred = result.predecessor_map();
    std::size_t time = 0;
    
    // Initialize all vertices
    for (auto v : vertices(g)) {
        if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(callbacks.on_initialize_vertex)>, null_callback>) {
            callbacks.on_initialize_vertex(v, g);
        }
    }
    
    // Visit all vertices (creates DFS forest for disconnected graphs)
    for (auto v : vertices(g)) {
        if (color(v) == vertex_color::white) {
            // Start vertex callback
            if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(callbacks.on_start_vertex)>, null_callback>) {
                callbacks.on_start_vertex(v, g);
            }
            
            pred(v) = v;  // Root of this tree
            dfs_visit_with_callbacks(g, v, result, time, callbacks);
        }
    }
}

/// DFS implementation without callbacks (all vertices)
template<typename G>
void dfs_impl_all(
    const G& g,
    dfs_result<G>& result
) {
    dfs_impl_all(g, result, dfs_callbacks{});
}

/// Multi-source DFS implementation with callbacks
template<typename G, std::ranges::input_range Sources, typename Callbacks>
    requires std::convertible_to<std::ranges::range_value_t<Sources>, vertex_descriptor_t<G>>
void dfs_impl_multi(
    const G& g,
    Sources&& sources,
    dfs_result<G>& result,
    Callbacks&& callbacks
) {
    using vertex_descriptor = vertex_descriptor_t<G>;
    
    auto color = result.color_map();
    auto pred = result.predecessor_map();
    std::size_t time = 0;
    
    // Initialize all vertices
    for (auto v : vertices(g)) {
        if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(callbacks.on_initialize_vertex)>, null_callback>) {
            callbacks.on_initialize_vertex(v, g);
        }
    }
    
    // Visit each source in order
    for (auto s : sources) {
        vertex_descriptor source = static_cast<vertex_descriptor>(s);
        if (color(source) == vertex_color::white) {
            // Start vertex callback
            if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(callbacks.on_start_vertex)>, null_callback>) {
                callbacks.on_start_vertex(source, g);
            }
            
            pred(source) = source;  // Root of this tree
            dfs_visit_with_callbacks(g, source, result, time, callbacks);
        }
    }
}

/// Multi-source DFS without callbacks
template<typename G, std::ranges::input_range Sources>
    requires std::convertible_to<std::ranges::range_value_t<Sources>, vertex_descriptor_t<G>>
void dfs_impl_multi(
    const G& g,
    Sources&& sources,
    dfs_result<G>& result
) {
    dfs_impl_multi(g, std::forward<Sources>(sources), result, dfs_callbacks{});
}

} // namespace detail

// =============================================================================
// depth_first_search - Public Interface
// =============================================================================

/// Perform depth-first search from a source vertex.
///
/// Returns a dfs_result containing:
/// - Discovery and finish times
/// - Predecessor tree for path reconstruction
/// - Cycle detection (via back edges)
/// - Vertices in discovery and finish order
///
/// Requirements:
/// - G must satisfy VertexListGraph and IncidenceGraph concepts
///
/// Complexity: O(V + E)
///
/// @param g The graph
/// @param source The source vertex
/// @return dfs_result containing DFS tree information
///
/// Example:
/// @code
///     auto result = depth_first_search(g, 0);
///     
///     if (result.has_cycle()) {
///         std::cout << "Graph contains a cycle\n";
///     }
///     
///     // Get topological order (if no cycle)
///     auto order = result.topological_order();
/// @endcode
///
template<typename G>
    requires VertexListGraph<G> && IncidenceGraph<G>
auto depth_first_search(const G& g, vertex_descriptor_t<G> source) {
    auto result = make_dfs_result(g);
    detail::dfs_impl(g, source, result);
    return result;
}

/// Perform depth-first search visiting all vertices.
///
/// This overload creates a DFS forest, visiting all vertices even in
/// disconnected graphs. Useful for topological sorting and detecting
/// cycles in the entire graph.
///
/// @param g The graph
/// @return dfs_result containing DFS forest information
///
/// Example:
/// @code
///     auto result = depth_first_search(g);
///     
///     // Topological sort of entire graph (if DAG)
///     if (!result.has_cycle()) {
///         auto sorted = result.topological_order();
///     }
/// @endcode
///
template<typename G>
    requires VertexListGraph<G> && IncidenceGraph<G>
auto depth_first_search(const G& g) {
    auto result = make_dfs_result(g);
    detail::dfs_impl_all(g, result);
    return result;
}

// =============================================================================
// depth_first_search - Named Parameters Overload
// =============================================================================

/// Perform depth-first search from a source vertex using named parameters.
///
/// This overload uses C++20 designated initializers for flexible parameter
/// specification.
///
/// Requirements:
/// - G must satisfy VertexListGraph and IncidenceGraph concepts
///
/// Complexity: O(V + E)
///
/// @param g The graph
/// @param source The source vertex
/// @param params Named parameters (see dfs_params)
/// @return dfs_result containing DFS tree information
///
/// Example:
/// @code
///     // With default parameters
///     auto r1 = depth_first_search(g, source, dfs_params{});
///
///     // With custom visitor
///     auto r2 = depth_first_search(g, source, dfs_params{
///         .visitor = my_dfs_visitor{}
///     });
/// @endcode
///
template<typename G, typename... ParamTypes>
    requires VertexListGraph<G> && IncidenceGraph<G>
auto depth_first_search(
    const G& g,
    vertex_descriptor_t<G> source,
    const dfs_params<ParamTypes...>& params
) {
    // For now, params.visitor is not used in the basic implementation
    // but the interface is ready for visitor support (Phase 2.3)
    auto result = make_dfs_result(g);
    detail::dfs_impl(g, source, result);
    return result;
}

/// Perform depth-first search on entire graph using named parameters.
///
/// @param g The graph
/// @param params Named parameters (see dfs_params)
/// @return dfs_result containing DFS forest information
///
template<typename G, typename... ParamTypes>
    requires VertexListGraph<G> && IncidenceGraph<G>
auto depth_first_search(
    const G& g,
    const dfs_params<ParamTypes...>& params
) {
    auto result = make_dfs_result(g);
    detail::dfs_impl_all(g, result);
    return result;
}

// =============================================================================
// depth_first_search - Callbacks Overload
// =============================================================================

/// Perform depth-first search from a source vertex with visitor callbacks.
///
/// This overload accepts a dfs_callbacks struct for event-driven processing
/// during the DFS traversal.
///
/// Requirements:
/// - G must satisfy VertexListGraph and IncidenceGraph concepts
///
/// Complexity: O(V + E)
///
/// @param g The graph
/// @param source The source vertex
/// @param callbacks Visitor callbacks for DFS events
/// @return dfs_result containing DFS tree information
///
/// Example:
/// @code
///     bool has_cycle = false;
///     auto result = depth_first_search(g, source, dfs_callbacks{
///         .on_back_edge = [&](auto e, const auto& g) {
///             has_cycle = true;
///             std::cout << "Cycle detected: " << source(e, g) 
///                       << " -> " << target(e, g) << "\n";
///         },
///         .on_finish_vertex = [](auto v, const auto&) {
///             std::cout << "Finished: " << v << "\n";
///         }
///     });
/// @endcode
///
template<typename G, typename... CallbackTypes>
    requires VertexListGraph<G> && IncidenceGraph<G>
auto depth_first_search(
    const G& g,
    vertex_descriptor_t<G> source,
    const dfs_callbacks<CallbackTypes...>& callbacks
) {
    auto result = make_dfs_result(g);
    detail::dfs_impl(g, source, result, callbacks);
    return result;
}

/// Perform depth-first search on entire graph with visitor callbacks.
///
/// This creates a DFS forest, visiting all vertices even in disconnected
/// graphs.
///
/// @param g The graph
/// @param callbacks Visitor callbacks for DFS events
/// @return dfs_result containing DFS forest information
///
/// Example:
/// @code
///     std::vector<std::size_t> finish_order;
///     auto result = depth_first_search(g, dfs_callbacks{
///         .on_finish_vertex = [&](auto v, const auto&) {
///             finish_order.push_back(v);
///         }
///     });
///     // finish_order is reverse topological order
/// @endcode
///
template<typename G, typename... CallbackTypes>
    requires VertexListGraph<G> && IncidenceGraph<G>
auto depth_first_search(
    const G& g,
    const dfs_callbacks<CallbackTypes...>& callbacks
) {
    auto result = make_dfs_result(g);
    detail::dfs_impl_all(g, result, callbacks);
    return result;
}

// =============================================================================
// depth_first_search - Multi-Source Overloads (Range-Based)
// =============================================================================

/// Perform depth-first search from multiple source vertices.
///
/// This overload accepts a range of source vertices, enabling multi-source DFS.
/// Each source that hasn't been visited becomes a new tree root in the DFS forest.
/// This is useful for:
/// - Starting DFS from a specific subset of vertices
/// - Processing vertices in a custom order
/// - Combining with std::views::filter for conditional traversal
///
/// Requirements:
/// - G must satisfy VertexListGraph and IncidenceGraph concepts
/// - Sources must be a range of vertex descriptors (or convertible to them)
///
/// Complexity: O(V + E)
///
/// @param g The graph
/// @param sources Range of source vertices (e.g., vector, array, views::filter result)
/// @return dfs_result containing DFS forest information
///
/// Example:
/// @code
///     // Multi-source DFS from vertices 0 and 5
///     auto result = depth_first_search(g, std::vector{0, 5});
///     
///     // DFS from all even-numbered vertices first
///     auto evens = vertices(g) | std::views::filter([](auto v) { return v % 2 == 0; });
///     auto result2 = depth_first_search(g, evens);
/// @endcode
///
template<typename G, std::ranges::input_range Sources>
    requires VertexListGraph<G> && IncidenceGraph<G> &&
             std::convertible_to<std::ranges::range_value_t<Sources>, vertex_descriptor_t<G>>
auto depth_first_search(const G& g, Sources&& sources) {
    auto result = make_dfs_result(g);
    detail::dfs_impl_multi(g, std::forward<Sources>(sources), result);
    return result;
}

/// Perform multi-source depth-first search with visitor callbacks.
///
/// This overload combines multi-source DFS with event-driven callbacks.
///
/// Requirements:
/// - G must satisfy VertexListGraph and IncidenceGraph concepts
/// - Sources must be a range of vertex descriptors
///
/// Complexity: O(V + E)
///
/// @param g The graph
/// @param sources Range of source vertices
/// @param callbacks Visitor callbacks for DFS events
/// @return dfs_result containing DFS forest information
///
/// Example:
/// @code
///     std::vector<int> order;
///     auto sources = std::vector{0, 5};
///     auto result = depth_first_search(g, sources, 
///         on_discover_vertex([&](auto v, const auto&) {
///             order.push_back(v);
///         })
///     );
/// @endcode
///
template<typename G, std::ranges::input_range Sources, typename... CallbackTypes>
    requires VertexListGraph<G> && IncidenceGraph<G> &&
             std::convertible_to<std::ranges::range_value_t<Sources>, vertex_descriptor_t<G>>
auto depth_first_search(
    const G& g, 
    Sources&& sources,
    const dfs_callbacks<CallbackTypes...>& callbacks
) {
    auto result = make_dfs_result(g);
    detail::dfs_impl_multi(g, std::forward<Sources>(sources), result, callbacks);
    return result;
}

} // namespace bgl

#endif // BGL_MODERN_DEPTH_FIRST_SEARCH_HPP
