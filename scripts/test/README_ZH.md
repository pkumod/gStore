# gStore测试帮助文档

## 1、前置条件

你需要先完成gStore的编译，编译请参考项目首页的快速开始部分，确保在gStore目录下能能正常执行一下指令

```bash
bin/gconsole -v
# gStore version: 1.4 Source distribution
# Copyright (c) 2016-2026, pkumod and topgraph and/or its affiliates.
```

## 2、测试脚本

gStore测试主要脚本包括

- gtest.sh：基本功能测试，包括构建、记载、查询、卸载、删除；
- basic_test.sh：正确性验证测试，包括bbug、lubm、num、small四个数据集的构建、加载、删除、导入、查询、卸载、删除；
- parser_test.sh：解析验证测试，测试数据集位于scripts/test/parser_test；
- update_test.sh：数据更新测试，并验证更新数据的正确性；
- transaction_test.sh：事务操作测试，包括开启、执行、提交、回滚，并验证数据的准确性；
- api_test.sh：HTTP API接口测试，涵盖常用的27类接口测试。

## 3、测试步骤

你可以选择全量测试或某个单项测试，具体测试步骤如下：

**全量测试**

全量测试会依次执行上述五个脚本，且会自动启停gserver服务

```bash
# 请确保当前所在位置为GSTORE_HOME目录下
sh scripts/test/test.sh
```

**单项测试**

单项测试需要先手动启动gserver服务，启动命令如下

```bash
# 请确保当前所在位置为GSTORE_HOME目录下
# -b表示在后台运行
bin/gserver -b

# IP black list enabled.
# [==================================================] Load cache done!   
# Server started at port 9000
```

再执行具体的测试脚本

```bash
# 请确保当前所在位置为GSTORE_HOME目录下
sh scripts/test/basic_test.sh
```

测试完成后，如需要关闭gserver服务可执行下面命令

```bash
# 请确保当前所在位置为GSTORE_HOME目录下
bin/gserver -t
# the Server [64543] stop successfully.
```
