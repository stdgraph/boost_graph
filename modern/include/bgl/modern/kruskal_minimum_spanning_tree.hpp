// BGL Modern - Kruskal's Minimum Spanning Tree Algorithm
// C++20 implementation of Kruskal's MST algorithm
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_KRUSKAL_MINIMUM_SPANNING_TREE_HPP
#define BGL_MODERN_KRUSKAL_MINIMUM_SPANNING_TREE_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>
#include <bgl/modern/property_map.hpp>

#include <algorithm>
#include <vector>
#include <numeric>
#include <ranges>
#include <concepts>
#include <type_traits>

namespace bgl {

// =============================================================================
// Disjoint Set (Union-Find) Implementation
// =============================================================================

/// Simple disjoint set (union-find) data structure for Kruskal's algorithm.
///
/// Uses path compression and union by rank for near-constant time operations.
///
template<typename Vertex>
    requires std::integral<Vertex>
class disjoint_sets {
public:
    explicit disjoint_sets(std::size_t n)
        : parent_(n)
        , rank_(n, 0)
    {
        std::iota(parent_.begin(), parent_.end(), Vertex{0});
    }
    
    /// Find the representative of the set containing x (with path compression)
    Vertex find_set(Vertex x) {
        if (parent_[x] != x) {
            parent_[x] = find_set(parent_[x]);  // Path compression
        }
        return parent_[x];
    }
    
    /// Union the sets containing x and y (by rank)
    void union_sets(Vertex x, Vertex y) {
        Vertex px = find_set(x);
        Vertex py = find_set(y);
        
        if (px == py) return;
        
        // Union by rank
        if (rank_[px] < rank_[py]) {
            parent_[px] = py;
        } else if (rank_[px] > rank_[py]) {
            parent_[py] = px;
        } else {
            parent_[py] = px;
            ++rank_[px];
        }
    }
    
    /// Check if x and y are in the same set
    bool same_set(Vertex x, Vertex y) {
        return find_set(x) == find_set(y);
    }
    
private:
    std::vector<Vertex> parent_;
    std::vector<std::size_t> rank_;
};

// =============================================================================
// mst_result - Result Type for MST Algorithms
// =============================================================================

/// Result type for minimum spanning tree algorithms.
///
/// This struct holds the computed MST edges and total weight.
/// It provides convenient accessors for querying MST information.
///
/// Usage:
/// @code
///     auto result = kruskal_minimum_spanning_tree(g, weight_map);
///     
///     // Get the total weight of the MST
///     double total = result.total_weight();
///     
///     // Iterate over MST edges
///     for (auto e : result.edges()) {
///         std::cout << source(e, g) << " -- " << target(e, g) << "\n";
///     }
///     
///     // Check if the graph is connected (has a spanning tree)
///     if (result.is_spanning_tree()) {
///         // MST spans all vertices
///     }
/// @endcode
///
/// @tparam G The graph type
/// @tparam WeightType The type used for edge weights (default: double)
///
template<typename G, typename WeightType = double>
class mst_result {
public:
    using graph_type = G;
    using vertex_descriptor = vertex_descriptor_t<G>;
    using edge_descriptor = edge_descriptor_t<G>;
    using weight_type = WeightType;
    using size_type = std::size_t;
    
    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------
    
    mst_result() = default;
    
    /// Construct with expected number of vertices (for capacity hint)
    explicit mst_result(size_type num_vertices)
        : num_vertices_(num_vertices)
    {
        // MST has V-1 edges
        if (num_vertices > 0) {
            edges_.reserve(num_vertices - 1);
        }
    }
    
    // -------------------------------------------------------------------------
    // Building the Result (for algorithm use)
    // -------------------------------------------------------------------------
    
    /// Add an edge to the MST
    void add_edge(edge_descriptor e, weight_type w) {
        edges_.push_back(e);
        total_weight_ += w;
    }
    
    /// Set the number of vertices in the original graph
    void set_num_vertices(size_type n) {
        num_vertices_ = n;
    }
    
    // -------------------------------------------------------------------------
    // Query Interface
    // -------------------------------------------------------------------------
    
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
    /// A spanning tree of V vertices has exactly V-1 edges
    bool is_spanning_tree() const {
        return num_vertices_ > 0 && edges_.size() == num_vertices_ - 1;
    }
    
    /// Check if the MST is empty
    bool empty() const {
        return edges_.empty();
    }
    
    /// Get the number of connected components in the original graph
    /// (V - num_edges gives the number of trees in the forest)
    size_type num_components() const {
        if (num_vertices_ == 0) return 0;
        return num_vertices_ - edges_.size();
    }
    
private:
    std::vector<edge_descriptor> edges_;
    weight_type total_weight_ = weight_type{0};
    size_type num_vertices_ = 0;
};

// =============================================================================
// Factory Functions
// =============================================================================

template<typename G, typename WeightType = double>
auto make_mst_result(const G& g) {
    return mst_result<G, WeightType>(num_vertices(g));
}

// =============================================================================
// kruskal_minimum_spanning_tree - Core Implementation
// =============================================================================

namespace detail {

/// Core Kruskal's MST implementation
template<typename G, typename WeightType, typename WeightFunc>
void kruskal_mst_impl(
    const G& g,
    mst_result<G, WeightType>& result,
    WeightFunc&& get_weight
) {
    using vertex_descriptor = vertex_descriptor_t<G>;
    using edge_descriptor = edge_descriptor_t<G>;
    using weight_type = WeightType;
    
    const auto n = num_vertices(g);
    if (n == 0) return;
    
    result.set_num_vertices(n);
    
    // Collect all edges with their weights
    std::vector<std::pair<weight_type, edge_descriptor>> weighted_edges;
    weighted_edges.reserve(num_edges(g));
    
    for (auto e : edges(g)) {
        weighted_edges.emplace_back(get_weight(e), e);
    }
    
    // Sort edges by weight (ascending)
    std::ranges::sort(weighted_edges, [](const auto& a, const auto& b) {
        return a.first < b.first;
    });
    
    // Initialize disjoint sets
    disjoint_sets<vertex_descriptor> dsets(n);
    
    // Process edges in order of increasing weight
    for (const auto& [weight, e] : weighted_edges) {
        vertex_descriptor u = source(e, g);
        vertex_descriptor v = target(e, g);
        
        // If u and v are in different components, add edge to MST
        if (!dsets.same_set(u, v)) {
            result.add_edge(e, weight);
            dsets.union_sets(u, v);
            
            // Early termination: MST complete when we have V-1 edges
            if (result.num_edges() == n - 1) {
                break;
            }
        }
    }
}

} // namespace detail

// =============================================================================
// kruskal_minimum_spanning_tree - Public Interface
// =============================================================================

/// Compute minimum spanning tree using Kruskal's algorithm.
///
/// Kruskal's algorithm is a greedy algorithm that finds a minimum spanning
/// forest (or tree if the graph is connected) by processing edges in order
/// of increasing weight and adding them if they don't create a cycle.
///
/// Requirements:
/// - G must satisfy VertexListGraph and EdgeListGraph concepts
/// - WeightFunc must be invocable with edge_descriptor and return a weight
///
/// Complexity: O(E log E) for sorting edges + O(E α(V)) for union-find
///             where α is the inverse Ackermann function (nearly constant)
///
/// @param g The graph (should be undirected for MST)
/// @param get_weight Function to get edge weight: (edge) -> weight
/// @return mst_result containing MST edges and total weight
///
/// Example:
/// @code
///     // With lambda weight accessor
///     auto result = kruskal_minimum_spanning_tree(g, [&](auto e) {
///         return g[e].weight;
///     });
///     
///     std::cout << "MST weight: " << result.total_weight() << "\n";
///     std::cout << "MST edges:\n";
///     for (auto e : result.edges()) {
///         std::cout << "  " << source(e, g) << " -- " << target(e, g) << "\n";
///     }
/// @endcode
///
template<typename G, typename WeightFunc>
    requires VertexListGraph<G> && EdgeListGraph<G> &&
             std::invocable<WeightFunc, edge_descriptor_t<G>>
auto kruskal_minimum_spanning_tree(const G& g, WeightFunc&& get_weight) {
    using weight_type = std::invoke_result_t<WeightFunc, edge_descriptor_t<G>>;
    
    auto result = make_mst_result<G, weight_type>(g);
    detail::kruskal_mst_impl(g, result, std::forward<WeightFunc>(get_weight));
    return result;
}

/// Compute minimum spanning tree with default unit weights.
///
/// This overload uses unit weights (1.0) for all edges, effectively finding
/// a spanning tree with minimum number of edges (any spanning tree).
///
/// @param g The graph
/// @return mst_result containing MST edges
///
template<typename G>
    requires VertexListGraph<G> && EdgeListGraph<G>
auto kruskal_minimum_spanning_tree(const G& g) {
    return kruskal_minimum_spanning_tree(g, [](auto) { return 1.0; });
}

} // namespace bgl

#endif // BGL_MODERN_KRUSKAL_MINIMUM_SPANNING_TREE_HPP
