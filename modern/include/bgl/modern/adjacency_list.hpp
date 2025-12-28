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
#include <bgl/modern/container_selectors.hpp>

#include <vector>
#include <list>
#include <set>
#include <ranges>
#include <concepts>
#include <utility>
#include <cstddef>
#include <functional>
#include <optional>
#include <iterator>
#include <algorithm>

namespace bgl {

// =============================================================================
// Forward Declarations
// =============================================================================

template<
    typename OutEdgeListS,
    typename VertexListS,
    typename DirectedS,
    typename VertexProperty,
    typename EdgeProperty,
    typename GraphProperty
>
class adjacency_list;

// =============================================================================
// Edge Descriptor
// =============================================================================

/// Edge descriptor for adjacency_list (used with index-based vertex descriptors)
template<typename VertexDescriptor>
struct adjacency_list_edge {
    VertexDescriptor source;
    VertexDescriptor target;
    std::size_t edge_index;
    
    bool operator==(const adjacency_list_edge&) const = default;
    auto operator<=>(const adjacency_list_edge&) const = default;
};

// =============================================================================
// Internal Storage Types
// =============================================================================

namespace detail {

/// Stored edge in out-edge list (minimal footprint)
template<typename VertexDescriptor>
struct stored_out_edge {
    VertexDescriptor target;
    std::size_t edge_index;
    
    bool operator==(const stored_out_edge&) const = default;
    
    // Ordering for set-based containers - order by target
    auto operator<=>(const stored_out_edge& other) const {
        return target <=> other.target;
    }
};

/// Hash for stored_out_edge (for unordered containers)
template<typename VertexDescriptor>
struct stored_out_edge_hash {
    std::size_t operator()(const stored_out_edge<VertexDescriptor>& e) const noexcept {
        return std::hash<VertexDescriptor>{}(e.target) ^ 
               (std::hash<std::size_t>{}(e.edge_index) << 1);
    }
};

/// Stored edge data (in edge list)
template<typename VertexDescriptor, typename EdgeProperty>
struct stored_edge_data {
    VertexDescriptor source;
    VertexDescriptor target;
    [[no_unique_address]] EdgeProperty property;
};

/// Vertex data storage (contains out-edges and optionally in-edges)
template<typename OutEdgeListS, typename VertexDescriptor, typename VertexProperty, bool IsBidirectional>
struct vertex_data {
    using stored_edge_type = stored_out_edge<VertexDescriptor>;
    using out_edge_container = container_t<OutEdgeListS, stored_edge_type>;
    
    out_edge_container out_edges;
    [[no_unique_address]] VertexProperty property;
};

/// Vertex data storage with in-edges (for bidirectional graphs)
template<typename OutEdgeListS, typename VertexDescriptor, typename VertexProperty>
struct vertex_data<OutEdgeListS, VertexDescriptor, VertexProperty, true> {
    using stored_edge_type = stored_out_edge<VertexDescriptor>;
    using out_edge_container = container_t<OutEdgeListS, stored_edge_type>;
    using in_edge_container = container_t<OutEdgeListS, stored_edge_type>;
    
    out_edge_container out_edges;
    in_edge_container in_edges;
    [[no_unique_address]] VertexProperty property;
};

// =============================================================================
// Adjacency List Implementation - vecS Vertex List Specialization
// =============================================================================

/// Implementation details for vecS vertex list
template<typename OutEdgeListS, typename DirectedS, typename VertexProperty, typename EdgeProperty>
struct adjacency_list_impl_vecS {
    using vertex_descriptor = std::size_t;
    using edge_descriptor = adjacency_list_edge<vertex_descriptor>;
    
    static constexpr bool is_directed = 
        std::same_as<DirectedS, directed_tag> || 
        std::same_as<DirectedS, bidirectional_tag>;
    
    static constexpr bool is_bidirectional = 
        std::same_as<DirectedS, bidirectional_tag>;
    
    using vertex_data_type = vertex_data<OutEdgeListS, vertex_descriptor, VertexProperty, is_bidirectional>;
    using edge_data_type = stored_edge_data<vertex_descriptor, EdgeProperty>;
    
    std::vector<vertex_data_type> vertices_;
    std::vector<edge_data_type> edges_;
    
    // Vertex operations
    vertex_descriptor add_vertex_impl() {
        vertices_.emplace_back();
        return vertices_.size() - 1;
    }
    
    vertex_descriptor add_vertex_impl(const VertexProperty& prop) {
        vertices_.emplace_back();
        vertices_.back().property = prop;
        return vertices_.size() - 1;
    }
    
    vertex_descriptor add_vertex_impl(VertexProperty&& prop) {
        vertices_.emplace_back();
        vertices_.back().property = std::move(prop);
        return vertices_.size() - 1;
    }
    
    std::size_t num_vertices_impl() const {
        return vertices_.size();
    }
    
    // Edge operations
    template<typename EP>
    std::pair<edge_descriptor, bool> add_edge_impl(vertex_descriptor u, vertex_descriptor v, EP&& prop) {
        using stored_edge_type = typename vertex_data_type::stored_edge_type;
        
        std::size_t edge_idx = edges_.size();
        edges_.push_back({u, v, std::forward<EP>(prop)});
        
        // Add to out-edges
        add_to_edge_list(vertices_[u].out_edges, stored_edge_type{v, edge_idx});
        
        // For undirected graphs, add reverse edge to adjacency
        if constexpr (!is_directed) {
            add_to_edge_list(vertices_[v].out_edges, stored_edge_type{u, edge_idx});
        }
        
        // For bidirectional graphs, track in-edges
        if constexpr (is_bidirectional) {
            add_to_edge_list(vertices_[v].in_edges, stored_edge_type{u, edge_idx});
        }
        
        return {{u, v, edge_idx}, true};
    }
    
    std::size_t num_edges_impl() const {
        return edges_.size();
    }
    
    // Degree queries
    std::size_t out_degree_impl(vertex_descriptor v) const {
        return vertices_[v].out_edges.size();
    }
    
    std::size_t in_degree_impl(vertex_descriptor v) const requires is_bidirectional {
        return vertices_[v].in_edges.size();
    }
    
    std::size_t in_degree_impl(vertex_descriptor v) const requires (!is_directed && !is_bidirectional) {
        return vertices_[v].out_edges.size();
    }
    
private:
    template<typename Container, typename Edge>
    static void add_to_edge_list(Container& c, Edge&& e) {
        if constexpr (SequenceSelector<OutEdgeListS>) {
            c.push_back(std::forward<Edge>(e));
        } else {
            c.insert(std::forward<Edge>(e));
        }
    }
};

// =============================================================================
// Adjacency List Implementation - listS Vertex List Specialization
// =============================================================================

/// Implementation details for listS vertex list (stable vertex descriptors)
template<typename OutEdgeListS, typename DirectedS, typename VertexProperty, typename EdgeProperty>
struct adjacency_list_impl_listS {
    static constexpr bool is_directed = 
        std::same_as<DirectedS, directed_tag> || 
        std::same_as<DirectedS, bidirectional_tag>;
    
    static constexpr bool is_bidirectional = 
        std::same_as<DirectedS, bidirectional_tag>;
    
    // Forward declare vertex_data_type to get vertex_descriptor
    struct vertex_data_type;
    
    using vertex_container = std::list<vertex_data_type>;
    using vertex_descriptor = typename vertex_container::iterator;
    using edge_descriptor = adjacency_list_edge<vertex_descriptor>;
    
    using stored_edge_type = stored_out_edge<vertex_descriptor>;
    using out_edge_container = container_t<OutEdgeListS, stored_edge_type>;
    using edge_data_type = stored_edge_data<vertex_descriptor, EdgeProperty>;
    
    // Now define vertex_data_type
    struct vertex_data_type {
        out_edge_container out_edges;
        [[no_unique_address]] std::conditional_t<is_bidirectional, out_edge_container, no_property> in_edges;
        [[no_unique_address]] VertexProperty property;
    };
    
    vertex_container vertices_;
    std::vector<edge_data_type> edges_;
    
    // Vertex operations
    vertex_descriptor add_vertex_impl() {
        vertices_.emplace_back();
        return std::prev(vertices_.end());
    }
    
    vertex_descriptor add_vertex_impl(const VertexProperty& prop) {
        vertices_.emplace_back();
        vertices_.back().property = prop;
        return std::prev(vertices_.end());
    }
    
    vertex_descriptor add_vertex_impl(VertexProperty&& prop) {
        vertices_.emplace_back();
        vertices_.back().property = std::move(prop);
        return std::prev(vertices_.end());
    }
    
    void remove_vertex_impl(vertex_descriptor v) {
        // Note: Caller must ensure no edges reference this vertex
        vertices_.erase(v);
    }
    
    std::size_t num_vertices_impl() const {
        return vertices_.size();
    }
    
    // Edge operations
    template<typename EP>
    std::pair<edge_descriptor, bool> add_edge_impl(vertex_descriptor u, vertex_descriptor v, EP&& prop) {
        std::size_t edge_idx = edges_.size();
        edges_.push_back({u, v, std::forward<EP>(prop)});
        
        // Add to out-edges
        add_to_edge_list(u->out_edges, stored_edge_type{v, edge_idx});
        
        // For undirected graphs, add reverse edge to adjacency
        if constexpr (!is_directed) {
            add_to_edge_list(v->out_edges, stored_edge_type{u, edge_idx});
        }
        
        // For bidirectional graphs, track in-edges
        if constexpr (is_bidirectional) {
            add_to_edge_list(v->in_edges, stored_edge_type{u, edge_idx});
        }
        
        return {{u, v, edge_idx}, true};
    }
    
    std::size_t num_edges_impl() const {
        return edges_.size();
    }
    
    // Degree queries
    std::size_t out_degree_impl(vertex_descriptor v) const {
        return v->out_edges.size();
    }
    
    std::size_t in_degree_impl(vertex_descriptor v) const requires is_bidirectional {
        return v->in_edges.size();
    }
    
    std::size_t in_degree_impl(vertex_descriptor v) const requires (!is_directed && !is_bidirectional) {
        return v->out_edges.size();
    }
    
private:
    template<typename Container, typename Edge>
    static void add_to_edge_list(Container& c, Edge&& e) {
        if constexpr (SequenceSelector<OutEdgeListS>) {
            c.push_back(std::forward<Edge>(e));
        } else {
            c.insert(std::forward<Edge>(e));
        }
    }
};

// =============================================================================
// Adjacency List Implementation - setS Vertex List Specialization
// =============================================================================

/// Implementation details for setS vertex list (ordered, stable vertex descriptors)
template<typename OutEdgeListS, typename DirectedS, typename VertexProperty, typename EdgeProperty>
struct adjacency_list_impl_setS {
    static constexpr bool is_directed = 
        std::same_as<DirectedS, directed_tag> || 
        std::same_as<DirectedS, bidirectional_tag>;
    
    static constexpr bool is_bidirectional = 
        std::same_as<DirectedS, bidirectional_tag>;
    
    // Use a wrapper that includes vertex ID for ordering
    struct vertex_wrapper;
    
    using vertex_container = std::set<vertex_wrapper>;
    using vertex_descriptor = typename vertex_container::iterator;
    using edge_descriptor = adjacency_list_edge<vertex_descriptor>;
    
    using stored_edge_type = stored_out_edge<vertex_descriptor>;
    using out_edge_container = container_t<OutEdgeListS, stored_edge_type>;
    using edge_data_type = stored_edge_data<vertex_descriptor, EdgeProperty>;
    
    struct vertex_wrapper {
        std::size_t id;  // Unique ID for ordering
        mutable out_edge_container out_edges;
        [[no_unique_address]] mutable std::conditional_t<is_bidirectional, out_edge_container, no_property> in_edges;
        [[no_unique_address]] mutable VertexProperty property;
        
        bool operator<(const vertex_wrapper& other) const {
            return id < other.id;
        }
        bool operator==(const vertex_wrapper& other) const {
            return id == other.id;
        }
    };
    
    vertex_container vertices_;
    std::vector<edge_data_type> edges_;
    std::size_t next_vertex_id_ = 0;
    
    // Vertex operations
    vertex_descriptor add_vertex_impl() {
        auto [it, inserted] = vertices_.insert(vertex_wrapper{next_vertex_id_++, {}, {}, {}});
        return it;
    }
    
    vertex_descriptor add_vertex_impl(const VertexProperty& prop) {
        auto [it, inserted] = vertices_.insert(vertex_wrapper{next_vertex_id_++, {}, {}, prop});
        return it;
    }
    
    vertex_descriptor add_vertex_impl(VertexProperty&& prop) {
        auto [it, inserted] = vertices_.insert(vertex_wrapper{next_vertex_id_++, {}, {}, std::move(prop)});
        return it;
    }
    
    void remove_vertex_impl(vertex_descriptor v) {
        vertices_.erase(v);
    }
    
    std::size_t num_vertices_impl() const {
        return vertices_.size();
    }
    
    // Edge operations
    template<typename EP>
    std::pair<edge_descriptor, bool> add_edge_impl(vertex_descriptor u, vertex_descriptor v, EP&& prop) {
        std::size_t edge_idx = edges_.size();
        edges_.push_back({u, v, std::forward<EP>(prop)});
        
        // Add to out-edges (mutable access via const_cast or mutable members)
        add_to_edge_list(u->out_edges, stored_edge_type{v, edge_idx});
        
        if constexpr (!is_directed) {
            add_to_edge_list(v->out_edges, stored_edge_type{u, edge_idx});
        }
        
        if constexpr (is_bidirectional) {
            add_to_edge_list(v->in_edges, stored_edge_type{u, edge_idx});
        }
        
        return {{u, v, edge_idx}, true};
    }
    
    std::size_t num_edges_impl() const {
        return edges_.size();
    }
    
    std::size_t out_degree_impl(vertex_descriptor v) const {
        return v->out_edges.size();
    }
    
    std::size_t in_degree_impl(vertex_descriptor v) const requires is_bidirectional {
        return v->in_edges.size();
    }
    
    std::size_t in_degree_impl(vertex_descriptor v) const requires (!is_directed && !is_bidirectional) {
        return v->out_edges.size();
    }
    
private:
    template<typename Container, typename Edge>
    static void add_to_edge_list(Container& c, Edge&& e) {
        if constexpr (SequenceSelector<OutEdgeListS>) {
            c.push_back(std::forward<Edge>(e));
        } else {
            c.insert(std::forward<Edge>(e));
        }
    }
};

// =============================================================================
// Implementation Selector
// =============================================================================

template<typename OutEdgeListS, typename VertexListS, typename DirectedS, 
         typename VertexProperty, typename EdgeProperty>
struct adjacency_list_impl_selector {
    // Default to vecS implementation
    using type = adjacency_list_impl_vecS<OutEdgeListS, DirectedS, VertexProperty, EdgeProperty>;
};

template<typename OutEdgeListS, typename DirectedS, typename VertexProperty, typename EdgeProperty>
struct adjacency_list_impl_selector<OutEdgeListS, listS, DirectedS, VertexProperty, EdgeProperty> {
    using type = adjacency_list_impl_listS<OutEdgeListS, DirectedS, VertexProperty, EdgeProperty>;
};

template<typename OutEdgeListS, typename DirectedS, typename VertexProperty, typename EdgeProperty>
struct adjacency_list_impl_selector<OutEdgeListS, setS, DirectedS, VertexProperty, EdgeProperty> {
    using type = adjacency_list_impl_setS<OutEdgeListS, DirectedS, VertexProperty, EdgeProperty>;
};

template<typename OutEdgeListS, typename VertexListS, typename DirectedS,
         typename VertexProperty, typename EdgeProperty>
using adjacency_list_impl = typename adjacency_list_impl_selector<
    OutEdgeListS, VertexListS, DirectedS, VertexProperty, EdgeProperty>::type;

} // namespace detail

// =============================================================================
// adjacency_list - Main Class Template
// =============================================================================

/// Modern C++20 adjacency list graph container.
///
/// Template Parameters:
/// - OutEdgeListS: Selector for out-edge container (vecS, listS, setS, etc.)
/// - VertexListS: Selector for vertex container (vecS, listS, setS)
/// - DirectedS: directed_tag, undirected_tag, or bidirectional_tag
/// - VertexProperty: Property type stored per vertex (default: no_property)
/// - EdgeProperty: Property type stored per edge (default: no_property)
/// - GraphProperty: Property type for the whole graph (default: no_property)
///
/// Example usage:
/// @code
///     // Simple graph with no properties (vecS default)
///     adjacency_list<> g;
///     
///     // Graph with listS for vertices (stable descriptors)
///     adjacency_list<vecS, listS, directed_tag> g2;
///     
///     // Graph with setS for edges (no parallel edges)
///     adjacency_list<setS, vecS, directed_tag, VertexData, EdgeData> g3;
///     
///     auto v = g3.add_vertex({.name = "A"});
///     auto u = g3.add_vertex({.name = "B"});
///     auto e = g3.add_edge(v, u, {.weight = 1.5});
/// @endcode
///
template<
    typename OutEdgeListS = vecS,
    typename VertexListS = vecS,
    typename DirectedS = directed_tag,
    typename VertexProperty = no_property,
    typename EdgeProperty = no_property,
    typename GraphProperty = no_property
>
class adjacency_list : private detail::adjacency_list_impl<OutEdgeListS, VertexListS, DirectedS, 
                                                            VertexProperty, EdgeProperty> {
    using impl_type = detail::adjacency_list_impl<OutEdgeListS, VertexListS, DirectedS,
                                                   VertexProperty, EdgeProperty>;
    
public:
    // -------------------------------------------------------------------------
    // Type Definitions
    // -------------------------------------------------------------------------
    
    using out_edge_list_selector = OutEdgeListS;
    using vertex_list_selector = VertexListS;
    using directed_category = DirectedS;
    using edge_parallel_category = detail::parallel_edge_category_for<OutEdgeListS>;
    using traversal_category = void;  // Supports multiple traversal patterns
    
    using vertex_descriptor = typename impl_type::vertex_descriptor;
    using edge_descriptor = typename impl_type::edge_descriptor;
    
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
    
    using vertex_property_type = VertexProperty;
    using edge_property_type = EdgeProperty;
    using graph_property_type = GraphProperty;
    
    static constexpr bool is_directed = impl_type::is_directed;
    static constexpr bool is_bidirectional = impl_type::is_bidirectional;
    
    /// Whether vertex descriptors are stable across vertex addition/removal
    static constexpr bool has_stable_vertex_descriptors = 
        std::same_as<VertexListS, listS> || std::same_as<VertexListS, setS>;
    
    /// Whether edge descriptors are stable across edge addition/removal
    static constexpr bool has_stable_edge_descriptors = 
        std::same_as<OutEdgeListS, listS> || std::same_as<OutEdgeListS, setS>;
    
private:
    [[no_unique_address]] GraphProperty graph_property_;
    
public:
    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------
    
    adjacency_list() = default;
    
    /// Construct with n vertices (each with default-constructed properties)
    /// Only available for vecS vertex list
    explicit adjacency_list(vertices_size_type n) 
        requires std::same_as<VertexListS, vecS>
    {
        this->vertices_.resize(n);
    }
    
    /// Construct with n vertices and initial vertex property
    adjacency_list(vertices_size_type n, const VertexProperty& vp)
        requires std::same_as<VertexListS, vecS>
    {
        this->vertices_.resize(n);
        for (auto& v : this->vertices_) {
            v.property = vp;
        }
    }
    
    // -------------------------------------------------------------------------
    // Vertex Operations
    // -------------------------------------------------------------------------
    
    /// Add a vertex with default property
    vertex_descriptor add_vertex() {
        return this->add_vertex_impl();
    }
    
    /// Add a vertex with the given property
    vertex_descriptor add_vertex(const VertexProperty& prop) {
        return this->add_vertex_impl(prop);
    }
    
    /// Add a vertex with moved property
    vertex_descriptor add_vertex(VertexProperty&& prop) {
        return this->add_vertex_impl(std::move(prop));
    }
    
    /// Remove a vertex (only for stable descriptor containers)
    void remove_vertex(vertex_descriptor v)
        requires has_stable_vertex_descriptors
    {
        this->remove_vertex_impl(v);
    }
    
    /// Clear a vertex (remove all edges, keep vertex)
    void clear_vertex(vertex_descriptor v) {
        // Implementation depends on container type
        if constexpr (std::same_as<VertexListS, vecS>) {
            this->vertices_[v].out_edges.clear();
            if constexpr (is_bidirectional) {
                this->vertices_[v].in_edges.clear();
            }
        } else {
            v->out_edges.clear();
            if constexpr (is_bidirectional) {
                v->in_edges.clear();
            }
        }
    }
    
    /// Get vertex property (const) - vecS version
    const VertexProperty& operator[](vertex_descriptor v) const 
        requires (!std::same_as<VertexProperty, no_property> && std::same_as<VertexListS, vecS>)
    {
        return this->vertices_[v].property;
    }
    
    /// Get vertex property (mutable) - vecS version
    VertexProperty& operator[](vertex_descriptor v)
        requires (!std::same_as<VertexProperty, no_property> && std::same_as<VertexListS, vecS>)
    {
        return this->vertices_[v].property;
    }
    
    /// Get vertex property (const) - iterator-based version
    const VertexProperty& operator[](vertex_descriptor v) const 
        requires (!std::same_as<VertexProperty, no_property> && !std::same_as<VertexListS, vecS>)
    {
        return v->property;
    }
    
    /// Get vertex property (mutable) - iterator-based version
    VertexProperty& operator[](vertex_descriptor v)
        requires (!std::same_as<VertexProperty, no_property> && !std::same_as<VertexListS, vecS>)
    {
        // For set-based containers, property is mutable
        return const_cast<VertexProperty&>(v->property);
    }
    
    // -------------------------------------------------------------------------
    // Edge Operations
    // -------------------------------------------------------------------------
    
    /// Add an edge with default property
    std::pair<edge_descriptor, bool> add_edge(vertex_descriptor u, vertex_descriptor v) {
        return this->add_edge_impl(u, v, EdgeProperty{});
    }
    
    /// Add an edge with the given property
    std::pair<edge_descriptor, bool> add_edge(vertex_descriptor u, vertex_descriptor v, 
                                               const EdgeProperty& prop) {
        return this->add_edge_impl(u, v, prop);
    }
    
    /// Add an edge with moved property
    std::pair<edge_descriptor, bool> add_edge(vertex_descriptor u, vertex_descriptor v,
                                               EdgeProperty&& prop) {
        return this->add_edge_impl(u, v, std::move(prop));
    }
    
    /// Get edge property (const)
    const EdgeProperty& operator[](edge_descriptor e) const
        requires (!std::same_as<EdgeProperty, no_property>)
    {
        return this->edges_[e.edge_index].property;
    }
    
    /// Get edge property (mutable)
    EdgeProperty& operator[](edge_descriptor e)
        requires (!std::same_as<EdgeProperty, no_property>)
    {
        return this->edges_[e.edge_index].property;
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
        return this->num_vertices_impl();
    }
    
    edges_size_type num_edges() const {
        return this->num_edges_impl();
    }
    
    degree_size_type out_degree(vertex_descriptor v) const {
        return this->out_degree_impl(v);
    }
    
    degree_size_type in_degree(vertex_descriptor v) const
        requires (is_bidirectional || !is_directed)
    {
        return this->in_degree_impl(v);
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
    // Internal Access (for friend functions)
    // -------------------------------------------------------------------------
    
    const auto& vertices_storage() const { return this->vertices_; }
    const auto& edges_storage() const { return this->edges_; }
    auto& vertices_storage() { return this->vertices_; }
    auto& edges_storage() { return this->edges_; }
};

// =============================================================================
// Free Functions for vecS vertex lists (ADL-findable)
// =============================================================================

// Vertex iteration - vecS
template<typename OE, typename D, typename VP, typename EP, typename GP>
auto vertices(const adjacency_list<OE, vecS, D, VP, EP, GP>& g) {
    return std::views::iota(std::size_t{0}, g.num_vertices());
}

// Vertex iteration - listS/setS (returns iterator range)
template<typename OE, typename VL, typename D, typename VP, typename EP, typename GP>
    requires (!std::same_as<VL, vecS>)
auto vertices(const adjacency_list<OE, VL, D, VP, EP, GP>& g) {
    // Return a view that yields vertex descriptors (iterators into the container)
    using graph_type = adjacency_list<OE, VL, D, VP, EP, GP>;
    using vertex_descriptor = typename graph_type::vertex_descriptor;
    
    // Create a range of vertex descriptors from the container
    auto& storage = g.vertices_storage();
    std::vector<vertex_descriptor> result;
    result.reserve(storage.size());
    for (auto it = storage.begin(); it != storage.end(); ++it) {
        result.push_back(it);
    }
    return result;
}

template<typename OE, typename VL, typename D, typename VP, typename EP, typename GP>
auto num_vertices(const adjacency_list<OE, VL, D, VP, EP, GP>& g) {
    return g.num_vertices();
}

// Edge iteration
template<typename OE, typename VL, typename D, typename VP, typename EP, typename GP>
auto num_edges(const adjacency_list<OE, VL, D, VP, EP, GP>& g) {
    return g.num_edges();
}

// Out-edge iteration - vecS
template<typename OE, typename D, typename VP, typename EP, typename GP>
auto out_edges(typename adjacency_list<OE, vecS, D, VP, EP, GP>::vertex_descriptor v,
               const adjacency_list<OE, vecS, D, VP, EP, GP>& g) {
    using graph_type = adjacency_list<OE, vecS, D, VP, EP, GP>;
    using edge_descriptor = typename graph_type::edge_descriptor;
    
    return g.vertices_storage()[v].out_edges 
        | std::views::transform([v, &g](const auto& se) {
            auto src = g.edges_storage()[se.edge_index].source;
            auto tgt = g.edges_storage()[se.edge_index].target;
            if constexpr (!graph_type::is_directed) {
                if (src != v) std::swap(src, tgt);
            }
            return edge_descriptor{src, tgt, se.edge_index};
        });
}

// Out-edge iteration - listS/setS
template<typename OE, typename VL, typename D, typename VP, typename EP, typename GP>
    requires (!std::same_as<VL, vecS>)
auto out_edges(typename adjacency_list<OE, VL, D, VP, EP, GP>::vertex_descriptor v,
               const adjacency_list<OE, VL, D, VP, EP, GP>& g) {
    using graph_type = adjacency_list<OE, VL, D, VP, EP, GP>;
    using edge_descriptor = typename graph_type::edge_descriptor;
    
    return v->out_edges 
        | std::views::transform([v, &g](const auto& se) {
            auto src = g.edges_storage()[se.edge_index].source;
            auto tgt = g.edges_storage()[se.edge_index].target;
            if constexpr (!graph_type::is_directed) {
                if (src != v) std::swap(src, tgt);
            }
            return edge_descriptor{src, tgt, se.edge_index};
        });
}

template<typename OE, typename VL, typename D, typename VP, typename EP, typename GP>
auto out_degree(typename adjacency_list<OE, VL, D, VP, EP, GP>::vertex_descriptor v,
                const adjacency_list<OE, VL, D, VP, EP, GP>& g) {
    return g.out_degree(v);
}

// In-edge iteration - vecS (bidirectional only)
template<typename OE, typename D, typename VP, typename EP, typename GP>
    requires (adjacency_list<OE, vecS, D, VP, EP, GP>::is_bidirectional)
auto in_edges(typename adjacency_list<OE, vecS, D, VP, EP, GP>::vertex_descriptor v,
              const adjacency_list<OE, vecS, D, VP, EP, GP>& g) {
    using graph_type = adjacency_list<OE, vecS, D, VP, EP, GP>;
    using edge_descriptor = typename graph_type::edge_descriptor;
    
    return g.vertices_storage()[v].in_edges
        | std::views::transform([v, &g](const auto& se) {
            return edge_descriptor{se.target, v, se.edge_index};
        });
}

// In-edge iteration - listS/setS (bidirectional only)
template<typename OE, typename VL, typename D, typename VP, typename EP, typename GP>
    requires (adjacency_list<OE, VL, D, VP, EP, GP>::is_bidirectional && !std::same_as<VL, vecS>)
auto in_edges(typename adjacency_list<OE, VL, D, VP, EP, GP>::vertex_descriptor v,
              const adjacency_list<OE, VL, D, VP, EP, GP>& g) {
    using graph_type = adjacency_list<OE, VL, D, VP, EP, GP>;
    using edge_descriptor = typename graph_type::edge_descriptor;
    
    return v->in_edges
        | std::views::transform([v](const auto& se) {
            return edge_descriptor{se.target, v, se.edge_index};
        });
}

template<typename OE, typename VL, typename D, typename VP, typename EP, typename GP>
    requires (adjacency_list<OE, VL, D, VP, EP, GP>::is_bidirectional || 
              !adjacency_list<OE, VL, D, VP, EP, GP>::is_directed)
auto in_degree(typename adjacency_list<OE, VL, D, VP, EP, GP>::vertex_descriptor v,
               const adjacency_list<OE, VL, D, VP, EP, GP>& g) {
    return g.in_degree(v);
}

// Edge source/target
template<typename OE, typename VL, typename D, typename VP, typename EP, typename GP>
auto source(typename adjacency_list<OE, VL, D, VP, EP, GP>::edge_descriptor e,
            const adjacency_list<OE, VL, D, VP, EP, GP>&) {
    return e.source;
}

template<typename OE, typename VL, typename D, typename VP, typename EP, typename GP>
auto target(typename adjacency_list<OE, VL, D, VP, EP, GP>::edge_descriptor e,
            const adjacency_list<OE, VL, D, VP, EP, GP>&) {
    return e.target;
}

// =============================================================================
// Graph Traits Specialization
// =============================================================================

template<typename OE, typename VL, typename D, typename VP, typename EP, typename GP>
struct graph_traits<adjacency_list<OE, VL, D, VP, EP, GP>> {
    using graph_type = adjacency_list<OE, VL, D, VP, EP, GP>;
    
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
// Backward Compatibility Aliases
// =============================================================================

namespace detail {

/// Helper to detect if a type is a direction tag
template<typename T>
concept DirectionTag = 
    std::same_as<T, directed_tag> ||
    std::same_as<T, undirected_tag> ||
    std::same_as<T, bidirectional_tag>;

/// Helper to detect if a type is a container selector
template<typename T>
concept SelectorTag = ContainerSelector<T>;

} // namespace detail

/// Backward compatibility alias for the old API where DirectedS was the first parameter.
/// 
/// Old API: adjacency_list<DirectedS, VertexProperty, EdgeProperty, GraphProperty>
/// New API: adjacency_list<OutEdgeListS, VertexListS, DirectedS, VertexProperty, EdgeProperty, GraphProperty>
///
/// This alias allows the old-style usage:
/// @code
///     adjacency_list<directed_tag> g;  // Works via this alias
///     adjacency_list<directed_tag, MyVertexProp, MyEdgeProp> g2;  // Also works
/// @endcode
///
template<
    typename DirectedS,
    typename VertexProperty = no_property,
    typename EdgeProperty = no_property,
    typename GraphProperty = no_property
>
    requires detail::DirectionTag<DirectedS>
using simple_adjacency_list = adjacency_list<vecS, vecS, DirectedS, VertexProperty, EdgeProperty, GraphProperty>;

// =============================================================================
// Hash Support for Edge Descriptors
// =============================================================================

} // namespace bgl

/// Hash support for stored_out_edge (required for hash_setS, hash_mapS, etc.)
template<typename V>
struct std::hash<bgl::detail::stored_out_edge<V>> {
    std::size_t operator()(const bgl::detail::stored_out_edge<V>& e) const noexcept {
        return std::hash<V>{}(e.target) ^ 
               (std::hash<std::size_t>{}(e.edge_index) << 1);
    }
};

template<typename V>
struct std::hash<bgl::adjacency_list_edge<V>> {
    std::size_t operator()(const bgl::adjacency_list_edge<V>& e) const noexcept {
        std::size_t h1 = std::hash<V>{}(e.source);
        std::size_t h2 = std::hash<V>{}(e.target);
        std::size_t h3 = std::hash<std::size_t>{}(e.edge_index);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

#endif // BGL_MODERN_ADJACENCY_LIST_HPP
