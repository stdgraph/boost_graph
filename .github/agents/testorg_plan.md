# Test Organization — Implementation Plan

Companion to [testorg_strategy.md](testorg_strategy.md). This document
provides concrete, agent-executable steps for each phase. Read the strategy
document first to understand the rationale; this document is the task list.

After completing **any step**, update [overview.md](overview.md) to reflect
the new state of the repository (directory layout, file counts, phase status).

---

## Before Starting Any Phase

1. Confirm the current working directory is the repo root
   (`boost/libs/graph`).
2. Read [test/Jamfile.v2](../../test/Jamfile.v2) to verify the current state
   matches the plan; earlier phases may already be partially complete.
3. Update the [Status](#status) table at the bottom of this file as steps
   are completed.

---

## Phase A — Mechanical moves

No logic changes. Every step is a `git mv` + include-path update. Each step
is independently reviewable.

---

### Step A-1 — Create target subdirectories

```
test/common/
test/concepts/
test/concepts/clustering/    ← will be created by moving concept_tests/clustering/
test/regressions/
test/data/
test/benchmarks/
```

Commands:
```sh
mkdir -p test/common test/concepts test/regressions test/data test/benchmarks
```

No source edits required. Verify with `ls test/`.

---

### Step A-2 — Move shared fixture headers to `test/common/`

**Files to move** (all currently in `test/`):

| Source | Destination |
|---|---|
| `test/graph_test.hpp` | `test/common/graph_test.hpp` |
| `test/graph_type.hpp` | `test/common/graph_type.hpp` |
| `test/typestr.hpp` | `test/common/typestr.hpp` |
| `test/cycle_test.hpp` | `test/common/cycle_test.hpp` |
| `test/test_graph.hpp` | `test/common/test_graph.hpp` |
| `test/test_construction.hpp` | `test/common/test_construction.hpp` |
| `test/test_destruction.hpp` | `test/common/test_destruction.hpp` |
| `test/test_direction.hpp` | `test/common/test_direction.hpp` |
| `test/test_iteration.hpp` | `test/common/test_iteration.hpp` |
| `test/test_properties.hpp` | `test/common/test_properties.hpp` |

**Do not move** `min_cost_max_flow_utils.hpp` — it stays next to its
consumers (see strategy §6).

**Move commands:**
```sh
cd test
git mv graph_test.hpp common/graph_test.hpp
git mv graph_type.hpp common/graph_type.hpp
git mv typestr.hpp    common/typestr.hpp
git mv cycle_test.hpp common/cycle_test.hpp
git mv test_graph.hpp common/test_graph.hpp
git mv test_construction.hpp common/test_construction.hpp
git mv test_destruction.hpp  common/test_destruction.hpp
git mv test_direction.hpp    common/test_direction.hpp
git mv test_iteration.hpp    common/test_iteration.hpp
git mv test_properties.hpp   common/test_properties.hpp
```

**Update `#include` directives:**

For each header, find all including files and update the path. The files
that remain in `test/` use a relative path like `"common/graph_test.hpp"`.
Files that move to a subdirectory (Phase A-3 onwards) will use
`"../common/graph_test.hpp"`.

Run per header to discover includers:
```sh
rg -l '"graph_test.hpp"'        test/
rg -l '"graph_type.hpp"'        test/
rg -l '"typestr.hpp"'           test/
rg -l '"cycle_test.hpp"'        test/
rg -l '"test_graph.hpp"'        test/
rg -l '"test_construction.hpp"' test/
rg -l '"test_destruction.hpp"'  test/
rg -l '"test_direction.hpp"'    test/
rg -l '"test_iteration.hpp"'    test/
rg -l '"test_properties.hpp"'   test/
```

For files still in `test/` (top-level): change `"foo.hpp"` →
`"common/foo.hpp"`.  
For files in subdirectories (after A-3 moves them): change `"foo.hpp"` →
`"../common/foo.hpp"`.

**Jamfile:** No changes needed in this step — headers are not listed in the
Jamfile.

**Verification:** `b2 test` (or a subset) must still pass after this step.

---

### Step A-3 — Move concept-check files to `test/concepts/`

**`*_cc.cpp` files to move** (stay as compile-only tests):

| Source | Destination |
|---|---|
| `test/adj_list_cc.cpp` | `test/concepts/adj_list_cc.cpp` |
| `test/adj_matrix_cc.cpp` | `test/concepts/adj_matrix_cc.cpp` |
| `test/bfs_cc.cpp` | `test/concepts/bfs_cc.cpp` |
| `test/dfs_cc.cpp` | `test/concepts/dfs_cc.cpp` |
| `test/dijkstra_cc.cpp` | `test/concepts/dijkstra_cc.cpp` |
| `test/edge_list_cc.cpp` | `test/concepts/edge_list_cc.cpp` |
| `test/filtered_graph_cc.cpp` | `test/concepts/filtered_graph_cc.cpp` |
| `test/grid_graph_cc.cpp` | `test/concepts/grid_graph_cc.cpp` |
| `test/reverse_graph_cc.cpp` | `test/concepts/reverse_graph_cc.cpp` |
| `test/vector_graph_cc.cpp` | `test/concepts/vector_graph_cc.cpp` |
| `test/graph_concepts.cpp` | `test/concepts/graph_concepts.cpp` |

**Existing `concept_tests/clustering/` directory** — rename the directory
to keep things consistent:
```sh
git mv test/concept_tests/clustering test/concepts/clustering
rmdir test/concept_tests
```
This is a directory move, not a file-by-file copy. After the move, remove the
now-empty `test/concept_tests/` parent directory if it still exists.

**SDB/LEDA conditional compile files** (only compiled when `-sSDB=` or
`-sLEDA=` is set) — move as well:

| Source | Destination |
|---|---|
| `test/stanford_graph_cc.cpp` | `test/concepts/stanford_graph_cc.cpp` |
| `test/leda_graph_cc.cpp` | `test/concepts/leda_graph_cc.cpp` |

**Move commands (representative):**
```sh
cd test
for f in adj_list_cc adj_matrix_cc bfs_cc dfs_cc dijkstra_cc edge_list_cc \
          filtered_graph_cc grid_graph_cc reverse_graph_cc vector_graph_cc \
          graph_concepts stanford_graph_cc leda_graph_cc; do
  git mv ${f}.cpp concepts/${f}.cpp
done
git mv concept_tests/clustering concepts/clustering
rmdir concept_tests
```

**Update `#include` directives** in moved files:  
Replace `"foo.hpp"` with `"../common/foo.hpp"` for any common header.

**Update `test/Jamfile.v2`:**  
Change every `[ compile adj_list_cc.cpp ]` → `[ compile concepts/adj_list_cc.cpp ]` etc.  
Change `concept_tests/clustering/...` references → `concepts/clustering/...`.

Change the SDB/LEDA section at the bottom:
```jam
compile concepts/stanford_graph_cc.cpp : ...
compile concepts/leda_graph_cc.cpp :     ...
```

**Verification:** `b2 test` must still pass.

---

### Step A-4 — Move regression tests to `test/regressions/`

**Files to move:**

| Source | Destination |
|---|---|
| `test/finish_edge_bug.cpp` | `test/regressions/finish_edge_bug.cpp` |
| `test/github-428-0.dot` | `test/regressions/github-428-0.dot` |
| `test/github-428-1.dot` | `test/regressions/github-428-1.dot` |

**Move commands:**
```sh
cd test
git mv finish_edge_bug.cpp regressions/finish_edge_bug.cpp
git mv github-428-0.dot    regressions/github-428-0.dot
git mv github-428-1.dot    regressions/github-428-1.dot
```

**Check if `finish_edge_bug.cpp` loads the `.dot` files** — if it does,
update the path it uses to locate them (check for any hardcoded path
strings or `TEST_DIR` usage in the source).

**Update `test/Jamfile.v2`:**  
Change `[ run finish_edge_bug.cpp ]` →
`[ run regressions/finish_edge_bug.cpp ]`.

Add a `path-constant REGRESSIONS_DIR : ./regressions ;` near the top of the
Jamfile alongside the other `path-constant` declarations, in case any
regression test needs to load its `.dot` files at runtime. Only add this
constant if the source or Jamfile actually needs a reusable regression-data
path after the move.

**Add a comment block** at the top of `regressions/finish_edge_bug.cpp`
referencing the original issue (check `git log -- test/finish_edge_bug.cpp`
to find the issue number).

**Verification:** `b2 test` must still pass.

---

### Step A-5 — Move data files to `test/data/`

**Files to move:**

| Source | Destination |
|---|---|
| `test/cycle_ratio_s382.90.dot` | `test/data/cycle_ratio_s382.90.dot` |
| `test/weighted_graph.gr` | `test/data/weighted_graph.gr` |
| `test/weighted_matching.dat` | `test/data/weighted_matching.dat` |
| `test/graphml_test.xml` | `test/data/graphml_test.xml` |
| `test/metric_tsp_approx.graph` | `test/data/metric_tsp_approx.graph` |
| `test/planar_input_graphs/` | `test/data/planar_input_graphs/` |
| `test/prgen_input_graphs/` | `test/data/prgen_input_graphs/` |

**Move commands:**
```sh
cd test
git mv cycle_ratio_s382.90.dot    data/cycle_ratio_s382.90.dot
git mv weighted_graph.gr          data/weighted_graph.gr
git mv weighted_matching.dat      data/weighted_matching.dat
git mv graphml_test.xml           data/graphml_test.xml
git mv metric_tsp_approx.graph    data/metric_tsp_approx.graph
git mv planar_input_graphs        data/planar_input_graphs
git mv prgen_input_graphs         data/prgen_input_graphs
```

**Update `test/Jamfile.v2` — path-constant declarations** (single-point
change; test source edits should usually not be required):
```jam
# Before:
path-constant TEST_DIR                  : . ;
path-constant PLANAR_INPUT_FILES        : ./planar_input_graphs ;
path-constant CYCLE_RATIO_INPUT_FILE    : ./cycle_ratio_s382.90.dot ;
path-constant METIS_INPUT_FILE          : ./weighted_graph.gr ;
path-constant WEIGHTED_MATCHING_INPUT_FILE : ./weighted_matching.dat ;

# After:
path-constant TEST_DIR                  : . ;
path-constant DATA_DIR                  : ./data ;
path-constant PLANAR_INPUT_FILES        : ./data/planar_input_graphs ;
path-constant CYCLE_RATIO_INPUT_FILE    : ./data/cycle_ratio_s382.90.dot ;
path-constant METIS_INPUT_FILE          : ./data/weighted_graph.gr ;
path-constant WEIGHTED_MATCHING_INPUT_FILE : ./data/weighted_matching.dat ;
```

Update the graphml and metric_tsp_approx entries in the Jamfile body:
```jam
# Before:
[ run graphml_test.cpp ... : : "graphml_test.xml" ]
[ run metric_tsp_approx.cpp ... : metric_tsp_approx.graph : : ]

# After:
[ run graphml_test.cpp ... : : "$(DATA_DIR)/graphml_test.xml" ]
[ run metric_tsp_approx.cpp ... : $(DATA_DIR)/metric_tsp_approx.graph : : ]
```

Also update the `mas_test.cpp` and `stoer_wagner_test.cpp` entries that
receive `$(TEST_DIR)` — verify whether they rely on it to find data files
now in `data/`; if so, change their Jamfile arguments to `$(DATA_DIR)`. Keep
this step Jamfile-only unless a specific test source proves it hardcodes an
old relative path.

**Verification:** `b2 test` must still pass.

---

## Phase B — Jamfile restructuring

Depends on Phase A. Each step should be a separate commit.

---

### Step B-1 — Add per-area Jamfiles

Create a minimal `Jamfile.v2` in each new subdirectory
(`concepts/`, `regressions/`) that declares a local test alias. Example
for `test/concepts/Jamfile.v2`:

```jam
project : requirements <library>/boost/graph//boost_graph ;

alias concepts_tests :
    [ compile adj_list_cc.cpp ]
    [ compile adj_matrix_cc.cpp ]
    [ compile bfs_cc.cpp ]
    [ compile dfs_cc.cpp ]
    [ compile dijkstra_cc.cpp ]
    [ compile edge_list_cc.cpp ]
    [ compile filtered_graph_cc.cpp ]
    [ compile grid_graph_cc.cpp ]
    [ compile reverse_graph_cc.cpp ]
    [ compile vector_graph_cc.cpp ]
    [ compile graph_concepts.cpp ]
    [ run clustering/compile_louvain_graph_types.cpp ]
    [ run clustering/compile_louvain_quality_function.cpp ]
    [ compile-fail clustering/compile_fail_louvain_directed.cpp ]
    [ compile-fail clustering/compile_fail_louvain_quality_function_empty.cpp ]
    [ compile-fail clustering/compile_fail_louvain_quality_function_invalid.cpp ]
    ;
```

Example for `test/regressions/Jamfile.v2`:
```jam
project : requirements <library>/boost/graph//boost_graph ;

alias regression_tests :
    [ run finish_edge_bug.cpp ]
    ;
```

---

### Step B-2 — Split the main Jamfile alias

Replace the monolithic `graph_test_regular` alias in `test/Jamfile.v2` so
it delegates to per-area aliases. Introduce subprojects for the subdirs that
have their own Jamfiles:

```jam
use-project /boost/graph/test/concepts   : concepts   ;
use-project /boost/graph/test/regressions : regressions ;

alias graph_test_regular :
    /boost/graph/test/concepts//concepts_tests
    /boost/graph/test/regressions//regression_tests
    # remaining top-level tests inline here until Phase B-1 fully covers them
    ;
```

The remaining top-level `.cpp` files (algorithms, structures, io, etc.) can
stay inline in `graph_test_regular` until they are reorganized in Phase C or
a dedicated follow-up PR.

---

### Step B-3 — Replace repetitive `graph.cpp` and `property_iter.cpp` blocks

**Current pattern in Jamfile (9 near-identical lines each):**
```jam
[ run graph.cpp : : : <define>TEST=1 : graph_1 ]
...
[ run graph.cpp : : : <define>TEST=9 : graph_9 ]

[ compile property_iter.cpp : <define>TEST=1 : property_iter_1 ]
...
[ compile property_iter.cpp : <define>TEST=9 : property_iter_9 ]
```

**Refactor target:** remove the repeated 9-line blocks without redefining
`graph_test_regular` inside a loop. Do **not** land a change that repeatedly
rebinds the alias.

Preferred implementation order:

1. Add the `TEST=N` meaning comments in the sources first (see Step E-1).
2. Introduce a small helper abstraction in `test/Jamfile.v2` only if the b2
   syntax is validated against `require-b2 5.0.1`.
3. If a clean helper cannot be expressed without risking Jamfile breakage,
   keep the explicit 9 entries and land only the source documentation in this
   step. Correctness matters more than deduplicating these lines.

**Acceptable helper shape:** a validated rule or variable expansion that emits
all nine `run` entries for `graph.cpp` and all nine `compile` entries for
`property_iter.cpp` while leaving a single final definition of
`graph_test_regular`.

**Also add a comment block in `graph.cpp` and `property_iter.cpp`** (see
Step E-1) before making this change, so the meaning of each `TEST=N` is
documented in the source.

---

### Step B-4 — Extract performance benchmarks to `test/benchmarks/`

**Files to move:**

| Source | Destination |
|---|---|
| `test/dijkstra_heap_performance.cpp` | `test/benchmarks/dijkstra_heap_performance.cpp` |

Also extract the benchmark-flavored run entries (those with large iteration
args) from `graph_test_regular`:
- `betweenness_centrality_test.cpp : 100` — consider whether 100 iterations
  is a correctness guard or a real benchmark. If the test is correct with
  `1` iteration, keep a `: 1` version in the main suite and add the `: 100`
  version in benchmarks.
- `dijkstra_no_color_map_compare.cpp : 10000` — move to benchmarks.
- `random_matching_test.cpp : 1000 1020` — move to benchmarks.

**Create `test/benchmarks/Jamfile.v2`:**
```jam
project : requirements <library>/boost/graph//boost_graph ;

# Not included in graph_test; run explicitly or via CI benchmark label.
alias benchmark_tests :
    [ run dijkstra_heap_performance.cpp /boost/timer//boost_timer : 10000 ]
    [ run dijkstra_no_color_map_compare.cpp : 10000 ]
    [ run random_matching_test.cpp : 1000 1020 ]
    ;
```

Remove the corresponding (currently commented-out) entries from
`graph_test_regular`. Do **not** include `benchmark_tests` in `graph_test`.

---

## Phase C — Naming and hygiene

Depends on Phase B. Steps can be done in parallel branches but each should
be a separate PR.

---

### Step C-1 — Rename test files to `*_test.cpp`

Rename every current `[ run ... ]` target in `test/Jamfile.v2` that does not
already end in `_test.cpp` or `_cc.cpp`, except for the `*2` variants deferred
to Step C-3 and the explicit framework-file exception noted below. The table
below was verified programmatically against the current Jamfile runtime
inventory, normalized through the Phase A-3, A-4, and B-4 path moves that must
already be in place before Phase C starts. If the Jamfile changes before this
step lands, regenerate the inventory and refresh the table in the same commit.

| Current name | Proposed new name | Notes |
|---|---|---|
| `test/adj_list_edge_list_set.cpp` | `test/adj_list_edge_list_set_test.cpp` | |
| `test/adj_list_loops.cpp` | `test/adj_list_loops_test.cpp` | |
| `test/bellman-test.cpp` | `test/bellman_test.cpp` | fix hyphen too |
| `test/bfs.cpp` | `test/bfs_test.cpp` | |
| `test/bidir_remove_edge.cpp` | `test/bidir_remove_edge_test.cpp` | |
| `test/bron_kerbosch_all_cliques.cpp` | `test/bron_kerbosch_all_cliques_test.cpp` | |
| `test/bundled_properties.cpp` | `test/bundled_properties_test.cpp` | |
| `test/closeness_centrality.cpp` | `test/closeness_centrality_test.cpp` | |
| `test/clustering_coefficient.cpp` | `test/clustering_coefficient_test.cpp` | |
| `test/concepts/clustering/compile_louvain_graph_types.cpp` | `test/concepts/clustering/compile_louvain_graph_types_test.cpp` | path assumes Phase A-3 already landed |
| `test/concepts/clustering/compile_louvain_quality_function.cpp` | `test/concepts/clustering/compile_louvain_quality_function_test.cpp` | path assumes Phase A-3 already landed |
| `test/cuthill_mckee_ordering.cpp` | `test/cuthill_mckee_ordering_test.cpp` | |
| `test/cycle_ratio_tests.cpp` | `test/cycle_ratio_test.cpp` | drop redundant plural while normalizing |
| `test/dag_longest_paths.cpp` | `test/dag_longest_paths_test.cpp` | |
| `test/degree_centrality.cpp` | `test/degree_centrality_test.cpp` | |
| `test/delete_edge.cpp` | `test/delete_edge_test.cpp` | |
| `test/dfs.cpp` | `test/dfs_test.cpp` | |
| `test/benchmarks/dijkstra_no_color_map_compare.cpp` | `test/benchmarks/dijkstra_no_color_map_compare_test.cpp` | path assumes Phase B-4 already landed |
| `test/eccentricity.cpp` | `test/eccentricity_test.cpp` | |
| `test/regressions/finish_edge_bug.cpp` | `test/regressions/finish_edge_bug_test.cpp` | path assumes Phase A-4 already landed |
| `test/graph.cpp` | `test/graph_test.cpp` | Check for name collision with `graph_test.hpp`; rename header first (already done in A-2) |
| `test/hawick_circuits.cpp` | `test/hawick_circuits_test.cpp` | |
| `test/index_graph.cpp` | `test/index_graph_test.cpp` | |
| `test/isomorphism.cpp` | `test/isomorphism_test.cpp` | |
| `test/johnson-test.cpp` | `test/johnson_test.cpp` | fix hyphen too |
| `test/king_ordering.cpp` | `test/king_ordering_test.cpp` | |
| `test/labeled_graph.cpp` | `test/labeled_graph_test.cpp` | |
| `test/lvalue_pmap.cpp` | `test/lvalue_pmap_test.cpp` | runtime entry in the Jamfile, not compile-only |
| `test/max_flow_algorithms_bundled_properties_and_named_params.cpp` | `test/max_flow_algorithms_bundled_properties_and_named_params_test.cpp` | |
| `test/mean_geodesic.cpp` | `test/mean_geodesic_test.cpp` | |
| `test/metric_tsp_approx.cpp` | `test/metric_tsp_approx_test.cpp` | |
| `test/min_degree_empty.cpp` | `test/min_degree_empty_test.cpp` | |
| `test/rcsp_custom_vertex_id.cpp` | `test/rcsp_custom_vertex_id_test.cpp` | |
| `test/rcsp_single_solution.cpp` | `test/rcsp_single_solution_test.cpp` | |
| `test/read_propmap.cpp` | `test/read_propmap_test.cpp` | |
| `test/sequential_vertex_coloring.cpp` | `test/sequential_vertex_coloring_test.cpp` | |
| `test/serialize.cpp` | `test/serialize_test.cpp` | |
| `test/subgraph.cpp` | `test/subgraph_test.cpp` | |
| `test/subgraph_add.cpp` | `test/subgraph_add_test.cpp` | |
| `test/subgraph_bundled.cpp` | `test/subgraph_bundled_test.cpp` | |
| `test/subgraph_props.cpp` | `test/subgraph_props_test.cpp` | |
| `test/tiernan_all_cycles.cpp` | `test/tiernan_all_cycles_test.cpp` | |
| `test/undirected_dfs.cpp` | `test/undirected_dfs_test.cpp` | |

Explicit exceptions and deferrals:

- `test/test_graphs.cpp` is a framework-driver exception for now. Keep it as-is
  unless the suite is reworked enough to justify a clearer driver name.
- Defer `test/transitive_closure_test2.cpp`,
  `test/vf2_sub_graph_iso_test_2.cpp`, and
  `test/weighted_matching_test2.cpp` to Step C-3, which decides whether each
  file is merged away or renamed to a descriptive `_test.cpp` name.
- Do not count compile-only or currently unlisted files toward this runtime
  inventory. In the current tree that includes `copy.cpp`, `swap.cpp`,
  `property_iter.cpp`, `dimacs.cpp`, `filtered_graph_properties_dijkstra.cpp`,
  `undirected_dfs_visitor.cpp`, and `bidir_vec_remove_edge.cpp`.

Use `git mv` for each rename. Update all corresponding Jamfile entries.

Before marking this step complete, rerun a programmatic audit and confirm that
the remaining non-`_test.cpp` runtime files are only the explicit exception and
the Step C-3 deferrals:

```sh
python3 - <<'PY'
from pathlib import Path
import re

jam = Path('test/Jamfile.v2').read_text().splitlines()
runtime = sorted({
    m.group(1)
    for line in jam
    for m in [re.match(r'\s*\[\s*run\s+([^\s\]]+\.cpp)\b', line)]
    if m
})

allowed = {
    'test_graphs.cpp',
    'transitive_closure_test2.cpp',
    'vf2_sub_graph_iso_test_2.cpp',
    'weighted_matching_test2.cpp',
}

remaining = [
    path for path in runtime
    if not path.endswith('_test.cpp') and not path.endswith('_cc.cpp')
    and path not in allowed
]

for path in remaining:
    print(path)

raise SystemExit(1 if remaining else 0)
PY
```

After renaming, verify no other file in the repo `#include`s a renamed
`.cpp` by name (unlikely but possible).

---

### Step C-2 — Migrate bare `assert` to Boost.Test

For each file **touched during Phase C** (not proactively across the whole
test suite):

1. Check whether it uses `#include <cassert>` / `assert(...)`.
2. If so, replace `assert(expr)` with `BOOST_TEST(expr)` (or
   `BOOST_CHECK(expr)` / `BOOST_REQUIRE(expr)` where appropriate).
3. Add `#include <boost/test/unit_test.hpp>` if not present.
4. Add a `BOOST_AUTO_TEST_CASE` wrapper if the file has a bare `main()`.

Do **not** migrate files that are not otherwise being touched in this phase
— leave those for a dedicated follow-up.

---

### Step C-3 — Resolve `*2` suffix variants

For each pair below, decide: **merge** (if the second test is a subset or
duplicate of the first) or **rename** (if it tests genuinely distinct
behavior).

| Pair | Action | Notes |
|---|---|---|
| `transitive_closure_test.cpp` / `transitive_closure_test2.cpp` | Investigate, then merge or rename | Check what `_test2` covers that `_test` does not |
| `vf2_sub_graph_iso_test.cpp` / `vf2_sub_graph_iso_test_2.cpp` | Investigate, then merge or rename | Hyphen vs. underscore inconsistency too |
| `weighted_matching_test.cpp` / `weighted_matching_test2.cpp` | Investigate, then merge or rename | |

**For each pair:**
1. Read both files.
2. Add a comment block at the top of the `*2` file describing what it tests
   differently from the primary (this is also required by Step E-2).
3. If merging: first preserve the distinguishing rationale in the primary file
  (or commit history), then fold the unique test cases into the primary file.
  Delete the `*2` file only after its distinct coverage intent has been
  captured and the Jamfile has been updated in the same commit.
4. If renaming: `git mv *2.cpp *_<descriptor>_test.cpp` (e.g.
   `transitive_closure_random_test.cpp`). Update Jamfile.

---

### Step C-4 — Resolve commented-out tests

The following entries are commented out in the Jamfile:

```jam
#[ run adj_list_invalidation.cpp ]
#[ run dijkstra_heap_performance.cpp /boost/timer//boost_timer : 10000 ]
#[ run relaxed_heap_test.cpp : 5000 15000 ]
```

**Actions:**

- `dijkstra_heap_performance.cpp` — already handled in Step B-4 (moved to
  benchmarks). Remove the commented-out line from `graph_test_regular`.

- `adj_list_invalidation.cpp` — Run `git log -- test/adj_list_invalidation.cpp`
  to find the rationale. Attach a comment:
  ```jam
  # TODO(#<issue>): re-enable once <reason>; see <commit-sha>
  #[ run adj_list_invalidation.cpp ]
  ```

- `relaxed_heap_test.cpp` — Same process. Note it is explicitly tagged
  "Unused and deprecated" in the current Jamfile. If confirmed deprecated,
  move the source to a `test/attic/` directory and leave the TODO comment.
  Do **not** delete the file (see strategy §8 rule 7).

---

## Phase D — CMake parity and CI tiers

D-1 depends on Phase C (the CMakeLists uses post-C-1 filenames); D-2 depends
only on D-1. Steps D-1 and D-2 can be done in a single PR once Phase C is
complete.

---

### Step D-1 — Create `test/CMakeLists.txt`

Create `test/CMakeLists.txt` mirroring the Jamfile structure.

**Structure:**
```cmake
cmake_minimum_required(VERSION 3.12)

# One target per test — never use file(GLOB)
# Mirror the Jamfile graph_test_regular alias

function(bgl_test name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} PRIVATE Boost::graph)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

# ---- Structures ----
bgl_test(test_graphs)
bgl_test(index_graph_test)
bgl_test(labeled_graph_test)
bgl_test(adjacency_matrix_test)
bgl_test(csr_graph_test)
bgl_test(grid_graph_test)
# ... (full list, one line per test)

# ---- Multi-variant tests (graph.cpp and property_iter.cpp) ----
# bgl_test() cannot express these — write them out explicitly:
add_executable(graph_1 graph_test.cpp)
target_compile_definitions(graph_1 PRIVATE TEST=1)
add_test(NAME graph_1 COMMAND graph_1)
# repeat for graph_2 .. graph_9 and property_iter_1 .. property_iter_9
# (property_iter is compile-only: use add_library(... OBJECT ...) with no add_test)

# ---- Concept checks (compile-only) ----
add_library(adj_list_cc OBJECT concepts/adj_list_cc.cpp)
# ... (one OBJECT target per *_cc.cpp)
```

**Rules:**
- Use a separate `add_executable` per test (not one combined binary).
- Use `add_test` for each runtime test.
- Do **not** add benchmarks to `add_test` (mirror the Jamfile's separation).
- Compile-only tests use `add_library(name OBJECT ...)` with no corresponding
  `add_test`.
- Compile-fail tests: skip for now (CMake has no built-in compile-fail
  mechanism without a try_compile wrapper); add a `TODO` comment.

**Update the root `CMakeLists.txt`** to add the test subdirectory:
```cmake
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()
```

---

### Step D-2 — Tag tests by cost

In the Jamfile, add `<test-info>` features to heavyweight tests. In
CMakeLists.txt, add `LABELS` to `set_tests_properties`. Agree on three
tiers:

| Tier | Criterion | Label |
|---|---|---|
| fast | <1 s, no large iteration count | `fast` |
| medium | 1–10 s or iteration-count args | `medium` |
| slow | >10 s, layout/planning algorithms | `slow` |

Apply the labels to the following known-heavyweight tests as a starting
point:
- `gursoy_atun_layout_test` — slow (layout)
- `layout_test` — slow (layout)
- `betweenness_centrality_test` — medium (100 iterations)
- `csr_graph_test` — medium (release-only in Jamfile is a hint)
- `all_planar_input_files_test` — medium/slow
- `two_graphs_common_spanning_trees_test` — medium

Leave all others untagged (default to `fast`) until profiling data is
available.

---

## Phase E prep — Coverage preparation

Depends on the step-level prerequisites below. These steps must be done
**before** any coverage run is attempted, but some of them intentionally
happen before later Phase B/C cleanup steps.

---

### Step E-1 — Document `TEST=N` intent in `graph.cpp` and `property_iter.cpp`

Open `test/graph.cpp` and `test/property_iter.cpp` (or their renamed
equivalents after Phase C-1). Add a comment block near the top (after
existing includes/copyright) that lists what each `TEST=N` value exercises.
Example:
```cpp
// TEST=N dispatch — what each value covers:
//   TEST=1 — adjacency_list<vecS,vecS,undirectedS>
//   TEST=2 — adjacency_list<listS,listS,directedS>
//   ...
//   (fill in by reading the #if TEST==N blocks below)
```

Do this **before** Step B-3 (the Jam loop refactor) so the loop's output
names remain meaningful.

---

### Step E-2 — Add coverage comments to `*2` variant files

For whichever `*2` files survive Step C-3 (not merged), add a comment at
the top:
```cpp
// Coverage note: this file tests <X> which is not covered by the primary
// <name>_test.cpp because <reason>.
```

---

### Step E-3 — Attach `TODO` comments to commented-out tests

If not already done in Step C-4, ensure every commented-out Jamfile entry
has a `# TODO(#issue):` annotation before this phase closes.

---

### Step E-4 — Verify CMakeLists.txt is coverage-friendly

Review `test/CMakeLists.txt` produced in Step D-1:
- Confirm every test is a **separate `add_executable`** — no combined binary.
- Confirm **no `file(GLOB)`** is used anywhere.
- Confirm that all test executables link only what they actually use (avoid
  pulling in unnecessary libraries that would inflate coverage noise).

---

### Step E-5 — Create `.github/coverage/` placeholder

```sh
mkdir -p .github/coverage
touch .github/coverage/.gitkeep
```

Add a `README.md` inside:
```markdown
# Coverage

Placeholder for coverage instrumentation presets, CI workflow additions,
and reporting scripts. Implementation deferred — see testorg_strategy.md §9.
```

Commit with message `chore: reserve .github/coverage/ for future use`.

---

## Notes for Agents

- **One commit per step.** Each step is independently reviewable.
- **Run `b2 test` after every Phase A step** to confirm no regressions before
  moving to the next step.
- **Update this plan's [Status](#status) table** and [overview.md](overview.md)
  after each completed step.
- **Use the `Depends on` column in the status table** to confirm prerequisites
  before starting a step.
- **Do not combine Phase A steps into one commit** — the diffs are easier to
  review and revert when kept separate.
- Prefer `rg` / `rg --files` over `grep` / `find` for repository searches.
- When in doubt about a file's category, check the strategy document (§4
  target layout) before inventing a new subdirectory.

---

## Status

Track progress here. Update `Status`, `Depends on`, and `Notes` as steps
complete.

| Step | Title | Depends on | Status | Notes |
|---|---|---|---|---|
| A-1 | Create subdirectories | none | complete | Done implicitly by A-2 moves (git tracks no empty dirs) |
| A-2 | Move shared headers → `common/` | A-1 | complete | commit 5a93569b |
| A-3 | Move concept checks → `concepts/` | A-1, A-2 | complete | commit 876cc7f9 |
| A-4 | Move regression tests → `regressions/` | A-1 | complete | commit 274840be; github-428-*.dot moved to data/ in A-5 (used by isomorphism.cpp, not finish_edge_bug.cpp) |
| A-5 | Move data files → `data/` | A-1 | complete | commit 691b4e7e; includes github-428-*.dot; isomorphism.cpp hardcoded paths updated |
| B-1 | Add per-area Jamfiles | Phase A | complete | concepts/Jamfile.v2 and regressions/Jamfile.v2 created |
| B-2 | Split main Jamfile alias | B-1 | complete | commit a7c577b8 |
| B-3 | Replace repetitive graph.cpp / property_iter.cpp blocks | B-2, E-1 | complete | commit 41f0a2af; kept explicit 9 entries per fallback policy, added comment block in Jamfile pointing at source documentation |
| B-4 | Extract benchmarks → `benchmarks/` | A-1 | complete | commit 0c987255; betweenness_centrality_test stays in main suite (arg=100 is vertex count, not iteration count) |
| C-1 | Rename files to `*_test.cpp` | Phase B | complete | commit 715cab00; 43 files renamed across test/, regressions/, concepts/clustering/, benchmarks/; audit passed |
| C-2 | Migrate bare `assert` to Boost.Test | C-1 | complete | commit 7b0dc440; only cycle_ratio_test.cpp among C-1-touched files used bare assert; replaced with BOOST_TEST (lightweight_test header already in use) |
| C-3 | Resolve `*2` suffix variants | C-1, E-2 | complete | commits ae7767ee (merged transitive_closure_test2 -> primary as deterministic fixture), 5dee8170 (vf2_sub_graph_iso_test_2 -> vf2_sub_graph_iso_empty_graph_test.cpp), c87c2e2a (weighted_matching_test2 -> weighted_matching_edge_cases_test.cpp). E-2 coverage notes folded into the rename commits. |
| C-4 | Resolve commented-out tests | Phase B | complete | commit 41c7150c. adj_list_invalidation.cpp: TODO references commit 1dfbbe2a (runtime crash, disabled deliberately). relaxed_heap_test.cpp: source already removed in commit 19c23ca2 long before this reorg; TODO marker retained in the Jamfile. dijkstra_heap_performance.cpp: handled by B-4. |
| D-1 | Create `test/CMakeLists.txt` | Phase C | not started | CMakeLists uses post-C-1 filenames |
| D-2 | Tag tests by cost | D-1 | not started | |
| E-1 | Document `TEST=N` intent | none; must precede B-3 | complete | commit 1c07297c; mappings verified against common/graph_type.hpp |
| E-2 | Add coverage comments to `*2` files | none; must precede C-3 | complete | done in-line with the C-3 rename commits (5dee8170, c87c2e2a); the transitive_closure variant was merged so no surviving file needs a note |
| E-3 | Attach TODO to commented-out tests | C-4 | complete | done in-line with commit 41c7150c |
| E-4 | Verify CMakeLists.txt is coverage-friendly | D-1 | not started | |
| E-5 | Create `.github/coverage/` placeholder | Phase D | not started | |
