// BGL Modern Example: BFS with Lambda Visitors
// Demonstrates modern callback-based graph traversal
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <bgl/modern/adjacency_list.hpp>
#include <bgl/modern/breadth_first_search.hpp>
#include <bgl/modern/visitor_callbacks.hpp>
#include <iostream>
#include <vector>
#include <string>

using namespace bgl;

struct VertexProps {
    std::string name;
    int value;
};

int main() {
    std::cout << "=== BFS with Lambda Visitors ===\n\n";
    
    // Create a graph representing a directory structure
    simple_adjacency_list<directed_tag, VertexProps> g(10);
    
    g[0] = {"root", 0};
    g[1] = {"home", 1};
    g[2] = {"etc", 1};
    g[3] = {"var", 1};
    g[4] = {"user1", 2};
    g[5] = {"user2", 2};
    g[6] = {"passwd", 2};
    g[7] = {"log", 2};
    g[8] = {"documents", 3};
    g[9] = {"pictures", 3};
    
    // Build tree structure
    g.add_edge(0, 1);  // root -> home
    g.add_edge(0, 2);  // root -> etc
    g.add_edge(0, 3);  // root -> var
    g.add_edge(1, 4);  // home -> user1
    g.add_edge(1, 5);  // home -> user2
    g.add_edge(2, 6);  // etc -> passwd
    g.add_edge(3, 7);  // var -> log
    g.add_edge(4, 8);  // user1 -> documents
    g.add_edge(4, 9);  // user1 -> pictures
    
    std::cout << "Directory tree traversal:\n\n";
    
    // Example 1: Simple discovery logging
    std::cout << "1. Discovery order:\n";
    {
        auto callbacks = on_discover_vertex([&g](auto v, const auto&) {
            std::cout << "  Discovered: " << g[v].name 
                      << " (level " << g[v].value << ")\n";
        });
        
        breadth_first_search(g, 0, callbacks);
    }
    
    // Example 2: Tree edge tracking
    std::cout << "\n2. Tree structure:\n";
    {
        auto callbacks = on_tree_edge([&g](auto e, const auto& graph) {
            auto src = source(e, graph);
            auto tgt = target(e, graph);
            std::cout << "  " << g[src].name << " -> " << g[tgt].name << "\n";
        });
        
        breadth_first_search(g, 0, callbacks);
    }
    
    // Example 3: Level-based processing
    std::cout << "\n3. Processing by level:\n";
    {
        std::vector<std::vector<std::string>> levels(4);
        
        auto callbacks = on_discover_vertex([&](auto v, const auto&) {
            levels[g[v].value].push_back(g[v].name);
        });
        
        breadth_first_search(g, 0, callbacks);
        
        for (std::size_t i = 0; i < levels.size(); ++i) {
            std::cout << "  Level " << i << ": ";
            for (const auto& name : levels[i]) {
                std::cout << name << " ";
            }
            std::cout << "\n";
        }
    }
    
    // Example 4: Comprehensive callbacks
    std::cout << "\n4. Detailed traversal events:\n";
    {
        int edge_count = 0;
        int vertex_count = 0;
        
        // For multiple callbacks, chain the helper functions
        auto callbacks = bfs_callbacks(
            [&](auto v, const auto& graph) { vertex_count++; },  // initialize
            [&g](auto v, const auto&) { std::cout << "  → Discovered " << g[v].name << "\n"; },  // discover
            [&g](auto v, const auto&) { std::cout << "  ⊙ Examining " << g[v].name << "\n"; },  // examine
            null_callback{},  // examine_edge
            [&](auto e, const auto& graph) { edge_count++; },  // tree_edge
            null_callback{},  // non_tree_edge
            null_callback{},  // gray_target
            null_callback{},  // black_target
            [&g](auto v, const auto&) { std::cout << "  ✓ Finished " << g[v].name << "\n"; }  // finish
        );
        
        breadth_first_search(g, 0, callbacks);
        
        std::cout << "\n  Total: " << vertex_count << " vertices, "
                  << edge_count << " tree edges\n";
    }
    
    // Example 5: Early termination with state
    std::cout << "\n5. Search until target found:\n";
    {
        std::string target = "documents";
        bool found = false;
        std::size_t steps = 0;
        
        auto callbacks = on_discover_vertex([&](auto v, const auto&) {
            steps++;
            std::cout << "  Step " << steps << ": " << g[v].name;
            
            if (g[v].name == target) {
                std::cout << " ← FOUND!\n";
                found = true;
            } else {
                std::cout << "\n";
            }
        });
        
        breadth_first_search(g, 0, callbacks);
        
        std::cout << "  Found '" << target << "' in " << steps << " steps\n";
    }
    
    // Example 6: Statistical collection
    std::cout << "\n6. Statistics:\n";
    {
        struct Stats {
            int total_vertices = 0;
            int leaf_nodes = 0;
            int internal_nodes = 0;
            double avg_out_degree = 0.0;
        } stats;
        
        // Multiple events require the full bfs_callbacks struct
        // We pass all callbacks in constructor order
        auto examine_cb = [&](auto v, const auto& graph) {
            stats.total_vertices++;
            auto degree = out_degree(v, graph);
            stats.avg_out_degree += degree;
            
            if (degree == 0) {
                stats.leaf_nodes++;
            } else {
                stats.internal_nodes++;
            }
        };
        
        auto callbacks = bfs_callbacks<
            null_callback,  // initialize
            null_callback,  // discover
            decltype(examine_cb),  // examine
            null_callback,  // examine_edge
            null_callback,  // tree_edge
            null_callback,  // non_tree_edge
            null_callback,  // gray_target
            null_callback,  // black_target
            null_callback   // finish
        >{
            .on_examine_vertex = examine_cb
        };
        
        breadth_first_search(g, 0, callbacks);
        
        stats.avg_out_degree /= stats.total_vertices;
        
        std::cout << "  Total vertices: " << stats.total_vertices << "\n";
        std::cout << "  Leaf nodes: " << stats.leaf_nodes << "\n";
        std::cout << "  Internal nodes: " << stats.internal_nodes << "\n";
        std::cout << "  Average out-degree: " << stats.avg_out_degree << "\n";
    }
    
    std::cout << "\n✓ Example complete!\n";
    return 0;
}
