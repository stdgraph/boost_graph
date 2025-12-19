// BGL Modern Example: Dijkstra with C++20 Ranges
// Demonstrates modern range-based operations with graph algorithms
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <bgl/modern/adjacency_list.hpp>
#include <bgl/modern/dijkstra_shortest_paths.hpp>
#include <iostream>
#include <ranges>
#include <vector>
#include <string>

using namespace bgl;

struct VertexProps {
    std::string name;
};

struct EdgeProps {
    double weight;
};

int main() {
    std::cout << "=== Dijkstra's Algorithm with C++20 Ranges ===\n\n";
    
    // Create a weighted graph
    adjacency_list<directed_tag, VertexProps, EdgeProps> g(6);
    
    // Set vertex names
    g[0].name = "A";
    g[1].name = "B";
    g[2].name = "C";
    g[3].name = "D";
    g[4].name = "E";
    g[5].name = "F";
    
    // Add weighted edges
    g.add_edge(0, 1, {.weight = 7.0});
    g.add_edge(0, 2, {.weight = 9.0});
    g.add_edge(0, 5, {.weight = 14.0});
    g.add_edge(1, 2, {.weight = 10.0});
    g.add_edge(1, 3, {.weight = 15.0});
    g.add_edge(2, 3, {.weight = 11.0});
    g.add_edge(2, 5, {.weight = 2.0});
    g.add_edge(3, 4, {.weight = 6.0});
    g.add_edge(4, 5, {.weight = 9.0});
    
    std::cout << "Graph with " << num_vertices(g) << " vertices:\n";
    for (auto v : vertices(g)) {
        std::cout << "  " << g[v].name << " -> ";
        
        // C++20 ranges: transform edges to target names
        auto neighbors = out_edges(v, g)
            | std::views::transform([&g](auto e) {
                return g[target(e, g)].name + 
                       " (" + std::to_string(g[e].weight) + ")";
            });
        
        for (const auto& neighbor : neighbors) {
            std::cout << neighbor << " ";
        }
        std::cout << "\n";
    }
    
    // Run Dijkstra's algorithm
    std::size_t start = 0;  // Start from vertex A
    
    auto weight_map = [&g](const auto& e) { return g[e].weight; };
    auto result = dijkstra_shortest_paths(g, start, weight_map);
    
    std::cout << "\n=== Shortest Paths from " << g[start].name << " ===\n";
    
    // C++20 ranges: filter and sort vertices by distance
    auto reachable_vertices = vertices(g)
        | std::views::filter([&result](auto v) {
            return result.distance_map()(v) < std::numeric_limits<double>::infinity();
        });
    
    std::vector<std::size_t> sorted_vertices;
    std::ranges::copy(reachable_vertices, std::back_inserter(sorted_vertices));
    
    std::ranges::sort(sorted_vertices, [&result](auto a, auto b) {
        return result.distance_map()(a) < result.distance_map()(b);
    });
    
    for (auto v : sorted_vertices) {
        std::cout << "To " << g[v].name << ": "
                  << "distance = " << result.distance_map()(v);
        
        // Reconstruct path
        if (v != start) {
            std::cout << ", path = " << g[start].name;
            std::vector<std::size_t> path;
            for (auto u = v; u != start; u = result.predecessor_map()(u)) {
                path.push_back(u);
            }
            std::ranges::reverse(path);
            for (auto u : path) {
                std::cout << " -> " << g[u].name;
            }
        }
        std::cout << "\n";
    }
    
    // Demonstrate range composition
    std::cout << "\n=== Vertices within distance 10 ===\n";
    
    auto close_vertices = vertices(g)
        | std::views::filter([&result](auto v) {
            return result.distance_map()(v) <= 10.0;
        })
        | std::views::transform([&g](auto v) {
            return g[v].name;
        });
    
    for (const auto& name : close_vertices) {
        std::cout << "  " << name << "\n";
    }
    
    // Find longest shortest path
    auto distances = vertices(g)
        | std::views::transform([&result](auto v) {
            return result.distance_map()(v);
        })
        | std::views::filter([](double d) {
            return d < std::numeric_limits<double>::infinity();
        });
    
    if (!std::ranges::empty(distances)) {
        auto max_dist = std::ranges::max(distances);
        std::cout << "\nMaximum shortest path distance: " << max_dist << "\n";
    }
    
    std::cout << "\n✓ Example complete!\n";
    return 0;
}
