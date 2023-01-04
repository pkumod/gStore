<!--
 * @Author: wangjian 2606583267@qq.com
 * @Date: 2023-01-04 10:12:47
 * @LastEditors: wangjian 2606583267@qq.com
 * @LastEditTime: 2023-01-04 10:12:48
 * @FilePath: /gstore/docs/INTRO.md
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->
>gStore Version:1.0
>
>Last Modification Data:2022-12-31
>
>Modified by:JiuhuaQi
>
>Modification description:

**The first essay to come up with Gstore System is [gStore_VLDBJ](pdf/gStoreVLDBJ.pdf), and You can find related essays and publications in [Related Essays](ESSAY.md).**

## What Is gStore

gStore is a graph-based RDF data management system(or what is commonly called a "triple store") that maintains the graph structure of the original [RDF](http://www.w3.org/TR/rdf11-concepts/) data. Its data model is a labeled, directed multi edge graph, where each vertex corresponds to a subject or an object. 

We represent a given [SPARQL](http://www.w3.org/TR/sparql11-overview/) query by a query graph Q. Query processing involves finding subgraph matches of Q over the RDF graph G, instead of joining tables in relational data management system. gStore incorporates an index over the RDF graph (called VS-tree) to speed up query processing. VS-tree is a height balanced tree with a number of associated pruning techniques to speed up subgraph matching.

(NOTICE: Homomorphism is used here, instead of isomorphism)

**The gStore project is supported by the National Science Foundation of China (NSFC), Natural Sciences and Engineering Research Council (NSERC) of Canada, and Hong Kong RGC.**

- - -

## What Is New In gStore

There are four important features in gStore:

- **RDF Graph Storage **

  gStore employs a native graph model to store and maintain very large RDF datasets. A single gStore instance on a single machine can store up to 5 billion RDF triples. gStore supports scale-out deployment with massive scaling capabilities to more than hundreds of billions of triples.

-  **Efficient Query Engine **

  gStore supports SPARQL 1.1, the standard RDF query language. gStore employs the subgraph matching-based query strategy as well as a series of query optimization techniques and structure-aware index to build an efficient graph-native SPARQL query engine..

- **Full RDF Graph Management Functionality **

  gStore supports full set of database operations, including insertion, deletion, updates, backup and restore. Transactional support is provided with three isolation levels to adapt to different requirements. gStore provides a series of APIs in multiple programming languages, including C++, Java, Python, NodeJS and PHP. 

-  **RDF Graph Analysis **

  gStore has various built-in graph analysis algorithms including path enumeration, cycle detection, PageRank, triangle counting and k-hop reachability. Also, gStore provides programable interfaces for users to implement their own graph analysis algorithms, which can be integrated seamlessly with SPARQL in gStore system. 

- - -

## Why gStore

With the advent of the era of big data, enterprises and institutions will have a large number of data, through the correlation between the data to mine the value of the data becomes more and more important, and the original relational database system (or database systems based on relational tables) can not effectively deal with the association relationship. In contrast, gStore can take advantage of the data association feature, and gStore has over a thousand times performance improvement over traditional relational databases in multi-hop association processing. 

In addition, after a series of test, we analyse and keep the result in [Test Results](TEST.md). gStore runs faster to answer complicated queries(for example, contain circles) than other database systems. For simple queries, both gStore and other database systems work well. 
		
What is more, gStore supports scale-out deployment with massive scaling capabilities to more than hundreds of billions of triples. Moreover, it has been applied in many scenarios such as finance, public security and the Internet, and has achieved good benefits.

- - -

## Open Source

The gStore source code is available as open-source code under the BSD license. You are welcome to use gStore, report bugs or suggestions, or join us to make gStore better. It is also ok for you to build all kinds of applications based on gStore, while respecting our work.

