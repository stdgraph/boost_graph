

* Versions of boost
  * github: https://github.com/boostorg/boost.git
  * github release: https://github.com/boostorg/boost/releases/tag/boost-1.90.0 (matches github directory structure)
  * boost.org: https://archives.boost.io/release/1.90.0/source/boost_1_90_0.tar.gz

* We are using version cloned from github, following github README for bgl

```
Development

Clone the whole boost project, which includes the individual Boost projects as submodules (see boost+git doc):

git clone https://github.com/boostorg/boost
cd boost
git submodule update --init
The Boost Graph Library is located in libs/graph/.

Boost Graph Library is mostly made of headers but also contains some compiled components. Here are the build commands:

./bootstrap.sh            <- compile b2
./b2 headers              <- just installs headers
./b2                      <- build compiled components
Note: The Boost Graph Library cannot currently be built outside of Boost itself.
```

### Property maps

Issue is that a vertex property can be stored in one of two ways.1990s

Property may be stored in an external container, so that it is looked up as an indexing operation:
```
   std::vector<double> weights;
   weight = weights[v]
```
Whereas a vertex property might be a field in a vertex object
```
    struct vertex {
      double weight;
    }
```
Lookup is with
```
    weight = v.weight
```
Property maps provide an abstraction so that an algorithm that needs properties can use uniform syntax, regardless
of how properties are actually stored.  Meaning
```
    weight = get(v, weights)
```
will return the associated property, whether the property is storect in an external container, or as a member of an object.

Currently, this abstraction requires a complicated use of preprocessor and so forth, we would like to be able to do something equivalent, but without using compliated preprocessor mechanisms.
We would like to be able to write our algorithms with a single syntax for accessing a property, regardless how that property is related to a vertex associated with a graph.
For example if we have
```
    using vertex_type = struct {
        size_t weight
    }
    using graph_type = struct {
        std::vector<vertex_type> vertices;
    }
```
In a generic graph algorithm parameterized by graph type, we could invoke it as
```
    dijkstra(g, s)
```
and dijkstra would use the weight member from the vertex structure when weight properties are needed in the algorithm.
Alternatively, we might define weight properties as
```
    using weights_container = std::vector<size_t>;
    using vertex_type = size_t;
```
and invoke generic dijkstra as
```
    weights_container weights;
    dijkstra(g, s, weights);
```
In this case, the algorithm would access the weight property from the external container.
However, since we want dijkstra to be a generic algorithm, we would like to be able to invoke it regardless of how properties are accessed.
Also, since it is a generic algorithm, we need a single syntax in the algorithm for accessing properties, regardless of how they are stored.



### Named parameters

Many algorithms require a large number of parameters, some of which are used, some not, some defaulted, in different contexts.
For instance Dijkstras algorithm might take the graph, starting vertext, container for distances, for weights, for predecessors, etc,
but in any particular use, we might only us a subset.  In other languages, we could use named (or keyword) parameters such as
```
  dijkstra(g, s, distance=d, weights=w)
```
and the predecessors would not be used.
BGL does this by chaining members of a parameter object
```
  dijkstra(g, s, params.distance(d).weights(w))
```
where unused members are defaulted.  The C++20 structure initialization should be a good alternative.