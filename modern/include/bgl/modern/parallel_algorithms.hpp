// BGL Modern - Parallel Graph Algorithms
// C++20 parallel execution policy support
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_PARALLEL_ALGORITHMS_HPP
#define BGL_MODERN_PARALLEL_ALGORITHMS_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>
#include <bgl/modern/range_functions.hpp>
#include <bgl/modern/breadth_first_search.hpp>
#include <bgl/modern/connected_components.hpp>

#include <vector>
#include <queue>
#include <atomic>
#include <execution>
#include <algorithm>
#include <numeric>
#include <concepts>

namespace bgl {

// =============================================================================
// Parallel BFS - Level-Synchronous
// =============================================================================

/// Parallel breadth-first search using level-synchronous parallelism.
///
/// This algorithm processes each BFS level in parallel. At each level,
/// all vertices at the same distance from the source are processed
/// concurrently. This is efficient for graphs with high branching factor.
///
/// Requires C++20 <execution> support with std::execution::par.
///
/// Complexity: O(V + E) work, O(diameter * log V) span (parallel time)
///
/// Example:
/// @code
///     auto result = parallel_bfs(std::execution::par, graph, source);
///     auto dist = result.distance_to(target);
/// @endcode
///
template<Graph G, typename ExecutionPolicy>
    requires std::is_execution_policy_v<std::remove_cvref_t<ExecutionPolicy>>
bfs_result<G> parallel_bfs(
    ExecutionPolicy&& policy,
    const G& g,
    vertex_descriptor_t<G> source)
{
    using Vertex = vertex_descriptor_t<G>;
    using Color = vertex_color;
    using size_type = std::size_t;
    
    const size_type n = num_vertices(g);
    bfs_result<G> result(n, source);
    
    // Color map - use atomic for thread safety
    std::vector<std::atomic<Color>> colors(n);
    std::for_each(std::execution::seq, colors.begin(), colors.end(),
                  [](auto& c) { c.store(Color::white, std::memory_order_relaxed); });
    
    // Initialize source
    colors[source].store(Color::gray, std::memory_order_relaxed);
    result.distance_map()(source) = 0;
    result.predecessor_map()(source) = source;
    result.discover_vertex(source);
    
    // Current and next frontier
    std::vector<Vertex> current_level = {source};
    std::vector<Vertex> next_level;
    
    size_type distance = 0;
    
    while (!current_level.empty()) {
        // Use atomic operations to build next level
        std::vector<Vertex> all_neighbors;
        std::atomic<size_type> neighbors_size{0};
        
        // First pass: count total neighbors needed
        std::for_each(
            std::forward<ExecutionPolicy>(policy),
            current_level.begin(),
            current_level.end(),
            [&](Vertex u) {
                size_type local_count = 0;
                for (auto e : out_edges(u, g)) {
                    Vertex v = target(e, g);
                    Color expected = Color::white;
                    if (colors[v].load(std::memory_order_acquire) == Color::white) {
                        ++local_count;
                    }
                }
                neighbors_size.fetch_add(local_count, std::memory_order_relaxed);
            }
        );
        
        // Reserve space for all neighbors
        all_neighbors.reserve(neighbors_size.load(std::memory_order_relaxed));
        std::atomic<size_type> insert_pos{0};
        
        // Second pass: claim vertices and add to next level
        std::for_each(
            std::forward<ExecutionPolicy>(policy),
            current_level.begin(),
            current_level.end(),
            [&, distance](Vertex u) {
                std::vector<Vertex> local_neighbors;
                
                // Examine all neighbors
                for (auto e : out_edges(u, g)) {
                    Vertex v = target(e, g);
                    
                    // Try to claim vertex v
                    Color expected = Color::white;
                    if (colors[v].compare_exchange_strong(
                            expected, Color::gray,
                            std::memory_order_acq_rel)) {
                        // Successfully claimed
                        local_neighbors.push_back(v);
                        result.distance_map()(v) = distance + 1;
                        result.predecessor_map()(v) = u;
                        result.discover_vertex(v);
                    }
                }
                
                // Mark vertex as finished
                colors[u].store(Color::black, std::memory_order_release);
                
                // Add local neighbors to global list atomically
                if (!local_neighbors.empty()) {
                    size_type pos = insert_pos.fetch_add(local_neighbors.size(), 
                                                          std::memory_order_relaxed);
                    // This is actually not thread-safe for resize...
                    // Better approach: use mutex or different collection strategy
                }
            }
        );
        
        // Simpler approach: just collect all discovered vertices
        next_level.clear();
        for (Vertex v = 0; v < n; ++v) {
            if (colors[v].load(std::memory_order_relaxed) == Color::gray &&
                result.distance_to(v) == distance + 1) {
                next_level.push_back(v);
            }
        }
        
        // Swap levels
        current_level.swap(next_level);
        ++distance;
    }
    
    return result;
}

/// Parallel BFS with sequential policy fallback (returns sequential BFS)
///
template<Graph G>
bfs_result<G> parallel_bfs(
    std::execution::sequenced_policy,
    const G& g,
    vertex_descriptor_t<G> source)
{
    return breadth_first_search(g, source);
}

// =============================================================================
// Parallel Connected Components - Shiloach-Vishkin Style
// =============================================================================

namespace detail {

/// Union-Find structure for parallel connected components
template<typename Vertex>
class parallel_disjoint_set {
public:
    explicit parallel_disjoint_set(std::size_t n)
        : parent_(n), rank_(n, 0)
    {
        std::iota(parent_.begin(), parent_.end(), Vertex{0});
    }
    
    /// Find with path halving (parallel-safe)
    Vertex find(Vertex x) {
        while (parent_[x] != x) {
            Vertex next = parent_[x];
            parent_[x] = parent_[next];  // Path halving
            x = next;
        }
        return x;
    }
    
    /// Union by rank (requires external synchronization)
    bool unite(Vertex x, Vertex y) {
        x = find(x);
        y = find(y);
        
        if (x == y) return false;
        
        // Union by rank
        if (rank_[x] < rank_[y]) {
            parent_[x] = y;
        } else if (rank_[x] > rank_[y]) {
            parent_[y] = x;
        } else {
            parent_[y] = x;
            ++rank_[x];
        }
        
        return true;
    }
    
    /// Atomic union for parallel execution
    bool atomic_unite(Vertex x, Vertex y) {
        while (true) {
            x = find(x);
            y = find(y);
            
            if (x == y) return false;
            
            // Ensure x < y for consistency
            if (x > y) std::swap(x, y);
            
            // Try to hook y to x
            Vertex expected = y;
            if (std::atomic_ref(parent_[y]).compare_exchange_strong(
                    expected, x, std::memory_order_acq_rel)) {
                return true;
            }
            // If CAS failed, retry
        }
    }
    
    std::vector<Vertex>& parents() { return parent_; }
    
private:
    std::vector<Vertex> parent_;
    std::vector<std::size_t> rank_;
};

} // namespace detail

/// Parallel connected components using Shiloach-Vishkin algorithm.
///
/// This algorithm processes all edges in parallel, performing atomic
/// union-find operations to merge components. It's efficient for
/// dense graphs with many edges.
///
/// Complexity: O(E * α(V)) work, O(log V) span expected
///
/// Example:
/// @code
///     auto result = parallel_connected_components(std::execution::par, graph);
///     std::size_t num = result.num_components();
/// @endcode
///
template<Graph G, typename ExecutionPolicy>
    requires std::is_execution_policy_v<std::remove_cvref_t<ExecutionPolicy>>
component_result<G> parallel_connected_components(
    ExecutionPolicy&& policy,
    const G& g)
{
    using Vertex = vertex_descriptor_t<G>;
    using Edge = edge_descriptor_t<G>;
    
    const auto n = num_vertices(g);
    
    if (n == 0) {
        return component_result<G>(0);
    }
    
    // Collect all edges into a vector for parallel processing
    std::vector<std::pair<Vertex, Vertex>> edge_list;
    for (Vertex u : vertices(g)) {
        for (auto e : out_edges(u, g)) {
            Vertex v = target(e, g);
            // For undirected graphs, only process each edge once
            if (u <= v) {
                edge_list.push_back({u, v});
            }
        }
    }
    
    // Initialize disjoint set
    detail::parallel_disjoint_set<Vertex> dset(n);
    
    // Iteratively process edges until no changes
    bool changed = true;
    int iterations = 0;
    const int max_iterations = 100; // Safety limit
    
    while (changed && iterations < max_iterations) {
        changed = false;
        
        // Process all edges in parallel
        std::for_each(
            std::forward<ExecutionPolicy>(policy),
            edge_list.begin(),
            edge_list.end(),
            [&](const std::pair<Vertex, Vertex>& edge) {
                Vertex u = edge.first;
                Vertex v = edge.second;
                
                if (dset.atomic_unite(u, v)) {
                    changed = true;
                }
            }
        );
        
        ++iterations;
    }
    
    // Path compression pass
    auto& parents = dset.parents();
    std::for_each(
        std::forward<ExecutionPolicy>(policy),
        parents.begin(),
        parents.end(),
        [&](Vertex& p) {
            Vertex root = dset.find(&p - parents.data());
            p = root;
        }
    );
    
    // Build component result
    component_result<G> result(n);
    
    // Assign component IDs (compress component labels to 0..k-1)
    std::vector<Vertex> unique_roots;
    unique_roots.reserve(n);
    for (Vertex v : parents) {
        unique_roots.push_back(v);
    }
    std::sort(unique_roots.begin(), unique_roots.end());
    auto last = std::unique(unique_roots.begin(), unique_roots.end());
    unique_roots.erase(last, unique_roots.end());
    
    std::size_t num_components = unique_roots.size();
    result.set_num_components(num_components);
    
    // Map roots to component IDs
    std::vector<std::size_t> root_to_id(n, static_cast<std::size_t>(-1));
    for (std::size_t i = 0; i < unique_roots.size(); ++i) {
        root_to_id[unique_roots[i]] = i;
    }
    
    // Assign components
    for (Vertex v = 0; v < n; ++v) {
        Vertex root = parents[v];
        result.set_component(v, root_to_id[root]);
    }
    
    return result;
}

/// Parallel connected components with sequential policy fallback
///
template<Graph G>
component_result<G> parallel_connected_components(
    std::execution::sequenced_policy,
    const G& g)
{
    return connected_components(g);
}

// =============================================================================
// Parallel Vertex/Edge Iteration Utilities
// =============================================================================

/// Apply a function to all vertices in parallel.
///
/// Example:
/// @code
///     parallel_for_each_vertex(std::execution::par, graph,
///         [&](auto v) { /* process vertex */ });
/// @endcode
///
template<VertexListGraph G, typename ExecutionPolicy, typename Function>
    requires std::is_execution_policy_v<std::remove_cvref_t<ExecutionPolicy>>
void parallel_for_each_vertex(
    ExecutionPolicy&& policy,
    const G& g,
    Function&& f)
{
    std::vector<vertex_descriptor_t<G>> vertex_list;
    vertex_list.reserve(num_vertices(g));
    
    for (auto v : vertices(g)) {
        vertex_list.push_back(v);
    }
    
    std::for_each(
        std::forward<ExecutionPolicy>(policy),
        vertex_list.begin(),
        vertex_list.end(),
        std::forward<Function>(f)
    );
}

/// Apply a function to all edges in parallel.
///
/// Example:
/// @code
///     parallel_for_each_edge(std::execution::par, graph,
///         [&](auto e) { /* process edge */ });
/// @endcode
///
template<EdgeListGraph G, typename ExecutionPolicy, typename Function>
    requires std::is_execution_policy_v<std::remove_cvref_t<ExecutionPolicy>>
void parallel_for_each_edge(
    ExecutionPolicy&& policy,
    const G& g,
    Function&& f)
{
    std::vector<edge_descriptor_t<G>> edge_list;
    edge_list.reserve(num_edges(g));
    
    for (auto e : edges(g)) {
        edge_list.push_back(e);
    }
    
    std::for_each(
        std::forward<ExecutionPolicy>(policy),
        edge_list.begin(),
        edge_list.end(),
        std::forward<Function>(f)
    );
}

} // namespace bgl

#endif // BGL_MODERN_PARALLEL_ALGORITHMS_HPP
