gStore
======

## gStore2.0

### Overview
gStore is a graph-based RDF data management system (or what is commonly called a “triple store”) that maintains the graph structure of the original RDF data. Its data model is a labeled, directed multiedge graph, where each vertex corresponds to a subject or an object. We also represent a given SPARQL query by a query graph Q. Query processing involves finding subgraph matches of Q over the RDF graph G. gStore incorporates an index over the RDF graph (called VS-tree) to speed up query processing. VS-tree is a heightbalanced tree with a number of associated pruning techniques to speed up subgraph matching.

### Install Steps
System Requirement: 64bit linux with g++, gcc, make installed.

Install gStore2.0, just with two commands. 

1) run
> **make lib_antlr**

to compile and link the library(static) we need.

2) run
> **make**

to compile the gStore code and build executable "gload", "gquery", "gserver", "gclient".
