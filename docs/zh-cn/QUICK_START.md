## 数据格式

​		`gStore`是基于RDF模型的图数据库引擎，其数据格式也是遵循RDF模型的。RDF 是用于描述现实中资源的W3C 标准，它是描述信息的一种通用方法，使信息可以被计算机应用程序读取并理解。现实中任何实体都可以表示成RDF 模型中的资源，比如，图书的书名、作者、修改日期、内容以及版权信息。这些资源可以用作知识图谱中对客观世界的概念、实体和事件的抽象。每个资源的一个属性及属性值，或者它与其他资源的一条关系，都被称为一条知识。属性和关系能表示成三元组。

​		一个三元组包括三个元素：主体（Subject）、属性（Property）1及客体（Object），通常描述的是两个资源间的关系或一个资源的某种属性。当某个三元组描述了某个资源的属性时，其三个元素也被称为主体、属性及属性值（Property Value）。比如，三元组<亚里士多德、出生地、Chalcis>表达了亚里士多德出生于Chalcis 的事实。

​       利用这些属性和关系，大量资源就能被连接起来，形成一个大RDF 知识图谱数据集。因此，一个知识图谱通常可以视作三元组的集合。这些三元组集合进而构成一个RDF 数据集。知识图谱的三元组集合可以选择关系型数据库或者图数据库进行存储。gStore 1.2新增**Turtle**，**TriG**，**RDF/XML**，**RDFa**，**JSON-LD**等多种RDF序列化格式的支持，下面介绍主流的一些数据格式。

### N-Triple

​      并且必须以SPARQL1.1语法提供查询。N-Triple格式文件示例如下：        

```
@prefix foaf:  <http://xmlns.com/foaf/0.1/> .	
_:a  foaf:name   "Johnny Lee Outlaw" .
_:a  foaf:mbox   <mailto:jlow@example.com> .
_:b  foaf:name   "Peter Goodguy" .
_:b  foaf:mbox   <mailto:peter@example.org> .
_:c  foaf:mbox   <mailto:carol@example.org> .
```

N-triple通常采用W3C定义的NT文件格式存储，如下表示了3条RDF数据，其中以`<`和`>`包裹的值是一个实体的URI，表示的是一个实体，而以`""`包裹的是字面值，表示的是实体某个属性的值，且后面通过`^^`表示该值的类型。如下3条RDF数据，分别表示了`张三`这个实体的两个属性`性别`和`年龄`，值分别为`男`和`28`，最后一条表示的是`张三`这个实体与`李四`这个实体之间存在着一个`好友`的关系。

```NT
<张三> <性别> "男"^^<http://www.w3.org/2001/XMLSchema#String>.
<张三> <年龄> "28"^^<http://www.w3.org/2001/XMLSchema#Int>.
<张三> <好友> <李四>.
```

​      关于N-Triple文件更详细的描述请参考[N-Triple](https://www.w3.org/TR/n-triples/)。并非SPARQL1.1中的所有语法都是在gStore中解析和回答的，例如，属性路径超出了gStore系统的能力。Turtle家族中的Turtle和TriG的用法类似，文件描述参考[Turtle](https://www.w3.org/TR/turtle/)和[TriG](https://www.w3.org/TR/trig/)

### Turtle

​	Turtle是一种更加简洁的RDF序列化格式，它在N-triple的基础上加强了可读性和简洁性，增加了缩写功能，例如前缀等，如下缩写形式可以表示四条RDF三个月会议，其中，同一主语的多个谓语可以`;`进行间隔，同一主谓的多个谓语可通过`，`分割，Turtle的示例文件如下：

```
@prefix swp: <http://www.semanticwebprimer.org/ontology/apartments.ttl#>.
@prefix dbpedia: <http://dbpedia.org/resource>.
@prefix dbpedia-owl: <http://dbpedia.org/ontology>.
@prefix xsd: <http://www.w3.org/2001/XMLSchema#>.

swp:BaronWayAppartment swp:hasNumberOfBedrooms "3"^^xsd:integer;
                        swp:isPartOf swp:BaronWayBuilding.

swp:BaronWayBuilding dbpedia-owl:location dbpedia:Amsterdam,
                                          dbpedia:Netherlands.
```

​	Turtle通常采用W3C定义的ttl文件格式存储，关于Turtle文件更详细的描述请参考[Turtle](https://www.w3.org/TR/turtle/)。

### TriG

​	TriG是对Turtle语言的扩展，可以进一步的对图做出命名，示例如下：

```
# This document contains a same data as the
previous example.

@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .
@prefix dc: <http://purl.org/dc/terms/> .
@prefix foaf: <http://xmlns.com/foaf/0.1/> .

# default graph - no {} used.
<http://example.org/bob> dc:publisher "Bob" .
<http://example.org/alice> dc:publisher "Alice" .

# GRAPH keyword to highlight a named graph
# Abbreviation of triples using ;
GRAPH <http://example.org/bob>
{
   [] foaf:name "Bob" ;
      foaf:mbox <mailto:bob@oldcorp.example.org> ;
      foaf:knows _:b .
}

GRAPH <http://example.org/alice>
{
    _:b foaf:name "Alice" ;
        foaf:mbox <mailto:alice@work.example.org>
}
```

### RDF/XML

RDF/XML是由W3C定义的语法，用于表达（即序列化）RDF图作为XML文档。 RDF/XML有时会误导地称为RDF，因为它是在定义RDF的其他W3C规范中引入的，并且从历史上看，它是第一种W3C标准RDF RDF序列化格式。关于RDF/XML文件更详细的描述请参考[**RDF/XML**](https://www.w3.org/TR/rdf-syntax-grammar/)，示例如下：

```
<?xml version="1.0" encoding="utf-8"?>
<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
            xmlns:dc="http://purl.org/dc/elements/1.1/">

  <rdf:Description rdf:about="http://www.w3.org/TR/rdf-syntax-grammar">
    <dc:title>RDF 1.1 XML Syntax</dc:title>
    <dc:title xml:lang="en">RDF 1.1 XML Syntax</dc:title>
    <dc:title xml:lang="en-US">RDF 1.1 XML Syntax</dc:title>
  </rdf:Description>

  <rdf:Description rdf:about="http://example.org/buecher/baum" xml:lang="de">
    <dc:title>Der Baum</dc:title>
    <dc:description>Das Buch ist außergewöhnlich</dc:description>
    <dc:title xml:lang="en">The Tree</dc:title>
  </rdf:Description>

</rdf:RDF>
```

### JSON-LD

JSON-LD是一种轻巧的链接数据格式。人类很容易读写。它基于已经成功的JSON格式，并提供了一种帮助JSON数据互操作的方法。 JSON-LD是用于编程环境，REST Web服务以及Apache CouchDB和MongoDB等非结构化数据库的理想数据格式。链接的数据使人们能够在网络上发布和使用信息。这是一种在网站上创建基于标准的，可读数据的网络的方法。它允许应用程序以一条链接的数据启动，并按照嵌入式链接链接到其他网络上不同站点上托管的链接数据，关于JSON-LD文件更详细的描述请参考[JSON-LD](https://www.w3.org/TR/json-ld/)。示例如下：

```
{
  "@context": {
    "name": "http://xmlns.com/foaf/0.1/name",
    "knows": "http://xmlns.com/foaf/0.1/knows"
  },
  "@id": "http://me.markus-lanthaler.com/",
  "name": "Markus Lanthaler",
  "knows": [
    {
      "name": "Dave Longley"
    }
  ]
}
```



<!-- <div STYLE="page-break-after: always;"></div> -->
## 初始化系统数据库

只要下载并编译gStore系统的代码，就会自动创建一个名为system（真实目录名称system.db）的数据库。这是管理系统统计信息的数据库，包括所有用户和所有数据库。您可以使用gquery命令查询此数据库，但禁止使用编辑器对其进行修改。

`system`数据库为gStore内置的系统数据库，该数据库无法删除，用于保存系统相关信息，尤其是已构建的数据库信息，如果`system`数据库损坏，可能导致`ghttp`无法启动，因此gStore提供了初始化系统数据库功能

### 命令行模式（ginit)

ginit用于初始化数据库

用法：

```shell
bin/ginit -db [db_name1],[db_name2],[...]
```

命令参数：

```
db_name1：数据库名称
```

如果没有写任何的数据库名称，则重新初始化的`system`数据库中将没有其他数据库信息

示例：

```shell
[root@localhost gStore]$ bin/ginit -db lubm
The database lubm is not exist, now create it.
...
system.db init successfully! Used 24 ms
```

<!-- <div STYLE="page-break-after: always;"></div> -->
## 创建数据库

创建数据库操作是gStore最重要的操作之一，也是用户安装gStore后需要做的第一个操作，gStore提供多种方式进行数据库创建操作。

### 命名行模式（gbuild）

gbuild命令用于从RDF格式文件创建新的数据库，使用方式：

```shell
bin/gbuild -db dbname -f filename  
```

参数含义：

	dbname：数据库名称
	filename：带“.nt”或者".n3"后缀的文件所在的文件路径，包含带".nt"或者".n3"后缀文件的 zip 压缩包文件路径

例如，我们从lubm.nt构建一个名为“lubm.db”的数据库，可以在数据文件夹中找到。

```shell
[root@localhost gStore]$ bin/gbuild -db lubm -f ./data/lubm/lubm.nt 
database path: ./dbhome/lubm.db
...
Save the database info to system database....
...
Add database info success, update num : 3
Build RDF database lubm successfully! Used 2869 ms
```

注意：

- 不能以空的RDF数据集来创建数据库
- 注意不能直接`cd`到`bin`目录下，而要在gStore安装根目录执行gbuild操作



### 可视化工具（gWorkbench)

gWorkbench是gStore的一个可视化管理工具，通过gWorkbench可以连接上gStore并通过数据库管理模块可以创建图数据库，具体内容详见【开发文档】-【可视化工具Workbench】-【查询功能】-【数据库管理】功能。



### HTTP API（ghttp）

gStore提供了ghttp组件作为http api服务组件，用户可以通过向ghttp发送http请求实现相关功能，ghttp中通过`build`请求来构建图数据库，具体内容详见【开发文档】-【常用API】-【ghttp接口说明】



### Socket API（gServer）

gStore提供了gServer组件作为Socket API服务组件，用户可以通过向gServer发送socket请求实现相关功能，gServer中通过`build`请求来构建图数据库，具体内容详见【开发文档】-【常用API】-【gServer接口说明】



<!-- <div STYLE="page-break-after: always;"></div> -->

## 数据库列表

数据库列表功能是获取当前所有可用的数据库列表信息，有如下几种形式

### 命令行模式（gshow)

gshow用于获取所有可用数据库列表信息。

用法：

```shell
bin/gshow
```

示例：

```shell
[root@localhost gStore]$ bin/gshow 
--------------------------------------------
| database | creator |     built_time      |
--------------------------------------------
| system   | root    |                     |
| small    | root    | 2024-08-08 10:24:01 |
| lubm     | root    | 2024-08-08 14:00:22 |
| num      | root    | 2024-08-08 10:31:42 |
| bbug     | root    | 2024-08-08 10:24:01 |
--------------------------------------------
```



### 可视化工具（gWorkbench)

gWorkbench是gStore的一个可视化管理工具，通过gWorkbench可以连接上gStore并通过数据库管理模块可以获取图数据库列表，具体内容详见【开发文档】-【可视化工具Workbench】-【查询功能】-【数据库管理】功能。



### HTTP API（ghttp）

gStore提供了ghttp组件作为http api服务组件，用户可以通过向ghttp发送http请求实现相关功能，ghttp中通过`show`命令实现相关功能，具体内容详见【开发文档】-【常用API】-【ghttp接口说明】



### Socket API（gServer）

gStore提供了gServer组件作为Socket API服务组件，用户可以通过向gServer发送socket请求实现相关功能，gServer中通过`show`请求来展示数据库列表，具体内容详见【开发文档】-【常用API】-【gServer接口说明】



<!-- <div STYLE="page-break-after: always;"></div> -->
## 数据库状态查询

数据库状态查询功能是获取指定数据库的统计信息，有如下几种方式。

### 命令行模式（gmonitor)

gmonitor用于获取指定数据库的统计信息。

用法：

```shell
bin/gmonitor -db db_name
```

参数含义：

	db_name：数据库名称

示例：

```shell
[root@localhost gStore]$ bin/gmonitor -db lubm
---------------------------------------
|     name      |        value        |
---------------------------------------
| database      | lubm                |
| creator       | root                |
| built_time    | 2024-08-08 14:00:22 |
| triple_num    | 99550               |
| entity_num    | 28413               |
| literal_num   | 0                   |
| subject_num   | 16196               |
| predicate_num | 17                  |
| disk_used     | 21 MB               |
---------------------------------------
```



### 可视化工具（gWorkbench)

gWorkbench是gStore的一个可视化管理工具，通过gWorkbench可以连接上gStore并通过数据库管理模块可以查询图数据库，具体内容详见【开发文档】-【可视化工具Workbench】-【查询功能】-【数据库管理】功能。



### HTTP API（ghttp）

gStore提供了ghttp组件作为http api服务组件，用户可以通过向ghttp发送http请求实现相关功能，ghttp中通过`monitor`获取数据库统计信息，具体内容详见【开发文档】-【常用API】-【ghttp接口说明】



<!-- <div STYLE="page-break-after: always;"></div> -->
## 数据库查询

​       数据库查询是gStore最重要的功能之一，gStore支持W3C定义的SPARQL 1.1查询语言，用户可以通过如下几种方式使用gStore数据库查询功能。

### 命令行模式（gquery)

gquery用于使用包含SPARQL查询的文件查询现有数据库。（每个文件包含一个精确的SPARQL语句，SPARQL语句不仅可以进行查询操作，还可以进行增加和删除操作，详细的SPARQL语句使用请参考第八章）

4.6.1.1查询名为db_name的数据库,输入以下命令：

```shell
bin/gquery -db db_name -q query_file 
```

参数含义： 

	db_name: 数据库名称
	query_file：以“.sql”结尾的SPARQL语句存放的文件路径(其他后缀名也可以)

例如，我们执行./data/lubm/lubm_q0.sql中的SPARQL语句查询lubm数据库

查询结果为：

```shell
[root@localhost gStore]$ bin/gquery -db lubm -q ./data/lubm/lubm_q0.sql
------------------------------------------------------------
|                            ?x                            |
------------------------------------------------------------
| <http://www.Department0.University0.edu/FullProfessor0>  |
| <http://www.Department1.University0.edu/FullProfessor0>  |
| <http://www.Department2.University0.edu/FullProfessor0>  |
| <http://www.Department3.University0.edu/FullProfessor0>  |
| <http://www.Department4.University0.edu/FullProfessor0>  |
| <http://www.Department5.University0.edu/FullProfessor0>  |
| <http://www.Department6.University0.edu/FullProfessor0>  |
| <http://www.Department7.University0.edu/FullProfessor0>  |
| <http://www.Department8.University0.edu/FullProfessor0>  |
| <http://www.Department9.University0.edu/FullProfessor0>  |
| <http://www.Department10.University0.edu/FullProfessor0> |
| <http://www.Department11.University0.edu/FullProfessor0> |
| <http://www.Department12.University0.edu/FullProfessor0> |
| <http://www.Department13.University0.edu/FullProfessor0> |
| <http://www.Department14.University0.edu/FullProfessor0> |
------------------------------------------------------------
query database successfully, Used 72 ms
```

4.6.1.2了解gquery的详细使用，可以输入以下命令进行查看：

```shell
bin/gquery --help
```

4.6.1.3进入gquery控制台命令：

```shell
bin/gquery -db dbname
```

程序显示命令提示符（“gsql>”），您可以在此处输入命令

>使用`help`看到所有命令的基本信息

>输入 `quit` 以退出gquery控制台。

>对于`sparql` 命令, 使用`sparql query_file`执行SPARQL查询语句，query_file为存放SPARQL语句的文件路径。当程序完成回答查询时，它会再次显示命令提示符。

我们也以lubm.nt为例。

```shell
(base) [root@iz8vb0u9hafhzz1mn5xcklz gStore]# bin/gquery -db lubm
query is:
select ?x where
{
        ?x      <ub:name>       <FullProfessor0>.
}


------------------------------------------------------------
|                            ?x                            |
------------------------------------------------------------
| <http://www.Department0.University0.edu/FullProfessor0>  |
| <http://www.Department1.University0.edu/FullProfessor0>  |
| <http://www.Department2.University0.edu/FullProfessor0>  |
| <http://www.Department3.University0.edu/FullProfessor0>  |
| <http://www.Department4.University0.edu/FullProfessor0>  |
| <http://www.Department5.University0.edu/FullProfessor0>  |
| <http://www.Department6.University0.edu/FullProfessor0>  |
| <http://www.Department7.University0.edu/FullProfessor0>  |
| <http://www.Department8.University0.edu/FullProfessor0>  |
| <http://www.Department9.University0.edu/FullProfessor0>  |
| <http://www.Department10.University0.edu/FullProfessor0> |
| <http://www.Department11.University0.edu/FullProfessor0> |
| <http://www.Department12.University0.edu/FullProfessor0> |
| <http://www.Department13.University0.edu/FullProfessor0> |
| <http://www.Department14.University0.edu/FullProfessor0> |
------------------------------------------------------------

gsql>help
help - print commands message
quit - quit the console normally
sparql - load query from the second argument

gsql>quit
```

注意：

- 如果没有答案，将打印“[empty result]”，并且在所有结果后面都有一个空行。
- 使用readline lib，因此您可以使用键盘中的箭头键查看命令历史记录，并使用和箭头键移动和修改整个命令。
- 实用程序支持路径完成。（不是内置命令完成）
- 注意不能直接`cd`到`bin`目录下，而要在gStore安装根目录执行`gquery`操作



### 可视化工具（gWorkbench)

gWorkbench是gStore的一个可视化管理工具，通过gWorkbench可以连接上gStore并通过数据库管理模块可以查询图数据库，具体内容详见【开发文档】-【可视化工具Workbench】-【查询功能】-【图数据库查询】功能。



### HTTP API（ghttp）

gStore提供了ghttp组件作为http api服务组件，用户可以通过向ghttp发送http请求实现相关功能，ghttp中通过`query`请求来查询图数据库包括查询、删除、插入，具体内容详见【开发文档】-【常用API】-【ghttp接口说明】



### Socket API（gServer）

gStore提供了gServer组件作为Socket API服务组件，用户可以通过向gServer发送socket请求实现相关功能，gServer中通过`query`请求来查询图数据库包括查询、删除、插入，具体内容详见【开发文档】-【常用API】-【gServer接口说明】



<!-- <div STYLE="page-break-after: always;"></div> -->
## 数据库导出

导出数据库功能可以将数据库导出成.nt文件。有如下三种形式：

### 命令行模式（gexport)

gexport用于导出某个数据库。

用法：

```shell
bin/gexport -db db_name -f path 
```

命令参数：

```
db_name：数据库名称
path：导出到指定文件夹下（如果为空，则默认导出到gStore根目录下）
```

示例：

```shell
[root@localhost gStore]# bin/gexport -db lubm	
gexport...
...
start exporting the database......
...
finish loading
lubm.db exported successfully! Used 187 ms
lubm.db export path: lubm_240808141125.nt
```




### 可视化工具（gWorkbench)

gWorkbench是gStore的一个可视化管理工具，通过gWorkbench可以连接上gStore并通过数据库管理模块可以导出图数据库，具体内容详见【开发文档】-【可视化工具Workbench】-【查询功能】-【数据库管理】功能。



### HTTP API（ghttp）

gStore提供了ghttp组件作为http api服务组件，用户可以通过向ghttp发送http请求实现相关功能，ghttp中通过`export`功能，具体内容详见【开发文档】-【常用API】-【ghttp接口说明】

<!-- <div STYLE="page-break-after: always;"></div> -->
## 数据库删除

删除数据库功能可以删除指定数据库，有如下三种形式

### 命令行模式（gdrop)

gdrop用于删除某个数据库。

用法：

```shell
bin/gdrop -db db_name
```

命令参数：

```
db_name：数据库名称
```

示例：

```shell
[root@localhost gStore]$ bin/drop -db lubm2
Begin to drop database....
...
Database lubm2 dropped successfully! Used 20 ms
```

为了删除数据库，您不应该只是输入`rm -r db_name.db`因为这不会更新名为的内置数据库`system`。相反，你应该输入`bin/gdrop -db db_name`。



### 可视化工具（gWorkbench)

gWorkbench是gStore的一个可视化管理工具，通过gWorkbench可以连接上gStore并通过数据库管理模块可以删除图数据库，具体内容详见【开发文档】-【可视化工具Workbench】-【查询功能】-【数据库管理】功能。



### HTTP API（ghttp）

gStore提供了ghttp组件作为http api服务组件，用户可以通过向ghttp发送http请求实现相关功能，ghttp中通过`drop`命令实现相关功能，具体内容详见【开发文档】-【常用API】-【ghttp接口说明】



### Socket API（gServer）

gStore提供了gServer组件作为Socket API服务组件，用户可以通过向gServer发送socket请求实现相关功能，gServer中通过`drop`请求来删除图数据库，具体内容详见【开发文档】-【常用API】-【gServer接口说明】

<!-- <div STYLE="page-break-after: always;"></div> -->
## 新增数据

插入RDF数据是gStore常规操作，用户可以通过如下几种方式来执行数据插入操作。

### 命令行模式（gadd)--文件

gadd用于将文件中的三元组插入现有数据库。

用法：

```shell
bin/gadd -db db_name -f rdf_triple_file_path
```

参数含义：

	db_name：数据库名称
	rdf_triple_file_path：带".nt"或者".n3"后缀的文件路径,包含带".nt"或者".n3"后缀文件的 zip 压缩包文件路径

示例：

```shell
[bookug@localhost gStore]$ bin/gadd -db lubm -f ./data/lubm/lubm.nt
...
finish loading
after inserted triples num 0,failed num 0,used 2452 ms
```

**注意：**

**1. gadd主要用于RDF文件数据插入**

**2. 不能直接`cd`到`bin`目录下，而要在gStore安装根目录执行`gadd`操作**



### 命令行模式（gquery)---SPARQL语句

SPARQL定义中可以通过`insert data`指令来实现数据插入，基于此原理，用户也可以通过编写SPARQL插入语句，然后使用gStore的`gquery`工具来实现数据插入，其中SPARQL插入语句示例如下：

```SPARQL
insert data {
   <张三> <性别> "男"^^<http://www.w3.org/2001/XMLSchema#String>.
   <张三> <年龄> "28"^^<http://www.w3.org/2001/XMLSchema#Int>.
   <张三> <好友> <李四>.
}
```

通过`{}`可以包含多条RDF数据，注意每条RDF数据都要以`.`结尾

由于可以使用数据库查询功能实现数据插入，因此也同样可以使用如下功能来进行数据插入。



### 可视化工具（gWorkbench)

gWorkbench是gStore的一个可视化管理工具，通过gWorkbench可以连接上gStore并通过数据库查询模块导入新增数据，具体内容详见【开发文档】-【可视化工具Workbench】-【查询功能】-【图数据库查询】功能。



### HTTP API（ghttp）

gStore提供了ghttp组件作为http api服务组件，用户可以通过向ghttp发送http请求实现相关功能，ghttp中通过`query`请求来插入数据以及通过`batchInsert`来批量插入数据，具体内容详见【开发文档】-【常用API】-【ghttp接口说明】



### Socket API（gServer）

gStore提供了gServer组件作为Socket API服务组件，用户可以通过向gServer发送socket请求实现相关功能，gServer中通过`query`请求来插入数据，具体内容详见【开发文档】-【常用API】-【gServer接口说明】



<!-- <div STYLE="page-break-after: always;"></div> -->
## 删除数据

删除RDF数据是gStore常规操作，用户可以通过如下几种方式来执行数据删除操作。

### 命令行模式（gsub）--文件删除

gsub用于从现有数据库中删除文件中的三元组。

用法：

```bash
bin/gsub -db db_name -f rdf_triple_file_path
```

参数含义：

	rdf_triple_file_path：带".nt"或者以“.n3"后缀的所要删除的数据文件路径,包含带".nt"或者".n3"后缀文件的 zip 压缩包文件路径

示例：

```shell
[root@localhost gStore]$ bin/gsub -db lubm -f ./data/lubm/lubm.nt
...
finish loading
after remove, used 2580 ms
```



### 命令行模式（gquery)---SPARQL语句

SPARQL定义中可以通过`delete data`指令来实现数据插入，基于此原理，用户也可以通过编写SPARQL插入语句，然后使用gStore的`gquery`工具来实现数据插入，其中SPARQL插入语句示例如下：

```SPARQL
delete data {
    <张三> <性别> "男"^^<http://www.w3.org/2001/XMLSchema#String>.
    <张三> <年龄> "28"^^<http://www.w3.org/2001/XMLSchema#Int>.
    <张三> <好友> <李四>.
}
```

通过`{}`可以包含多条RDF数据，注意每条RDF数据都要以`.`结尾

另外SPARQL中还可以通过`delete where`语句来实现根据子查询结构删除数据，如下所示。

```SPARQL
delete where
{
   <张三> ?x ?y.
}
```

该语句表示删除`张三`实体的所有信息（包括属性和关系）

由于可以使用数据库查询功能实现数据插入，因此也同样可以使用如下功能来进行数据插入。



### 可视化工具（gWorkbench)

gWorkbench是gStore的一个可视化管理工具，通过gWorkbench可以连接上gStore并通过数据库查询模块通过编写SPARQL语句可以删除数据，具体内容详见【开发文档】-【可视化工具Workbench】-【查询功能】-【图数据库查询】功能。



### HTTP API（ghttp）

gStore提供了ghttp组件作为http api服务组件，用户可以通过向ghttp发送http请求实现相关功能，ghttp中通过`query`请求来删除数据以及通过`batchRemove`来批量删除数据，具体内容详见【开发文档】-【常用API】-【ghttp接口说明】



### Socket API（gServer）

gStore提供了gServer组件作为Socket API服务组件，用户可以通过向gServer发送socket请求实现相关功能，gServer中通过`query`请求来删除数据，具体内容详见【开发文档】-【常用API】-【gServer接口说明】



## 控制台服务

### 启动gconsole

**(1) 登录**

```shell
bin/gconsole [-u <usr_name>]
```

缺失usr_name则会提示输入

```bash
# bin/gconsole 
Enter user name: root
Enter password: 

Gstore Console , an interactive shell based utility to communicate with gstore repositories.
Gstore version: 1.3 Source distribution
Copyright (c) 2016, 2024, pkumod and topgraph and/or its affiliates.

Welcome to the Gstore Console.
Commands end with ;. Cross line input is allowed.
Comment start with #. Redirect (> and >>) is supported.
CTRL+C to quit current command. CTRL+D to exit this console.
Type 'help;' for help. 

gstore> 
```

**(2)帮助**

```shell
bin/gconsole --help
```

```bash
# bin/gconsole --help
Gstore Ver 1.3 for Linux on x86_64 (Source distribution)
Gstore Console(gconsole), an interactive shell based utility to communicate with gstore repositories.
Copyright (c) 2016, 2024, pkumod and topgraph and/or its affiliates.

Usage: bin/gconsole [OPTIONS]
  -?, --help          Display this help and exit.
  -u, --user          username. 
  
Supported command in gconsole: Type "?" or "help" in the console to see info of all commands.
```

**(3)命令**

- **以`;`结尾，可以跨行**
- **支持`>`和`>>`重定向输出**
- **支持单行注释，`#`开始直到换行为注释内容**
- **支持命令补全、文件名补全、行编辑、历史命令**
- **`ctrl+C`放弃当前命令（结束执行或放弃编辑），`ctrl+D`退出命令行**
- 进入gconsole需登录；符合当前[用户权限](# 权限说明)的命令才运行执行，以及仅显示符合当前用户权限的内容
  - 7种权限：`query`，`load`，`unload`，`update`，`backup`，`restore`，`export` 
- 命令关键词不区分大小写，命令行option、数据库名、用户名、密码等区分大小写。**命令关键词如下**：
  - 数据库操作： `sparql`，`create`，`use`，`drop`，`show`，`showdbs`，`backup`，`restore`，`export`，`pdb`
  - 身份：`flushpriv`，`pusr`，`setpswd`
  - 查看配置信息：`settings`，`version`
  - 权限管理：`setpriv`，`showusrs`，`pusr`
  - 用户管理：`addusr`，`delusr`，`setpswd`，`showusrs`
  - 帮助和其他：`quit`，`help/?`，`pwd`，`clear`

### 数据库操作

**(1)创建数据库**

```shell
create <database_name> [<nt_file_path>]
```

- 从nt_file_path读取`.nt`文件,或包含`.nt`文件的`.zip`压缩包并创建数据库database_name，或者创建空数据库，当前用户被赋予对database_name的[全部权限](# 权限说明)
- \<database_name>：数据库名不能是`system`
- \<nt_file_path>：文件路径

```bash
gstore> create example friend.nt；
... (this is build database process output, omitted in this document)
Add database info success.
Build RDF database example successfully!
```

**(2)删除数据库**

```bash
drop <database_name>
```

- 不能删除当前数据库
- \<database_name>：数据库名称，仅能指定为当前用户拥有[全部权限](# 权限说明)的数据库

```bash
gstore> drop <database_name>;
```

**(3)指定/切换当前数据库**

```
use <database_name>
```

- 指定/切换当前数据库，把之前的当前数据库从内存卸载（如果有的化）并把指定的当前数据库加载到内存

- 目标数据库需要已经创建

- \<database_name>：数据库名称，仅能指定为当前用户拥有[load和unload权限](# 权限说明)的数据库

```bash
gstore> use example;
... (this is load process output, omitted in this document)
Current database switch to mytest successfully.
```

**(4)查询/更新当前数据库**

- 在当前数据库上进行sparql查询
- 需要先`use <database_name>`指定当前数据库

**(5)直接输入SPARQL语句**

```bash
gstore> # show all in db
     -> SELECT ?x ?y ?z 
     -> WHERE{
     ->     ?x ?y ?z. # comment
     -> } ;
... (this is query process output, omitted in this document)
---------------------------------------
|    ?x     |     ?y      |    ?z     |
---------------------------------------
| <Alice>   | <关注>      | <Bob>     |
| <Bob>     | <关注>      | <Alice>   |
| <Carol>   | <关注>      | <Bob>     |
| <Dave>    | <关注>      | <Alice>   |
| <Dave>    | <关注>      | <Eve>     |
| <Alice>   | <喜欢>      | <Bob>     |
| <Bob>     | <喜欢>      | <Eve>     |
| <Eve>     | <喜欢>      | <Carol>   |
| <Carol>   | <喜欢>      | <Bob>     |
| <Francis> | <喜欢>      | <Carol>   |
| <Alice>   | <不喜欢>    | <Eve>     |
| <Carol>   | <不喜欢>    | <Francis> |
| <Francis> | <不喜欢>    | <Eve>     |
| <Francis> | <不喜欢>    | <Dave>    |
| <Dave>    | <不喜欢>    | <Francis> |
---------------------------------------
query database successfully, Used 127 ms

```

支持重定向举例（所有命令均支持重定向输出）

```bash
gstore> # support redirect
     -> SELECT ?x ?y ?z 
     -> WHERE{
     ->     ?x ?y ?z.
     -> } > my_test_data/output ;
Redirect output to file: my_test_data/output
```

**(6)输入SPARQL语句文件**

格式：

```shell
sparql sparql_file
## sparql_file是sparql语句文件，可以是相对路径，也可以是绝对路径
```

- 指定sparql文件，文件中包含多条sparql语句需要以`;`间隔，最后`;`可有可不有

- 文件内容支持单行注释，`#`开头

```shell
gstore> sparql all.sparql ;
... (this is query process output, omitted in this document)
----------------------------------
|    ?s     |    ?p    |   ?o    |
----------------------------------
| <Alice>   | <关注>   | <Bob>   |
| <Bob>     | <关注>   | <Alice> |
| <Carol>   | <关注>   | <Bob>   |
| <Dave>    | <关注>   | <Alice> |
| <Dave>    | <关注>   | <Eve>   |
| <Alice>   | <喜欢>   | <Bob>   |
| <Bob>     | <喜欢>   | <Eve>   |
| <Eve>     | <喜欢>   | <Carol> |
| <Carol>   | <喜欢>   | <Bob>   |
| <Francis> | <喜欢>   | <Carol> |
----------------------------------
query database successfully, Used 121 ms
```

- query.sparql内容举例：

```SPARQL
SELECT	?x ?y ?z
WHERE {
    ?x ?y ?z.
}
```

**(7)显示数据库信息**

```shell
show [<database_name>] [-n <displayed_triple_num>]
```

- 显示meta信息和前displayed_triple_num行三元组（默认显示前10行三元组）
- \<database_name>：数据库名称，不指定database_name则显示当前数据库的信息
- -n <displayed_triple_num>：显示行数

```bash
gstore> show example;
... (this is load and query process output, omitted in this document)
---------------------------------------
|     name      |        value        |
---------------------------------------
| database      | example             |
| creator       | root                |
| built_time    | 2024-08-06 17:23:47 |
| triple_num    | 15                  |
| entity_num    | 6                   |
| literal_num   | 0                   |
| subject_num   | 6                   |
| predicate_num | 3                   |
---------------------------------------
----------------------------------
|    ?s     |    ?p    |   ?o    |
----------------------------------
| <Alice>   | <关注>   | <Bob>   |
| <Bob>     | <关注>   | <Alice> |
| <Carol>   | <关注>   | <Bob>   |
| <Dave>    | <关注>   | <Alice> |
| <Dave>    | <关注>   | <Eve>   |
| <Alice>   | <喜欢>   | <Bob>   |
| <Bob>     | <喜欢>   | <Eve>   |
| <Eve>     | <喜欢>   | <Carol> |
| <Carol>   | <喜欢>   | <Bob>   |
| <Francis> | <喜欢>   | <Carol> |
----------------------------------
```

**(8)查看所有数据库**

```shell
showdbs
```

- 仅显示当前用户有查询权限的数据库

```bash
gstore> showdbs;
--------------------------------------
| database | creater |    status     |
--------------------------------------
| system   | root    |               |
| test1    | root    | already_built |
| test     | root    | already_built |
| test2    | root    | already_built |
| test3    | root    | already_built |
| example  | root    | already_built |
--------------------------------------
```

**(9)显示当前数据库名**

```shell
pdb
```

```bash
gstore> pdb;
example
```

**(10)备份当前数据库**

```shell
backup [<backup_folder>]
```

- \<backup_folder>指定备份的路径

```bash
gstore> backup;
... (this is backup process output, omitted in this document)
Database example backup successfully.
```

```bash
gstore> backup back;
... (this is backup process output, omitted in this document)
Database example backup successfully.
```

**(11)导出当前数据库**

```
export <file_path>
```

- \<file_path>：指定导出的路径

```bash
gstore> export example.nt;
Database example export successfully.
```

**(11)恢复数据库**

```shell
restore <database_name> <backup_path>
```

- \<database_name>：数据库名称
- \<backup_path>：备份文件路径

```bash
gstore> restore example backups/example.db_220929114732/
... (this is restore process output, omitted in this document)
Database example restore successfully.
```

### 身份

**(1)刷新权限**

```shell
flushpriv
```

- 读取db刷新当前用户权限

```bash
gstore> flushpriv;
Privilige Flushed for current user successfully.
```

**(2)显示当前用户名和权限**

```shell
pusr [<database_name>]
```

- `pusr `显示当前用户名
- `pusr <database_name>`显示当前用户名和当前用户在<database_name>上的权限

```bash
gstore> pusr;
usrname: yuanzhiqiu

gstore> pusr example;
usrname: yuanzhiqiu
privilege on example: query load unload update backup restore export 
```

**(3)修改当前用户密码**

```
setpswd
```

- 需要输入密码验证身份

```bash
gstore> setpswd;
Enter old password: 
Enter new password: 
Enter new password again: 
Not Matched.
Enter new password: 
Enter new password again: 
Password set successfully.
```

### 查看配置信息

**(1)查看配置**

```shell
settings [<conf_name>]
```

- settings显示所有配置信息

```bash
gstore> settings;
Settings: 
thread_num      30
... (this is other settings, omitted in this document)
You can Edit configuration file to change settings: ./conf/conf.ini
```

- \<conf_name>：显示指定名称conf_name的配置信息

```bash
gstore> settings ip_deny_path;
"./conf/ipDeny.config"
You can Edit configuration file to change settings: ./conf/conf.ini
```

**(2)查看版本**

```shell
version
```

- 输出当前本版信息

```bash
gstore> version;
Gstore version: 1.3 Source distribution
Copyright (c) 2016, 2024, pkumod and topgraph and/or its affiliates.
```

**(3)权限管理**

权限管理相关命令只有系统用户有权限执行，权限说明如下：

- 权限：某用户对某数据库所拥有的权限

- 7种权限：`query`，`load`，`unload`，`update`，`backup`，`restore`，`export`

  ```
  [1]query [2]load [3]unload [4]update [5]backup [6]restore [7]export
  ```

  7种权限都有称为拥有全部权限

- 系统用户拥有对全部数据库的全部权限（系统用户名在conf.ini和system.db中定义）

**(4)设置用户权限**

```
setpriv <usrname> <database_name>
```

- 需要输入密码验证root身份
- \<usrname>：被设置的用户名
- \<database_name>：数据库名称

```bash
gstore> setpriv zero example;
Enter your password: 
[1]query [2]load [3]unload [4]update [5]backup [6]restore [7]export [8]all
Enter privilege number to assign separated by whitespace: 
1 2 3
[will set priv:]00001110
Privilege set successfully.
```

**(5)查看用户权限**

```shell
pusr <database_name> <usrname>
```

- \<database_name>：数据库名称
- \<usrname>：用户名

```bash
gstore> pusr example yuanzhiqiu;
privilege on example: query load unload update backup restore export 
```

**(6)查看所有用户和权限**

```shell
showusrs
```

显示有哪些用户，并显示每个用户的数据库权限（仅显示非root用户的；仅显示该用户有has_xxx_priv的数据库）

格式为：

```
user
------------------------
privilege on databases
```

```bash
----------------------------------------------------------
| user |                    privilege                    |
----------------------------------------------------------
| root | all privilege on all db                         |
| uki  | unload: query load unload update backup export  |
----------------------------------------------------------

```

### 用户管理

用户管理相关命令只有系统用户有权限执行

**(1)添加用户**

```shell
addusr <usrname>
```

- 需要输入密码验证root身份
- \<usrname>：新用户名

```bash
gstore> addusr uki;
Enter your password: 
Enter password for new user: hello
Add usr uki successfully.
```

**(2)删除用户**

```shell
delusr <usrname>
```

- 需要输入密码验证root身份
- \<usrname>：待删除的用户名

```bash
gstore> delusr cat;
Enter your password: 
Del usr cat successfully.
```

**(3)重置密码**

```
setpswd <usrname>
```

- 需要输入密码验证root身份
- \<usrname>：用户名

```bash
gstore> setpswd zero;
Enter your password: 
Enter new password: 
Enter new password again: 
Password set successfully.
```

**(4)查看所有用户**

```shell
showusrs
```

```bash
----------------------------------------------------------
| user |                    privilege                    |
----------------------------------------------------------
| root | all privilege on all db                         |
| uki  | unload: query load unload update backup export  |
----------------------------------------------------------
```



### 帮助和其他

**(1)结束/取消当前命令**

`ctrl+C`

结束当前正在执行的命令

```bash
...(executing some cmd) (ctrl+C here)

gstore>
```

或放弃当前在输入的命令

```bash
gstore> SELECT ?x ?y ?z
     -> WHERE{ (ctrl+C here)
gstore> 
```

**(2)退出**

`quit`/`ctrl+D`

如果当前数据库没有卸载则会从内存中卸载

```bash
gstore> quit;

[xxx@xxx xxx]#
```

```bash
gstore> (ctrl+D here)

[xxx@xxx xxx]#
```

**(3)帮助**

```
help [edit/usage/<command>]
```

- `help`显示所有命令帮助信息

```bash
gstore> help;
Gstore Console(gconsole), an interactive shell based utility to communicate with gStore repositories.

For information about gStore products and services, visit:
   http://www.gstore.cn/
For developer information, including the gStore Reference Manual, visit:
   http://www.gstore.cn/pcsite/index.html#/documentation

Commands end with ;. Cross line input is allowed.
Comment start with #.
Type 'cancel;' to quit the current input statement.

List of all gconsole commands:
sparql          Answer SPARQL query(s) in file.
create          Build a database from a dataset or create 
...(this is help msg for other commands, omitted in this document)

Other help arg:
edit    Display line editing shortcut keys supported by gconsole.
usage   Display all commands as well as their usage.
```

- `help edit`显示GNU readline的行编辑快捷键操作

```bash
gstore> help edit;
Frequently used GNU Readline shortcuts:
CTRL-a  move cursor to the beginning of line
CTRL-e  move cursor to the end of line
CTRL-d  delete a character
CTRL-f  move cursor forward (right arrow)
CTRL-b  move cursor backward (left arrow)
CTRL-p  previous line, previous command in history (up arrow)
CTRL-n  next line, next command in history (down arrow)
CTRL-k  kill the line after the cursor, add to clipboard
CTRL-u  kill the line before the cursor, add to clipboard
CTRL-y  paste from the clipboard
ALT-b   move cursor back one word
ALT-f   move cursor forward one word
For more about GNU Readline shortcuts, see https://en.wikipedia.org/wiki/GNU_Readline#Emacs_keyboard_shortcuts
```

- `help usage`显示所有命令的描述和usage

```bash
gstore> help usage;
List of all gconsole commands:
Note that all text commands must be end with ';' but need not be in one line.
sparql          Answer SPARQL query(s) in file.
                sparql <; separated SPARQL file>;
create          Build a database from a dataset or create an empty database.
                create <database_name> [<nt_file_path>];
...(this is help msg for other commands, omitted in this document)
```

- `help <command>`显示<command>的帮助信息

```bash
gstore> help use;
use    Set current database.
       use <database_name>;
```

**(4)清屏**

```bash
gstore> clear;
```

**(5)查看当前工作路径**

```bash
gstore> pwd;
/<path_to_gstore>/gstore
```



## HTTP API服务

gStore提供了两套http API服务，分别是grpc和ghttp，用户通过向API服务发送http请求，可以实现对gStore的远程连接和远程操作 



### 开启http api服务

Store编译后，在gStore的bin目录下会有一个grpc服务和ghttp服务，默认不启动，需要用户手动启动http服务，后台运行启动命令如下：

```shell
nohup bin/grpc -db db_name -p serverPort -c enable &
```

```shell
nohup bin/ghttp -db db_name -p serverPort -c enable &
```

参数说明：

```
db_name: 要启动ghttp的数据库名称（可选项，该参数主要作用在于，启动时将把该数据库相关信息load到内存中）

serverPort: http监听端口，需保证该端口不会被服务器防火墙禁止（可选项，如果不填则默认端口号为9000）

enable: 是否加载CSR资源，0为否，1为是（可选项，用于内置高级查询函数及用户自定义图分析算子函数。默认值为0）
```

`grpc`和 `ghttp` 都支持GET和POST请求类型。

同时支持并发只读查询，但是当包含更新的查询到来时，整个数据库将被锁定。在具有数十个内核线程的计算机上，建议并发运行查询的数量低于300，但我们可以在实验中同时运行13000个查询。要使用并发功能，最好将“打开文件”和“最大进程”的系统设置修改为65535或更大。

**如果发送包含更新的查询, 您最好经常向控制台发送`checkpoint` 命令； 否则，更新可能无法与磁盘同步，并且如果  服务器异常停止则会丢失（例如，键入“Ctrl + C”）**



### 关闭http api服务

关闭服务请使用以下指令进行关闭，最好不要只需输入命令`Ctrl + C`或`kill`来停止，因为这不安全。

```shell
bin/shutdown
```



### HTTP API接口 

ghttp 和 gRPC 提供了丰富的 API 接口，以便用户可以远程操作 gStore 大部分功能，具体接口详见【开发文档】-【常用API】-【gRPC接口说明】和【gHttp接口说明】。

<!-- <div STYLE="page-break-after: always;"></div> -->
## Socket API服务

gServer是gStore提供的外部访问接口，是一个socket API服务，用户通过socket双向通信，可以实现对gStore的远程连接和远程操作 。

### 开启gServer服务

gStore编译后，在gStore的bin目录下会有一个gServer服务，但该服务默认不启动，需要用户手动启动gServer服务，启动命令如下：

```shell
bin/gserver -s
```

其他可选参数说明：

```
-t,--stop: 关闭gserver服务；
-r,--restart：重启gserver服务；
-p,--port：修改socket连接端口配置，默认端口为9000，修改后需要重启gserver服务
-P,--printport：打印当前socket连接端口配置
-d,--debug：启动debug模式（保持gserver服务在前台运行）
-k,--kill：强制关闭服务，建议仅在无法正常关闭服务时使用
```



### 关闭gServer服务

关闭gServer服务请使用以下指令进行关闭，最好不要只需输入命令`Ctrl + C`或`kill`来停止gServer，因为这不安全。

```shell
bin/gserver -t
```



### gServer相关API

gServer提供了丰富的API接口以便用户可以远程操作gStore大部分功能，具体接口详见【开发文档】-【常用API】-【gServer 接口说明】



<div STYLE="page-break-after: always;"></div>
