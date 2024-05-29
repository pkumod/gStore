## 图模式 (Graph Patterns)

本文档主要参考了 [SPARQL 1.1 标准文档](https://www.w3.org/TR/sparql11-query/)，同时也增加了 gStore 自身定制化的内容，如果想要详细了解 gStore 支持的 SPARQL 语句，请仔细阅读我们的文档吧！

除特殊说明处，本文档将持续使用以下的 RDF 数据实例作为查询的对象：

```
<刘亦菲> <姓名> "刘亦菲" .
<刘亦菲> <姓名> "Crystal Liu" .
<刘亦菲> <性别> "女" .
<刘亦菲> <星座> "处女座" .
<刘亦菲> <职业> "演员" .

<林志颖> <姓名> "林志颖" .
<林志颖> <性别> "男" .
<林志颖> <职业> "演员" .
<林志颖> <职业> "导演" .

<胡军> <姓名> "胡军" .
<胡军> <性别> "男" .
<胡军> <星座> "双鱼座" .
<胡军> <职业> "演员" .
<胡军> <职业> "配音" .
<胡军> <职业> "制片" .
<胡军> <职业> "导演" .

<天龙八部> <主演> <林志颖> .
<天龙八部> <主演> <刘亦菲> .
<天龙八部> <主演> <胡军> .
<天龙八部> <类型> <武侠片> .
<天龙八部> <类型> <古装片> .
<天龙八部> <类型> <爱情片> .
<天龙八部> <豆瓣评分> "8.3"^^<http://www.w3.org/2001/XMLSchema#float> .
<天龙八部> <上映时间> "2003-12-11T00:00:00"^^<http://www.w3.org/2001/XMLSchema#dateTime> .

<恋爱大赢家> <主演> <林志颖> .
<恋爱大赢家> <主演> <刘亦菲> .
<恋爱大赢家> <类型> <爱情片> .
<恋爱大赢家> <类型> <剧情片> .
<恋爱大赢家> <豆瓣评分> "6.1"^^<http://www.w3.org/2001/XMLSchema#float> .
<恋爱大赢家> <上映时间> "2004-11-30T00:00:00"^^<http://www.w3.org/2001/XMLSchema#dateTime> .
```

由于 SPARQL 1.1 标准文档暂无官方中文译本，因此下文中术语首次出现时将注明其英文原文。

按照标准，SPARQL查询中的**关键词均不区分大小写**。

<br/>

###  最简单的图模式

我们先给出一个最简单的查询：

```sparql
SELECT ?movie
WHERE
{
	?movie <主演> <刘亦菲> .
}
```

查询由两部分组成：**SELECT 语句**指定需要输出查询结果的变量，**WHERE 语句**提供用来与数据图匹配的图模式。上面的查询中，图模式由单条**三元组** `?movie <主演> <刘亦菲>` 构成，其中作为主语的 `?movie` 是**变量**，作为谓词的 `<主演>` 和作为宾语的 `<刘亦菲>` 是 **IRI** (International Resource Identifier, 国际资源标识符)。这个查询将返回由刘亦菲主演的所有影视作品，在示例数据上运行结果如下：

| ?movie        |
| ------------- |
| \<天龙八部>   |
| \<恋爱大赢家> |

三元组的主语、谓词、宾语都可以是 IRI ；宾语还可以是 **RDF 字面量（RDF Literal）**。以下查询将给出示例数据中所有职业为导演的人物：

```sparql
SELECT ?person
WHERE
{
	?person <职业> "导演" .
}
```

其中 `"导演"` 是一个 RDF 字面量。

结果如下：

| ?person   |
| --------- |
| \<胡军>   |
| \<林志颖> |

当前 gStore 版本下，带有数据类型的 RDF 字面量在查询中需要添加与数据文件中相应的后缀。例如，以下查询将给出豆瓣评分为 8.3 的影视作品：

```sparql
SELECT ?movie
WHERE
{
	?movie <豆瓣评分> "8.3"^^<http://www.w3.org/2001/XMLSchema#float> .
}
```

结果如下：

| ?movie      |
| ----------- |
| \<天龙八部> |

其他的常见数据类型包括 `<http://www.w3.org/2001/XMLSchema#integer>`（整数类型），`<http://www.w3.org/2001/XMLSchema#decimal>`（定点类型），`xsd:double`（双精度浮点类型），`<http://www.w3.org/2001/XMLSchema#string>`（字符串类型），`<http://www.w3.org/2001/XMLSchema#boolean>`（布尔类型），`<http://www.w3.org/2001/XMLSchema#dateTime>`（日期时间类型）。数据文件中也可能出现其他数据类型，只需在查询中使用 `^^<数据类型后缀>` 的形式即可。

### 基本图模式 (Basic Graph Pattern)

**基本图模式**即为三元组的集合；上一节中的两个查询的 `WHERE` 语句均只有最外层大括号，因此属于**基本图模式**；加上最外层大括号，即为由单个基本图模式构成的**组图模式（Group Graph Pattern）**。

上一节的两个查询中基本图模式都由单个三元组构成。以下查询使用了由多个三元组构成的基本图模式，将给出示例数据中天龙八部的所有男性主演：

```sparql
SELECT ?person
WHERE
{
	<天龙八部> <主演> ?person .
	?person <性别> "男" .
}
```

结果如下：

| ?person   |
| --------- |
| \<胡军>   |
| \<林志颖> |

###  组图模式 (Group Graph Pattern)

**组图模式**以配对的大括号分隔。组图模式既可以像上一节介绍的那样由单个基本图模式构成，也可以由多个子组图模式和以下的 **OPTIONAL**, **UNION**, **MINUS** 三种运算嵌套而成。**FILTER** 则在一个组图模式的范围内过滤结果。

**OPTIONAL**

关键词 OPTIONAL 使用的语法如下：

```
pattern1 OPTIONAL { pattern2 }
```

查询结果必须匹配 `pattern1` ，并选择性地匹配 `pattern2` 。`pattern2` 被称为 OPTIONAL 图模式。如果 `pattern2` 存在匹配，则将其加入 `pattern1` 的匹配结果；否则，仍然输出 `pattern1` 的匹配结果。因此，OPTIONAL 常用于应对部分数据缺失的情况。

下面的查询给出示例数据中人物的性别和星座信息。其中，只要存在性别信息的人物都会被返回，不论是否同时存在该人物的星座信息；若同时存在，则额外返回。

```sparql
SELECT ?person ?gender ?horoscope
WHERE
{
	?person <性别> ?gender .
	OPTIONAL
	{
		?person <星座> ?horoscope .
	}
}
```

结果如下：

| ?person   | ?gender | ?horoscope |
| --------- | ------- | ---------- |
| \<刘亦菲> | "女"    | "处女座"   |
| \<林志颖> | "男"    |            |
| \<胡军>   | "男"    | "双鱼座"   |

**UNION**

关键词 UNION 的使用语法与 OPTIONAL 类似。以 UNION 相连的图模式中，只要存在一个与某数据匹配，该数据就与以 UNION 相连的整体匹配。因此，UNION 可以理解为对它所连接的各图模式的匹配结果集合求并集（由于允许重复结果，实际上采用多重集语义）。

下面的查询给出示例数据中类别是古装片或剧情片的影视作品：

```sparql
SELECT ?movie
WHERE
{
	{?movie <类型> <古装片> .}
	UNION
	{?movie <类型> <剧情片> .}
}
```

结果如下：

| ?movie        |
| ------------- |
| \<天龙八部>   |
| \<恋爱大赢家> |

**MINUS**

关键词 MINUS 的使用语法与 OPTIONAL, UNION 类似。MINUS 左边和右边的图模式的匹配均会被计算，从左边的图模式的匹配结果中移除能与右边的图模式匹配的部分作为最终结果。因此，MINUS 可以理解为对它所连接的两个图模式的匹配结果集合求差（左为被减集合，多重集语义）。

下面的查询将给出示例数据中主演了天龙八部但没有主演恋爱大赢家的人物：

```sparql
SELECT ?person
WHERE
{
	<天龙八部> <主演> ?person .
	MINUS
	{<恋爱大赢家> <主演> ?person .}
}
```

结果如下：

| ?person |
| ------- |
| \<胡军> |

**FILTER**

关键词 FILTER 之后紧随着一个约束条件，当前组图模式中不满足此条件的结果将被过滤掉，不被返回。FILTER 条件中可以使用等式、不等式以及各种内建函数。

下面的查询将给出示例数据中豆瓣评分高于 8 分的影视作品：

```sparql
SELECT ?movie
WHERE
{
	?movie <豆瓣评分> ?score .
	FILTER (?score > "8"^^<http://www.w3.org/2001/XMLSchema#float>)
}
```

结果如下：

| ?movie      |
| ----------- |
| \<天龙八部> |

无论 FILTER 放置在一个组图模式中的什么位置，只要仍然处于同一个嵌套层，则其语义不变，约束条件的作用范围仍然是当前组图模式。比如以下的查询就与前一个查询等价：

```sparql
SELECT ?movie
WHERE
{
	FILTER (?score > "8"^^<http://www.w3.org/2001/XMLSchema#float>)
	?movie <豆瓣评分> ?score .
}
```

常用于 FILTER 条件的一个内建函数是正则表达式 **REGEX** 。下面的查询将给出示例数据中的刘姓人物：

```sparql
SELECT ?person
WHERE
{
	?person <姓名> ?name .
	FILTER REGEX(?name, "刘.*")
}
```

结果如下：

| ?person   |
| --------- |
| \<刘亦菲> |

<br/>



## 赋值（Assignment）

以下关键词属于赋值函数，可在查询体内进行变量定义或提供内联数据 。

### BIND: 绑定变量

```
BIND(value, name)
```

**参数**

`value`: string类型的字符串值

`name`: 自定义的参数名称

**示例：**

查询刘亦菲或胡军的职业，并在返回的结果中分类标记：

```SPARQL
SELECT ?info ?work WHERE
{
   {
       BIND("刘亦菲" as ?info).
       <刘亦菲> <职业> ?work .
   }
   UNION
   {
       BIND("胡军" as ?info).
       <胡军> <职业> ?work.
   }
}
```

最终的结果输出如下（为方便阅读，省略了字符串最外层的双引号和内部双引号转义）：

```json
{
	"bindings": [
        {
            "info": {"type": "literal","value": "刘亦菲"},
            "work": {"type": "literal","value": "演员"}
        },
        {
            "info": {"type": "literal","value": "胡军"},
            "work": {"type": "literal","value": "演员"}
        },
        {
            "info": {"type": "literal","value": "胡军"},
            "work": {"type": "literal","value": "导演"}
        },
        {
            "info": {"type": "literal","value": "胡军"},
            "work": {"type": "literal","value": "配音"}
        },
        {
            "info": {"type": "literal","value": "胡军"},
            "work": {"type": "literal","value": "制片"}
        }
    ]
}
```

后续还会进一步完善BIND表达式/函数，比如支持实体对象的赋值绑定等。

### CONCAT: 拼接多个字符

```
CONCAT(val_1, val_2,...val_n)
```

**参数**

`val_i`: string类型的字符串值

**示例：**

将查询到的人物姓名、性别、工作连接在一起输出：

```SPARQL
SELECT (CONCAT(?name, ",", ?gender, ",", ?work) as ?info) WHERE
{
        ?s <姓名> ?name.
        ?s <性别> ?gender.
        ?s <职业> ?work.
}
```

最终的结果输出如下（为方便阅读，省略了字符串最外层的双引号和内部双引号转义）：

```json
{
    "bindings": [
        {
            "info": {"type": "literal","value": "刘亦菲,女,演员"}
        },
        {
            "info": {"type": "literal","value": "Crystal Liu,女,演员"}
        },
        {
            "info": {"type": "literal","value": "林志颖,男,演员"}
        },
        {
            "info": {"type": "literal","value": "林志颖,男,导演"}
        },
        {
            "info": {"type": "literal","value": "胡军,男,演员"}
        },
        {
            "info": {"type": "literal","value": "胡军,男,导演"}
        },
        {
            "info": {"type": "literal","value": "胡军,男,配音"}
        },
        {
            "info": {"type": "literal","value": "胡军,男,制片"}
        }
    ]
}
```



## 聚合函数 (Aggregates)

聚合函数用在 SELECT 语句中，语法如下：

```sparql
SELECT (AGGREGATE_NAME(?x) AS ?y)
WHERE
{
	...
}
```

其中，`AGGREGATE_NAME` 是聚合函数的名称，变量 `?x` 是聚合函数作用的对象，变量 `?y` 是最终结果中聚合函数值的列名。

聚合函数作用于各组结果。返回的全部结果默认作为一组。gStore支持的聚合函数如下所示：

**COUNT**

用于计数的聚合函数。

下面的查询将给出示例数据中职业为演员的人物的数目：

```sparql
SELECT (COUNT(?person) AS ?count_person)
WHERE
{
	?person <职业> "演员" .
}
```

结果如下：

| ?count_person                                    |
| ------------------------------------------------ |
| "3"^^\<http://www.w3.org/2001/XMLSchema#integer> |

**SUM**

用于求和的聚合函数。

下面的查询将给出示例数据中所有电影的豆瓣评分之和：

```sparql
SELECT (SUM(?score) AS ?sum_score)
WHERE
{
	?movie <豆瓣评分> ?score .
}
```

结果如下：

| ?sum_score                                            |
| ----------------------------------------------------- |
| "14.400000"^^<http://www.w3.org/2001/XMLSchema#float> |

**AVG**

用于求平均值的聚合函数。

下面的查询将给出示例数据中所有电影的平均豆瓣评分：

```sparql
SELECT (AVG(?score) AS ?avg_score)
WHERE
{
	?movie <豆瓣评分> ?score .
}
```

结果如下：

| ?avg_score                                           |
| ---------------------------------------------------- |
| "7.200000"^^<http://www.w3.org/2001/XMLSchema#float> |

**MIN**

用于求最小值的聚合函数。

下面的查询将给出示例数据中所有电影的最低豆瓣评分：

```sparql
SELECT (MIN(?score) AS ?min_score)
WHERE
{
	?movie <豆瓣评分> ?score .
}
```

结果如下：

| ?min_score                                      |
| ----------------------------------------------- |
| "6.1"^^<http://www.w3.org/2001/XMLSchema#float> |

**MAX**

用于求最大值的聚合函数。

下面的查询将给出示例数据中所有电影的最高豆瓣评分：

```sparql
SELECT (MAX(?score) AS ?max_score)
WHERE
{
	?movie <豆瓣评分> ?score .
}
```

结果如下：

| ?max_score                                      |
| ----------------------------------------------- |
| "8.3"^^<http://www.w3.org/2001/XMLSchema#float> |

**GROUP BY**

如果希望按照某一个变量的值对结果分组，可以使用关键词 GROUP BY 。例如，下面的查询将给出示例数据中的所有职业及对应的人数：

```sparql
SELECT ?occupation (COUNT(?person) AS ?count_person)
WHERE
{
	?person <职业> ?occupation .
}
GROUP BY ?occupation
```

结果如下：

| ?occupation | ?count_person                                    |
| ----------- | ------------------------------------------------ |
| "演员"      | "3"^^\<http://www.w3.org/2001/XMLSchema#integer> |
| "导演"      | "2"^^\<http://www.w3.org/2001/XMLSchema#integer> |
| "配音"      | "1"^^\<http://www.w3.org/2001/XMLSchema#integer> |
| "制片"      | "1"^^\<http://www.w3.org/2001/XMLSchema#integer> |

<br/>



## 结果序列修饰符 (Solution Sequences and Modifiers)

以下的关键词均属于结果序列修饰符，它们对查询结果做后处理，以形成最终返回的结果。

**DISTINCT: 去除重复结果**

SELECT 语句不带关键词 DISTINCT 的查询会在最终结果中保留重复的结果。例如下面的查询给出示例数据中所有的职业：

```sparql
SELECT ?occupation
WHERE
{
	?person <职业> ?occupation .
}
```

结果如下：

| ?occupation |
| ----------- |
| "演员"      |
| "演员"      |
| "演员"      |
| "导演"      |
| "导演"      |
| "制片"      |
| "配音"      |

如果希望查看不重复的职业种类，则可以在 SELECT 语句中添加关键词 DISTINCT ：

```sparql
SELECT DISTINCT ?occupation
WHERE
{
	?person <职业> ?occupation .
}
```

结果如下：

| ?occupation |
| ----------- |
| "演员"      |
| "导演"      |
| "制片"      |
| "配音"      |

DISTINCT 也可以在聚合函数 COUNT 中使用。下面的查询给出示例数据中的职业种类数目：

```sparql
SELECT (COUNT(DISTINCT ?occupation) AS ?count_occupation)
WHERE
{
	?person <职业> ?occupation .
}
```

结果如下：

| ?count_occupation                                |
| ------------------------------------------------ |
| "4"^^\<http://www.w3.org/2001/XMLSchema#integer> |

**ORDER BY: 排序**

查询结果默认是无序的。如果希望根据某些变量的值对结果进行排序，可以在 WHERE 语句后面添加 ORDER BY 语句。例如下面的查询将示例数据中的影视作品按照豆瓣评分排序，未指定顺序时默认为升序：

```sparql
SELECT ?movie ?score
WHERE
{
	?movie <豆瓣评分> ?score
}
ORDER BY ?score
```

结果如下：

| ?movie        | ?score                                           |
| ------------- | ------------------------------------------------ |
| \<恋爱大赢家> | "6.1"^^\<http://www.w3.org/2001/XMLSchema#float> |
| \<天龙八部>   | "8.3"^^\<http://www.w3.org/2001/XMLSchema#float> |

如果希望降序排序，需要用关键词 DESC 修饰变量名：

```sparql
SELECT ?movie ?score
WHERE
{
	?movie <豆瓣评分> ?score
}
ORDER BY DESC(?score)
```

结果如下：

| ?movie        | ?score                                           |
| ------------- | ------------------------------------------------ |
| \<天龙八部>   | "8.3"^^\<http://www.w3.org/2001/XMLSchema#float> |
| \<恋爱大赢家> | "6.1"^^\<http://www.w3.org/2001/XMLSchema#float> |

ORDER BY 语句可以包含多个以空格分隔的变量，每个变量都可用 DESC 修饰。gStore 暂不支持在 ORDER BY 语句中使用含四则运算的表达式及内建函数。

**OFFSET: 跳过一定数量的结果**

OFFSET 语句放在 WHERE 语句之后，其语法如下：

```sparql
OFFSET nonnegative_integer
```

其中 `nonnegative_integer` 须为非负整数，表示需要跳过的结果数量。`OFFSET 0` 符合语法，但不会对结果产生影响。由于查询结果默认无序，SPARQL 语义不保证跳过的结果满足任何确定性的条件。因此，OFFSET 语句一般与 ORDER BY 语句配合使用。

下面的查询将示例数据中的影视作品按豆瓣评分从低到高排序，并跳过评分最低的影视作品：

```sparql
SELECT ?movie ?score
WHERE
{
	?movie <豆瓣评分> ?score .
}
ORDER BY ?score
OFFSET 1
```

结果如下：

| ?movie      | ?score                                           |
| ----------- | ------------------------------------------------ |
| \<天龙八部> | "8.3"^^\<http://www.w3.org/2001/XMLSchema#float> |

**LIMIT: 限制结果数量**

LIMIT 语句的语法与 OFFSET 语句类似：

```sparql
LIMIT nonnegative_integer
```

其中 `nonnegative_integer` 须为非负整数，表示允许的最大结果数量。与 OFFSET 类似，由于查询结果默认无序，LIMIT 语句一般与 ORDER BY 语句配合使用。

下面的查询给出示例数据中豆瓣评分最高的影视作品：

```sparql
SELECT ?movie ?scoreWHERE{	?movie <豆瓣评分> ?score .}ORDER BY DESC(?score)LIMIT 1
```

结果如下：

| ?movie      | ?score                                           |
| ----------- | ------------------------------------------------ |
| \<天龙八部> | "8.3"^^\<http://www.w3.org/2001/XMLSchema#float> |

<br/>



## 图更新

通过 **INSERT DATA** ，**DELETE DATA** 和 **DELETE WHERE** 查询，我们可以向数据库中插入或从数据库中删除三元组。

**INSERT DATA**

INSERT DATA 用于向数据库中插入三元组。其语法与 SELECT 查询类似，区别在于构成组图模式的三元组中不能含有变量。

下面的查询向示例数据中插入影视作品仙剑奇侠传的相关信息：

```sparql
INSERT DATA
{
	<仙剑奇侠传> <主演> <胡歌> .
	<仙剑奇侠传> <主演> <刘亦菲> .
	<仙剑奇侠传> <类型> <武侠片> .
	<仙剑奇侠传> <类型> <古装片> .
	<仙剑奇侠传> <类型> <爱情片> .
	<仙剑奇侠传> <豆瓣评分> "8.9"^^<http://www.w3.org/2001/XMLSchema#float> .
}
```

“图模式-最简单的图模式”一节中出现过的查询

```sparql
SELECT ?movie
WHERE
{
	?movie <主演> <刘亦菲> .
}
```

在插入上述数据后，结果变为：

| ?movie        |
| ------------- |
| \<天龙八部>   |
| \<恋爱大赢家> |
| \<仙剑奇侠传> |

**DELETE DATA**

DELETE DATA 用于从数据库中删除三元组。其用法与 INSERT DATA 完全类似。

DELETE WHERE

DELETE DATA 用于从数据库中删除符合条件的三元组；相比起 DELETE DATA ，它的 WHERE 语句与 SELECT 查询的 WHERE 语句是完全相同的，也就是说三元组中允许含有变量。例如，下面的查询删除示例数据中所有武侠片的相关信息：

```sparql
DELETE WHERE{	?movie <类型> <武侠片> .	?movie ?y ?z .}
```

此时再次运行“图模式-最简单的图模式”一节中出现过的查询：

```sparql
SELECT ?movie
WHERE
{
	?movie <主演> <刘亦菲> .
}
```

结果变为：

| ?movie        |
| ------------- |
| \<恋爱大赢家> |





## 高级功能

在**内核版本 v0.9.1** 中，gStore 加入了与数据图中结点间的路径和中心度相关的一系列查询，目前包括环路查询、最短路径查询、K跳可达性查询和Personalized PageRank查询。

在使用高级功能时，需要加载CSR资源；在启动HTTP API服务时，需要加上参数`-c 1`，请详见【快速入门】-【HTTP API服务】。

### 示例数据

为了更好地演示路径相关高级功能，使用以下的社交关系数据作为示例数据：

```
<Alice> <关注> <Bob> .
<Alice> <喜欢> <Bob> .
<Alice> <不喜欢> <Eve> .
<Bob> <关注> <Alice> .
<Bob> <喜欢> <Eve> .
<Carol> <关注> <Bob> .
<Carol> <喜欢> <Bob> .
<Carol> <不喜欢> <Francis> .
<Dave> <关注> <Alice> .
<Dave> <关注> <Eve> .
<Dave> <不喜欢> <Francis> .
<Eve> <喜欢> <Carol> .
<Francis> <喜欢> <Carol> .
<Francis> <不喜欢> <Dave> .
<Francis> <不喜欢> <Eve> .
```

上述数据的图示如下：

![虚构社交网络](https://gstore-web.oss-cn-zhangjiakou.aliyuncs.com/mdimg/%E8%99%9A%E6%9E%84%E7%A4%BE%E4%BA%A4%E7%BD%91%E7%BB%9C.png)

如无特殊说明，返回路径的函数均以如下 JSON 格式字符串表示一条路径/一个环/一个子图：

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

最终返回值以如下形式表示一组路径/一组环/一组子图：（其中 `paths` 的元素为上述格式）

```JSON
{ "paths": [{...}, {...}, ...] }
```

### 路径相关查询

**（1）环路查询**

查询是否存在包含结点 `u`  和 `v` 的一个环。

```
cyclePath(u, v, directed, pred_set)
cycleBoolean(u, v, directed, pred_set)
```

用于 SELECT 语句中，与聚合函数使用语法相同。

**参数**

`u`, `v` ：变量或结点 IRI

`directed` ：布尔值，为真表示有向，为假表示无向（图中所有边视为双向）

`pred_set` ：构成环的边上允许出现的谓词集合。若设置为空 `{}` ，则表示允许出现数据中的所有谓词

**返回值**

- `cyclePath` ：以 JSON 形式返回包含结点 `u`  和 `v` 的一个环（若存在）。若 `u` 或 `v` 为变量，对变量的每组有效值返回一个环。
- `cycleBoolean` ：若存在包含结点 `u`  和 `v` 的一个环，返回真；否则，返回假。

下面的查询询问是否存在包含 Carol 、一个 Francis 不喜欢的人（示例数据中即为 Dave 或 Eve ），且构成它的边只能由“喜欢”关系标记的有向环：

```sparql
select (cycleBoolean(?x, <Carol>, true, {<喜欢>}) as ?y)
where
{
	<Francis> <不喜欢> ?x .
}
```

结果如下：

| ?y                                           |
| -------------------------------------------- |
| "true"^^\<http://www.w3.org/2001/XMLSchema#> |

如果希望输出一个满足以上条件的环，则使用下面的查询：

```sparql
SELECT (cyclePath(?x, <Carol>, true, {<喜欢>}) as ?y)
WHERE
{
	<Francis> <不喜欢> ?x .
}
```

结果如下，可见其中一个满足条件的环由 Eve 喜欢 Carol - Carol 喜欢 Bob - Bob 喜欢 Eve 顺次构成：（为方便阅读，省略了字符串最外层的双引号和内部双引号的转义）

```json
{
	"paths":[{
    "src":"<Eve>",
    "dst":"<Carol>",
    "edges":
    [{"fromNode":2,"toNode":3,"predIRI":"<喜欢>"},{"fromNode":3,"toNode":1,"predIRI":"<喜欢>"},{"fromNode":1,"toNode":2,"predIRI":"<喜欢>"}],
    "nodes":
    [{"nodeIndex":1,"nodeIRI":"<Bob>"},{"nodeIndex":3,"nodeIRI":"<Carol>"},{"nodeIndex":2,"nodeIRI":"<Eve>"}]
	}]
}
```

下图标红的部分即为这个环：

![虚构社交网络_cycle](https://gstore-web.oss-cn-zhangjiakou.aliyuncs.com/mdimg/%E8%99%9A%E6%9E%84%E7%A4%BE%E4%BA%A4%E7%BD%91%E7%BB%9C_cycle.png)

**（2）最短路径查询**

查询从结点 `u` 到结点`v` 的最短路径。

```
shortestPath(u, v, directed, pred_set)
shortestPathLen(u, v, directed, pred_set)
```

用于 SELECT 语句中，与聚合函数使用语法相同。

**参数**

`u` , `v` ：变量或结点 IRI

`directed` ：布尔值，为真表示有向，为假表示无向（图中所有边视为双向）

`pred_set` ：构成最短路径的边上允许出现的谓词集合。若设置为空 `{}` ，则表示允许出现数据中的所有谓词

**返回值**

- `shortestPath` ：以 JSON 形式返回从结点 `u`  到 `v` 的一条最短路径（若可达）。若 `u` 或 `v` 为变量，对变量的每组有效值返回一条最短路径。
- `shortestPathLen` ：返回从结点 `u` 到 `v` 的最短路径长度（若可达）。若 `u` 或 `v` 为变量，对变量的每组有效值返回一个最短路径长度数值。

下面的查询返回从 Francis 到一个 Bob 喜欢、关注或不喜欢，且没有被 Francis 不喜欢的人（示例数据中即为 Alice）的最短路径，边上的关系可以是喜欢或关注：

```sparql
SELECT (shortestPath(<Francis>, ?x, true, {<喜欢>, <关注>}) AS ?y)
WHERE
{
	<Bob> ?pred ?x .
	MINUS { <Francis> <不喜欢> ?x . }
}
```

下图标红的部分即为这条最短路径：

![虚构社交网络_shortestpath](https://gstore-web.oss-cn-zhangjiakou.aliyuncs.com/mdimg/%E8%99%9A%E6%9E%84%E7%A4%BE%E4%BA%A4%E7%BD%91%E7%BB%9C_shortestpath.png)

结果如下：（为方便阅读，省略了字符串最外层的双引号和内部双引号的转义）

```json
{
	"paths":[{
		"src":"<Francis>",
		"dst":"<Alice>",
		"edges":
		[{"fromNode":4,"toNode":3,"predIRI":"<喜欢>"},{"fromNode":3,"toNode":1,"predIRI":"<喜欢>"},{"fromNode":1,"toNode":0,"predIRI":"<关注>"}],
		"nodes":
		[{"nodeIndex":0,"nodeIRI":"<Alice>"},{"nodeIndex":1,"nodeIRI":"<Bob>"},{"nodeIndex":3,"nodeIRI":"<Carol>"},{"nodeIndex":4,"nodeIRI":"<Francis>"}]
		}]
}
```

如果希望只输出最短路径长度，则使用下面的查询：

```sparql
SELECT (shortestPathLen(<Francis>, ?x, true, {<喜欢>, <关注>}) AS ?y)
WHERE
{
	<Bob> ?pred ?x .
	MINUS { <Francis> <不喜欢> ?x . }
}
```

结果如下：（为方便阅读，省略了字符串最外层的双引号和内部双引号的转义）

```json
{"paths":[{"src":"<Francis>","dst":"<Alice>","length":3}]}
```

**（3）单源最短路径**

查询以结点 u 为源节点到其余节点的最短路径。

```
SSSP(u, directed, pred_set)
SSSPLen(u, directed, pred_set)
```

**参数**

`u`：变量或结点 IRI，表示源结点 

`directed`：布尔值，为真表示有向，为假表示无向（图中所有边视为双向） 

`pred_set`：考虑的谓词集合（若设置为空 `{}` ，则表示允许出现数据中的所有谓词）

**返回值**

`SSSP` 返回最短路径，返回值为以下形式，其中 src 为 u 对应的 IRI；dst 为某可达节点对应的 IRI ；nodes 包含路径中涉及节点的下标和IRI；edges 包含路径中涉及边的首尾节点下标和谓词 IRI 。

```cpp
{
    "paths": [
        {
            "src": "<src_IRI>",
            "dst": "<dst_IRI>",
            "edges": [
                {
                    "fromNode": 0,
                    "toNode": 1,
                    "predIRI": "<pred>"
                }
            ],
            "nodes": [
                {
                    "nodeIndex": 0,
                    "nodeIRI": "<src_IRI>"
                },
                {
                    "nodeIndex": 1,
                    "nodeIRI": "<dst_IRI>"
                }
            ]
        },
        ...
    ]
}
```

`SSSPLen` 返回最短路径长度，返回值为以下形式，其中 src 为 u 对应的 IRI；dst为某可达节点对应的IRI ；length为src到dst的最短路长度。

```cpp
{
    "paths": [
        {
            "src": "<src_IRI>",
            "dst": "<dst_IRI>",
            "length": 0
        },
        ...
    ]
}
```

**（4）可达性 / K 跳可达性查询**

查询从结点 `u` 到结点 `v` 是否可达 / 是否 K 跳可达（即存在以 `u` 为起点、以 `v` 为终点，长度小于或等于 `K` 的路径）。

```
kHopReachable(u, v, directed, k, pred_set)
kHopReachablePath(u, v, directed, k, pred_set)
```

**参数**

`u`, `v` ：变量或结点 IRI

`k` ：若置为非负整数，则为路径长度上限（查询 K 跳可达性）；若置为负数，则查询可达性

`directed` ：布尔值，为真表示有向，为假表示无向（图中所有边视为双向）

`pred_set` ：构成路径的边上允许出现的谓词集合。若设置为空 `{}` ，则表示允许出现数据中的所有谓词

**返回值**

- `kHopReachable`：若从结点 `u` 到结点 `v` 可达（或 K 跳可达，取决于参数 `k` 的取值），返回真；否则，返回假。若 `u` 或 `v` 为变量，对变量的每组有效值返回一个真/假值。
- `kHopReachablePath`：返回任意一条从结点 `u` 到结点 `v` 的路径（若可达）或K跳路径，即长度小于或等于`k`的路径（若K跳可达，取决于参数 `k` 的取值）。若 `u` 或 `v` 为变量，对变量的每组有效值返回一条路径（若可达）或K跳路径（若K跳可达）。

下面的查询效仿上一节“最短路径查询”中的示例查询：起点为 Francis ，终点为一个 Bob 喜欢、关注或不喜欢，且没有被 Francis 不喜欢的人（示例数据中即为 Alice）。询问这两人之间是否通过喜欢或关注关系 2 跳或以内可达。

```sparql
SELECT (kHopReachable(<Francis>, ?x, true, 2, {<喜欢>, <关注>}) AS ?y)
WHERE
{
	<Bob> ?pred ?x .
	MINUS { <Francis> <不喜欢> ?x . }
}
```

由于已知满足条件的最短路径长度为 3 ：

![虚构社交网络_shortestpath](https://gstore-web.oss-cn-zhangjiakou.aliyuncs.com/mdimg/%E8%99%9A%E6%9E%84%E7%A4%BE%E4%BA%A4%E7%BD%91%E7%BB%9C_shortestpath.png)

因此上述查询的结果为假：

```json
{"paths":[{"src":"<Francis>","dst":"<Alice>","value":"false"}]}
```

另一方面，Francis 和 Alice 之间是可达的，只是最短路径长度超出了上述限制。因此若查询可达性（将 `k` 设置为负数），则会返回真：

```sparql
SELECT (kHopReachable(<Francis>, ?x, true, -1, {<喜欢>, <关注>}) AS ?y)
WHERE
{
	<Bob> ?pred ?x .
	MINUS { <Francis> <不喜欢> ?x . }
}
```

结果如下：

```json
{"paths":[{"src":"<Francis>","dst":"<Alice>","value":"true"}]}
```

若希望返回一条两人之间满足条件的路径，则可以调用`kHopReachablePath`函数：

```SPARQL
SELECT (kHopReachablePath(<Francis>, ?x, true, -1, {<喜欢>, <关注>}) AS ?y)
WHERE
{
	<Bob> ?pred ?x .
	MINUS { <Francis> <不喜欢> ?x . }
}
```

此时结果可能为上述最短路径：

```json
{
	"paths":[{
		"src":"<Francis>",
		"dst":"<Alice>",
		"edges":
		[{"fromNode":4,"toNode":3,"predIRI":"<喜欢>"},{"fromNode":3,"toNode":1,"predIRI":"<喜欢>"},{"fromNode":1,"toNode":0,"predIRI":"<关注>"}],
		"nodes":
		[{"nodeIndex":0,"nodeIRI":"<Alice>"},{"nodeIndex":1,"nodeIRI":"<Bob>"},{"nodeIndex":3,"nodeIRI":"<Carol>"},{"nodeIndex":4,"nodeIRI":"<Francis>"}]
		}]
}
```

也可能是下图中含有环的、同样满足条件的非最短路径：

![虚构社交网络_khoppath](https://gstore-web.oss-cn-zhangjiakou.aliyuncs.com/mdimg/%E8%99%9A%E6%9E%84%E7%A4%BE%E4%BA%A4%E7%BD%91%E7%BB%9C_khoppath.png)

**（5）所有K跳路径**

 查询从结点 `u` 到结点 `v` 是所有 K 跳可达的路径。

```SPARQL
kHopEnumerate(u, v, directed, k, pred_set)
```

**参数**

`u`, `v` ：变量或结点 IRI

`k` ：若置为非负整数，则为路径长度上限（查询 K 跳可达性）；若置为负数，则查询可达性

`directed` ：布尔值，为真表示有向，为假表示无向（图中所有边视为双向）

`pred_set` ：构成路径的边上允许出现的谓词集合。若设置为空 `{}` ，则表示允许出现数据中的所有谓词

**返回值**

返回所有从结点 `u` 到结点 `v` 的路径（若可达）或K跳路径，即长度小于或等于`k`的路径（若K跳可达，取决于参数 `k` 的取值）。若 `u` 或 `v` 为变量，对变量的每组有效值返回所有路径（若可达）或K跳路径（若K跳可达）。

**例子**

查询起点为 Alice ，终点为一个 Francis 喜欢、关注或不喜欢，且没有被 Alice 不喜欢的人（示例数据中即为 Carol）。查询这两人之间通过喜欢或关注关系 3 跳或以内可达的路径。 

```SPARQL
SELECT (kHopEnumerate(<Alice>, ?x, true, 3, {<喜欢>, <关注>}) AS ?y) 
WHERE 
{
    <Francis> ?pred ?x.
    MINUS { <Alice> <不喜欢> ?x . }
}
```

下图标红的部分即为3跳或以内可达的路径：

![](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/mdimg/%E8%99%9A%E6%8B%9F%E7%BD%91%E7%BB%9C%E7%A4%BE%E4%BA%A4%E5%9B%BE-kHopEmulate.png)

结果如下： （为方便阅读，省略了字符串最外层的双引号和内部双引号的转义） 

```json
{
    "paths":[{
        "src":"<Alice>",
        "dst":"<Carol>",
        "edges":[{"fromNode":0,"toNode":1,"predIRI":"<喜欢>"},{"fromNode":1,"toNode":2,"predIRI":"<喜欢>"},{"fromNode":2,"toNode":3,"predIRI":"<喜欢>"}],
        "nodes":[{"nodeIndex":3,"nodeIRI":"<Carol>"},{"nodeIndex":2,"nodeIRI":"<Eve>"},{"nodeIndex":0,"nodeIRI":"<Alice>"},{"nodeIndex":1,"nodeIRI":"<Bob>"}]},{"src":"<Alice>","dst":"<Carol>","edges":[{"fromNode":0,"toNode":1,"predIRI":"<关注>"},{"fromNode":1,"toNode":2,"predIRI":"<喜欢>"},{"fromNode":2,"toNode":3,"predIRI":"<喜欢>"}],"nodes":[{"nodeIndex":3,"nodeIRI":"<Carol>"},{"nodeIndex":2,"nodeIRI":"<Eve>"},{"nodeIndex":0,"nodeIRI":"<Alice>"},{"nodeIndex":1,"nodeIRI":"<Bob>"}]}]
}
```

**（6）K跳计数**

查询从结点 u 开始，统计其k层可所访问到的结点个数。

```cpp
kHopCount(u, directed, k, pred_set)
```

**参数**

`u`：变量或结点 IRI，表示源结点 

`directed`：布尔值，为真表示有向，为假表示无向（图中所有边视为双向） 

`k` ：跳数（仅统计此跳数可达的节点数）

`pred_set`：考虑的谓词集合（若设置为空 `{}` ，则表示允许出现数据中的所有谓词）

**返回值**

返回值为以下形式，其中 src 为 u 对应的 IRI ；depth 为所处层数/高度（等于参数k）；count 为所处层数访问到的结点总数，类型为整型。

```cpp
{
    "paths":[
        { "src": "<Alice>", "depth": 3, "count": 1}
    ]
}
```

**（7）K跳邻居**

查询从结点 u 开始，其k层可所访问到的结点。

```cpp
kHopNeighbor(u, directed, k, pred_set, ret_num)
```

**参数**

`u`：变量或结点 IRI，表示源结点 

`directed`：布尔值，为真表示有向，为假表示无向（图中所有边视为双向） 

`k` ：跳数（仅统计此跳数可达的节点数）

`pred_set`：考虑的谓词集合（若设置为空 `{}` ，则表示允许出现数据中的所有谓词）

`ret_num`：整数，选填，默认为100，表示最多返回 `ret_num` 个结点 IRI （若总结点数不足 `ret_num` 个，则返回所有结点 IRI ）

**返回值**

返回值为以下形式，其中 src 为 u 对应的 IRI ；depth 为所处层数/高度（等于参数k）；dst为所处层数访问到的结点列表。

```cpp
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

**（8）宽度优先遍历计数**

查询从结点 u 开始，以宽度优先的遍历顺序，在不同层所访问到的结点个数。

```cpp
bfsCount(u, directed, pred_set)
```

**参数**

`u`：变量或结点 IRI，表示源结点 

`directed`：布尔值，为真表示有向，为假表示无向（图中所有边视为双向） 

`pred_set`：考虑的谓词集合（若设置为空 `{}` ，则表示允许出现数据中的所有谓词）

**返回值**

返回值为以下形式，其中 src 为 u 对应的 IRI ；depth 为所处层数/高度（默认 depth 为 0 时只访问 u 自身）；count 为所处层数访问到的结点总数，类型为整型。

```cpp
{"paths":
    [
        {"src":"<Alice>",
         "results":[{"depth":0, "count":1}, ...]
        }
    ]
}
```

**例子**

下面的查询返回以Alice为源节点的有向宽度优先遍历计数，边上的关系可以是喜欢、关注或不喜欢，查询语句为：

```cpp
SELECT(bfsCount(<Alice>,true,{<喜欢>,<关注>,<不喜欢>}) AS ?y)
WHERE{}
```

结果如下：

```cpp
{"paths":
    [
        {"src":"<Alice>",
         "results":[{"depth":0, "count":1}, {"depth":1, "count":2}, {"depth":2, "count":1}, {"depth":3, "count":1}, {"depth":4, "count":1}]
        }
    ]
}
```

### 重要性分析查询

**（1） PageRank**

查询图中各结点的 PageRank 值，使用迭代法进行计算。

```cpp
PR(directed, pred_set, alpha, maxIter, tol)
```

**参数**

`directed`：布尔值，为真表示有向，为假表示无向（图中所有边视为双向） 

`pred_set`：考虑的谓词集合（若设置为空 `{}` ，则表示允许出现数据中的所有谓词）

`alpha`：PageRank 的阻尼参数，体现为随机游走模型中按照图中边游走的概率

`maxIter`：迭代法求解 PageRank 值的最大迭代次数

`tol`：两次迭代间 PageRank 值误差的容忍程度

**返回值**

返回值为以下形式，其中 src 为节点 IRI ；result 为对应结点在图中的 PageRank 值，类型为浮点型。

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

 

**（2）Personalized PageRank**


```
PPR(u, hopCnt, pred_set, retNum)
```

调用 FORA 算法 [1]，计算相对于 u 的 top-K PPR 值。

[1] S. Wang, R. Yang, X. Xiao, Z. Wei, and Y. Yang, “FORA: Simple and Effective Approximate Single-Source Personalized PageRank,” in *Proceedings of the 23rd ACM SIGKDD International Conference on Knowledge Discovery and Data Mining*, Halifax NS Canada, Aug. 2017, pp. 505–514. doi: [10.1145/3097983.3098072](https://doi.org/10.1145/3097983.3098072).

**参数**

`u`：变量或 IRI，表示源结点

`hopCnt`: 整数，为 -1 时，不限跳数计算 PPR ；否则限制在 `hopCnt` 跳内

`pred_set` ：考虑的谓词集合（若给出空，则默认为考虑所有谓词）

`retNum`：整数，表示返回 PPR 值前 `retNum` 大的结点 IRI 及其对应的 PPR（若总结点数不足 `retNum` 个，则返回所有结点 IRI 及其对应的 PPR）

**返回值**

返回值为以下形式，其中 src 为 u 对应的 IRI 或变量查询出的结果；dst 含有哪些目标结点取决于函数的第二个参数；对应的 PPR 值为双精度浮点数。

（注：由于 FORA 是具有随机性的近似算法，每次的返回值有微小差别是正常的。）

```
{"paths":
	[
		{"src":"<Francis>", "results":
			[{"dst":"<Alice>", "PPR":0.1}, {"dst":"<Bob>", "PPR": 0.01}, ...]
		}, ...
	]
}
```

**例子**

返回Bob喜欢、关注或不喜欢的所有人各自对应的拥有top-3 PPR 值的三个人（及其 PPR 值）：

```sparql
select (PPR(?x, -1, {}, 3) as ?y)
where
{
	<Bob> ?pred ?x .
}
```

**（3） 紧密中心度**

查询某节点到达其他节点的难易程度。

```SPARQL
closenessCentrality(u, directed, pred_set)
```

**参数**

`u` : 变量或节点 IRI，表示源结点

`directed`: 布尔值，为真表示有向，为假表示无向（图中所有边视为双向） 

`pre_set`: 考虑的谓词集合（若设置为空 `{}` ，则表示允许出现数据中的所有谓词）

**返回值**

返回值为以下形式，其中 src 为 u 对应的 IRI ；result 为结点 u 在图中的紧密中心度，类型为浮点型。

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

**例子：**

例1、查询返回以**Alice**在**无向图**（图中所有边视为双向）中的紧密中心度，边上的关系可以是**喜欢**或**关注**，SPARQL查询语句为：

查询返回以**Alice**在**无向图**（图中所有边视为双向）中的紧密中心度，边上的关系可以是**喜欢**或**关注**，SPARQL查询语句为：

```SPARQL
SELECT (closenessCentrality(<Alice>, false, {<喜欢>, <关注>}) AS ?x) WHERE{}
```

结果如下（为方便阅读，省略了字符串最外层的双引号和内部双引号转义）：

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

上述查询，Alice到达其余各节点的最短距离如下，可计算出平均距离为1.8，紧密中心度值为1/1.8 = 0.555556，与执行结果一致。

```json
{
	"Bob" : 1,
    "Dave" : 1,
    "Eve" : 2,
    "Carol" : 2,
    "Francis" : 3
}
```

例2、查询返回以**Alice**在**有向图**中的紧密中心度，边上的关系可以是**喜欢**或**关注**，SPARQL查询语句为：

```SPARQL
SELECT (closenessCentrality(<Alice>, true, {<喜欢>, <关注>}) AS ?x) WHERE{}
```

结果如下（为方便阅读，省略了字符串最外层的双引号和内部双引号转义）：

```json
{
    "paths": [
        {
            "src": "<Alice>",
            "result": 0.500000
        }
    ]
}
```

在上述查询，Alice到达其余各节点的最短距离如下，可计算出平均距离为2，紧密中心度值为1/2 = 0.5，与执行结果一致。

```json
{
	"Bob" : 1,
    "Eve" : 2,
    "Carol" : 3
}
```

**（4）三角形计数**

统计图中三角形数量。

```SPARQL
triangleCounting(directed, pred_set)
```

**参数**

`u` : 变量或节点 IRI，表示源结点

`directed` : 布尔值，为真表示有向，为假表示无向（图中所有边视为双向）。若为有向则仅计数 cycle 类型三角形 

`pre_set` : 考虑的谓词集合（若设置为空 `{}` ，则表示允许出现数据中的所有谓词）

**返回值**

返回值为以下形式：

```json
{
    "paths": [2]
}
```

**例子：**

下面的查询询问该图中有向三角形的数目，且构成它的边只能由**喜欢**关系标记，SPARQL查询语句为：

```SPARQL
select (triangleCounting(true, {<喜欢>}) as ?y) where {}
```

![虚构社交网络_cycle](https://gstore-web.oss-cn-zhangjiakou.aliyuncs.com/mdimg/%E8%99%9A%E6%9E%84%E7%A4%BE%E4%BA%A4%E7%BD%91%E7%BB%9C_cycle.png)

结果如下，即该图中构成它的边只能由“喜欢”关系标记的有向三角形的数目为1，即 Bob -> Eve -> Carol -> Bob：

```json
{
    "paths":[1]
}
```

**（5）标签传播**

基于标签传播查询图中各结点的聚类情况，可用于社区发现等多种应用。

```
labelProp(directed, pred_set)
```

**参数**

`directed`：布尔值，为真表示有向，为假表示无向（图中所有边视为双向） 

`pred_set`：考虑的谓词集合（若设置为空 `{}` ，则表示允许出现数据中的所有谓词）

`maxIter`: 最大迭代次数

**返回值**

返回值为数组的数组（嵌套数组），其中元素为节点 IRI ，对应图中节点的一个划分。

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

**（6）弱连通分量**

返回图的所有弱连通分量。

```
WCC(pred_set)
```

**参数**

`pred_set` ：构成弱连通分量的边上允许出现的谓词集合。若设置为空 `{}` ，则表示允许出现数据中的所有谓词

**返回值**

嵌套数组，形式与标签传播的返回值相同。

**（7）局部集聚系数**

查询结点 u 的局部集聚系数，即所有与它相连的结点之间所连的边的数量（即实际形成的以 u 为顶点的三角形数目），除以这些结点之间可以连出的最大边数（即最大可能形成的以 u 为顶点的三角形数目）。

```
clusterCoeff(u, directed, pred_set)
```

**参数**

`u`：变量或结点 IRI

`directed`：布尔值，为真表示有向，为假表示无向（图中所有边视为双向） 。将图视为有向时，仅计数 cycle 类型三角形（详见三角形计数的介绍）

`pred_set`：考虑的谓词集合（若设置为空 `{}` ，则表示允许出现数据中的所有谓词）

**返回值**

返回值为结点 u 的局部集聚系数，对应的值为双精度浮点数（形式详见以下例子）。

**（8）整体集聚系数**

查询图的整体集聚系数。

```
clusterCoeff(directed, pred_set)
```

**参数**

`directed`：布尔值，为真表示有向，为假表示无向（图中所有边视为双向） 。将图视为有向时，仅计数 cycle 类型三角形（详见三角形计数的介绍）

`pred_set`：考虑的谓词集合（若设置为空 `{}` ，则表示允许出现数据中的所有谓词）

**返回值**

返回值为图的整体集聚系数，对应的值为双精度浮点数。

**（9） 紧密中心度**

返回结点 u 的紧密中心度。

```cpp
closenessCentrality(u, directed, pred_set)
```

**参数**

`u`：变量或结点 IRI，表示源结点 

`directed`：布尔值，为真表示有向，为假表示无向（图中所有边视为双向） 

`pred_set`：考虑的谓词集合（若设置为空 `{}` ，则表示允许出现数据中的所有谓词）

**返回值**

返回值为以下形式，其中 src 为 u 对应的 IRI ；result 为结点 u 在图中的紧密中心度，类型为浮点型。

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

**（10）Louvain**

鲁汶算法。

```cpp
louvain(directed, pred_set, maxIter, increase)
```

**参数**

`directed`：布尔值，为真表示有向，为假表示无向（图中所有边视为双向） 

`pred_set`：考虑的谓词集合（若设置为空 `{}` ，则表示允许出现数据中的所有谓词）

`maxIter`：第一阶段最大迭代轮数(>=1)

`increase`：模块度增益阈值(0~1) 

**返回值**

返回值为以下形式，count为划分的社区数量，details为划分的各社区信息，包括社区编号communityId、成员数量menberNum

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



<div STYLE="page-break-after: always;"></div>
