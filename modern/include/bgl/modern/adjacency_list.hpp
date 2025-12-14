// BGL Modern - Adjacency List Graph Container
// C++20 implementation with concepts and ranges
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_ADJACENCY_LIST_HPP
#define BGL_MODERN_ADJACENCY_LIST_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>

#include <vector>
#include <ranges>
#include <concepts>
#include <utility>
#include <cstddef>
#include <functional>
#include <optional>

namespace bgl {

// =============================================================================
// Graph Selector Tags
// =============================================================================

// Directed/Undirected tags already defined in graph_traits.hpp:
// - directed_tag
// - undirected_tag
// - bidirectional_tag

/// Selector for vertex container type
struct vecS {};  // std::vector
struct listS {}; // std::list (TODO)
struct setS {};  // std::set (TODO)

/// Empty property placeholder
struct no_property {};

// =============================================================================
// Edge Descriptor
// =============================================================================

/// Edge descriptor for adjacency_list
template<typename VertexDescriptor>
struct adjacency_list_edge {
    VertexDescriptor source;
    VertexDescriptor target;
    std::size_t edge_index;
    
    bool operator==(const adjacency_list_edge&) const = default;
    auto operator<=>(const adjacency_list_edge&) const = default;
};

// =============================================================================
// adjacency_list - Main Class Template
// =============================================================================

/// Modern C++20 adjacency list graph container.
///
/// Template Parameters:
/// - DirectedS: directed_tag, undirected_tag, or bidirectional_tag
/// - VertexProperty: Property type stored per vertex (default: no_property)
/// - EdgeProperty: Property type stored per edge (default: no_property)
/// - GraphProperty: Property type for the whole graph (default: no_property)
///
/// Example usage:
/// @code
///     // Simple graph with no properties
///     adjacency_list<directed_tag> g;
///     
///     // Graph with vertex and edge properties
///     struct VertexData { std::string name; int id; };
///     struct EdgeData { double weight; };
///     adjacency_list<directed_tag, VertexData, EdgeData> g2;
///     
///     auto v = g2.add_vertex({.name = "A", .id = 1});
///     auto u = g2.add_vertex({.name = "B", .id = 2});
///     auto e = g2.add_edge(v, u, {.weight = 1.5});
///     
///     g2[v].name = "Updated";
///     g2[e].weight = 2.0;
/// @endcode
///
template<
    typename DirectedS = directed_tag,
    typename VertexProperty = no_property,
    typename EdgeProperty = no_property,
    typename GraphProperty = no_property
>
class adjacency_list {
public:
    // -------------------------------------------------------------------------
    // Type Definitions
    // -------------------------------------------------------------------------
    
    using directed_category = DirectedS;
    using edge_parallel_category = allow_parallel_edge_tag;
    using traversal_category = void;  // Supports multiple traversal patterns
    
    using vertex_descriptor = std::size_t;
    using edge_descriptor = adjacency_list_edge<vertex_descriptor>;
    
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
    
    using vertex_property_type = VertexProperty;
    using edge_property_type = EdgeProperty;
    using graph_property_type = GraphProperty;
    
    static constexpr bool is_directed = 
        std::same_as<DirectedS, directed_tag> || 
        std::same_as<DirectedS, bidirectional_tag>;
    
    static constexpr bool is_bidirectional = 
        std::same_as<DirectedS, bidirectional_tag>;
    
private:
    // -------------------------------------------------------------------------
    // Internal Storage
    // -------------------------------------------------------------------------
    
    struct stored_edge {
        vertex_descriptor target;
        std::size_t edge_index;
        
        // For bidirectional graphs, we need to track reverse edges
        // stored_edge* reverse = nullptr;  // TODO for bidirectional
    };
    
    struct vertex_data {
        std::vector<stored_edge> out_edges;
        std::vector<stored_edge> in_edges;  // Only used for bidirectional
        [[no_unique_address]] VertexProperty property;
    };
    
    struct edge_data {
        vertex_descriptor source;
        vertex_descriptor target;
        [[no_unique_address]] EdgeProperty property;
    };
    
    std::vector<vertex_data> vertices_;
    std::vector<edge_data> edges_;
    [[no_unique_address]] GraphProperty graph_property_;
    
public:
    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------
    
    adjacency_list() = default;
    
    /// Construct with n vertices (each with default-constructed properties)
    explicit adjacency_list(vertices_size_type n) 
        : vertices_(n)
    {}
    
    /// Construct with n vertices and initial vertex property
    adjacency_list(vertices_size_type n, const VertexProperty& vp)
        : vertices_(n)
    {
        for (auto& v : vertices_) {
            v.property = vp;
        }
    }
    
    // -------------------------------------------------------------------------
    // Vertex Operations
    // -------------------------------------------------------------------------
    
    /// Add a vertex with default property
    vertex_descriptor add_vertex() {
        vertices_.emplace_back();
        return vertices_.size() - 1;
    }
    
    /// Add a vertex with the given property
    vertex_descriptor add_vertex(const VertexProperty& prop) {
        vertices_.emplace_back();
        vertices_.back().property = prop;
        return vertices_.size() - 1;
    }
    
    /// Add a vertex with moved property
    vertex_descriptor add_vertex(VertexProperty&& prop) {
        vertices_.emplace_back();
        vertices_.back().property = std::move(prop);
        return vertices_.size() - 1;
    }
    
    /// Get vertex property (const)
    const VertexProperty& operator[](vertex_descriptor v) const 
        requires (!std::same_as<VertexProperty, no_property>)
    {
        return vertices_[v].property;
    }
    
    /// Get vertex property (mutable)
    VertexProperty& operator[](vertex_descriptor v)
        requires (!std::same_as<VertexProperty, no_property>)
    {
        return vertices_[v].property;
    }
    
    // -------------------------------------------------------------------------
    // Edge Operations
    // -------------------------------------------------------------------------
    
    /// Add an edge with default property
    std::pair<edge_descriptor, bool> add_edge(vertex_descriptor u, vertex_descriptor v) {
        std::size_t edge_idx = edges_.size();
        edges_.push_back({u, v, EdgeProperty{}});
        vertices_[u].out_edges.push_back({v, edge_idx});
        
        // For undirected graphs, add reverse edge to adjacency
        if constexpr (!is_directed) {
            vertices_[v].out_edges.push_back({u, edge_idx});
        }
        
        // For bidirectional graphs, track in-edges
        if constexpr (is_bidirectional) {
            vertices_[v].in_edges.push_back({u, edge_idx});
        }
        
        return {{u, v, edge_idx}, true};
    }
    
    /// Add an edge with the given property
    std::pair<edge_descriptor, bool> add_edge(vertex_descriptor u, vertex_descriptor v, 
                                               const EdgeProperty& prop) {
        std::size_t edge_idx = edges_.size();
        edges_.push_back({u, v, prop});
        vertices_[u].out_edges.push_back({v, edge_idx});
        
        if constexpr (!is_directed) {
            vertices_[v].out_edges.push_back({u, edge_idx});
        }
        
        if constexpr (is_bidirectional) {
            vertices_[v].in_edges.push_back({u, edge_idx});
        }
        
        return {{u, v, edge_idx}, true};
    }
    
    /// Add an edge with moved property
    std::pair<edge_descriptor, bool> add_edge(vertex_descriptor u, vertex_descriptor v,
                                               EdgeProperty&& prop) {
        std::size_t edge_idx = edges_.size();
        edges_.push_back({u, v, std::move(prop)});
        vertices_[u].out_edges.push_back({v, edge_idx});
        
        if constexpr (!is_directed) {
            vertices_[v].out_edges.push_back({u, edge_idx});
        }
        
        if constexpr (is_bidirectional) {
            vertices_[v].in_edges.push_back({u, edge_idx});
        }
        
        return {{u, v, edge_idx}, true};
    }
    
    /// Get edge property (const)
    const EdgeProperty& operator[](edge_descriptor e) const
        requires (!std::same_as<EdgeProperty, no_property>)
    {
        return edges_[e.edge_index].property;
    }
    
    /// Get edge property (mutable)
    EdgeProperty& operator[](edge_descriptor e)
        requires (!std::same_as<EdgeProperty, no_property>)
    {
        return edges_[e.edge_index].property;
    }
    
    // -------------------------------------------------------------------------
    // Graph Property
    // -------------------------------------------------------------------------
    
    const GraphProperty& graph_property() const 
        requires (!std::same_as<GraphProperty, no_property>)
    {
        return graph_property_;
    }
    
    GraphProperty& graph_property()
        requires (!std::same_as<GraphProperty, no_property>)
    {
        return graph_property_;
    }
    
    // -------------------------------------------------------------------------
    // Size Queries
    // -------------------------------------------------------------------------
    
    vertices_size_type num_vertices() const {
        return vertices_.size();
    }
    
    edges_size_type num_edges() const {
        return edges_.size();
    }
    
    degree_size_type out_degree(vertex_descriptor v) const {
        return vertices_[v].out_edges.size();
    }
    
    degree_size_type in_degree(vertex_descriptor v) const
        requires (is_bidirectional || !is_directed)
    {
        if constexpr (is_bidirectional) {
            return vertices_[v].in_edges.size();
        } else {
            // For undirected, in_degree == out_degree
            return vertices_[v].out_edges.size();
        }
    }
    
    degree_size_type degree(vertex_descriptor v) const
        requires (!is_directed || is_bidirectional)
    {
        if constexpr (is_bidirectional) {
            return out_degree(v) + in_degree(v);
        } else {
            return out_degree(v);
        }
    }
    
    // -------------------------------------------------------------------------
    // Iterators and Ranges (Friend functions defined below)
    // -------------------------------------------------------------------------
    
    // Internal access for friend functions
    const std::vector<vertex_data>& vertices_storage() const { return vertices_; }
    const std::vector<edge_data>& edges_storage() const { return edges_; }
};

// =============================================================================
// Free Functions (ADL-findable)
// =============================================================================

// Vertex iteration
template<typename D, typename VP, typename EP, typename GP>
auto vertices(const adjacency_list<D, VP, EP, GP>& g) {
    return std::views::iota(std::size_t{0}, g.num_vertices());
}

template<typename D, typename VP, typename EP, typename GP>
auto num_vertices(const adjacency_list<D, VP, EP, GP>& g) {
    return g.num_vertices();
}

// Edge iteration
template<typename D, typename VP, typename EP, typename GP>
auto num_edges(const adjacency_list<D, VP, EP, GP>& g) {
    return g.num_edges();
}

// Out-edge iteration
template<typename D, typename VP, typename EP, typename GP>
auto out_edges(typename adjacency_list<D, VP, EP, GP>::vertex_descriptor v,
               const adjacency_list<D, VP, EP, GP>& g) {
    using graph_type = adjacency_list<D, VP, EP, GP>;
    using edge_descriptor = typename graph_type::edge_descriptor;
    
    return g.vertices_storage()[v].out_edges 
        | std::views::transform([v, &g](const auto& se) {
            // Get correct source for undirected graphs
            auto src = g.edges_storage()[se.edge_index].source;
            auto tgt = g.edges_storage()[se.edge_index].target;
            // For undirected, ensure consistent source/target relative to v
            if constexpr (!graph_type::is_directed) {
                if (src != v) std::swap(src, tgt);
            }
            return edge_descriptor{src, tgt, se.edge_index};
        });
}

template<typename D, typename VP, typename EP, typename GP>
auto out_degree(typename adjacency_list<D, VP, EP, GP>::vertex_descriptor v,
                const adjacency_list<D, VP, EP, GP>& g) {
    return g.out_degree(v);
}

// In-edge iteration (bidirectional only)
template<typename D, typename VP, typename EP, typename GP>
    requires (adjacency_list<D, VP, EP, GP>::is_bidirectional)
auto in_edges(typename adjacency_list<D, VP, EP, GP>::vertex_descriptor v,
              const adjacency_list<D, VP, EP, GP>& g) {
    using graph_type = adjacency_list<D, VP, EP, GP>;
    using edge_descriptor = typename graph_type::edge_descriptor;
    
    return g.vertices_storage()[v].in_edges
        | std::views::transform([v, &g](const auto& se) {
            return edge_descriptor{se.target, v, se.edge_index};
        });
}

template<typename D, typename VP, typename EP, typename GP>
    requires (adjacency_list<D, VP, EP, GP>::is_bidirectional || 
              !adjacency_list<D, VP, EP, GP>::is_directed)
auto in_degree(typename adjacency_list<D, VP, EP, GP>::vertex_descriptor v,
               const adjacency_list<D, VP, EP, GP>& g) {
    return g.in_degree(v);
}

// Edge source/target
template<typename D, typename VP, typename EP, typename GP>
auto source(typename adjacency_list<D, VP, EP, GP>::edge_descriptor e,
            const adjacency_list<D, VP, EP, GP>&) {
    return e.source;
}

template<typename D, typename VP, typename EP, typename GP>
auto target(typename adjacency_list<D, VP, EP, GP>::edge_descriptor e,
            const adjacency_list<D, VP, EP, GP>&) {
    return e.target;
}

// =============================================================================
// Graph Traits Specialization
// =============================================================================

template<typename D, typename VP, typename EP, typename GP>
struct graph_traits<adjacency_list<D, VP, EP, GP>> {
    using graph_type = adjacency_list<D, VP, EP, GP>;
    
    using vertex_descriptor = typename graph_type::vertex_descriptor;
    using edge_descriptor = typename graph_type::edge_descriptor;
    using directed_category = typename graph_type::directed_category;
    using edge_parallel_category = typename graph_type::edge_parallel_category;
    using traversal_category = typename graph_type::traversal_category;
    using vertices_size_type = typename graph_type::vertices_size_type;
    using edges_size_type = typename graph_type::edges_size_type;
    using degree_size_type = typename graph_type::degree_size_type;
};

// =============================================================================
// Hash Support for Edge Descriptors
// =============================================================================

} // namespace bgl

template<typename V>
struct std::hash<bgl::adjacency_list_edge<V>> {
    std::size_t operator()(const bgl::adjacency_list_edge<V>& e) const noexcept {
        // Combine source, target, and edge_index
        std::size_t h1 = std::hash<V>{}(e.source);
        std::size_t h2 = std::hash<V>{}(e.target);
        std::size_t h3 = std::hash<std::size_t>{}(e.edge_index);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

#endif // BGL_MODERN_ADJACENCY_LIST_HPP
