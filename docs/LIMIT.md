1. gserver not robust enough, very easy to break out(need to deal with all kinds of exceptions).

2. queries related with uncertain predicates are not supported.

3. only `select` queries can be used now. All aggregate queries are not supported, as well as insert/modify/remove operations.

4. only support RDF datasets in N-Triples format

5. the cost of disk and memory is still very large

6. the VStree index consumes too much time and memory, and the filter effects are not good(less than 25%). 

