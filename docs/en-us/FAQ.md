## Frequently Asked Questions

1. What is the data model of gStore?
   <br>Answer: The data model of gStore is a native graph model that maintains the graph structure of the original RDF knowledge graph.</br>
   
2. What query language does gStore use?
   <br>Answer: gStore uses the SPARQL 1.1 standard query language and converts SPARQL queries for RDF into subgraph matching queries for RDF graphs.</br>

3. What is the storage capacity of the stand-alone version of gStore?
   <br>A: A single machine supports 5 billion RDF triples.</br>

4. What operating systems does gStroe support?
   <br>A: gStore currently supports Linux-based kernel operating systems, such as CentOS and Ubuntu.</br>

5. What are the requirements for the installation environment of gStore?
   <br>A: For the environment requirements of gStore, please refer to Developer Resources > Installation Instructions > System Requirements.</br>

6. What are the installation methods of gStore?
   <br>Answer: The latest version 1.0 of gStore currently supports three installation methods:</br>

   (1) Static compiled version installation: download and unzip it to use (click [here](http://file.gstore.cn/f/9a89ea8572e64262b1af/?dl=1) to download)

   (2) Source code compilation and installation: For details, please refer to Developer Resources > Installation Instructions > gStore Compilation (the required environment needs to be installed first)

   (3) Docker installation: For details, please refer to Developer Resources > Installation Instructions > Deploy gStore using Docker

7. Can gStore be operated through interfaces?
   <br>A: gStore currently provides three interface modes: HTTP, RPC, and Socket, corresponding respectively to the three services: bin/ghttp, bin/grpc, and bin/gserver. For specific usage, please refer to Developer Resources > Quick Start > HTTP API Service and Socket API Service.</br>

8. How to start up the interface service of gStore?
   <br>Answer: Starting up ghttp: bin/ghtp - p [port]</br>

   Starting up grpc: bin/grpc - p [port]

   Starting up server: bin/gserver - s

   For specific usage, please refer to Developer Resources > Quick Start > HTTP API Service and Socket API Service.

9. Which path queries does gStore support?
   <br>Answer: Path queries currently supported by gStore include: shortest path query, cycle query, K-hop reachability query, and breadth-first search count.</br>

10. What data file formats does gStore support for building databases?
    <br>Answer: The file formats supported by gStore are nt, n3, ttl and rdf.</br>