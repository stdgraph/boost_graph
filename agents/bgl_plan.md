# Plan to modernize the boost graph library (BGL)


We want to modernize BGL to be an idiomatic C++20 library. Examples include
- ranges
- concepts
- range-based for support
- replace the use of boost libraries with standard C++ library equivilents 

Do a thorough review of the existing library in the following directories
- Headers are in `boost/graph`
- Examples, documents, tests are in `libs/graph`

Also review existing design elements, such as the Property Maps, and make 
recommendations for alternate designs.
