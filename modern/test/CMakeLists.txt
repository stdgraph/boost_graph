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

message(STATUS "BGL Modern tests configured")
