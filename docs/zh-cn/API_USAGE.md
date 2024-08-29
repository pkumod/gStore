## API介绍

gStore通过http和Socket服务向用户提供API服务，其组件为grpc、ghttp和gserver。

### HTTP API介绍

我们现在为 gRPC 和 ghttp 提供 C++、Java、Python、PHP 和 Node.js API。请参考`api/http/cpp`、`api/http/java`、`api/http/python`、`api/http/php`和`api/http/nodejs`中的示例代码。要使用这些示例，请确保已经生成了可执行文件。**接下来，使用`bin/grpc`或`bin/ghttp`命令启动http服务。**如果您知道一个正在运行的可用http服务器，并尝试连接到它，也是可以的。然后，对于c++和java代码，您需要编译目录`api/http/cpp/example`和`api/http/java/example`中的示例代码。

<!-- <div STYLE="page-break-after: always;"></div> -->

#### HTTP API 结构

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

我们现在为gServer提供c++（后续会逐步完善java、python、php和nodejs）API。请参考api/socket/cpp中的示例代码。要使用这些示例，请确保已经生成了可执行文件。**接下来，使用`bin/gserver -s`命令启动gServer服务**。如果您知道一个正在运行的可用gServer服务器，并尝试连接到它，也是可以的。然后，对于c++和java代码，您需要编译目录`api/socket/cpp/example`中的示例代码。

**具体启动和关闭gServer可见【开发文档】-【快速入门】-【Socket API 服务】**。

**Socket API启动完成后，就可以通过Socket进行连接了，gServer的默认端口为9000**

<!-- <div STYLE="page-break-after: always;"></div> -->

#### Socket API 结构

gStore的Socket API放在gStore根目录的API/Socket目录中，其内容如下:

- gStore/api/socket/

  - cpp/ (the C++ API)

    - example/ (使用C++ API的示例程序)
      - CppAPIExample.cpp
      - Makefile  (编译示例程序)
    - src/
      - Client.cpp  (C++ API的源代码)
      - Client.h
      - Makefile (编译和构建lib)



- - -

<!-- <div STYLE="page-break-after: always;"></div> -->

## http接口说明

### ghttp服务接口对接方式

>ghttp接口采用的是`http`协议，支持多种方式访问接口，如果ghttp启动的端口为`9000`,则接口URL地址为：
>
>```json
>http://ip:9000/
>```
>
>接口支持 `get`请求和`post`请求，其中`get`请求参数是放在url中，`post`请求是将参数放在`body`请求
>**注意：`GET`请求中各参数如果含有特殊字符，如？，@,&等字符时，需要采用urlencode进行编码，尤其是`sparql`参数必须进行编码**

### grpc服务接口对接方式

>grpc接口采用的是`http`协议，支持多种方式访问接口，如果grpc启动的端口为`9000`,则接口URL地址为：
>
>```json
>http://ip:9000/grpc
>```
>
>接口支持 `get`请求和`post`请求，其中`get`请求参数是放在url中；`post`请求是将参数放在`body`请求或者以`form`表达方式请求。
>
>post请求方式一<font color=red>（推荐）</font>：参数以JSON结构通过`httprequest`的`body`中的`raw`方式传递（要求RequestHeader参数Content-Type:application/json）
>
>post请求方式二：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）
>
>**注意：`GET`请求中各参数如果含有特殊字符，如？，@,&等字符时，需要采用urlencode进行编码，尤其是`sparql`参数必须进行编码**

### 接口列表

> 红色字体部分为最新版本新增，蓝色字体部分为最新版本修改

| 接口名称                                               | 含义                      | 备注                                                         |
| ------------------------------------------------------ | ------------------------- | ------------------------------------------------------------ |
| 系统相关接口                                           |                           |                                                              |
| &emsp;check                                            | 心跳信号                  | 检测ghttp心跳信号                                            |
| &emsp;login                                            | 登录数据库                | 用于验证用户名和密码                                         |
| &emsp;testConnect                                      | 测试连接性                | 用于检测ghttp是否连接                                        |
| &emsp;getCoreVersion                                   | 获取gStore版本号          | 获取gStore版本号                                             |
| &emsp;ipmanage                                         | 黑白名单管理              | 维护访问gstore的IP黑白名单                                   |
| &emsp;upload                                           | 上传文件                  | 支持上传的文件类型有nt、ttl、n3、rdf、txt、zip               |
| &emsp;download                                         | 下载文件                  | 支持下载gstore主目录及其子目录下的文件                       |
| &emsp;stat                                             | 查询系统资源              | 统计CPU、内存、磁盘可用空间信息                              |
| &emsp;shutdown                                         | 关闭ghttp服务             |                                                              |
| 数据库操作接口                                         |                           |                                                              |
| &emsp;show                                             | 显示数据库列表            | 显示所有数据库列表                                           |
| &emsp;load                                             | 加载图数据库              | 将数据库加载到内存中                                         |
| &emsp;unload                                           | 卸载图数据库              | 将数据库从内存中卸载                                         |
| &emsp;monitor                                          | 统计图数据库              | 统计指定数据库相关信息（如三元组数量等）                     |
| &emsp;<font color=blue>build(更新)</font>              | 构建图数据库              | 数据库文件需在服务器本地                                     |
| &emsp;drop                                             | 删除图数据库              | 可以逻辑删除和物理删除                                       |
| &emsp;<font color=blue>backup(更新)</font>             | 备份数据库                | 备份数据库信息                                               |
| &emsp;backuppath                                       | 获取备份数据库路径        | 返回默认备份路径下./backups所有的备份文件列表                |
| &emsp;<font color=blue>restore(更新)</font>            | 还原数据库                | 还原数据库信息                                               |
| &emsp;query                                            | 查询数据库                | 包括查询、删除、插入                                         |
| &emsp;export                                           | 导出数据库                | 导出数据库为NT文件                                           |
| &emsp;<font color=blue>batchInsert(更新)</font>        | 批量插入数据              | 批量插入NT数据                                               |
| &emsp;<font color=blue>batchRemove(更新)</font>        | 批量删除数据              | 批量删除NT数据                                               |
| &emsp;rename                                           | 重命名图数据库            | 修改图数据库名称                                             |
| &emsp;<font color=red>checkOperationState(新增)</font> | 查询异步操作状态          | 针对build, backup, restore, batch_insert, batch_remove异步执行的情况 |
| 数据库事务接口                                         |                           |                                                              |
| &emsp;begin                                            | 启动事务                  | 事务启动，需要与tquery配合使用                               |
| &emsp;tquery                                           | 查询数据库（带事务）      | 带事务模式的数据查询（仅限于insert和delete）                 |
| &emsp;commit                                           | 提交事务                  | 事务完成后提交事务                                           |
| &emsp;rollback                                         | 回滚事务                  | 回滚事务到begin状态                                          |
| &emsp;checkpoint                                       | 将数据写入磁盘            | 当对数据库进行了insert或delete操作后，需要手动执行checkpoint |
| 用户管理接口                                           |                           |                                                              |
| &emsp;showuser                                         | 显示所有用户列表          | 显示所有用户列表信息                                         |
| &emsp;usermanage                                       | 用户管理                  | 新增、删除、修改用户信息                                     |
| &emsp;userprivilegemanage                              | 用户权限管理              | 新增、删除、修改用户权限信息                                 |
| &emsp;userpassword                                     | 修改用户密码              | 修改用户密码                                                 |
| 自定义函数接口                                         |                           |                                                              |
| &emsp;funquery                                         | 查询算子函数              | 分页获取自定义算子函数列表                                   |
| &emsp;funcudb                                          | 管理算子函数              | 算子函数的新增、修改、删除、编译                             |
| &emsp;funreview                                        | 预览算子函数              | 在创建和更新时，可通过预览接口查看最后生成的算子函数源码     |
| 日志接口                                               |                           |                                                              |
| &emsp;txnlog                                           | 获取transaction的日志信息 | 以json返回transcation的日志信息                              |
| &emsp;querylogdate                                     | 获取query日志的日期列表   | 查询已有query日志的日期列表                                  |
| &emsp;querylog                                         | 获取query的日志信息       | 以json返回query的日志信息                                    |
| &emsp;accesslogdate                                    | 获取API日志的日期         | 查询已有API日志的日期列表                                    |
| &emsp;accesslog                                        | 获取API的访问日志         | 以json返回API的访问日志信息                                  |
| <font color=red>推理引擎接口(新增)</font>              |                           |                                                              |
| &emsp;addReason                                        | 新增规则                  |                                                              |
| &emsp;listReason                                       | 查询规则                  |                                                              |
| &emsp;compileReason                                    | 编译规则                  |                                                              |
| &emsp;executeReason                                    | 执行规则                  |                                                              |
| &emsp;disableReason                                    | 禁用规则                  |                                                              |
| &emsp;showReason                                       | 查看规则                  |                                                              |
| &emsp;removeReason                                     | 删除规则                  |                                                              |

### 接口详细说明

> 该节中将详细阐述各个接口的输入和输出参数，假设ghttp server的ip地址为127.0.0.1，端口为9000

### 系统相关接口

#### 心跳检测 check

**简要描述**

- 检测服务是否在线

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

####  登录 login

**简要描述**

- 登陆用户（验证用户名和密码）
- 更新内容：登录成功后将返回gStore的全路径信息RootPath


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
| password   | 是   | string | 密码（明文)                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数名      | 类型   | 说明                                         |
| :---------- | :----- | -------------------------------------------- |
| StatusCode  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg   | string | 返回具体信息                                 |
| CoreVersion | string | 内核版本                                     |
| licensetype | string | 证书类型（开源版还是企业版）                 |
| RootPath    | string | gStore根目录全路径                           |
| type        | string | HTTP服务类型，固定值为ghttp                  |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "login successfully",
    "CoreVersion": "1.0",
    "licensetype": "opensource",
    "Rootpath": "/data/gstore",
    "type": "ghttp"
}
```

####  测试连接 testConnect

**简要描述**

- 测试服务器可否连接（用于workbench)


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
| password   | 是   | string | 密码（明文)                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数名      | 类型   | 说明                                         |
| :---------- | :----- | -------------------------------------------- |
| StatusCode  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg   | string | 返回具体信息                                 |
| CoreVersion | string | 内核版本号                                   |
| licensetype | string | 授权类型（开源版还是企业版）                 |
| type        | string | HTTP服务类型，固定值为ghttp                  |


**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "CoreVersion": "1.0",
    "licensetype": "opensource",
    "type": "ghttp"
}
```

####  获取版本号 getCoreVersion

**简要描述**

- 获取gstore版本号（用于workbench)

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
| password   | 是   | string | 密码（明文)                            |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密   |

**返回值**

| 参数名      | 类型   | 说明                                         |
| :---------- | :----- | -------------------------------------------- |
| StatusCode  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg   | string | 返回具体信息                                 |
| CoreVersion | string | 内核版本号                                   |
| type        | string | HTTP服务类型，固定值为ghttp                  |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "CoreVersion": "1.0",
    "type": "ghttp"
}
```

####  黑白名单管理 ipmanage

**简要描述**

- 黑白名单管理

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
| password   | 是   | string | 密码（明文)                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| type       | 是   | string | 操作类型，固定值为**1**              |

保存黑白名单

| 参数名     | 必选 | 类型   | 说明                                                         |
| ---------- | ---- | ------ | ------------------------------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`ipmanage`**                             |
| username   | 是   | string | 用户名                                                       |
| password   | 是   | string | 密码（明文)                                                  |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| type       | 是   | string | 操作类型，固定值为**2**                                      |
| ip_type    | 是   | string | 名单类型，1-黑名单 2-白名单                                  |
| ips        | 是   | string | IP名单（多个用**,**分割，支持范围配置，使用**-**连接如：ip1-ip2） |

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

| 参数                | 类型   | 说明                                         |
| :------------------ | :----- | -------------------------------------------- |
| StatusCode          | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg           | string | 返回具体信息                                 |
| ResponseBody        | object | 返回数据（只有查询时才返回）                 |
| &emsp;&emsp;ip_type | string | 名单类型，1-黑名单 2-白名单                  |
| &emsp;&emsp;ips     | array  | 名单列表                                     |

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

####  上传文件 upload

**简要描述**

- 上传文件，目前支持的上传文件格式为nt、ttl、txt

**请求URL**

- ghttp服务: ` http://127.0.0.1:9000/file/upload `<font color="red">【注意，地址变化】</font>
- grpc服务: ` http://127.0.0.1:9000/grpc/file/upload `<font color="red">【注意，地址变化】</font>

**请求方式**

- POST 

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`form-data`(要求RequestHeader参数Content-Type:multipart/form-data)

**参数**

| 参数名     | 必选 | 类型    | 说明                                 |
| :--------- | :--- | :------ | ------------------------------------ |
| username   | 是   | string  | 用户名                               |
| password   | 是   | string  | 密码（明文)                          |
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

####  下载文件 download

**简要描述**

- 下载文件，目前支持的下载gStore根目录下的文件

**请求URL**

- ghttp服务: ` http://127.0.0.1:9000/file/download `<font color="red">【注意，地址变化】</font>
- grpc服务: ` http://127.0.0.1:9000/grpc/file/download `<font color="red">【注意，地址变化】</font>

**请求方式**

- POST 

**参数传递方式**

- post请求：参数以Form表单方式传递（要求RequestHeader参数Content-Type:application/x-www-form-urlencoded）

**参数**

| 参数名     | 必选 | 类型   | 说明                                                       |
| :--------- | :--- | :----- | ---------------------------------------------------------- |
| username   | 是   | string | 用户名（该用户名默认是system）                             |
| password   | 是   | string | 密码（明文)                                                |
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

#### 系统资源统计 stat

**简要描述**

- 统计系统资源信息

**请求URL**

- ghttp服务: ` http://127.0.0.1:9000/ghttp/api `  
- grpc服务: ` http://127.0.0.1:9000/grpc/api `

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
| password   | 是   | string | 密码（明文)                          |
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
    "cpu_usage": "10.596026",
    "mem_usage": "2681.507812",
    "disk_available": "12270"
}
```

####  关闭API服务 shutdown

**简要描述**

- 关闭ghttp

**请求URL**

- ghttp服务: ` http://127.0.0.1:9000/shutdown `<font color="red">【注意，地址变化】</font>
- grpc服务: ` http://127.0.0.1:9000/grpc/shutdown `<font color="red">【注意，地址变化】</font>

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
{
    "StatusCode": 0,
    "StatusMsg": "Server stopped successfully."
}
```

### 数据库操作接口

####  查看 show

**简要描述**

- 显示所有数据库列表

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
| password   | 是   | string | 密码（明文)                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数名                 | 类型   | 说明                                         |
| :--------------------- | :----- | -------------------------------------------- |
| StatusCode             | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg              | string | 返回具体信息                                 |
| ResponseBody           | array  | JSON数组（每个都是一个数据库信息）           |
| &emsp;&emsp;database   | string | 数据库名称                                   |
| &emsp;&emsp;creator    | string | 创建者                                       |
| &emsp;&emsp;built_time | string | 创建时间                                     |
| &emsp;&emsp;status     | string | 数据库状态                                   |

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

####  加载 load

**简要描述**

- 将数据库加载到内存中，load操作是很多操作的前置条件，如query，monitor等

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                                       |
| :--------- | :--- | :----- | ---------------------------------------------------------- |
| operation  | 是   | string | 操作名称，固定值为**`load`**                               |
| username   | 是   | string | 用户名                                                     |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                       |
| password   | 是   | string | 密码（明文)                                                |
| db_name    | 是   | string | 数据库名称（不需要.db）                                    |
| csr        | 否   | string | 是否加载CSR资源，默认为0（使用高级查询函数时，需要设置为1) |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |
| csr        | string | 是否加载CSR资源                              |


**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database loaded successfully.",
    "csr": "1"
}
```

####  卸载 unload

**简要描述**

- 将数据库从内存中卸载（所有的更改都会刷回硬盘）


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
| password   | 是   | string | 密码（明文)                         |
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

####  统计 monitor

**简要描述**

- 获取数据库统计信息（需要先load数据库）
- 更新内容：返回值中参数的名称调整（把含有空格和下划线的参数改用驼峰式，如：triple num -> tripleNum）

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
| password   | 是   | string | 密码（明文)                         |
| encryption | 否   | string | 为空，则密码为明文，=1表示用md5加密 |
| db_name    | 是   | string | 数据库名称（不需要.db）             |

**返回值**

| 参数名        | 类型   | 说明                                         |
| :------------ | :----- | -------------------------------------------- |
| StatusCode    | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg     | string | 返回具体信息                                 |
| database      | string | 数据库名称                                   |
| creator       | string | 创建者                                       |
| builtTime     | string | 创建时间                                     |
| tripleNum     | string | 三元组数量                                   |
| entityNum     | int    | 实体数量                                     |
| literalNum    | int    | 字面值数量（属性值）                         |
| subjectNum    | int    | 主语数量                                     |
| predicateNum  | int    | 谓词数量                                     |
| connectionNum | int    | 连接数量                                     |
| diskUsed      | int    | 磁盘空间（MB）                               |
| subjectList   | Array  | 实体类型统计                                 |


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

####  构建 build(更新)

**简要描述**

- 根据已有的NT文件创建数据库，或者构建空库（`1.2`版本支持）
- 文件必须存在gStore服务器上
- 可先通过3.1.6upload接口上传数据文件到gStore服务器

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名                          | 必选 | 类型   | 说明                                                         |
| :------------------------------ | :--- | :----- | ------------------------------------------------------------ |
| operation                       | 是   | string | 操作名称，固定值为**`build`**                                |
| username                        | 是   | string | 用户名                                                       |
| password                        | 是   | string | 密码（明文)                                                  |
| encryption                      | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name                         | 是   | string | 数据库名称（不需要.db）                                      |
| db_path                         | 否   | string | 数据库文件路径，支持的文件类型为（可以是绝对路径，也可以是相对路径，相对路径以gStore安装根目录为参照目录） |
| <font color=red>async</font>    | 否   | string | 如果**async为“true”**，则立即返回查询opt_id，服务器异步处理逻辑，客户端可通过opt_id调用checkOperationState接口，查看操作状态信息 |
| <font color=red>callback</font> | 否   | string | 回调接口，例如:http://127.0.0.1:8080/callback                |

**返回值**

| 参数名                        | 类型   | 说明                                                         |
| :---------------------------- | :----- | ------------------------------------------------------------ |
| StatusCode                    | int    | 返回值代码值（具体请参考附表：返回值代码表）                 |
| StatusMsg                     | string | 返回具体信息                                                 |
| failed_num                    | int    | 构建失败数量                                                 |
| <font color=red>opt_id</font> | string | 返回操作opt_id，客户端可通过opt_id调用checkOperationState接口，查看操作状态信息 |

**返回示例**（ 默认）

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Import RDF file to database done.",
    "failed_num": 0,
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**返回示例** (async=“true“)

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Operation Success.",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**返回示例**（callback=“http://127.0.0.1:8080/callback”）

``` json
# 向callback进行数据推送，数据如下
{
    "operation": "build",
    "StatusCode": 0,
    "StatusMsg": "Import RDF file to database done.",
    "failed_num": 0,
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

####  删除 drop

**简要描述**

- 将数据库删除（可以逻辑删除，也可以物理删除）


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
| password   | 是   | string | 密码（明文)                                                  |
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

####  备份 backup(更新)

**简要描述**

- 对数据库进行备份


**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名                          | 必选 | 类型   | 说明                                                         |
| :------------------------------ | :--- | :----- | ------------------------------------------------------------ |
| operation                       | 是   | string | 操作名称，固定值为**backup**                                 |
| username                        | 是   | string | 用户名                                                       |
| password                        | 是   | string | 密码（明文)                                                  |
| encryption                      | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name                         | 是   | string | 需要操作的数据库                                             |
| backup_path                     | 否   | string | 备份文件路径（可以是相对路径，也可以是绝对路径,相对路径以gStore根目录为参考），默认为gStore根目录下的backups目录 |
| <font color=red>async</font>    | 否   | string | 如果**async为“true”**，则立即返回查询opt_id，服务器异步处理逻辑，客户端可通过opt_id调用checkOperationState接口，查看操作状态信息 |
| <font color=red>callback</font> | 否   | string | 回调接口，例如:http://127.0.0.1:8080/callback                |

**返回值**

| 参数名                        | 类型   | 说明                                                         |
| :---------------------------- | :----- | ------------------------------------------------------------ |
| StatusCode                    | int    | 返回值代码值（具体请参考附表：返回值代码表）                 |
| StatusMsg                     | string | 返回具体信息                                                 |
| backupfilepath                | string | 备份文件路径（该值可以作为restore的输入参数值）              |
| <font color=red>opt_id</font> | string | 返回操作opt_id，客户端可通过opt_id调用checkOperationState接口，查看操作状态信息 |

**返回示例**（默认）

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database backup successfully.",
    "backupfilepath": "./backups/lubm.db_210828211529",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**返回示例** (async=“true“)

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Operation Success.",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**返回示例**（callback=“http://127.0.0.1:8080/callback”）

``` json
# 向callback进行数据推送，数据如下
{
    "operation": "backup",
    "StatusCode": 0,
    "StatusMsg": "Database backup successfully.",
    "backupfilepath": "./backups/lubm.db_210828211529",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

####  查询备份路径 backuppath

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
| password   | 是   | string | 密码（明文)                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| db_name    | 是   | string | 需要查询的数据库名称                 |

**返回值**

| 参数名     | 类型   | 说明                                            |
| :--------- | :----- | ----------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表）    |
| StatusMsg  | string | 返回具体信息                                    |
| paths      | Array  | 备份文件路径（该值可以作为restore的输入参数值） |


**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "paths": [
        "./backups/lubm.db_220828211529",
        "./backups/lubm.db_221031094522"
    ]
}
```

####  还原 restore(更新)

**简要描述**

- 对备份数据库进行还原


**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名                          | 必选 | 类型   | 说明                                                         |
| :------------------------------ | :--- | :----- | ------------------------------------------------------------ |
| operation                       | 是   | string | 操作名称，固定值为**`restore`**                              |
| username                        | 是   | string | 用户名                                                       |
| password                        | 是   | string | 密码（明文)                                                  |
| encryption                      | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name                         | 是   | string | 需要操作的数据库                                             |
| backup_path                     | 是   | string | 备份文件完整路径【带时间戳的】（可以是相对路径，也可以是绝对路径，相对路径以gStore根目录为参考） |
| <font color=red>async</font>    | 否   | string | 如果**async为“true”**，则立即返回查询opt_id，服务器异步处理逻辑，客户端可通过opt_id调用checkOperationState接口，查看操作状态信息 |
| <font color=red>callback</font> | 否   | string | 回调接口，例如:http://127.0.0.1:8080/callback                |

**返回值**

| 参数名                        | 类型   | 说明                                                         |
| :---------------------------- | :----- | ------------------------------------------------------------ |
| StatusCode                    | int    | 返回值代码值（具体请参考附表：返回值代码表）                 |
| StatusMsg                     | string | 返回具体信息                                                 |
| <font color=red>opt_id</font> | string | 返回操作opt_id，客户端可通过opt_id调用checkOperationState接口，查看操作状态信息 |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database lumb restore successfully.",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**返回示例** (async=“true“)

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Operation Success.",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**返回示例**（callback=“http://127.0.0.1:8080/callback”）

``` json
# 向callback进行数据推送，数据如下
{
    "StatusCode": 0,
    "StatusMsg": "Database lumb restore successfully.",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

####  查询 query

**简要描述**

- 对数据库进行查询


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
| password   | 是   | string | 密码（明文)                                                  |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name    | 是   | string | 需要操作的数据库                                             |
| format     | 否   | string | 结果集返回格式（可选值有：json，html和file），默认是json     |
| sparql     | 是   | string | 要执行的sparql语句（如果是get请求的话，sparql需要进行url编码） |

**返回值**

| 参数名               | 类型   | 说明                                         |
| :------------------- | :----- | -------------------------------------------- |
| StatusCode           | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg            | string | 返回具体信息                                 |
| head                 | object | 头部信息                                     |
| &emsp;&emsp;link     | array  |                                              |
| &emsp;&emsp;vars     | array  |                                              |
| results              | object | 结果信息（详情请见返回示例）                 |
| &emsp;&emsp;bindings | array  |                                              |
| AnsNum               | int    | 结果数                                       |
| OutputLimit          | int    | 最大返回结果数（-1为不限制）                 |
| ThreadId             | string | 查询线程编号                                 |
| QueryTime            | string | 查询耗时（毫秒）                             |


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

####  导出 export

**简要描述**

- 对数据库进行导出


**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                                         |
| :--------- | :--- | :----- | ------------------------------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**`export`**                               |
| username   | 是   | string | 用户名                                                       |
| password   | 是   | string | 密码（明文)                                                  |
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

####  批量插入 batchInsert(更新)

**简要描述**

- 批量插入数据


**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名                          | 必选 | 类型   | 说明                                                         |
| :------------------------------ | :--- | :----- | ------------------------------------------------------------ |
| operation                       | 是   | string | 操作名称，固定值为**batchInsert**                            |
| username                        | 是   | string | 用户名                                                       |
| password                        | 是   | string | 密码（明文)                                                  |
| encryption                      | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name                         | 是   | string | 数据库名                                                     |
| file                            | 否   | string | 要插入的数据nt文件或zip包（可以是相对路径也可以是绝对路径），与dir参数不能同时为空值，二者都有值时，取file参数 |
| dir                             | 否   | string | 要插入的数据nt文件夹（可以是相对路径也可以是绝对路径），与file参数不能同时为空值，二者都有值时，取file参数 |
| <font color=red>async</font>    | 否   | string | 如果**async为“true”**，则立即返回查询opt_id，服务器异步处理逻辑，客户端可通过opt_id调用checkOperationState接口，查看操作状态信息 |
| <font color=red>callback</font> | 否   | string | 回调接口，例如:http://127.0.0.1:8080/callback                |

**返回值**

| 参数名                        | 类型   | 说明                                                         |      |
| :---------------------------- | :----- | ------------------------------------------------------------ | ---- |
| StatusCode                    | int    | 返回值代码值（具体请参考附表：返回值代码表）                 |      |
| StatusMsg                     | string | 返回具体信息                                                 |      |
| success_num                   | int    | 执行成功的数量                                               |      |
| failed_num                    | int    | 执行失败数量                                                 |      |
| <font color=red>opt_id</font> | string | 返回操作opt_id，客户端可通过opt_id调用checkOperationState接口，查看操作状态信息 |      |

**返回示例** （默认）

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Batch insert data successfully.",
    "success_num": 25,
    "failed_num": 0,
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**返回示例** (async=“true”)

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Operation Success.",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**返回示例** （callback=“http://127.0.0.1:8080/callback”）

``` json
# 向callback进行数据推送，数据如下
{
    "operation": "batchInsert",
    "StatusCode": 0,
    "StatusMsg": "Batch insert data successfully.",
    "success_num": 25,
    "failed_num": 0,
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

####  批量删除 batchRemove(更新)

**简要描述**

- 批量插入数据

**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名                          | 必选 | 类型   | 说明                                                         |
| :------------------------------ | :--- | :----- | ------------------------------------------------------------ |
| operation                       | 是   | string | 操作名称，固定值为**`batchRemove`**                          |
| username                        | 是   | string | 用户名                                                       |
| password                        | 是   | string | 密码（明文)                                                  |
| encryption                      | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name                         | 是   | string | 数据库名                                                     |
| file                            | 是   | string | 要删除的数据nt文件或者zip包（可以是相对路径也可以是绝对路径） |
| <font color=red>async</font>    | 否   | string | 如果**async为“true”**，则立即返回查询opt_id，服务器异步处理逻辑，客户端可通过opt_id调用checkOperationState接口，查看操作状态信息 |
| <font color=red>callback</font> | 否   | string | 回调接口，例如:http://127.0.0.1:8080/callback                |

**返回值**

| 参数名                        | 类型   | 说明                                                         |
| :---------------------------- | :----- | ------------------------------------------------------------ |
| StatusCode                    | int    | 返回值代码值（具体请参考附表：返回值代码表）                 |
| StatusMsg                     | string | 返回具体信息                                                 |
| success_num                   | int    | 执行成功的记录数                                             |
| <font color=red>opt_id</font> | string | 返回操作opt_id，客户端可通过opt_id调用checkOperationState接口，查看操作状态信息 |

**返回示例** （默认）

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Batch remove data successfully.",
    "success_num": 25,
    "opt_id": "XXXX-XXXX-XXXX-XXXX",
}
```

**返回示例** ((async=“true)

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Operation Success.",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**返回示例** （callback=“http://127.0.0.1:8080/callback”）

``` json
# 向callback进行数据推送，数据如下
{
    "operation": "batchRemove",
    "StatusCode": 0,
    "StatusMsg": "Batch remove data successfully.",
    "success_num": 25,
    "opt_id": "XXXX-XXXX-XXXX-XXXX",
}
```

####  重命名 rename

**简要描述**

- 重命名数据库


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
| password   | 是   | string | 密码（明文)                          |
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

####  查询异步操作状态 checkOperationState

**简要描述**

- 查询异步操作状态（build, backup, restore, batch_insert, batch_remove）


**请求方式**

- GET/POST 

**参数传递方式**

- GET请求，参数直接以URL方式传递
- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名                        | 必选 | 类型   | 说明                                      |
| :---------------------------- | :--- | :----- | ----------------------------------------- |
| operation                     | 是   | string | 操作名称，固定值为**checkOperationState** |
| username                      | 是   | string | 用户名                                    |
| password                      | 是   | string | 密码（明文)                               |
| encryption                    | 否   | string | 为空，则密码为明文，为1表示用md5加密      |
| <font color=red>opt_id</font> | 是   | string | 操作序列号                                |

**返回值**

| 参数名      | 类型   | 说明                                         |
| :---------- | :----- | -------------------------------------------- |
| StatusCode  | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg   | string | 返回具体信息                                 |
| state       | int    | 执行状态（0 执行中,、-1 执行失败、1 已完成） |
| success_num | int    | 执行成功的记录数                             |
| failed_num  | int    | 执行失败的记录数                             |

**返回示例**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Batch insert data successfully.",
    "state": 1,
    "success_num": 100,
    "failed_num": 0
}
```

###  数据库事务接口

####  开启 begin

**简要描述**

- 开始事务


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
| password   | 是   | string | 密码（明文)                                                  |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密                         |
| db_name    | 是   | string | 数据库名称                                                   |
| isolevel   | 是   | string | 事务隔离等级 1:SR(seriablizable）2:SI(snapshot isolation) 3:RC(read committed) |

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

####  执行 tquery 

**简要描述**

- 事务型查询

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
| password   | 是   | string | 密码（明文)                          |
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

####  提交 commit 

**简要描述**

- 事务提交


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
| password   | 是   | string | 密码（明文)                         |
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

####  回滚 rollback 

**简要描述**

- 事务回滚


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
| password   | 是   | string | 密码（明文)                         |
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

#### 落盘 checkpoint

**简要描述**

- 收到将数据刷回到硬盘（使得数据最终生效）


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
| password   | 是   | string | 密码（明文)                          |
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

###  用户管理接口

####  查看 showuser

**简要描述**

- 显示所有用户信息


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
| password   | 是   | string | 密码（明文)                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |

**返回值**

| 参数名                        | 类型   | 说明                                         |
| :---------------------------- | :----- | -------------------------------------------- |
| StatusCode                    | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg                     | string | 返回具体信息                                 |
| ResponseBody                  | array  | JSON对象数组                                 |
| &emsp;&emsp;username          | string | 用户名                                       |
| &emsp;&emsp;password          | string | 密码                                         |
| &emsp;&emsp;query_privilege   | string | 查询权限（数据库名以逗号分隔)                |
| &emsp;&emsp;update_privilege  | string | 更新权限（数据库名以逗号分隔)                |
| &emsp;&emsp;load_privilege    | string | 加载权限（数据库名以逗号分隔)                |
| &emsp;&emsp;unload_privilege  | string | 卸载权限（数据库名以逗号分隔)                |
| &emsp;&emsp;backup_privilege  | string | 备份权限（数据库名以逗号分隔)                |
| &emsp;&emsp;restore_privilege | string | 还原权限（数据库名以逗号分隔)                |
| &emsp;&emsp;export_privilege  | string | 导出权限（数据库名以逗号分隔)                |


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

####  增删改 usermanage

**简要描述**

- 对用户进行管理（包括增、删、改）

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
| password    | 是   | string | 密码（明文)                                                  |
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



####  授权 userprivilegemanage

**简要描述**

- 对用户权限进行管理（包括增、删、改）


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
| password    | 是   | string | 密码（明文)                                                  |
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

####  修改密码 userpassword

**简要描述**

- 修改用户密码

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
    "StatusCode": 0,
    "StatusMsg": "change password done."
}
```

###  自定义函数接口

####  查询 funquery

**简要描述**

- 算子函数查询

**请求方式**

- POST

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名                | 必选 | 类型   | 说明                                 |
| --------------------- | ---- | ------ | ------------------------------------ |
| operation             | 是   | string | 操作名称，固定值为**`funquery`**     |
| username              | 是   | string | 用户名                               |
| password              | 是   | string | 密码（明文)                          |
| encryption            | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| funInfo               | 否   | object | 查询参数                             |
| &emsp;&emsp;funName   | 否   | string | 函数名称                             |
| &emsp;&emsp;funStatus | 否   | string | 状态（1-待编译 2-已编译 3-异常）     |

**返回值**

| 参数                  | 类型   | 说明                                         |
| :-------------------- | :----- | -------------------------------------------- |
| StatusCode            | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg             | string | 返回具体信息                                 |
| list                  | array  | JSON数组（如果没有数据，则不返回空数组）     |
| &emsp;&emsp;funName   | string | 名称                                         |
| &emsp;&emsp;funDesc   | string | 描述                                         |
| &emsp;&emsp;funArgs   | string | 参数类型（1-无K跳参数 2-有K跳参数）          |
| &emsp;&emsp;funBody   | string | 函数内容                                     |
| &emsp;&emsp;funSubs   | string | 函数子方法                                   |
| &emsp;&emsp;funStatus | string | 状态（1-待编译 2-已编译 3-异常）             |
| &emsp;&emsp;lastTime  | string | 最后编辑时间（yyyy-MM-dd HH:mm:ss）          |

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

####  增删改编译 funcudb

**简要描述**

- 算子函数管理（新增、修改、删除、编译）

**请求方式**

- POST

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名                | 必选 | 类型   | 说明                                                    |
| --------------------- | ---- | ------ | ------------------------------------------------------- |
| operation             | 是   | string | 操作名称，固定值为**`funcudb`**                         |
| username              | 是   | string | 用户名                                                  |
| password              | 是   | string | 密码（明文)                                             |
| encryption            | 否   | string | 为空，则密码为明文，为1表示用md5加密                    |
| type                  | 是   | string | 1:新增，2:修改，3:删除，4:编译                          |
| funInfo               | 是   | object | 算子函数                                                |
| &emsp;&emsp;funName   | 是   | string | 函数名称                                                |
| &emsp;&emsp;funDesc   | 否   | string | 描述                                                    |
| &emsp;&emsp;funArgs   | 否   | string | 参数类型（1无K跳参数，2有K跳参数）                      |
| &emsp;&emsp;funBody   | 否   | string | 函数内容（以`{}`包裹的内容）                            |
| &emsp;&emsp;funSubs   | 否   | string | 子函数（可用于fun_body中调用）                          |
| &emsp;&emsp;funReturn | 否   | string | 返回类型（`path`:返回路径类结果，`value`:返回值类结果） |

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

####  预览 funreview

**简要描述**

- 预览算子函数

**请求方式**

- POST

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名                | 必选 | 类型   | 说明                                                    |
| --------------------- | ---- | ------ | ------------------------------------------------------- |
| operation             | 是   | string | 操作名称，固定值为**`funreview`**                       |
| username              | 是   | string | 用户名                                                  |
| password              | 是   | string | 密码（明文)                                             |
| encryption            | 否   | string | 为空，则密码为明文，为1表示用md5加密                    |
| funInfo               | 是   | object | 算子函数                                                |
| &emsp;&emsp;funName   | 是   | string | 函数名称                                                |
| &emsp;&emsp;funDesc   | 否   | string | 描述                                                    |
| &emsp;&emsp;funArgs   | 是   | string | 参数类型（1无K跳参数，2有K跳参数）                      |
| &emsp;&emsp;funBody   | 是   | string | 函数内容（以`{}`包裹的内容）                            |
| &emsp;&emsp;funSubs   | 是   | string | 子函数（可用于fun_body中调用）                          |
| &emsp;&emsp;funReturn | 是   | string | 返回类型（`path`:返回路径类结果，`value`:返回值类结果） |

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

###  日志接口

####  事务日志 txnlog

**简要描述**

- 获取事务日志（该功能只对root用户生效）
- 更新内容：增加分页查询参数

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
| password   | 是   | string | 密码（明文)                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| pageNo     | 是   | int    | 页号，取值范围1-N，默认1             |
| pageSize   | 是   | int    | 每页数，取值范围1-N，默认10          |

**返回值**

| 参数名                 | 类型   | 说明                                                         |
| :--------------------- | :----- | ------------------------------------------------------------ |
| StatusCode             | int    | 返回值代码值（具体请参考附表：返回值代码表）                 |
| StatusMsg              | string | 返回具体信息                                                 |
| totalSize              | int    | 总数                                                         |
| totalPage              | int    | 总页数                                                       |
| pageNo                 | int    | 当前页号                                                     |
| pageSize               | int    | 每页数                                                       |
| list                   | array  | 日志JSON数组                                                 |
| &emsp;&emsp;db_name    | string | 数据库名称                                                   |
| &emsp;&emsp;TID        | string | 事务ID                                                       |
| &emsp;&emsp;user       | string | 操作用户                                                     |
| &emsp;&emsp;state      | string | 状态<br />COMMITED-提交<br />RUNNING-执行中<br />ROLLBACK-回滚<br />ABORTED-中止 |
| &emsp;&emsp;begin_time | string | 开始时间                                                     |
| &emsp;&emsp;end_time   | string | 结束时间                                                     |


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

####  获取查询日志日期 querylogdate

**简要描述**

- 获取gstore的查询日志的日期（用于querylog接口的date选择参数）

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
| password   | 是   | string | 密码（明文)                          |
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

####  查询日志 querylog

**简要描述**

- 获取查询日志

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
| password   | 是   | string | 密码（明文)                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| date       | 是   | string | 日期，格式为yyyyMMdd                 |
| pageNo     | 是   | int    | 页号，取值范围1-N，默认1             |
| pageSize   | 是   | int    | 每页数，取值范围1-N，默认10          |

**返回值**

| 参数                | 类型      | 说明                                         |
| :------------------ | :-------- | -------------------------------------------- |
| StatusCode          | int       | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg           | string    | 返回具体信息                                 |
| totalSize           | int       | 总数                                         |
| totalPage           | int       | 总页数                                       |
| pageNo              | int       | 当前页号                                     |
| pageSize            | int       | 每页数                                       |
| list                | JSONArray | 日志JSON数组                                 |
| &emsp;QueryDateTime | string    | 查询时间                                     |
| &emsp;Sparql        | string    | SPARQL语句                                   |
| &emsp;Format        | string    | 查询返回格式                                 |
| &emsp;RemoteIP      | string    | 请求IP                                       |
| &emsp;FileName      | string    | 查询结果集文件                               |
| &emsp;QueryTime     | int       | 耗时(毫秒)                                   |
| &emsp;AnsNum        | int       | 结果数                                       |

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

####  获取访问日志日期 accesslogdate

**简要描述**

- 获取API日志的日期（用于accesslog接口的date选择参数）

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
| password   | 是   | string | 密码（明文)                           |
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

#### 访问日志 accesslog

**简要描述**

- 获取API的访问日志

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
| password   | 是   | string | 密码（明文)                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| date       | 是   | string | 日期，格式为yyyyMMdd                 |
| pageNo     | 是   | int    | 页号，取值范围1-N，默认1             |
| pageSize   | 是   | int    | 每页数，取值范围1-N，默认10          |

**返回值**

| 参数             | 类型      | 说明                                         |
| :--------------- | :-------- | -------------------------------------------- |
| StatusCode       | int       | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg        | string    | 返回具体信息                                 |
| totalSize        | int       | 总数                                         |
| totalPage        | int       | 总页数                                       |
| pageNo           | int       | 当前页号                                     |
| pageSize         | int       | 每页数                                       |
| list             | JSONArray | 日志JSON数组                                 |
| &emsp;ip         | string    | 访问ip                                       |
| &emsp;operation  | string    | 操作类型                                     |
| &emsp;createtime | string    | 操作时间                                     |
| &emsp;code       | string    | 操作结果（参考附表：返回值代码表）           |
| &emsp;msg        | string    | 日志描述                                     |

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

###  推理引擎

**参数type共有7种操作类型：**

- 1:addReason 
- 2:listReason 
- 3:compileReason 
- 4:executeReason 
- 5:disableReason 
- 6.showReason 
- 7.deleteReason

####  新增 addReason 

**简要描述**

- 新增推理规则

**请求方式**

- 支持POST，不支持GET 

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名                                        | 必选 | 类型   | 说明                                                 |
| :-------------------------------------------- | :--- | :----- | ---------------------------------------------------- |
| operation                                     | 是   | string | 操作名称，固定值为**reasonManage**                   |
| username                                      | 是   | string | 用户名                                               |
| password                                      | 是   | string | 密码（明文)                                          |
| encryption                                    | 否   | string | 为空，则密码为明文，为1表示用md5加密                 |
| db_name                                       | 是   | string | 数据库名                                             |
| type                                          | 是   | string | 操作类型 : "1"                                       |
| ruleinfo                                      | 是   | Object | 规则信息："ruleinfo": {"rulename":"", ...}，规则详情 |
| &emsp;&emsp;rulename                          | 是   | string | 规则名称（同一个数据库，规则名称唯一）               |
| &emsp;&emsp;description                       | 否   | string | 规则描述                                             |
| &emsp;&emsp;isenable                          | 是   | int    | 是否启用，可选值：1启用    0禁用                     |
| &emsp;&emsp;type                              | 是   | int    | 推理类型，可选值：1关系推理   0属性推理              |
| &emsp;&emsp;logic                             | 是   | int    | 条件与条件的关系，可选值：1逻辑与 0逻辑或            |
| &emsp;&emsp;conditions                        | 是   | array  | 规则集合                                             |
| &emsp;&emsp;&emsp;&emsp;patterns              | 是   | array  | 三元组集合                                           |
| &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;subject   | 是   | string | 主语                                                 |
| &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;predicate | 是   | string | 谓词                                                 |
| &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;object    | 是   | string | 宾语                                                 |
| &emsp;&emsp;&emsp;&emsp;filters               | 否   | array  | 过滤条件                                             |
| &emsp;&emsp;&emsp;&emsp;count_info            | 否   | Object | 聚集函数（待定）                                     |
| &emsp;&emsp;return                            | 是   | Object | 返回对象                                             |
| &emsp;&emsp;&emsp;&emsp;source                | 是   | string | 起始节点                                             |
| &emsp;&emsp;&emsp;&emsp;target                | 否   | string | 终止节点（关系推理有用）                             |
| &emsp;&emsp;&emsp;&emsp;label                 | 否   | string | 谓词（系统自动加上Rule:前缀)                         |
| &emsp;&emsp;&emsp;&emsp;value                 | 否   | string | 属性值（属性推理有用）                               |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**关系推理规则示例**

```json
{
	"rulename": "舅舅",
	"description": "母亲的男性同胞是舅舅",
	"isenable": 1,
	"type": 1,
	"logic": 1,
	"conditions": [
		{
			"patterns": [
				{
					"subject": "?x ",
					"predicate": "<母亲>",
					"object": "?y"
				},
				{
					"subject": "?y ",
					"predicate": "<父亲>",
					"object": "?z"
				},
				{
					"subject": "?k",
					"predicate": "<父亲>",
					"object": "?z"
				},
				{
					"subject": "?k",
					"predicate": "<性别>",
					"object": "\"男\"^^<http://www.w3.org/2001/XMLSchema#string>"
				}
			],
			"filters": [],
			"count_info": {}
		}
	],
	"return": {
		"source": "?x",
		"target": "?k",
		"label": "舅舅"
	}
}
```

**属性推理规则示例**

```json
{
	"rulename": "孤儿",
	"description": "父母双亡为孤儿",
	"isenable": 1,
	"type": 0,
	"logic": 1,
	"conditions": [
		{
			"patterns": [
				{
					"subject": "?x ",
					"predicate": "<父亲>",
					"object": "?y"
				},
				{
					"subject": "?x",
					"predicate": "<母亲>",
					"object": "?z"
				},
				{
					"subject": "?y",
					"predicate": "<状态>",
					"object": "\"去世\"^^<http://www.w3.org/2001/XMLSchema#string>"
				},
				{
					"subject": "?z",
					"predicate": "<状态>",
					"object": "\"去世\"^^<http://www.w3.org/2001/XMLSchema#string>"
				}
			],
			"filters": [],
			"count_info": {}
		}
	],
	"return": {
		"source": "?x",
		"label": "状态",
		"value": "\"孤儿\"^^<http://www.w3.org/2001/XMLSchema#string>"
	}
}
```

**返回值**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Save Successfully! the file path is ./dbhome/lubm.db/reason_rule_files/test.json"
}
```

####  查询 listReason 

**简要描述**

- 显示规则列表

**请求方式**

- 支持POST，不支持GET 

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**reasonManage**   |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文)                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| db_name    | 是   | string | 数据库名                             |
| type       | 是   | string | 操作类型：“2”                        |

**返回值**

| 参数名                            | 类型      | 说明                                         |
| :-------------------------------- | :-------- | -------------------------------------------- |
| StatusCode                        | int       | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg                         | string    | 返回具体信息                                 |
| num                               | int       | 规则数量                                     |
| list                              | JSONArray | 规则名称列表                                 |
| &emsp;rulename                    | string    | 规则名称（同一个数据库，规则名称唯一）       |
| &emsp;description                 | string    | 规则描述                                     |
| &emsp;isenable                    | int       | 是否启用，可选值：1启用    0禁用             |
| &emsp;type                        | int       | 推理类型，可选值：1关系推理   0属性推理      |
| &emsp;&emsp;logic                 | int       | 条件与条件的关系，可选值：1逻辑与 0逻辑或    |
| &emsp;conditions                  | array     | 规则集合                                     |
| &emsp;&emsp;condition             | arry      | 规则                                         |
| &emsp;&emsp;&emsp;patterns        | array     | 三元组集合                                   |
| &emsp;&emsp;&emsp;&emsp;subject   | string    | 主语                                         |
| &emsp;&emsp;&emsp;&emsp;predicate | string    | 谓词                                         |
| &emsp;&emsp;&emsp;&emsp;object    | string    | 宾语                                         |
| &emsp;&emsp;&emsp;filters         | array     | 过滤条件                                     |
| &emsp;&emsp;&emsp;count_info      | Object    | 聚集函数（待定）                             |
| &emsp;return                      | Object    | 返回对象                                     |
| &emsp;&emsp;source                | string    | 起始节点                                     |
| &emsp;&emsp;target                | string    | 终止节点（关系推理有用）                     |
| &emsp;&emsp;label                 | string    | 谓词（系统自动加上Rule:前缀)                 |
| &emsp;&emsp;value                 | string    | 属性值（属性推理有用）                       |
| &emsp;status                      | string    | 状态                                         |
| &emsp;insert_sparql               | string    | insert_sparql语句                            |
| &emsp;delete_sparql               | string    | delete_sparql语句                            |
| &emsp;createtime                  | string    | 创建时间                                     |

**返回值**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "ok",
    "list": [
        {
            "ruleid": "002",
            "rulename": "孤儿",
            "description": "父母双亡为孤儿",
            "conditions": [
                {
                    "condition": {
                        "patterns": [
                            {
                                "subject": "?x ",
                                "predicate": "<父亲>",
                                "object": "?y"
                            },
                            {
                                "subject": "?x",
                                "predicate": "<母亲>",
                                "object": "?z"
                            },
                            {
                                "subject": "?y",
                                "predicate": "<状态>",
                                "object": "\"去世\"^^<http://www.w3.org/2001/XMLSchema#string>"
                            },
                            {
                                "subject": "?z",
                                "predicate": "<状态>",
                                "object": "\"去世\"^^<http://www.w3.org/2001/XMLSchema#string>"
                            }
                        ],
                        "filters": [],
                        "count_info": {}
                    },
                    "logic": 1
                }
            ],
            "isenable": 1,
            "type": 0,
            "return": {
                "source": "?x",
                "label": "状态",
                "value": "\"孤儿\"^^<http://www.w3.org/2001/XMLSchema#string>"
            },
            "createtime": "2023-12-29 16:58:00",
            "status": "已执行",
            "insert_sparql": "insert {?x <Rule:状态> \"孤儿\"^^<http://www.w3.org/2001/XMLSchema#string>. } where {  ?x  <父亲> ?y. ?x <母亲> ?z. ?y <状态> \"去世\"^^<http://www.w3.org/2001/XMLSchema#string>. ?z <状态> \"去世\"^^<http://www.w3.org/2001/XMLSchema#string>. }",
            "delete_sparql": "delete where {?x <Rule:状态> \"孤儿\"^^<http://www.w3.org/2001/XMLSchema#string>.}"
        },
        {
            "ruleid": "001",
            "rulename": "舅舅2",
            "description": "母亲的男性同胞是舅舅",
            "conditions": [
                {
                    "condition": {
                        "patterns": [
                            {
                                "subject": "?x ",
                                "predicate": "<母亲>",
                                "object": "?y"
                            },
                            {
                                "subject": "?y ",
                                "predicate": "<父亲>",
                                "object": "?z"
                            },
                            {
                                "subject": "?k",
                                "predicate": "<父亲>",
                                "object": "?z"
                            },
                            {
                                "subject": "?k",
                                "predicate": "<性别>",
                                "object": "\"男\"^^<http://www.w3.org/2001/XMLSchema#string>"
                            }
                        ],
                        "filters": [],
                        "count_info": {}
                    },
                    "logic": 1
                }
            ],
            "isenable": 1,
            "type": 1,
            "return": {
                "source": "?x",
                "target": "?k",
                "label": "舅舅",
                "value": ""
            },
            "createtime": "2023-12-29 16:58:00",
            "status": "已执行",
            "insert_sparql": "insert { ?x <Rule:舅舅> ?k. } where {  ?x  <母亲> ?y. ?y  <父亲> ?z. ?k <父亲> ?z. ?k <性别> \"男\"^^<http://www.w3.org/2001/XMLSchema#string>. }",
            "delete_sparql": "delete where {?x <Rule:舅舅> ?y.}"
        }
    ],
    "num": 2
}
```

####  编译 compileReason

**简要描述**

- 规则编译

**请求方式**

- 支持POST，不支持GET 

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**reasonManage**   |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文)                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| db_name    | 是   | string | 数据库名                             |
| type       | 是   | string | 操作类型：“3”                        |
| rulename   | 是   | string | 规则名称                             |

**返回值**

| 参数名        | 类型   | 说明                                         |
| :------------ | :----- | -------------------------------------------- |
| StatusCode    | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg     | string | 返回具体信息                                 |
| insert_sparql | string | insert sparql语句                            |
| delete_sparql | string | delete sparql语句                            |

**返回值**

``` json
{
    "insert_sparql": "insert {?x <Rule:状态> \"孤儿\"^^<http://www.w3.org/2001/XMLSchema#string>. } where {  ?x  <父亲> ?y. ?x <母亲> ?z. ?y <状态> \"去世\"^^<http://www.w3.org/2001/XMLSchema#string>. ?z <状态> \"去世\"^^<http://www.w3.org/2001/XMLSchema#string>. }",
    "delete_sparql": "delete where {?x <Rule:状态> \"孤儿\"^^<http://www.w3.org/2001/XMLSchema#string>.}",
    "StatusCode": 0,
    "StatusMsg": "ok"
}
```

####  执行 executeReason

**简要描述**

- 规则执行

**请求方式**

- 支持POST，不支持GET 

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**reasonManage**   |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文)                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| db_name    | 是   | string | 数据库名                             |
| type       | 是   | string | 操作类型：“4”                        |
| rulename   | 是   | string | 规则名称                             |

**返回值**

| 参数名        | 类型   | 说明                                         |
| :------------ | :----- | -------------------------------------------- |
| StatusCode    | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg     | string | 返回具体信息                                 |
| insert_sparql | string | insert sparql语句                            |
| AnsNum        | int    | 成功数量                                     |

**返回值**

``` json
{
    "insert_sparql": "insert {?x <Rule:状态> \"孤儿\"^^<http://www.w3.org/2001/XMLSchema#string>. } where {  ?x  <父亲> ?y. ?x <母亲> ?z. ?y <状态> \"去世\"^^<http://www.w3.org/2001/XMLSchema#string>. ?z <状态> \"去世\"^^<http://www.w3.org/2001/XMLSchema#string>. }",
    "AnsNum": 0, //本次执行影响的记录数（如果之前已经有相关关系，则不计算）
    "StatusCode": 0,
    "StatusMsg": "ok"
}
```

####  禁用 disableReason

**简要描述**

- 规则失效

**请求方式**

- 支持POST，不支持GET 

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**reasonManage**   |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文)                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| db_name    | 是   | string | 数据库名                             |
| type       | 是   | string | 操作类型：“5”                        |
| rulename   | 是   | string | 规则名称                             |

**返回值**

| 参数名        | 类型   | 说明                                         |
| :------------ | :----- | -------------------------------------------- |
| StatusCode    | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg     | string | 返回具体信息                                 |
| delete_sparql | string | delete sparql语句                            |
| AnsNum        | int    | 成功数量                                     |

**返回值**

``` json
{
    "delete_sparql": "delete where {?x <Rule:状态> \"孤儿\"^^<http://www.w3.org/2001/XMLSchema#string>.}",
    "AnsNum": 1, //本次执行影响的记录数（如果之前已经有相关关系，则不计算）
    "StatusCode": 0,
    "StatusMsg": "ok"
}
```

####  详情 showReason

**简要描述**

- 显示规则明细

**请求方式**

- 支持POST，不支持GET 

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**reasonManage**   |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文)                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| db_name    | 是   | string | 数据库名                             |
| type       | 是   | string | 操作类型：“6”                        |
| rulename   | 是   | string | 规则名称                             |

**返回值**

| 参数名                      | 类型      | 说明                                         |
| :-------------------------- | :-------- | -------------------------------------------- |
| StatusCode                  | int       | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg                   | string    | 返回具体信息                                 |
| ruleinfo                    | Object    | Object                                       |
| insert_sparql               | string    | insert sparql语句                            |
| delete_sparql               | string    | delete sparql语句                            |
| status                      | string    | 状态                                         |
| rulename                    | string    | 规则名称（同一个数据库，规则名称唯一）       |
| description                 | string    | 规则描述                                     |
| isenable                    | int       | 是否启用                                     |
| type                        | int       | 1：表示关系推理   0：表示属性推理            |
| &emsp;logic                 | int       | 条件与条件的关系，可选值：1逻辑与 0逻辑或    |
| createtime                  | string    | 创建时间                                     |
| conditions                  | JSONArray | 规则条件（数组）                             |
| &emsp;condition             | Object    | 规则                                         |
| &emsp;&emsp;patterns        | array     | 三元组集合                                   |
| &emsp;&emsp;&emsp;subject   | string    | 主语                                         |
| &emsp;&emsp;&emsp;predicate | string    | 谓词                                         |
| &emsp;&emsp;&emsp;object    | string    | 宾语                                         |
| &emsp;&emsp;filters         | array     | 过滤条件                                     |
| &emsp;&emsp;count_info      | Object    | 聚集函数（待定）                             |
| return                      | Object    | 返回对象                                     |
| &emsp;source                | string    | 起始节点                                     |
| &emsp;target                | string    | 终止节点（关系推理有用）                     |
| &emsp;label                 | string    | 谓词（系统自动加上Rule:前缀)                 |
| &emsp;value                 | string    | 属性值（属性推理有用）                       |

**返回值**

``` json
{
    "ruleid": "002",
    "rulename": "孤儿",
    "description": "父母双亡为孤儿",
    "conditions": [
        {
            "condition": {
                "patterns": [
                    {
                        "subject": "?x ",
                        "predicate": "<父亲>",
                        "object": "?y"
                    },
                    {
                        "subject": "?x",
                        "predicate": "<母亲>",
                        "object": "?z"
                    },
                    {
                        "subject": "?y",
                        "predicate": "<状态>",
                        "object": "\"去世\"^^<http://www.w3.org/2001/XMLSchema#string>"
                    },
                    {
                        "subject": "?z",
                        "predicate": "<状态>",
                        "object": "\"去世\"^^<http://www.w3.org/2001/XMLSchema#string>"
                    }
                ],
                "filters": [],
                "count_info": {}
            },
            "logic": 1
        }
    ],
    "isenable": 1,
    "type": 0,
    "return": {
        "source": "?x",
        "label": "状态",
        "value": "\"孤儿\"^^<http://www.w3.org/2001/XMLSchema#string>"
    },
    "createtime": "2023-12-29 16:58:00",
    "status": "已失效", //当前规则状态
    "insert_sparql": "insert {?x <Rule:状态> \"孤儿\"^^<http://www.w3.org/2001/XMLSchema#string>. } where {  ?x  <父亲> ?y. ?x <母亲> ?z. ?y <状态> \"去世\"^^<http://www.w3.org/2001/XMLSchema#string>. ?z <状态> \"去世\"^^<http://www.w3.org/2001/XMLSchema#string>. }",
    "delete_sparql": "delete where {?x <Rule:状态> \"孤儿\"^^<http://www.w3.org/2001/XMLSchema#string>.}",
    "StatusCode": 0,
    "StatusMsg": "ok"
}
```

#### 删除 deleteReason

**简要描述**

- 删除规则

**请求方式**

- 支持POST，不支持GET 

**参数传递方式**

- POST请求，`httprequest` 中的`body`中的`raw`，以`JSON`结构传递

**参数**

| 参数名     | 必选 | 类型   | 说明                                 |
| :--------- | :--- | :----- | ------------------------------------ |
| operation  | 是   | string | 操作名称，固定值为**reasonManage**   |
| username   | 是   | string | 用户名                               |
| password   | 是   | string | 密码（明文)                          |
| encryption | 否   | string | 为空，则密码为明文，为1表示用md5加密 |
| db_name    | 是   | string | 数据库名                             |
| type       | 是   | string | 操作类型：“7”                        |
| rulename   | 是   | string | 规则名称                             |

**返回值**

| 参数名     | 类型   | 说明                                         |
| :--------- | :----- | -------------------------------------------- |
| StatusCode | int    | 返回值代码值（具体请参考附表：返回值代码表） |
| StatusMsg  | string | 返回具体信息                                 |

**返回值**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "the reason file has been remove successfully! file path:./dbhome/reason.db/reason_rule_files/孤儿.json"
}
```



### 附表1 返回值代码表

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



## socket接口说明

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
**（6）query**

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
| password | 是   | string | 密码（明文）                |

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

<!-- <div STYLE="page-break-after: always;"></div> -->

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

