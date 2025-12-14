// BGL Modern - Prim's Minimum Spanning Tree Algorithm
// C++20 implementation of Prim's MST algorithm
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_PRIM_MINIMUM_SPANNING_TREE_HPP
#define BGL_MODERN_PRIM_MINIMUM_SPANNING_TREE_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>
#include <bgl/modern/property_map.hpp>
#include <bgl/modern/kruskal_minimum_spanning_tree.hpp>  // for mst_result

#include <queue>
#include <vector>
#include <limits>
#include <ranges>
#include <concepts>
#include <type_traits>
#include <optional>

namespace bgl {

// =============================================================================
// prim_result - Extended Result Type for Prim's Algorithm
// =============================================================================

/// Result type for Prim's minimum spanning tree algorithm.
///
/// This extends mst_result with predecessor information, which is naturally
/// produced by Prim's algorithm (similar to Dijkstra's).
///
/// Usage:
/// @code
///     auto result = prim_minimum_spanning_tree(g, source, weight_map);
///     
///     // Get the total weight of the MST
///     double total = result.total_weight();
///     
///     // Iterate over MST edges
///     for (auto e : result.edges()) {
///         std::cout << source(e, g) << " -- " << target(e, g) << "\n";
///     }
///     
///     // Get predecessor tree (useful for path reconstruction)
///     auto pred = result.predecessor_of(v);
/// @endcode
///
/// @tparam G The graph type
/// @tparam WeightType The type used for edge weights (default: double)
///
template<typename G, typename WeightType = double>
class prim_result {
public:
    using graph_type = G;
    using vertex_descriptor = vertex_descriptor_t<G>;
    using edge_descriptor = edge_descriptor_t<G>;
    using weight_type = WeightType;
    using size_type = std::size_t;
    
    /// Sentinel value indicating unreachable vertices
    static constexpr weight_type infinity() {
        if constexpr (std::numeric_limits<weight_type>::has_infinity) {
            return std::numeric_limits<weight_type>::infinity();
        } else {
            return std::numeric_limits<weight_type>::max();
        }
    }
    
    /// Sentinel value for null vertex
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
    
    prim_result() = default;
    
    /// Construct with expected number of vertices
    explicit prim_result(size_type num_vertices)
        : num_vertices_(num_vertices)
        , keys_(num_vertices, infinity())
        , predecessors_(num_vertices, null_vertex())
        , in_mst_(num_vertices, 0)
    {
        if (num_vertices > 0) {
            edges_.reserve(num_vertices - 1);
        }
    }
    
    // -------------------------------------------------------------------------
    // Property Map Access (for algorithm use)
    // -------------------------------------------------------------------------
    
    auto key_map() {
        return [this](vertex_descriptor v) -> weight_type& {
            return keys_[v];
        };
    }
    
    auto predecessor_map() {
        return [this](vertex_descriptor v) -> vertex_descriptor& {
            return predecessors_[v];
        };
    }
    
    auto in_mst_map() {
        return [this](vertex_descriptor v) -> char& {
            return in_mst_[v];
        };
    }
    
    // -------------------------------------------------------------------------
    // Building the Result (for algorithm use)
    // -------------------------------------------------------------------------
    
    void add_edge(edge_descriptor e, weight_type w) {
        edges_.push_back(e);
        total_weight_ += w;
    }
    
    void set_source(vertex_descriptor s) {
        source_ = s;
        keys_[s] = weight_type{0};
        predecessors_[s] = s;
    }
    
    // -------------------------------------------------------------------------
    // Query Interface
    // -------------------------------------------------------------------------
    
    /// Get the source vertex (root of the MST)
    vertex_descriptor source_vertex() const {
        return source_;
    }
    
    /// Get the edges in the MST
    const std::vector<edge_descriptor>& edges() const {
        return edges_;
    }
    
    /// Get a range of MST edges
    auto edge_range() const {
        return std::ranges::subrange(edges_.begin(), edges_.end());
    }
    
    /// Get the total weight of the MST
    weight_type total_weight() const {
        return total_weight_;
    }
    
    /// Get the number of edges in the MST
    size_type num_edges() const {
        return edges_.size();
    }
    
    /// Check if the result is a spanning tree (graph was connected)
    bool is_spanning_tree() const {
        return num_vertices_ > 0 && edges_.size() == num_vertices_ - 1;
    }
    
    /// Check if the MST is empty
    bool empty() const {
        return edges_.empty();
    }
    
    /// Get the number of connected components
    size_type num_components() const {
        if (num_vertices_ == 0) return 0;
        return num_vertices_ - edges_.size();
    }
    
    /// Get the predecessor of a vertex in the MST
    vertex_descriptor predecessor_of(vertex_descriptor v) const {
        return predecessors_[v];
    }
    
    /// Get the key (minimum edge weight) used to add a vertex
    weight_type key_of(vertex_descriptor v) const {
        return keys_[v];
    }
    
    /// Check if a vertex is in the MST
    bool is_in_mst(vertex_descriptor v) const {
        return in_mst_[v] != 0;
    }
    
    /// Get the predecessors array (for compatibility)
    const std::vector<vertex_descriptor>& predecessors() const {
        return predecessors_;
    }
    
private:
    std::vector<edge_descriptor> edges_;
    std::vector<weight_type> keys_;
    std::vector<vertex_descriptor> predecessors_;
    std::vector<char> in_mst_;  // Using char instead of bool for reference stability
    weight_type total_weight_ = weight_type{0};
    size_type num_vertices_ = 0;
    vertex_descriptor source_ = null_vertex();
};

// =============================================================================
// Factory Functions
// =============================================================================

template<typename G, typename WeightType = double>
auto make_prim_result(const G& g) {
    return prim_result<G, WeightType>(num_vertices(g));
}

// =============================================================================
// prim_minimum_spanning_tree - Core Implementation
// =============================================================================

namespace detail {

/// Core Prim's MST implementation using a priority queue
template<typename G, typename WeightType, typename WeightFunc>
void prim_mst_impl(
    const G& g,
    vertex_descriptor_t<G> start,
    prim_result<G, WeightType>& result,
    WeightFunc&& get_weight
) {
    using vertex_descriptor = vertex_descriptor_t<G>;
    using weight_type = WeightType;
    
    const auto n = num_vertices(g);
    if (n == 0) return;
    
    result.set_source(start);
    
    auto key = result.key_map();
    auto pred = result.predecessor_map();
    auto in_mst = result.in_mst_map();
    
    // Track which edge connects each vertex to the MST
    std::vector<std::optional<edge_descriptor_t<G>>> connecting_edge(n);
    
    // Priority queue: (key, vertex)
    // Using greater<> for min-heap
    using pq_entry = std::pair<weight_type, vertex_descriptor>;
    std::priority_queue<pq_entry, std::vector<pq_entry>, std::greater<pq_entry>> pq;
    
    pq.push({weight_type{0}, start});
    
    while (!pq.empty()) {
        auto [k, u] = pq.top();
        pq.pop();
        
        // Skip if already in MST
        if (in_mst(u)) {
            continue;
        }
        
        // Add u to MST
        in_mst(u) = true;
        
        // Add the connecting edge (except for the source)
        if (connecting_edge[u]) {
            result.add_edge(*connecting_edge[u], k);
        }
        
        // Update keys for adjacent vertices
        for (auto e : out_edges(u, g)) {
            vertex_descriptor v = target(e, g);
            weight_type w = get_weight(e);
            
            if (!in_mst(v) && w < key(v)) {
                key(v) = w;
                pred(v) = u;
                connecting_edge[v] = e;
                pq.push({w, v});
            }
        }
    }
}

} // namespace detail

// =============================================================================
// prim_minimum_spanning_tree - Public Interface
// =============================================================================

/// Compute minimum spanning tree using Prim's algorithm.
///
/// Prim's algorithm grows the MST from a starting vertex by repeatedly
/// adding the minimum weight edge that connects a vertex in the tree
/// to a vertex outside the tree.
///
/// Requirements:
/// - G must satisfy VertexListGraph and IncidenceGraph concepts
/// - WeightFunc must be invocable with edge_descriptor and return a weight
///
/// Complexity: O((V + E) log V) with binary heap
///
/// @param g The graph (should be undirected for MST)
/// @param source The starting vertex
/// @param get_weight Function to get edge weight: (edge) -> weight
/// @return prim_result containing MST edges, total weight, and predecessors
///
/// Example:
/// @code
///     auto result = prim_minimum_spanning_tree(g, 0, [&](auto e) {
///         return g[e].weight;
///     });
///     
///     std::cout << "MST weight: " << result.total_weight() << "\n";
///     std::cout << "MST edges:\n";
///     for (auto e : result.edges()) {
///         std::cout << "  " << source(e, g) << " -- " << target(e, g) << "\n";
///     }
///     
///     // Predecessor tree
///     for (auto v : vertices(g)) {
///         if (result.predecessor_of(v) != v) {
///             std::cout << v << " parent: " << result.predecessor_of(v) << "\n";
///         }
///     }
/// @endcode
///
template<typename G, typename WeightFunc>
    requires VertexListGraph<G> && IncidenceGraph<G> &&
             std::invocable<WeightFunc, edge_descriptor_t<G>>
auto prim_minimum_spanning_tree(
    const G& g,
    vertex_descriptor_t<G> source,
    WeightFunc&& get_weight
) {
    using weight_type = std::invoke_result_t<WeightFunc, edge_descriptor_t<G>>;
    
    auto result = make_prim_result<G, weight_type>(g);
    detail::prim_mst_impl(g, source, result, std::forward<WeightFunc>(get_weight));
    return result;
}

/// Compute minimum spanning tree starting from vertex 0.
///
/// Convenience overload that uses vertex 0 as the starting point.
///
template<typename G, typename WeightFunc>
    requires VertexListGraph<G> && IncidenceGraph<G> &&
             std::invocable<WeightFunc, edge_descriptor_t<G>>
auto prim_minimum_spanning_tree(const G& g, WeightFunc&& get_weight) {
    return prim_minimum_spanning_tree(g, vertex_descriptor_t<G>{0}, 
                                       std::forward<WeightFunc>(get_weight));
}

/// Compute minimum spanning tree with default unit weights.
///
/// @param g The graph
/// @param source The starting vertex
/// @return prim_result containing MST edges
///
template<typename G>
    requires VertexListGraph<G> && IncidenceGraph<G>
auto prim_minimum_spanning_tree(const G& g, vertex_descriptor_t<G> source) {
    return prim_minimum_spanning_tree(g, source, [](auto) { return 1.0; });
}

/// Compute minimum spanning tree with default unit weights from vertex 0.
///
template<typename G>
    requires VertexListGraph<G> && IncidenceGraph<G>
auto prim_minimum_spanning_tree(const G& g) {
    return prim_minimum_spanning_tree(g, vertex_descriptor_t<G>{0});
}

} // namespace bgl

#endif // BGL_MODERN_PRIM_MINIMUM_SPANNING_TREE_HPP
