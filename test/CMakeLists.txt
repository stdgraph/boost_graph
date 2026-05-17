# Copyright 2026 Boost.Graph contributors
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#
# CMake parity for test/Jamfile.v2 (step D-1 / D-2 of testorg_plan).
#
# Rules (see .github/agents/testorg_plan.md, Phase D and Step E-4):
#   * One add_executable per test — never file(GLOB).
#   * Compile-only tests use add_library(... OBJECT ...) and no add_test.
#   * Benchmarks are NOT registered with add_test (mirror the Jamfile).
#   * Compile-fail tests are not yet expressed here; see the TODO at the
#     bottom of this file.
#   * Optional Boost libraries (timer, serialization, filesystem, system)
#     are guarded with if(TARGET ...) so the test tree works in pruned
#     boost builds.

cmake_minimum_required(VERSION 3.12)

# ---- Data file path constants (mirror Jamfile path-constants) ----
set(BGL_TEST_SRC_DIR                "${CMAKE_CURRENT_SOURCE_DIR}")
set(BGL_TEST_DATA_DIR               "${CMAKE_CURRENT_SOURCE_DIR}/data")
set(BGL_PLANAR_INPUT_FILES          "${BGL_TEST_DATA_DIR}/planar_input_graphs")
set(BGL_CYCLE_RATIO_INPUT_FILE      "${BGL_TEST_DATA_DIR}/cycle_ratio_s382.90.dot")
set(BGL_METIS_INPUT_FILE            "${BGL_TEST_DATA_DIR}/weighted_graph.gr")
set(BGL_WEIGHTED_MATCHING_INPUT_FILE "${BGL_TEST_DATA_DIR}/weighted_matching.dat")
set(BGL_GRAPHML_INPUT_FILE          "${BGL_TEST_DATA_DIR}/graphml_test.xml")
set(BGL_METRIC_TSP_INPUT_FILE       "${BGL_TEST_DATA_DIR}/metric_tsp_approx.graph")

# ---- Helpers ----
function(bgl_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE Boost::graph)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

function(bgl_test_with_args name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE Boost::graph)
    add_test(NAME ${name} COMMAND ${name} ${ARGN})
endfunction()

function(bgl_compile_only name)
    add_library(${name} OBJECT ${name}.cpp)
    target_link_libraries(${name} PRIVATE Boost::graph)
endfunction()

# ---- Plain runtime tests (no extra args, no extra libs) ----
bgl_test(test_graphs)
bgl_test(index_graph_test)
bgl_test(labeled_graph_test)
bgl_test(adj_list_edge_list_set_test)
bgl_test(adj_list_loops_test)
bgl_test(bfs_test)
bgl_test(bellman_test)
bgl_test(bidir_remove_edge_test)
bgl_test(bipartite_test)
bgl_test(csr_graph_test)   # Jamfile gates this to <variant>release; CMake runs it in all configs.
bgl_test(dag_longest_paths_test)
bgl_test(dfs_test)
bgl_test(undirected_dfs_test)
bgl_test(disjoint_set_test)
bgl_test(dominator_tree_test)
bgl_test(filter_graph_vp_test)
bgl_test(generator_test)
bgl_test(gursoy_atun_layout_test)
bgl_test(layout_test)
bgl_test(sequential_vertex_coloring_test)
bgl_test(subgraph_test)
bgl_test(subgraph_bundled_test)
bgl_test(subgraph_props_test)
bgl_test(isomorphism_test)
bgl_test(adjacency_matrix_test)
bgl_test(bundled_properties_test)
bgl_test(floyd_warshall_test)
bgl_test(astar_search_test)
bgl_test(biconnected_components_test)
bgl_test(min_degree_empty_test)
bgl_test(cuthill_mckee_ordering_test)
bgl_test(king_ordering_test)
bgl_test(matching_test)
bgl_test(weighted_matching_edge_cases_test)
bgl_test(max_flow_test)
bgl_test(boykov_kolmogorov_max_flow_test)
bgl_test(basic_planarity_test)
bgl_test(make_connected_test)
bgl_test(make_bicon_planar_test)
bgl_test(make_maximal_planar_test)
bgl_test(named_vertices_test)
bgl_test(r_c_shortest_paths_test)
bgl_test(rcsp_custom_vertex_id_test)
bgl_test(rcsp_single_solution_test)
bgl_test(is_straight_line_draw_test)
bgl_test(bron_kerbosch_all_cliques_test)
bgl_test(tiernan_all_cycles_test)
bgl_test(closeness_centrality_test)
bgl_test(degree_centrality_test)
bgl_test(mean_geodesic_test)
bgl_test(eccentricity_test)
bgl_test(clustering_coefficient_test)
bgl_test(core_numbers_test)
bgl_test(louvain_quality_function_test)
bgl_test(louvain_clustering_test)
bgl_test(read_propmap_test)
bgl_test(mcgregor_subgraphs_test)
bgl_test(grid_graph_test)
bgl_test(incremental_components_test)
bgl_test(two_graphs_common_spanning_trees_test)
bgl_test(random_spanning_tree_test)
bgl_test(vf2_sub_graph_iso_test)
bgl_test(vf2_sub_graph_iso_empty_graph_test)
bgl_test(hawick_circuits_test)
bgl_test(successive_shortest_path_nonnegative_weights_test)
bgl_test(cycle_canceling_test)
bgl_test(strong_components_test)
bgl_test(find_flow_cost_bundled_properties_and_named_params_test)
bgl_test(max_flow_algorithms_bundled_properties_and_named_params_test)
bgl_test(delete_edge_test)
bgl_test(johnson_test)
bgl_test(lvalue_pmap_test)

# ---- Runtime tests that take command-line arguments ----
# 100 is the random-graph vertex count (correctness guard; default is 300).
bgl_test_with_args(betweenness_centrality_test 100)
bgl_test_with_args(subgraph_add_test "${BGL_TEST_SRC_DIR}")
bgl_test_with_args(metis_test "${BGL_METIS_INPUT_FILE}")
bgl_test_with_args(weighted_matching_test "${BGL_WEIGHTED_MATCHING_INPUT_FILE}")
bgl_test_with_args(mas_test "${BGL_TEST_SRC_DIR}")
bgl_test_with_args(stoer_wagner_test "${BGL_TEST_DATA_DIR}")
bgl_test_with_args(graphml_test "${BGL_GRAPHML_INPUT_FILE}")
bgl_test_with_args(graphviz_test --log_level=all)

# ---- Runtime tests that need optional Boost libraries ----
# transitive_closure_test links Boost::timer in the Jamfile.
bgl_test(transitive_closure_test)
if(TARGET Boost::timer)
    target_link_libraries(transitive_closure_test PRIVATE Boost::timer)
endif()

# metric_tsp_approx_test links Boost::timer and takes a data file path.
bgl_test_with_args(metric_tsp_approx_test "${BGL_METRIC_TSP_INPUT_FILE}")
if(TARGET Boost::timer)
    target_link_libraries(metric_tsp_approx_test PRIVATE Boost::timer)
endif()

# cycle_ratio_test takes a data file path; Jamfile also pulls in Boost::graph
# explicitly (already linked via bgl_test_with_args).
bgl_test_with_args(cycle_ratio_test "${BGL_CYCLE_RATIO_INPUT_FILE}")

bgl_test(serialize_test)
if(TARGET Boost::serialization)
    target_link_libraries(serialize_test PRIVATE Boost::serialization)
endif()

# ---- Multi-variant tests: graph_test.cpp built 9 times with TEST=1..9 ----
# Mirrors the Jamfile loop. See the TEST=N comment block at the top of
# test/graph_test.cpp for what each value exercises.
foreach(N RANGE 1 9)
    add_executable(graph_${N} graph_test.cpp)
    target_link_libraries(graph_${N} PRIVATE Boost::graph)
    target_compile_definitions(graph_${N} PRIVATE TEST=${N})
    add_test(NAME graph_${N} COMMAND graph_${N})
endforeach()

# ---- Multi-variant compile-only: property_iter.cpp x9 ----
# See the TEST=N comment block at the top of test/property_iter.cpp.
foreach(N RANGE 1 9)
    add_library(property_iter_${N} OBJECT property_iter.cpp)
    target_link_libraries(property_iter_${N} PRIVATE Boost::graph)
    target_compile_definitions(property_iter_${N} PRIVATE TEST=${N})
endforeach()

# ---- Compile-only tests in test/ ----
bgl_compile_only(copy)
bgl_compile_only(swap)
bgl_compile_only(dimacs)
bgl_compile_only(filtered_graph_properties_dijkstra)

# ---- Filesystem-dependent runtime tests ----
# The Jamfile alias graph_test_with_filesystem disables these on darwin +
# cxxstd=11. We rely instead on whether Boost::filesystem is available.
if(TARGET Boost::filesystem AND TARGET Boost::system)
    add_executable(all_planar_input_files_test all_planar_input_files_test.cpp)
    target_link_libraries(all_planar_input_files_test
        PRIVATE Boost::graph Boost::filesystem Boost::system)
    add_test(NAME all_planar_input_files_test
        COMMAND all_planar_input_files_test "${BGL_PLANAR_INPUT_FILES}")

    add_executable(parallel_edges_loops_test parallel_edges_loops_test.cpp)
    target_link_libraries(parallel_edges_loops_test
        PRIVATE Boost::graph Boost::filesystem Boost::system)
    add_test(NAME parallel_edges_loops_test
        COMMAND parallel_edges_loops_test "${BGL_PLANAR_INPUT_FILES}")
endif()

# ---- Concept checks (compile-only, in test/concepts/) ----
foreach(cc IN ITEMS
        adj_list_cc
        adj_matrix_cc
        bfs_cc
        dfs_cc
        dijkstra_cc
        edge_list_cc
        filtered_graph_cc
        grid_graph_cc
        reverse_graph_cc
        vector_graph_cc
        graph_concepts)
    add_library(${cc} OBJECT concepts/${cc}.cpp)
    target_link_libraries(${cc} PRIVATE Boost::graph)
endforeach()

# ---- Clustering concept compile-pass tests (runtime; mirror Jamfile run) ----
foreach(t IN ITEMS
        compile_louvain_graph_types_test
        compile_louvain_quality_function_test)
    add_executable(${t} concepts/clustering/${t}.cpp)
    target_link_libraries(${t} PRIVATE Boost::graph)
    add_test(NAME ${t} COMMAND ${t})
endforeach()

# TODO(boostorg/graph#issue): the three compile-fail tests under
# concepts/clustering/ (compile_fail_louvain_directed.cpp,
# compile_fail_louvain_quality_function_empty.cpp,
# compile_fail_louvain_quality_function_invalid.cpp) have no direct CMake
# equivalent. b2's `compile-fail` rule has no built-in counterpart; the
# usual workaround is a per-file try_compile() driven by execute_process,
# which is out of scope for D-1. Tracked as a follow-up.

# ---- Regression tests (test/regressions/) ----
add_executable(finish_edge_bug_test regressions/finish_edge_bug_test.cpp)
target_link_libraries(finish_edge_bug_test PRIVATE Boost::graph)
add_test(NAME finish_edge_bug_test COMMAND finish_edge_bug_test)

# ---- Benchmarks (test/benchmarks/) ----
# Per plan: benchmarks are NOT registered with add_test. Build them as
# executables so they remain compilable, but exclude them from `ctest`
# and from the default build (EXCLUDE_FROM_ALL).
foreach(b IN ITEMS
        dijkstra_no_color_map_compare_test
        dijkstra_heap_performance
        random_matching_test)
    add_executable(${b} benchmarks/${b}.cpp)
    target_link_libraries(${b} PRIVATE Boost::graph)
    set_target_properties(${b} PROPERTIES EXCLUDE_FROM_ALL TRUE)
endforeach()
# dijkstra_heap_performance links Boost::timer in the Jamfile.
if(TARGET Boost::timer)
    target_link_libraries(dijkstra_heap_performance PRIVATE Boost::timer)
endif()

# =====================================================================
# Step D-2 — Test cost tiers (labels for ctest filtering: -L <label>)
# =====================================================================
# Tiers:
#   fast    < 1s, no large iteration count (default; not tagged here)
#   medium  1-10s, iteration-count args, or release-only hint
#   slow    > 10s, layout/planning algorithms
#
# Only the known-heavyweight tests from the plan are labelled. Everything
# else defaults to fast. Refine as profiling data becomes available.

set_tests_properties(gursoy_atun_layout_test          PROPERTIES LABELS "slow")
set_tests_properties(layout_test                      PROPERTIES LABELS "slow")
set_tests_properties(betweenness_centrality_test      PROPERTIES LABELS "medium")
set_tests_properties(csr_graph_test                   PROPERTIES LABELS "medium")
set_tests_properties(two_graphs_common_spanning_trees_test
                                                      PROPERTIES LABELS "medium")
if(TARGET Boost::filesystem AND TARGET Boost::system)
    set_tests_properties(all_planar_input_files_test  PROPERTIES LABELS "slow")
    set_tests_properties(parallel_edges_loops_test    PROPERTIES LABELS "medium")
endif()
