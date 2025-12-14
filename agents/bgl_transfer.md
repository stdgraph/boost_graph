# Preserve boost graph changes for development

The changes I've made since yesterday were experimental and to demonstrate how an agent can be used to refactor the boost graph code.

The current project directory is a container for many submodules, including graph. Submodules appear in the `libs/` directory.
For instance, the graph submodule is in `libs/graph`.

The following directories have been added in the refactoring:
`agents/` which hold agent instructions
`libs/graph/modern` which holds the refactored graph code.

Changes have been committed to the `bgl2` branch in both repos.

I want to take the changes and make it available in separate repos(s) to continue the work.

Questions to answer:
- I assume it would be best to fork the original repos into new repos in github, and then push the changes there?
- Can I just fork the graph repo at https://github.com/boostorg/graph, without also forking the main repo at https://github.com/boostorg?
    - That would imply I should move the agents/ directory to graph

