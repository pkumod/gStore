# TODO

- full_test中捕捉stderr信息（重要信息如时间结果应该确保是标准输出？），结果第一行是变量名（有select *时应该和jena分列比）

- record the usage of Virtuoso and Sesame(maybe change their source code to be used by full_test)

- 查询控制器+数值型查询（局部和整体的区别，一个查询中是否允许同时包含数值和非数值，应该考虑全局编码还是拆分后均用B+树的范围查询）

- 完成毕业论文（图数据库查询器优化），第一部分应给出介绍和运行框架图，之后对每个部分的功能作简单说明，重点阐述自己设计/改进的模块

- - -

# TEST

using multi-join and stream, compared with jena

gstore performs worser than jena in these cases:

bsbm series: self1.sql, sellf3.sql, self8.sql (self4,5,6)

dbpedia series: q3.sql, q4.sql, q5.sql (q9.sql)

lubm series: q0.sql, q2.sql, q13.sql, q16.sql

watdiv series: C1.sql, F3.sql 

- - -

# DEBUG

- - -

# BETTER

- 在index_join中考虑所有判断情况、一次多边、交集/过滤等等，multi_join不动

- 在join时两个表时有太多策略和条件，对大的需要频繁使用的数据列可考虑建立BloomFilter进行过滤

- not operate on the same db when connect to local server and native!

- VStree部分的内存和时间开销都很大，测试gstore时应打印/分析各部分的时间。打印编码实例用于分析和测试，如何划分或运算空间(异或最大或夹角最大，立方体，只取决于方向而非长度)

- - -

# DOCS:


- how about STL:
http://www.zhihu.com/question/38225973?sort=created
http://www.zhihu.com/question/20201972
http://www.oschina.net/question/188977_58777

- - -

# WARN

重定义问题绝对不能忍受，现已全部解决（否则会影响其他库的使用,vim的quickfix也会一直显示）
类型不匹配问题也要注意，尽量不要有（SparqlLexer.c的多字节常量问题）
变量定义但未使用，对antlr3生成的解析部分可以不考虑（文件太大，自动生成，影响不大）
以后最好使用antlr最新版（支持C++的）来重新生成，基于面向对象，防止与Linux库中的定义冲突！
（目前是在重定义项前加前缀）

- - -

# KEEP

- always use valgrind to test and improve

- 此版本用于开发，最终需要将整个项目转到gStore仓库中用于发布。转移时先删除gStore中除.git和LICENSE外的所有文件，然后复制即可（不要复制LICENSE，因为版本不同；也不用复制NOTES.md，因为仅用于记录开发事项）

- build git-page for gStore

- 测试时应都在热启动的情况下比较才有意义！！！gStore应该开-O3优化，且注释掉-g以及代码中所有debug宏

- 新算法在冷启动时时间不理想，即便只是0轮join开销也很大，对比时采用热启动

- 不能支持非连通图查询

- - -

# ADVICE

- 构造极小型RDF数据用于调试

- 将KVstore模块中在堆中寻找Node*的操作改为用treap实现(或多存指针避开搜索？)

- 尽量合并相似模块：比如storage和LRUCache，多处对heap的需要等等

- 无法查询谓词，因为VSTREE中只能过滤得到点的候选解，如果有对边的查询是否可以分离另加考虑。

- Ａ->constant->B被视为不连通图不处理，是否可以在Ｑｕｅｒｙ中修改使得到结果？select ?v0 where { ?v1 <predicate> ?v2 . }

- Can not operate on the same db when connect to local server and native!

- auto关键字和智能指针？

- 实现内存池来管理内存？

- 在BasicQuery.cpp中的encodeBasicQuery函数中发现有pre_id==-1时就可以直接中止查询，返回空值！

- 能不能不用转换到表连接来获得结果，目前只相当于用图算法做了一遍预处理！

- join可以不按照树序，考虑评估每两个表的连接代价
1. 用机器学习（对查询分类寻找最优，梯度下降，调参）评估深搜顺序的好坏
2. 压缩字符串：整体控制是否启动（比如安装时），同时/不同时用于内存和硬盘。对单个string根据结构判断是否压缩？（一个标志位）关键词映射？string相关操作主要是比较，相关压缩算法必须有效且不能太复杂！
3. 实现对谓词的查询（再看论文）
4. 将查询里的常量加入变量集，否则可能不连通而无法查询，也可能影响join效率。如A->c, B->c，本来应该是通过c相连的子图才对，但目前的gstore无法识别。 

