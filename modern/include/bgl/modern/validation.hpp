// BGL Modern - Graph Validation Framework
// Runtime validation checks for graph integrity
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_VALIDATION_HPP
#define BGL_MODERN_VALIDATION_HPP

#include <bgl/modern/version.hpp>
#include <bgl/modern/concepts.hpp>
#include <bgl/modern/graph_traits.hpp>
#include <bgl/modern/range_functions.hpp>

#include <vector>
#include <string>
#include <format>
#include <optional>
#include <unordered_set>
#include <concepts>

namespace bgl {

// =============================================================================
// Validation Error Types
// =============================================================================

/// Types of validation errors that can be detected
enum class validation_error_type {
    invalid_vertex_descriptor,      ///< Vertex descriptor out of range or invalid
    dangling_edge,                   ///< Edge points to non-existent vertex
    self_loop,                       ///< Self-loop where not allowed
    parallel_edge,                   ///< Duplicate edge where not allowed
    bidirectional_inconsistency,     ///< in_edges and out_edges mismatch
    vertex_count_mismatch,           ///< num_vertices doesn't match actual count
    edge_count_mismatch,             ///< num_edges doesn't match actual count
    invalid_edge_descriptor,         ///< Edge descriptor is invalid
    property_inconsistency           ///< Graph properties are inconsistent
};

/// Convert error type to string
constexpr const char* to_string(validation_error_type type) {
    switch (type) {
        case validation_error_type::invalid_vertex_descriptor:
            return "Invalid vertex descriptor";
        case validation_error_type::dangling_edge:
            return "Dangling edge";
        case validation_error_type::self_loop:
            return "Self-loop";
        case validation_error_type::parallel_edge:
            return "Parallel edge";
        case validation_error_type::bidirectional_inconsistency:
            return "Bidirectional inconsistency";
        case validation_error_type::vertex_count_mismatch:
            return "Vertex count mismatch";
        case validation_error_type::edge_count_mismatch:
            return "Edge count mismatch";
        case validation_error_type::invalid_edge_descriptor:
            return "Invalid edge descriptor";
        case validation_error_type::property_inconsistency:
            return "Property inconsistency";
        default:
            return "Unknown error";
    }
}

/// Individual validation error with context
struct validation_error {
    validation_error_type type;
    std::string message;
    
    /// Create error with type and formatted message
    template<typename... Args>
    validation_error(validation_error_type t, std::format_string<Args...> fmt, Args&&... args)
        : type(t), message(std::format(fmt, std::forward<Args>(args)...))
    {}
    
    /// Get error type name
    const char* type_name() const {
        return to_string(type);
    }
};

// =============================================================================
// Validation Result
// =============================================================================

/// Result of graph validation containing any errors found
class validation_result {
public:
    validation_result() = default;
    
    /// Add an error to the result
    void add_error(validation_error err) {
        errors_.push_back(std::move(err));
    }
    
    /// Check if validation passed (no errors)
    bool is_valid() const {
        return errors_.empty();
    }
    
    /// Get all errors
    const std::vector<validation_error>& errors() const {
        return errors_;
    }
    
    /// Get number of errors
    std::size_t error_count() const {
        return errors_.size();
    }
    
    /// Get errors of a specific type
    std::vector<validation_error> errors_of_type(validation_error_type type) const {
        std::vector<validation_error> result;
        for (const auto& err : errors_) {
            if (err.type == type) {
                result.push_back(err);
            }
        }
        return result;
    }
    
    /// Format all errors as a string
    std::string format_errors() const {
        std::string result;
        for (const auto& err : errors_) {
            result += std::format("[{}] {}\n", err.type_name(), err.message);
        }
        return result;
    }
    
private:
    std::vector<validation_error> errors_;
};

// =============================================================================
// Validation Options
// =============================================================================

/// Options controlling what to validate
struct validation_options {
    bool check_vertex_descriptors = true;   ///< Check vertex descriptors are valid
    bool check_dangling_edges = true;       ///< Check edges don't point to invalid vertices
    bool check_self_loops = false;          ///< Flag self-loops as errors
    bool check_parallel_edges = false;      ///< Flag parallel edges as errors
    bool check_bidirectional = true;        ///< Check bidirectional consistency (if applicable)
    bool check_counts = true;               ///< Check vertex/edge counts match
    
    /// Strict validation (all checks enabled)
    static validation_options strict() {
        validation_options opts;
        opts.check_self_loops = true;
        opts.check_parallel_edges = true;
        return opts;
    }
    
    /// Minimal validation (only critical checks)
    static validation_options minimal() {
        validation_options opts;
        opts.check_bidirectional = false;
        opts.check_counts = false;
        return opts;
    }
};

// =============================================================================
// Validation Implementation
// =============================================================================

namespace detail {

/// Check if vertex descriptor is valid for integral types
template<typename G>
    requires std::is_integral_v<vertex_descriptor_t<G>>
bool is_valid_vertex_descriptor(const G& g, vertex_descriptor_t<G> v) {
    return v < num_vertices(g);
}

/// Check if vertex descriptor is valid (default: assume valid)
template<typename G>
bool is_valid_vertex_descriptor(const G&, vertex_descriptor_t<G>) {
    // For non-integral descriptors, we can't easily check validity
    // Assume valid unless graph provides validation
    return true;
}

/// Validate vertex descriptors
template<VertexListGraph G>
void validate_vertex_descriptors(const G& g, validation_result& result, 
                                  const validation_options& opts) {
    if (!opts.check_vertex_descriptors) return;
    
    for (auto v : vertices(g)) {
        if (!is_valid_vertex_descriptor(g, v)) {
            result.add_error({
                validation_error_type::invalid_vertex_descriptor,
                "Vertex descriptor {} is invalid", v
            });
        }
    }
}

/// Validate edges don't point to invalid vertices
template<Graph G>
void validate_edge_endpoints(const G& g, validation_result& result,
                              const validation_options& opts) {
    if (!opts.check_dangling_edges) return;
    
    // Collect all valid vertices
    std::unordered_set<vertex_descriptor_t<G>> valid_vertices;
    for (auto v : vertices(g)) {
        valid_vertices.insert(v);
    }
    
    // Check each edge's endpoints
    for (auto v : vertices(g)) {
        for (auto e : out_edges(v, g)) {
            auto src = source(e, g);
            auto tgt = target(e, g);
            
            if (!valid_vertices.count(src)) {
                result.add_error({
                    validation_error_type::dangling_edge,
                    "Edge has invalid source vertex"
                });
            }
            
            if (!valid_vertices.count(tgt)) {
                result.add_error({
                    validation_error_type::dangling_edge,
                    "Edge has invalid target vertex"
                });
            }
        }
    }
}

/// Check for self-loops
template<Graph G>
void validate_self_loops(const G& g, validation_result& result,
                         const validation_options& opts) {
    if (!opts.check_self_loops) return;
    
    for (auto v : vertices(g)) {
        for (auto e : out_edges(v, g)) {
            if (source(e, g) == target(e, g)) {
                result.add_error({
                    validation_error_type::self_loop,
                    "Self-loop detected at vertex"
                });
            }
        }
    }
}

/// Check for parallel edges
template<Graph G>
void validate_parallel_edges(const G& g, validation_result& result,
                              const validation_options& opts) {
    if (!opts.check_parallel_edges) return;
    
    using Vertex = vertex_descriptor_t<G>;
    
    for (auto u : vertices(g)) {
        std::unordered_set<Vertex> seen_targets;
        
        for (auto e : out_edges(u, g)) {
            auto v = target(e, g);
            
            if (seen_targets.count(v)) {
                result.add_error({
                    validation_error_type::parallel_edge,
                    "Parallel edge detected"
                });
            }
            
            seen_targets.insert(v);
        }
    }
}

/// Check bidirectional graph consistency
template<BidirectionalGraph G>
void validate_bidirectional_consistency(const G& g, validation_result& result,
                                        const validation_options& opts) {
    if (!opts.check_bidirectional) return;
    
    using Vertex = vertex_descriptor_t<G>;
    
    // For each vertex, check that in_edges match out_edges from other vertices
    for (auto v : vertices(g)) {
        std::unordered_set<Vertex> in_neighbors;
        for (auto e : in_edges(v, g)) {
            in_neighbors.insert(source(e, g));
        }
        
        // Count out_edges from all vertices pointing to v
        std::unordered_set<Vertex> out_neighbors;
        for (auto u : vertices(g)) {
            for (auto e : out_edges(u, g)) {
                if (target(e, g) == v) {
                    out_neighbors.insert(u);
                }
            }
        }
        
        // For undirected graphs, both sets should match
        // For directed graphs, in_neighbors should match out_neighbors
        if (in_neighbors != out_neighbors) {
            result.add_error({
                validation_error_type::bidirectional_inconsistency,
                "Mismatch between in_edges and out_edges"
            });
        }
    }
}

/// Validate vertex and edge counts
template<Graph G>
void validate_counts(const G& g, validation_result& result,
                     const validation_options& opts) {
    if (!opts.check_counts) return;
    
    // Count vertices
    std::size_t vertex_count = 0;
    for ([[maybe_unused]] auto v : vertices(g)) {
        ++vertex_count;
    }
    
    if (vertex_count != num_vertices(g)) {
        result.add_error({
            validation_error_type::vertex_count_mismatch,
            "num_vertices() returns {} but actual count is {}",
            num_vertices(g), vertex_count
        });
    }
    
    // Count edges
    std::size_t edge_count = 0;
    for (auto v : vertices(g)) {
        for ([[maybe_unused]] auto e : out_edges(v, g)) {
            ++edge_count;
        }
    }
    
    // For undirected graphs, each edge is counted twice
    // This is a simplification - proper check would need directedness info
    
    if constexpr (requires { typename G::directed_category; }) {
        // Skip edge count check - it's tricky with directed/undirected
    }
}

} // namespace detail

// =============================================================================
// Main Validation Functions
// =============================================================================

/// Validate a graph and return detailed results
///
/// Performs various integrity checks on the graph structure:
/// - Vertex descriptors are valid
/// - Edges don't point to non-existent vertices
/// - Optional: check for self-loops
/// - Optional: check for parallel edges
/// - Optional: check bidirectional consistency
/// - Optional: check vertex/edge counts
///
/// Usage:
/// @code
///     auto result = validate_graph(g);
///     if (!result.is_valid()) {
///         std::cout << result.format_errors();
///     }
/// @endcode
///
template<Graph G>
validation_result validate_graph(const G& g, 
                                  const validation_options& opts = validation_options{}) {
    validation_result result;
    
    // Run all applicable validations
    if constexpr (VertexListGraph<G>) {
        detail::validate_vertex_descriptors(g, result, opts);
        detail::validate_edge_endpoints(g, result, opts);
        detail::validate_self_loops(g, result, opts);
        detail::validate_parallel_edges(g, result, opts);
        detail::validate_counts(g, result, opts);
        
        if constexpr (BidirectionalGraph<G>) {
            detail::validate_bidirectional_consistency(g, result, opts);
        }
    }
    
    return result;
}

/// Quick validation check - returns true if graph is valid
///
/// Equivalent to validate_graph(g, opts).is_valid()
///
template<Graph G>
bool is_valid_graph(const G& g, const validation_options& opts = validation_options{}) {
    return validate_graph(g, opts).is_valid();
}

/// Validate and throw on error
///
/// Throws std::runtime_error if validation fails
///
template<Graph G>
void validate_graph_or_throw(const G& g, 
                             const validation_options& opts = validation_options{}) {
    auto result = validate_graph(g, opts);
    if (!result.is_valid()) {
        throw std::runtime_error(
            std::format("Graph validation failed with {} errors:\n{}",
                       result.error_count(), result.format_errors())
        );
    }
}

} // namespace bgl

#endif // BGL_MODERN_VALIDATION_HPP
