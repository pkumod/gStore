## Graph Patterns

This document mainly refer to the  [SPARQL 1.1 Standard document](https://www.w3.org/TR/sparql11-query/), but also increased the gStore own customized content, if you want to learn more about gStore SPARQL statement of support, Please read our documentation carefully!  

 

Unless otherwise specified, this document will continue to use the following RDF data instances as objects for queries:  

```
<YifeiLiu> <name> "Yifei Liu" .
<YifeiLiu> <name> "Crystal Liu" .
<YifeiLiu> <gender> "female" .
<YifeiLiu> <constellation> "Virgo" .
<YifeiLiu> <occupation> "actor" .

<ZhiyingLin> <name> "Zhiying Lin" .
<ZhiyingLin> <gender> "male" .
<ZhiyingLin> <occupation> "actor" .
<ZhiyingLin> <occupation> "director" .

<JunHu> <name> "Jun Hu" .
<JunHu> <gender> "male" .
<JunHu> <constellation> "pisces" .
<JunHu> <occupation> "actor" .
<JunHu> <occupation> "voice actors" .
<JunHu> <occupation> "producer" .
<JunHu> <occupation> "director" .

<TianlongBabu> <featured> <ZhiyingLin> .
<TianlongBabu> <featured> <YifeiLiu> .
<TianlongBabu> <featured> <JunHu> .
<TianlongBabu> <type> <ActionMovie> .
<TianlongBabu> <type> <CostumeFilms> .
<TianlongBabu> <type> <RomanticMovie> .
<TianlongBabu> <score> "8.3"^^<http://www.w3.org/2001/XMLSchema#float> .
<TianlongBabu> <releaseTime> "2003-12-11T00:00:00"^^<http://www.w3.org/2001/XMLSchema#dateTime> .

<TheLoveWinner> <featured> <ZhiyingLin> .
<TheLoveWinner> <featured> <YifeiLiu> .
<TheLoveWinner> <type> <RomanticMovie> .
<TheLoveWinner> <type> <FeatureFilm> .
<TheLoveWinner> <score> "6.1"^^<http://www.w3.org/2001/XMLSchema#float> .
<TheLoveWinner> <releaseTime> "2004-11-30T00:00:00"^^<http://www.w3.org/2001/XMLSchema#dateTime> .
```

Since there is no official Chinese translation of the SPARQL 1.1 standard document, the English version of the term will be indicated when it first appears in the following paragraphs.

By standard, **keywords in SPARQL queries are case insensitive**

<br/>

###  The simplest graph mode

Let's start with the simplest query：

```sparql
SELECT ?movie
WHERE
{
	?movie <featured> <YifeiLiu> .
}
```

The query consists of two parts: the **SELECT statement ** specifies the variables that need to output the query results, and the **WHERE statement ** provides the graph pattern used to match the data graph. In the above query, the graph pattern consists of a single **triplet** `?movie <featured> <YifeiLiu>` `?movie` is **variable** , and `<featured>` as predicate and`<YifeiLiu> `as object are **IRI** (International Resource Identifier). This query will return all movies and TV works starring Yifei Liui. The results run on the sample data are as follows:

| ?movie           |
| ---------------- |
| \<TianlongBabu>  |
| \<TheLoveWinner> |

The subject, predicate and object of a triple can all be IRI. Objects can also be **RDF literals **. The following query will give all the people in the sample data whose profession is director：

```sparql
SELECT ?person
WHERE
{
	?person <occupation> "director" .
}
```

Where 'director' is an RDF literal

Result are as follows：

| ?person       |
| ------------- |
| \<JunHu>      |
| \<ZhiyingLin> |

Under the current version of gStore, RDF literals with data types are queried with suffixes corresponding to those in the data file. For example, the following query will give a douban rating of 8.3:

```sparql
SELECT ?movie
WHERE
{
	?movie <score> "8.3"^^<http://www.w3.org/2001/XMLSchema#float> .
}
```

Results are as follows：

| ?movie          |
| --------------- |
| \<TianlongBabu> |

Other common data types include `<http://www.w3.org/2001/XMLSchema#integer>`（integer），`<http://www.w3.org/2001/XMLSchema#decimal>`（point type），`xsd:double`（A double-precision floating point type），`<http://www.w3.org/2001/XMLSchema#string>`（String type），`<http://www.w3.org/2001/XMLSchema#boolean>`（Boolean），`<http://www.w3.org/2001/XMLSchema#dateTime>`（Date/time).Other data types may also appear in data files, simply using the form `^^< data type suffix >` in the query.

### Basic Graph Pattern

**Base graph pattern** is a collection of triples; The two queries in the previous section both have only the outermost braces and therefore belong to the **basic graph mode**; Enclosing the outermost braces is a single base graph pattern **Group Graph Pattern**。

The basic graph pattern in the two queries in the previous section consists of a single triple. The following query uses a basic graph pattern consisting of multiple triples to give all male leads of Tianlongba in the sample data:

```sparql
SELECT ?person
WHERE
{
	<TianlongBabu> <featured> ?person .
	?person <gender> "male" .
}
```

Result are as follows：

| ?person       |
| ------------- |
| \<JunHu>      |
| \<ZhiyingLin> |

###  Group Graph Pattern

**Group graph patterns** are separated by paired braces. A group graph pattern can be composed of a single base graph pattern, as described in the previous section, or multiple subgroups of graph patterns nested with the following operations: **OPTIONAL**, **UNION**, and **MINUS**. **FILTER** filters the results within the range of a group graph pattern

**OPTIONAL**

The keyword OPTIONAL uses the following syntax:

```
pattern1 OPTIONAL { pattern2 }
```



The result of the query must match `pattern1` and selectively match `pattern2`. `Pattern2` is known as the OPTIONAL graph pattern. If there is a match for `pattern2`, add it to the match for `Pattern1`; Otherwise, the match for `pattern1`is still printed. For this reason, OPTIONAL is often used when some data is missing.  

 

The following query gives the gender and constellation information of the person in the sample data. Among them, as long as there is gender information of the character will be returned, regardless of whether there is the constellation information of the character; If both exist, additional returns are returned



```sparql
SELECT ?person ?gender ?horoscope
WHERE
{
	?person <gender> ?gender .
	OPTIONAL
	{
		?person <constellation> ?horoscope .
	}
}
```

Results are as follows：

| ?person       | ?gender  | ?horoscope |
| ------------- | -------- | ---------- |
| \<YifeiLiu>   | "female" | "Virgo"    |
| \<ZhiyingLin> | "male"   |            |
| \<JunHu>      | "male"   | "pisces"   |

**UNION**

The keyword UNION is syntactically similar to OPTIONAL. In a graph pattern joined by UNION, as long as there is one that matches a piece of data, that data matches the whole joined by UNION. Therefore, UNION can be understood as finding the set of matching results of each graph pattern it joins (actually using multiple set semantics because it allows repeating results).  

 

The following query gives the sample data for films and television works whose category is costume film or drama film：

```sparql
SELECT ?movie
WHERE
{
	{?movie <type> <CostumeFilms> .}
	UNION
	{?movie <type> <FeatureFilm> .}
}
```

Results are as follows：

| ?movie             |
| ------------------ |
| \<Tianlong Babu>   |
| \<The Love Winner> |

**MINUS**

The usage syntax of the keywords MINUS is similar to OPTIONAL and UNION. MINUS The matching of the left and right graph patterns will be calculated, and the part that can match the right graph pattern will be removed from the matching result of the left graph pattern as the final result. Therefore, MINUS can be understood as the difference of the matching result set of the two graph patterns connected to it (the left is the subtracted set, multiple set semantics).  

 

The following query will give the sample data of the characters who starred in The Dragon Eight but did not star in the love winner：

```sparql
SELECT ?person
WHERE
{
	<Tianlong Babu> <featured> ?person .
	MINUS
	{<The Love Winner> <featured> ?person .}
}
```

Results are as follows：

| ?person  |
| -------- |
| \<JunHu> |

**FILTER**

The keyword FILTER is followed by a constraint condition, and the results that do not meet this condition in the current pattern group will be filtered out and not returned. FILTER conditions can use equations, inequalities, and various built-in functions.  

 

The following query will give the film and television works with douban score higher than 8 points in the sample data:

```sparql
SELECT ?movie
WHERE
{
	?movie <score> ?score .
	FILTER (?score > "8"^^<http://www.w3.org/2001/XMLSchema#float>)
}
```

Results are as follows ：

| ?movie          |
| --------------- |
| \<TianlongBabu> |

No matter where a FILTER is placed in a group graph pattern, as long as it remains in the same nesting layer, its semantics remain unchanged and the scope of the constraint remains the current group graph pattern. For example, the following query is equivalent to the previous one：

```sparql
SELECT ?movie
WHERE
{
	FILTER (?score > "8"^^<http://www.w3.org/2001/XMLSchema#float>)
	?movie <score> ?score .
}
```

One of the built-in functions commonly used for FILTER conditions is the regular expression **REGEX**. The following query gives the liu surname in the sample data：

```sparql
SELECT ?person
WHERE
{
	?person <name> ?name .
	FILTER REGEX(?name, "Yi.*")
}
```

Results are as follows：

| ?person     |
| ----------- |
| \<YifeiLiu> |



## Assignment

The following keywords belong to assignment functions and can be used to define variables in the query body or provide inline data.

### BIND: assigning to a variable

```
BIND(value, name)
```

**Parameter**

`value`: a string value
`name`: name of a variable that has not yet appeared in the query

**Sample**

Query the occupation of YifeiLiu, JunHu and categorize the tags in the results returned:

```sql
SELECT ?a ?x WHERE
{
	{
		BIND("YifeiLiu" as ?info).
		<YifeiLiu> <occupation> ?profession.
	}
	UNION
	{
		BIND("JunHu" as ?info).
		<JunHu> <accupation> ?profession.
	}
}
```

The result is as follows: (For the convenience of reading, the escaping of the outermost double quotes and the inner double quotes of the string is omitted)

```json
{
	"bindings": [
		{
			"info": {"type": "literal", "value": "YifeiLiu"},
			"profession": {"type": "literal", "value": "actor"}
		},
		{
			"info": {"type": "literal", "value": "JunHu"},
			"profession": {"type": "literal", "value": "actor"}
		},
		{
			"info": {"type": "literal", "value": "JunHu"},
			"profession": {"type": "literal", "value": "voice actors"}
		},
		{
			"info": {"type": "literal", "value": "JunHu"},
			"profession": {"type": "literal", "value": "producer"}
		},
		{
			"info": {"type": "literal", "value": "JunHu"},
			"profession": {"type": "literal", "value": "director"}
		}
	]
}
```

Further improvements will be made to BIND expressions/functions, such as support for binding an entity  (IRI) to a variable.

### CONCAT: concat multiple characters

```
CONCAT(val_1, val_2,...val_n)
```

**Parameter**

`val_i`: string-typed string value

**Sample：**

Connect the name, gender and profession of the people found with each other：

```SPARQL
SELECT (CONCAT(?name, ",", str(?gender), ",", ?profession) as ?info) WHERE 
{
	?s <name> ?name.
	?s <gender> ?gender.
	?s <occupation> ?profession.
}
```

The final result output is as follows (for ease of reading, the outer double quotation marks and the inner double quotation escape in the string are omitted)：

```json
{
    "bindings": [
        {
            "info": {"type": "literal","value": "YifeiLiu,female,actor"}
        },
        {
            "info": {"type": "literal","value": "Crystal Liu,female,actor"}
        },
        {
            "info": {"type": "literal","value": "ZhiyingLin,male,actor"}
        },
        {
            "info": {"type": "literal","value": "ZhiyingLin,male,director"}
        },
        {
            "info": {"type": "literal","value": "JunHu,male,actor"}
        },
        {
            "info": {"type": "literal","value": "JunHu,male,director"}
        },
        {
            "info": {"type": "literal","value": "JunHu,male,voice actors"}
        },
        {
            "info": {"type": "literal","value": "JunHu,male,producer"}
        }
    ]
}
```



## Aggregates

Aggregate functions are used in SELECT statements with the following syntax:

```sparql
SELECT (AGGREGATE_NAME(?x) AS ?y)
WHERE
{
	...
}
```



Where `AGGREGATE_NAME` is the name of the aggregation function, variable `?X`is the aggregate function on the object, variable`?Y` is the column name of the aggregate function value in the final result.  

 

The aggregate function acts on each group of outcomes. All results are returned as a set by default. The aggregation functions supported by gStore are as follows

**COUNT**

Aggregate function for counting.  

The following query gives the number of actors in the sample data：

```sparql
SELECT (COUNT(?person) AS ?count_person)
WHERE
{
	?person <occupation> "actor" .
}
```

Results are as follows：

| ?count_person                                    |
| ------------------------------------------------ |
| "3"^^\<http://www.w3.org/2001/XMLSchema#integer> |

**SUM**

Aggregate function for summation.  

 

The following query will give the sum of douban ratings for all movies in the sample data:

```sparql
SELECT (SUM(?score) AS ?sum_score)
WHERE
{
	?movie <score> ?score .
}
```

Results are as follows：

| ?sum_score                                            |
| ----------------------------------------------------- |
| "14.400000"^^<http://www.w3.org/2001/XMLSchema#float> |

**AVG**

An aggregate function for averaging.  

The following query gives the average Douban score for all movies in the sample data:

```sparql
SELECT (AVG(?score) AS ?avg_score)
WHERE
{
	?movie <score> ?score .
}
```

Results are as follows：

| ?avg_score                                           |
| ---------------------------------------------------- |
| "7.200000"^^<http://www.w3.org/2001/XMLSchema#float> |

**MIN**

An aggregate function for minimizing.  

The following query will give the lowest Douban score for all movies in the sample data:

```sparql
SELECT (MIN(?score) AS ?min_score)
WHERE
{
	?movie <score> ?score .
}
```

Results are as follows：

| ?min_score                                      |
| ----------------------------------------------- |
| "6.1"^^<http://www.w3.org/2001/XMLSchema#float> |

**MAX**

Aggregate function used to find the maximum value.  

The following query will give the highest Douban ratings for all movies in the sample data:

```sparql
SELECT (MAX(?score) AS ?max_score)
WHERE
{
	?movie <score> ?score .
}
```

Results are as follows：

| ?max_score                                      |
| ----------------------------------------------- |
| "8.3"^^<http://www.w3.org/2001/XMLSchema#float> |

**GROUP BY**

If you want to GROUP the results BY the value of a variable, you can use the keyword GROUP BY. For example, the following query gives all occupations and their corresponding numbers in the sample data:

```sparql
SELECT ?occupation (COUNT(?person) AS ?count_person)
WHERE
{
	?person <occupation> ?occupation .
}
GROUP BY ?occupation
```

Results are as follows：

| ?occupation    | ?count_person                                    |
| -------------- | ------------------------------------------------ |
| "actor"        | "3"^^\<http://www.w3.org/2001/XMLSchema#integer> |
| "director"     | "2"^^\<http://www.w3.org/2001/XMLSchema#integer> |
| "voice actors" | "1"^^\<http://www.w3.org/2001/XMLSchema#integer> |
| "producer"     | "1"^^\<http://www.w3.org/2001/XMLSchema#integer> |

<br/>



## Solution Sequences and Modifiers

The following keywords are result sequence modifiers that post process the query results to form the final returned results.

**DISTINCT: Removes duplicate results**  

A query with a SELECT statement without the keyword DISTINCT will retain duplicate results in the final result. For example, the following query gives all the occupations in the sample data

```sparql
SELECT ?occupation
WHERE
{
	?person <occupation> ?occupation .
}
```

Results are as follows：

| ?occupation    |
| -------------- |
| "actor"        |
| "actor"        |
| "actor"        |
| "director"     |
| "director"     |
| "producer"     |
| "voice actors" |

If you want to see DISTINCT job categories, you can add the keyword DISTINCT in the SELECT statement:

```sparql
SELECT DISTINCT ?occupation
WHERE
{
	?person <occupation> ?occupation .
}
```

Results are as follows：

| ?occupation    |
| -------------- |
| "actor"        |
| "director"     |
| "producer"     |
| "voice actors" |

DISTINCT can also be used in the aggregate function COUNT. The following query gives the number of occupations in the sample data:

```sparql
SELECT (COUNT(DISTINCT ?occupation) AS ?count_occupation)
WHERE
{
	?person <occupation> ?occupation .
}
```

Results are as follows：

| ?count_occupation                                |
| ------------------------------------------------ |
| "4"^^\<http://www.w3.org/2001/XMLSchema#integer> |

**ORDER BY: 排序**

Query results are unordered by default. If you want to sort the results based on the values of some variables, you can add an ORDER BY statement after the WHERE statement. For example, the following query will sort the film and television works in the sample data according to douban score. If the order is not specified, it will be in ascending order by default

```sparql
SELECT ?movie ?score
WHERE
{
	?movie <score> ?score
}
ORDER BY ?score
```

Results are as follows：

| ?movie            | ?score                                           |
| ----------------- | ------------------------------------------------ |
| \<The LoveWinner> | "6.1"^^\<http://www.w3.org/2001/XMLSchema#float> |
| \<TianlongBabu>   | "8.3"^^\<http://www.w3.org/2001/XMLSchema#float> |

If you want to sort in descending order, you need to modify the variable name with the keyword DESC:

```sparql
SELECT ?movie ?score
WHERE
{
	?movie <score> ?score
}
ORDER BY DESC(?score)
```

Results are as follows：

| ?movie           | ?score                                           |
| ---------------- | ------------------------------------------------ |
| \<TianlongBabu>  | "8.3"^^\<http://www.w3.org/2001/XMLSchema#float> |
| \<TheLoveWinner> | "6.1"^^\<http://www.w3.org/2001/XMLSchema#float> |

The ORDER BY statement can contain multiple space-separated variables, each of which can be decorated with DESC. gStore does not currently support the use of four-operation expressions and built-in functions in ORDER BY statements.  

**OFFSET: skips a certain number of results**  

The OFFSET statement follows the WHERE statement and has the following syntax：

```sparql
OFFSET nonnegative_integer
```

`nonnegative_INTEGER` must be a non-negative integer, indicating the number of results to be skipped. `OFFSET 0` is syntactic but has no effect on the result. Because the query results are unordered by default, SPARQL semantics do not guarantee that the skipped results meet any deterministic conditions. Therefore, the OFFSET statement is typically used in conjunction with the ORDER BY statement。

The following query sorts the film and television works in the sample data by douban score from lowest to highest, and skips the film and television works with the lowest score:

```sparql
SELECT ?movie ?score
WHERE
{
	?movie <score> ?score .
}
ORDER BY ?score
OFFSET 1
```

Results are as follows：

| ?movie          | ?score                                           |
| --------------- | ------------------------------------------------ |
| \<TianlongBabu> | "8.3"^^\<http://www.w3.org/2001/XMLSchema#float> |

**LIMIT: Limit the number of results**

The syntax of the LIMIT statement is similar to that of the OFFSET statement:

```sparql
LIMIT nonnegative_integer
```

`nonnegative_INTEGER` must be a non-negative integer, indicating the maximum number of results allowed. Similar to OFFSET, the LIMIT statement is typically used in conjunction with the ORDER BY statement because the query result defaults to unordered.  

The following query gives the film and television works with the highest douban score in the sample data:

```sparql
SELECT ?movie ?scoreWHERE{	?movie <score> ?score .}ORDER BY DESC(?score)LIMIT 1
```

Results are as follows：

| ?movie          | ?score                                           |
| --------------- | ------------------------------------------------ |
| \<TianlongBabu> | "8.3"^^\<http://www.w3.org/2001/XMLSchema#float> |



## Update graph

By **INSERT DATA**, **DELETE DATA**, and **DELETE WHERE** queries, we can INSERT or DELETE triples from the database.

**INSERT DATA**

INSERT DATA is used to INSERT triples into a database. The syntax is similar to that of a SELECT query, except that there are no variables in the triples that make up the group graph pattern.

The following query inserts the relevant information of the film and TELEVISION works Chinese Paladin into the sample data：

```sparql
INSERT DATA
{
	<Paladin> <featured> <GeHu> .
	<Paladin> <featured> <YifeiLiu> .
	<Paladin> <type> <ActionMovie> .
	<Paladin> <type> <CostumeFilms> .
	<Paladin> <type> <RomanticMovie> .
	<Paladin> <score> "8.9"^^<http://www.w3.org/2001/XMLSchema#float> .
}
```

The query that appears in the "Graph pattern - The simplest Graph pattern" section

```sparql
SELECT ?movie
WHERE
{
	?movie <featured> <YifeiLiu> .
}
```

After inserting the above data, the result becomes:

| ?movie           |
| ---------------- |
| \<TianlongBabu>  |
| \<TheLoveWinner> |
| \<Paladin>       |

**DELETE DATA**

DELETE DATA is used to DELETE triples from a database. The usage is exactly similar to INSERT DATA.

**DELETE WHERE**

DELETE DATA is used to DELETE eligible triples from the database; In contrast to DELETE DATA, its WHERE statement is exactly the same as the WHERE statement of a SELECT query, meaning that variables are allowed in triples. For example, the following query removes all information about swordsman films from the sample data:

```sparql
DELETE WHERE{	?movie <type> <ActionMovie> .	?movie ?y ?z .}
```

Run the query that appeared in the "Graph patterns - The simplest Graph patterns" section again:

```sparql
SELECT ?movie
WHERE
{
	?movie <featured> <YifeiLiu> .
}
```

Result change to：

| ?movie           |
| ---------------- |
| \<TheLoveWinner> |



## Advanced functions

In **kernel version V0.9.1**, gStore has added a number of queries related to the path and centrality of nodes in the data graph, including loop query, shortest path query, K-hop reachable query and Personalized PageRank query.

When using advanced functions, you need to load the CSR resources. When starting the HTTP API service, you need to add the parameter `-c 1`. Please see [Quick Start] - [Common API] - [HTTP API service] for details.  

### Sample data

To better demonstrate the advanced functionality, use the following social relationship data as sample data：

```
<Alice> <focus> <Bob> .
<Alice> <like> <Bob> .
<Alice> <dislike> <Eve> .
<Bob> <focus> <Alice> .
<Bob> <like> <Eve> .
<Carol> <focus> <Bob> .
<Carol> <like> <Bob> .
<Carol> <dislike> <Francis> .
<Dave> <focus> <Alice> .
<Dave> <focus> <Eve> .
<Dave> <dislike> <Francis> .
<Eve> <like> <Carol> .
<Francis> <like> <Carol> .
<Francis> <dislike> <Dave> .
<Francis> <dislike> <Eve> .
```

The above data are illustrated below:

<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/%E7%A4%BE%E4%BA%A4%E5%9B%BE.png" alt="社交图" style="zoom:50%;" />

Unless otherwise specified, functions that return paths represent a path/a ring/a subgraph in JSON format as follows:

```JSON
{
    "src": "<src_IRI>", "dst": "<dst_IRI>",
  	"edges": [
    	{ "fromNode": 0, "toNode": 1, "predIRI": "<pred>" }
    ],
  	"nodes": [
        { "nodeIndex": 0, "nodeIRI": "<src_IRI>" },
        { "nodeIndex": 1, "nodeIRI": "<dst_IRI>" }
    ]
}
```

The final return value represents a set of paths/rings/subgraphs as follows :(where the 'paths' element has the format above)

```JSON
{ "paths": [{...}, {...}, ...] }
```

### Path-related query

**(1) The cycle detection query**

Queries for the existence of a cycle containing nodes ` u `and `v`

```
cyclePath(u, v, directed, pred_set)
cycleBoolean(u, v, directed, pred_set)
```

Used in SELECT statements, using the same syntax as aggregate functions

Parameter

`u`, `v` ：Variable or node IRI

`directed` ：A Boolean value, true for directed and false for undirected (all edges in the graph are considered bidirectional

`pred_set` ：The set of predicates that make up the edges of a ring. If set to null '{}', all predicates in the data are allowed

**Return value**

- `cyclePath` ：Returns a ring containing the nodes`u` and`v `(if any) in JSON form. If `u` or `v` is a variable, a loop is returned for each set of valid values of the variable
- `cycleBoolean` ：Return true if there is a ring containing nodes `u `and `v`; Otherwise, return false

The following query asks if there is a directed ring that contains Carol, a person Francis dislikes (Dave or Eve in the sample data), and whose edges can only be marked by a "like" relationship：

```sparql
select (cycleBoolean(?x, <Carol>, true, {<like>}) as ?y)where{	<Francis> <dislike> ?x .}
```

Results are as follows：

| ?y                                           |
| -------------------------------------------- |
| "true"^^\<http://www.w3.org/2001/XMLSchema#> |

If you want to output a loop that meets the above criteria, use the following query:

```sparql
SELECT (cyclePath(?x, <Carol>, true, {<like>}) as ?y)
WHERE
{
	<Francis> <dislike> ?x .
}
```

Results are as follows，It can be seen that one of the rings satisfying the condition is formed by Eve likes carol-carol likes bob-bob likes Eve in sequence :(the escape of the outermost double quotation marks and the inner double quotation marks are omitted for ease of reading)

```json
{
	"paths":[{
    "src":"<Eve>",
    "dst":"<Carol>",
    "edges":
    [{"fromNode":2,"toNode":3,"predIRI":"<like>"},{"fromNode":3,"toNode":1,"predIRI":"<like>"},{"fromNode":1,"toNode":2,"predIRI":"<like>"}],
    "nodes":
    [{"nodeIndex":1,"nodeIRI":"<Bob>"},{"nodeIndex":3,"nodeIRI":"<Carol>"},{"nodeIndex":2,"nodeIRI":"<Eve>"}]
	}]
}
```

The red part below is the ring:

<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/%E7%A4%BE%E4%BA%A4%E5%9B%BE2.png" alt="社交图2" style="zoom:50%;" />

**(2) Shortest path Query**

Query the shortest path from node `u` to node `v`

```
shortestPath(u, v, directed, pred_set)
shortestPathLen(u, v, directed, pred_set)
```

Used in SELECT statements, using the same syntax as aggregate function.

**Parameter**

`u` , `v` ：Variable or node IRI

`directed` ：Boolean value, true for directed, false for undirected (all edges in the graph are considered bidirectional)

`pred_set` ：The set of predicates that are allowed to occur on the side that makes up the shortest path. If set to null '{}', all predicates in the data are allowed

**Return value**

- `shortestPath` ：Returns a shortest path (if reachable) from node `u` to `v` in JSON form. If `u` or `v` is a variable, a shortest path is returned for each set of valid values of the variable.  
- `shortestPathLen` ：Returns the shortest path length (if reachable) from node `u` to `v`. If `u` or `v` is a variable, return a shortest path length value for each set of valid values of the variable。

The following query returns the shortest path from Francis to a person (Alice in the example data) that Bob likes, cares about, or dislikes, and is not disliked by Francis, with a relationship that can be like or care about.

```sparql
SELECT (shortestPath(<Francis>, ?x, true, {<like>, <focus>}) AS ?y)WHERE{	<Bob> ?pred ?x .	MINUS { <Francis> <dislike> ?x . }}
```

The red part below is the shortest path:

<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/%E7%A4%BE%E4%BA%A4%E5%9B%BE3.png" alt="社交图3" style="zoom:50%;" />

Results are as follows：(For easy reading, the outermost double quotation mark and the escape of the inner double quotation mark are omitted.)

```json
{	"paths":[{		"src":"<Francis>",		"dst":"<Alice>",		"edges":		[{"fromNode":4,"toNode":3,"predIRI":"<like>"},{"fromNode":3,"toNode":1,"predIRI":"<like>"},{"fromNode":1,"toNode":0,"predIRI":"<focus>"}],		"nodes":		[{"nodeIndex":0,"nodeIRI":"<Alice>"},{"nodeIndex":1,"nodeIRI":"<Bob>"},{"nodeIndex":3,"nodeIRI":"<Carol>"},{"nodeIndex":4,"nodeIRI":"<Francis>"}]		}]}
```

If you want to output only the shortest path length, use the following query:

```sparql
SELECT (shortestPathLen(<Francis>, ?x, true, {<like>, <focus>}) AS ?y)WHERE{	<Bob> ?pred ?x .	MINUS { <Francis> <dislike> ?x . }}
```

Results are as follows：(For easy reading, the outermost double quotation mark and the escape of the inner double quotation mark are omitted)

```json
{"paths":[{"src":"<Francis>","dst":"<Alice>","length":3}]}
```

**(3) Reachability/K-hop reachability query**

Query whether node `u` is reachable or K-hop reachable to node `v` (i.e., there exists a path with `u` as its source and `v` as its destination whose length does not exceed `k`).

```
kHopReachable(u, v, directed, k, pred_set)kHopReachablePath(u, v, directed, k, pred_set)
```

**Parameter**

`u`, `v` ：Variable or node IRI

`k` ：If it is set to a non-negative integer, it is the upper limit of the path length (query k-hop reachability). If set to negative, query reachability

`directed` ：A Boolean value, true for directed and false for undirected (all edges in the graph are considered bidirectional

`pred_set` ：The set of predicates that are allowed to occur on the side that constitutes a path. If set to null `{}`, all predicates in the data are allowed

**Return value**

- `kHopReachable`：Return true if node `u` is reachable to node `v` (or K hop reachable, depending on the value of parameter `K`); Otherwise, return false. If `u` or `v` is a variable, return a true/false value for each set of valid values of the variable
- `kHopReachablePath`：Returns any path from node `u` to node `v` (if reachable) or a k-hop path, that is, a path with length less than or equal to `K` (if reachable, depending on the value of `K`). If `u` or `v` is a variable, return a path (if reachable) or a k-hop path (if reachable) for each set of valid values of the variable

The following query follows the example query from the previous section, "Shortest path Query" : It starts with Francis and ends with a person that Bob likes, cares about, or dislikes, and is not disliked by Francis (which is Alice in the example data). Ask if the relationship between the two people is 2 hops or within reach through liking or following.

```sparql
SELECT (kHopReachable(<Francis>, ?x, true, 2, {<like>, <focus>}) AS ?y)WHERE{	<Bob> ?pred ?x .	MINUS { <Francis> <dislike> ?x . }}
```

Since the shortest path length satisfying the condition is known to be 3:

<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/%E7%A4%BE%E4%BA%A4%E5%9B%BE3-9966466.png" alt="社交图3" style="zoom:50%;" />

Therefore, the above query results are false：

```json
{"paths":[{"src":"<Francis>","dst":"<Alice>","value":"false"}]}
```

Francis and Alice, on the other hand, are reachable, but the shortest path length exceeds the above limit. So a query for reachability (with `k` set to negative) returns true:

```sparql
SELECT (kHopReachable(<Francis>, ?x, true, -1, {<like>, <focus>}) AS ?y)WHERE{	<Bob> ?pred ?x .	MINUS { <Francis> <dislike> ?x . }}
```

Results are as follows：

```json
{"paths":[{"src":"<Francis>","dst":"<Alice>","value":"true"}]}
```

If you want to return a path that satisfies the condition between two people, you can call the `kHopReachablePath` function：

```SPARQL
SELECT (kHopReachablePath(<Francis>, ?x, true, -1, {<like>, <focus>}) AS ?y)WHERE{	<Bob> ?pred ?x .	MINUS { <Francis> <dislike> ?x . }}
```

The result may be the shortest path described above：

```json
{	"paths":[{		"src":"<Francis>",		"dst":"<Alice>",		"edges":		[{"fromNode":4,"toNode":3,"predIRI":"<like>"},{"fromNode":3,"toNode":1,"predIRI":"<like>"},{"fromNode":1,"toNode":0,"predIRI":"<focus>"}],		"nodes":		[{"nodeIndex":0,"nodeIRI":"<Alice>"},{"nodeIndex":1,"nodeIRI":"<Bob>"},{"nodeIndex":3,"nodeIRI":"<Carol>"},{"nodeIndex":4,"nodeIRI":"<Francis>"}]		}]}
```

It could also be a non-shortest path with a ring in it, as shown in the figure below:

<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/%E7%A4%BE%E4%BA%A4%E5%9B%BE4.png" alt="社交图4" style="zoom:50%;" />

**(4) All K-hop paths**

Query all K-hop reachable paths from node `u` to node `v`.


```
kHopEnumerate(u, v, directed, k, pred_set)
```

**Parameter**

`u`, `v` ：Variable or node IRI

`k` ： if set to a non-negative integer, it is the upper limit of the path length (querying K-hop reachability); if set to a negative number, it queries reachability

`directed` ：Boolean value, true indicates directed, false indicates undirected (all edges in the graph are considered bidirectional)

`pred_set` ：The set of predicates allowed on the edges that form the path. If set to an empty `{}`, it means that all predicates in the data are allowed.

**Return Value**

Return all paths (if reachable) or K-hop paths (if K-hop reachable, depending on the value of parameter `k`) from node `u` to node `v`. If `u` or `v` is a variable, return all paths (if reachable) or K-hop paths (if K-hop reachable) for each valid value of the variable.

**Example**

The return value is in the following form, where SRC is the result of IRI or variable query corresponding to u. Which destination nodes DST contains depends on the second argument to the function; The corresponding PPR value is a double precision floating point number.

```
SELECT (kHopEnumerate(<Alice>, ?x, true, 3, {<like>, <focus>}) AS ?y) 
WHERE 
{
    <Francis> ?pred ?x.
    MINUS { <Alice> <dislike> 
```

Query the path between Alice and Francis, who is liked, followed, or disliked by Francis and not disliked by Alice. The path should be reachable within 3 hops through liking or following relationship. (Example data: Carol)



![](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/mdimg/%E8%99%9A%E6%8B%9F%E7%BD%91%E7%BB%9C%E7%A4%BE%E4%BA%A4%E5%9B%BE-kHopEmulate.png)



```
{
    "paths":[{
        "src":"<Alice>",
        "dst":"<Carol>",
        "edges":[{"fromNode":0,"toNode":1,"predIRI":"<like>"},{"fromNode":1,"toNode":2,"predIRI":"<like>"},{"fromNode":2,"toNode":3,"predIRI":"<like>"}],
        "nodes":[{"nodeIndex":3,"nodeIRI":"<Carol>"},{"nodeIndex":2,"nodeIRI":"<Eve>"},{"nodeIndex":0,"nodeIRI":"<Alice>"},{"nodeIndex":1,"nodeIRI":"<Bob>"}]},{"src":"<Alice>","dst":"<Carol>","edges":[{"fromNode":0,"toNode":1,"predIRI":"<focus>"},{"fromNode":1,"toNode":2,"predIRI":"<like>"},{"fromNode":2,"toNode":3,"predIRI":"<like>"}],"nodes":[{"nodeIndex":3,"nodeIRI":"<Carol>"},{"nodeIndex":2,"nodeIRI":"<Eve>"},{"nodeIndex":0,"nodeIRI":"<Alice>"},{"nodeIndex":1,"nodeIRI":"<Bob>"}]}]
}
```



**(5) kHopCount** 

Query the number of nodes accessible from node `u` within k layers.

```c++
kHopCount(u, directed, k, pred_set)
```

**Parameter**

`u`: variable or node IRI
`k`: Number of hops (only counting the nodes reachable within this number of hops)
`directed`: Boolean value, true for directed, false for undirected (all edges in the graph are considered bidirectional) 
`pred_set`: The set of predicates that are allowed to occur on the edges that makes up the shortest path. If set to null `{}`, all predicates in the data are allowed.

**Return value**

The return value is in the following format, where src is the IRI corresponding to u; depth is the level/height at which the node is located (equal to the parameter k); and count is the total number of nodes visited at the current level, with a type of integer.

```
{
    "paths":[
        { "src": "<Alice>", "depth": 3, "count": 1}
    ]
}
```

**(6) KHopNeighbor**

Query the nodes that are reachable from node u within k layers.

```sql
kHopNeighbor(u, directed, k, pred_set, ret_num)
```

**Parameter**

`u`: variable or node IRI
`k`: Number of hops (only counting the nodes reachable within this number of hops)
`directed`: Boolean value, true for directed, false for undirected (all edges in the graph are considered bidirectional) 
`ret_num`：Integer, optional, defaulted to 100, representing the maximum number of node IRIs to return. If the total number of nodes is less than ret_num, all node IRIs will be returned.

**Return value:**

The return value is in the following format, where src is the IRI corresponding to `u`; depth is the level/height at which the node is located (equal to the parameter `k`); and dst is a list of nodes visited at the current level.

```json
{
    "paths":[
        { 
            "src": "<Alice>", 
            "depth": 3, 
            "dst": [
                "<Car>"
            ]
        }
    ]
}
```



**(7) bfsCount**

The query starts from node u and outputs the number of nodes accessed in different layers in the breadth-first traversal order.

```
bfsCount(u, directed, pred_set)
```

**Parameter**

`u`: variable or node IRI, indicating the source node
`directed`: boolean value, true for directed, false for undirected (all edges in the graph are considered bidirectional) 
`pred_set`: The set of predicates that are allowed to occur on the side that makes up the shortest path. If set to null `{}`, all predicates in the data are allowed.

**Return value**

The return value is in the following form, where src is the IRI corresponding to u; depth is the number of layers/height (only accesses u when depth is 0); count is the total number of nodes accessed at the layer, of integer type.

```json
{"paths":
	[
		{"src":"<Alice>",
		"results":[{"depth":0, "count":1}, ...]
		}
	]
}
```

**Sample**

The following query returns the directed breadth-first traversal count with Alice as the source node. The relationship on the edge can be like, follow, or dislike, and the query is:

```json
SELECT(bfsCount(<Alice>,true,{<like>,<focus>,<dislike>}) AS ?y)
WHERE{}
```

The results are as follows:

```json
{"paths":
	[
		{"src":"<Alice>",
         "results":[{"depth":0, "count":1}, {"depth":1, "count":2}, {"depth":2, "count":1}, {"depth":3, "count":1}, {"depth":4, "count":1}]
		}
	]
}
```

### Importance analysis

**(1) PageRank**

```
PR(directed, pred_set, alpha, maxIter, tol)
```

**Parameter**

`directed`：Boolean value, true indicating directed, false indicating undirected (all edges in the graph are considered bidirectional)

`pred_set`：The set of predicates to be considered (if set to an empty `{}`, it means allowing all predicates in the data)

`alpha`：The damping factor of PageRank, representing the probability of following an edge in a random walk model

`maxIter`：The maximum number of iterations for solving PageRank using an iterative method

`tol`：The tolerance level for the error between two iterations in the PageRank value

**Return value**

The return value is in the following format, where src is the IRI of the node; result is the PageRank value of the corresponding node in the graph, with a type of float.

```json
{
    "paths": [
        {
            "src": "<Alice>",
            "results": 0.1
        }
    ]
}
```

**(2) Personalized PageRank**

```
PPR(u, hopCnt, pred_set, retNum)
```

The FORA algorithm [1] is invoked to calculate the top-K PPR value relative to u.

[1] S. Wang, R. Yang, X. Xiao, Z. Wei, and Y. Yang, “FORA: Simple and Effective Approximate Single Source Personalized PageRank,” in Proceedings of the 23rd ACM SIGKDD International Conference on Knowledge Discovery and Data Mining, Halifax NS Canada, Aug. 2017, pp. 505–514. doi: 10.1145/3097983.3098072.  

**Parameter**

`u`: variable or node IRI, indicating the source node
`hopCnt`: an integer. When it's -1, PPR can be calculated without limitation. Otherwise, it is restricted to `hopCnt` hops
`pred_set`: The set of predicates that are allowed to occur on the side that makes up the shortest path. If set to null '{}', all predicates in the data are allowed

`retNum`: an integer, indicating that the `retNum` node IRIs with the largest PPR and their corresponding PPR values need to be returned (If the total number of nodes in the graph is fewer than `retNum`, then return all node IRIs and their corresponding PPR values)

**Return value**

The return value is in the following form, where src is u's IRI, or its query result if u is a variable; which destination nodes dst contains depends on the last argument of the function; the PPR value is a double-precision floating point number.

(Note: Since FORA is an approximate algorithm with randomness, it is normal for the return value to be slightly different each time.)

```json
{"paths":
	[
		{"src":"<Francis>", "results":
			[{"dst":"<Alice>", "PPR":0.1}, {"dst":"<Bob>", "PPR": 0.01}, ...]
		}, ...
	]
}
```

**Sample**

Return the three people (and their PPR values) with top-3 PPR values corresponding to all the people Bob likes, follows, or dislikes:

```sql
select (PPR(?x, -1, {}, 3) as ?y)
where
{
	<Bob> ?pred ?x .
}
```

**(3) closenessCentrality**

Query how easy it is for a node to reach other nodes.

```
closenessCentrality(u, directed, pred_set)
```

**Parameter**

`u`: variable or node IRI, indicating the source node
`directed`: boolean value, true for directed, false for undirected (all edges in the graph are considered bidirectional) 
`pred_set`: The set of predicates that are allowed to occur on the side that makes up the shortest path. If set to null `{}`, all predicates in the data are allowed.

**Return value**

The return value is in the following form, where src is the IRI corresponding to u; result is the closeness centrality of node u in the graph and is of double-precision floating-point type.

```json
{
	"paths": [
		{
			"src": "<Alice>",
			"result": 0.5
		}
	]
}
```

**Sample**

Example 1. The query returns the closeness centrality of **Alice** in an **undirected graph** (all edges in the graph are regarded as bidirectional), and the relationship between edges can be **like** or **follow**. The SPARQL query is:

```sql
SELECT (closenessCentrality(<Alice>, false, {<like>, <focus>}) AS ?x) WHERE{}
```

The result is as follows: (For the convenience of reading, the escaping of the outermost double quotes and the inner double quotes of the string is omitted)

```json
{
	"paths":[
		{
			"src": "<Alice>",
			"result": 0.555556
		}
	]
}
```

In the above query, the shortest distance for Alice to reach the other nodes is as follows, and the average distance can be calculated as 1.8, and the value of closeness centrality is 1/1.8 =0.555556, which is consistent with the result.

```json
{
	"Bob" : 1,
	"Dave" : 1,
	"Eve" : 2,
	"Carol" : 2,
	"Francis" : 3
}
```

Example 2. The query returns **Alice**'s closeness centrality in the **directed graph**, and the relationship of edges can be **like** or **follow**. The SPARQL query is:

```sql
SELECT (closenessCentrality(<Alice>, true, {<like>, <focus>}) AS ?x) WHERE{}
```

The result is as follows: (For the convenience of reading, the escaping of the outermost double quotes and the inner double quotes of the string is omitted)

```json
{
	"paths":[
		{
			"src": "<Alice>",
			"result": 0.500000
		}
	]
}
```

In the above query, the shortest distance for Alice to reach the other nodes is as follows, and the average distance can be calculated as 2, and the value of closeness centrality is 1/2 =0.5, which is consistent with the result.

```json
{
	"Bob" : 1,
	"Eve" : 2,
	"Carol" : 3
}
```

**(4)  triangleCounting**

Count the number of triangles in the graph.

```
triangleCounting(directed, pred_set)
```

**Parameter**

`u`: variable or node IRI, indicating the source node
`directed`: boolean value, true for directed, false for undirected (all edges in the graph are considered bidirectional) 
`pred_set`: The set of predicates that are allowed to occur on the side that makes up the shortest path. If set to null `{}`, all predicates in the data are allowed.

**Return value**

The return value is in the following form:

```json
{
	"paths": [2]
}
```

**Sample**

Query the number of directed triangles in the graph, and the edges that form it can only be labeled by the **like** relation. The SPARQL query statement is:

```sql
select (triangleCounting(true, {<like>}) as ?y) where {}
```

<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/%E7%A4%BE%E4%BA%A4%E5%9B%BE4-9966557.png" alt="社交图4" style="zoom:50%;" />

The result is as follows. The number of directed triangles in the graph whose sides can only be marked by the "like" relation is 1, that is, Bob -> Eve -> Carol-> Bob:

```json
{
	"paths": [1]
}
```

<div STYLE="page-break-after: always;"></div>
**(5) tag propagation**

Based on tag propagation, it is possible to query the clustering status of each node in a graph and apply it to various applications such as community detection.

```
labelProp(directed, pred_set)
```

**Parameter **

`directed`：Boolean value, true indicating directed, false indicating undirected (all edges in the graph are considered bidirectional)

`pred_set`：The set of predicates to consider (if set to an empty `{}`, it means allowing all predicates in the data)

`maxIter`: Maximum iteration times

**Return value**

The returned value is an array of arrays (nested arrays), where the elements are node IRIs, corresponding to a partition of the nodes in the graph.

```cpp
{
    "paths": [
        [
            "<Alice>",
            "<Bob>"
        ],
        [
            "<Carol>"
        ]
    ]
}
```

**(6) Weakly Connected Components**

Return all weakly connected components of the graph.

```
WCC(pred_set)
```

**Parameters**

`pred_set`: The set of predicates allowed on the edges that form the weakly connected components. If set to an empty `{}`, it means that all predicates in the data are allowed to appear.

**Return value**

Nested arrays, with the same form as the return value of label propagation.

**(7) Local Agglomeration Coefficient**

Query the local clustering coefficient of node u, which is the number of edges between all nodes connected to it (i.e., the number of triangles formed with u as the vertex), divided by the maximum number of edges that can be connected between these nodes (i.e., the maximum number of triangles that can be formed with u as the vertex).

```
clusterCoeff(u, directed, pred_set)
```

**Parameters**

`u`: variable or node IRI

`directed`: Boolean value, true indicates directed, false indicates undirected (all edges in the graph are considered bidirectional). When the graph is considered directed, only cycle-type triangles are counted (see the introduction to triangle counting for details).

`pred_set`: the set of predicates considered (if set to an empty `{}`, it means that all predicates in the data are allowed to appear)

**Return value**

The return value is the local clustering coefficient of node u, and the corresponding value is a double-precision floating-point number (see the following example for details).

**(9) Overall agglomeration coefficient**

Query the overall clustering coefficient of the graph.

```
clusterCoeff(directed, pred_set)
```

**Parameters**

`directed`: Boolean value, true indicates directed, false indicates undirected (all edges in the graph are considered bidirectional). When the graph is considered directed, only cycle-type triangles are counted (see the introduction to triangle counting for details).

`pred_set`: the set of predicates considered (if set to an empty `{}`, it means that all predicates in the data are allowed to appear)

**Return value**

The return value is the overall clustering coefficient of the graph, and the corresponding value is a double-precision floating-point number.

**(10) Closeness centrality**

Return the closeness centrality of node u.

```cpp
closenessCentrality(u, directed, pred_set)
```

**Parameters**

`u`: variable or node IRI representing the source node.

`directed`: Boolean value, true indicating directed, false indicating undirected (all edges in the graph are considered bidirectional).

`pred_set`: the set of predicates considered (if set to an empty `{}`, it means that all predicates in the data are allowed to appear)

**Return value**

The return value is in the following format: src is the IRI corresponding to u; result is the closeness centrality of node u in the graph, which is of type float.

```cpp
{
    "paths": [
        {
            "src": "<Alice>",
            "results": 0.1
        }
    ]
}
```

**(11) Louvain**

```cpp
louvain(directed, pred_set, maxIter, increase)
```

**Parameter:**

`directed`: Boolean value, true indicating directed, false indicating undirected (all edges in the graph are considered bidirectional).

`pred_set`: The set of predicates to consider (if set to an empty {} , it means that all predicates in the data are allowed to appear).

`maxIter`: The maximum number of iterations in the first stage (>=1).

`increase`: Threshold for the gain in modularity (0~1).

**Return value:**

The return value is in the following format: count is the number of communities divided, details is the information of each divided community, including community ID (communityId) and member number (memberNum).

```cpp
{
    "count": 3,
    "details": [
        { "communityId": "2", "menberNum": 5},
        { "communityId": "4", "menberNum": 5},
        { "communityId": "5", "menberNum": 4}
    ]
}
```








