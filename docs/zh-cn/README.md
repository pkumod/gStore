<div align="center">
  <img src="../logo.png" style="height: 200px;" alt="gStore logo"/>
</div>

<div align="center">
  <h4> <a href="../../README.md" target="_blank"> English </a> | 中文 | <a href="https://en.gstore.cn" target="_blank"> Website </a> | <a href="https://www.gstore.cn" target="_blank"> 网站 </a> |  <a href="https://gstore-docs.pages.dev" target="_blank"> Documentation </a> | <a href="https://gstore-docs.pages.dev/zh-cn/" target="_blank"> 文档 </a> </h4>
</div>

<div align="center">
  <a href="https://github.com/hrz6976/gstore/commits"><img alt="GitHub commit activity" src="https://img.shields.io/github/commit-activity/y/pkumod/gstore?logo=github"/></a>
  <a href="https://github.com/hrz6976/gstore/contributors"><img alt="GitHub contributors" src="https://img.shields.io/github/contributors-anon/hrz6976/gstore?logo=github&color=%23ffd664"/></a>
  <a href="https://github.com/hrz6976/gstore/actions" ><img alt="GitHub Actions Workflow Status" src="https://img.shields.io/github/actions/workflow/status/hrz6976/gStore/ci.yml?logo=github"/></a>
  <a href="https://codecov.io/github/hrz6976/gStore" ><img src="https://codecov.io/github/hrz6976/gStore/graph/badge.svg?token=0NE3KM5AV8"/></a>
  <a href="https://hub.docker.com/repository/docker/hrz6976/gstore"><img alt="Docker Image Version (latest semver)" src="https://img.shields.io/docker/v/hrz6976/gstore?logo=docker&label=docker&color=%2328a8ea"/></a>
  <span><img alt="Static Badge" src="https://img.shields.io/badge/arch-amd64_arm64_loongarch-%23f23f46?logo=amazonec2"></span>
</div>

## 知识图谱简介


近年来随着“人工智能”概念再度活跃，除了“深度学习”这个炙手可热的名词以外，“知识图谱”无疑也是研究者、工业界和投资人心目中的又一颗“银弹”。简单地说，“知识图谱”是一种数据模型，是以图形（Graph）的方式来展现“实体”、实体“属性”，以及实体之间的“关系”。下图是截取的Google的知识图谱介绍网页中的一个例子。在例子中有4个实体，分别是“达芬奇”，“意大利”，“蒙拉丽莎”和“米可朗基罗”。这个图明确地展示了“达芬奇”的逐个属性和属性值（例如名字、生日和逝世时间等），以及之间的关系（例如蒙拉丽莎是达芬奇的画作，达芬奇出生在意大利等）。

![](https://gstore-web.oss-cn-zhangjiakou.aliyuncs.com/mdimg/Google.jpg)

目前知识图谱普遍采用了语义网框架中RDF(Resource Description Framework,资源模式框架)模型来表示数据。语义网是万维网之父蒂姆·伯纳斯-李(Tim Berners-Lee)在1998年提出的概念，其核心是构建以数据为中心的网络，即Web of Data。其中RDF是W3C的语义网框架中的数据描述的标准，通常称之为RDF三元组<主体 (subject)，谓词 (predicate)，宾语(object)>。其中主体一定是一个被描述的资源，由URI来表示。谓词可以表示主体的属性，或者表示主体和宾语之间某种关系；当表示属性时，宾语就是属性值，通常是一个字面值（literal）；否则宾语是另外一个由URI表示的资源。
&ensp;&ensp;下图展示了一个人物类百科的RDF三元组的知识图谱数据集。例如y:Abraham_Lincoln表示一个实体URI（其中y表示前缀http://en.wikipedia.org/wiki/），其有三个属性(hasName,BornOdate,DiedOnDate)和一个关系（DiedIn）。

![image](https://gstore-web.oss-cn-zhangjiakou.aliyuncs.com/mdimg/RDF.jpg)

<center>图 1-1 RDF数据的例子</center>


&ensp;&ensp;面向RDF数据集，W3C提出了一种结构化查询语言SPARQL；它类似于面向关系数据库的查询语言SQL。和SQL一样，SPARQL也是一种描述性的结构化查询语言，即用户只需要按照SPARQL定义的语法规则去描述其想查询的信息即可，不需要明确指定如何进行查询的计算机的实现步骤。2008年1月， SPARQL成为W3C的正式标准。SPARQL中的WHERE子句定义了查询条件，其也是由三元组来表示。我们不过多的介绍语法细节，有兴趣的读者可以参考[1]。下面的例子解释了SPARQL语言。假设我们需要在上面的RDF数据中查询“在1809年2月12日出生，并且在1865年4月15日逝世的人的姓名？” 这个查询可以表示成如下图的SPARQL语句。

```SPARQL
SELECT ?name                              # 查询返回的变量值
WHERE
{
    ?m <hasName> ?name.                   # 查询条件
    ?m <BornOnDate> "1809-02-12" .
    ?m <DiedOnDate> "1865-04-15 .
}
```

<center>图 1-2 SPARQL查询的例子</center>

&ensp;&ensp;知识图谱数据管理的一个核心问题是如何有效地存储RDF数据集和快速回答SPARQL查询。总的来说，有两套完全不同的思路。其一是我们可以利用已有的成熟的数据库管理系统（例如关系数据库系统）来存储知识图谱数据，将面向RDF知识图谱的SPARQL查询转换为面向此类成熟数据库管理系统的查询，例如面向关系数据库的SQL查询，利用已有的关系数据库产品或者相关技术来回答查询。这里面最核心的研究问题是如何构建关系表来存储RDF知识图谱数据，并且使得转换的SQL查询语句查询性能更高；其二是直接开发面向RDF知识图谱数据的Native的知识图谱数据存储和查询系统（Native RDF图数据库系统），考虑到RDF知识图谱管理的特性，从数据库系统的底层进行优化。

![image](https://gstore-web.oss-cn-zhangjiakou.aliyuncs.com/mdimg/4.jpg)

<center>图 1-3 RDF图和SPARQL查询图</center>

## gStore简介

&ensp;&ensp;我们所研发的gStore系统属于后者，gStore是由北京大学王选计算机研究所数据管理实验室（PKUMOD）历经十年研发面向RDF数据模型的开源图数据库系统（通常称为Triple Store）。不同于传统基于关系数据库的知识图谱数据管理方法，gStore**原生基于图数据模型**(Native Graph Model)，维持了**原始RDF知识图谱的图结构**；其数据模型是有标签、有向的多边图，每个顶点对应着一个主体或客体。我们将面向RDF的SPARQL查询，转换为**面向RDF图的子图匹配查询**，利用我们所提出的**基于图结构的索引(VS-tree)来加速查询的性能**。 图1-3显示了上例所对应的RDF图和SPARQL查询图结构。回答SPARQL查询本质上就是在RDF图中找到SPARQL查询图的子图匹配的位置，这就是基于图数据库的回答SPARQL查询的理论基础。在图1-3例子中，由节点005，009，010和011所推导的子图就是查询图的一个匹配，根据此匹配很容易知道SPARQL的查询结果是“Abraham Lincoln”。关于gStore的核心学术思路，请参考开发资源-论文和专利所发表的论文。

&ensp;&ensp;gStore开始于北京大学王选计算机研究所数据管理组（PKUMOD）邹磊教授与滑铁卢大学Tamer Ozsu教授、香港科技大学Lei Chen教授所撰写的VLDB 2011论文(Lei Zou, Jinghui Mo, Lei Chen,M. Tamer Ozsu, Dongyan Zhao, gStore: Answering SPARQL Queries Via Subgraph Matching, Proc. VLDB 4(8): 482-493, 2011), 在论文中提出了利用子图匹配的方法回答SPARQL中的BGP (Basic Graph Pattern)语句的查询执行方案。该文章发表以后，PKUMOD实验室在中国自然科学基金委项目和中国科技部重点研发课题等资助下，持续从事gStore系统的开源、维护和系统优化工作。目前Github上开源的gStore系统可以支持W3C定义的SPARQL 1.1标准（具体可支持的SPARQL语法，请参考【SPARQL查询语言】）；

​    经过一系列的测试，测试结果表明gStore 在回答复杂查询（例如，包含圆圈）方面比其他数据库系统运行得更快。对于简单的查询，gStore 和其他数据库系统都运行良好。gStore单机版本可以支持**50亿以上**的RDF三元组存储和SPARQL查询，分布式系统gStore（分布式版本，目前未开源）具有非常好的可扩展性，根据“中国软件测评中心”给出的测试报告显示，分布式gStore系统在百亿规模的RDF三元组数据集上具有秒级查询时间。

&ensp;&ensp;gStore系统在Github上开源以来，一直采用开源社区中广泛使用的BSD 3-Clause开源协议，以促进gStore相关知识图谱技术生态的建设。根据该协议，我们要求使用者在充分需要尊重代码作者的著作权前提下，允许使用者自由的修改和重新发布代码，也允许使用者在gStore代码基础上自由地开发商业软件，以及发布和销售；但是以上的前提是必须满足第10章“法律条款”中，根据BSD 3-Clause开源协议，我们所拟定的相关法律条款。我们严格要求使用者，在其所发布的基于gStore代码基础上开发的软件上标有“powered by gStore”和gStore标识（详见参考gStore标识）。我们强烈建议使用者在使用gStore前，参考“开源与法律条款”中有关规定。
