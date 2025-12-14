// BGL Modern - Coroutine-Based Graph Traversals
// Lazy BFS and DFS using C++20 coroutines
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_COROUTINE_TRAVERSE_HPP
#define BGL_MODERN_COROUTINE_TRAVERSE_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/generator.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>

#include <queue>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <concepts>
#include <cstddef>

namespace bgl {

// =============================================================================
// BFS Traversal Event Types
// =============================================================================

/// Event types for BFS traversal
enum class bfs_event {
    discover_vertex,    ///< First time vertex is seen
    examine_vertex,     ///< Vertex is being processed
    examine_edge,       ///< Edge is being examined
    tree_edge,          ///< Edge leads to undiscovered vertex
    non_tree_edge,      ///< Edge leads to already-discovered vertex
    finish_vertex       ///< All edges of vertex have been examined
};

/// Result from BFS traversal - vertex with event type
template<typename Vertex>
struct bfs_step {
    Vertex vertex;
    bfs_event event;
    std::size_t depth;  ///< Distance from start vertex
    
    bool operator==(const bfs_step&) const = default;
};

// =============================================================================
// DFS Traversal Event Types
// =============================================================================

/// Event types for DFS traversal
enum class dfs_event {
    discover_vertex,    ///< First time vertex is seen (pre-order)
    examine_edge,       ///< Edge is being examined
    tree_edge,          ///< Edge leads to undiscovered vertex
    back_edge,          ///< Edge leads to ancestor in DFS tree
    forward_or_cross_edge, ///< Edge leads to already-finished vertex
    finish_vertex       ///< All descendants have been visited (post-order)
};

/// Result from DFS traversal - vertex with event type
template<typename Vertex>
struct dfs_step {
    Vertex vertex;
    dfs_event event;
    std::size_t depth;  ///< Depth in DFS tree
    
    bool operator==(const dfs_step&) const = default;
};

// =============================================================================
// bfs_traverse - Coroutine BFS
// =============================================================================

/// Perform breadth-first traversal yielding vertices lazily.
///
/// This is a coroutine that yields each discovered vertex exactly once,
/// in BFS order. Iteration can be terminated early by breaking from the loop.
///
/// Example:
/// @code
///     for (auto v : bfs_traverse(graph, start)) {
///         std::cout << "Visited: " << v << "\n";
///         if (v == target) break;  // Early termination
///     }
/// @endcode
///
template<Graph G>
generator<vertex_descriptor_t<G>> bfs_traverse(
    const G& g,
    vertex_descriptor_t<G> start)
{
    using Vertex = vertex_descriptor_t<G>;
    
    std::unordered_set<Vertex> discovered;
    std::queue<Vertex> queue;
    
    discovered.insert(start);
    queue.push(start);
    
    while (!queue.empty()) {
        Vertex u = queue.front();
        queue.pop();
        
        co_yield u;
        
        for (auto e : out_edges(u, g)) {
            Vertex v = target(e, g);
            if (!discovered.count(v)) {
                discovered.insert(v);
                queue.push(v);
            }
        }
    }
}

/// BFS traversal with full event information.
///
/// Yields bfs_step objects containing the vertex, event type, and depth.
///
template<Graph G>
generator<bfs_step<vertex_descriptor_t<G>>> bfs_traverse_events(
    const G& g,
    vertex_descriptor_t<G> start)
{
    using Vertex = vertex_descriptor_t<G>;
    using Step = bfs_step<Vertex>;
    
    std::unordered_set<Vertex> discovered;
    std::queue<std::pair<Vertex, std::size_t>> queue;  // (vertex, depth)
    
    discovered.insert(start);
    queue.push({start, 0});
    co_yield Step{start, bfs_event::discover_vertex, 0};
    
    while (!queue.empty()) {
        auto [u, depth] = queue.front();
        queue.pop();
        
        co_yield Step{u, bfs_event::examine_vertex, depth};
        
        for (auto e : out_edges(u, g)) {
            Vertex v = target(e, g);
            
            if (!discovered.count(v)) {
                discovered.insert(v);
                queue.push({v, depth + 1});
                co_yield Step{v, bfs_event::discover_vertex, depth + 1};
            }
        }
        
        co_yield Step{u, bfs_event::finish_vertex, depth};
    }
}

/// BFS traversal from multiple start vertices.
///
/// Useful for disconnected graphs or multi-source BFS.
///
template<Graph G, std::ranges::input_range StartRange>
    requires std::same_as<std::ranges::range_value_t<StartRange>, vertex_descriptor_t<G>>
generator<vertex_descriptor_t<G>> bfs_traverse_multi(
    const G& g,
    StartRange&& starts)
{
    using Vertex = vertex_descriptor_t<G>;
    
    std::unordered_set<Vertex> discovered;
    std::queue<Vertex> queue;
    
    for (Vertex s : starts) {
        if (!discovered.count(s)) {
            discovered.insert(s);
            queue.push(s);
        }
    }
    
    while (!queue.empty()) {
        Vertex u = queue.front();
        queue.pop();
        
        co_yield u;
        
        for (auto e : out_edges(u, g)) {
            Vertex v = target(e, g);
            if (!discovered.count(v)) {
                discovered.insert(v);
                queue.push(v);
            }
        }
    }
}

// =============================================================================
// dfs_traverse - Coroutine DFS
// =============================================================================

/// Perform depth-first traversal yielding vertices lazily.
///
/// This is a coroutine that yields each discovered vertex exactly once,
/// in DFS pre-order. Iteration can be terminated early.
///
/// Example:
/// @code
///     for (auto v : dfs_traverse(graph, start)) {
///         std::cout << "Visited: " << v << "\n";
///         if (v == target) break;  // Early termination
///     }
/// @endcode
///
template<Graph G>
generator<vertex_descriptor_t<G>> dfs_traverse(
    const G& g,
    vertex_descriptor_t<G> start)
{
    using Vertex = vertex_descriptor_t<G>;
    
    std::unordered_set<Vertex> visited;
    std::stack<Vertex> stack;
    
    stack.push(start);
    
    while (!stack.empty()) {
        Vertex u = stack.top();
        stack.pop();
        
        if (visited.count(u)) continue;
        visited.insert(u);
        
        co_yield u;
        
        // Push neighbors in reverse order for consistent ordering
        std::vector<Vertex> neighbors;
        for (auto e : out_edges(u, g)) {
            neighbors.push_back(target(e, g));
        }
        for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it) {
            if (!visited.count(*it)) {
                stack.push(*it);
            }
        }
    }
}

/// DFS traversal with full event information.
///
/// Yields dfs_step objects with pre-order discover and post-order finish events.
///
template<Graph G>
generator<dfs_step<vertex_descriptor_t<G>>> dfs_traverse_events(
    const G& g,
    vertex_descriptor_t<G> start)
{
    using Vertex = vertex_descriptor_t<G>;
    using Step = dfs_step<Vertex>;
    
    enum class Color { White, Gray, Black };
    
    std::unordered_map<Vertex, Color> color;
    
    // Stack entries: (vertex, depth, phase)
    // phase 0 = entering, phase 1 = finished examining edges
    struct StackEntry {
        Vertex vertex;
        std::size_t depth;
        bool finished;
    };
    std::stack<StackEntry> stack;
    
    stack.push({start, 0, false});
    
    while (!stack.empty()) {
        auto [u, depth, finished] = stack.top();
        stack.pop();
        
        if (finished) {
            color[u] = Color::Black;
            co_yield Step{u, dfs_event::finish_vertex, depth};
            continue;
        }
        
        if (color[u] == Color::Gray || color[u] == Color::Black) {
            continue;
        }
        
        color[u] = Color::Gray;
        co_yield Step{u, dfs_event::discover_vertex, depth};
        
        // Push finish marker
        stack.push({u, depth, true});
        
        // Push unvisited neighbors
        std::vector<Vertex> neighbors;
        for (auto e : out_edges(u, g)) {
            neighbors.push_back(target(e, g));
        }
        
        for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it) {
            Vertex v = *it;
            if (color.find(v) == color.end() || color[v] == Color::White) {
                stack.push({v, depth + 1, false});
            }
        }
    }
}

/// DFS traversal from multiple start vertices.
///
/// Visits all vertices reachable from any start vertex.
///
template<Graph G, std::ranges::input_range StartRange>
    requires std::same_as<std::ranges::range_value_t<StartRange>, vertex_descriptor_t<G>>
generator<vertex_descriptor_t<G>> dfs_traverse_multi(
    const G& g,
    StartRange&& starts)
{
    using Vertex = vertex_descriptor_t<G>;
    
    std::unordered_set<Vertex> visited;
    
    for (Vertex start : starts) {
        if (visited.count(start)) continue;
        
        std::stack<Vertex> stack;
        stack.push(start);
        
        while (!stack.empty()) {
            Vertex u = stack.top();
            stack.pop();
            
            if (visited.count(u)) continue;
            visited.insert(u);
            
            co_yield u;
            
            std::vector<Vertex> neighbors;
            for (auto e : out_edges(u, g)) {
                neighbors.push_back(target(e, g));
            }
            for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it) {
                if (!visited.count(*it)) {
                    stack.push(*it);
                }
            }
        }
    }
}

// =============================================================================
// Full Graph Traversal
// =============================================================================

/// BFS traversal of entire graph (all components).
///
/// Starts from vertex 0 and continues from unvisited vertices.
///
template<VertexListGraph G>
generator<vertex_descriptor_t<G>> bfs_traverse_all(const G& g)
{
    using Vertex = vertex_descriptor_t<G>;
    
    std::unordered_set<Vertex> discovered;
    
    for (Vertex start : vertices(g)) {
        if (discovered.count(start)) continue;
        
        std::queue<Vertex> queue;
        discovered.insert(start);
        queue.push(start);
        
        while (!queue.empty()) {
            Vertex u = queue.front();
            queue.pop();
            
            co_yield u;
            
            for (auto e : out_edges(u, g)) {
                Vertex v = target(e, g);
                if (!discovered.count(v)) {
                    discovered.insert(v);
                    queue.push(v);
                }
            }
        }
    }
}

/// DFS traversal of entire graph (all components).
///
template<VertexListGraph G>
generator<vertex_descriptor_t<G>> dfs_traverse_all(const G& g)
{
    using Vertex = vertex_descriptor_t<G>;
    
    std::unordered_set<Vertex> visited;
    
    for (Vertex start : vertices(g)) {
        if (visited.count(start)) continue;
        
        std::stack<Vertex> stack;
        stack.push(start);
        
        while (!stack.empty()) {
            Vertex u = stack.top();
            stack.pop();
            
            if (visited.count(u)) continue;
            visited.insert(u);
            
            co_yield u;
            
            std::vector<Vertex> neighbors;
            for (auto e : out_edges(u, g)) {
                neighbors.push_back(target(e, g));
            }
            for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it) {
                if (!visited.count(*it)) {
                    stack.push(*it);
                }
            }
        }
    }
}

} // namespace bgl

#endif // BGL_MODERN_COROUTINE_TRAVERSE_HPP
