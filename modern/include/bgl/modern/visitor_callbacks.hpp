// BGL Modern - Visitor Callbacks
// Lambda-friendly visitor patterns for graph algorithms (C++20)
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// =============================================================================
// VISITOR CALLBACKS OVERVIEW
// =============================================================================
//
// This header provides callback-based visitor structs for BFS and DFS that
// replace the traditional BGL visitor concept with a more lambda-friendly
// approach using C++20 designated initializers.
//
// Example usage:
// @code
//     // Multiple callbacks
//     auto result = breadth_first_search(g, source, bfs_callbacks{
//         .on_discover_vertex = [](auto v) { std::cout << "Discovered: " << v << "\n"; },
//         .on_examine_edge = [](auto e, auto& g) { std::cout << "Edge: " << source(e, g) << "->" << target(e, g) << "\n"; }
//     });
//
//     // Single callback shorthand
//     auto result2 = breadth_first_search(g, source, 
//         on_discover_vertex([](auto v) { std::cout << v << "\n"; })
//     );
// @endcode
//
// =============================================================================

#ifndef BGL_MODERN_VISITOR_CALLBACKS_HPP
#define BGL_MODERN_VISITOR_CALLBACKS_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/graph_traits.hpp>

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace bgl {

// =============================================================================
// Null Callback (No-op)
// =============================================================================

/// Null callback that does nothing. Used as default for optional callbacks.
struct null_callback {
    template<typename... Args>
    constexpr void operator()(Args&&...) const noexcept {}
};

// Constant instance for convenience
inline constexpr null_callback no_op{};

// =============================================================================
// Callback Concepts
// =============================================================================

/// Concept for a vertex callback: invocable with (vertex_descriptor)
template<typename F, typename Vertex>
concept VertexCallback = std::invocable<F, Vertex>;

/// Concept for an edge callback: invocable with (edge_descriptor, const Graph&)
template<typename F, typename Edge, typename Graph>
concept EdgeCallback = std::invocable<F, Edge, const Graph&>;

/// Concept for a vertex-graph callback: invocable with (vertex_descriptor, const Graph&)
template<typename F, typename Vertex, typename Graph>
concept VertexGraphCallback = std::invocable<F, Vertex, const Graph&>;

// =============================================================================
// bfs_callbacks - BFS Visitor Callbacks
// =============================================================================

/// Callback-based visitor for breadth-first search.
///
/// All callbacks are optional and default to no-op. Use C++20 designated
/// initializers to specify only the callbacks you need.
///
/// BFS Events:
/// - initialize_vertex: Called on every vertex before the search starts
/// - discover_vertex: Called when a vertex is first discovered
/// - examine_vertex: Called when a vertex is popped from the queue
/// - examine_edge: Called for each out-edge of a vertex being examined
/// - tree_edge: Called when an edge becomes part of the BFS tree
/// - non_tree_edge: Called for non-tree edges (gray or black targets)
/// - gray_target: Called when an edge target is already in the queue
/// - black_target: Called when an edge target is already finished
/// - finish_vertex: Called when all out-edges have been examined
///
/// Example:
/// @code
///     auto result = breadth_first_search(g, source, bfs_callbacks{
///         .on_discover_vertex = [](auto v) { 
///             std::cout << "Discovered: " << v << "\n"; 
///         },
///         .on_tree_edge = [](auto e, const auto& g) {
///             std::cout << "Tree edge: " << source(e, g) << "->" << target(e, g) << "\n";
///         }
///     });
/// @endcode
///
template<
    typename InitializeVertex = null_callback,
    typename DiscoverVertex = null_callback,
    typename ExamineVertex = null_callback,
    typename ExamineEdge = null_callback,
    typename TreeEdge = null_callback,
    typename NonTreeEdge = null_callback,
    typename GrayTarget = null_callback,
    typename BlackTarget = null_callback,
    typename FinishVertex = null_callback
>
struct bfs_callbacks {
    /// Called on every vertex before the search starts
    InitializeVertex on_initialize_vertex = {};
    
    /// Called when a vertex is first discovered (added to queue)
    DiscoverVertex on_discover_vertex = {};
    
    /// Called when a vertex is removed from the queue for examination
    ExamineVertex on_examine_vertex = {};
    
    /// Called for each out-edge of a vertex being examined
    ExamineEdge on_examine_edge = {};
    
    /// Called when an edge becomes part of the BFS tree
    TreeEdge on_tree_edge = {};
    
    /// Called for edges to already-discovered vertices
    NonTreeEdge on_non_tree_edge = {};
    
    /// Called when an edge points to a gray vertex (in queue)
    GrayTarget on_gray_target = {};
    
    /// Called when an edge points to a black vertex (finished)
    BlackTarget on_black_target = {};
    
    /// Called when all out-edges of a vertex have been examined
    FinishVertex on_finish_vertex = {};
    
    // Helper to check if this is effectively empty (all null callbacks)
    static constexpr bool is_null() {
        return std::is_same_v<InitializeVertex, null_callback> &&
               std::is_same_v<DiscoverVertex, null_callback> &&
               std::is_same_v<ExamineVertex, null_callback> &&
               std::is_same_v<ExamineEdge, null_callback> &&
               std::is_same_v<TreeEdge, null_callback> &&
               std::is_same_v<NonTreeEdge, null_callback> &&
               std::is_same_v<GrayTarget, null_callback> &&
               std::is_same_v<BlackTarget, null_callback> &&
               std::is_same_v<FinishVertex, null_callback>;
    }
};

// Deduction guide for bfs_callbacks
template<typename... Args>
bfs_callbacks(Args...) -> bfs_callbacks<Args...>;

// =============================================================================
// dfs_callbacks - DFS Visitor Callbacks
// =============================================================================

/// Callback-based visitor for depth-first search.
///
/// All callbacks are optional and default to no-op. Use C++20 designated
/// initializers to specify only the callbacks you need.
///
/// DFS Events:
/// - initialize_vertex: Called on every vertex before the search starts
/// - start_vertex: Called on each root vertex of a DFS tree
/// - discover_vertex: Called when a vertex is first discovered
/// - examine_edge: Called for each out-edge of a vertex being examined
/// - tree_edge: Called when an edge becomes part of the DFS tree
/// - back_edge: Called for edges to ancestors (indicates a cycle)
/// - forward_or_cross_edge: Called for forward/cross edges (directed graphs)
/// - finish_edge: Called after an edge has been processed
/// - finish_vertex: Called when all out-edges have been examined
///
/// Example:
/// @code
///     bool has_cycle = false;
///     depth_first_search(g, dfs_callbacks{
///         .on_back_edge = [&](auto e, const auto& g) {
///             has_cycle = true;
///             std::cout << "Cycle detected via edge: " 
///                       << source(e, g) << "->" << target(e, g) << "\n";
///         }
///     });
/// @endcode
///
template<
    typename InitializeVertex = null_callback,
    typename StartVertex = null_callback,
    typename DiscoverVertex = null_callback,
    typename ExamineEdge = null_callback,
    typename TreeEdge = null_callback,
    typename BackEdge = null_callback,
    typename ForwardOrCrossEdge = null_callback,
    typename FinishEdge = null_callback,
    typename FinishVertex = null_callback
>
struct dfs_callbacks {
    /// Called on every vertex before the search starts
    InitializeVertex on_initialize_vertex = {};
    
    /// Called on each root vertex when starting a new DFS tree
    StartVertex on_start_vertex = {};
    
    /// Called when a vertex is first discovered
    DiscoverVertex on_discover_vertex = {};
    
    /// Called for each out-edge of a vertex being examined
    ExamineEdge on_examine_edge = {};
    
    /// Called when an edge becomes part of the DFS tree
    TreeEdge on_tree_edge = {};
    
    /// Called for edges to gray vertices (ancestors, indicates cycle)
    BackEdge on_back_edge = {};
    
    /// Called for forward or cross edges (directed graphs only)
    ForwardOrCrossEdge on_forward_or_cross_edge = {};
    
    /// Called after processing an edge (before continuing with siblings)
    FinishEdge on_finish_edge = {};
    
    /// Called when all out-edges of a vertex have been examined
    FinishVertex on_finish_vertex = {};
    
    // Helper to check if this is effectively empty (all null callbacks)
    static constexpr bool is_null() {
        return std::is_same_v<InitializeVertex, null_callback> &&
               std::is_same_v<StartVertex, null_callback> &&
               std::is_same_v<DiscoverVertex, null_callback> &&
               std::is_same_v<ExamineEdge, null_callback> &&
               std::is_same_v<TreeEdge, null_callback> &&
               std::is_same_v<BackEdge, null_callback> &&
               std::is_same_v<ForwardOrCrossEdge, null_callback> &&
               std::is_same_v<FinishEdge, null_callback> &&
               std::is_same_v<FinishVertex, null_callback>;
    }
};

// Deduction guide for dfs_callbacks
template<typename... Args>
dfs_callbacks(Args...) -> dfs_callbacks<Args...>;

// =============================================================================
// Single-Event Callback Wrappers
// =============================================================================
//
// These wrapper functions allow using a single lambda for common use cases
// without needing the full callbacks struct.
//
// Example:
// @code
//     auto result = breadth_first_search(g, source, 
//         on_discover_vertex([](auto v) { std::cout << v << "\n"; })
//     );
// @endcode

namespace callbacks {

/// Wrapper for a discover_vertex callback
template<typename F>
struct discover_vertex_wrapper {
    F callback;
    
    template<typename Vertex, typename Graph>
    void operator()(Vertex v, const Graph&) const {
        if constexpr (std::invocable<F, Vertex>) {
            callback(v);
        } else {
            callback(v);
        }
    }
};

/// Wrapper for a tree_edge callback
template<typename F>
struct tree_edge_wrapper {
    F callback;
    
    template<typename Edge, typename Graph>
    void operator()(Edge e, const Graph& g) const {
        callback(e, g);
    }
};

/// Wrapper for a back_edge callback
template<typename F>
struct back_edge_wrapper {
    F callback;
    
    template<typename Edge, typename Graph>
    void operator()(Edge e, const Graph& g) const {
        callback(e, g);
    }
};

/// Wrapper for a finish_vertex callback
template<typename F>
struct finish_vertex_wrapper {
    F callback;
    
    template<typename Vertex, typename Graph>
    void operator()(Vertex v, const Graph&) const {
        if constexpr (std::invocable<F, Vertex>) {
            callback(v);
        } else {
            callback(v);
        }
    }
};

} // namespace callbacks

/// Create a BFS callbacks struct with only on_discover_vertex set
template<typename F>
auto on_discover_vertex(F&& f) {
    return bfs_callbacks<
        null_callback,  // initialize_vertex
        std::decay_t<F>, // discover_vertex
        null_callback,  // examine_vertex
        null_callback,  // examine_edge
        null_callback,  // tree_edge
        null_callback,  // non_tree_edge
        null_callback,  // gray_target
        null_callback,  // black_target
        null_callback   // finish_vertex
    >{
        .on_discover_vertex = std::forward<F>(f)
    };
}

/// Create a BFS callbacks struct with only on_tree_edge set
template<typename F>
auto on_tree_edge(F&& f) {
    return bfs_callbacks<
        null_callback,  // initialize_vertex
        null_callback,  // discover_vertex
        null_callback,  // examine_vertex
        null_callback,  // examine_edge
        std::decay_t<F>, // tree_edge
        null_callback,  // non_tree_edge
        null_callback,  // gray_target
        null_callback,  // black_target
        null_callback   // finish_vertex
    >{
        .on_tree_edge = std::forward<F>(f)
    };
}

/// Create a DFS callbacks struct with only on_discover_vertex set
template<typename F>
auto on_dfs_discover_vertex(F&& f) {
    return dfs_callbacks<
        null_callback,  // initialize_vertex
        null_callback,  // start_vertex
        std::decay_t<F>, // discover_vertex
        null_callback,  // examine_edge
        null_callback,  // tree_edge
        null_callback,  // back_edge
        null_callback,  // forward_or_cross_edge
        null_callback,  // finish_edge
        null_callback   // finish_vertex
    >{
        .on_discover_vertex = std::forward<F>(f)
    };
}

/// Create a DFS callbacks struct with only on_back_edge set
template<typename F>
auto on_back_edge(F&& f) {
    return dfs_callbacks<
        null_callback,  // initialize_vertex
        null_callback,  // start_vertex
        null_callback,  // discover_vertex
        null_callback,  // examine_edge
        null_callback,  // tree_edge
        std::decay_t<F>, // back_edge
        null_callback,  // forward_or_cross_edge
        null_callback,  // finish_edge
        null_callback   // finish_vertex
    >{
        .on_back_edge = std::forward<F>(f)
    };
}

/// Create a DFS callbacks struct with only on_finish_vertex set
template<typename F>
auto on_finish_vertex(F&& f) {
    return dfs_callbacks<
        null_callback,  // initialize_vertex
        null_callback,  // start_vertex
        null_callback,  // discover_vertex
        null_callback,  // examine_edge
        null_callback,  // tree_edge
        null_callback,  // back_edge
        null_callback,  // forward_or_cross_edge
        null_callback,  // finish_edge
        std::decay_t<F>  // finish_vertex
    >{
        .on_finish_vertex = std::forward<F>(f)
    };
}

// =============================================================================
// Visitor Concepts
// =============================================================================

/// Check if a type is a bfs_callbacks instantiation
template<typename T>
struct is_bfs_callbacks : std::false_type {};

template<typename... Args>
struct is_bfs_callbacks<bfs_callbacks<Args...>> : std::true_type {};

template<typename T>
inline constexpr bool is_bfs_callbacks_v = is_bfs_callbacks<std::remove_cvref_t<T>>::value;

/// Check if a type is a dfs_callbacks instantiation
template<typename T>
struct is_dfs_callbacks : std::false_type {};

template<typename... Args>
struct is_dfs_callbacks<dfs_callbacks<Args...>> : std::true_type {};

template<typename T>
inline constexpr bool is_dfs_callbacks_v = is_dfs_callbacks<std::remove_cvref_t<T>>::value;

/// Concept for BFS visitor callbacks
template<typename T>
concept BFSCallbacks = is_bfs_callbacks_v<T>;

/// Concept for DFS visitor callbacks  
template<typename T>
concept DFSCallbacks = is_dfs_callbacks_v<T>;

} // namespace bgl

#endif // BGL_MODERN_VISITOR_CALLBACKS_HPP
