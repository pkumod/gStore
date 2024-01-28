**The first essay to come up with Gstore System is [gStore_VLDBJ](pdf/gStoreVLDBJ.pdf), and You can find related essays and publications in [Related Essays](ESSAY.md).**

## What Is gStore

gStore is a graph-based RDF data management system(or what is commonly called a "triple store") that maintains the graph structure of the original [RDF](http://www.w3.org/TR/rdf11-concepts/) data. Its data model is a labeled, directed multi edge graph, where each vertex corresponds to a subject or an object. 

We represent a given [SPARQL](http://www.w3.org/TR/sparql11-overview/) query by a query graph Q. Query processing involves finding subgraph matches of Q over the RDF graph G, instead of joining tables in relational data management system. gStore incorporates an index over the RDF graph (called VS-tree) to speed up query processing. VS-tree is a height balanced tree with a number of associated pruning techniques to speed up subgraph matching.

(NOTICE: Homomorphism is used here, instead of isomorphism)

**The gStore project is supported by the National Science Foundation of China (NSFC), Natural Sciences and Engineering Research Council (NSERC) of Canada, and Hong Kong RGC.**

- - -

## What Is New In gStore

There are three important features in gStore:

- gStore manages RDF repository from a graph database perspective.

- gStore supports both query and update efficiently.

- gStore can handle, in a uniform manner, different data types (strings and numerical data) and SPARQL queries with wild cards, aggregate, range operators(only theoretically, not achieved so far)

- - -

## Why gStore

After a series of test, we analyse and keep the result in [Test Results](TEST.md). gStore runs faster to answer complicated queries(for example, contain circles) than other database systems. For simple queries, both gStore and other database systems work well. 
	
In addition, now is the big data era and more and more structured data is coming, while the original relational database systems(or database systems based on relational tables) cannot deal with them efficiently. In contrast, gStore can utilize the features of data structures, and improve the performance. 
	
What is more, gStore is a high-extensible project. Many new ideas of graph database have be proposed, and most of them can be used in gStore. For example, some members of our group are designing a distributed gstore system.

- - -

## Open Source

The gStore source code is available as open-source code under the BSD license. You are welcome to use gStore, report bugs or suggestions, or join us to make gStore better. It is also ok for you to build all kinds of applications based on gStore, while respecting our work.

