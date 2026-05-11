# Boost Graph Library — Overview

The Boost Graph Library (BGL) provides a generic interface for graph data
structures and algorithms using C++ templates. Following the same
generic-programming philosophy as the C++ Standard Library, BGL decouples
algorithms from data structures through a concept-based interface: any graph
type that models the required concepts works with any algorithm, including
user-defined graph types. The library ships ~150 public headers and two
compiled components (GraphML and DOT readers).

**C++ standard:** C++14  
**Build systems:** Boost.Build (`build.jam`) and CMake (`CMakeLists.txt`)  
**Build note:** Full compilation and testing require a Boost superproject
checkout; see [overview/build-and-test.md](overview/build-and-test.md) for
both the repo-local and superproject workflows.

---

## Contents

| Document | Covers |
|----------|--------|
| [overview/purpose.md](overview/purpose.md) | What BGL is, why it matters, primary audience |
| [overview/organization.md](overview/organization.md) | Repository layout, build files, compiled components |
| [overview/architecture.md](overview/architecture.md) | Generic programming model, `graph_traits`, concepts, property maps, named parameters, visitors |
| [overview/concepts.md](overview/concepts.md) | Graph concept hierarchy and refinement relationships |
| [overview/graph-types.md](overview/graph-types.md) | Survey of built-in graph data structures |
| [overview/algorithms.md](overview/algorithms.md) | Survey of algorithm families with header and doc links |
| [overview/build-and-test.md](overview/build-and-test.md) | Building, testing, and running examples |
| [overview/extending.md](overview/extending.md) | Adding new algorithms, graph types, property maps |
| [overview/glossary.md](overview/glossary.md) | Definitions of core BGL terms |

---

## Quick-reference: finding what you need

| Goal | Start here |
|------|-----------|
| Understand the library's design philosophy | [overview/architecture.md](overview/architecture.md) |
| Choose a graph data structure | [overview/graph-types.md](overview/graph-types.md) |
| Find an algorithm by family | [overview/algorithms.md](overview/algorithms.md) |
| Understand what a term means | [overview/glossary.md](overview/glossary.md) |
| Build or run tests | [overview/build-and-test.md](overview/build-and-test.md) |
| Add a new algorithm or graph type | [overview/extending.md](overview/extending.md) |
| Navigate the repository layout | [overview/organization.md](overview/organization.md) |

---

## Authoritative sources in this repo

| Source | Authority |
|--------|-----------|
| [`include/boost/graph/`](../../include/boost/graph/) | Factual API truth — headers win over docs on any discrepancy |
| [`doc/`](../../doc/) | Long-form concept and algorithm documentation |
| [`README.md`](../../README.md) | Narrative description, build commands, CI links |
| [`meta/libraries.json`](../../meta/libraries.json) | Authors, category, C++ standard |

---

## How to update this overview

1. Read [overview_strategy.md](overview_strategy.md) for the full production
   process and the list of revision triggers.
2. Run Phases B–D from that strategy (Phase A only if the top-level layout
   changed).
3. Update this stamp when done.

**Last reviewed:** branch `test_reorg`, commit `05e28619`, 2026-05-11
