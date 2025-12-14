// BGL Modern - Strongly Connected Components Algorithm
// C++20 implementation using Tarjan's algorithm
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_STRONG_COMPONENTS_HPP
#define BGL_MODERN_STRONG_COMPONENTS_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>
#include <bgl/modern/connected_components.hpp>  // for component_result

#include <vector>
#include <stack>
#include <algorithm>
#include <ranges>
#include <concepts>

namespace bgl {

// =============================================================================
// strong_component_result - Result Type for SCCs
// =============================================================================

/// Result type for strongly connected components algorithms.
///
/// Extends component_result with SCC-specific information like
/// topological order of components.
///
template<typename G>
class strong_component_result : public component_result<G> {
public:
    using base_type = component_result<G>;
    using vertex_descriptor = typename base_type::vertex_descriptor;
    using component_type = typename base_type::component_type;
    using size_type = typename base_type::size_type;
    
    strong_component_result() = default;
    
    explicit strong_component_result(size_type num_vertices)
        : base_type(num_vertices)
    {}
    
    /// Get components in reverse topological order
    /// (SCCs are numbered in reverse topological order by Tarjan's algorithm)
    const std::vector<component_type>& component_order() const {
        return component_order_;
    }
    
    void add_component_to_order(component_type c) {
        component_order_.push_back(c);
    }
    
    /// Check if the graph is strongly connected (single SCC containing all vertices)
    bool is_strongly_connected() const {
        return this->num_components() == 1;
    }
    
private:
    std::vector<component_type> component_order_;
};

// =============================================================================
// Factory Functions
// =============================================================================

template<typename G>
auto make_strong_component_result(const G& g) {
    return strong_component_result<G>(num_vertices(g));
}

// =============================================================================
// strong_components - Core Implementation (Tarjan's Algorithm)
// =============================================================================

namespace detail {

/// Tarjan's SCC algorithm state
template<typename G>
struct tarjan_state {
    using vertex_descriptor = vertex_descriptor_t<G>;
    
    std::vector<std::size_t> index;
    std::vector<std::size_t> lowlink;
    std::vector<bool> on_stack;
    std::stack<vertex_descriptor> stack;
    std::size_t current_index = 0;
    std::size_t current_component = 0;
    
    static constexpr std::size_t undefined = static_cast<std::size_t>(-1);
    
    explicit tarjan_state(std::size_t n)
        : index(n, undefined)
        , lowlink(n, undefined)
        , on_stack(n, false)
    {}
};

/// Tarjan's SCC DFS visit
template<typename G>
void tarjan_visit(
    const G& g,
    vertex_descriptor_t<G> v,
    tarjan_state<G>& state,
    strong_component_result<G>& result
) {
    using vertex_descriptor = vertex_descriptor_t<G>;
    
    // Set the depth index for v
    state.index[v] = state.current_index;
    state.lowlink[v] = state.current_index;
    ++state.current_index;
    state.stack.push(v);
    state.on_stack[v] = true;
    
    // Consider successors of v
    for (auto e : out_edges(v, g)) {
        vertex_descriptor w = target(e, g);
        
        if (state.index[w] == tarjan_state<G>::undefined) {
            // Successor w has not yet been visited; recurse on it
            tarjan_visit(g, w, state, result);
            state.lowlink[v] = std::min(state.lowlink[v], state.lowlink[w]);
        } else if (state.on_stack[w]) {
            // Successor w is in stack and hence in the current SCC
            state.lowlink[v] = std::min(state.lowlink[v], state.index[w]);
        }
    }
    
    // If v is a root node, pop the stack and generate an SCC
    if (state.lowlink[v] == state.index[v]) {
        std::size_t component_id = state.current_component++;
        
        vertex_descriptor w;
        do {
            w = state.stack.top();
            state.stack.pop();
            state.on_stack[w] = false;
            result.set_component(w, component_id);
        } while (w != v);
        
        result.add_component_to_order(component_id);
    }
}

/// Core Tarjan's SCC implementation
template<typename G>
void strong_components_impl(const G& g, strong_component_result<G>& result) {
    tarjan_state<G> state(num_vertices(g));
    
    for (auto v : vertices(g)) {
        if (state.index[v] == tarjan_state<G>::undefined) {
            tarjan_visit(g, v, state, result);
        }
    }
    
    result.set_num_components(state.current_component);
}

} // namespace detail

// =============================================================================
// strong_components - Public Interface
// =============================================================================

/// Compute strongly connected components of a directed graph.
///
/// Uses Tarjan's algorithm to identify strongly connected components.
/// Each vertex is assigned a component number from 0 to num_components()-1.
/// Components are numbered in reverse topological order.
///
/// Requirements:
/// - G must satisfy VertexListGraph and IncidenceGraph concepts
/// - Graph should be directed
///
/// Complexity: O(V + E)
///
/// @param g The graph
/// @return strong_component_result containing SCC assignments
///
/// Example:
/// @code
///     auto result = strong_components(g);
///     
///     std::cout << "Number of SCCs: " << result.num_components() << "\n";
///     
///     for (auto v : vertices(g)) {
///         std::cout << "Vertex " << v << " is in SCC " 
///                   << result.component_of(v) << "\n";
///     }
///     
///     // Check if entire graph is strongly connected
///     if (result.num_components() == 1) {
///         std::cout << "Graph is strongly connected!\n";
///     }
/// @endcode
///
template<typename G>
    requires VertexListGraph<G> && IncidenceGraph<G>
auto strong_components(const G& g) {
    auto result = make_strong_component_result(g);
    detail::strong_components_impl(g, result);
    return result;
}

} // namespace bgl

#endif // BGL_MODERN_STRONG_COMPONENTS_HPP
