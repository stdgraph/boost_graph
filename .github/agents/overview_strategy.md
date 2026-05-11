# Strategy for Creating the Project Overview

This document describes the strategy an agent (or human) should follow to
produce the project overview artifacts requested in
[overview_goal.md](overview_goal.md). It is the *process* counterpart to that
goal: it does not contain the overview itself, only the recipe for building
and maintaining one.

## 1. Audience and Success Criteria

Two audiences must be served simultaneously:

- **Agent audience** — needs a reusable, machine-friendly context block that
  enables informed decisions about where to read, edit, build, or test.
  Optimized for: stable structure, predictable headings, explicit file paths,
  links to authoritative sources in the repo, and minimal prose.
- **Human audience** — needs a concise, jargon-light narrative that conveys
  what the project is, why it matters, and how it is organized. Optimized
  for: a strong opening summary, a layered table of contents, and links into
  deeper material.

The overview is successful when:

1. A new contributor can answer "what is this and how is it organized?" in
   under five minutes of reading.
2. An agent given only the overview can locate the right subdirectory or
   header file for a typical task without additional searching.
3. Both audiences can drill from the top-level summary into precise detail
   without reading unrelated material.

## 2. Document Set and Layout

Because the Boost.Graph library is large, a **single document is
insufficient**. Produce a top-level overview plus topical subdocuments.

Recommended layout under `.github/agents/`:

- `overview.md` — top-level entry point. Short. Links to everything else.
- `overview/` — directory of focused subdocuments. Suggested files:
  - `purpose.md` — what the library is and the problems it solves.
  - `architecture.md` — concept-based design, generic programming model,
    role of traits, visitors, property maps, named parameters.
  - `organization.md` — repository layout (`include/`, `src/`, `example/`,
    `test/`, `doc/`, `build/`, `meta/`, `CMakeLists.txt`, `build.jam`).
  - `graph-types.md` — survey of the main graph data structures
    (`adjacency_list`, `adjacency_matrix`, `compressed_sparse_row`,
    `grid_graph`, `directed_graph`, `undirected_graph`, etc.).
  - `algorithms.md` — survey of algorithm families (search, shortest paths,
    flow, components, layout, planarity, coloring, MST, etc.) with links
    into `include/boost/graph/` headers and `doc/` pages.
  - `concepts.md` — the graph concept hierarchy and refinement relationships.
  - `build-and-test.md` — how to build and run tests with CMake and with
    Boost.Build (`b2`), where examples live, and how docs are produced.
  - `extending.md` — how to add a new algorithm, graph type, or visitor in a
    way that fits existing conventions.
  - `glossary.md` — short definitions of recurring terms (descriptor,
    property map, visitor, traversal category, etc.).

Only create a subdocument when its topic is large enough that inlining would
bloat the top-level file. Prefer fewer, denser documents over many tiny ones.

**Out of scope — Parallel BGL.** Parallel BGL and related MPI-oriented
surfaces should be documented separately from this core overview set.

- Do not mix MPI-specific material into the main graph-types and algorithms
  surveys.
- If parallel or distributed graph support is present in a given checkout,
  cover it in a separate document set with its own scope and dependencies.

## 3. Information Sources (in priority order)

Mine these sources before writing any prose. Cite them by relative path so
the overview stays verifiable.

1. **`README.md`** and **`index.html`** — official top-level descriptions;
   authoritative for *narrative* (purpose, audience, positioning).
2. **`meta/libraries.json`** — Boost library metadata: authors, category,
   and the list of Boost libraries this one depends on. Use the dependency
   list when describing build prerequisites.
3. **`include/boost/graph/`** — the public API; authoritative for *factual*
   claims about what types, algorithms, and concepts exist. When headers
   and `doc/` disagree, headers win.
4. **`doc/`** — long-form HTML documentation; rich source of concept and
   algorithm descriptions. Summarize and link; do not copy. Note that some
   pages predate recent header changes and may be stale; flag any
   discrepancy you discover rather than propagating it.
5. **`example/`** — idiomatic usage patterns worth highlighting.
6. **`test/`** — confirms which components are actively maintained.
7. **`CMakeLists.txt`**, **`build.jam`**, **`build/Jamfile.v2`** — build
   system entry points; describe both because the project supports both.
8. **`src/`** — the (small) set of non-header-only components.

If a claim cannot be traced to one of these sources, mark it as inferred or
omit it.

## 4. Production Workflow

Follow these phases in order. Each phase has an explicit exit criterion.

### Phase A — Reconnaissance
- Record the **target Boost release or branch** the overview is being
  written against (e.g., the current checked-out branch). Put this in the
  header of `overview.md` so future readers can judge staleness.
- List the top-level directories and note their sizes and roles.
- Read [README.md](../../README.md), [index.html](../../index.html), and
  [meta/libraries.json](../../meta/libraries.json).
- Skim [doc/table_of_contents.html](../../doc/table_of_contents.html) to
  learn the documentation's own taxonomy. Reuse that taxonomy where
  possible so the overview aligns with existing user expectations.
- **Exit when:** you can name every top-level directory's purpose in one
  sentence.

### Phase B — Inventory
- Enumerate the headers in `include/boost/graph/` and group them by theme
  (graph types, algorithms, IO, properties, utility, planar, parallel,
  distributed, etc.).
- Enumerate algorithm families and note one representative header per
  family.
- Note generated/auxiliary files that should *not* be presented as core
  API (e.g., detail/, internal headers).
- **Exit when:** you have a grouped list covering every public header.

### Phase C — Drafting
- Write `overview.md` first as a thin index: one paragraph per major topic
  with a link to the corresponding subdocument (even if the subdocument is
  still a stub).
- Fill in subdocuments in priority order: `organization.md`,
  `architecture.md`, `graph-types.md`, `algorithms.md`, then the rest.
- Keep paragraphs short. Prefer bullet lists and tables over long prose.
- For each algorithm or graph type mentioned, link to (a) its header under
  `include/boost/graph/` and (b) its doc page under `doc/` when one exists.
- **Exit when:** every link resolves and every section has at least a
  one-paragraph summary.

### Phase D — Verification
- Spot-check that named headers and doc pages actually exist.
- Re-read each subdocument from the perspective of each audience: would an
  agent get lost? would a newcomer get bored?
- Confirm there is no duplication across subdocuments; if two cover the
  same ground, consolidate.
- **Exit when:** all links resolve, no section duplicates another, and the
  top-level document fits comfortably on one screen.

### Phase E — Maintenance Hooks
- Add a short "How to update this overview" note at the end of
  `overview.md` pointing back to this strategy file.
- Record the last-reviewed date or commit so future maintainers can
  estimate staleness.

## 5. Style Conventions

- **Format:** Markdown only. No HTML except where unavoidable.
- **Headings:** Use ATX (`#`) headings. Top-level document starts at `#`;
  subdocuments also start at `#` so they are self-contained.
- **Links:** Use document-relative Markdown links consistently within the
  overview set. Never invent URLs.
- **Code identifiers:** Wrap type, function, and macro names in backticks.
- **File references in prose:** Use Markdown links to the file rather than
  bare backticked names, so both audiences can navigate.
- **Tone:** Neutral, descriptive, present tense. Avoid marketing language.
- **Length budget:**
  - `overview.md`: ~1 screen (roughly 60–120 lines).
  - Each subdocument: ~1–3 screens. Split further if exceeded.

## 6. Content Checklist

Each overview document should be checked against this list before being
considered complete.

- [ ] States its scope in the first paragraph.
- [ ] Identifies its intended audience(s) implicitly through its content.
- [ ] (Subdocuments only) Links back to `overview.md`.
- [ ] Links forward to the authoritative source(s) it summarizes.
- [ ] Contains no information that cannot be traced to a source listed in
      §3, or marks such information as inferred.
- [ ] Avoids restating Boost-wide conventions that are documented
      elsewhere, except by reference.
- [ ] Has been re-read once specifically for an agent reader and once for a
      human newcomer.

## 7. Anti-Goals

To keep the overview useful, explicitly **avoid** the following:

- Reproducing API reference material that already lives under `doc/` or in
  header comments. Summarize and link instead.
- Tutorials or "getting started" walkthroughs — those belong in `example/`
  and `doc/`.
- Opinions about future direction, deprecations, or roadmap unless sourced
  from a maintained file in the repo.
- Performance claims without a citation.
- Deep dives into a single algorithm; those belong in that algorithm's own
  documentation.
- Verbatim mirrors of directory listings (e.g., a flat dump of
  `include/boost/graph/`). They go stale on the next commit. Group and
  summarize instead.

## 8. When to Revise

Trigger a review of the overview when any of the following occurs:

- A new top-level directory appears or an existing one is removed.
- A new graph type or algorithm family is added under
  `include/boost/graph/`.
- The build system entry points (`CMakeLists.txt`, `build.jam`) change in
  a structural way.
- The Boost release that ships this library changes its documented
  organization for the library.

Revision should re-run Phases B–D; Phase A only needs repeating if the
top-level layout changed.
