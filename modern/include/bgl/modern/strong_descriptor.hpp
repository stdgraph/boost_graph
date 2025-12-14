// BGL Modern - Strong Typing for Descriptors
// C++20 implementation with type-safe vertex/edge descriptors
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_STRONG_DESCRIPTOR_HPP
#define BGL_MODERN_STRONG_DESCRIPTOR_HPP

#include <bgl/modern/version.hpp>

#include <cstddef>
#include <compare>
#include <functional>
#include <concepts>
#include <type_traits>

namespace bgl {

// =============================================================================
// Descriptor Tags
// =============================================================================

/// Tag type for vertex descriptors
struct vertex_tag {};

/// Tag type for edge descriptors  
struct edge_tag {};

// =============================================================================
// descriptor<Tag> - Strong Type Wrapper
// =============================================================================

/// A strongly-typed descriptor wrapper that prevents mixing vertex and edge descriptors.
///
/// This template wraps an underlying index type (default std::size_t) with a tag
/// to provide compile-time type safety. Attempting to use a vertex_descriptor
/// where an edge_descriptor is expected (or vice versa) will result in a
/// compile-time error.
///
/// Example:
/// @code
///     using vertex_descriptor = descriptor<vertex_tag>;
///     using edge_descriptor = descriptor<edge_tag>;
///     
///     vertex_descriptor v{0};
///     edge_descriptor e{0};
///     
///     // v == e;  // Compile error! Different types
///     
///     auto v2 = v;  // OK
///     if (v < v2) { ... }  // OK - same type comparison
/// @endcode
///
template<typename Tag, typename IndexType = std::size_t>
class descriptor {
public:
    using tag_type = Tag;
    using index_type = IndexType;
    
    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------
    
    /// Default constructor - creates an invalid/null descriptor
    constexpr descriptor() noexcept = default;
    
    /// Construct from an index value
    constexpr explicit descriptor(index_type idx) noexcept 
        : index_(idx) 
    {}
    
    // -------------------------------------------------------------------------
    // Value Access
    // -------------------------------------------------------------------------
    
    /// Get the underlying index value
    [[nodiscard]] constexpr index_type value() const noexcept {
        return index_;
    }
    
    /// Implicit conversion to index_type for backward compatibility
    /// This allows using descriptors as array indices
    [[nodiscard]] constexpr operator index_type() const noexcept {
        return index_;
    }
    
    // -------------------------------------------------------------------------
    // Comparison Operators
    // -------------------------------------------------------------------------
    
    /// Three-way comparison (C++20 spaceship operator)
    [[nodiscard]] constexpr auto operator<=>(const descriptor&) const noexcept = default;
    
    /// Equality comparison
    [[nodiscard]] constexpr bool operator==(const descriptor&) const noexcept = default;
    
    // -------------------------------------------------------------------------
    // Validity Check
    // -------------------------------------------------------------------------
    
    /// Null/invalid descriptor constant
    static constexpr index_type null_value = static_cast<index_type>(-1);
    
    /// Check if descriptor is valid (not null)
    [[nodiscard]] constexpr bool valid() const noexcept {
        return index_ != null_value;
    }
    
    /// Check if descriptor is null/invalid
    [[nodiscard]] constexpr bool null() const noexcept {
        return index_ == null_value;
    }
    
    /// Create a null descriptor
    [[nodiscard]] static constexpr descriptor null_descriptor() noexcept {
        return descriptor{null_value};
    }
    
private:
    index_type index_ = null_value;
};

// =============================================================================
// Type Aliases for Common Use Cases
// =============================================================================

/// Strongly-typed vertex descriptor
using strong_vertex_descriptor = descriptor<vertex_tag>;

/// Strongly-typed edge descriptor (for simple edge index)
using strong_edge_index = descriptor<edge_tag>;

// =============================================================================
// strong_edge_descriptor - Full Edge Descriptor with Type Safety
// =============================================================================

/// A strongly-typed edge descriptor containing source, target, and index.
///
/// This provides type safety for edges in graphs where edge descriptors
/// contain vertex information. The source and target use strong_vertex_descriptor.
///
/// Example:
/// @code
///     strong_edge_descriptor e{
///         strong_vertex_descriptor{0},
///         strong_vertex_descriptor{1},
///         0
///     };
///     
///     auto src = e.source;  // strong_vertex_descriptor
///     auto tgt = e.target;  // strong_vertex_descriptor
/// @endcode
///
template<typename VertexDescriptor = strong_vertex_descriptor, 
         typename IndexType = std::size_t>
struct strong_edge_descriptor {
    using vertex_descriptor_type = VertexDescriptor;
    using index_type = IndexType;
    
    VertexDescriptor source;
    VertexDescriptor target;
    IndexType edge_index;
    
    /// Default comparison
    [[nodiscard]] constexpr auto operator<=>(const strong_edge_descriptor&) const noexcept = default;
    [[nodiscard]] constexpr bool operator==(const strong_edge_descriptor&) const noexcept = default;
    
    /// Get the edge index value
    [[nodiscard]] constexpr IndexType index() const noexcept {
        return edge_index;
    }
};

// =============================================================================
// Concept for Descriptor Types
// =============================================================================

/// Concept to check if a type is a descriptor
template<typename T>
concept Descriptor = requires {
    typename T::tag_type;
    typename T::index_type;
} && requires(T d) {
    { d.value() } -> std::convertible_to<typename T::index_type>;
    { d.valid() } -> std::same_as<bool>;
};

/// Concept to check if a type is a vertex descriptor
template<typename T>
concept VertexDescriptor = Descriptor<T> && std::same_as<typename T::tag_type, vertex_tag>;

/// Concept to check if a type is an edge descriptor
template<typename T>
concept EdgeDescriptor = Descriptor<T> && std::same_as<typename T::tag_type, edge_tag>;

// =============================================================================
// Type Traits
// =============================================================================

/// Check if a type is a descriptor
template<typename T>
struct is_descriptor : std::false_type {};

template<typename Tag, typename Index>
struct is_descriptor<descriptor<Tag, Index>> : std::true_type {};

template<typename T>
inline constexpr bool is_descriptor_v = is_descriptor<T>::value;

/// Check if a type is a vertex descriptor
template<typename T>
struct is_vertex_descriptor : std::false_type {};

template<typename Index>
struct is_vertex_descriptor<descriptor<vertex_tag, Index>> : std::true_type {};

template<typename T>
inline constexpr bool is_vertex_descriptor_v = is_vertex_descriptor<T>::value;

/// Check if a type is an edge descriptor
template<typename T>
struct is_edge_descriptor : std::false_type {};

template<typename Index>
struct is_edge_descriptor<descriptor<edge_tag, Index>> : std::true_type {};

template<typename T>
inline constexpr bool is_edge_descriptor_v = is_edge_descriptor<T>::value;

// =============================================================================
// Helper for Creating Descriptors
// =============================================================================

/// Create a vertex descriptor from an index
template<typename Index = std::size_t>
[[nodiscard]] constexpr auto make_vertex_descriptor(Index idx) noexcept {
    return descriptor<vertex_tag, Index>{idx};
}

/// Create an edge descriptor from an index
template<typename Index = std::size_t>
[[nodiscard]] constexpr auto make_edge_descriptor(Index idx) noexcept {
    return descriptor<edge_tag, Index>{idx};
}

} // namespace bgl

// =============================================================================
// Standard Library Specializations
// =============================================================================

/// Hash specialization for descriptor types
template<typename Tag, typename Index>
struct std::hash<bgl::descriptor<Tag, Index>> {
    [[nodiscard]] constexpr std::size_t operator()(
        const bgl::descriptor<Tag, Index>& d) const noexcept 
    {
        return std::hash<Index>{}(d.value());
    }
};

/// Hash specialization for strong_edge_descriptor
template<typename V, typename I>
struct std::hash<bgl::strong_edge_descriptor<V, I>> {
    [[nodiscard]] constexpr std::size_t operator()(
        const bgl::strong_edge_descriptor<V, I>& e) const noexcept 
    {
        // Combine source, target, and index hashes
        std::size_t h1 = std::hash<V>{}(e.source);
        std::size_t h2 = std::hash<V>{}(e.target);
        std::size_t h3 = std::hash<I>{}(e.edge_index);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

#endif // BGL_MODERN_STRONG_DESCRIPTOR_HPP
