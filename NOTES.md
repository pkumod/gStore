# TODO

至少需要维护三个版本：
（最初的版本记为0，保留不再更新）
只提供查询，kvstore中无需保留重复，效率更高，版本1.1（保留，不再添加新功能，只修复bug）；
需支持插删，kvstore中需要保留重复，效率较低，版本1.2
分布式版本，由配置文件指定，在源代码中加一层判断，合为一体
(未彻底合并时暂定为1.0)

和virtuoso/jena对比（运行插删的sparql语句） 必须保证插删查正确，且得出性能报告

to support 10 billion in a single machine, the memory should > 100G
change Bstr length to long long type
(also update anywhere related to length or strlen, such as Signature::encodeStr2Entity)
change entity/literal ID type to unsigned
+++++++++++++++++++++
应统计并降低IO开销（主要是kvstore），尽量顺序连续、预读取
比如加载时除了索引结构外，可以先读入一批节点
另外如果可以把bstr和节点分离，确保每个节点都是相同大小（预先分配定量数组）
所有bstr串单独放到一个文件中，不删除不修改，只能附加
大小超过一个阈值后，进行整理，移除无效的字段
节点中只保留Bstr的文件偏移，用long long类型
可以考虑将id2string直接使用倒排表实现，不用B+树（因为插删是很少的）
但string2id很麻烦，不能直接用倒排表计算

get 不能全部返回去重的，因为插删也需要调用get
除了join模块，其他只和查询相关的模块，最好也改为去重的

新建一个ID管理模块，要求高效稳定，放在Util中

watdiv5000 exit unexpectedly -- p2so index
541740149   549240150
entity 26060745
literal 23964574
pre 86

调试成组插删，提升效率
目前存在的问题在于small_aa small_ab q3.sql
toStartJoin to add literals p2olist the olist is not right

精简索引，如p2s和p2o可借用p2so来完成，同样得比如s2po和o2ps
而且sp2o普遍比较大，也可以用s2po加二分查找来完成
最终只需要完成6+3棵kv树
(很多问题，实现s2o要排序，代价可能非常高)
（可以考虑共用s2索引，同时存下o和po作为value，一个key对应两个Bstr）

implement so2p with s2p and o2p may cause error
s p o1
s2 p o

kvstore保留重复，可能太多，如何解决？
压缩？可能引入额外的压缩信息，而且要有一个解压过程

使用redis来实现KVstore?

join(preFilter根据p的num数量)和stream中的问题(判断失误或内存泄露)
将b树返回num改为直接用已有的实现(应该在底层实现么，如何保证效率)
在Query中建立p2num和sp2num等对应，避免反复搜索。
(o2p o2ps o2s是否应该划分为entity/literal来加速, id2string是否用倒排表)

考虑用内联汇编在大循环里做优化(测试对比效率提升)
最好把entity和literal彻底分开

Join::preFilter -- 某些时候先过滤，某些时候后过滤，视结果数目和边的过滤性能而定
不用似乎也不会出错
when using allFilterByPres, not always good, sometimes too costly!(dbpedia2014, self6.sql)
use "well-designed" in GeneralEvaluation to enable not all selected query; 
三个瓶颈：getFinalResult copyToResult allPreFilter
join_basic的效率非常关键，是核心

另一种过滤：用sp2num预估数量，或者也预估vstree的结果数量，set triple dealed(以后避免重复处理)
可以考虑只在拼接时add literal(需要考虑常量约束，entity与literal分开考虑)，最多提前加一个保证join启动
在idlist中寻找entity和literal的临界点时，注意左小右大，不要单纯扫描
是否最后再考虑卫星边的约束更好？（卫星边一定是单度的）
entity和literal没必要放在一起作交，另外交的顺序或许也很重要
（每次应选最小的两个，或者多路merge的形式）

+++++++++++++++++++++
带标签的路径，而不是过滤加验证的方式 |图仿真避开无用的候选集和join
分析cas的结构和查询模式
ask转换为常量三元组的判断
+++++++++++++++++++++
研究透彻本组的论文，包括李友焕师兄的
讨论应尽量在issues中
join use 2-loop with so2p function maybe not good

- - -

# TEST

watdiv5000在建立p2?索引时出错(p2so索引太大？)

建立日志系统方便调试？
出错时如何回滚，直接进行版本管理？
(每次插删前，将原来的索引做一个备份？)

测试时每个查询除了时间外，还要记录一下结果数，备用
a whole test on dbpedia must be done before commited!!!(just check query answer size)
dbpedia q0.sql gstore比virtuoso慢，因为过滤花了很久，而且候选集很大，之后又要反复判断
bsbm100000 self3.sql   pre_filter too costly
3204145 for ?v0    1 for ?v1  total 3204145
maybe we should start from ?v1 or using p2s or p2o(maybe p2so first, then pre_filter, then generate)
self5.sql 中也是preFilter时间太长
可以考虑视情况进行preFilter，或者研究下开销为何有时那么大
或许可以不用，或者仍然全用s2p或改判断条件，或者只过滤单度顶点的边约束

virtuoso: dbpedia2014 这个测试很慢，容易出问题，最好单列
100441525 ms to load
and the size is 17672699904/1000-39846 KB
50ms for q0.sql
217ms for q1.sql
210ms for q2.sql
23797ms for q3.sql
5536ms for q4.sql
2736ms for q5.sql
9515231ms for q9.sql

virtuoso: bsbm_10000
40137ms to load ,the size is 635437056/1000-39846 KB

#### using multi-join and stream, compared with jena
gstore performs worser than jena in these cases:
bsbm series: self1.sql, sellf3.sql, self8.sql (self4,5,6)
dbpedia series: q3.sql, q4.sql, q5.sql (q9.sql)
lubm series: q0.sql, q2.sql, q13.sql, q16.sql
watdiv series: C1.sql, F3.sql 

#### performance of vstree have great impact on join process, due to the candidate size
the build time is 3 orders...it seems not directly related with the length of signature
(so we can extend to better length!)

#### BSBM: when query contains "^^"(self0.sql), gstore output nothing. and when the results contain "^^"(self3.sql, self8.sql), gstore will omit the thing linked by "^^".
(this causes miss match because the other three dbms support this symbol. Virtuoso, however, can deal with queries containing "^^"
 but will not output "^^..." in results)

#### sesame does not support lubm(invalid IRI),  and unable to deal with too large datasets like dbpedia2014, watdiv_300M, bsbm_100000...

- - -

# DEBUG

error when use query "...." > ans.txt in gconsole

- - -

# BETTER

#### 在BasicQuery.cpp中的encodeBasicQuery函数中发现有pre_id==-1时就可以直接中止查询，返回空值！

#### 将KVstore模块中在堆中寻找Node*的操作改为用treap实现(或多存指针避开搜索？)

#### 无法查询谓词，因为VSTREE中只能过滤得到点的候选解，如果有对边的查询是否可以分离另加考虑(hard to join)。(或集成到vstree中, add 01/10 at the beginning to divide s/o and p. however, result is 11 after OR, predicates are not so many, so if jump into s/o branch, too costly. and ho wabout the information encoding?)

- - -

# DOCS:

#### how about STL:
http://www.zhihu.com/question/38225973?sort=created
http://www.zhihu.com/question/20201972
http://www.oschina.net/question/188977_58777

- - -

# WARN

重定义问题绝对不能忍受，现已全部解决（否则会影响其他库的使用,vim的quickfix也会一直显示）
(因为和QueryTree冲突，最终搁置)
类型不匹配问题也要注意，尽量不要有（SparqlLexer.c的多字节常量问题）
变量定义但未使用，对antlr3生成的解析部分可以不考虑（文件太大，自动生成，影响不大）
以后最好使用antlr最新版（支持C++的）来重新生成，基于面向对象，防止与Linux库中的定义冲突！
（目前是在重定义项前加前缀）

- - -

# KEEP

大量循环内的语句必须尽可能地优化!!!

- gStore also use subgraph homomorphism!

- always use valgrind to test and improve

- 此版本用于开发，最终需要将整个项目转到gStore仓库中用于发布。转移时先删除gStore中除.git和LICENSE外的所有文件，然后复制即可（不要复制LICENSE，因为版本不同；也不用复制NOTES.md，因为仅用于记录开发事项）

- build git-page for gStore

- 测试时应都在热启动的情况下比较才有意义！！！gStore应该开-O2优化，且注释掉-g以及代码中所有debug宏

- 新算法在冷启动时时间不理想，即便只是0轮join开销也很大，对比时采用热启动

- 不能支持非连通图查询(应该分割为多个连通图)

- - -

# ADVICE

#### 数值型查询 实数域 [-bound, bound] 类型很难匹配，有必要单独编码么？    数据集中不应有范围    Query中编码过滤后还需验证
x>a, x<b, >=, <=, a<x<b, x=c
vstree中遇到"1237"^^<...integer>时不直接取字符串，而是转换为数值并编码
难点：约束条件转换为析取范式， 同一变量的约束，不同变量间的约束

#### 如何用RDF表示社交网络，以及社交应用的并发问题

#### construct a paper, revisit gStore: join method, encoding way, query plan

#### 阅读分析PG源代码

#### 单元测试保证正确性和效率，valgrind分析性能瓶颈与内存泄露，重复越多的部分越应该深入优化(甚至是汇编级别)

#### we can try on watdiv_1000/watdiv_2000/watdiv_1000M/freebase/bio2rdf

#### limited depth recursive encoding strategy: build using topological ordering, how about updates? (append neighbor vertices encodings) (efficience and correctness?)

#### 目前的vstree按B+树形式实现，其实没有必要将一个节点的标签存两次。可以父节点放child子节点放entry，也可以在父节点存entry数组，根节点的entry单独提出。
(这样需要从上到下考虑)

#### 论文中的vs*tree结构其实更复杂，节点之间有很多边相连，每层做一次子图匹配，过滤效果应该会更好，之后join代价很可能也会更小。目前的vstree仿照B+树实现更简单，每次单独过滤出一个节点的候选集，之后再统一join，但效果差强人意。

#### 在index_join中考虑所有判断情况、一次多边、交集/过滤等等，multi_join不动

#### 在join时两个表时有太多策略和条件，对大的需要频繁使用的数据列可考虑建立BloomFilter进行过滤

#### not operate on the same db when connect to local server and native!

#### VStree部分的内存和时间开销都很大，测试gstore时应打印/分析各部分的时间。打印编码实例用于分析和测试，如何划分或运算空间(异或最大或夹角最大，立方体，只取决于方向而非长度)

#### full_test中捕捉stderr信息（重要信息如时间结果应该确保是标准输出？），结果第一行是变量名（有select *时应该和jena分列比）

#### Can not operate on the same db when connect to local server and native!

#### auto关键字和智能指针？

#### 实现内存池来管理内存？

#### join可以不按照树序，考虑评估每两个表的连接代价
1. 用机器学习（对查询分类寻找最优，梯度下降，调参）评估深搜顺序的好坏
2. 压缩字符串：整体控制是否启动（比如安装时），同时/不同时用于内存和硬盘。对单个string根据结构判断是否压缩？（一个标志位）关键词映射？string相关操作主要是比较，相关压缩算法必须有效且不能太复杂！
3. 实现对谓词的查询（再看论文）
4. 将查询里的常量加入变量集，否则可能不连通而无法查询，也可能影响join效率。如A->c, B->c，本来应该是通过c相连的子图才对，但目前的gstore无法识别。 

#### 考虑使用并行优化：load过程能否并行？vstree过滤时可多个点并行进行，另外join时可用pipeline策略，每得到一个就传给后续去操作。

#### 写清楚每个人的贡献


