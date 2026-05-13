# Test Organization — Observations, Recommendations, and Agent Notes

This document captures the current state of the `test/` directory in
`boost/libs/graph`, the problems with how it is organized, and a concrete,
incremental strategy for improving it. It is intended for both human
contributors and agents tasked with refactoring or extending the test suite.

Companion to [overview.md](overview.md). Scope is limited to the test
infrastructure; library code organization is out of scope.

---

## 1. Snapshot of the Current State

As of this writing:

- [test/](../../test/) contains **~113 `.cpp` files** in a single flat
  directory.
- [test/Jamfile.v2](../../test/Jamfile.v2) is **~223 lines**, dominated by a
  single `graph_test_regular` alias listing every test individually.
- 10 shared headers act as informal fixtures:
  [graph_test.hpp](../../test/graph_test.hpp),
  [graph_type.hpp](../../test/graph_type.hpp),
  [typestr.hpp](../../test/typestr.hpp),
  [cycle_test.hpp](../../test/cycle_test.hpp),
  [min_cost_max_flow_utils.hpp](../../test/min_cost_max_flow_utils.hpp),
  [test_construction.hpp](../../test/test_construction.hpp),
  [test_destruction.hpp](../../test/test_destruction.hpp),
  [test_direction.hpp](../../test/test_direction.hpp),
  [test_iteration.hpp](../../test/test_iteration.hpp),
  [test_properties.hpp](../../test/test_properties.hpp),
  [test_graph.hpp](../../test/test_graph.hpp).
- Only one structured subdirectory exists:
  [test/concept_tests/clustering/](../../test/concept_tests/clustering/).
- Data files live inline (`.dot`, `.gr`, `.dat`, `.xml`,
  `planar_input_graphs/`, `prgen_input_graphs/`).
- Two CMake-packaging tests exist (`cmake_install_test/`,
  `cmake_subdir_test/`); the main library [CMakeLists.txt](../../CMakeLists.txt)
  builds the library but **does not register the tests**.

---

## 2. Observations (What's Working)

- A single top-level `test-suite graph_test` alias makes CI integration
  simple.
- Shared fixtures already exist — the bones of a real fixture framework are
  present, just unorganized.
- Both `compile` and `compile-fail` test types are used where appropriate
  (concept checks, SFINAE).
- Optional dependencies (SGB, LEDA) are gated behind `-s` variables at the
  bottom of the Jamfile and do not pollute the default build.
- Data files live next to the tests that use them, which keeps tests
  self-contained even if the layout is messy.

---

## 3. Observations (Problems)

1. **Flat directory.** ~113 `.cpp` files in one folder. No way to build or
   reason about a topical subset (e.g. "just the planar tests") without
   grepping the Jamfile.
2. **Inconsistent naming.** `*_test.cpp`, `*-test.cpp`, and bare names
   (`bfs.cpp`, `dfs.cpp`, `subgraph.cpp`, `copy.cpp`) coexist. The `*_cc.cpp`
   convention for concept checks is internally consistent but isolated.
3. **Concept-check tests are scattered.** `*_cc.cpp` files live among
   algorithm tests, while `concept_tests/clustering/` exists as the only
   subdirectory using a different layout.
4. **Jamfile is a 200-line wall** with no internal grouping or comments.
   Easy to drop a test by accident in a merge.
5. **Dead/commented-out tests** are left in place with no tracking issue:
   - `adj_list_invalidation.cpp` (file exists, not run)
   - `dijkstra_heap_performance.cpp` (perf benchmark, commented out)
   - `relaxed_heap_test.cpp`
6. **Performance benchmarks mixed with correctness tests.** Several entries
   take iteration counts as arguments (`betweenness_centrality_test.cpp : 100`,
   `dijkstra_no_color_map_compare.cpp : 10000`,
   `random_matching_test.cpp : 1000 1020`). These should be in a separate
   `benchmark/` target.
7. **Numbered-variant tests.** `graph.cpp` and `property_iter.cpp` each have
   nine near-identical Jam lines parameterized by `<define>TEST=N` with no
   documentation of what each `N` means.
8. **Confusing `*2` suffix variants.** `transitive_closure_test.cpp` /
   `..._test2.cpp`, `vf2_sub_graph_iso_test.cpp` / `..._test_2.cpp`,
   `weighted_matching_test.cpp` / `..._test2.cpp`. Either merge or rename to
   describe what's different.
9. **General test utilities are not marked as such.** `typestr.hpp` and
   `graph_type.hpp` have no obvious home; easy to overlook.
10. **GitHub-issue regressions are mixed in by issue number**
    (`github-428-0.dot`, `github-428-1.dot`, `finish_edge_bug.cpp`).
11. **CMake parity is missing.** [CMakeLists.txt](../../CMakeLists.txt)
    builds the library but never adds tests. Boost is moving toward
    CMake-buildable test suites; current state guarantees Jam/CMake drift.
12. **Test-data discovery is inconsistent.** Some tests are passed
    `$(TEST_DIR)`; others rely on relative paths that only work when run
    from `test/`.
13. **No grouping by speed/cost.** CI cannot pick a "fast" subset without
    knowing which entries are heavyweight.

---

## 4. Recommended Target Layout

```
test/
  CMakeLists.txt           # mirrors Jamfile, one entry per .cpp
  Jamfile.v2               # delegates to per-area Jamfiles
  common/                  # shared fixture headers + general utilities
  concepts/                # all *_cc.cpp + compile-fail tests
    clustering/            # (existing)
  structures/              # adj_list, adj_matrix, csr, grid, edge_list,
                           #   subgraph, filtered, reverse_graph, labeled,
                           #   directed_graph, compressed_sparse_row
  algorithms/
    traversal/             # bfs, dfs, undirected_dfs
    shortest_paths/        # dijkstra, bellman, johnson, floyd_warshall,
                           #   astar, dag_longest, r_c_shortest_paths
    flow/                  # max_flow, boykov_kolmogorov, cycle_canceling,
                           #   successive_shortest_path, find_flow_cost
    components/            # connected, biconnected, strong, incremental
    matching/              # matching, weighted_matching, random_matching
    planar/                # basic_planarity, make_*_planar,
                           #   is_straight_line_draw, all_planar_*
    layout/                # layout, gursoy_atun, fruchterman, circle, kamada
    centrality/            # betweenness, closeness, degree, eccentricity,
                           #   mean_geodesic
    coloring/              # sequential_vertex_coloring, edge_coloring
    ordering/              # cuthill_mckee, king, min_degree
    cycles/                # tiernan, hawick, bron_kerbosch, cycle_ratio
    isomorphism/           # isomorphism, vf2, mcgregor
    spanning_trees/        # random_spanning_tree, two_graphs_common_*
    misc/                  # transitive_closure, dominator_tree, mas,
                           #   stoer_wagner, core_numbers, louvain, dag,
                           #   bipartite
  io/                      # graphml, graphviz, metis, dimacs, serialize,
                           #   read_propmap
  generators/              # generator_test, erdos_renyi, ...
  regressions/             # finish_edge_bug + github-NNN-*.{cpp,dot}
  data/                    # *.dot, *.gr, *.dat, *.xml,
                           #   planar_input_graphs/, prgen_input_graphs/
  cmake_install_test/      # (existing, unchanged)
  cmake_subdir_test/       # (existing, unchanged)
  benchmarks/              # dijkstra_heap_performance, perf-flavored runs
                           # own Jamfile alias + CTest label; not in graph_test
```

---

## 5. Incremental Strategy

Each step is independently reviewable. Order is chosen so that early steps
are mechanical and low-risk; later steps require design discussion.

### Phase A — Pure mechanical moves (low risk)

1. **Move shared headers** into `test/common/` and update includes.
2. **Move concept checks** (`*_cc.cpp`, `compile-fail` clustering tests,
   `graph_concepts.cpp`) into `test/concepts/`. Update Jamfile paths.
3. **Move regression tests** (`finish_edge_bug.cpp`, `github-NNN-*` files)
   into `test/regressions/`.
4. **Move data files** under `test/data/` and update the `path-constant`
   declarations at the top of [test/Jamfile.v2](../../test/Jamfile.v2).
   Single point of change; no test source edits required.

### Phase B — Jamfile restructuring

5. **Split Jamfile** so `graph_test_regular` becomes:
   ```jam
   alias graph_test_regular :
       [ alias-of structures//tests ]
       [ alias-of algorithms//tests ]
       [ alias-of io//tests ]
       [ alias-of concepts//tests ]
       [ alias-of regressions//tests ]
       ;
   ```
   Each subdir gets its own `Jamfile.v2`.
6. **Replace the 9× `graph.cpp` and 9× `property_iter.cpp` blocks** with a
   Jam loop, and document what each `TEST=N` value exercises in the source.
7. **Move performance tests** into `test/benchmarks/` with its own Jamfile
   alias and CTest label, not pulled into `graph_test`.

### Phase C — Naming and hygiene

8. **Standardize on `*_test.cpp`.** One-shot `git mv` PR. `*_cc.cpp` is
   retained for compile-only concept checks. No redirector stubs needed.
9. **Migrate bare `assert` / `BOOST_CHECK` to Boost.Test** in any file
   touched during this phase. Track remaining migrations as follow-up.
10. **Resolve `*2` variants.** Either merge or rename
    (`..._small.cpp`/`..._random.cpp`/`..._regression.cpp` etc.).
11. **Either remove or re-enable** the commented-out tests in the Jamfile;
    if kept, attach a `# TODO(#issue):` line.

### Phase D — CMake parity and CI tiers

11. **Add `test/CMakeLists.txt`** that lists the same tests via
    `add_executable` + `add_test`, mirroring the Jam structure.
12. **Tag tests by cost** (e.g. via Jam features `<test-info>` and CTest
    `LABELS`) so CI can pick a fast subset.

### Phase E — Coverage preparation (done during this effort, before coverage is run)

Coverage analysis will be done as a **separate, later effort**. However,
several decisions during Phases A–D affect how clean and actionable that
report will be. Make these choices now:

13. **Record the intent of each `TEST=N` variant** (step 6 above). Coverage
    of `graph.cpp` and `property_iter.cpp` is uninterpretable until you know
    which code paths each `N` is meant to exercise. Add a comment block in
    each source listing the meaning of each value.
14. **Document what the `*2` variants cover** before merging or renaming
    them (step 9). Add a brief comment at the top of each file: what is
    unique about this variant vs. the primary test? This makes the coverage
    merge/delete decision trivial when the time comes.
15. **Do not delete commented-out tests** (`adj_list_invalidation.cpp`,
    `relaxed_heap_test.cpp`) during Phase C. Attach a `# TODO(#issue):`
    comment and leave them for the coverage phase to evaluate — they may
    cover paths that no active test reaches.
16. **Ensure `test/CMakeLists.txt`** (step 11) uses a structure amenable to
    a coverage preset: keep each test as a separate `add_executable` target
    (not one combined binary) so per-test coverage deltas are visible.
    Avoid `file(GLOB)` for test discovery — explicit lists ensure coverage
    runs stay in sync with what CI actually builds.
17. **Create a `coverage/` subdirectory** under `.github/` (or reuse
    `.github/workflows/`) as a placeholder for the future instrumentation
    preset and reporting scripts. Leave it empty for now. This reserves the
    convention and avoids ad-hoc placement later.

---

## 6. Things to Leave Alone

- The shared fixture headers themselves — they're fine; only their location
  should change.
- [test/cmake_install_test/](../../test/cmake_install_test/) and
  [test/cmake_subdir_test/](../../test/cmake_subdir_test/) — these test
  packaging, not graph behavior, and belong where they are.
- The SGB/LEDA optional blocks at the bottom of
  [test/Jamfile.v2](../../test/Jamfile.v2) — already well-isolated.
- `min_cost_max_flow_utils.hpp` — domain-specific helper that lives next to
  its only consumers; OK as-is even after the broader refactor (move only
  if its consumers move together).

---

## 7. Phase Sequencing Summary

| Phase | What | Depends on |
|---|---|---|
| A | Mechanical moves (headers, concepts, regressions, data) | Nothing |
| B | Jamfile split into per-area subfiles | Phase A |
| C | Naming cleanup, merge `*2` variants, resolve dead tests | Phase E prep work |
| D | `test/CMakeLists.txt` + `add_test` + cost tagging | Phase B |
| E prep | Document `TEST=N` intent, `*2` comments, coverage layout | Phase D |
| Coverage | Instrumentation, CI pipeline, threshold policy | Phase E prep |

---

## 8. Notes for Agents

When asked to work on test organization or to add a new test, an agent
should:

1. **Read this document first**, then [test/Jamfile.v2](../../test/Jamfile.v2)
   to confirm the current state. The repo layout may have advanced past one
   or more phases above.
2. **Prefer editing existing fixture headers** in `common/` (or `test/`
   pre-refactor) over inventing new helpers for one-off tests.
3. **Always use `*_test.cpp` for new runtime tests** and `*_cc.cpp` for
   compile-only concept checks. Do not use bare names or hyphenated names.
4. **Always use Boost.Test** for new tests. Do not use bare `assert` or
   Catch2. When modifying an existing file that uses bare `assert`, migrate
   it to Boost.Test in the same commit.
5. **Do not add a new test by editing only the source file** — every test
   must be wired into the appropriate Jamfile (and CMakeLists.txt once
   Phase D lands). The Jam pattern is `[ run name.cpp ]` for runtime tests,
   `[ compile name.cpp ]` for compile-only, and `[ compile-fail name.cpp ]`
   for SFINAE/concept negative checks.
6. **Do not commit performance benchmarks into the regular suite.** Route
   them to `test/benchmarks/` with its own Jamfile alias and CTest label.
7. **Do not delete a commented-out test** without first checking
   `git log -- <path>` for the rationale; some are kept intentionally for
   future reactivation.
8. **When moving files, update both Jam and CMake** (once parity exists)
   in the same commit. Path-constant declarations at the top of the Jamfile
   are the cheapest place to absorb data-file relocations.
9. **Regression tests for GitHub issues** belong in `regressions/` and
   should reference the issue number both in the filename and in a comment
   at the top of the source.
10. **Concept checks are compile-only** by convention here; do not promote
    them to `run` tests.
11. **When in doubt about category**, prefer `algorithms/misc/` over
    inventing a new subdirectory — categories should be earned by having
    at least three related tests.

---

## 9. Open Questions

### Settled decisions

- **Naming:** `*_test.cpp` for all test sources, `*_cc.cpp` retained for
  compile-only concept checks. Phase C renames apply universally.
- **Benchmarks:** live under `test/benchmarks/` so CI can discover and
  optionally include them via a CTest label or Jam feature without a
  separate top-level directory.
- **Test framework:** Boost.Test throughout. New tests must use it; existing
  tests using bare `assert` should be migrated to Boost.Test incrementally
  during Phase C or when a file is otherwise touched. Do not introduce
  Catch2 or lightweight test.
- **CMake test discovery:** explicit `add_executable` / `add_test` per file,
  mirroring the Jam style. No `file(GLOB)`. This keeps Jam and CMake lists
  in sync and avoids surprise inclusions.

### Deferred to coverage phase

- Which coverage tool: `gcov`/`lcov`/`genhtml` (GCC), `llvm-cov`/`llvm-profdata`
  (Clang), or a higher-level wrapper (e.g. `fastcov`, Codecov CLI)?
- CI integration: per-PR diff view (Codecov, Coveralls, self-hosted badge) or
  periodic scheduled report only?
- Coverage threshold policy: hard fail, soft warning, or informational only?
- What is exempt: LEDA/SGB optional adaptors, deprecated APIs, generated code?
- Should commented-out tests (`adj_list_invalidation.cpp`,
  `relaxed_heap_test.cpp`) be re-enabled once coverage reveals gaps they fill,
  or deleted if coverage shows they are fully redundant?
- Do the nine `TEST=N` variants of `graph.cpp` / `property_iter.cpp` each
  add distinct coverage? (Answer this with coverage data, not guessing.)
