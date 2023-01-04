> gstore version: 1.0
>
> Last modification date: 2022-12-31
>
> Modified by: JianWang
>
> Modification description: 

**This file maintains details of the bugs not solved currently.**

---

#### 1 Can not support single-quoted string literal

1.1 The bin/gbuild local command will throw a parse exception when a single-quoted string literal is built（SPARQL supports both single-quoted and double-quoted string literals）, for example：
   ```sparql
   <a> <b> 'c'.
   ```
2.1 The API allows you to insert single-quoted string literal triples, but the query will return a parse error error, for example:

   2.1.1 Insert sparql
   ```sparql
   insert data where {<a> <b> 'c'.}
   ```
   
   2.1.2 Query sparql：
   ```sparql
   select ?val where {<a> <b> ?val.}
   ```
---

