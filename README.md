# Boost Graph Library

===================

A generic interface for traversing graphs, using C++ templates.

The full documentation is available on [boost.org](http://www.boost.org/doc/libs/release/libs/graph/doc/index.html).

## Support, bugs and feature requests

Bugs and feature requests can be reported through the [Github issue page](https://github.com/boostorg/graph/issues).

See also:

* [Current open issues](https://github.com/boostorg/graph/issues)
* [Closed issues](https://github.com/boostorg/graph/issues?utf8=%E2%9C%93&q=is%3Aissue+is%3Aclosed)
* Old issues still open on [Trac](https://svn.boost.org/trac/boost/query?status=!closed&component=graph&desc=1&order=id)
* Closed issues on [Trac](https://svn.boost.org/trac/boost/query?status=closed&component=graph&col=id&col=summary&col=status&col=owner&col=type&col=milestone&col=version&desc=1&order=id)

You can submit your changes through a [pull request](https://github.com/boostorg/graph/pulls). One of the maintainers will take a look (remember that it can take some time).

There is no mailing-list specific to Boost Graph, although you can use the general-purpose Boost [mailing-list](http://lists.boost.org/mailman/listinfo.cgi/boost-users) using the tag [graph].


### Build Status

|                  |  Master  |   Develop   |
|------------------|----------|-------------|
| Github Actions | [![Build Status](https://github.com/boostorg/graph/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/boostorg/graph/actions) | [![Build Status](https://github.com/boostorg/graph/actions/workflows/ci.yml/badge.svg?branch=develop)](https://github.com/boostorg/graph/actions) |
|Drone | [![Build Status](https://drone.cpp.al/api/badges/boostorg/graph/status.svg?ref=refs/heads/master)](https://drone.cpp.al/boostorg/graph) | [![Build Status](https://drone.cpp.al/api/badges/boostorg/graph/status.svg)](https:/drone.cpp.al/boostorg/graph) |

## Development

Clone the whole boost project, which includes the individual Boost projects as submodules ([see boost+git doc](https://github.com/boostorg/boost/wiki/Getting-Started)):

    git clone https://github.com/boostorg/boost
    cd boost
    git submodule update --init

The Boost Graph Library is located in `libs/graph/`.

Boost Graph Library is mostly made of headers but also contains some compiled components. Here are the build commands:

    ./bootstrap.sh            <- compile b2
    ./b2 headers              <- just installs headers
    ./b2                      <- build compiled components

**Note:** The Boost Graph Library cannot currently be built outside of Boost itself.

### Running tests
First, make sure you are in `libs/graph/test`.
You can either run all the 300+ tests listed in `Jamfile.v2` or run a single test:

    ../../../b2                        <- run all tests
    ../../../b2 cycle_canceling_test   <- single test

You can also check the [regression tests reports](http://beta.boost.org/development/tests/develop/developer/graph.html).

## C++20 Modernization (bgl2)

A modernized C++20 version of the Boost Graph Library is being developed in the [stdgraph/boost_graph](https://github.com/stdgraph/boost_graph) fork. This refactoring effort aims to bring the library up to modern C++ standards.

### Using the bgl2 Branch

To work with the modernized version, add the fork as a remote and switch to the `bgl2` branch:

    cd libs/graph
    git remote add stdfork https://github.com/stdgraph/boost_graph.git
    git fetch stdfork
    git checkout bgl2
    git branch --set-upstream-to=stdfork/bgl2 bgl2

This associates your local `bgl2` branch with the fork's `bgl2` branch, so `git pull` and `git push` will use the fork by default.

The agents directory is located at `libs/graph/agents/` (relative to the boost root) so everything is contained in a single repository.

### Using an AI Agent
After the repos have been set up you'll need to prime the agent with the context your working with.
Enter the following in your agent's chat command line to prime the agent with the context it needs to continue the refactoring.
```
This project is focused on refactoring the graph library in libs/graph
All agent files are in libs/graph/agents/
Review libs/graph/agents/bgl_todo.md instructions to prepare for future work
All refactoring output must be written under the libs/graph/modern/ directory
```

