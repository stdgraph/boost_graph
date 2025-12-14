// BGL Modern - Connected Components Algorithm
// C++20 implementation of connected components
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_CONNECTED_COMPONENTS_HPP
#define BGL_MODERN_CONNECTED_COMPONENTS_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>

#include <vector>
#include <ranges>
#include <concepts>
#include <algorithm>

namespace bgl {

// =============================================================================
// component_result - Result Type for Connected Components
// =============================================================================

/// Result type for connected components algorithms.
///
/// This struct holds the computed component assignments for all vertices.
/// It provides convenient accessors for querying component information.
///
/// Usage:
/// @code
///     auto result = connected_components(g);
///     
///     // Get number of components
///     std::size_t num = result.num_components();
///     
///     // Get component of a specific vertex
///     std::size_t comp = result.component_of(v);
///     
///     // Get all vertices in a component
///     for (auto v : result.vertices_in_component(0)) {
///         std::cout << v << " ";
///     }
/// @endcode
///
/// @tparam G The graph type
///
template<typename G>
class component_result {
public:
    using graph_type = G;
    using vertex_descriptor = vertex_descriptor_t<G>;
    using component_type = std::size_t;
    using size_type = std::size_t;
    
    static constexpr component_type no_component = static_cast<component_type>(-1);
    
    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------
    
    component_result() = default;
    
    explicit component_result(size_type num_vertices)
        : components_(num_vertices, no_component)
    {}
    
    // -------------------------------------------------------------------------
    // Property Map Access (for algorithm use)
    // -------------------------------------------------------------------------
    
    auto component_map() {
        return [this](vertex_descriptor v) -> component_type& {
            return components_[v];
        };
    }
    
    // -------------------------------------------------------------------------
    // Building the Result (for algorithm use)
    // -------------------------------------------------------------------------
    
    void set_component(vertex_descriptor v, component_type c) {
        components_[v] = c;
        if (c >= num_components_) {
            num_components_ = c + 1;
        }
    }
    
    void set_num_components(size_type n) {
        num_components_ = n;
    }
    
    // -------------------------------------------------------------------------
    // Query Interface
    // -------------------------------------------------------------------------
    
    /// Get the component number of a vertex
    component_type component_of(vertex_descriptor v) const {
        return components_[v];
    }
    
    /// Get the number of connected components
    size_type num_components() const {
        return num_components_;
    }
    
    /// Check if graph is connected (single component)
    bool is_connected() const {
        return num_components_ == 1;
    }
    
    /// Check if two vertices are in the same component
    bool same_component(vertex_descriptor u, vertex_descriptor v) const {
        return components_[u] == components_[v];
    }
    
    /// Get size of a specific component
    size_type component_size(component_type c) const {
        return std::ranges::count(components_, c);
    }
    
    /// Get all vertices in a specific component
    auto vertices_in_component(component_type c) const {
        std::vector<vertex_descriptor> verts;
        for (size_type i = 0; i < components_.size(); ++i) {
            if (components_[i] == c) {
                verts.push_back(static_cast<vertex_descriptor>(i));
            }
        }
        return verts;
    }
    
    /// Get the raw component map
    const std::vector<component_type>& components() const {
        return components_;
    }
    
private:
    std::vector<component_type> components_;
    size_type num_components_ = 0;
};

// =============================================================================
// Factory Functions
// =============================================================================

template<typename G>
auto make_component_result(const G& g) {
    return component_result<G>(num_vertices(g));
}

// =============================================================================
// connected_components - Core Implementation
// =============================================================================

namespace detail {

/// DFS visit for connected components
template<typename G>
void cc_dfs_visit(
    const G& g,
    vertex_descriptor_t<G> u,
    component_result<G>& result,
    std::size_t component_id
) {
    using vertex_descriptor = vertex_descriptor_t<G>;
    
    result.set_component(u, component_id);
    
    for (auto e : out_edges(u, g)) {
        vertex_descriptor v = target(e, g);
        if (result.component_of(v) == component_result<G>::no_component) {
            cc_dfs_visit(g, v, result, component_id);
        }
    }
}

/// Core connected components implementation using DFS
template<typename G>
void connected_components_impl(const G& g, component_result<G>& result) {
    std::size_t component_id = 0;
    
    for (auto v : vertices(g)) {
        if (result.component_of(v) == component_result<G>::no_component) {
            cc_dfs_visit(g, v, result, component_id);
            ++component_id;
        }
    }
    
    result.set_num_components(component_id);
}

} // namespace detail

// =============================================================================
// connected_components - Public Interface
// =============================================================================

/// Compute connected components of an undirected graph.
///
/// Uses depth-first search to identify connected components. Each vertex
/// is assigned a component number from 0 to num_components()-1.
///
/// Requirements:
/// - G must satisfy VertexListGraph and IncidenceGraph concepts
/// - Graph should be undirected (or treated as undirected)
///
/// Complexity: O(V + E)
///
/// @param g The graph
/// @return component_result containing component assignments
///
/// Example:
/// @code
///     auto result = connected_components(g);
///     
///     std::cout << "Number of components: " << result.num_components() << "\n";
///     
///     for (auto v : vertices(g)) {
///         std::cout << "Vertex " << v << " is in component " 
///                   << result.component_of(v) << "\n";
///     }
///     
///     if (result.is_connected()) {
///         std::cout << "Graph is connected!\n";
///     }
/// @endcode
///
template<typename G>
    requires VertexListGraph<G> && IncidenceGraph<G>
auto connected_components(const G& g) {
    auto result = make_component_result(g);
    detail::connected_components_impl(g, result);
    return result;
}

} // namespace bgl

#endif // BGL_MODERN_CONNECTED_COMPONENTS_HPP
