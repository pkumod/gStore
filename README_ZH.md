<div align="center">
  <img src="https://www.gstore.cn/pcsite/docs/logo.png" style="height: 200px;" alt="gStore logo"/>
</div>

[English](README.md) | **中文** | [Website](https://en.gstore.cn) | [网站](https://www.gstore.cn)

<div align="center">
  <a href="https://github.com/pkumod/gStore/commits/1.2/"><img alt="GitHub commit activity" src="https://img.shields.io/github/commit-activity/m/pkumod/gstore?logo=github"/></a>
  <a href="https://github.com/pkumod/gStore/graphs/contributors"><img alt="GitHub contributors" src="https://img.shields.io/github/contributors-anon/pkumod/gstore?logo=github&color=%23ffd664"/></a>
  <a href="https://hub.docker.com/repository/docker/pkumodlab/gstore-docker/general"><img alt="Docker Image Version (V1.2)" src="https://img.shields.io/docker/v/pkumod/gstore?logo=docker&label=docker&color=%2328a8ea"/></a>
  <span><img alt="Static Badge" src="https://img.shields.io/badge/arch-x86_amd64_arm64_loongarch-%23f23f46?logo=amazonec2"></span>
</div>

gStore是一个开源的图形数据库引擎(或“三元存储”)，为管理大型数据库而产生 [RDF](http://www.w3.org/TR/rdf11-concepts/) 数据集 [SPARQL](https://www.w3.org/TR/sparql11-query/) 查询语言。它适用于Linux系统和amd64、arm64和loongarch处理器。 gStore是 [北京大学数据管理实验室](https://mod.icst.pku.edu.cn/english/index.htm), [Waterloo大学](https://uwaterloo.ca/), 和来自开源社区 [贡献值](https://github.com/pkumod/gStore/contributors) 的合作成果。

:认证: gStore在 [BSD 3-Caluse](https://opensource.org/license/bsd-3-clause/) 认证下发布, 在对应第三方库的许可下。详情请查看 [LICENSE](LICENSE) 。

:问题: 查看 [FAQ](docs/zh-cn/FAQ.md) 了解常见问题。[BUGS](docs/zh-cn/BUGS.md) 和 [LIMIT](docs/zh-cn/LIMIT.md)列出了已知的错误和限制。如果你发现任何错误，请随时 [open an issue](https://github.com/pkumod/gStore/issues/new/choose)提出。

:反馈: 如果你有任何问题或建议， 请打开 [GitHub Discussions](https://github.com/pkumod/gStore/discussions)提问。

:在线文档: 对应建议，项目架构或则更多, 请查看 [在线文档](https://www.gstore.cn/pcsite/index.html#/documentation).

**正式的帮助文档已经发布在 [English(EN)](help/gStore_help.pdf) 和 [中文(ZH)](help/gStore_help_ZH.pdf).**

**正式实验结果已公布在 [Experiment](test/formal_experiment.pdf).**

**我们已经建立一个名为gStore的网站，您可以访问 [gStore主页](https://www.gstore.cn/).**

## gstore获取

gStore已上传至gitee(代码云)，建议中国大陆用户下载速度更快。网址是 https://gitee.com/PKUMOD/gStore , 您也可以打开 https://github.com/pkumod/gStore, 下载gStore.zip，然后解压缩压缩包

### Docker方式


```bash
$ docker pull pkumodlab/gstore-docker:latest
```

完整的说明文档在 [Docker部署说明](docs/zh-cn/DOCKER_DEPLOY_CN.md).

### clone方式（推荐）


```bash
git clone https://github.com/pkumod/gStore.git
```

完整的说明文档在 [安装指南](docs/zh-cn/INSTALL_INSTRUCT.md).

## 快速开始

**N-Triple 数据格式介绍**

​      RDF数据应该以n-triple格式提供(目前不支持XML)，查询必须以[SPARQL1.1](https://www.w3.org/TR/sparql11-query/)语法提供查询。[SPARQL1.1](https://www.w3.org/TR/sparql11-query/)格式文件示例如下：        

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

**初始化系统数据库**

```shell
bin/ginit
```

**创建数据库**

```shell
bin/gbuild -db lubm -f data/lubm/lubm.nt 
```

**展示数据库列表**

```shell
bin/gshow
```

**数据库数据查询**

```shell
bin/gquery -db lubm -q data/lubm/lubm_q0.sql
```

完整的说明文档在 [快速开始](docs/zh-cn/QUICK_START.md).


## 引用gStore


如果您在您的研究中使用gStore，请引用以下资料:


```bibtex
@article{zou2014gstore,
  title={gStore: a graph-based SPARQL query engine},
  author={Zou, Lei and {\"O}zsu, M Tamer and Chen, Lei and Shen, Xuchuan and Huang, Ruizhe and Zhao, Dongyan},
  journal={The VLDB journal},
  volume={23},
  pages={565--590},
  year={2014},
  publisher={Springer}
}
```


或者引用这个资源库:


```bibtex
@misc{gStore,
  author = {gStore Authors},
  title = {gStore},
  year = {2021},
  publisher = {GitHub},
  journal = {GitHub repository},
  howpublished = {\url{https://github.com/pkumod/gStore}},
}
```


## 版本更新记录
**gStore 1.2 版本**

- 更新时间：2023-11-11
- 更新功能
  - **优化 ORDER BY 语句**：精简 ORDER BY 执行逻辑、去除不必要的类型判断和转换，大幅提升执行效率。
  - **优化构建模块**：支持构建空库。
  - **优化三元组解析器**：支持纯数字 IRI ，支持仅由数字和字母组成的 IRI 、支持以数字开头的 IRI。
  - **新增API接口**：gStore 1.2 的 ghttp 和 gRPC 服务都增加**上传文件**、**下载文件**、**统计系统资源**、**重命名**、**获取备份路径**五个接口。
  - **新增内置高级函数**：gStore 1.2 版本新增七个高级函数，分别是**单源最短路径（SSSP、SSSPLen）**，**标签传播（labelProp）**，**弱连通分量（WCC）**，**整体/局部集聚系数（clusteringCoeff）**，**鲁汶算法（louvain）**、**K跳计数（kHopCount）**、**K跳邻居（kHopNeighbor）**。
  - **新增支持在 SELECT 语句中调用 CONCAT 函数**。
  - **优化部分本地命令和API接口**：优化本地命令 gconsole ，优化了构建、加载、统计图数据库等接口，修复了可能导致内存泄漏的潜在 bug 。
  - **新增多种数据格式支持**：新增**Turtle**，**TriG**，**RDF/XML**，**RDFa**，**JSON-LD**等多种格式的支持。
  - **自定义图分析算法编辑功能优化**：对自定义图分析算法编辑功能界面进行重新设计，并优化动态编译算法，提升编译效率。
  - **Bug修复**：修复了一系列Bug。

**gStore 1.0 版本**

- 更新时间：2022-10-01

- 更新功能

  - **支持用户自定义图分析算子函数**：用户可通过API接口或可视化管理平台 gStore-workbench 对自定义图分析算子函数进行管理，通过接口函数获取图数据的结点数、边数、任意给定结点的邻居等，以此为基本单元进行实现自定义的图分析算子函数，并支持动态编译和动态运行；

  - **新增 gRPC 网络接口服务**：gRPQ 是基于开源库 `workflow` 实现的一个基于 HTTP 协议的高性能网络接口服务，进一步提高了接口服务的效率和稳定性。gRPC  与 ghttp 的对比实验结果表明，gRPC 在并发访问性能方面有巨大提升，在**2000/QPS**情况下，拒绝访问失败率为**0%**。

  - **新增 gConsole 模块**：在 gStore 1.0 中我们重磅推出了 gConsole 模块，实现了具有上下文信息的**“长会话”**操作gStore。

  - **优化器与执行器分离**：gStore 1.0 解耦了优化器和执行器，从原有的深度耦合的贪心策略，转化为基于动态规划的查询优化器和基于广度优先遍历的查询执行器。

  - **优化 Top-K 查询**：我们在 gStore 中实现了基于 DP-B 算法的 Top-K SPARQL 处理框架，包括查询切分，子结果聚合等部分。

  - **支持 ACID 事务处理**：gStore 1.0 通过引入多版本管理机制，可以对插入和删除操作启动 ACID 事务，用户可以对事务进行开启、提交、回滚等操作。目前 gStore 1.0 支持四个隔离等级：read-uncommitted（读未提交）、read-committed（读并提交）、repeatable read（可重复读）、serializable（可串行化）。

  - **重构database内核，优化执行树生成逻辑**: 在 gStore1.0 中，引入了两种连接操作（worst-case-optimal join 和 binary join）优化查询执行，进一步提升查询效率。

  - **优化日志模块**：基于 log4cplus 库，实现统一格式的系统日志输出，用户可配置日志输出的方式（控制台输出、文件输出），输出格式以及输出级别等。

  - **新增内置高级函数**：gStore 1.0 版本新增四个高级查询函数，分别是三角形计数（triangleCounting）、紧密中心度（closenessCentrality）、宽度优先遍历结点计数（bfsCount）和所有K跳路径（kHopEnumeratePath）。

  - **新增 BIND 语句支持**：支持在 BIND 语句中使用代数或逻辑表达式对变量赋值的功能。

  - **优化部分本地命令和 API 接口，并修复一系列bug**：优化 shutdown 命令，修复 gmonitor 统计数据不准确等问题。

**gStore 0.9.1 版本**

- 更新时间：2021-11-23

- 更新功能

  - 将gStore内核解析与执行进行分离，通过join order等技术进一步提升查询性能，在复杂查询中性能可以提升40%以上；

  - 重写gStore的http service组件ghttp，并增加了用户权限、心跳检测、批量导入、批量删除等功能，并编写了规范的ghttp api接口文档(见接口列表)，进一步丰富ghttp的功能，提升ghttp的健壮性；

  - 新增了Personalized PageRank（PPR）自定义函数，Personalized PageRank自定义函数可用于计算实体间的相关度，从而在图中找出影响度最大的节点；

  - 新增Filter语句中对算术及逻辑运算的支持，如算术运算（如?x + ?y = 5）；逻辑运算（如 ?x + ?y = 5 && ?y > 0）等；

  - 增加事务处理功能，支持begin/tquery/commit/rollback等事务操作；

  - 新增gServer组件，实现Socket API双向通信，用户除了通过ghttp组件远程访问gStore之外，还可以通过gServer组件远程访问gStore；

  - 规划本地操作指令格式，引入--help指令，用户可以查看各功能的详细指令格式，如bin/gbuild -h/--help可以详细查看gbuild命令的指令格式 ；

  - 修复一系列bug。

    

**gStore 0.9.0版本**

- 更新时间：：2021-02-10

- 更新功能：
  - 将 SPARQL 解析器生成器从 ANTLR v3 升级到最新的、文档齐全且维护良好的 v4；
  - 支持在 SPARQL 查询中编写没有数据类型后缀的数字文字；
  - 支持 SELECT 子句中的算术和逻辑运算符；
  - 支持 SELECT 子句中的聚合 SUM、AVG、MIN 和 MAX；
  - 额外的支持内置在过滤器中，函数功能，包括`datatype`，`contains`，`ucase`，`lcase`，`strstarts`，`now`，`year`，`month`，`day`，和`abs`;
  - 支持路径相关功能作为SPARQL 1.1的扩展，包括环路检测、最短路径和K-hop可达性；
  - 支持数据库全量和增量备份和恢复，管理员配置可以开启自动全量备份；
  - 支持基于日志的回滚操作；
  - 支持具有三级隔离的事务：已提交读、快照隔离和可序列化；
  - 扩展数据结构以容纳多达 50 亿个三元组的大规模图。

<!--**You can write your information in [survey](http://59.108.48.38/survey) if you like.**-->

- - -

## 帮助文档

如果你想要了解gstore系统的详细信息，或则一些高级操作（例如：使用API, server/client）, 请看以下内容。

- [文档下载](docs/zh-cn/DOCUMENT_DOWNLOAD.md)

- [更新日志](docs/zh-cn/UPDATE_LOG.md)

- [知识图谱与gstore介绍](docs/zh-cn/GRAPH_INTRODUCE.md)

- [安装指南](docs/zh-cn/INSTALL_INSTRUCT.md)

- [快速入门](docs/zh-cn/QUICK_START.md)

- [常用API](docs/zh-cn/API_USAGE.md)

- [SPARQL查询语言](docs/zh-cn/SPARQL_QUERY.md)

- [可视化工具Workbench](docs/zh-cn/WORKBENCH_USAGE.md)

- [gStore云平台介绍](docs/zh-cn/CLOUD_INTRODUCE.md)

- [项目大事记](docs/zh-cn/CHRONOLOGY.md)

- [开源与法律条款](docs/zh-cn/LEGAL_PROVISION.md)

- [gstore标识](docs/zh-cn/GSTORE_LOGO.md)

- - -


## 其他信息

bug记录在 [bug报告](docs/zh-cn/BUGS.md) 中。如果您发现此文件中不存在错误，欢迎您通过 [社区Web](https://www.gstore.cn/pcsite/index.html#/forum) 提问提交错误。

通过gstore去实现app，我们记录了所面对的一系列挑战信息，这些信息记录在 [实现流程](docs/zh-cn/TIPS.md)。

如果不需要及时回复，欢迎您在github问题部分报告任何建议或错误。但是，如果您想紧急要求我们处理您的报告，请发送电子邮件至 <gstore@pku.edu.cn> 提交您的建议和错误。我们整个团队的名单都在里面 [邮件列表](docs/zh-cn/MAIL.md)。

当你使用当前的gStore项目时，有一些限制，你可以在上面看到 [限制描述](docs/zh-cn/LIMIT.md)。

有时你可能会发现一些奇怪的现象(但不是错误的情况)，或者一些难以理解/解决的事情(不知道下一步该怎么做)，那么请访问 [常见问题](docs/zh-cn/FAQ.md) 页面。

图数据库引擎是一个新领域，我们仍在努力走得更远。我们下一步计划做的事情是 [未来计划](docs/zh-cn/PLAN.md), 我们希望越来越多的人支持甚至加入我们。你可以用很多方式支持:

- 观察我们的项目

- 通过这个仓库并向我们提交请求

- 请下载并使用本系统，报告错误或建议

- ...

激发我们或为这个项目做出贡献的人将被列入 [感谢名单](docs/zh-cn/THANK.md)。



<!--This whole document is divided into different pieces, and each them is stored in a markdown file. You can see/download the combined markdown file in [help_markdown](docs/gStore_help.md), and for html file, please go to [help_html](docs/gStore_help.html). What is more, we also provide help file in pdf format, and you can visit it in [help_pdf](docs/latex/gStore_help.pdf).-->