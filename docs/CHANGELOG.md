<!--
 * @Author: wangjian 2606583267@qq.com
 * @Date: 2023-01-04 10:10:12
 * @LastEditors: wangjian 2606583267@qq.com
 * @LastEditTime: 2023-01-04 10:10:12
 * @FilePath: /gstore/docs/CHANGELOG.md
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->
> gStore Version:1.0
>
> Last Modification Data:2022-12-31
>
> Modified by:JiuhuaQi
>
> Modification description:

### **gStore 1.0 Version**

- Update Time：2022-10-01
- Updated functions:
  - Support of user-defined graph analysis functions: users can manage their own graph analysis functions through the API interfaces or the visual management platform gStore-workbench. Users can obtain the number of nodes and edges of the graph and neighbors of any given node, etc. through interface functions and use them as basic units to implement their own graph analysis functions. Dynamic compilation and execution of user-defined graph analysis functions are supported.
  - The gRPC network interface service: gRPC is a high-performance network interface service based on HTTP protocol implemented based on the open source library `workflow`, which further improves the efficiency and stability of the interface service. Experiments show that gRPC achieves a great improvement in concurrent access performance compared with ghttp, the previous network interface; for example, in the case of **2000/QPS**, the rate of denied access is **0%**.
  - gConsole module: in gStore 1.0, we launched the gConsole module, which enables the long-session operation of gStore with contextual information.
  - Decoupling of the optimizer and executor: gStore 1.0 decouples the optimizer and executor, converting from the original deeply coupled greedy strategy to a query optimizer based on dynamic programming and a query executor based on breadth-first traversal.
  - Optimization of Top-K queries: We implemented a Top-K SPARQL processing framework based on the DP-B algorithm in gStore, including query segmentation and sub-result aggregation.
  - Support of ACID transactions: by introducing the multi-version management mechanism, gStore 1.0 can start ACID transactions for insert and delete operations, which users can open, commit, and roll back. Currently gStore 1.0 supports four isolation levels: read-uncommitted, read-committed, repeatable read and serializable.
  - Reconstruction of database kernel and optimization of the plan tree generation logic: in gStore 1.0, two types of join operations (worst-case-optimal joins and binary joins) are introduced to optimize query execution and further improve query efficiency.
  - Optimized logging module: based on the log4cplus library, the system logs can be output in a unified format. Users can configure the log output mode (console output or file output), output format, and output level.
  - New built-in advanced functions: gStore 1.0 supports four new advanced functions, namely triangleCounting, closenessCentrality, bfsCount and kHopEnumeratePath.
  - Extended support for BIND statements: gStore 1.0 supports assigning values to variables using algebraic or logical expressions in BIND statements.
  - Optimization of some local commands and API interfaces (e.g., the shutdown command), and fixing a series of bugs (e.g., more accurate gmonitor statistics).

### **gStore 0.9.1 Version**

- Update Time：2021-11-23
- Updated functions:
  - By separating gStore kernel parsing and execution, the query performance can be further improved through join order and other technologies, and the performance can be improved by more than 40% in complex queries;
  - Rewrite the http service component ghttp of gStore, and add user permissions, heartbeat detection, batch import, batch delete and other functions, and write a standard GHTTP API document (see interface list), further enrich the functions of GHTTP, improve the robustness of GHTTP;
  - Add Personalized PageRank (PPR) custom function, which can be used to calculate the relatedness between entities to find the node with the most influence;
  - Added support for arithmetic and logical operations in Filter statements, such as arithmetic operations (e.g.? x + ? Y = 5); Logical operations (e.g.? x + ? y = 5 && ? Y > 0);
  - The gServer component is added to realize two-way Socket API communication. Users can access gStore remotely through GHTTP component and gServer component;
  - The local operation command format is planned, and the --help command is introduced. Users can view the detailed command format of each function. For example, bin/gbuild -h/–help can view the detailed command format of gbuild;
  - Fixed a number of bugs.

### **gStore 0.9.0 Version**

- Update Time：2021-02-10
- Updated functions：
  - Upgrade the SPARQL parser generator from ANTLR V3 to the latest, well-documented, and well-maintained V4;
  - Support writing numeric literals without data type suffixes in SPARQL queries;
  - Support for arithmetic and logical operators in SELECT clauses;
  - Supports aggregations of SUM, AVG, MIN, and MAX in the SELECT clause;
  - Additional support is built into filters, and function functions including `datatype`，`contains`，`ucase`，`lcase`，`strstarts`，`now`，`year`，`month`，`day`，and`abs`;
  - Supports path-related functions as an extension of SPARQL 1.1, including loop detection, shortest paths, and K-Hop reachability;
  - Supports full and incremental database backup and recovery. Administrators can enable automatic full backup;
  - Supports Log-based rollback operations;
  - Supports transactions with three levels of isolation: committed reads, snapshot isolation, and serializable ;
  - Expand the data structure to accommodate large-scale graphs of up to 5 billion triples.

