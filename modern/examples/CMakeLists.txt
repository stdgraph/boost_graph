# BGL Modern - Examples
# C++20 Graph Library Examples

# Dijkstra with ranges
add_executable(dijkstra_ranges dijkstra_ranges.cpp)
target_link_libraries(dijkstra_ranges PRIVATE bgl::modern)

# BFS with lambda visitors
add_executable(bfs_lambda_visitor bfs_lambda_visitor.cpp)
target_link_libraries(bfs_lambda_visitor PRIVATE bgl::modern)

# Custom graph type
add_executable(custom_graph_type custom_graph_type.cpp)
target_link_libraries(custom_graph_type PRIVATE bgl::modern)

message(STATUS "BGL Modern examples configured")
