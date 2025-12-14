# BGL Modern - Test Suite
# C++20 Concept and Range Tests

enable_testing()

# C++20 feature verification test
add_executable(test_cpp20_features test_cpp20_features.cpp)
target_link_libraries(test_cpp20_features PRIVATE bgl::modern)
add_test(NAME cpp20_features COMMAND test_cpp20_features)

# Graph concepts test
add_executable(test_concepts test_concepts.cpp)
target_link_libraries(test_concepts PRIVATE bgl::modern)
add_test(NAME concepts COMMAND test_concepts)

# Graph traits test
add_executable(test_graph_traits test_graph_traits.cpp)
target_link_libraries(test_graph_traits PRIVATE bgl::modern)
add_test(NAME graph_traits COMMAND test_graph_traits)

# Range functions test
add_executable(test_range_functions test_range_functions.cpp)
target_link_libraries(test_range_functions PRIVATE bgl::modern)
add_test(NAME range_functions COMMAND test_range_functions)

# Property map test
add_executable(test_property_map test_property_map.cpp)
target_link_libraries(test_property_map PRIVATE bgl::modern)
add_test(NAME property_map COMMAND test_property_map)

# Algorithm result test
add_executable(test_algorithm_result test_algorithm_result.cpp)
target_link_libraries(test_algorithm_result PRIVATE bgl::modern)
add_test(NAME algorithm_result COMMAND test_algorithm_result)

# Dijkstra shortest paths test
add_executable(test_dijkstra test_dijkstra.cpp)
target_link_libraries(test_dijkstra PRIVATE bgl::modern)
add_test(NAME dijkstra COMMAND test_dijkstra)

# Bellman-Ford shortest paths test
add_executable(test_bellman_ford test_bellman_ford.cpp)
target_link_libraries(test_bellman_ford PRIVATE bgl::modern)
add_test(NAME bellman_ford COMMAND test_bellman_ford)

# Breadth-first search test
add_executable(test_bfs test_bfs.cpp)
target_link_libraries(test_bfs PRIVATE bgl::modern)
add_test(NAME bfs COMMAND test_bfs)

# Depth-first search test
add_executable(test_dfs test_dfs.cpp)
target_link_libraries(test_dfs PRIVATE bgl::modern)
add_test(NAME dfs COMMAND test_dfs)

# Algorithm parameters (named parameters) test
add_executable(test_algorithm_params test_algorithm_params.cpp)
target_link_libraries(test_algorithm_params PRIVATE bgl::modern)
add_test(NAME algorithm_params COMMAND test_algorithm_params)

# Visitor callbacks test
add_executable(test_visitors test_visitors.cpp)
target_link_libraries(test_visitors PRIVATE bgl::modern)
add_test(NAME visitors COMMAND test_visitors)

# Minimum spanning tree test
add_executable(test_mst test_mst.cpp)
target_link_libraries(test_mst PRIVATE bgl::modern)
add_test(NAME mst COMMAND test_mst)

# Connectivity algorithms test
add_executable(test_connectivity test_connectivity.cpp)
target_link_libraries(test_connectivity PRIVATE bgl::modern)
add_test(NAME connectivity COMMAND test_connectivity)

# Adjacency list container test
add_executable(test_adjacency_list test_adjacency_list.cpp)
target_link_libraries(test_adjacency_list PRIVATE bgl::modern)
add_test(NAME adjacency_list COMMAND test_adjacency_list)

# Strong descriptor test
add_executable(test_strong_descriptor test_strong_descriptor.cpp)
target_link_libraries(test_strong_descriptor PRIVATE bgl::modern)
add_test(NAME strong_descriptor COMMAND test_strong_descriptor)

# Adjacency matrix container test
add_executable(test_adjacency_matrix_container test_adjacency_matrix_container.cpp)
target_link_libraries(test_adjacency_matrix_container PRIVATE bgl::modern)
add_test(NAME adjacency_matrix_container COMMAND test_adjacency_matrix_container)

# CSR graph container test
add_executable(test_csr_graph test_csr_graph.cpp)
target_link_libraries(test_csr_graph PRIVATE bgl::modern)
add_test(NAME csr_graph COMMAND test_csr_graph)

# Grid graph container test
add_executable(test_grid_graph test_grid_graph.cpp)
target_link_libraries(test_grid_graph PRIVATE bgl::modern)
add_test(NAME grid_graph COMMAND test_grid_graph)

# Coroutine traversal test
add_executable(test_coroutine_traverse test_coroutine_traverse.cpp)
target_link_libraries(test_coroutine_traverse PRIVATE bgl::modern)
add_test(NAME coroutine_traverse COMMAND test_coroutine_traverse)

# Parallel algorithms test
add_executable(test_parallel_algorithms test_parallel_algorithms.cpp)
target_link_libraries(test_parallel_algorithms PRIVATE bgl::modern)
add_test(NAME parallel_algorithms COMMAND test_parallel_algorithms)

# Composable algorithms test
add_executable(test_composable_algorithms test_composable_algorithms.cpp)
target_link_libraries(test_composable_algorithms PRIVATE bgl::modern)
add_test(NAME composable_algorithms COMMAND test_composable_algorithms)

# Validation framework test
add_executable(test_validation test_validation.cpp)
target_link_libraries(test_validation PRIVATE bgl::modern)
add_test(NAME validation COMMAND test_validation)

# Concept checking test (static assertions)
add_executable(test_concept_checking test_concept_checking.cpp)
target_link_libraries(test_concept_checking PRIVATE bgl::modern)
add_test(NAME concept_checking COMMAND test_concept_checking)

# Ranges integration test
add_executable(test_ranges_integration test_ranges_integration.cpp)
target_link_libraries(test_ranges_integration PRIVATE bgl::modern)
add_test(NAME ranges_integration COMMAND test_ranges_integration)

# Algorithm features test (structured returns, lambda visitors, named params)
add_executable(test_algorithm_features test_algorithm_features.cpp)
target_link_libraries(test_algorithm_features PRIVATE bgl::modern)
add_test(NAME algorithm_features COMMAND test_algorithm_features)

message(STATUS "BGL Modern tests configured")
