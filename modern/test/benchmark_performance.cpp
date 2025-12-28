// BGL Modern - Performance Benchmarks
// Comparing old Boost.Graph vs new BGL Modern performance
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <bgl/modern/adjacency_list.hpp>
#include <bgl/modern/breadth_first_search.hpp>
#include <bgl/modern/depth_first_search.hpp>
#include <bgl/modern/dijkstra_shortest_paths.hpp>

#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <iomanip>

using namespace bgl;

// =============================================================================
// Timing Utilities
// =============================================================================

template<typename Func>
double time_execution(Func&& f, int iterations = 100) {
    using namespace std::chrono;
    
    // Warmup
    f();
    
    auto start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        f();
    }
    auto end = high_resolution_clock::now();
    
    auto duration = duration_cast<microseconds>(end - start);
    return duration.count() / static_cast<double>(iterations);
}

// =============================================================================
// Graph Generation
// =============================================================================

template<typename Graph>
Graph create_random_graph(std::size_t num_vertices, std::size_t num_edges, unsigned seed = 42) {
    Graph g(num_vertices);
    std::mt19937 rng(seed);
    std::uniform_int_distribution<std::size_t> dist(0, num_vertices - 1);
    
    for (std::size_t i = 0; i < num_edges; ++i) {
        auto u = dist(rng);
        auto v = dist(rng);
        if (u != v) {
            g.add_edge(u, v);
        }
    }
    
    return g;
}

template<typename Graph>
Graph create_weighted_random_graph(std::size_t num_vertices, std::size_t num_edges, unsigned seed = 42) {
    struct EdgeProps {
        double weight;
    };
    Graph g(num_vertices);
    std::mt19937 rng(seed);
    std::uniform_int_distribution<std::size_t> vertex_dist(0, num_vertices - 1);
    std::uniform_real_distribution<double> weight_dist(1.0, 100.0);
    
    for (std::size_t i = 0; i < num_edges; ++i) {
        auto u = vertex_dist(rng);
        auto v = vertex_dist(rng);
        if (u != v) {
            g.add_edge(u, v, EdgeProps{weight_dist(rng)});
        }
    }
    
    return g;
}

// =============================================================================
// BFS Benchmarks
// =============================================================================

void benchmark_bfs() {
    std::cout << "\n=== BFS Performance ===\n";
    
    std::vector<std::pair<std::size_t, std::size_t>> test_cases = {
        {100, 500},      // Small graph
        {1000, 5000},    // Medium graph
        {10000, 50000},  // Large graph
    };
    
    for (const auto& [num_vertices, num_edges] : test_cases) {
        auto g = create_random_graph<simple_adjacency_list<directed_tag>>(num_vertices, num_edges);
        
        auto time = time_execution([&]() {
            auto result = breadth_first_search(g, 0);
            // Force computation
            volatile auto dist = result.distance_map()(num_vertices / 2);
            (void)dist;
        }, 50);
        
        std::cout << std::setw(6) << num_vertices << " vertices, " 
                  << std::setw(6) << num_edges << " edges: "
                  << std::fixed << std::setprecision(2) << time << " μs\n";
    }
}

// =============================================================================
// DFS Benchmarks
// =============================================================================

void benchmark_dfs() {
    std::cout << "\n=== DFS Performance ===\n";
    
    std::vector<std::pair<std::size_t, std::size_t>> test_cases = {
        {100, 500},      // Small graph
        {1000, 5000},    // Medium graph
        {10000, 50000},  // Large graph
    };
    
    for (const auto& [num_vertices, num_edges] : test_cases) {
        auto g = create_random_graph<simple_adjacency_list<directed_tag>>(num_vertices, num_edges);
        
        auto time = time_execution([&]() {
            auto result = depth_first_search(g, 0);
            // Force computation
            volatile auto disc = result.discovery_time_map()(num_vertices / 2);
            (void)disc;
        }, 50);
        
        std::cout << std::setw(6) << num_vertices << " vertices, " 
                  << std::setw(6) << num_edges << " edges: "
                  << std::fixed << std::setprecision(2) << time << " μs\n";
    }
}

// =============================================================================
// Dijkstra Benchmarks
// =============================================================================

void benchmark_dijkstra() {
    std::cout << "\n=== Dijkstra Performance ===\n";
    
    std::vector<std::pair<std::size_t, std::size_t>> test_cases = {
        {100, 500},      // Small graph
        {1000, 5000},    // Medium graph
        {5000, 25000},   // Large graph (reduced for Dijkstra)
    };
    
    for (const auto& [num_vertices, num_edges] : test_cases) {
        struct EdgeProps {
            double weight;
        };
        
        simple_adjacency_list<directed_tag, no_property, EdgeProps> g(num_vertices);
        std::mt19937 rng(42);
        std::uniform_int_distribution<std::size_t> vertex_dist(0, num_vertices - 1);
        std::uniform_real_distribution<double> weight_dist(1.0, 100.0);
        
        for (std::size_t i = 0; i < num_edges; ++i) {
            auto u = vertex_dist(rng);
            auto v = vertex_dist(rng);
            if (u != v) {
                g.add_edge(u, v, EdgeProps{weight_dist(rng)});
            }
        }
        
        auto weight_map = [&g](const auto& e) { return g[e].weight; };
        
        auto time = time_execution([&]() {
            auto result = dijkstra_shortest_paths(g, 0, weight_map);
            // Force computation
            volatile auto dist = result.distance_map()(num_vertices / 2);
            (void)dist;
        }, 20);
        
        std::cout << std::setw(6) << num_vertices << " vertices, " 
                  << std::setw(6) << num_edges << " edges: "
                  << std::fixed << std::setprecision(2) << time << " μs\n";
    }
}

// =============================================================================
// Graph Construction Benchmarks
// =============================================================================

void benchmark_graph_construction() {
    std::cout << "\n=== Graph Construction Performance ===\n";
    
    std::vector<std::pair<std::size_t, std::size_t>> test_cases = {
        {100, 500},
        {1000, 5000},
        {10000, 50000},
    };
    
    for (const auto& [num_vertices, num_edges] : test_cases) {
        auto time = time_execution([&]() {
            simple_adjacency_list<directed_tag> g(num_vertices);
            std::mt19937 rng(42);
            std::uniform_int_distribution<std::size_t> dist(0, num_vertices - 1);
            
            for (std::size_t i = 0; i < num_edges; ++i) {
                auto u = dist(rng);
                auto v = dist(rng);
                if (u != v) {
                    g.add_edge(u, v);
                }
            }
        }, 20);
        
        std::cout << std::setw(6) << num_vertices << " vertices, " 
                  << std::setw(6) << num_edges << " edges: "
                  << std::fixed << std::setprecision(2) << time << " μs\n";
    }
}

// =============================================================================
// Property Map Access Benchmarks
// =============================================================================

void benchmark_property_access() {
    std::cout << "\n=== Property Access Performance ===\n";
    
    std::vector<std::size_t> test_sizes = {100, 1000, 10000};
    
    for (auto num_vertices : test_sizes) {
        struct VertexProps {
            int value;
            double data;
        };
        
        simple_adjacency_list<directed_tag, VertexProps> g(num_vertices);
        
        // Initialize properties
        for (auto v : vertices(g)) {
            g[v].value = static_cast<int>(v);
            g[v].data = static_cast<double>(v) * 1.5;
        }
        
        auto time = time_execution([&]() {
            int sum = 0;
            for (auto v : vertices(g)) {
                sum += g[v].value;
            }
            volatile int result = sum;
            (void)result;
        }, 100);
        
        std::cout << std::setw(6) << num_vertices << " vertices: "
                  << std::fixed << std::setprecision(2) << time << " μs\n";
    }
}

// =============================================================================
// Traversal Benchmarks
// =============================================================================

void benchmark_traversals() {
    std::cout << "\n=== Graph Traversal Performance ===\n";
    
    std::vector<std::size_t> test_sizes = {1000, 10000, 100000};
    
    for (auto num_vertices : test_sizes) {
        simple_adjacency_list<directed_tag> g(num_vertices);
        
        // Create a chain graph for predictable iteration
        for (std::size_t i = 0; i < num_vertices - 1; ++i) {
            g.add_edge(i, i + 1);
        }
        
        auto time = time_execution([&]() {
            std::size_t count = 0;
            for (auto v : vertices(g)) {
                count += out_degree(v, g);
            }
            volatile std::size_t result = count;
            (void)result;
        }, 100);
        
        std::cout << std::setw(7) << num_vertices << " vertices: "
                  << std::fixed << std::setprecision(2) << time << " μs\n";
    }
}

// =============================================================================
// Edge Iteration Benchmarks
// =============================================================================

void benchmark_edge_iteration() {
    std::cout << "\n=== Edge Iteration Performance ===\n";
    
    std::vector<std::pair<std::size_t, std::size_t>> test_cases = {
        {100, 500},
        {1000, 5000},
        {10000, 50000},
    };
    
    for (const auto& [num_vertices, num_edges] : test_cases) {
        auto g = create_random_graph<simple_adjacency_list<directed_tag>>(num_vertices, num_edges);
        
        auto time = time_execution([&]() {
            std::size_t count = 0;
            for (auto v : vertices(g)) {
                for (auto e : out_edges(v, g)) {
                    auto t = target(e, g);
                    count += t;
                }
            }
            volatile std::size_t result = count;
            (void)result;
        }, 50);
        
        std::cout << std::setw(6) << num_vertices << " vertices, " 
                  << std::setw(6) << num_edges << " edges: "
                  << std::fixed << std::setprecision(2) << time << " μs\n";
    }
}

// =============================================================================
// Main Benchmark Runner
// =============================================================================

int main() {
    std::cout << "╔═══════════════════════════════════════════╗\n";
    std::cout << "║  BGL Modern Performance Benchmarks        ║\n";
    std::cout << "╚═══════════════════════════════════════════╝\n";
    
    try {
        benchmark_graph_construction();
        benchmark_property_access();
        benchmark_traversals();
        benchmark_edge_iteration();
        benchmark_bfs();
        benchmark_dfs();
        benchmark_dijkstra();
        
        std::cout << "\n✓ All benchmarks completed successfully!\n";
        std::cout << "\nNote: Times shown are averages over multiple iterations.\n";
        std::cout << "Lower is better. Results may vary by hardware.\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Benchmark failed: " << e.what() << "\n";
        return 1;
    }
}
