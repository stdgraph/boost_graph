// Test graph validation framework
#include <bgl/modern/validation.hpp>
#include <bgl/modern/adjacency_list.hpp>
#include <bgl/modern/adjacency_matrix.hpp>
#include <iostream>
#include <cassert>

using namespace bgl;

// Convenience aliases for directionality
using directed_t = directed_tag;
using undirected_t = undirected_tag;
using bidirectional_t = bidirectional_tag;

// =============================================================================
// Helper Functions
// =============================================================================

void test_valid_graph() {
    std::cout << "Testing valid graph...\n";
    
    // Create a simple valid graph
    adjacency_list<undirected_t> g(5);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    g.add_edge(3, 4);
    
    auto result = validate_graph(g);
    assert(result.is_valid());
    assert(result.error_count() == 0);
    assert(is_valid_graph(g));
    
    std::cout << "  ✓ Valid graph passes validation\n";
}

void test_validation_options() {
    std::cout << "Testing validation options...\n";
    
    // Graph with self-loop
    adjacency_list<directed_t> g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(1, 1);  // Self-loop
    
    // Default options don't check self-loops
    {
        auto result = validate_graph(g);
        assert(result.is_valid());
    }
    
    // Strict options check self-loops
    {
        auto result = validate_graph(g, validation_options::strict());
        assert(!result.is_valid());
        assert(result.error_count() >= 1);
        
        auto self_loop_errors = result.errors_of_type(validation_error_type::self_loop);
        assert(self_loop_errors.size() >= 1);
    }
    
    std::cout << "  ✓ Validation options work correctly\n";
}

void test_self_loops() {
    std::cout << "Testing self-loop detection...\n";
    
    adjacency_list<directed_t> g(4);
    g.add_edge(0, 0);  // Self-loop
    g.add_edge(0, 1);
    g.add_edge(2, 2);  // Another self-loop
    
    validation_options opts;
    opts.check_self_loops = true;
    
    auto result = validate_graph(g, opts);
    assert(!result.is_valid());
    
    auto self_loop_errors = result.errors_of_type(validation_error_type::self_loop);
    assert(self_loop_errors.size() == 2);
    
    std::cout << "  ✓ Self-loops detected correctly\n";
}

void test_parallel_edges() {
    std::cout << "Testing parallel edge detection...\n";
    
    // Use vecS for edge list to allow parallel edges
    adjacency_list<directed_t, vecS, vecS> g(3);
    g.add_edge(0, 1);
    g.add_edge(0, 1);  // Parallel edge
    g.add_edge(1, 2);
    g.add_edge(1, 2);  // Another parallel edge
    
    validation_options opts;
    opts.check_parallel_edges = true;
    
    auto result = validate_graph(g, opts);
    assert(!result.is_valid());
    
    auto parallel_errors = result.errors_of_type(validation_error_type::parallel_edge);
    assert(parallel_errors.size() >= 2);
    
    std::cout << "  ✓ Parallel edges detected correctly\n";
}

void test_error_formatting() {
    std::cout << "Testing error formatting...\n";
    
    adjacency_list<directed_t> g(3);
    g.add_edge(0, 0);  // Self-loop
    
    validation_options opts;
    opts.check_self_loops = true;
    
    auto result = validate_graph(g, opts);
    assert(!result.is_valid());
    
    // Check error formatting
    std::string formatted = result.format_errors();
    assert(!formatted.empty());
    assert(formatted.find("Self-loop") != std::string::npos);
    
    // Check individual error
    const auto& errors = result.errors();
    assert(!errors.empty());
    assert(errors[0].type == validation_error_type::self_loop);
    assert(errors[0].type_name() == std::string("Self-loop"));
    
    std::cout << "  ✓ Error formatting works correctly\n";
}

void test_directed_graph() {
    std::cout << "Testing directed graph validation...\n";
    
    adjacency_list<directed_t> g(4);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    g.add_edge(3, 0);  // Cycle
    
    auto result = validate_graph(g);
    assert(result.is_valid());
    
    std::cout << "  ✓ Valid directed graph passes\n";
}

void test_undirected_graph() {
    std::cout << "Testing undirected graph validation...\n";
    
    adjacency_list<undirected_t> g(5);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    g.add_edge(3, 4);
    g.add_edge(4, 0);
    
    auto result = validate_graph(g);
    assert(result.is_valid());
    
    std::cout << "  ✓ Valid undirected graph passes\n";
}

void test_bidirectional_graph() {
    std::cout << "Testing bidirectional graph validation...\n";
    
    adjacency_list<bidirectional_t> g(4);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    
    // Bidirectional graph should maintain in/out edge consistency
    auto result = validate_graph(g);
    assert(result.is_valid());
    
    std::cout << "  ✓ Valid bidirectional graph passes\n";
}

void test_empty_graph() {
    std::cout << "Testing empty graph validation...\n";
    
    adjacency_list<directed_t> g(0);
    
    auto result = validate_graph(g);
    assert(result.is_valid());
    assert(result.error_count() == 0);
    
    std::cout << "  ✓ Empty graph is valid\n";
}

void test_single_vertex() {
    std::cout << "Testing single vertex graph...\n";
    
    adjacency_list<directed_t> g(1);
    
    auto result = validate_graph(g);
    assert(result.is_valid());
    
    std::cout << "  ✓ Single vertex graph is valid\n";
}

void test_complete_graph() {
    std::cout << "Testing complete graph...\n";
    
    const std::size_t n = 5;
    adjacency_list<undirected_t> g(n);
    
    // Add all edges to make complete graph
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = i + 1; j < n; ++j) {
            g.add_edge(i, j);
        }
    }
    
    auto result = validate_graph(g);
    assert(result.is_valid());
    
    std::cout << "  ✓ Complete graph is valid\n";
}

void test_adjacency_matrix() {
    std::cout << "Testing adjacency matrix validation...\n";
    
    adjacency_matrix<directed_t> g(4);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    g.add_edge(3, 0);
    
    auto result = validate_graph(g);
    assert(result.is_valid());
    
    std::cout << "  ✓ Adjacency matrix validates correctly\n";
}

void test_minimal_validation() {
    std::cout << "Testing minimal validation...\n";
    
    adjacency_list<directed_t> g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    
    auto result = validate_graph(g, validation_options::minimal());
    assert(result.is_valid());
    
    std::cout << "  ✓ Minimal validation works\n";
}

void test_strict_validation() {
    std::cout << "Testing strict validation...\n";
    
    // Graph with self-loop and parallel edges
    adjacency_list<directed_t, vecS, vecS> g(3);
    g.add_edge(0, 0);  // Self-loop
    g.add_edge(0, 1);
    g.add_edge(0, 1);  // Parallel edge
    g.add_edge(1, 2);
    
    auto result = validate_graph(g, validation_options::strict());
    assert(!result.is_valid());
    assert(result.error_count() >= 2);  // At least self-loop and parallel edge
    
    std::cout << "  ✓ Strict validation catches all issues\n";
}

void test_validate_or_throw() {
    std::cout << "Testing validate_or_throw...\n";
    
    // Valid graph should not throw
    adjacency_list<directed_t> g1(3);
    g1.add_edge(0, 1);
    g1.add_edge(1, 2);
    
    bool threw = false;
    try {
        validate_graph_or_throw(g1);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(!threw);
    
    // Invalid graph should throw
    adjacency_list<directed_t> g2(2);
    g2.add_edge(0, 0);  // Self-loop
    
    validation_options opts;
    opts.check_self_loops = true;
    
    threw = false;
    try {
        validate_graph_or_throw(g2, opts);
    } catch (const std::runtime_error& e) {
        threw = true;
        std::string msg = e.what();
        assert(msg.find("validation failed") != std::string::npos);
    }
    assert(threw);
    
    std::cout << "  ✓ validate_or_throw works correctly\n";
}

void test_is_valid_graph() {
    std::cout << "Testing is_valid_graph...\n";
    
    adjacency_list<directed_t> g1(3);
    g1.add_edge(0, 1);
    g1.add_edge(1, 2);
    
    assert(is_valid_graph(g1));
    
    adjacency_list<directed_t> g2(2);
    g2.add_edge(0, 0);
    
    validation_options opts;
    opts.check_self_loops = true;
    
    assert(!is_valid_graph(g2, opts));
    
    std::cout << "  ✓ is_valid_graph works correctly\n";
}

void test_complex_graph() {
    std::cout << "Testing complex graph structure...\n";
    
    adjacency_list<bidirectional_t> g(10);
    
    // Create a more complex structure
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    g.add_edge(1, 4);
    g.add_edge(2, 5);
    g.add_edge(2, 6);
    g.add_edge(3, 7);
    g.add_edge(4, 7);
    g.add_edge(5, 8);
    g.add_edge(6, 8);
    g.add_edge(7, 9);
    g.add_edge(8, 9);
    
    auto result = validate_graph(g);
    assert(result.is_valid());
    
    std::cout << "  ✓ Complex graph validates correctly\n";
}

// =============================================================================
// Main Test Runner
// =============================================================================

int main() {
    std::cout << "=== Graph Validation Framework Tests ===\n\n";
    
    try {
        test_valid_graph();
        test_validation_options();
        test_self_loops();
        test_parallel_edges();
        test_error_formatting();
        test_directed_graph();
        test_undirected_graph();
        test_bidirectional_graph();
        test_empty_graph();
        test_single_vertex();
        test_complete_graph();
        test_adjacency_matrix();
        test_minimal_validation();
        test_strict_validation();
        test_validate_or_throw();
        test_is_valid_graph();
        test_complex_graph();
        
        std::cout << "\n✓ All validation tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}
