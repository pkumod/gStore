git commit: http://www.cnblogs.com/ctaodream/p/6066694.html
<type>(<scope>) : <subject>
<空行>
<body>
<空行>
<footer>
type:
+ fix :修复bug  
+ doc : 文档改变
+ style : 代码格式改变
+ refactor :某个已有功能重构
+ perf :性能优化
+ test :增加测试
+  build :改变了build工具 如 grunt换成了 npm
+ revert: 撤销上一次的 commit 
scope:用来说明此次修改的影响范围 可以随便填写任何东西，commitizen也给出了几个 如：location 、browser、compile，不过我推荐使用
all ：表示影响面大 ，如修改了网络框架  会对真个程序产生影响
loation： 表示影响小，某个小小的功能
module：表示会影响某个模块 如登录模块、首页模块 、用户管理模块等等
subject: 用来简要描述本次改动，概述就好了
body:具体的修改信息 应该尽量详细
footer：放置写备注啥的，如果是 bug ，可以把bug id放入

---

# NOTICE

一般在实践应用中不是单机，而是CS模式，因为API也是基于CS模式的。
那么在CS模式中，可以禁用server端对query结果的输出，提升效率
方式是控制Util/Util.h中OUTPUT_QUERY_RESULT宏的开启

在使用gserver时，不能在数据库没有unload时再用gbuild或其他命令修改数据库，仅限于C/S模式
将IRC聊天放到gstore文档上，freenode #gStore

storage中大量使用long类型，文件大小也可能达到64G，最好在64位机器上运行。
在将unsigned转换为long long或者unsigned long long的时候要注意补全问题，long类型不知是否类似情况
也许可以考虑Bstr中不存length只存str(内存中还有对齐的开销)，但对于特别长的串来说可能strlen过于耗时

# 推广

必须建立一个官方网站，可以展示下团队、demo，需要建立社区/论坛并维护
另外要有桌面应用或者网页应用，以可视化的方式操作数据库，类似virtuoso和neo4j那种
server  118.89.115.42 gstore-pku.com

---

1. gstore开发应用需要一个强大的中间层，设计范式
2. 组建configure，单机和分布式合并，以后直接在合并的基础上做修改
考虑使用hbase，结合云平台

---

论文：新的join策略，特殊的子图同态问题，如何选择顺序
动态估价的评估函数要考虑方向性，因为可能含literal变量，对应的候选集大小不可靠，只能单向。
但每条边总是含subject的，所以每条边总是可以备选的。不过问题是literal变量应该先做还是后做

另一种过滤方式：直接用key-value索引，比如?x-?y-constant，可能就比较适合，如果用vstree先过滤出的候选集太大的话
考虑对线状查询或星形查询做特殊处理，这里的形状仅指需要join的部分
比如?x-?y-constant, why not just use key-value to generate sequentially
但这种没有考虑到更远的约束，可能导致不少中间解是无效的，实际上线状图的拼接顺序也不一定是从两端开始

---

# 数据库可恢复性、事务与日志

以后可能要支持事务，那样就需要日志能够支持UNDO, REDO等操作，可以以后根据需要再修改。
数据库多版本，可以指定恢复到一个带标签的版本（比如时间作为标签，用户指定标签）


# 并行策略- 线程控制模块

不宜使用并行框架，可使用C的pthread，boost的thread库，或者启用C++11，gcc编译器需要高于4.8.1才能完整支持C++11
但boost的安装更麻烦，而且没有C++11安全，所以综合考虑，还是选择C++11
而且现在C++14标准都已经出来，11标准也已经得到比较广的应用了，而像openmp这种高级并行框架也是不合适的，因为不方便统筹规划每一线程
但如果只在 Linux 下编程，不用考虑平台兼容性，那么 C++11 的 thread 的附加值几乎为零（我认为它过度设计了，同时损失了一些功能），你自己把 Pthreads 封装成一个简单好用的线程库只需要两三百行代码，用十几个 pthreads 函数实现 4 个 class：thread、mutex、lock_guard、condvar，而且 RAII 的好处也享受到了。
Linux上最好还是使用POSIX标准的pthread库，由glibc提供
编译时采用-pthread选项


fork时子进程获得父进程数据空间、堆和栈的复制
所以无论是server还是内部的并行，都应采用多线程，避免内存消耗
但开多少个用户线程，这是个很大的问题，在各个阶段用户线程的分配（一个线程不断复制）

目前B+树本身还无法并行，哪怕只有查询，也可能因为内外存交换导致并行出错。
可以让陈语嫣先做预加载，并进行两表分块并行的处理，之后再使B+树支持多线程
目前vstree没有内外存交换，只读时完全可以支持多线程
但实际应用中必然存在着读写可能和写写可能，后期必须要引入事务
对于读写问题，应该使用读写锁，采用强读者的模式，即优先将锁分配给读者
另一种方式是不用锁，为索引建立溢出页，这是一种比较好的处理并行的方式
注意：读写锁本身就比较适合读多写少的情况，另外内外存交换锁不能严重降低并发度

多用try_lock避免死锁，对于gStore系统，使用溢出页是一种思路，但多版本两阶段锁协议更全面
一个sparql语句（包括查询和更新）就是一个事务，要支持事务的并发，保证安全和可恢复

多个用户级线程对应一个内核级线程，对应同一内核线程的一个用户级线程发生了未被处理的异常，全组线程都要崩溃。
线程总数最佳应设为(线程等待时间+线程运行时间)/线程运行时间*cpu数量   (这里应该是逻辑cpu数量，一般一个物理cpu会被处理为两个逻辑cpu)
综合进程与线程的优势：
http://blog.csdn.net/qq_16209077/article/details/52769609


线程池的实现：
http://www.cnblogs.com/cpper-kaixuan/p/3640485.html
http://blog.csdn.net/turkeyzhou/article/details/8755976
http://www.cnblogs.com/osyun/archive/2012/08/31/2664938.html
http://blog.csdn.net/revv/article/details/3248424
http://blog.csdn.net/jcjc918/article/details/50395528
用户线程总数设为逻辑cpu的两倍即可，总线程数保持不变，但各个层次的线程分配是动态调整的。
如果query少，那么一个query内部就可以用更多的线程，反之反之
gstore的IO占用率已经极高，并行读取优化不大；同时IO等待也不太长(否则在SSD上应该表现较好)，所以流水线也不能开太多
但如何使得线程在处理完一个任务后不立即退出，而是可复用？(每个线程跑while循环)
当系统闲置时，应销毁多余的线程，回收系统资源。
如果是多用户，线程数可以开得很大，因为某用户的线程等待时间可能很长
设定线程数上限，指定线程种类，超出上限的任务应该等待
http://blog.csdn.net/infoworld/article/details/8670951

??server针对多用户的高并发线程应该和后面具体的查询响应线程分开，前者可建立百千(等待时间可能非常长)，然后detached?
每个user的请求被挂载到线程池的job队列中？处理完成后再将结果返还给对应用户？

暂时可以先支持只读事务的并行，以及一个事务内部的并行处理，这需要处理内外存交换导致的不一致问题
内外存交换除了换入换出外还可能有堆的更新问题，这需要加锁，但加锁会导致阻塞
是否需要一个专门的缓存管理模块
多线程中，各个kvtree最好和vstree保持一致，否则很容易出问题(比如一颗kvtree读了修改之前的value，随后在vstree上却又读了修改之后的value)


# TODO

Http通讯中answer的\n\t问题，可能是因为没有url encode的关系
返回结果是否需要进行encode？如果是json是没有问题的
但若一个string中本身就含有空格或\t，还能正确传输么？
因为返回的结果不是URL，所以可以不处理

triple num改为unsigned long long，争取单机最大支持到100亿数据集，只要entity等数目不超过20亿。
triple数目的类型应该为long long
---
解决方法：对于ID2string，仍然用char*和unsigned，但对于s2xx p2xx o2xx，应该用long long*和unsigned来表示，这样最高可支持到40亿triple
注意：在B+树中是以long long*的方式存，但读出后应该全部换成unsigned*和unsigned搭配的方式(最长支持20亿个po对)
UBSTR: 类型bstr的length问题也需要解决 如果把类型直接改成long long，空间开销一下子就上升了一倍
解决方法：对于ID2string，仍然用char*和unsigned，但对于s2xx p2xx o2xx，应该用unsigned long long*和unsigned来表示，这样最高可支持到40亿triple
(其实这个不是特别必要，很少会有这种情况，我们处理的triple数目一般限制在20亿，就算是type这种边，po对数也就是跟entity数目持平，很难达到5亿)
---
那么是否可以调整entity与literal的分界线，如果entity数目一般都比literal数目多的话
直接把literal从大到小编号，可在ID模块中指定顺序，这样每个Datbase模块应该有自己独特的分界线，其他模块用时也需要注意
编号完成之后可以取剩下的编号的中间值作为边界，这样entity跟literal都有一定的增长空间


加锁后就不必因为交换而调堆，写的时候要锁住整棵树，因为得从root节点锁住（整颗树用一把读写锁）
可以考虑不使用锁，而是在node中用一位来标志是否被锁住，锁住则不能交换出去或写入，但可以继续查看内容
在操作缓存数组的时候，也要考虑多线程冲突问题
---
B+树中的heap策略是否真的高效?? 交换时与其update heap权值，不如直接加锁，或者如果是堆顶元素不应换出，可以先移除后插入。update heap中查找太耗时
kvstore的heap可以通过沉底的方法保证多线程一致么？

kvstore变慢：s2o这种索引的性能不关键，关键在于压缩后sp2o和op2s的性能查了不少！是否应二分查找pre，好像s2po中不同的pre并不多，避免线性查找
但根据排查，sp2o不是慢在得到po list后找p的过程，而是慢在读叶节点的过程，按理说这是应该比找sp要快的
也可以考虑为B+树节点设计两套block，因为大block跳数少，小block更能有效利用空间。
在同一个文件中很难处理两套block，但可以用两套文件，分别对应一套block，设计高位来支持逻辑地址，判断属于哪个文件

缓存是先申请先得到足够的，设置安全上限，如总共100G，已用不能超过90G，因为还有很多零散的内存是没有统计的
---
缓存管理模块，负责主要模块的缓存申请和释放，包括各个数据库等，调度各棵树的内存，不负责树内部的管理
线程管理模块和缓存管理模块都应该掌控全局
全局只写内存(除非内存放不下而发生内外存交换)，空闲时或者定期刷到磁盘，但如果为了可恢复性就要记log，这是很耗时的。
BufferManager和ThreadManager在最底层，新建GstoreApplication统领全局，无论是main里面的应用还是server应用，都调用GA里的函数进行处理
NOTICE: BufferManager只支持大体的管理，最好是预申请一大片缓存或者总体申请报销，零零碎碎的new/malloc不用管，避免反复调用开销太大
GA里面可以管理多个数据库，第一件事就是启动GA，读取init.conf进行配置，包含当前数据库列表的set，新建时已存在则提示，加载时不存在则报错
注意保持API不变
每个数据库在一个时刻只能被一个GA使用，因为若是两个GA同时使用，内存独立，会有各种预料不到的问题。一般使用都是CS模式，所以一个GstoreApplication就足够了。
通过加锁，数据库被GA使用时则加锁，设置标记，不等待，避免server导入数据库后又用gbuild重建数据库
---
树索引的内外存可以和读写共用读写锁，即要换出外存或写入时都要获得读写锁中的写锁
(因为更新操作不多，所以锁等待时间不必过于在意，但树节点过多导致的锁开销可能很大)
(树从上到下获得锁，走严格的树协议，原来的storage堆替换策略也可以继续用，不过要加上锁的保护)
一般来说，保证弱一致性就可以了。

开发专门对kvstore的性能测试工具，包括增删查等，默认是用dbpedia170M数据集，需要修改makefile和test/kvstore_test.cpp
join缓存   vstree交换(先把capacity设小)    
成组插删还没完全支持和测试，需要修改KVstore.cpp

hulin添加join缓存，这样的话两表拼接就不适合多线程并行，本身中间表那种添加方式就很难并行化（要改表要加锁，不好）
后面实现全新的join，两表采用索引拼接的方式，就可用多线程并行，但递归调整得在此轮的所有两表拼接完成后（后面再考虑调整时如何并行）
索引拼接的坏处：当重复ID很少时空间开销可能比中间表还大；拼接完需要递归更新直到全局收敛，次数是中间表的行数乘上列数；编程复杂度高。
实际上大部分图都是稀疏图，基本不会出现重复ID，所以还是用中间表的方式比较好(中间表上的处理也已经非常完善)
join_two里面可以分块并行，但不能用太多线程，在末尾添加新记录时要加锁


gstore后续需要开发的地方：
事务操作：最小粒度是一个sparql/BGP一个事务，要支持workload(多个查询)为一个事务   最终一致性加锁就行了，顺序一致性则要考虑各种先后关系回滚等等，多版本两阶段锁协议
多领域多库解决方案。
owl文件，如何定义一个范式转换规则，变成高效可用的RDF格式
select from和select graph有何不同？考虑支持N-quad格式，第四列是一个graph label，说明这条边属于哪个graph
是否可以直接分成多个数据库来处理，各个数据库之间无边相连，同一查询可能包含多个数据库，比如比较字面值等等，类似关系数据库的表拼接。
(这样的话查询就不仅属于一个数据库，那么应该新建Application层，并把GeneralEvaluation上升到Application层中)

任务分配：
---
数据库连接池 保持连接而不是每次都用socket(http?，用boost库)
分页查询(先将整个查询结果缓存，需要考虑内外存交换)
陈佳棋的任务：s和p对应同一个实体，应该先重命名，再过滤。还有一种情况是两者只是名字相同，实则并无关系
高阶谓词逻辑  <type> <type> <predicate>
---
陈佳棋找人负责：
模仿海量图大作业，基于gStore开发一个社交应用，要求可以批量导入且实时查询
查询级别的缓存（测试时可将查询复制后再随机打乱）
多查询优化
---
王力博：
安全备份 数据库的多版本备份，动态删除
gserver for multiple users, not load db for each connection, but get the same db pointer
assign the pointer to the user who require this db, and ensure that each db is just kept once in memory
有什么办法去检测一个db是否存在呢？(首先要支持导入多个数据库)
如果不存在  就新建一个  再进行查询  如果存在  就直接进行查询
gserver-gclient if gclient quit(without unload), then restart, there maybe exist too many gserver process(so 3305 is occupied)
or the reason maybe gserver still dealing with the previous job, then a new connection and a new job comes
如何避免整个server崩溃或卡死，无论单个查询/建立等操作遇到任何问题
---
胡琳：
彭鹏师兄的数据集bug
优化谓词查询，谓词少而entity/literal多，所以先过滤得到谓词的解是一种可以考虑的策略
以谓词为节点，以s/o为信息，来过滤得到谓词的结果
需要一个查询计划进行选择，可能有些?p应该先做，有些?s/?o应该先做
---
陈语嫣：
网站设计以及和外包方的联系
之后用pthread将join_two函数内部的拼接并行化，先实现一个最基本的版本即可
---
张雨的任务：单起点单终点的正则表达式路径问题，如果是多起点多终点？
---
WARN：B+树删除时，向旁边兄弟借或者合并，要注意兄弟的定义，应该是同一父节点才对！
考虑使用 sigmod2016 那篇图同构的论文方法，实现一套join
但那个是基于路径的，BFS和索引连接的思想值得借用，可作为另外一套join方法
@hulin
叶子节点是否也可以先过滤先join，或者说非核心节点，sparql查询图中算度数时是否不要考虑常量？
新的方法：基于谓词的频率动态调整顺序，但中间结果的保留是另一个问题，是用中间表还是索引连接，是否需要multiJoin？
另外超级点和超级边的概念也需要被提出
---
李荆的任务：
考虑出入度数，编码为1的个数？应该不用，在编码的邻居信息中能够得到体现。
第二步编码应该更长，点和边对应着放在一起。按出入边分区，一步点，二步边分区和二步点。
对一个节点保留其最长链信息没啥用，因为数据图基本是连通的，最长链就是图的最长链。
多步编码不应分开，而应在编码逐一扩展，第二步可以依旧保留详细信息，最好用更长编码，因为信息更多。
可能有相同的谓词对应多个邻居，同样的谓词只要保留一个即可，不同邻居可以重合。
第三步可以只记谓词，第四步可以只记边的出入向。
vstree并行过滤
---
实现其他的join思路，比如基于过滤效果

如何在preFilter和join的开销之间做平衡
preFilter中的限制条件是否过于严格

寻找查询图的特征，分类做查询计划：
先对于每个查询，确定各部分的开销比例

fix the full_test:how to sum the db size right?
for virtuoso, better to reset each time and the given configure file(need to reserver the temp logs)

load过程先导入满足内存的内容，或者先来几轮搜索但不输出结果，避免开头的查询要读磁盘。vstree直接全导入内存？
先完成合并测试，再测lubm500M和bsbm500M  -- 90 server

jemalloc??

各版本对比的表格中应加一列几何平均数，现实中大多数查询是简单查询，最好还有一个平均数，对应着把数据做归一化后求和
dbpedia q6现在应该统计并对比结果了，包含在测试结果中

在改善kvstore后，build过程明显加快了很多，现在vstree的建立时间成了瓶颈
preFilter中不仅要看谓词，也要看表大小以及度数，有些节点最好过滤！！！

允许同时使用多个数据库，查询时指明数据库，这样的话可以支持from，但各个数据库是相互独立的；
添加scala API;
git管理开发；

从下往上建立B+树和vstree树
加快更新操作，如insert和delete等，是否考虑增加修改的源操作
删除时数据全部删光会出大问题，保留空树？或者插入时考虑为空树的情况？
---
更新量太多可直接重建索引，少量更新可写到overflow page中，在系统闲置时合并
比如维护一个insert和delete记录索引，每次需要查多个，判断关系
同一triple在同一索引中最多出现一次：原来已有/没有，先删后插，先插后删时等价的？
若原来已有，插入应该被放弃？若原来没有，删除应该被放弃？

常量triple怎么办，有些点比较重要必须精确过滤，不仅仅看谓词还要看度数等等
---
评估函数用机器学习的方式？学习参数？学习模型？
但机器学习对新问题没有一个基本的界，不像数学化的评估函数，对任何情况都能保证一个上界
(目前数据库里面很少有人利用机器学习来估价)

URI是唯一的，RDF图中不存在相同标签的节点，无法合并相似节点，也没法利用自同构的预处理加快查询
DBpedia最新的数据集，原来的相对太小了
count函数的优化：深搜计数即可，不必生成中间表
B+树每个节点内部添加索引，对keys分段？


是否可以考虑把literal也作为vstree中的节点，加快过滤？
join过程中生成literal可能开销大，且可以考虑用周围边对literal进行过滤。。。
但sp2o生成的literal链应该不会很长？

join buffer:use array instead of map, an ID can be in several columns, id2string maybe error(different pre, different list)!
use Bstr[2^16], only for entity, also as buffers(after intersecting with candidates)
if in id range, then use, otherwise generate(not update?) (when merging with hulin, discuss if exists)
Method:scan all linking point from left to right to find a node without literals(if not?), and buffer for this column
scan the column and get the maxID and minID, set minID as offset and only need to build a maxID-minID array
(the interval smaller, the better, how about using hash-conflict-list here)

set different buffer size for different trees in build/query, for example, sp2o and op2s with 16G, p2so with 16G
set string2id and id2string as 1-2G

测试时最好也统计出最长的string，和string的总大小

---

方正智汇：调用gStore做后台数据库，需要定义范式，比如领域、模型等等
递归删除的效率问题：比如删除一整个模型(没有关系数据库删除整个表快)
可以考虑加一个域的约束，根据这种边来全部删除
domain可用前缀表表达，但x可属于多个domain

批量插入批量修改的规模很大
需要支持修改谓词，不会出现p p o的情况
加谓词比较好加
删类的谓词，那么实例有该谓词的都得删

模型/领域 每个entity一个领域id？ 只给class分配id？只给最上层class分配id？(这样查询太复杂，效率低)
划分多个数据库，同样考虑模式，如果领域和领域之间没有交集
如果领域之间不完全独立，不能直接划分为多个数据库！如果划分需要考虑连接，划分后可直接用于分布式数据库应用

---

bsbm300M q8.sql - new gstore worser
it is because this query is linear, which means the dfs join order not optimized, and the time spent in preFilter maybe not change
maybe using non-dfs order for join?
we need to compare the time of each part in q8.sql

用vf2来对比？借鉴它的思路？不过vf2做的是标准的子图同构，而非子图包含，需要修改
vf2与gStore相互借鉴，另一种过滤方式？
客观来说，多边时vstree还是比较好的

对B+树加锁，可以直接更新节点的rank值，使其rank值最大（最高位或次高位），解锁时清除相应位
(不过这样会引入比较大的开销，因为内存堆的修改过程是比较耗时的，需要线性的查找时间)
实际上B+树应侧重于读的性能而非读写的平衡，比如不用块结构，直接把一个节点存成一片。
也可以使得每个节点固定大小，顺序存储，所有value存成记录文件，只增不减。
query过程几乎没有修改，需要free的block几乎没有，可以直接存free块号和当前最大块号，
而无需存所有块的使用情况。也是由于这个原因，树的查询中几乎不对堆做修改，避开了耗时的堆指针查询操作。

修复：需要返回pre结果的谓词查询，首先要知道哪些谓词位于select中，这需要扫描查询

change to BestJoin using index lists and super edges:
the memory cost of join will be cut down, so we can cache more for querying!
define different cache size for build and query in makefile with -D 
?how about BFS instead of DFS?

需要一个整体的并行模块：
并行的线程总数要根据cpu的线程总数来设置，不能超过太多
三种粒度，多个sparql query的并行，一个sparql内部多个basic query的并行，一个basic query内部的join流水线，两表拼接时的分块并行
并行时对kvstore部分的影响（vstree因为基本是全内存，所以没啥影响），换入换出问题，需要加锁（禁止换出）
树协议加锁？
多query并行时join的内存开销可能超标，需要在vector申请失败时抛出异常，其他地方捕捉异常处理（比如等待内存空闲）

关注jena的更新，以及其是否修复bug
每个模块的时间和提升
gStore性能表格，前后对比

to support 10 billion in a single machine, the memory should > 100G
change Bstr length to long long type
(also update anywhere related to length or strlen, such as Signature::encodeStr2Entity)
change entity/literal ID type to unsigned
+++++++++++++++++++++
新建一个ID管理模块，要求高效稳定，放在Util中
在storage, Database, VStree多个地方用到
better move StringIndex to KVstore
In unit test, insert/delete should be tested to improve coverage!!!
too many entities for lubm500M, so it is very slow


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

test on real data of applications is needed
when case is ?s p1 o       ?o p2 o
this will be viewed as two basic queries and answered separately
is this good? or if considered as one BGP, how can we do 2-hop encoding on it?

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

统计数据库大小时使用du -h还是ls -lR是个问题，但一般也不会相差太多
gStore中的内存空洞问题，一方面是可能某个块没有写满，另一方面是可能预申请的块太多。

- - -

# TEST

signature.binary in db file are not removed in program, but it should be delete to save space

单元测试：
http://www.cnblogs.com/linux-sir/archive/2012/08/25/2654557.html
http://www.ibm.com/developerworks/cn/linux/l-cn-cppunittest/
https://my.oschina.net/vaero/blog/214893

watdiv5000在建立p2?索引时出错(p2so索引太大？)
jena在测试lubm5000的查询时，似乎答案有问题，如q0.sql

若将signature编码长度由800扩大到1200+1000，build时间大幅上升，查询时间有些不变甚至加长，有些减小一半。
for q6.txt in DBpedia, the latter signature length returns the time of 16366841ms, while teh size is the same as original, 457393467
(but the size of q6.sql in jena is 17852675, which one is right?)

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

lubm5000 q1 q2 delete/insert/query
1003243 none after vstree

dbpedia q6.sql

build db error if triple num > 500M

- - -

# BETTER

#### 添加数据访问层，数据范式和生成数据访问的源码

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

- 如果select中某变量未出现在查询图中，应该对该变量返回空值还是直接认为该查询非法

- - -

# ADVICE

#### 考虑利用hdfs或者hbase，这样就可以利用各公司已有的数据库系统，但这是否会和已有的内外存交换冲突？

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


## SSD
how to set /home/ssd r/w/x for everybody
mkfs.ext4 -E stride=128,stripe-width=128 /dev/sdb
tune2fs -O ^has_journal /dev/sdb
modify /etc/fstab:
/dev/sdb /home/ssd ext4 noatime,commit=600,errors=remount-ro 0 1
to check if valid:   mount /home/ssd
ensure that IO scheduler is noop(just ssd) or deadline(ssd+disk), otherwise:
add in rc.local: echo deadline(noop,cfq) >(>>) /sys/block/sdb/queue/scheduler
(open trim if ssd and system permitted)

## 考虑支持GPU (可作为企业版的扩展功能)

## RAID

## 学习Orient DB的方式，可同时支持无模式、全模式和混合模式
ACID? neo4j GraphDB

## 单个文件的gStore？嵌入式，轻便，类似sqlite，方便移植，做成库的方式给python等调用

## 联邦数据库，避免数据重导入，上层查询分块
mysql适合存属性，而gstore适合处理关系，trinity适合做算法
gstore是否能处理各种图算法需求呢，比如对两点求最短路？比如正则路径查询？

## 没必要关闭IO缓冲同步，因为用的基本都是C语言的输入输出操作

是否可以考虑用vf2算法来作子图同构？比较效率，相互结合？
考虑按谓词频度划分，比如建立两棵sp2o树，两者的缓存大小应该不同

gStore实现的是子图同态算法，要实现子图同构，只要保证中间表扩展时每一行中不会出现重复的元素即可

Consider the use of Bloom Filter and FM-sketches

---

## DataSet

http://www.hprd.org/download/



## GIT USAGE

http://www.ruanyifeng.com/blog/2014/06/git_remote.html
https://git-scm.com/book/zh/v1/%E8%B5%B7%E6%AD%A5-%E5%88%9D%E6%AC%A1%E8%BF%90%E8%A1%8C-Git-%E5%89%8D%E7%9A%84%E9%85%8D%E7%BD%AE

To see all operations in Git, use git reflog

#### how to commit a message

package.json
http://www.json.cn/
https://www.oschina.net/news/69705/git-commit-message-and-changelog-guide
https://sanwen8.cn/p/44eCof7.html

1. commit one by one, a commit just do one thing

2. place a empty line between head and body, body and footer

3. the first letter of header should be in uppercase, and the header should not be too long, just a wonderful summary
FIX: ...       ADD:...   REF:...  代码重构   SUB:...

4. each line should not be too long, add your real name and the influence in footer(maybe cause the code struct to change)

