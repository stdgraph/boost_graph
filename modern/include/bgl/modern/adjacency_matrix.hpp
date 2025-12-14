// BGL Modern - Adjacency Matrix Graph Container
// C++20 implementation with concepts and ranges
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_ADJACENCY_MATRIX_HPP
#define BGL_MODERN_ADJACENCY_MATRIX_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>

#include <vector>
#include <ranges>
#include <concepts>
#include <utility>
#include <cstddef>
#include <optional>

namespace bgl {

// =============================================================================
// Matrix Edge Descriptor
// =============================================================================

/// Edge descriptor for adjacency_matrix (source, target pair)
template<typename VertexDescriptor>
struct matrix_edge {
    VertexDescriptor source;
    VertexDescriptor target;
    
    bool operator==(const matrix_edge&) const = default;
    auto operator<=>(const matrix_edge&) const = default;
};

// =============================================================================
// adjacency_matrix - Main Class Template
// =============================================================================

/// Modern C++20 adjacency matrix graph container.
///
/// An adjacency matrix represents a graph as an NxN matrix where element (i,j)
/// indicates whether there's an edge from vertex i to vertex j. This provides
/// O(1) edge lookup but uses O(V²) space.
///
/// Best for:
/// - Dense graphs where E ≈ V²
/// - Algorithms requiring frequent edge existence queries
/// - Small to medium sized graphs
///
/// Template Parameters:
/// - DirectedS: directed_tag or undirected_tag
/// - VertexProperty: Property type stored per vertex (default: no_property)
/// - EdgeProperty: Property type stored per edge (default: no_property)
///
/// Example:
/// @code
///     adjacency_matrix<directed_tag> g(100);  // 100 vertices
///     g.add_edge(0, 1);
///     g.add_edge(1, 2);
///     
///     if (g.has_edge(0, 1)) {
///         std::cout << "Edge exists!\n";
///     }
/// @endcode
///
template<
    typename DirectedS = directed_tag,
    typename VertexProperty = no_property,
    typename EdgeProperty = no_property
>
class adjacency_matrix {
public:
    // -------------------------------------------------------------------------
    // Type Definitions
    // -------------------------------------------------------------------------
    
    using directed_category = DirectedS;
    using edge_parallel_category = disallow_parallel_edge_tag;
    using traversal_category = void;
    
    using vertex_descriptor = std::size_t;
    using edge_descriptor = matrix_edge<vertex_descriptor>;
    
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
    
    struct edge_data {
        bool exists = false;
        [[no_unique_address]] EdgeProperty property;
    };
    
    vertices_size_type num_vertices_ = 0;
    std::vector<edge_data> matrix_;  // Flattened NxN matrix
    std::vector<VertexProperty> vertex_properties_;
    edges_size_type num_edges_ = 0;
    
    // Convert (u, v) to linear index
    [[nodiscard]] constexpr std::size_t index(vertex_descriptor u, vertex_descriptor v) const {
        return u * num_vertices_ + v;
    }
    
public:
    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------
    
    adjacency_matrix() = default;
    
    /// Construct with n vertices
    explicit adjacency_matrix(vertices_size_type n)
        : num_vertices_(n)
        , matrix_(n * n)
        , vertex_properties_(n)
    {}
    
    /// Construct with n vertices and initial vertex property
    adjacency_matrix(vertices_size_type n, const VertexProperty& vp)
        : num_vertices_(n)
        , matrix_(n * n)
        , vertex_properties_(n, vp)
    {}
    
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
    // Edge Operations
    // -------------------------------------------------------------------------
    
    /// Check if edge exists
    [[nodiscard]] bool has_edge(vertex_descriptor u, vertex_descriptor v) const {
        return matrix_[index(u, v)].exists;
    }
    
    /// Add an edge (returns false if edge already exists)
    std::pair<edge_descriptor, bool> add_edge(vertex_descriptor u, vertex_descriptor v) {
        auto idx = index(u, v);
        if (matrix_[idx].exists) {
            return {{u, v}, false};
        }
        
        matrix_[idx].exists = true;
        ++num_edges_;
        
        // For undirected, also set (v, u)
        if constexpr (!is_directed) {
            if (u != v) {
                matrix_[index(v, u)].exists = true;
            }
        }
        
        return {{u, v}, true};
    }
    
    /// Add an edge with property
    std::pair<edge_descriptor, bool> add_edge(vertex_descriptor u, vertex_descriptor v,
                                               const EdgeProperty& prop) {
        auto idx = index(u, v);
        if (matrix_[idx].exists) {
            return {{u, v}, false};
        }
        
        matrix_[idx].exists = true;
        matrix_[idx].property = prop;
        ++num_edges_;
        
        if constexpr (!is_directed) {
            if (u != v) {
                auto ridx = index(v, u);
                matrix_[ridx].exists = true;
                matrix_[ridx].property = prop;
            }
        }
        
        return {{u, v}, true};
    }
    
    /// Remove an edge
    void remove_edge(vertex_descriptor u, vertex_descriptor v) {
        auto idx = index(u, v);
        if (matrix_[idx].exists) {
            matrix_[idx].exists = false;
            matrix_[idx].property = EdgeProperty{};
            --num_edges_;
            
            if constexpr (!is_directed) {
                if (u != v) {
                    auto ridx = index(v, u);
                    matrix_[ridx].exists = false;
                    matrix_[ridx].property = EdgeProperty{};
                }
            }
        }
    }
    
    /// Get edge property (const)
    const EdgeProperty& operator[](edge_descriptor e) const
        requires (!std::same_as<EdgeProperty, no_property>)
    {
        return matrix_[index(e.source, e.target)].property;
    }
    
    /// Get edge property (mutable)
    EdgeProperty& operator[](edge_descriptor e)
        requires (!std::same_as<EdgeProperty, no_property>)
    {
        return matrix_[index(e.source, e.target)].property;
    }
    
    /// Get edge if it exists
    [[nodiscard]] std::optional<edge_descriptor> edge(vertex_descriptor u, vertex_descriptor v) const {
        if (has_edge(u, v)) {
            return edge_descriptor{u, v};
        }
        return std::nullopt;
    }
    
    // -------------------------------------------------------------------------
    // Size Queries
    // -------------------------------------------------------------------------
    
    [[nodiscard]] vertices_size_type num_vertices() const { return num_vertices_; }
    [[nodiscard]] edges_size_type num_edges() const { return num_edges_; }
    
    [[nodiscard]] degree_size_type out_degree(vertex_descriptor v) const {
        degree_size_type count = 0;
        for (vertex_descriptor u = 0; u < num_vertices_; ++u) {
            if (matrix_[index(v, u)].exists) ++count;
        }
        return count;
    }
    
    [[nodiscard]] degree_size_type in_degree(vertex_descriptor v) const {
        degree_size_type count = 0;
        for (vertex_descriptor u = 0; u < num_vertices_; ++u) {
            if (matrix_[index(u, v)].exists) ++count;
        }
        return count;
    }
    
    // -------------------------------------------------------------------------
    // Internal Access for Free Functions
    // -------------------------------------------------------------------------
    
    [[nodiscard]] const std::vector<edge_data>& matrix() const { return matrix_; }
};

// =============================================================================
// Free Functions (ADL-findable)
// =============================================================================

// Vertex iteration
template<typename D, typename VP, typename EP>
auto vertices(const adjacency_matrix<D, VP, EP>& g) {
    return std::views::iota(std::size_t{0}, g.num_vertices());
}

template<typename D, typename VP, typename EP>
auto num_vertices(const adjacency_matrix<D, VP, EP>& g) {
    return g.num_vertices();
}

template<typename D, typename VP, typename EP>
auto num_edges(const adjacency_matrix<D, VP, EP>& g) {
    return g.num_edges();
}

// Out-edge iteration
template<typename D, typename VP, typename EP>
auto out_edges(typename adjacency_matrix<D, VP, EP>::vertex_descriptor v,
               const adjacency_matrix<D, VP, EP>& g) {
    using edge_descriptor = typename adjacency_matrix<D, VP, EP>::edge_descriptor;
    
    return std::views::iota(std::size_t{0}, g.num_vertices())
        | std::views::filter([v, &g](auto u) { return g.has_edge(v, u); })
        | std::views::transform([v](auto u) { return edge_descriptor{v, u}; });
}

template<typename D, typename VP, typename EP>
auto out_degree(typename adjacency_matrix<D, VP, EP>::vertex_descriptor v,
                const adjacency_matrix<D, VP, EP>& g) {
    return g.out_degree(v);
}

// In-edge iteration
template<typename D, typename VP, typename EP>
auto in_edges(typename adjacency_matrix<D, VP, EP>::vertex_descriptor v,
              const adjacency_matrix<D, VP, EP>& g) {
    using edge_descriptor = typename adjacency_matrix<D, VP, EP>::edge_descriptor;
    
    return std::views::iota(std::size_t{0}, g.num_vertices())
        | std::views::filter([v, &g](auto u) { return g.has_edge(u, v); })
        | std::views::transform([v](auto u) { return edge_descriptor{u, v}; });
}

template<typename D, typename VP, typename EP>
auto in_degree(typename adjacency_matrix<D, VP, EP>::vertex_descriptor v,
               const adjacency_matrix<D, VP, EP>& g) {
    return g.in_degree(v);
}

// Edge source/target
template<typename D, typename VP, typename EP>
auto source(typename adjacency_matrix<D, VP, EP>::edge_descriptor e,
            const adjacency_matrix<D, VP, EP>&) {
    return e.source;
}

template<typename D, typename VP, typename EP>
auto target(typename adjacency_matrix<D, VP, EP>::edge_descriptor e,
            const adjacency_matrix<D, VP, EP>&) {
    return e.target;
}

// Edge lookup
template<typename D, typename VP, typename EP>
auto edge(typename adjacency_matrix<D, VP, EP>::vertex_descriptor u,
          typename adjacency_matrix<D, VP, EP>::vertex_descriptor v,
          const adjacency_matrix<D, VP, EP>& g) {
    return g.edge(u, v);
}

// =============================================================================
// Graph Traits Specialization
// =============================================================================

template<typename D, typename VP, typename EP>
struct graph_traits<adjacency_matrix<D, VP, EP>> {
    using graph_type = adjacency_matrix<D, VP, EP>;
    
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
template<typename V>
struct std::hash<bgl::matrix_edge<V>> {
    std::size_t operator()(const bgl::matrix_edge<V>& e) const noexcept {
        return std::hash<V>{}(e.source) ^ (std::hash<V>{}(e.target) << 1);
    }
};

#endif // BGL_MODERN_ADJACENCY_MATRIX_HPP
