## API介绍

gStore通过http和Socket服务向用户提供API服务，其组件为grpc、ghttp和gserver。

### HTTP API介绍

我们现在为 gRPC 和 ghttp 提供 C++、Java、Python、PHP 和 Node.js API。请参考`api/http/cpp`、`api/http/java`、`api/http/python`、`api/http/php`和`api/http/nodejs`中的示例代码。要使用这些示例，请确保已经生成了可执行文件。**接下来，使用`bin/grpc`或`bin/ghttp`命令启动http服务。**如果您知道一个正在运行的可用http服务器，并尝试连接到它，也是可以的。然后，对于c++和java代码，您需要编译目录`api/http/cpp/example`和`api/http/java/example`中的示例代码。

**具体启动和关闭http服务可见【开发文档】-【快速入门】-【HTTP API 服务】**。

**API 服务启动完成后，grpc 访问地址如下：**

```
http://serverip:port/grpc/
```

**ghttp访问地址如下：**

```
http://serverip:port/
```

其中`serverip`为gstore服务器所在的ip地址，`port`为服务启动的端口



### Socket API介绍

我们现在为gServer提供c++（后续会逐步完善java、python、php和nodejs）API。请参考api/socket/cpp中的示例代码。要使用这些示例，请确保已经生成了可执行文件。**接下来，使用`bin/gserver -s`命令启动gServer服务。**如果您知道一个正在运行的可用gServer服务器，并尝试连接到它，也是可以的。然后，对于c++和java代码，您需要编译目录`api/socket/cpp/example`中的示例代码。

**具体启动和关闭gServer可见【开发文档】-【快速入门】-【Socket API 服务】**。

**Socket API启动完成后，就可以通过Socket进行连接了，gServer的默认端口为9000**





<!-- <div STYLE="page-break-after: always;"></div> -->
## HTTP API 结构
gStore的HTTP API放在gStore根目录的API/HTTP目录中，其内容如下:

- gStore/api/http/

  - cpp/ (the C++ API)

    - example/ (使用C++ API的示例程序)
      - Benchmark.cpp
      - GET-example.cpp
      - POST-example.cpp
      - Transaction-example.cpp
      - Makefile  (编译示例程序)
    - src/
      - GstoreConnector.cpp  (C++ API的源代码)
      - GstoreConnector.h
      - Makefile (编译和构建lib)
  - java/ (the Java API)
    - example/  (使用Java API的示例程序)
      - Benckmark.java
      - GETexample.java
      - POSTexample.java
      - Makefile
    - src/
      - jgsc/
        - GstoreConnector.java  (Java API的源代码)
      - Makefile

  - python/ (the Python API)
    - example/  (python API的示例程序)
      - Benchmark.py

      - GET-example.py

      - POST-example.py

    - src/ 
      - GstoreConnector.py
  - nodejs/ (the Node.js API)
    - example/ (使用Nodejs API的示例程序)
      - POST-example.js
      - GET-example.js
    - src
      - GstoreConnector.js (Nodejs API的源代码)

      - LICENSE

      - package.json
  - php/ (the PHP API)

    - example/  (php API的示例程序)	

      - Benchmark.php

      - POST-example.php

      - GET-example.php

    - src/

      - GstoreConnector.php  (PHP API的源代码)



- - -

<!-- <div STYLE="page-break-after: always;"></div> -->
## ghttp接口说明

### 接口对接方式

ghttp接口采用的是`http`协议，支持多种方式访问接口，如果ghttp启动的端口为`9000`,则接口对接内容如下：

接口地址：

```http
http://ip:9000/
```

接口支持 `get`请求和`post`请求，其中`get`请求参数是放在url中，`post`请求是将参数放在`body`请求

 

**注意：`GET`请求中各参数如果含有特殊字符，如？，@,&等字符时，需要采用urlencode进行编码，尤其是`sparql`参数必须进行编码**

 



### 接口列表

| 接口名称                                  | 含义                      | 备注                                                         |
| ----------------------------------------- | ------------------------- | ------------------------------------------------------------ |
| <font color=red>build（更新）</font>      | 构建图数据库              | 数据库文件需在服务器本地                                     |
| check                                     | 心跳信号                  | 检测ghttp心跳信号                                            |
| load                                      | 加载图数据库              | 将数据库加载到内存中                                         |
| unload                                    | 卸载图数据库              | 将数据库从内存中卸载                                         |
| <font color=red>monitor（更新）</font>    | 统计图数据库              | 统计指定数据库相关信息（如三元组数量等）                     |
| drop                                      | 删除图数据库              | 可以逻辑删除和物理删除                                       |
| show                                      | 显示数据库列表            | 显示所有数据库列表                                           |
| usermanage                                | 用户管理                  | 新增、删除、修改用户信息                                     |
| showuser                                  | 显示所有用户列表          | 显示所有用户列表信息                                         |
| userprivilegemanage                       | 用户权限管理              | 新增、删除、修改用户权限信息                                 |
| userpassword                              | 修改用户密码              | 修改用户密码                                                 |
| backup                                    | 备份数据库                | 备份数据库信息                                               |
| <font color=red>backuppath（新增）</font> | 获取备份数据库路径        | 返回默认备份路径下./backups所有的备份文件列表                |
| restore                                   | 还原数据库                | 还原数据库信息                                               |
| query                                     | 查询数据库                | 包括查询、删除、插入                                         |
| export                                    | 导出数据库                | 导出数据库为NT文件                                           |
| login                                     | 登陆数据库                | 用于验证用户名和密码                                         |
| begin                                     | 启动事务                  | 事务启动，需要与tquery配合使用                               |
| tquery                                    | 查询数据库（带事务）      | 带事务模式的数据查询（仅限于insert和delete）                 |
| commit                                    | 提交事务                  | 事务完成后提交事务                                           |
| rollback                                  | 回滚事务                  | 回滚事务到begin状态                                          |
| <font color=red>txnlog（更新）</font>     | 获取transaction的日志信息 | 以json返回transcation的日志信息                              |
| checkpoint                                | 将数据写入磁盘            | 当对数据库进行了insert或delete操作后，需要手动执行checkpoint |
| testConnect                               | 测试连接性                | 用于检测ghttp是否连接                                        |
| getCoreVersion                            | 获取gStore版本号          | 获取gStore版本号                                             |
| batchInsert                               | 批量插入数据              | 批量插入NT数据                                               |
| batchRemove                               | 批量删除数据              | 批量删除NT数据                                               |
| shutdown                                  | 关闭ghttp服务             |                                                              |
| querylog                                  | 获取query的日志信息       | 以json返回query的日志信息                                    |
| querylogdate                              | 获取query日志的日期列表   | 查询已有query日志的日期列表                                  |
| accesslog                                 | 获取API的访问日志         | 以json返回API的访问日志信息                                  |
| accesslogdate                             | 获取API日志的日期         | 查询已有API日志的日期列表                                    |
| ipmanage                                  | 黑白名单管理              | 维护访问gstore的IP黑白名单                                   |
| funquery                                  | 查询算子函数              | 分页获取自定义算子函数列表                                   |
| funcudb                                   | 管理算子函数              | 算子函数的新增、修改、删除、编译                             |
| funreview                                 | 预览算子函数              | 在创建和更新时，可通过预览接口查看最后生成的算子函数源码     |
| <font color=red>upload（新增）</font>     | 上传文件                  | 支持上传的文件类型有nt、ttl、n3、rdf、txt                    |
| <font color=red>download（新增）</font>   | 下载文件                  | 支持下载gstore主目录及其子目录下的文件                       |
| <font color=red>rename（新增）</font>     | 重命名图数据库            | 修改图数据库名称                                             |
| <font color=red>stat（新增）</font>       | 查询系统资源              | 统计CPU、内存、磁盘可用空间信息                              |

### 接口详细说明

> 该节中将详细阐述各个接口的输入和输出参数，假设ghttp server的ip地址为127.0.0.1，端口为9000

**（1）build**

**简要描述**

- 根据已有的NT文件创建数据库
- 文件必须存在gStore服务器上

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                                         |
| :--------- | :--- | :----- | ------------------------------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`build`**                                |
| username   | 是   | string | 用户名                                                       |
| password   | 是   | string | 密码（明文）                                                  |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name    | 是   | string | 数据库名称（不需要.db）                                      |
| db_path    | 是   | string | 数据库文件路径（可以是绝对路径，也可以是相对路径，相对路径以gStore安装根目录为参照目录） |

**返回值**

| 参数名                                    | 类型   | 说明                                         |
| :---------------------------------------- | :----- | -------------------------------------------- |
| StatusCode                                | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg                                 | string | 返回具体信息                                 |
| <font color=red>failed_num（新增）</font> | int    | 构建失败数量                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Import RDF file to database done.",
    "failed_num": 0
}
```



**（2）check**

**简要描述**

- 检测ghttp服务是否在线

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名    | 必选 | 类型   | 说明                          |
| :-------- | :--- | :----- | ----------------------------- |
| operation | 是   | string | 操作名称，固定值为**`check`** |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "the ghttp server is running..."
}
```



**（3） load**

**简要描述**

- 将数据库加载到内存中，load操作是很多操作的前置条件，如query，monitor等
- 更新内容：增加load_csr参数，默认为false

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名                     | 必选 | 类型   | 说明                                                       |
| :------------------------- | :--- | :----- | ---------------------------------------------------------- |
| operation                  | 是   | string | 操作名称，固定值为**`load`**                               |
| username                   | 是   | string | 用户名                                                     |
| encryption                 | 否   | string | 为空，则密码为明文，为1表示用md5加密                       |
| password                   | 是   | string | 密码（明文）                                                |
| db_name                    | 是   | string | 数据库名称（不需要.db）                                    |
| <font color=red>csr</font> | 否   | string | 是否加载CSR资源，默认为0（使用高级查询函数时，需要设置为1) |

**返回值**

| 参数名                     | 类型   | 说明                                         |
| :------------------------- | :----- | -------------------------------------------- |
| StatusCode                 | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg                  | string | 返回具体信息                                 |
| <font color=red>csr</font> | string | 是否加载CSR资源                              |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database loaded successfully.",
    "csr": "1"
}
```

**（4）monitor（更新）**

**简要描述**

- 获取数据库统计信息（需要先load数据库）
- 更新内容：返回值中参数的名称调整（把含有空格和下划线的参数改用驼峰式，如：triple num -> tripleNum）

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                |
| :--------- | :--- | :----- | ----------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`monitor`**     |
| username   | 是   | string | 用户名                              |
| password   | 是   | string | 密码（明文）                         |
| encryption | 否   | string | 为空，则密码为明文，=1表示用md5加密 |
| db_name    | 是   | string | 数据库名称（不需要.db）             |

**返回值**

| 参数名                             | 类型   | 说明                                         |
| :--------------------------------- | :----- | -------------------------------------------- |
| StatusCode                         | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg                          | string | 返回具体信息                                 |
| database                           | string | 数据库名称                                   |
| creator                            | string | 创建者                                       |
| builtTime                          | string | 创建时间                                     |
| tripleNum                          | string | 三元组数量                                   |
| entityNum                          | int    | 实体数量                                     |
| literalNum                         | int    | 字符数量（属性值）                           |
| subjectNum                         | int    | 主语数量                                     |
| predicateNum                       | int    | 谓词数量                                     |
| connectionNum                      | int    | 连接数量                                     |
| <font color=red>diskUsed</font>    | int    | 磁盘空间（MB）                               |
| <font color=red>subjectList</font> | Array  | 实体类型统计                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "database": "test_lubm",
    "creator": "root",
    "builtTime": "2021-08-27 21:29:46",
    "tripleNum": "99550",
    "entityNum": 28413,
    "literalNum": 0,
    "subjectNum": 14569,
    "predicateNum": 17,
    "connectionNum": 0,
    "diskUsed": 3024,
    "subjectList": [
        {
            "name": "ub:Lecturer",
            "value": 93
        },
        {
            "name": "ub:AssistantProfessor",
            "value": 146
        },
        {
            "name": "ub:University",
            "value": 1
        }
    ]
}
```



**（5）unload**

**简要描述**

- 将数据库从内存中卸载（所有的更改都会刷回硬盘）

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                |
| :--------- | :--- | :----- | ----------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`unload`**      |
| db_name    | 是   | string | 数据库名称（不需要.db）             |
| username   | 是   | string | 用户名                              |
| password   | 是   | string | 密码（明文）                         |
| encryption | 否   | string | 为空，则密码为明文，=1表示用md5加密 |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database unloaded."
}
```

**（6）drop**

**简要描述**

- 将数据库删除（可以逻辑删除，也可以物理删除）

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                                         |
| :--------- | :--- | :----- | ------------------------------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`drop`**                                 |
| username   | 是   | string | 用户名                                                       |
| password   | 是   | string | 密码（明文）                                                  |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name    | 是   | string | 数据库名称（不需要.db）                                      |
| is_backup  | 否   | string | true:代表逻辑删除，false:代表物理删除（默认为true），如果是逻辑删除，将文件夹变成.bak文件夹，用户可以通过修改文件夹名为.db，然后调用 `bin/ginit -db 数据库名` 的方式将其加入system数据库中，从而实现恢复 |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database test_lubm dropped."
}
```

**（7）show**

**简要描述**

- 显示所有数据库列表

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`show`**         |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数名          | 类型      | 说明                                         |
| :-------------- | :-------- | -------------------------------------------- |
| StatusCode      | int       | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg       | string    | 返回具体信息                                 |
| ResponseBody    | JSONArray | JSON数组（每个都是一个数据库信息）           |
| ---- database   | string    | 数据库名称                                   |
| ---- creator    | string    | 创建者                                       |
| ---- built_time | string    | 创建时间                                     |
| ---- status     | string    | 数据库状态                                   |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Get the database list successfully!",
    "ResponseBody": [
        {
            "database": "lubm",
            "creator": "root",
            "built_time": "2021-08-22 11:08:57",
            "status": "loaded"
        },
        {
            "database": "movie",
            "creator": "root",
            "built_time": "2021-08-27 20:56:56",
            "status": "unloaded"
        }
    ]
}
```

**（8）usermanage**

**简要描述**

- 对用户进行管理（包括增、删、改）

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名      | 必选 | 类型   | 说明                                                         |
| :---------- | :--- | :----- | ------------------------------------------------------------ |
| operation   | 是   | string | 操作名称，固定值为**`usermanage`**                           |
| username    | 是   | string | 用户名                                                       |
| password    | 是   | string | 密码（明文）                                                  |
| encryption  | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| type        | 是   | string | 操作类型（1：adduser，2：deleteUser，3：alterUserPassword）  |
| op_username | 是   | string | 操作的用户名                                                 |
| op_password | 是   | string | 操作的密码（如果是修改密码，该密码为要修改的密码）（如果包含特殊字符，且采用get请求，需要对其值进行URLEncode编码） |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 1004,
    "StatusMsg": "username already existed, add user failed."
}
```

**备注说明**

- 新增的用户默认具备的接口权限：`login`、`check`、`testConnect`、`getCoreVersion`、`show`、`funquery`、`funcudb`、`funreview`、`userpassword`
- 具备`query`权限的用户还同时具备以下接口权限：`query`、`monitor`
- 具备`update`权限的用户还同时具备以下接口权限：`batchInsert`、`batchRemove`、`begin`、`tquery`、`commit`、`rollback`
- 不在授权管理范围的接口权限只有root用户才能调用，如：`build`、`drop`、`usermanage`、`showuser`、`userprivilegemanage`、`txnlog`、`checkpoint`、`shutdown`、`querylog`、`accesslog`、`ipmanage`

**（9） showuser**

**简要描述**

- 显示所有用户信息

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`showuser`**     |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数名                 | 类型      | 说明                                         |
| :--------------------- | :-------- | -------------------------------------------- |
| StatusCode             | int       | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg              | string    | 返回具体信息                                 |
| ResponseBody           | JsonArray | JSON对象数组                                 |
| ---- username          | string    | 用户名                                       |
| ---- password          | string    | 密码                                         |
| ---- query_privilege   | string    | 查询权限（数据库名以逗号分隔）                |
| ---- update_privilege  | string    | 更新权限（数据库名以逗号分隔）                |
| ---- load_privilege    | string    | 加载权限（数据库名以逗号分隔）                |
| ---- unload_privilege  | string    | 卸载权限（数据库名以逗号分隔）                |
| ---- backup_privilege  | string    | 备份权限（数据库名以逗号分隔）                |
| ---- restore_privilege | string    | 还原权限（数据库名以逗号分隔）                |
| ---- export_privilege  | string    | 导出权限（数据库名以逗号分隔）                |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "ResponseBody": [
        {
            "username": "liwenjie",
            "password": "shuaige1982",
            "query_privilege": "",
            "update_privilege": "",
            "load_privilege": "",
            "unload_privilege": "",
            "backup_privilege": "",
            "restore_privilege": "",
            "export_privilege": ""
        },
        {
            "username": "root",
            "password": "123456",
            "query_privilege": "all",
            "update_privilege": "all",
            "load_privilege": "all",
            "unload_privilege": "all",
            "backup_privilege": "all",
            "restore_privilege": "all",
            "export_privilege": "all"
        }
    ]
}
```



**（10）userprivilegemanage**

**简要描述**

- 对用户权限进行管理（包括增、删、改）

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名      | 必选 | 类型   | 说明                                                         |
| :---------- | :--- | :----- | ------------------------------------------------------------ |
| operation   | 是   | string | 操作名称，固定值为**`userprivilegemanage`**                  |
| username    | 是   | string | 用户名                                                       |
| password    | 是   | string | 密码（明文）                                                  |
| encryption  | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| type        | 是   | string | 操作类型（1：add privilege，2：delete privilege， 3：clear privilege ） |
| op_username | 是   | string | 操作的用户名                                                 |
| privileges  | 否   | string | 需要操作的权限序号（多个权限使用逗号 `,` 分隔，如果是clear Privilege 可以为空）1:query, 2:load, 3:unload, 4:update, 5:backup, 6:restore, 7:export |
| db_name     | 否   | string | 需要操作的数据库（如果是clearPrivilege可以为空）             |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "add privilege query successfully. \r\nadd privilege load successfully. \r\nadd privilege unload successfully. \r\nadd privilege update successfully. \r\nadd privilege backup successfully. \r\n"
}
```

**（11） userpassword**

**简要描述**

- 修改用户密码

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名      | 必选 | 类型   | 说明                                 |
| :---------- | :--- | :----- | ------------------------------------ |
| operation   | 是   | string | 操作名称，固定值为**`userpassword`** |
| username    | 是   | string | 用户名                               |
| password    | 是   | string | 密码                                 |
| encryption  | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| op_password | 是   | string | 新密码（明文）                       |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 1004,
    "StatusMsg": "change password done."
}
```



**（12）backup**

**简要描述**

- 对数据库进行备份

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名      | 必选 | 类型   | 说明                                                         |
| :---------- | :--- | :----- | ------------------------------------------------------------ |
| operation   | 是   | string | 操作名称，固定值为**backup**                                 |
| username    | 是   | string | 用户名                                                       |
| password    | 是   | string | 密码（明文）                                                  |
| encryption  | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name     | 是   | string | 需要操作的数据库                                             |
| backup_path | 否   | string | 备份文件路径（可以是相对路径，也可以是绝对路径,相对路径以gStore根目录为参考），默认为gStore根目录下的backups目录 |

**返回值**

| 参数名         | 类型   | 说明                                            |
| :------------- | :----- | ----------------------------------------------- |
| StatusCode     | int    | 返回值代码值（具体请参考附表：返回值代码表）    |
| StatusMsg      | string | 返回具体信息                                    |
| backupfilepath | string | 备份文件路径（该值可以作为restore的输入参数值） |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database backup successfully.",
    "backupfilepath": "./backups/lubm.db_210828211529"
}
```

**（13）backuppath（新增）**

**简要描述**

- 获取数据库在默认备份路径下的所有备份文件

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**backuppath**     |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| db_name    | 是   | string | 需要查询的数据库名称                 |

**返回值**

| 参数名     | 类型   | 说明                                            |
| :--------- | :----- | ----------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表）    |
| StatusMsg  | string | 返回具体信息                                    |
| paths      | Array  | 备份文件路径（该值可以作为restore的输入参数值） |

**返回示例**

```json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "paths": [
        "./backups/lubm.db_220828211529",
        "./backups/lubm.db_221031094522"
    ]
}
```

**（14）restore**

**简要描述**

- 对备份数据库进行还原

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名      | 必选 | 类型   | 说明                                                         |
| :---------- | :--- | :----- | ------------------------------------------------------------ |
| operation   | 是   | string | 操作名称，固定值为**`restore`**                              |
| username    | 是   | string | 用户名                                                       |
| password    | 是   | string | 密码（明文）                                                  |
| encryption  | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name     | 是   | string | 需要操作的数据库                                             |
| backup_path | 是   | string | 备份文件完整路径【带时间戳的】（可以是相对路径，也可以是绝对路径，相对路径以gStore根目录为参考） |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database lumb restore successfully."
}
```

**（15）query**

**简要描述**

- 对数据库进行查询

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                                         |
| :--------- | :--- | :----- | ------------------------------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`query`**                                |
| username   | 是   | string | 用户名                                                       |
| password   | 是   | string | 密码（明文）                                                  |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name    | 是   | string | 需要操作的数据库                                             |
| format     | 否   | string | 结果集返回格式（可选值有：json，html和file），默认是json     |
| sparql     | 是   | string | 要执行的sparql语句（如果是get请求的话，sparql需要进行url编码） |

**返回值**

| 参数名      | 类型   | 说明                                         |
| :---------- | :----- | -------------------------------------------- |
| StatusCode  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg   | string | 返回具体信息                                 |
| head        | JSON   | 头部信息                                     |
| results     | JSON   | 结果信息（详情请见返回示例）                 |
| AnsNum      | int    | 结果数                                       |
| OutputLimit | int    | 最大返回结果数（-1为不限制）                 |
| ThreadId    | string | 查询线程编号                                 |
| QueryTime   | string | 查询耗时（毫秒）                             |

**返回示例**

``` json
{
    "head": {
        "link": [],
        "vars": [
            "x"
        ]
    },
    "results": {
        "bindings": [
            {
                "x": {
                    "type": "uri",
                    "value": "十面埋伏"
                }
            },
            {
                "x": {
                    "type": "uri",
                    "value": "投名状"
                }
            },
            {
                "x": {
                    "type": "uri",
                    "value": "如花"
                }
            }
        ]
    },
    "StatusCode": 0,
    "StatusMsg": "success",
    "AnsNum": 15,
    "OutputLimit": -1,
    "ThreadId": "140595527862016",
    "QueryTime": "1"
}
```

**（16）export**

**简要描述**

- 对数据库进行导出

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                                         |
| :--------- | :--- | :----- | ------------------------------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`restore`**                              |
| username   | 是   | string | 用户名                                                       |
| password   | 是   | string | 密码（明文）                                                  |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name    | 是   | string | 需要操作的数据库                                             |
| db_path    | 是   | string | 导出路径（可以是相对路径，也可以是绝对路径，相对路径以gStore根目录为参考） |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| filepath   | string | 导出文件的路径                               |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Export the database successfully.",
    "filepath": "export/lubm_210828214603.nt"
}
```

**（17）login**

**简要描述**

- 登陆用户（验证用户名和密码）
- 更新内容：登录成功后将返回gStore的全路径信息RootPath

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`login`**        |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数名      | 类型   | 说明                                         |
| :---------- | :----- | -------------------------------------------- |
| StatusCode  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg   | string | 返回具体信息                                 |
| CoreVersion | string | 内核版本                                     |
| licensetype | string | 证书类型（开源版还是企业版）                 |
| RootPath    | string | gStore根目录全路径                           |
| type        | string | HTTP服务类型（固定值，为 ghttp）             |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "login successfully",
    "CoreVersion": "1.0.0",
    "licensetype": "opensource",
    "Rootpath": "/data/gstore",
    "type": "ghttp"
}
```

**（18）begin**

**简要描述**

- 开始事务

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                                         |
| :--------- | :--- | :----- | ------------------------------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`begin`**                                |
| username   | 是   | string | 用户名                                                       |
| password   | 是   | string | 密码（明文）                                                  |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name    | 是   | string | 数据库名称                                                   |
| isolevel   | 是   | string | 事务隔离等级 1:RC(read committed)  2:SI(snapshot isolation) 3:SR(seriablizable） |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| TID        | string | 事务ID(该ID非常重要，需要以这个作为参数)     |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "transaction begin success",
    "TID": "1"
}
```

**（19） tquery** 

**简要描述**

- 事务型查询

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`tquery`**       |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| db_name    | 是   | string | 数据库名称                           |
| tid        | 是   | string | 事务ID                               |
| sparql     | 是   | string | sparql语句                           |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| head       | JSON   | 头部信息（执行查询类语句时才返回此字段）     |
| results    | JSON   | 结果信息（执行查询类语句时才返回此字段）     |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success"
}
```

**（20）commit** 

**简要描述**

- 事务提交

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                |
| :--------- | :--- | :----- | ----------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`commit`**      |
| username   | 是   | string | 用户名                              |
| password   | 是   | string | 密码（明文）                         |
| encryption | 否   | string | 为空，则密码为明文，=1表示用md5加密 |
| db_name    | 是   | string | 数据库名称                          |
| tid        | 是   | string | 事务ID                              |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "transaction commit success. TID: 1"
}
```

**（21） rollback** 

**简要描述**

- 事务回滚

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                |
| :--------- | :--- | :----- | ----------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`rollback`**    |
| username   | 是   | string | 用户名                              |
| password   | 是   | string | 密码（明文）                         |
| encryption | 否   | string | 为空，则密码为明文，=1表示用md5加密 |
| db_name    | 是   | string | 数据库名称                          |
| tid        | 是   | string | 事务ID                              |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "transaction rollback success. TID: 2"
}
```

**（22）txnlog（更新）**

**简要描述**

- 获取事务日志（该功能只对root用户生效）
- 更新内容：增加分页查询参数

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`txnlog`**       |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| pageNo     | 是   | int    | 页号，取值范围1-N，默认1             |
| pageSize   | 是   | int    | 每页条数，取值范围1-N，默认10        |

**返回值**

| 参数名          | 类型      | 说明                                                         |
| :-------------- | :-------- | ------------------------------------------------------------ |
| StatusCode      | int       | 返回值代码值（具体请参考附表：返回值代码表）                 |
| StatusMsg       | string    | 返回具体信息                                                 |
| totalSize       | int       | 总数                                                         |
| totalPage       | int       | 总页数                                                       |
| pageNo          | int       | 当前页号                                                     |
| pageSize        | int       | 每页条数                                                     |
| list            | JSONArray | 日志JSON数组                                                 |
| ---- db_name    | string    | 数据库名称                                                   |
| ---- TID        | string    | 事务ID                                                       |
| ---- user       | string    | 操作用户                                                     |
| ---- state      | string    | 状态<br />COMMITED-提交<br />RUNNING-执行中<br />ROLLBACK-回滚<br />ABORTED-中止 |
| ---- begin_time | string    | 开始时间                                                     |
| ---- end_time   | string    | 结束时间                                                     |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Get Transaction log success",
    "totalSize": 2,
    "totalPage": 1,
    "pageNo": 1,
    "pageSize": 10,
    "list": [
        {
            "db_name": "lubm2",
            "TID": "1",
            "user": "root",
            "begin_time": "1630376221590",
            "state": "COMMITED",
            "end_time": "1630376277349"
        },
        {
            "db_name": "lubm2",
            "TID": "2",
            "user": "root",
            "begin_time": "1630376355226",
            "state": "ROLLBACK",
            "end_time": "1630376379508"
        }
    ]
}
```

**（23）checkpoint**

**简要描述**

- 收到将数据刷回到硬盘（使得数据最终生效）

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`checkpoint`**   |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| db_name    | 是   | string | 数据库名称                           |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database saved successfully."
}
```



**（24）testConnect**

**简要描述**

- 测试服务器可否连接（用于workbench)
- 更新：返回值中添加 type 指示 HTTP 服务类型

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`testConnect`**  |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数名      | 类型   | 说明                                         |
| :---------- | :----- | -------------------------------------------- |
| StatusCode  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg   | string | 返回具体信息                                 |
| CoreVersion | string | 内核版本号                                   |
| licensetype | string | 授权类型（开源版还是企业版）                 |
| type        | string | HTTP服务类型（固定值，为 ghttp）             |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "CoreVersion": "1.0.0",
    "licensetype": "opensource",
    "type": "ghttp"
}
```



**（25）getCoreVersion**

**简要描述**

- 获取服务器版本号（用于workbench)
- 更新：返回值中添加 type 指示 HTTP 服务类型

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                   |
| :--------- | :--- | :----- | -------------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`getCoreVersion`** |
| username   | 是   | string | 用户名                                 |
| password   | 是   | string | 密码（明文）                            |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密   |

**返回值**

| 参数名                      | 类型   | 说明                                         |
| :-------------------------- | :----- | -------------------------------------------- |
| StatusCode                  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg                   | string | 返回具体信息                                 |
| CoreVersion                 | string | 内核版本号                                   |
| <font color=red>type</font> | string | HTTP服务类型（固定值，为 ghttp）             |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "CoreVersion": "1.0.0",
    "type": "ghttp"
}
```



**（26） batchInsert**

**简要描述**

- 批量插入数据

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                                 |
| :--------- | :--- | :----- | ---------------------------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**batchInsert**                    |
| username   | 是   | string | 用户名                                               |
| password   | 是   | string | 密码（明文）                                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                 |
| db_name    | 是   | string | 数据库名                                             |
| file       | 是   | string | 要插入的数据nt文件（可以是相对路径也可以是绝对路径） |

**返回值**

| 参数名      | 类型   | 说明                                         |
| :---------- | :----- | -------------------------------------------- |
| StatusCode  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg   | string | 返回具体信息                                 |
| success_num | string | 执行成功的数量                               |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Batch insert data successfully.",
    "success_num": "25"
}
```



**（27）batchRemove**

**简要描述**

- 批量插入数据

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                                 |
| :--------- | :--- | :----- | ---------------------------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`batchRemove`**                  |
| username   | 是   | string | 用户名                                               |
| password   | 是   | string | 密码（明文）                                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                 |
| db_name    | 是   | string | 数据库名                                             |
| file       | 是   | string | 要删除的数据nt文件（可以是相对路径也可以是绝对路径） |

**返回值**

| 参数名      | 类型   | 说明                                         |
| :---------- | :----- | -------------------------------------------- |
| StatusCode  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg   | string | 返回具体信息                                 |
| success_num | int    | 执行成功的记录数                             |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Batch remove data successfully.",
    "success_num": "25"
}
```



**（28）shutdown**

**简要描述**

- 关闭ghttp

**请求URL**

- ` http://127.0.0.1:9000/shutdown `<font color="red">【注意，地址变化】</font>

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名   | 必选 | 类型   | 说明                                                        |
| :------- | :--- | :----- | ----------------------------------------------------------- |
| username | 是   | string | 用户名（该用户名默认是system）                              |
| password | 是   | string | 密码（该密码需要到服务器的system.db/password.txt文件中查看) |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
无返回值，成功则默认收不到信息（该处要完善），失败返回错误JSON信息
```



**（29）querylog**

**简要描述**

- 获取查询日志

**请求URL**

`http://127.0.0.1:9000`

**请求方式**

- GET/POST

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| ---------- | ---- | ------ | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`querylog`**     |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| date       | 是   | string | 日期，格式为yyyyMMdd                 |
| pageNo     | 是   | int    | 页号，取值范围1-N，默认1             |
| pageSize   | 是   | int    | 每页条数，取值范围1-N，默认10        |

**返回值**

| 参数               | 类型      | 说明                                         |
| :----------------- | :-------- | -------------------------------------------- |
| StatusCode         | int       | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg          | string    | 返回具体信息                                 |
| totalSize          | int       | 总数                                         |
| totalPage          | int       | 总页数                                       |
| pageNo             | int       | 当前页号                                     |
| pageSize           | int       | 每页条数                                     |
| list               | JSONArray | 日志JSON数组                                 |
| ---- QueryDateTime | string    | 查询时间                                     |
| ---- Sparql        | string    | SPARQL语句                                   |
| ---- Format        | string    | 查询返回格式                                 |
| ---- RemoteIP      | string    | 请求IP                                       |
| ---- FileName      | string    | 查询结果集文件                               |
| ---- QueryTime     | int       | 耗时(毫秒)                                   |
| ---- AnsNum        | int       | 结果数                                       |

**返回示例**

```json
{
	"StatusCode":0,
    "StatusMsg":"Get query log success",
	"totalSize":64,
	"totalPage":13,
	"pageNo":2,
	"pageSize":5,
	"list":[
		{
			"QueryDateTime":"2021-11-16 14:55:52:90ms:467microseconds",
			"Sparql":"select ?name where { ?name <不喜欢> <Eve>. }",
			"Format":"json",
			"RemoteIP":"183.67.4.126",
			"FileName":"140163774674688_20211116145552_847890509.txt",
			"QueryTime":0,
			"AnsNum":2
		}
        ......
    ]
}
```



**（30）querylogdate**

**简要描述**

- 获取gstore的查询日志的日期（用于querylog接口的date选择参数）

**请求URL**

`http://127.0.0.1:9000`

**请求方式**

- GET/POST

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| ---------- | ---- | ------ | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`querylogdate`** |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数       | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| list       | array  | 日期列表                                     |

**返回示例**

```json
{
	"StatusCode":0,
    "StatusMsg":"Get query log date success",
	"list":[
		"20220828",
        "20220826",
        "20220825",
        "20220820"
    ]
}
```



**（31） accesslog**

**简要描述**

- 获取API的访问日志

**请求URL**

`http://127.0.0.1:9000`

**请求方式**

- GET/POST

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| ---------- | ---- | ------ | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`accesslog`**    |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| date       | 是   | string | 日期，格式为yyyyMMdd                 |
| pageNo     | 是   | int    | 页号，取值范围1-N，默认1             |
| pageSize   | 是   | int    | 每页条数，取值范围1-N，默认10        |

**返回值**

| 参数            | 类型      | 说明                                         |
| :-------------- | :-------- | -------------------------------------------- |
| StatusCode      | int       | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg       | string    | 返回具体信息                                 |
| totalSize       | int       | 总数                                         |
| totalPage       | int       | 总页数                                       |
| pageNo          | int       | 当前页号                                     |
| pageSize        | int       | 每页条数                                     |
| list            | JSONArray | 日志JSON数组                                 |
| ---- ip         | string    | 访问ip                                       |
| ---- operation  | string    | 操作类型                                     |
| ---- createtime | string    | 操作时间                                     |
| ---- code       | string    | 操作结果（参考附表：返回值代码表）           |
| ---- msg        | string    | 日志描述                                     |

**返回示例**

```json
{
	"StatusCode":0,
    "StatusMsg":"Get access log success",
	"totalSize":64,
	"totalPage":13,
	"pageNo":2,
	"pageSize":5,
	"list":[
		{
            "ip":"127.0.0.1",
            "operation":"StopServer",
            "createtime":"2021-12-14 09:55:16", 
            "code":0,
            "msg":"Server stopped successfully."
        }
        ......
    ]
}
```



**（32）accesslogdate**

**简要描述**

- 获取API日志的日期（用于accesslog接口的date选择参数）

**请求URL**

`http://127.0.0.1:9000`

**请求方式**

- GET/POST

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                  |
| ---------- | ---- | ------ | ------------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`accesslogdate`** |
| username   | 是   | string | 用户名                                |
| password   | 是   | string | 密码（明文）                           |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密  |

**返回值**

| 参数       | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| list       | array  | 日期列表                                     |

**返回示例**

```json
{
	"StatusCode":0,
    "StatusMsg":"Get access log date success",
	"list":[
		"20220913",
        "20220912",
        "20220911",
        "20220818",
        "20220731",
        "20220712",
        "20220620",
    ]
}
```

**（33） ipmanage**

**简要描述**

- 黑白名单管理

**请求URL**

`http://127.0.0.1:9000`

**请求方式**

- GET/POST

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

查询黑白名单

| 参数名     | 必选 | 类型   | 说明                                 |
| ---------- | ---- | ------ | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`ipmanage`**     |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| type       | 是   | string | 操作类型，固定值为**1**              |

保存黑白名单

| 参数名     | 必选 | 类型   | 说明                                                         |
| ---------- | ---- | ------ | ------------------------------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`ipmanage`**                             |
| username   | 是   | string | 用户名                                                       |
| password   | 是   | string | 密码（明文）                                                  |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| type       | 是   | string | 操作类型，固定值为**2**                                      |
| ip_type    | 是   | string | 名单类型，1-黑名单 2-白名单                                  |
| ips        | 是   | string | IP名单（多个用**,**分割，支持范围配置，使用**-**连接如：ip1-1p2） |

```json
//保存POST示例
{
    "operation": "ipmanage",
    "username": "root",
    "password": "123456",
    "type": "2",
    "ip_type":"1",
    "ips": "192.168.1.111,192.168.1.120-192.168.1.129"
}
```

**返回值**

| 参数         | 类型       | 说明                                         |
| :----------- | :--------- | -------------------------------------------- |
| StatusCode   | int        | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg    | string     | 返回具体信息                                 |
| ResponseBody | JSONObject | 返回数据（只有查询时才返回）                 |
| ---- ip_type | string     | 名单类型，1-黑名单 2-白名单                  |
| ---- ips     | array      | 名单列表                                     |

**返回示例**

```json
// 查询黑白名单返回
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "ResponseBody": {
        "ip_type": "1",
        "ips": [
            "192.168.1.111",
            "192.168.1.120-192.168.1.129"
        ]
    }
}
// 保存黑白名单返回
{
    "StatusCode": 0,
    "StatusMsg": "success"
}
```

**（34）funquery**

**简要描述**

- 算子函数查询

**请求URL**

`http://127.0.0.1:9000`

**请求方式**

- POST

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名         | 必选 | 类型       | 说明                                 |
| -------------- | ---- | ---------- | ------------------------------------ |
| operation      | 是   | string     | 操作名称，固定值为**`funquery`**     |
| username       | 是   | string     | 用户名                               |
| password       | 是   | string     | 密码（明文）                          |
| encryption     | 否   | string     | 为空，则密码为明文，为1表示用md5加密 |
| funInfo        | 否   | JSONObject | 查询参数                             |
| ---- funName   | 否   | string     | 函数名称                             |
| ---- funStatus | 否   | string     | 状态（1-待编译 2-已编译 3-异常）     |

**返回值**

| 参数           | 类型      | 说明                                         |
| :------------- | :-------- | -------------------------------------------- |
| StatusCode     | int       | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg      | string    | 返回具体信息                                 |
| list           | JSONArray | JSON数组（如果没有数据，则不返回空数组）     |
| ---- funName   | string    | 名称                                         |
| ---- funDesc   | string    | 描述                                         |
| ---- funArgs   | string    | 参数类型（1-无K跳参数 2-有K跳参数）          |
| ---- funBody   | string    | 函数内容                                     |
| ---- funSubs   | string    | 函数子方法                                   |
| ---- funStatus | string    | 状态（1-待编译 2-已编译 3-异常）             |
| ---- lastTime  | string    | 最后编辑时间（yyyy-MM-dd HH:mm:ss）          |

**返回示例**

```json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "list": [
        {
            "funName": "demo",
            "funDesc": "this is demo",
            "funArgs": "2",
            "funBody": "{\nstd::cout<<\"uid=\"<<uid<<endl;\nstd::cout<<\"vid=\"<<vid<<endl;\nstd::cout<<\"k=\"<<k<<endl;\nreturn \"success\";\n}",
            "funSubs": "",
            "funStatus": "1",
            "lastTime": "2022-03-15 11:32:25"
        }
    ]
}
```

**（35）funcudb**

**简要描述**

- 算子函数管理（新增、修改、删除、编译）

**请求URL**

`http://127.0.0.1:9000`

**请求方式**

- POST

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名         | 必选 | 类型       | 说明                                                         |
| -------------- | ---- | ---------- | ------------------------------------------------------------ |
| operation      | 是   | string     | 操作名称，固定值为**`funcudb`**                              |
| username       | 是   | string     | 用户名                                                       |
| password       | 是   | string     | 密码（明文）                                                  |
| encryption     | 否   | string     | 为空，则密码为明文，为1表示用md5加密                         |
| type           | 是   | string     | 1:新增，2:修改，3:删除，4:编译                               |
| funInfo        | 是   | JSONObject | 算子函数                                                     |
| ---- funName   | 是   | string     | 函数名称                                                     |
| ---- funDesc   | 否   | string     | 描述                                                         |
| ---- funArgs   | 否   | string     | 参数类型（1无K跳参数，2有K跳参数）：<font color=red>新增、修改必填</font> |
| ---- funBody   | 否   | string     | 函数内容（以`{}`包裹的内容）：<font color=red>新增、修改必填</font> |
| ---- funSubs   | 否   | string     | 子函数（可用于fun_body中调用）                               |
| ---- funReturn | 否   | string     | 返回类型（`path`:返回路径类结果，`value`:返回值类结果）：<font color=red>新增、修改必填</font> |

**返回值**

| 参数       | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

```json
{
    "StatusCode": 0,
    "StatusMsg": "success"
}
```

**（36）funreview**

**简要描述**

- 预览算子函数

**请求URL**

`http://127.0.0.1:9000`

**请求方式**

- POST

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名         | 必选 | 类型       | 说明                                                    |
| -------------- | ---- | ---------- | ------------------------------------------------------- |
| operation      | 是   | string     | 操作名称，固定值为**`funreview`**                       |
| username       | 是   | string     | 用户名                                                  |
| password       | 是   | string     | 密码（明文）                                             |
| encryption     | 否   | string     | 为空，则密码为明文，为1表示用md5加密                    |
| funInfo        | 是   | JSONObject | 算子函数                                                |
| ---- funName   | 是   | string     | 函数名称                                                |
| ---- funDesc   | 否   | string     | 描述                                                    |
| ---- funArgs   | 是   | string     | 参数类型（1无K跳参数，2有K跳参数）                      |
| ---- funBody   | 是   | string     | 函数内容（以`{}`包裹的内容）                            |
| ---- funSubs   | 是   | string     | 子函数（可用于fun_body中调用）                          |
| ---- funReturn | 是   | string     | 返回类型（`path`:返回路径类结果，`value`:返回值类结果） |

**返回值**

| 参数       | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| result     | string | 函数源码（需要进行decode转码处理）           |

**返回示例**

```json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "Result": "%23include+%3Ciostream%3E%0A%23include+%22..%2F..%2FDatabase%2FCSRUtil.h%22%0A%0Ausing+..."
}
```

**（37）upload（新增）**

**简要描述**

- 上传文件，目前支持的上传文件格式为nt、ttl、txt

**请求URL**

- ` http://127.0.0.1:9000/file/upload `<font color="red">【注意，地址变化】</font>

**请求方式**

- POST 

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`form-data`(要求RequestHeader参数Content-Type:multipart/form-data)

**参数**

| 参数名     | 必选 | 类型    | 说明                                 |
| :--------- | :--- | :------ | ------------------------------------ |
| username   | 是   | string  | 用户名                               |
| password   | 是   | string  | 密码（明文）                          |
| encryption | 否   | string  | 为空，则密码为明文，为1表示用md5加密 |
| file       | 是   | boudary | 待上传的文件的二进制文件流           |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| filepath   | string | 上传成功后返回的相对路径地址                 |

**返回示例**

```json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "filepath": "./upload/test_20221101164622.nt"
}
```

**（38） download（新增）**

**简要描述**

- 下载文件，目前支持的下载gStore根目录下的文件

**请求URL**

- ` http://127.0.0.1:9000/file/download `<font color="red">【注意，地址变化】</font>

**请求方式**

- POST 

**参数传递方式**

- post请求：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                                       |
| :--------- | :--- | :----- | ---------------------------------------------------------- |
| username   | 是   | string | 用户名（该用户名默认是system）                             |
| password   | 是   | string | 密码（明文）                                                |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                       |
| filepath   | 是   | string | 待下载的文件路径（只支持下载gstore主目录及子目录下的文件） |

**返回值**

- 以二进制流的形式响应

**返回示例**

```
Content-Range: bytes 0-389/389
Content-Type: application/octet-stream
Date: Tue, 01 Nov 2022 17:21:40 GMT
Content-Length: 389
Connection: Keep-Alive
```

**（39）rename**

**简要描述**

- 重命名数据库

**请求URL**

- ` http://127.0.0.1:9000 `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**rename**         |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| db_name    | 是   | string | 数据库名称                           |
| new_name   | 是   | string | 数据库新名称                         |

**返回值**

| 参数名         | 类型   | 说明                                         |
| :------------- | :----- | -------------------------------------------- |
| StatusCode     | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg      | string | 返回具体信息                                 |
| cup_usage      | string | CPU使用比例                                  |
| mem_usage      | string | 内存使用（单位MB）                           |
| disk_available | string | 可用磁盘空间（单位MB）                       |

**返回示例**

```json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "cup_usage": "10.596026",
    "mem_usage": "2681.507812",
    "disk_available": "12270"
}
```

**（40） stat**

**简要描述**

- 统计系统资源信息

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**stat**           |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数名         | 类型   | 说明                                         |
| :------------- | :----- | -------------------------------------------- |
| StatusCode     | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg      | string | 返回具体信息                                 |
| cup_usage      | string | CPU使用比例                                  |
| mem_usage      | string | 内存使用（单位MB）                           |
| disk_available | string | 可用磁盘空间（单位MB）                       |

**返回示例**

```json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "cup_usage": "10.596026",
    "mem_usage": "2681.507812",
    "disk_available": "12270"
}
```



**附表1  返回值代码表**

|           代码值            | 涵义                                        |
| :-------------------------: | ------------------------------------------- |
|  <font color=gree>0</font>  | Success                                     |
| <font color=red>1000</font> | The method type is  not support             |
| <font color=red>1001</font> | Authentication Failed                       |
| <font color=red>1002</font> | Check Privilege  Failed                     |
| <font color=red>1003</font> | Param is illegal                            |
| <font color=red>1004</font> | The operation conditions  are not satisfied |
| <font color=red>1005</font> | Operation failed                            |
| <font color=red>1006</font> | Add privilege Failed                        |
| <font color=red>1007</font> | Loss of lock                                |
| <font color=red>1008</font> | Transcation manage Failed                   |
| <font color=red>1100</font> | The operation is  not defined               |
| <font color=red>1101</font> | IP Blocked                                  |
|                             |                                             |



## grpc接口说明

### 接口对接方式

grpc接口采用的是`http`协议，支持多种方式访问接口，如果grpc启动的端口为`9000`,则接口对接内容如下：

接口地址：

```http
http://ip:9000/grpc
```

接口支持 `get`请求和`post`请求，其中`get`请求参数是放在url中；`post`请求是将参数放在`body`请求或者以`form`表达方式请求。

post请求方式一<font color=red>（推荐）</font>：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

post请求方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

 

**注意：`GET`请求中各参数如果含有特殊字符，如？，@,&等字符时，需要采用urlencode进行编码，尤其是`sparql`参数必须进行编码**

 



### 接口列表

| 接口名称                                  | 含义                    | 备注                                                         |
| ----------------------------------------- | ----------------------- | ------------------------------------------------------------ |
| check                                     | 心跳信号                | 检测服务心跳信号                                             |
| login                                     | 登陆数据库              | 主要是用于验证用户名和密码                                   |
| testConnect                               | 测试连接性              | 检测服务是否可连接                                           |
| getCoreVersion                            | 获取gStore版本号        | 获取gStore版本号                                             |
| ipmanage                                  | 黑白名单管理            | 维护访问gstore的IP黑白名单                                   |
| show                                      | 显示数据库列表          | 显示所有数据库列表                                           |
| load                                      | 加载图数据库            | 将数据库加载到内存中                                         |
| unload                                    | 卸载图数据库            | 将数据库从内存中卸载                                         |
| <font color=red>monitor（更新）</font>    | 统计图数据库            | 统计指定数据库相关信息（如三元组数量等）                     |
| <font color=red>build（更新）</font>      | 构建图数据库            | 数据库文件需在服务器本地                                     |
| drop                                      | 删除图数据库            | 可以逻辑删除和物理删除                                       |
| backup                                    | 备份数据库              | 备份数据库信息                                               |
| <font color=red>backuppath（新增）</font> | 获取备份数据库路径      | 返回默认备份路径下./backups所有的备份文件列表                |
| restore                                   | 还原数据库              | 还原数据库信息                                               |
| query                                     | 查询数据库              | 包括查询、删除、插入                                         |
| export                                    | 导出数据库              | 导出数据库为NT文件                                           |
| begin                                     | 启动事务                | 事务启动，需要与tquery配合使用                               |
| tquery                                    | 查询数据库（带事务）    | 带事务模式的数据查询（仅限于insert和delete）                 |
| commit                                    | 提交事务                | 事务完成后提交事务                                           |
| rollback                                  | 回滚事务                | 回滚事务到begin状态                                          |
| checkpoint                                | 将数据写入磁盘          | 当对数据库进行了insert或delete操作后，需要手动执行checkpoint |
| batchInsert                               | 批量插入数据            | 批量插入NT数据                                               |
| batchRemove                               | 批量删除数据            | 批量删除NT数据                                               |
| usermanage                                | 用户管理                | 新增、删除、修改用户信息                                     |
| showuser                                  | 显示所有用户列表        | 显示所有用户列表信息                                         |
| userprivilegemanage                       | 用户权限管理            | 新增、删除、修改用户权限信息                                 |
| userpassword                              | 修改用户密码            | 修改用户密码                                                 |
| <font color=red>txnlog（更新）</font>     | 获取事务的日志信息      | 以json返回transcation的日志信息                              |
| querylog                                  | 获取query的日志信息     | 以json返回query的日志信息                                    |
| querylogdate                              | 获取query日志的日期列表 | 查询已有query日志的日期列表                                  |
| accesslog                                 | 获取API的访问日志       | 以json返回API的访问日志信息                                  |
| accesslogdate                             | 获取API日志的日期       | 查询已有API日志的日期列表                                    |
| funquery                                  | 查询算子函数            | 分页获取自定义算子函数列表                                   |
| funcudb                                   | 管理算子函数            | 算子函数的新增、修改、删除、编译                             |
| funreview                                 | 预览算子函数            | 查看最后生成的算子函数源码                                   |
| shutdown                                  | 关闭grpc服务            |                                                              |
| <font color=red>upload(新增)</font>       | 上传文件                | 支持上传的文件类型有nt、ttl、n3、rdf、txt                    |
| <font color=red>download(新增)</font>     | 下载文件                | 支持下载gstore主目录及其子目录下的文件                       |
| <font color=red>rename(新增)</font>       | 重命名图数据库          | 修改图数据库名称                                             |
| <font color=red>stat(新增)</font>         | 查询系统资源            | 统计CPU、内存、磁盘可用空间信息                              |

### 接口详细说明

> 该节中将详细阐述各个接口的输入和输出参数，假设grpc server的ip地址为127.0.0.1，端口为9000

**（1） check**

**简要描述**

- 检测服务是否在线

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名    | 必选 | 类型   | 说明                          |
| :-------- | :--- | :----- | ----------------------------- |
| operation | 是   | string | 操作名称，固定值为**`check`** |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "the grpc server is running..."
}
```

**（2）login**

**简要描述**

- 登陆用户（验证用户名和密码）

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`login`**        |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数名      | 类型   | 说明                                         |
| :---------- | :----- | -------------------------------------------- |
| StatusCode  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg   | string | 返回具体信息                                 |
| CoreVersion | string | 内核版本                                     |
| licensetype | string | 证书类型（开源版/企业版）                    |
| RootPath    | string | gStore根目录全路径                           |
| type        | string | HTTP服务类型，固定值为grpc                   |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "login successfully",
    "CoreVersion": "1.2",
    "licensetype": "opensource",
    "Rootpath": "/data/gstore",
    "type": "grpc"
}
```

**（3）testConnect**

**简要描述**

- 测试服务器可否连接（用于workbench)

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`testConnect`**  |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数名      | 类型   | 说明                                         |
| :---------- | :----- | -------------------------------------------- |
| StatusCode  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg   | string | 返回具体信息                                 |
| CoreVersion | string | 内核版本号                                   |
| licensetype | string | 授权类型（开源版/企业版）                    |
| type        | string | HTTP服务类型，固定值为grpc                   |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "CoreVersion": "1.2",
    "licensetype": "opensource",
    "type": "grpc"
}
```

**（4）getCoreVersion**

**简要描述**

- 获取服务器版本号（用于workbench)

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                   |
| :--------- | :--- | :----- | -------------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`getCoreVersion`** |
| username   | 是   | string | 用户名                                 |
| password   | 是   | string | 密码（明文）                            |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密   |

**返回值**

| 参数名      | 类型   | 说明                                         |
| :---------- | :----- | -------------------------------------------- |
| StatusCode  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg   | string | 返回具体信息                                 |
| CoreVersion | string | 内核版本号                                   |
| type        | string | HTTP服务类型，固定值为grpc                   |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "CoreVersion": "1.2",
    "type": "grpc"
}
```

**（5）ipmanage**

**简要描述**

- 黑白名单管理

**请求URL**

`http://127.0.0.1:9000/grpc/api`

**请求方式**

- GET/POST

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

查询黑白名单

| 参数名     | 必选 | 类型   | 说明                                 |
| ---------- | ---- | ------ | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`ipmanage`**     |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| type       | 是   | string | 操作类型，固定值为**1**              |

保存黑白名单

| 参数名     | 必选 | 类型   | 说明                                                         |
| ---------- | ---- | ------ | ------------------------------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`ipmanage`**                             |
| username   | 是   | string | 用户名                                                       |
| password   | 是   | string | 密码（明文）                                                  |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| type       | 是   | string | 操作类型，固定值为**2**                                      |
| ip_type    | 是   | string | 名单类型，1-黑名单 2-白名单                                  |
| ips        | 是   | string | IP名单（多个用**,**分割，支持范围配置，使用**-**连接如：ip1-1p2） |

```json
//保存POST示例
{
    "operation": "ipmanage",
    "username": "root",
    "password": "123456",
    "type": "2",
    "ip_type":"1",
    "ips": "192.168.1.111,192.168.1.120-192.168.1.129"
}
```

**返回值**

| 参数         | 类型       | 说明                                         |
| :----------- | :--------- | -------------------------------------------- |
| StatusCode   | int        | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg    | string     | 返回具体信息                                 |
| ResponseBody | JSONObject | 返回数据（只有查询时才返回）                 |
| ---- ip_type | string     | 名单类型，1-黑名单 2-白名单                  |
| ---- ips     | array      | 名单列表                                     |

**返回示例**

```json
// 查询黑白名单返回
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "ResponseBody": {
        "ip_type": "1",
        "ips": [
            "192.168.1.111",
            "192.168.1.120-192.168.1.129"
        ]
    }
}
// 保存黑白名单返回
{
    "StatusCode": 0,
    "StatusMsg": "success"
}
```

**（6）show**

**简要描述**

- 显示所有数据库列表

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`show`**         |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数名          | 类型      | 说明                                         |
| :-------------- | :-------- | -------------------------------------------- |
| StatusCode      | int       | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg       | string    | 返回具体信息                                 |
| ResponseBody    | JSONArray | JSON数组（每个都是一个数据库信息）           |
| ---- database   | string    | 数据库名称                                   |
| ---- creator    | string    | 创建者                                       |
| ---- built_time | string    | 创建时间                                     |
| ---- status     | string    | 数据库状态                                   |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Get the database list successfully!",
    "ResponseBody": [
        {
            "database": "lubm",
            "creator": "root",
            "built_time": "2021-08-22 11:08:57",
            "status": "loaded"
        },
        {
            "database": "movie",
            "creator": "root",
            "built_time": "2021-08-27 20:56:56",
            "status": "unloaded"
        }
    ]
}
```

**（7）load**

**简要描述**

- 将数据库加载到内存中，load操作是很多操作的前置条件，如query，monitor等

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                                       |
| :--------- | :--- | :----- | ---------------------------------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`load`**                               |
| username   | 是   | string | 用户名                                                     |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                       |
| password   | 是   | string | 密码（明文）                                                |
| db_name    | 是   | string | 数据库名称（不需要.db）                                    |
| csr        | 否   | string | 是否加载CSR资源，默认为0（使用高级查询函数时，需要设置为1) |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| csr        | string | 是否加载CSR资源（0-否，1-是）                |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database loaded successfully.",
    "csr": "1"
}
```

**（8）unload**

**简要描述**

- 将数据库从内存中卸载（所有的更改都会刷回硬盘）

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                |
| :--------- | :--- | :----- | ----------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`unload`**      |
| db_name    | 是   | string | 数据库名称（不需要.db）             |
| username   | 是   | string | 用户名                              |
| password   | 是   | string | 密码（明文）                         |
| encryption | 否   | string | 为空，则密码为明文，=1表示用md5加密 |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database unloaded."
}
```

**（9）monitor**

**简要描述**

- 获取数据库统计信息（需要先load数据库）

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                |
| :--------- | :--- | :----- | ----------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`monitor`**     |
| username   | 是   | string | 用户名                              |
| password   | 是   | string | 密码（明文）                         |
| encryption | 否   | string | 为空，则密码为明文，=1表示用md5加密 |
| db_name    | 是   | string | 数据库名称（不需要.db）             |

**返回值**

| 参数名                             | 类型   | 说明                                         |
| :--------------------------------- | :----- | -------------------------------------------- |
| StatusCode                         | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg                          | string | 返回具体信息                                 |
| database                           | string | 数据库名称                                   |
| creator                            | string | 创建者                                       |
| builtTime                          | string | 创建时间                                     |
| tripleNum                          | string | 三元组数量                                   |
| entityNum                          | int    | 实体数量                                     |
| literalNum                         | int    | 字符数量（属性值）                           |
| subjectNum                         | int    | 主语数量                                     |
| predicateNum                       | int    | 谓词数量                                     |
| connectionNum                      | int    | 连接数量                                     |
| <font color=red>diskUsed</font>    | int    | 磁盘空间（MB）                               |
| <font color=red>subjectList</font> | Array  | 实体类型统计                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "database": "test_lubm",
    "creator": "root",
    "builtTime": "2021-08-27 21:29:46",
    "tripleNum": "99550",
    "entityNum": 28413,
    "literalNum": 0,
    "subjectNum": 14569,
    "predicateNum": 17,
    "connectionNum": 0,
    "diskUsed": 3024,
    "subjectList": [
        {
            "name": "ub:Lecturer",
            "value": 93
        },
        {
            "name": "ub:AssistantProfessor",
            "value": 146
        },
        {
            "name": "ub:University",
            "value": 1
        }
    ]
}
```

**（10）build**

**简要描述**

- 根据已有的NT文件创建数据库
- 文件必须存在gStore服务器上

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                                         |
| :--------- | :--- | :----- | ------------------------------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`build`**                                |
| username   | 是   | string | 用户名                                                       |
| password   | 是   | string | 密码（明文）                                                  |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name    | 是   | string | 数据库名称（不需要.db）                                      |
| db_path    | 是   | string | 数据库文件路径（可以是绝对路径，也可以是相对路径，相对路径以gStore安装根目录为参照目录） |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| failed_num | int    | 构建失败数量                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Import RDF file to database done.",
    "failed_num": 0
}
```

**（11）drop**

**简要描述**

- 将数据库删除（可以逻辑删除，也可以物理删除）

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                                         |
| :--------- | :--- | :----- | ------------------------------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`drop`**                                 |
| username   | 是   | string | 用户名                                                       |
| password   | 是   | string | 密码（明文）                                                  |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name    | 是   | string | 数据库名称（不需要.db）                                      |
| is_backup  | 否   | string | true:代表逻辑删除，false:代表物理删除（默认为true），如果是逻辑删除，将文件夹变成.bak文件夹，用户可以通过修改文件夹名为.db，然后调用 `bin/ginit -db 数据库名` 的方式将其加入system数据库中，从而实现恢复 |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database test_lubm dropped."
}
```

**（12）backup**

**简要描述**

- 对数据库进行备份

**请求URL**

- ` http://127.0.0.1:9000/ `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名      | 必选 | 类型   | 说明                                                         |
| :---------- | :--- | :----- | ------------------------------------------------------------ |
| operation   | 是   | string | 操作名称，固定值为**backup**                                 |
| username    | 是   | string | 用户名                                                       |
| password    | 是   | string | 密码（明文）                                                  |
| encryption  | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name     | 是   | string | 需要操作的数据库                                             |
| backup_path | 否   | string | 备份文件路径（可以是相对路径，也可以是绝对路径,相对路径以gStore根目录为参考），默认为gStore根目录下的backups目录 |

**返回值**

| 参数名         | 类型   | 说明                                            |
| :------------- | :----- | ----------------------------------------------- |
| StatusCode     | int    | 返回值代码值（具体请参考附表：返回值代码表）    |
| StatusMsg      | string | 返回具体信息                                    |
| backupfilepath | string | 备份文件路径（该值可以作为restore的输入参数值） |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database backup successfully.",
    "backupfilepath": "./backups/lubm.db_210828211529"
}
```

**（13）backuppath**

**简要描述**

- 获取数据库在默认备份路径下的所有备份文件

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**backuppath**     |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| db_name    | 是   | string | 需要查询的数据库名称                 |

**返回值**

| 参数名     | 类型   | 说明                                            |
| :--------- | :----- | ----------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表）    |
| StatusMsg  | string | 返回具体信息                                    |
| paths      | Array  | 备份文件路径（该值可以作为restore的输入参数值） |

**返回示例**

```json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "paths": [
        "./backups/lubm.db_220828211529",
        "./backups/lubm.db_221031094522"
    ]
}
```

**（14）restore**

**简要描述**

- 对备份数据库进行还原

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名      | 必选 | 类型   | 说明                                                         |
| :---------- | :--- | :----- | ------------------------------------------------------------ |
| operation   | 是   | string | 操作名称，固定值为**`restore`**                              |
| username    | 是   | string | 用户名                                                       |
| password    | 是   | string | 密码（明文）                                                  |
| encryption  | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name     | 是   | string | 需要操作的数据库                                             |
| backup_path | 是   | string | 备份文件完整路径【带时间戳的】（可以是相对路径，也可以是绝对路径，相对路径以gStore根目录为参考） |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database lumb restore successfully."
}
```

**（15）query**

**简要描述**

- 对数据库进行查询

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                                         |
| :--------- | :--- | :----- | ------------------------------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`query`**                                |
| username   | 是   | string | 用户名                                                       |
| password   | 是   | string | 密码（明文）                                                  |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name    | 是   | string | 需要操作的数据库                                             |
| format     | 否   | string | 结果集返回格式（可选值有：json, file, json+file），默认是json |
| sparql     | 是   | string | 要执行的sparql语句（如果是get请求的话，sparql需要进行url编码） |

**返回值**

| 参数名      | 类型   | 说明                                         |
| :---------- | :----- | -------------------------------------------- |
| StatusCode  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg   | string | 返回具体信息                                 |
| head        | JSON   | 头部信息                                     |
| results     | JSON   | 结果信息（详情请见返回示例）                 |
| AnsNum      | int    | 结果数                                       |
| OutputLimit | int    | 最大返回结果数（-1为不限制）                 |
| ThreadId    | string | 查询线程编号                                 |
| QueryTime   | string | 查询耗时（毫秒）                             |
| FileName    | string | 结果文件名称（format值为file、json+file时）  |

**返回示例**

``` json
// format: json
{
    "head": {
        "link": [],
        "vars": [
            "x"
        ]
    },
    "results": {
        "bindings": [
            {
                "x": {
                    "type": "uri",
                    "value": "十面埋伏"
                }
            },
            {
                "x": {
                    "type": "uri",
                    "value": "投名状"
                }
            },
            {
                "x": {
                    "type": "uri",
                    "value": "如花"
                }
            }
        ]
    },
    "StatusCode": 0,
    "StatusMsg": "success",
    "AnsNum": 15,
    "OutputLimit": -1,
    "ThreadId": "140595527862016",
    "QueryTime": "1"
}
```

```json
// format:file
// 结果文件根目录：%gstore_home%/query_result
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "AnsNum": 12,
    "OutputLimit": -1,
    "ThreadId": "140270360303360",
    "QueryTime": "1",
    "FileName": "140270360303360_20220914172612_258353606.txt"
}
```

**（16）export**

**简要描述**

- 对数据库进行导出

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                                         |
| :--------- | :--- | :----- | ------------------------------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`restore`**                              |
| username   | 是   | string | 用户名                                                       |
| password   | 是   | string | 密码（明文）                                                  |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name    | 是   | string | 需要操作的数据库                                             |
| db_path    | 是   | string | 导出路径（可以是相对路径，也可以是绝对路径，相对路径以gStore根目录为参考） |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| filepath   | string | 导出文件的路径                               |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Export the database successfully.",
    "filepath": "export/lubm_210828214603.nt"
}
```

**（17）begin**

**简要描述**

- 开始事务

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                                         |
| :--------- | :--- | :----- | ------------------------------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`begin`**                                |
| username   | 是   | string | 用户名                                                       |
| password   | 是   | string | 密码（明文）                                                  |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name    | 是   | string | 数据库名称                                                   |
| isolevel   | 是   | string | 事务隔离等级 1:RC(read committed)  2:SI(snapshot isolation) 3:SR(seriablizable） |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| TID        | string | 事务ID(该ID非常重要，需要以这个作为参数)     |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "transaction begin success",
    "TID": "1"
}
```

**（18）tquery** 

**简要描述**

- 事务型查询

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`tquery`**       |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| db_name    | 是   | string | 数据库名称                           |
| tid        | 是   | string | 事务ID                               |
| sparql     | 是   | string | sparql语句                           |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| head       | JSON   | 头部信息（执行查询类语句时才返回此字段）     |
| results    | JSON   | 结果信息（执行查询类语句时才返回此字段）     |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success"
}
```

**（19）commit** 

**简要描述**

- 事务提交

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                |
| :--------- | :--- | :----- | ----------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`commit`**      |
| username   | 是   | string | 用户名                              |
| password   | 是   | string | 密码（明文）                         |
| encryption | 否   | string | 为空，则密码为明文，=1表示用md5加密 |
| db_name    | 是   | string | 数据库名称                          |
| tid        | 是   | string | 事务ID                              |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Transaction commit success. TID: 1"
}
```

**（20）rollback** 

**简要描述**

- 事务回滚

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                |
| :--------- | :--- | :----- | ----------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`rollback`**    |
| username   | 是   | string | 用户名                              |
| password   | 是   | string | 密码（明文）                         |
| encryption | 否   | string | 为空，则密码为明文，=1表示用md5加密 |
| db_name    | 是   | string | 数据库名称                          |
| tid        | 是   | string | 事务ID                              |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Transaction rollback success. TID: 2"
}
```

**（21）checkpoint**

**简要描述**

- 收到将数据刷回到硬盘（使得数据最终生效）

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`checkpoint`**   |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| db_name    | 是   | string | 数据库名称                           |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database saved successfully."
}
```

**（22）batchInsert**

**简要描述**

- 批量插入数据

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                                 |
| :--------- | :--- | :----- | ---------------------------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**batchInsert**                    |
| username   | 是   | string | 用户名                                               |
| password   | 是   | string | 密码（明文）                                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                 |
| db_name    | 是   | string | 数据库名                                             |
| file       | 是   | string | 要插入的数据nt文件（可以是相对路径也可以是绝对路径） |

**返回值**

| 参数名      | 类型   | 说明                                         |
| :---------- | :----- | -------------------------------------------- |
| StatusCode  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg   | string | 返回具体信息                                 |
| success_num | string | 执行成功的数量                               |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Batch insert data successfully.",
    "success_num": "25"
}
```

**（23）batchRemove**

**简要描述**

- 批量插入数据

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                                 |
| :--------- | :--- | :----- | ---------------------------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`batchRemove`**                  |
| username   | 是   | string | 用户名                                               |
| password   | 是   | string | 密码（明文）                                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                 |
| db_name    | 是   | string | 数据库名                                             |
| file       | 是   | string | 要删除的数据nt文件（可以是相对路径也可以是绝对路径） |

**返回值**

| 参数名      | 类型   | 说明                                         |
| :---------- | :----- | -------------------------------------------- |
| StatusCode  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg   | string | 返回具体信息                                 |
| success_num | int    | 执行成功的记录数                             |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Batch remove data successfully.",
    "success_num": "25"
}
```

**（24）usermanage**

**简要描述**

- 对用户进行管理（包括增、删、改）

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名      | 必选 | 类型   | 说明                                                         |
| :---------- | :--- | :----- | ------------------------------------------------------------ |
| operation   | 是   | string | 操作名称，固定值为**`usermanage`**                           |
| username    | 是   | string | 用户名                                                       |
| password    | 是   | string | 密码（明文）                                                  |
| encryption  | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| type        | 是   | string | 操作类型（1：adduser，2：deleteUser，3：alterUserPassword）  |
| op_username | 是   | string | 被操作的用户名                                               |
| op_password | 否   | string | 被操作的用户密码，删除用户时可为空（如果是修改密码，该密码为要设置的新密码）（如果包含特殊字符，且采用get请求，需要对其值进行URLEncode编码） |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Add user done."
}
```

**备注说明**

- 新增的用户默认具备的接口权限：`login`、`check`、`testConnect`、`getCoreVersion`、`show`、`funquery`、`funcudb`、`funreview`、`userpassword`
- 具备`query`权限的用户还同时具备以下接口权限：`query`、`monitor`
- 具备`update`权限的用户还同时具备以下接口权限：`batchInsert`、`batchRemove`、`begin`、`tquery`、`commit`、`rollback`
- 不在授权管理范围的接口权限只有root用户才能调用，如：`build`、`drop`、`usermanage`、`showuser`、`userprivilegemanage`、`checkpoint`、`shutdown`、`txnlog`、`querylog`、`accesslog`、`ipmanage`

**（25） showuser**

**简要描述**

- 显示所有用户信息

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`showuser`**     |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数名                 | 类型      | 说明                                         |
| :--------------------- | :-------- | -------------------------------------------- |
| StatusCode             | int       | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg              | string    | 返回具体信息                                 |
| ResponseBody           | JsonArray | JSON对象数组                                 |
| ---- username          | string    | 用户名                                       |
| ---- password          | string    | 密码                                         |
| ---- query_privilege   | string    | 查询权限（数据库名以逗号分隔）                |
| ---- update_privilege  | string    | 更新权限（数据库名以逗号分隔）                |
| ---- load_privilege    | string    | 加载权限（数据库名以逗号分隔）                |
| ---- unload_privilege  | string    | 卸载权限（数据库名以逗号分隔）                |
| ---- backup_privilege  | string    | 备份权限（数据库名以逗号分隔）                |
| ---- restore_privilege | string    | 还原权限（数据库名以逗号分隔）                |
| ---- export_privilege  | string    | 导出权限（数据库名以逗号分隔）                |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "ResponseBody": [
        {
            "username": "test",
            "password": "123456",
            "query_privilege": "lubm10,lubm100",
            "update_privilege": "",
            "load_privilege": "lubm10,lubm100",
            "unload_privilege": "lubm10,lubm100",
            "backup_privilege": "",
            "restore_privilege": "",
            "export_privilege": ""
        },
        {
            "username": "root",
            "password": "123456",
            "query_privilege": "all",
            "update_privilege": "all",
            "load_privilege": "all",
            "unload_privilege": "all",
            "backup_privilege": "all",
            "restore_privilege": "all",
            "export_privilege": "all"
        }
    ]
}
```

**（26） userprivilegemanage**

**简要描述**

- 对用户权限进行管理（包括增、删、改）

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名      | 必选 | 类型   | 说明                                                         |
| :---------- | :--- | :----- | ------------------------------------------------------------ |
| operation   | 是   | string | 操作名称，固定值为**`userprivilegemanage`**                  |
| username    | 是   | string | 用户名                                                       |
| password    | 是   | string | 密码（明文）                                                  |
| encryption  | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| type        | 是   | string | 操作类型（1：add privilege，2：delete privilege， 3：clear privilege ） |
| op_username | 是   | string | 操作的用户名                                                 |
| privileges  | 否   | string | 需要操作的权限序号（多个权限使用逗号 `,` 分隔，如果是clear Privilege 可以为空）1:query, 2:load, 3:unload, 4:update, 5:backup, 6:restore, 7:export |
| db_name     | 否   | string | 需要操作的数据库（如果是clearPrivilege可以为空）             |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "add privilege query successfully. \r\nadd privilege load successfully. \r\nadd privilege unload successfully. \r\nadd privilege update successfully. \r\nadd privilege backup successfully. \r\n"
}
```

**（27） userpassword**

**简要描述**

- 修改用户密码

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

| 参数名      | 必选 | 类型   | 说明                                 |
| :---------- | :--- | :----- | ------------------------------------ |
| operation   | 是   | string | 操作名称，固定值为**`userpassword`** |
| username    | 是   | string | 用户名                               |
| password    | 是   | string | 密码                                 |
| encryption  | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| op_password | 是   | string | 新密码（明文）                       |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Change password done."
}
```

**（28）txnlog**

**简要描述**

- 获取事务日志

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`txnlog`**       |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| pageNo     | 是   | int    | 页号，取值范围1-N，默认1             |
| pageSize   | 是   | int    | 每页数，取值范围1-N，默认10          |

**返回值**

| 参数名          | 类型      | 说明                                                         |
| :-------------- | :-------- | ------------------------------------------------------------ |
| StatusCode      | int       | 返回值代码值（具体请参考附表：返回值代码表）                 |
| StatusMsg       | string    | 返回具体信息                                                 |
| totalSize       | int       | 总数                                                         |
| totalPage       | int       | 总页数                                                       |
| pageNo          | int       | 当前页号                                                     |
| pageSize        | int       | 每页数                                                       |
| list            | JSONArray | 日志JSON数组                                                 |
| ---- db_name    | string    | 数据库名称                                                   |
| ---- TID        | string    | 事务ID                                                       |
| ---- user       | string    | 操作用户                                                     |
| ---- state      | string    | 状态<br />COMMITED-提交<br />RUNNING-执行中 <br />ROLLBACK-回滚 <br />ABORTED-中止 |
| ---- begin_time | string    | 开始时间                                                     |
| ---- end_time   | string    | 结束时间                                                     |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Get Transaction log success",
    "totalSize": 2,
    "totalPage": 1,
    "pageNo": 1,
    "pageSize": 10,
    "list": [
        {
            "db_name": "lubm2",
            "TID": "1",
            "user": "root",
            "begin_time": "1630376221590",
            "state": "COMMITED",
            "end_time": "1630376277349"
        },
        {
            "db_name": "lubm2",
            "TID": "2",
            "user": "root",
            "begin_time": "1630376355226",
            "state": "ROLLBACK",
            "end_time": "1630376379508"
        }
    ]
}
```

**（29）querylog**

**简要描述**

- 获取查询日志

**请求URL**

`http://127.0.0.1:9000/grpc/api`

**请求方式**

- GET/POST

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| ---------- | ---- | ------ | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`querylog`**     |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| date       | 是   | string | 日期，格式为yyyyMMdd                 |
| pageNo     | 是   | int    | 页号，取值范围1-N，默认1             |
| pageSize   | 是   | int    | 每页数，取值范围1-N，默认10          |

**返回值**

| 参数               | 类型      | 说明                                         |
| :----------------- | :-------- | -------------------------------------------- |
| StatusCode         | int       | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg          | string    | 返回具体信息                                 |
| totalSize          | int       | 总数                                         |
| totalPage          | int       | 总页数                                       |
| pageNo             | int       | 当前页号                                     |
| pageSize           | int       | 每页数                                       |
| list               | JSONArray | 日志JSON数组                                 |
| ---- QueryDateTime | string    | 查询时间                                     |
| ---- RemoteIP      | string    | 请求IP                                       |
| ---- Sparql        | string    | SPARQL语句                                   |
| ---- AnsNum        | int       | 结果数                                       |
| ---- Format        | string    | 查询返回格式                                 |
| ---- FileName      | string    | 查询结果集文件                               |
| ---- QueryTime     | int       | 耗时(毫秒)                                   |
| ---- StatusCode    | int       | 执行状态码                                   |
| ---- DbName        | string    | 查询数据库名称                               |

**返回示例**

```json
{
	"StatusCode":0,
    "StatusMsg":"Get query log success",
	"totalSize":64,
	"totalPage":13,
	"pageNo":2,
	"pageSize":5,
	"list":[
		{
			"QueryDateTime":"2021-11-16 14:55:52:90ms:467microseconds",
			"Sparql":"select ?name where { ?name <不喜欢> <Eve>. }",
			"Format":"json",
			"RemoteIP":"183.67.4.126",
			"FileName":"140163774674688_20211116145552_847890509.txt",
			"QueryTime":0,
			"AnsNum":2,
            "StatusCode": 0,
            "DbName": "demo"
		}
    ]
}
```

**（30）querylogdate**

**简要描述**

- 获取gstore的查询日志的日期（用于querylog接口的date选择参数）

**请求URL**

`http://127.0.0.1:9000/grpc/api`

**请求方式**

- GET/POST

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| ---------- | ---- | ------ | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`querylogdate`** |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数       | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| list       | array  | 日期列表                                     |

**返回示例**

```json
{
	"StatusCode":0,
    "StatusMsg":"Get query log date success",
	"list":[
		"20220828",
        "20220826",
        "20220825",
        "20220820"
    ]
}
```

**（31）ccesslog**

**简要描述**

- 获取gstore的访问日志

请求URL

`http://127.0.0.1:9000/grpc/api`

**请求方式**

- GET/POST

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| ---------- | ---- | ------ | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`accesslog`**    |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| date       | 是   | string | 日期，格式为yyyyMMdd                 |
| pageNo     | 是   | int    | 页号，取值范围1-N，默认1             |
| pageSize   | 是   | int    | 每页数，取值范围1-N，默认10          |

**返回值**

| 参数            | 类型      | 说明                                         |
| :-------------- | :-------- | -------------------------------------------- |
| StatusCode      | int       | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg       | string    | 返回具体信息                                 |
| totalSize       | int       | 总数                                         |
| totalPage       | int       | 总页数                                       |
| pageNo          | int       | 当前页号                                     |
| pageSize        | int       | 每页数                                       |
| list            | JSONArray | 日志JSON数组                                 |
| ---- ip         | string    | 访问ip                                       |
| ---- operation  | string    | 操作类型                                     |
| ---- createtime | string    | 操作时间                                     |
| ---- code       | string    | 操作结果（参考附表：返回值代码表）           |
| ---- msg        | string    | 日志描述                                     |

**返回示例**

```json
{
	"StatusCode":0,
    "StatusMsg":"Get access log success",
	"totalSize":64,
	"totalPage":13,
	"pageNo":2,
	"pageSize":5,
	"list":[
		{
            "ip":"127.0.0.1",
            "operation":"StopServer",
            "createtime":"2021-12-14 09:55:16", 
            "code":0,
            "msg":"Server stopped successfully."
        }
    ]
}
```

**（32）accesslogdate**

**简要描述**

- 获取API日志的日期（用于accesslog接口的date选择参数）

**请求URL**

`http://127.0.0.1:9000/grpc/api`

**请求方式**

- GET/POST

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                  |
| ---------- | ---- | ------ | ------------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`accesslogdate`** |
| username   | 是   | string | 用户名                                |
| password   | 是   | string | 密码（明文）                           |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密  |

**返回值**

| 参数       | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| list       | array  | 日期列表                                     |

**返回示例**

```json
{
	"StatusCode":0,
    "StatusMsg":"Get access log date success",
	"list":[
		"20220913",
        "20220912",
        "20220911",
        "20220818",
        "20220731",
        "20220712",
        "20220620",
    ]
}
```

**（33）funquery**

**简要描述**

- 算子函数查询

**请求URL**

`http://127.0.0.1:9000/grpc/api`

**请求方式**

- POST

**参数传递方式**

- POST请求，参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

**参数**

| 参数名         | 必选 | 类型       | 说明                                 |
| -------------- | ---- | ---------- | ------------------------------------ |
| operation      | 是   | string     | 操作名称，固定值为**`funquery`**     |
| username       | 是   | string     | 用户名                               |
| password       | 是   | string     | 密码（明文）                         |
| encryption     | 否   | string     | 为空，则密码为明文，为1表示用md5加密 |
| funInfo        | 否   | JSONObject | 查询参数                             |
| ---- funName   | 否   | string     | 函数名称                             |
| ---- funStatus | 否   | string     | 状态（1-待编译 2-已编译 3-异常）     |

**返回值**

| 参数           | 类型      | 说明                                         |
| :------------- | :-------- | -------------------------------------------- |
| StatusCode     | int       | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg      | string    | 返回具体信息                                 |
| list           | JSONArray | JSON数组（如果没有数据，则不返回空数组）     |
| ---- funName   | string    | 名称                                         |
| ---- funDesc   | string    | 描述                                         |
| ---- funArgs   | string    | 参数类型（1-无K跳参数 2-有K跳参数）          |
| ---- funBody   | string    | 函数内容                                     |
| ---- funSubs   | string    | 函数子方法                                   |
| ---- funStatus | string    | 状态（1-待编译 2-已编译 3-异常）             |
| ---- lastTime  | string    | 最后编辑时间（yyyy-MM-dd HH:mm:ss）          |

**返回示例**

```json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "list": [
        {
            "funName": "demo",
            "funDesc": "this is demo",
            "funArgs": "2",
            "funBody": "{\nstd::cout<<\"uid=\"<<uid<<endl;\nstd::cout<<\"vid=\"<<vid<<endl;\nstd::cout<<\"k=\"<<k<<endl;\nreturn \"success\";\n}",
            "funSubs": "",
            "funStatus": "1",
            "lastTime": "2022-03-15 11:32:25"
        }
    ]
}
```

**（34）funcudb**

**简要描述**

- 算子函数管理（新增、修改、删除、编译）

**请求URL**

`http://127.0.0.1:9000`

**请求方式**

- POST

**参数传递方式**

- POST请求，参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

**参数**

| 参数名         | 必选 | 类型       | 说明                                                         |
| -------------- | ---- | ---------- | ------------------------------------------------------------ |
| operation      | 是   | string     | 操作名称，固定值为**`funcudb`**                              |
| username       | 是   | string     | 用户名                                                       |
| password       | 是   | string     | 密码（明文）                                                 |
| encryption     | 否   | string     | 为空，则密码为明文，为1表示用md5加密                         |
| type           | 是   | string     | 1:新增，2:修改，3:删除，4:编译                               |
| funInfo        | 是   | JSONObject | 算子函数                                                     |
| ---- funName   | 是   | string     | 函数名称                                                     |
| ---- funDesc   | 否   | string     | 描述                                                         |
| ---- funArgs   | 否   | string     | 参数类型（1无K跳参数，2有K跳参数）：<font color=red>新增、修改必填</font> |
| ---- funBody   | 否   | string     | 函数内容（以`{}`包裹的内容）：<font color=red>新增、修改必填</font> |
| ---- funSubs   | 否   | string     | 子函数（可用于fun_body中调用）                               |
| ---- funReturn | 否   | string     | 返回类型（`path`:单路径结果，`value`:值结果，multipath:多路径结果）：<font color=red>新增、修改必填</font> |

**返回值**

| 参数       | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

```json
{
    "StatusCode": 0,
    "StatusMsg": "Function create success."
}
```

**（35）funreview**

**简要描述**

- 预览算子函数

**请求URL**

`http://127.0.0.1:9000/grpc/api`

**请求方式**

- POST

**参数传递方式**

- POST请求，参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

**参数**

| 参数名         | 必选 | 类型       | 说明                                                         |
| -------------- | ---- | ---------- | ------------------------------------------------------------ |
| operation      | 是   | string     | 操作名称，固定值为**`funreview`**                            |
| username       | 是   | string     | 用户名                                                       |
| password       | 是   | string     | 密码（明文）                                                  |
| encryption     | 否   | string     | 为空，则密码为明文，为1表示用md5加密                         |
| funInfo        | 是   | JSONObject | 算子函数                                                     |
| ---- funName   | 是   | string     | 函数名称                                                     |
| ---- funDesc   | 否   | string     | 描述                                                         |
| ---- funArgs   | 是   | string     | 参数类型（1无K跳参数，2有K跳参数）                           |
| ---- funBody   | 是   | string     | 函数内容（以`{}`包裹的内容）                                 |
| ---- funSubs   | 是   | string     | 子函数（可用于fun_body中调用）                               |
| ---- funReturn | 是   | string     | 返回类型（`path`:单路径结果，`value`:值结果，multipath:多路径结果） |

**返回值**

| 参数       | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| Result     | string | 函数源码（需要进行decode转码处理）           |

**返回示例**

```json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "Result": "%23include+%3Ciostream%3E%0A%23include+%22..%2F..%2FDatabase%2FCSRUtil.h%22%0A%0Ausing+..."
}
```

**（36）shutdown**

**简要描述**

- 关闭grpc服务

**请求URL**

- ` http://127.0.0.1:9000/grpc/shutdown `<font color="red">【注意，地址变化】</font>

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名   | 必选 | 类型   | 说明                                                         |
| :------- | :--- | :----- | ------------------------------------------------------------ |
| username | 是   | string | 用户名（该用户名默认是system）                               |
| password | 是   | string | 密码（该密码需要到服务器的system.db/passwordxxxx.txt文件中查看，其中xxxx表示启动服务的端口号) |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Server stopped successfully."
}
```

**（37）upload**

**简要描述**

- 上传文件，目前支持的上传文件格式为nt、ttl、txt

**请求URL**

- ` http://127.0.0.1:9000/grpc/file/upload `<font color="red">【注意，地址变化】</font>

**请求方式**

- POST 

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`form-data`(要求RequestHeader参数Content-Type:multipart/form-data)

**参数**

| 参数名     | 必选 | 类型    | 说明                                 |
| :--------- | :--- | :------ | ------------------------------------ |
| username   | 是   | string  | 用户名                               |
| password   | 是   | string  | 密码（明文）                       |
| encryption | 否   | string  | 为空，则密码为明文，为1表示用md5加密 |
| file       | 是   | boudary | 待上传的文件的二进制文件流           |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| filepath   | string | 上传成功后返回的相对路径地址                 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "filepath": "./upload/test_20221101164622.nt"
}
```

**（38）download**

**简要描述**

- 下载文件，目前支持的下载gStore根目录下的文件

**请求URL**

- ` http://127.0.0.1:9000/grpc/file/download `<font color="red">【注意，地址变化】</font>

**请求方式**

- POST 

**参数传递方式**

- post请求方式：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                                       |
| :--------- | :--- | :----- | ---------------------------------------------------------- |
| username   | 是   | string | 用户名（该用户名默认是system）                             |
| password   | 是   | string | 密码（明文）                                                |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                       |
| filepath   | 是   | string | 待下载的文件路径（只支持下载gstore主目录及子目录下的文件） |

**返回值**

以二进制流的形式响应

**返回示例**

Response Headers示例如下：

``` 
Content-Range: bytes 0-389/389
Content-Type: application/octet-stream
Date: Tue, 01 Nov 2022 17:21:40 GMT
Content-Length: 389
Connection: Keep-Alive
```

**（30）rename**

**简要描述**

- 重命名数据库

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**rename**         |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                         |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| db_name    | 是   | string | 数据库名称                           |
| new_name   | 是   | string | 数据库新名称                         |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success"
}
```

**（40）stat**

**简要描述**

- 统计系统资源信息

**请求URL**

- ` http://127.0.0.1:9000/grpc/api `

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递

- POST请求

  方式一：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）

  方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**stat**           |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文）                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数名         | 类型   | 说明                                         |
| :------------- | :----- | -------------------------------------------- |
| StatusCode     | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg      | string | 返回具体信息                                 |
| cup_usage      | string | CPU使用比例                                  |
| mem_usage      | string | 内存使用（单位MB）                           |
| disk_available | string | 可用磁盘空间（单位MB）                       |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "cup_usage": "10.596026",
    "mem_usage": "2681.507812",
    "disk_available": "12270"
}
```



**附表1 返回值代码表**

|           代码值            | 涵义                                        |
| :-------------------------: | ------------------------------------------- |
|  <font color=gree>0</font>  | Success                                     |
|  <font color=red>14</font>  | Route not fund                              |
| <font color=red>1000</font> | The method type is  not support             |
| <font color=red>1001</font> | Authentication Failed                       |
| <font color=red>1002</font> | Check Privilege  Failed                     |
| <font color=red>1003</font> | Param is illegal                            |
| <font color=red>1004</font> | The operation conditions  are not satisfied |
| <font color=red>1005</font> | Operation failed                            |
| <font color=red>1006</font> | Add privilege Failed                        |
| <font color=red>1007</font> | Loss of lock                                |
| <font color=red>1008</font> | Transcation manage Failed                   |
| <font color=red>1100</font> | The operation is  not defined               |
| <font color=red>1101</font> | IP Blocked                                  |
|                             |                                             |



<div STYLE="page-break-after: always;"></div>

## C++ HTTP API

要使用C++ API，请将该短语`#include "client.h"`放在cpp代码中，具体使用如下：

**构造初始化函数**
	

	GstoreConnector(std::string serverIP,int serverPort, std::string httpType, std::string username, std::string password);
	功能：初始化   
	参数含义：[服务器IP]，[服务器上http端口]，[http服务类型], [用户名]，[密码]
	使用示例：GstoreConnector gc("127.0.0.1", 9000, "ghttp", "root", "123456");

**构建数据库：build**
	

	std::string build(std::string db_name, std::string rdf_file_path, std::string request_type);
	功能：通过RDF文件新建一个数据库
	参数含义：[数据库名称]，[.nt文件路径]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.build("lubm", "data/lubm/lubm.nt");

**加载数据库：load**

	std::string load(std::string db_name, std::string request_type);
	功能：加载你建立的数据库
	参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.load("lubm");

**停止加载数据库：unload**
	

	std::string unload(std::string db_name, std::string request_type);
	功能：停止加载数据库
	参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.unload("lubm");

**用户管理：user**

	std::string user(std::string type, std::string username2, std::string addition, std::string request_type);
	功能：添加、删除用户或修改用户的权限，必须由根用户执行操作
	1.添加、删除用户：
	参数含义：["add_user"添加用户，"delete_user"删除用户]，[用户名],[密码],[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.user("add_user", "user1", "111111");
	2.修改用户的权限：
	参数含义：["add_query"添加查询权限，"delete_query"删除查询权限，"add_load"添加加载权限，"delete_load"删除加载权限，"add_unload"添加不加载权限，"delete_unload"删除不加载权限，"add_update"添加更新权限，"delete_update"删除更新权限，"add_backup"添加备份权限，"delete_bakup"删除备份权限，"add_restore"添加还原权限，"delete_restore"删除还原权限,"add_export"添加导出权限，"delete_export"删除导出权限]，[用户名],[数据库名],[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.user("add_query", "user1", "lubm");

**显示用户：showUser**

	std::string showUser(std::string request_type);
	功能：显示所有用户
	参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.showUser();

**数据库查询：query**

	std::string query(std::string db_name, std::string format, std::string sparql, std::string request_type);
	功能：查询数据库
	参数含义：[数据库名称]，[查询结果类型json,html或text]，[sparql语句]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：
	std::string res = gc.query("lubm", "json", sparql);
	std::cout << res << std::endl;  //输出结果

**删除数据库：drop**

	std::string drop(std::string db_name, bool is_backup, std::string request_type);
	功能：直接删除数据库或删除数据库同时留下备份
	参数含义：[数据库名称]，[false不备份，true备份]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.drop("lubm", false);  //直接删除数据库不留下备份

**监控数据库：monitor**

	std::string monitor(std::string db_name, std::string request_type);
    功能：显示特定数据库的信息
    参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：gc.monitor("lubm");

**保存数据库：checkpoint**

	std::string checkpoint(std::string db_name, std::string request_type);
    功能：如果更改了数据库，保存数据库
    参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：gc.checkpoint("lubm");

**展示数据库：show**

	std::string show(std::string request_type);
    功能：显示所有已创建的数据库
    参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：gc.show();

**显示内核版本信息：getCoreVersion**

	std::string getCoreVersion(std::string request_type);
    功能：得到内核版本信息
    参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：gc.getCoreVersion();

**显示API版本信息：getAPIVersion**

	std::string getAPIVersion(std::string request_type);
	功能：得到API版本信息
	参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.getAPIVersion();

**查询数据库并保存文件：fquery**

	void fquery(std::string db_name, std::string format, std::string sparql, std::string filename, std::string request_type);
	功能：查询数据库并保留结果到文件
	参数含义：[数据库名称]，[查询结果类型json,html或text]，[sparql语句]，[文件名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.fquery("lubm", "json", sparql, "ans.txt");

**导出数据库**

	std::string exportDB(std::string db_name, std::string dir_path, std::string request_type);
	功能：导出数据库到文件夹下
	参数含义：[数据库名称]，[数据库导出的文件夹路径]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.exportDB("lubm", "/root/gStore/");



<!-- <div STYLE="page-break-after: always;"></div> -->
## Java  HTTP API

要使用Java API，请参阅gStore/api/http/java/src/jgsc/GstoreConnector.java。具体使用如下：

**构造初始化函数**

	public class GstoreConnector(String serverIP, int serverPort, String httpType, String username, String password);
	功能：初始化   
	参数含义：[服务器IP]，[服务器上http端口]，[http服务类型]，[用户名]，[密码]
	使用示例：GstoreConnector gc = new GstoreConnector("127.0.0.1", 9000, "ghttp", "root", "123456");

**构建数据库：build**

	public String build(String db_name, String rdf_file_path, String request_type);
	功能：通过RDF文件新建一个数据库
	参数含义：[数据库名称]，[.nt文件路径]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.build("lubm", "data/lubm/lubm.nt");

**加载数据库：load**

	public String load(String db_name, String request_type);
	功能：加载你建立的数据库
	参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.load("lubm");

**停止加载数据库：unload**

	public String unload(String db_name, String request_type);
	功能：停止加载数据库
	参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.unload("lubm");

**用户管理：user**

	public String user(String type, String username2, String addition, String request_type);
	功能：添加、删除用户或修改用户的权限，必须由根用户执行操作
	1.添加、删除用户：
	参数含义：["add_user"添加用户，"delete_user"删除用户]，[用户名],[密码],[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.user("add_user", "user1", "111111");
	2.修改用户的权限：
	参数含义：["add_query"添加查询权限，"delete_query"删除查询权限，"add_load"添加加载权限，"delete_load"删除加载权限，"add_unload"添加不加载权限，"delete_unload"删除不加载权限，"add_update"添加更新权限，"delete_update"删除更新权限，"add_backup"添加备份权限，"delete_bakup"删除备份权限，"add_restore"添加还原权限，"delete_restore"删除还原权限,"add_export"添加导出权限，"delete_export"删除导出权限]，[用户名],[数据库名],[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.user("add_query", "user1", "lubm");

**显示用户：showUser**

	public String showUser(String request_type);
	功能：显示所有用户
	参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.showUser();

**数据库查询：query**

	public String query(String db_name, String format, String sparql, String request_type);
	功能：查询数据库
	参数含义：[数据库名称]，[查询结果类型json,html或text]，[sparql语句]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：
	String res = gc.query("lubm", "json", sparql);
	System.out.println(res); //输出结果

**删除数据库：drop**

	public String drop(String db_name, boolean is_backup, String request_type);
	功能：直接删除数据库或删除数据库同时留下备份
	参数含义：[数据库名称]，[false不备份，true备份]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.drop("lubm", false);  //直接删除数据库不留下备份

**监控数据库：monitor**

	public String monitor(String db_name, String request_type);
    功能：显示特定数据库的信息
    参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：gc.monitor("lubm");

**保存数据库：checkpoint**

	public String checkpoint(String db_name, String request_type);
    功能：如果更改了数据库，保存数据库
    参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：gc.checkpoint("lubm");

**展示数据库：show**

	public String show(String request_type);
    功能：显示所有已创建的数据库
    参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：gc.show();

**显示内核版本信息：getCoreVersion**

	public String getCoreVersion(String request_type);
    功能：得到内核版本信息
    参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：gc.getCoreVersion();

**显示API版本信息：getAPIVersion**

	public String getAPIVersion(String request_type);
    功能：得到API版本信息
    参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：gc.getAPIVersion();

**查询数据库并保存文件：fquery**

	public void fquery(String db_name, String format, String sparql, String filename, String request_type);
    功能：查询数据库并保留结果到文件
    参数含义：[数据库名称]，[查询结果类型json,html或text]，[sparql语句]，[文件名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：gc.fquery("lubm", "json", sparql, "ans.txt");

**导出数据库**

	public String exportDB(String db_name, String dir_path, String request_type);
    功能：导出数据库到文件夹下
    参数含义：[数据库名称]，[数据库导出的文件夹路径]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：gc.exportDB("lubm", "/root/gStore/");

<!-- <div STYLE="page-break-after: always;"></div> -->
## Python HTTP API

要使用Python API，请参阅gStore/api/http/python/src/GstoreConnector.py。具体使用如下：

**构造初始化函数**

	def __init__(self, ip, port, username, password, http_type='ghttp'):
	功能：初始化   
	参数含义：[服务器IP]，[服务器上http端口]，[用户名]，[密码]，[http服务类型，默认为ghttp]
	使用示例：gc = GstoreConnector.GstoreConnector("127.0.0.1", 9000, "root", "123456")

**心跳检测: check**

	 def check(self, request_type='GET'):
	 功能：检测http服务是否在线
	 参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	 使用示例：res = gc.check()

**登录授权: login**

	def login(self, username, password, request_type='GET'):
	功能: 登录HTTP API服务。如果在调用其他API接口时提示"the ip has been locked"，可通过login接口登录成功后解除IP锁定
	Parameter definition：[username],[password],[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example: res = gc.login("root","123456")

**构建数据库：build**

	def build(self, db_name, db_path, request_type='GET'):
	功能：通过RDF文件新建一个数据库
	参数含义：[数据库名称]，[RDF文件路径]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：res = gc.build("lubm", "data/lubm/lubm.nt")

**删除数据库：drop**

	def drop(self, db_name, is_backup, request_type='GET'):
	功能：直接删除数据库或删除数据库同时留下备份
	参数含义：[数据库名称]，[false不备份，true备份]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：res = gc.drop("lubm", false)

**加载数据库：load**

	def load(self, db_name, csr='0', request_type='GET'):
	功能：加载已构建的数据库
	参数含义：[数据库名称]，[是否记载CSR资源，默认不加载]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：res = gc.load("lubm")

**卸载数据库：unload**

	def unload(self, db_name, request_type='GET'):
	功能：将数据库从内存中卸载（所有的更改都会刷回硬盘）
	参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：res = gc.unload("lubm")

**展示数据库：show**

	def show(self, request_type='GET'):
    功能：显示所有已创建的数据库
    参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：res = gc.show()

**监控数据库：monitor**   	

	def monitor(self, db_name, request_type='GET'):
    功能：显示特定数据库的信息
    参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：res = gc.monitor("lubm")

**数据库查询：query**

	def query(self, db_name, format, sparql, request_type='GET'):
	功能：查询数据库
	参数含义：[数据库名称]，[查询结果类型json,html或text]，[sparql语句]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：
    sparql = "select ?x ?p where { ?x ?p <FullProfessor0>. }"
	res = gc.query("lubm", "json", sparql)
	print(res)

**显示用户：showUser**

	def showuser(self, request_type='GET'):
	功能：显示所有用户
	参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：res = gc.showUser()

**用户管理：usermanage**

	def usermanage(self, type, op_username, op_password, equest_type='GET'):
	功能：添加、删除用户或修改用户的密码，必须由根用户执行操作
	1.添加、删除用户：
	参数含义：["1"添加用户, "2"删除用户, "3"修改密码]，[用户名], [密码], [请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：res = gc.user("1", "test", "123456")

**修改用户权限：userprivilegemanage**

	def userprivilegemanage(self, type, op_username, privileges, db_name, request_type='GET'):
    功能：修改用户的权限
	参数含义：["1" 新增权限, "2" 删除权限, "3" 清空权限], [用户名], ["1" 查询, "2" 加载, "3" 卸载, "4" 更新, "5" 备份, "6" 还原,  "7" 导出],  [请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：
	给用户test添加lubm库的所有权限
	res = gc.userprivilegemanage("1", "test", "1,2,3,4,5,6,7", "lubm")
	删除用户test对lubm库的"备份"和"还原"权限
	res = gc.userprivilegemanage("2", "test", "5,7", "lubm")
	清空用户test的所有权限 
	res = gc.userprivilegemanage("3", "test", "", "")

**获取内核版本信息：getCoreVersion**

	def getCoreVersion(self, request_type='GET'):
    功能：得到内核版本信息
    参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：res = gc.getCoreVersion()

**开启事务: begin**

	def begin(self, db_name, isolevel, request_type='GET'):
	功能：开启事务，将返回事务id用于后续的事务操作
	参数含义：[数据库名称], ["1" seriablizable "2" snapshot isolation "3" read committed], [请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.begin("lubm", "1")

**执行事务查询：fquery**

	def tquery(self, db_name, tid, sparql, request_type='GET'):
    功能：执行事务类型的查询
    参数含义：[数据库名称]，[事务id]，[sparql语句]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：gc.fquery("lubm", "1", sparql)

**提交事务: commit**

	def commit(self, db_name, tid, request_type='GET'):
	功能：提交事务 
	参数含义：[数据库名称]，[事务id], [请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.commit("lubm", "1")

**回滚事务: rollback**

	def rollback(self, db_name, tid, request_type='GET'):
	功能：回滚事务
	参数含义：[数据库名称]，[事务id], [请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.rollback("lubm", "1")

**获取事务日志: getTransLog**

	def getTransLog(self, page_no=1, page_size=10, request_type='GET'):
	功能：获取事务日志信息
	参数含义：[数据库名称], [页数], [每页条数], [请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.getTransLog(1, 10)

**保存数据库：checkpoint**

	def checkpoint(self, db_name, request_type='GET'):
    功能：将数据刷回到硬盘，一般在执行事务相关操作后调用
    参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：res = gc.checkpoint("lubm")

**导出数据库：exportDB**

	def exportDB(self, db_name, db_path, request_type='GET'):
    功能：导出数据库到文件夹下
    参数含义：[数据库名称]，[数据库导出的文件夹路径]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：res = gc.exportDB("lubm", "/root/gStore/")

<!-- <div STYLE="page-break-after: always;"></div> -->
## Nodejs  HTTP API

在使用Nodejs API之前，键入`npm install request`并`npm install request-promise`在nodejs文件夹下添加所需的模块。

要使用Nodejs API，请参阅gStore/api/http/nodejs/GstoreConnector.js。具体使用如下：

**构造初始化函数**

	class GstoreConnector(ip = '', port, httpType = 'ghttp', username = '', password = '');
	功能：初始化   
	参数含义：[服务器IP]，[服务器上http端口]，[http服务类型]，[用户名]，[密码]
	使用示例：gc =  new GstoreConnector("127.0.0.1", 9000, "ghttp", "root", "123456");

**构建数据库：build**

	async build(db_name = '', rdf_file_path = '', request_type);
	功能：通过RDF文件新建一个数据库
	参数含义：[数据库名称]，[.nt文件路径]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：res = gc.build("lubm", "data/lubm/lubm.nt");

**加载数据库：load**

	async load(db_name = '', request_type);
	功能：加载你建立的数据库
	参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：res = gc.load("lubm");

**停止加载数据库：unload**

	async unload(db_name = '', request_type);
	功能：停止加载数据库
	参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：res = gc.unload("lubm");

**用户管理：user**

	async user(type = '', username2 = '' , addition = '' , request_type);
	功能：添加、删除用户或修改用户的权限，必须由根用户执行操作
	1.添加、删除用户：
	参数含义：["add_user"添加用户，"delete_user"删除用户]，[用户名],[密码],[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：res = gc.user("add_user", "user1", "111111");
	2.修改用户的权限：
	参数含义：["add_query"添加查询权限，"delete_query"删除查询权限，"add_load"添加加载权限，"delete_load"删除加载权限，"add_unload"添加不加载权限，"delete_unload"删除不加载权限，"add_update"添加更新权限，"delete_update"删除更新权限，"add_backup"添加备份权限，"delete_bakup"删除备份权限，"add_restore"添加还原权限，"delete_restore"删除还原权限,"add_export"添加导出权限，"delete_export"删除导出权限]，[用户名],[数据库名],[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：res = gc.user("add_query", "user1", "lubm");

**显示用户：showUser**

	async showUser(request_type);
	功能：显示所有用户
	参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：res = gc.showUser();

**数据库查询：query**

	async query(db_name = '', format = '' , sparql = '' , request_type);
	功能：查询数据库
	参数含义：[数据库名称]，[查询结果类型json,html或text]，[sparql语句]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：
	res = gc.query("lubm", "json", sparql);
	console.log(JSON.stringify(res,",")); //输出结果

**删除数据库：drop**

	async drop(db_name = '', is_backup , request_type);
	功能：直接删除数据库或删除数据库同时留下备份
	参数含义：[数据库名称]，[false不备份，true备份]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：res = gc.drop("lubm", false);  //直接删除数据库不留下备份

**监控数据库：monitor**   	

	async monitor(db_name = '', request_type);
    功能：显示特定数据库的信息
    参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：res = gc.monitor("lubm");

**保存数据库：checkpoint**

	async checkpoint(db_name = '', request_type);
    功能：如果更改了数据库，保存数据库
    参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：res = gc.checkpoint("lubm");

**展示数据库：show**

	async show(request_type);
    功能：显示所有已创建的数据库
    参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：res = gc.show();

**显示内核版本信息：getCoreVersion**	

	async getCoreVersion(request_type);
    功能：得到内核版本信息
    参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：res = gc.getCoreVersion();

**显示API版本信息：getAPIVersion**			

	async getAPIVersion(request_type);
	功能：得到API版本信息	
	参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：res = gc.getAPIVersion();

**查询数据库并保存文件：fquery**

	async fquery(db_name = '', format = '' , sparql = '' , filename = '' , request_type);
	功能：查询数据库并保留结果到文件
	参数含义：[数据库名称]，[查询结果类型json,html或text]，[sparql语句]，[文件名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：gc.fquery("lubm", "json", sparql, "ans.txt");

**导出数据库**

	async exportDB(db_name = '' , dir_path = '' , request_type); 
	功能：导出数据库到文件夹下
	参数含义：[数据库名称]，[数据库导出的文件夹路径]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：res = gc.exportDB("lubm", "/root/gStore/");

<!-- <div STYLE="page-break-after: always;"></div> -->
## PHP HTTP API

要使用Php API，请参阅gStore/api/http/php/src/GstoreConnector.php。具体使用如下：

**构造初始化函数**

	class GstoreConnector($ip, $port, $httpType, $username, $password)
	功能：初始化   
	参数含义：[服务器IP]，[服务器上http端口]，[http服务类型]，[用户名]，[密码]
	使用示例：$gc = new GstoreConnector("127.0.0.1", 9000, "ghttp", "root", "123456");

**构建数据库：build**

	function build($db_name, $rdf_file_path, $request_type)
	功能：通过RDF文件新建一个数据库
	参数含义：[数据库名称]，[.nt文件路径]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：
	$res = $gc->build("lubm", "data/lubm/lubm.nt");
	echo $res . PHP_EOL;

**加载数据库：load**

	function load($db_name, $request_type)
	功能：加载你建立的数据库
	参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：
	$ret = $gc->load("test");
	echo $ret . PHP_EOL;

**停止加载数据库：unload**

	function unload($db_name, $request_type)
	功能：停止加载数据库
	参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：
	$ret = $gc->unload("test");
	echo $ret . PHP_EOL;

**用户管理：user**

	function user($type, $username2, $addition, $request_type)
	功能：添加、删除用户或修改用户的权限，必须由根用户执行操作
	1.添加、删除用户：
	参数含义：["add_user"添加用户，"delete_user"删除用户]，[用户名],[密码],[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：
	$res = $gc->user("add_user", "user1", "111111");
	echo $res . PHP_EOL;
	2.修改用户的权限：
	参数含义：["add_query"添加查询权限，"delete_query"删除查询权限，"add_load"添加加载权限，"delete_load"删除加载权限，"add_unload"添加不加载权限，"delete_unload"删除不加载权限，"add_update"添加更新权限，"delete_update"删除更新权限，"add_backup"添加备份权限，"delete_bakup"删除备份权限，"add_restore"添加还原权限，"delete_restore"删除还原权限,"add_export"添加导出权限，"delete_export"删除导出权限]，[用户名],[数据库名],[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：
	$res = $gc->user("add_user", "user1", "lubm");
	echo $res . PHP_EOL;

**显示用户：showUser**

	function showUser($request_type)
	功能：显示所有用户
	参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：
	$res = $gc->showUser();
	echo $res. PHP_EOL;

**数据库查询：query**

	function query($db_name, $format, $sparql, $request_type)
    功能：查询数据
	参数含义：[数据库名称]，[查询结果类型json,html或text]，[sparql语句]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：
	$res = $gc->query("lubm", "json", $sparql);
	echo $res. PHP_EOL; //输出结果

**删除数据库：drop**

	function drop($db_name, $is_backup, $request_type)
	功能：直接删除数据库或删除数据库同时留下备份
	参数含义：[数据库名称]，[false不备份，true备份]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：
	$res = $gc->drop("lubm", false); //直接删除数据库不留下备份
	echo $res. PHP_EOL;             

**监控数据库：monitor**  	

	function monitor($db_name, $request_type)
    功能：显示特定数据库的信息
    参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：$res = $gc->monitor("lubm");echo $res. PHP_EOL;

**保存数据库：checkpoint**

	function checkpoint($db_name, $request_type)
    功能：如果更改了数据库，保存数据库
    参数含义：[数据库名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：$res = $gc->checkpoint("lubm");echo $res. PHP_EOL;

**展示数据库：show**

	function show($request_type)
    功能：显示所有已创建的数据库
    参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：$res = $gc->show();echo $res. PHP_EOL;

**显示内核版本信息：getCoreVersion**

	function getCoreVersion($request_type)
    功能：得到内核版本信息
    参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
    使用示例：$res = $gc->getCoreVersion();echo $res. PHP_EOL;

**显示API版本信息：getAPIVersion**	

	function getAPIVersion($request_type)
	功能：得到API版本信息	
	参数含义：[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：
	$res = $gc->getAPIVersion();
	echo $res. PHP_EOL;

**查询数据库并保存文件：fquery**

	function fquery($db_name, $format, $sparql, $filename, $request_type)
	功能：查询数据库并保留结果到文件
	参数含义：[数据库名称]，[查询结果类型json,html或text]，[sparql语句]，[文件名称]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：$gc->fquery("lubm", "json", $sparql, "ans.txt");

**导出数据库**

	function exportDB($db_name, $dir_path, $request_type)
	功能：导出数据库到文件夹下
	参数含义：[数据库名称]，[数据库导出的文件夹路径]，[请求类型"GET"和"post",如果请求类型为“GET”，则可以省略]
	使用示例：$res = $gc->exportDB("lubm", "/root/gStore/");

<!-- <div STYLE="page-break-after: always;"></div> -->
## gServer接口说明

### 接口对接方式

gServer接口采用的是`socket`协议，支持多种方式访问接口，如果Main目录下的gserver启动的端口为`9000`,则接口对接内容如下：

接口地址：

```http
http://ip:9000/
```

接口支持输入一个json格式的参数列表，如下所示：

```json
{"op": "[op_type]", "[paramname1]": "[paramvalue1]", "[paramname2]": "[paramvalue2]"}
```

 



### 接口列表

| 接口名称 | 含义           | 备注                       |
| -------- | -------------- | -------------------------- |
| build    | 构建图数据库   | 数据库文件需在服务器本地   |
| load     | 加载图数据库   | 将数据库加载到内存中       |
| unload   | 卸载图数据库   | 将数据库从内存中卸载       |
| drop     | 删除图数据库   | 可以逻辑删除和物理删除     |
| show     | 显示数据库列表 | 显示所有数据库列表         |
| query    | 查询数据库     | 包括查询、删除、插入       |
| stop     | 关闭服务端     | 只有root用户可以操作       |
| close    | 关闭客户端连接 | 处理客户端关闭连接请求     |
| login    | 登陆数据库     | 主要是用于验证用户名和密码 |
|          |                |                            |


### 接口详细说明

> 该节中将详细阐述各个接口的输入和输出参数，假设gserver的ip地址为127.0.0.1，端口为9000

**（1） build 创建数据库**

**简要描述**

- 根据已有的NT文件创建数据库
- 文件必须存在gStore服务器上

**请求ip**

- ` 127.0.0.1 `

**请求端口号**

- ` 9000 `

**参数传递方式**

- 以`JSON`结构传递

**参数**

| 参数名  | 必选 | 类型   | 说明                                                         |
| :------ | :--- | :----- | ------------------------------------------------------------ |
| op      | 是   | string | 操作名称，固定值为**build**                                  |
| db_name | 是   | string | 数据库名称（不需要.db）                                      |
| db_path | 是   | string | 数据库文件路径（可以是绝对路径，也可以是相对路径，相对路径以gStore安装根目录为参照目录） |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Import RDF file to database done."
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**（2） load**

**简要描述**

- 将数据库加载到内存中，load操作是很多操作的前置条件，如query等

**请求ip**

- ` 127.0.0.1 `

**请求端口号**

- ` 9000 `

**参数传递方式**

- 以`JSON`结构传递

**参数**

| 参数名  | 必选 | 类型   | 说明                       |
| :------ | :--- | :----- | -------------------------- |
| op      | 是   | string | 操作名称，固定值为**load** |
| db_name | 是   | string | 数据库名称（不需要.db）    |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Load database successfully."
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**（3）unload**

**简要描述**

- 将数据库从内存中卸载（所有的更改都会刷回硬盘）

**请求ip**

- ` 127.0.0.1 `

**请求端口号**

- ` 9000 `

**参数传递方式**

- 以`JSON`结构传递

**参数**

| 参数名  | 必选 | 类型   | 说明                         |
| :------ | :--- | :----- | ---------------------------- |
| op      | 是   | string | 操作名称，固定值为**unload** |
| db_name | 是   | string | 数据库名称（不需要.db）      |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Unload database done."
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**（4） drop**

**简要描述**

- 将数据库删除

**请求ip**

- ` 127.0.0.1 `

**请求端口号**

- ` 9000 `

**参数传递方式**

- 以`JSON`结构传递

**参数**

| 参数名  | 必选 | 类型   | 说明                       |
| :------ | :--- | :----- | -------------------------- |
| op      | 是   | string | 操作名称，固定值为**drop** |
| db_name | 是   | string | 数据库名称（不需要.db）    |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Drop database done."
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**（5）show**

**简要描述**

- 显示所有数据库列表

**请求ip**

- ` 127.0.0.1 `

**请求端口号**

- ` 9000 `

**参数传递方式**

- 以`JSON`结构传递

**参数**

| 参数名 | 必选 | 类型   | 说明                       |
| :----- | :--- | :----- | -------------------------- |
| op     | 是   | string | 操作名称，固定值为**show** |

**返回值**

| 参数名            | 类型      | 说明                                         |
| :---------------- | :-------- | -------------------------------------------- |
| StatusCode        | int       | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg         | string    | 返回具体信息                                 |
| ResponseBody      | JSONArray | JSON数组（每个都是一个数据库信息）           |
| -------- database | string    | 数据库名称                                   |
| ---------status   | string    | 数据库状态                                   |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "ResponseBody": [
           "lubm": "loaded",
           "lubm10K": "unloaded"
    ]
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**（5）query**

**简要描述**

- 对数据库进行查询

**请求ip**

- ` 127.0.0.1 `

**请求端口号**

- ` 9000 `

**参数传递方式**

- 以`JSON`结构传递

**参数**

| 参数名  | 必选 | 类型   | 说明                        |
| :------ | :--- | :----- | --------------------------- |
| op      | 是   | string | 操作名称，固定值为**query** |
| db_name | 是   | string | 需要操作的数据库            |
| format  | 否   | string | 结果集返回格式，默认是json  |
| sparql  | 是   | string | 要执行的sparql语句          |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| head       | JSON   | 头部信息                                     |
| results    | JSON   | 结果信息（详情请见返回示例）                 |

**返回示例** 

``` json
{
    "head": {
        "link": [],
        "vars": [
            "x"
        ]
    },
    "results": {
        "bindings": [
            {
                "x": {
                    "type": "uri",
                    "value": "十面埋伏"
                }
            },
            {
                "x": {
                    "type": "uri",
                    "value": "投名状"
                }
            },
            {
                "x": {
                    "type": "uri",
                    "value": "如花"
                }
            }
        ]
    },
    "StatusCode": 0,
    "StatusMsg": "success"
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**（7） login**

**简要描述**

- 登陆用户（验证用户名和密码）

**请求ip**

- ` 127.0.0.1 `

**请求端口号**

- ` 9000 `

**参数传递方式**

- 以`JSON`结构传递

**参数**

| 参数名   | 必选 | 类型   | 说明                        |
| :------- | :--- | :----- | --------------------------- |
| op       | 是   | string | 操作名称，固定值为**login** |
| username | 是   | string | 用户名                      |
| password | 是   | string | 密码（明文）                 |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例** 

``` json
{
    "StatusCode": 1001,
    "StatusMsg": "wrong password."
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**（8）stop**

**简要描述**

- 关闭服务端

**请求ip**

- ` 127.0.0.1 `

**请求端口号**

- ` 9000 `

**参数传递方式**

- 以`JSON`结构传递

**参数**

| 参数名 | 必选 | 类型   | 说明                       |
| :----- | :--- | :----- | -------------------------- |
| op     | 是   | string | 操作名称，固定值为**stop** |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Server stopped."
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**（9） close**

**简要描述**

- 关闭与客户端的连接

**请求ip**

- ` 127.0.0.1 `

**请求端口号**

- ` 9000 `

**参数传递方式**

- 以`JSON`结构传递

**参数**

| 参数名 | 必选 | 类型   | 说明                        |
| :----- | :--- | :----- | --------------------------- |
| op     | 是   | string | 操作名称，固定值为**close** |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回示例** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Connection disconnected."
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**附表1 返回值代码表**

| 代码值 | 涵义                                        |
| ------ | ------------------------------------------- |
| 0      | Success                                     |
| 1000   | The method type is  not support             |
| 1001   | Authentication Failed                       |
| 1002   | Check Privilege  Failed                     |
| 1003   | Param is illegal                            |
| 1004   | The operation conditions  are not satisfied |
| 1005   | Operation failed                            |
| 1006   | Add privilege Failed                        |
| 1007   | Loss of lock                                |
| 1008   | Transcation manage Failed                   |
| 1100   | The operation is  not defined               |
| 1101   | IP Blocked                                  |
|        |                                             |



<!-- <div STYLE="page-break-after: always;"></div> -->
<!-- <div STYLE="page-break-after: always;"></div> -->
