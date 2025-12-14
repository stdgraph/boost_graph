// BGL Modern - Grid Graph Container
// C++20 implementation with concepts and ranges
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_GRID_GRAPH_HPP
#define BGL_MODERN_GRID_GRAPH_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>

#include <array>
#include <ranges>
#include <concepts>
#include <cstddef>
#include <utility>
#include <functional>
#include <optional>
#include <numeric>

namespace bgl {

// =============================================================================
// Grid Edge Descriptor
// =============================================================================

/// Edge descriptor for grid graph (source coords, dimension, direction)
template<std::size_t Dimensions>
struct grid_edge {
    std::array<std::size_t, Dimensions> source_coords;
    std::array<std::size_t, Dimensions> target_coords;
    std::size_t dimension;  // Which dimension the edge traverses
    bool positive;          // Direction in that dimension
    
    bool operator==(const grid_edge&) const = default;
    auto operator<=>(const grid_edge&) const = default;
};

// =============================================================================
// grid_graph - Main Class Template
// =============================================================================

/// Modern C++20 N-dimensional grid graph.
///
/// Represents an implicit graph where vertices are points on an N-dimensional
/// grid and edges connect adjacent points along each dimension. No edge or
/// adjacency data is stored - everything is computed from coordinates.
///
/// Space complexity: O(1) for the graph structure (plus O(N) for dimensions)
/// Edges are implicit based on grid coordinates.
///
/// Best for:
/// - Image processing (2D grids)
/// - Scientific computing (3D/4D grids)
/// - Game pathfinding on rectangular maps
/// - Lattice computations
///
/// Template Parameters:
/// - Dimensions: Number of dimensions (1, 2, 3, ...)
/// - Wrapped: If true, grid wraps around (torus topology)
///
/// Example:
/// @code
///     // Create a 100x100 2D grid
///     grid_graph<2> g({100, 100});
///     
///     // Vertex at (5, 10)
///     auto v = g.vertex_at({5, 10});
///     
///     // Iterate neighbors
///     for (auto e : out_edges(v, g)) {
///         auto [x, y] = g.coordinates(target(e, g));
///         std::cout << "Neighbor at (" << x << ", " << y << ")\n";
///     }
/// @endcode
///
template<std::size_t Dimensions, bool Wrapped = false>
class grid_graph {
    static_assert(Dimensions > 0, "Grid must have at least 1 dimension");
    
public:
    // -------------------------------------------------------------------------
    // Type Definitions
    // -------------------------------------------------------------------------
    
    using directed_category = undirected_tag;
    using edge_parallel_category = disallow_parallel_edge_tag;
    using traversal_category = void;
    
    using vertex_descriptor = std::size_t;  // Linear index into grid
    using edge_descriptor = grid_edge<Dimensions>;
    using coordinates_type = std::array<std::size_t, Dimensions>;
    
    using vertices_size_type = std::size_t;
    using edges_size_type = std::size_t;
    using degree_size_type = std::size_t;
    
    static constexpr std::size_t dimensions = Dimensions;
    static constexpr bool is_wrapped = Wrapped;
    
private:
    coordinates_type dimensions_;
    vertices_size_type num_vertices_ = 0;
    
    // Strides for converting between linear index and coordinates
    std::array<std::size_t, Dimensions> strides_;
    
    void compute_strides() {
        strides_[0] = 1;
        for (std::size_t d = 1; d < Dimensions; ++d) {
            strides_[d] = strides_[d - 1] * dimensions_[d - 1];
        }
    }
    
public:
    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------
    
    grid_graph() : dimensions_{}, strides_{} {}
    
    /// Construct from dimension sizes
    explicit grid_graph(coordinates_type dims)
        : dimensions_(dims)
    {
        num_vertices_ = std::accumulate(
            dims.begin(), dims.end(), std::size_t{1}, std::multiplies<>{});
        compute_strides();
    }
    
    /// Construct from dimension sizes (initializer list for convenience)
    template<typename... Sizes>
        requires (sizeof...(Sizes) == Dimensions && (std::convertible_to<Sizes, std::size_t> && ...))
    grid_graph(Sizes... sizes)
        : grid_graph(coordinates_type{static_cast<std::size_t>(sizes)...})
    {}
    
    // -------------------------------------------------------------------------
    // Coordinate Conversion
    // -------------------------------------------------------------------------
    
    /// Convert linear vertex index to coordinates
    [[nodiscard]] coordinates_type coordinates(vertex_descriptor v) const {
        coordinates_type result;
        for (std::size_t d = Dimensions; d > 0; --d) {
            result[d - 1] = v / strides_[d - 1];
            v %= strides_[d - 1];
        }
        return result;
    }
    
    /// Convert coordinates to linear vertex index
    [[nodiscard]] vertex_descriptor vertex_at(const coordinates_type& coords) const {
        vertex_descriptor v = 0;
        for (std::size_t d = 0; d < Dimensions; ++d) {
            v += coords[d] * strides_[d];
        }
        return v;
    }
    
    // -------------------------------------------------------------------------
    // Dimension Queries
    // -------------------------------------------------------------------------
    
    /// Get size in a particular dimension
    [[nodiscard]] std::size_t length(std::size_t dim) const {
        return dimensions_[dim];
    }
    
    /// Get all dimension sizes
    [[nodiscard]] const coordinates_type& lengths() const {
        return dimensions_;
    }
    
    // -------------------------------------------------------------------------
    // Size Queries
    // -------------------------------------------------------------------------
    
    [[nodiscard]] vertices_size_type num_vertices() const { return num_vertices_; }
    
    [[nodiscard]] edges_size_type num_edges() const {
        if constexpr (Wrapped) {
            // Each dimension contributes dim[d] * (product of other dims) edges
            edges_size_type total = 0;
            for (std::size_t d = 0; d < Dimensions; ++d) {
                total += num_vertices_;  // Each vertex has one edge in each dimension
            }
            return total / 2;  // Undirected, so divide by 2
        } else {
            // Count edges: for each dimension d, there are (dim[d]-1) * (product of other dims) edges
            edges_size_type total = 0;
            for (std::size_t d = 0; d < Dimensions; ++d) {
                if (dimensions_[d] > 1) {
                    edges_size_type edges_in_dim = num_vertices_ / dimensions_[d] * (dimensions_[d] - 1);
                    total += edges_in_dim;
                }
            }
            return total;
        }
    }
    
    [[nodiscard]] degree_size_type degree(vertex_descriptor v) const {
        if constexpr (Wrapped) {
            return 2 * Dimensions;  // Always 2 neighbors per dimension
        } else {
            degree_size_type deg = 0;
            auto coords = coordinates(v);
            for (std::size_t d = 0; d < Dimensions; ++d) {
                if (coords[d] > 0) ++deg;
                if (coords[d] < dimensions_[d] - 1) ++deg;
            }
            return deg;
        }
    }
    
    // -------------------------------------------------------------------------
    // Neighbor Access
    // -------------------------------------------------------------------------
    
    /// Get neighbor in a specific direction
    /// Returns nullopt if at boundary (for non-wrapped grids)
    [[nodiscard]] std::optional<vertex_descriptor> neighbor(
        vertex_descriptor v, std::size_t dim, bool positive) const 
    {
        auto coords = coordinates(v);
        
        if constexpr (Wrapped) {
            if (positive) {
                coords[dim] = (coords[dim] + 1) % dimensions_[dim];
            } else {
                coords[dim] = (coords[dim] + dimensions_[dim] - 1) % dimensions_[dim];
            }
            return vertex_at(coords);
        } else {
            if (positive) {
                if (coords[dim] >= dimensions_[dim] - 1) return std::nullopt;
                coords[dim]++;
            } else {
                if (coords[dim] == 0) return std::nullopt;
                coords[dim]--;
            }
            return vertex_at(coords);
        }
    }
    
    /// Check if edge exists between two vertices
    [[nodiscard]] bool has_edge(vertex_descriptor u, vertex_descriptor v) const {
        auto u_coords = coordinates(u);
        auto v_coords = coordinates(v);
        
        std::size_t diff_dims = 0;
        for (std::size_t d = 0; d < Dimensions; ++d) {
            auto diff = (u_coords[d] > v_coords[d]) 
                ? u_coords[d] - v_coords[d] 
                : v_coords[d] - u_coords[d];
            
            if constexpr (Wrapped) {
                // Handle wrap-around
                diff = std::min(diff, dimensions_[d] - diff);
            }
            
            if (diff == 1) ++diff_dims;
            else if (diff > 1) return false;
        }
        
        return diff_dims == 1;
    }
};

// =============================================================================
// Free Functions (ADL-findable)
// =============================================================================

// Vertex iteration
template<std::size_t D, bool W>
auto vertices(const grid_graph<D, W>& g) {
    return std::views::iota(std::size_t{0}, g.num_vertices());
}

template<std::size_t D, bool W>
auto num_vertices(const grid_graph<D, W>& g) {
    return g.num_vertices();
}

template<std::size_t D, bool W>
auto num_edges(const grid_graph<D, W>& g) {
    return g.num_edges();
}

// Out-edge iteration
template<std::size_t D, bool W>
auto out_edges(typename grid_graph<D, W>::vertex_descriptor v,
               const grid_graph<D, W>& g) {
    using edge_descriptor = typename grid_graph<D, W>::edge_descriptor;
    
    // Generate all possible neighbor directions
    std::vector<edge_descriptor> edges;
    edges.reserve(2 * D);
    
    auto src_coords = g.coordinates(v);
    
    for (std::size_t dim = 0; dim < D; ++dim) {
        for (bool positive : {false, true}) {
            if (auto n = g.neighbor(v, dim, positive)) {
                edge_descriptor e;
                e.source_coords = src_coords;
                e.target_coords = g.coordinates(*n);
                e.dimension = dim;
                e.positive = positive;
                edges.push_back(e);
            }
        }
    }
    
    return edges;
}

template<std::size_t D, bool W>
auto out_degree(typename grid_graph<D, W>::vertex_descriptor v,
                const grid_graph<D, W>& g) {
    return g.degree(v);
}

// For undirected, in_degree == out_degree
template<std::size_t D, bool W>
auto in_degree(typename grid_graph<D, W>::vertex_descriptor v,
               const grid_graph<D, W>& g) {
    return g.degree(v);
}

// Edge source/target
template<std::size_t D, bool W>
auto source(const typename grid_graph<D, W>::edge_descriptor& e,
            const grid_graph<D, W>& g) {
    return g.vertex_at(e.source_coords);
}

template<std::size_t D, bool W>
auto target(const typename grid_graph<D, W>::edge_descriptor& e,
            const grid_graph<D, W>& g) {
    return g.vertex_at(e.target_coords);
}

// =============================================================================
// Graph Traits Specialization
// =============================================================================

template<std::size_t D, bool W>
struct graph_traits<grid_graph<D, W>> {
    using graph_type = grid_graph<D, W>;
    
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
// Convenience Type Aliases
// =============================================================================

using grid_graph_2d = grid_graph<2>;
using grid_graph_3d = grid_graph<3>;

template<bool Wrapped = false>
using grid_graph_2d_wrapped = grid_graph<2, Wrapped>;

template<bool Wrapped = false>
using grid_graph_3d_wrapped = grid_graph<3, Wrapped>;

} // namespace bgl

// Hash for grid edge descriptor
template<std::size_t D>
struct std::hash<bgl::grid_edge<D>> {
    std::size_t operator()(const bgl::grid_edge<D>& e) const noexcept {
        std::size_t h = 0;
        for (std::size_t i = 0; i < D; ++i) {
            h ^= std::hash<std::size_t>{}(e.source_coords[i]) << (i * 4);
            h ^= std::hash<std::size_t>{}(e.target_coords[i]) << ((i + D) * 4);
        }
        return h ^ (e.dimension << 1) ^ e.positive;
    }
};

#endif // BGL_MODERN_GRID_GRAPH_HPP
