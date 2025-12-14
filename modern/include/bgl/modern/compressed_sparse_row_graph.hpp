// BGL Modern - Compressed Sparse Row Graph Container
// C++20 implementation with concepts, ranges, and std::span
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_COMPRESSED_SPARSE_ROW_GRAPH_HPP
#define BGL_MODERN_COMPRESSED_SPARSE_ROW_GRAPH_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>

#include <vector>
#include <span>
#include <ranges>
#include <concepts>
#include <utility>
#include <cstddef>
#include <algorithm>
#include <iterator>

namespace bgl {

// =============================================================================
// CSR Edge Descriptor
// =============================================================================

/// Edge descriptor for CSR graph (edge index)
struct csr_edge_descriptor {
    std::size_t source_vertex;
    std::size_t edge_index;  // Global edge index
    std::size_t target_vertex;
    
    bool operator==(const csr_edge_descriptor&) const = default;
    auto operator<=>(const csr_edge_descriptor&) const = default;
};

// =============================================================================
// compressed_sparse_row_graph - Main Class Template
// =============================================================================

/// Modern C++20 Compressed Sparse Row (CSR) graph container.
///
/// CSR is an extremely memory-efficient representation for static graphs.
/// Edges are stored in a compact array with row pointers indicating where
/// each vertex's adjacency list begins.
///
/// Layout:
/// - row_start[v] = index in targets where vertex v's edges begin
/// - row_start[v+1] = index where vertex v's edges end
/// - targets[row_start[v]..row_start[v+1]] = adjacent vertices
///
/// Best for:
/// - Static graphs (edge structure doesn't change after construction)
/// - Large sparse graphs with millions of vertices
/// - Cache-efficient traversal
/// - Parallel algorithms
///
/// Template Parameters:
/// - DirectedS: directed_tag or undirected_tag (undirected stores each edge twice)
/// - VertexProperty: Property type stored per vertex (default: no_property)
/// - EdgeProperty: Property type stored per edge (default: no_property)
///
/// Example:
/// @code
///     // Edge list as (source, target) pairs
///     std::vector<std::pair<int, int>> edges = {{0,1}, {0,2}, {1,2}, {2,3}};
///     compressed_sparse_row_graph<directed_tag> g(4, edges);
///     
///     // Iterate edges with std::span
///     for (auto v : vertices(g)) {
///         std::span<const std::size_t> neighbors = g.adjacent_vertices(v);
///         for (auto u : neighbors) {
///             std::cout << v << " -> " << u << "\n";
///         }
///     }
/// @endcode
///
template<
    typename DirectedS = directed_tag,
    typename VertexProperty = no_property,
    typename EdgeProperty = no_property
>
class compressed_sparse_row_graph {
public:
    // -------------------------------------------------------------------------
    // Type Definitions
    // -------------------------------------------------------------------------
    
    using directed_category = DirectedS;
    using edge_parallel_category = allow_parallel_edge_tag;
    using traversal_category = void;
    
    using vertex_descriptor = std::size_t;
    using edge_descriptor = csr_edge_descriptor;
    
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
    
    using vertex_property_type = VertexProperty;
    using edge_property_type = EdgeProperty;
    
    static constexpr bool is_directed = std::same_as<DirectedS, directed_tag>;
    
private:
    // -------------------------------------------------------------------------
    // Internal Storage
    // -------------------------------------------------------------------------
    
    vertices_size_type num_vertices_ = 0;
    std::vector<std::size_t> row_start_;    // Size: num_vertices + 1
    std::vector<std::size_t> targets_;       // Size: num_edges (or 2*num_edges for undirected)
    std::vector<VertexProperty> vertex_properties_;
    std::vector<EdgeProperty> edge_properties_;
    edges_size_type num_edges_ = 0;  // Logical number of edges
    
public:
    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------
    
    compressed_sparse_row_graph() : row_start_(1, 0) {}
    
    /// Construct empty graph with n vertices
    explicit compressed_sparse_row_graph(vertices_size_type n)
        : num_vertices_(n)
        , row_start_(n + 1, 0)
        , vertex_properties_(n)
    {}
    
    /// Construct from edge list
    /// Each edge is a pair (source, target)
    template<std::ranges::input_range EdgeRange>
        requires std::convertible_to<std::ranges::range_value_t<EdgeRange>, 
                                      std::pair<vertex_descriptor, vertex_descriptor>>
    compressed_sparse_row_graph(vertices_size_type n, EdgeRange&& edges)
        : num_vertices_(n)
        , row_start_(n + 1, 0)
        , vertex_properties_(n)
    {
        build_from_edges(std::forward<EdgeRange>(edges));
    }
    
    /// Construct from edge list with edge properties
    template<std::ranges::input_range EdgeRange, std::ranges::input_range PropRange>
        requires std::convertible_to<std::ranges::range_value_t<EdgeRange>,
                                      std::pair<vertex_descriptor, vertex_descriptor>>
    compressed_sparse_row_graph(vertices_size_type n, EdgeRange&& edges, PropRange&& props)
        : num_vertices_(n)
        , row_start_(n + 1, 0)
        , vertex_properties_(n)
    {
        build_from_edges_with_props(std::forward<EdgeRange>(edges), 
                                     std::forward<PropRange>(props));
    }
    
private:
    template<std::ranges::input_range EdgeRange>
    void build_from_edges(EdgeRange&& edges) {
        // Collect edges into temporary storage
        std::vector<std::pair<vertex_descriptor, vertex_descriptor>> edge_list;
        for (auto&& e : edges) {
            edge_list.push_back(e);
            if constexpr (!is_directed) {
                if (e.first != e.second) {
                    edge_list.push_back({e.second, e.first});
                }
            }
        }
        
        num_edges_ = is_directed ? edge_list.size() : edge_list.size() / 2;
        
        // Sort by source vertex
        std::ranges::sort(edge_list, {}, &std::pair<vertex_descriptor, vertex_descriptor>::first);
        
        // Build row_start
        std::fill(row_start_.begin(), row_start_.end(), 0);
        for (const auto& e : edge_list) {
            ++row_start_[e.first + 1];
        }
        for (std::size_t i = 1; i <= num_vertices_; ++i) {
            row_start_[i] += row_start_[i - 1];
        }
        
        // Build targets
        targets_.resize(edge_list.size());
        for (const auto& e : edge_list) {
            targets_.push_back(e.second);
        }
        targets_.clear();
        targets_.reserve(edge_list.size());
        for (const auto& e : edge_list) {
            targets_.push_back(e.second);
        }
    }
    
    template<std::ranges::input_range EdgeRange, std::ranges::input_range PropRange>
    void build_from_edges_with_props(EdgeRange&& edges, PropRange&& props) {
        std::vector<std::tuple<vertex_descriptor, vertex_descriptor, EdgeProperty>> edge_list;
        
        auto prop_it = std::ranges::begin(props);
        for (auto&& e : edges) {
            EdgeProperty p = (prop_it != std::ranges::end(props)) ? *prop_it++ : EdgeProperty{};
            edge_list.push_back({e.first, e.second, p});
            if constexpr (!is_directed) {
                if (e.first != e.second) {
                    edge_list.push_back({e.second, e.first, p});
                }
            }
        }
        
        num_edges_ = is_directed ? edge_list.size() : edge_list.size() / 2;
        
        // Sort by source
        std::ranges::sort(edge_list, {}, [](const auto& t) { return std::get<0>(t); });
        
        // Build row_start
        std::fill(row_start_.begin(), row_start_.end(), 0);
        for (const auto& e : edge_list) {
            ++row_start_[std::get<0>(e) + 1];
        }
        for (std::size_t i = 1; i <= num_vertices_; ++i) {
            row_start_[i] += row_start_[i - 1];
        }
        
        // Build targets and properties
        targets_.reserve(edge_list.size());
        edge_properties_.reserve(edge_list.size());
        for (const auto& e : edge_list) {
            targets_.push_back(std::get<1>(e));
            edge_properties_.push_back(std::get<2>(e));
        }
    }
    
public:
    // -------------------------------------------------------------------------
    // Vertex Operations
    // -------------------------------------------------------------------------
    
    /// Get vertex property (const)
    const VertexProperty& operator[](vertex_descriptor v) const
        requires (!std::same_as<VertexProperty, no_property>)
    {
        return vertex_properties_[v];
    }
    
    /// Get vertex property (mutable)
    VertexProperty& operator[](vertex_descriptor v)
        requires (!std::same_as<VertexProperty, no_property>)
    {
        return vertex_properties_[v];
    }
    
    // -------------------------------------------------------------------------
    // Edge Operations with std::span
    // -------------------------------------------------------------------------
    
    /// Get adjacent vertices as a span (zero-copy view)
    [[nodiscard]] std::span<const std::size_t> adjacent_vertices(vertex_descriptor v) const {
        auto begin = row_start_[v];
        auto end = row_start_[v + 1];
        return std::span<const std::size_t>(targets_.data() + begin, end - begin);
    }
    
    /// Get edge properties for out-edges as a span
    [[nodiscard]] std::span<const EdgeProperty> adjacent_edge_properties(vertex_descriptor v) const
        requires (!std::same_as<EdgeProperty, no_property>)
    {
        auto begin = row_start_[v];
        auto end = row_start_[v + 1];
        return std::span<const EdgeProperty>(edge_properties_.data() + begin, end - begin);
    }
    
    /// Get edge property (const)
    const EdgeProperty& operator[](edge_descriptor e) const
        requires (!std::same_as<EdgeProperty, no_property>)
    {
        return edge_properties_[e.edge_index];
    }
    
    /// Get edge property (mutable)
    EdgeProperty& operator[](edge_descriptor e)
        requires (!std::same_as<EdgeProperty, no_property>)
    {
        return edge_properties_[e.edge_index];
    }
    
    // -------------------------------------------------------------------------
    // Raw Data Access (for algorithms needing direct memory access)
    // -------------------------------------------------------------------------
    
    /// Get row_start array as span
    [[nodiscard]] std::span<const std::size_t> row_start() const {
        return std::span<const std::size_t>(row_start_);
    }
    
    /// Get targets array as span
    [[nodiscard]] std::span<const std::size_t> targets() const {
        return std::span<const std::size_t>(targets_);
    }
    
    /// Get edge properties as span
    [[nodiscard]] std::span<const EdgeProperty> edge_properties() const
        requires (!std::same_as<EdgeProperty, no_property>)
    {
        return std::span<const EdgeProperty>(edge_properties_);
    }
    
    // -------------------------------------------------------------------------
    // Size Queries
    // -------------------------------------------------------------------------
    
    [[nodiscard]] vertices_size_type num_vertices() const { return num_vertices_; }
    [[nodiscard]] edges_size_type num_edges() const { return num_edges_; }
    
    [[nodiscard]] degree_size_type out_degree(vertex_descriptor v) const {
        return row_start_[v + 1] - row_start_[v];
    }
};

// =============================================================================
// Free Functions (ADL-findable)
// =============================================================================

// Vertex iteration
template<typename D, typename VP, typename EP>
auto vertices(const compressed_sparse_row_graph<D, VP, EP>& g) {
    return std::views::iota(std::size_t{0}, g.num_vertices());
}

template<typename D, typename VP, typename EP>
auto num_vertices(const compressed_sparse_row_graph<D, VP, EP>& g) {
    return g.num_vertices();
}

template<typename D, typename VP, typename EP>
auto num_edges(const compressed_sparse_row_graph<D, VP, EP>& g) {
    return g.num_edges();
}

// Out-edge iteration
template<typename D, typename VP, typename EP>
auto out_edges(typename compressed_sparse_row_graph<D, VP, EP>::vertex_descriptor v,
               const compressed_sparse_row_graph<D, VP, EP>& g) {
    using edge_descriptor = typename compressed_sparse_row_graph<D, VP, EP>::edge_descriptor;
    
    auto row_data = g.row_start();
    auto start = row_data[v];
    auto count = row_data[v + 1] - start;
    auto targets = g.adjacent_vertices(v);
    
    return std::views::iota(std::size_t{0}, count)
        | std::views::transform([v, start, targets](auto i) {
            return edge_descriptor{v, start + i, targets[i]};
        });
}

template<typename D, typename VP, typename EP>
auto out_degree(typename compressed_sparse_row_graph<D, VP, EP>::vertex_descriptor v,
                const compressed_sparse_row_graph<D, VP, EP>& g) {
    return g.out_degree(v);
}

// Edge source/target
template<typename D, typename VP, typename EP>
auto source(typename compressed_sparse_row_graph<D, VP, EP>::edge_descriptor e,
            const compressed_sparse_row_graph<D, VP, EP>&) {
    return e.source_vertex;
}

template<typename D, typename VP, typename EP>
auto target(typename compressed_sparse_row_graph<D, VP, EP>::edge_descriptor e,
            const compressed_sparse_row_graph<D, VP, EP>&) {
    return e.target_vertex;
}

// =============================================================================
// Graph Traits Specialization
// =============================================================================

template<typename D, typename VP, typename EP>
struct graph_traits<compressed_sparse_row_graph<D, VP, EP>> {
    using graph_type = compressed_sparse_row_graph<D, VP, EP>;
    
    using vertex_descriptor = typename graph_type::vertex_descriptor;
    using edge_descriptor = typename graph_type::edge_descriptor;
    using directed_category = typename graph_type::directed_category;
    using edge_parallel_category = typename graph_type::edge_parallel_category;
    using traversal_category = typename graph_type::traversal_category;
    using vertices_size_type = typename graph_type::vertices_size_type;
    using edges_size_type = typename graph_type::edges_size_type;
    using degree_size_type = typename graph_type::degree_size_type;
};

} // namespace bgl

// Hash for edge descriptor
template<>
struct std::hash<bgl::csr_edge_descriptor> {
    std::size_t operator()(const bgl::csr_edge_descriptor& e) const noexcept {
        return std::hash<std::size_t>{}(e.edge_index);
    }
};

#endif // BGL_MODERN_COMPRESSED_SPARSE_ROW_GRAPH_HPP
