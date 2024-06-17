## 安装和部署

gStore Workbench是gStore团队开发用于在线管理gStore图数据库及对gStore进行查询可视化的web工具，目前gStore官网提供workbench下载，下载链接为http://www.gstore.cn，选择【产品】-【gstore workbench】，填入相关信息后，您将获取一个workbench 压缩包，但需要安装和部署，下面将详细介绍安装部署的步骤。

- **下载tomcat**

workbench 是一个web网站，需要一个web服务器作为web容器来运行，我们推荐采用tomcat8作为web服务器，下载地址为[https://tomcat.apache.org/download-80.cgi](https://tomcat.apache.org/download-80.cgi)。下载压缩包之后要解压。

- **把workbench压缩包放到tomcat的webapps目录并解压**

- **到tomcat的bin目录下**

启动tomcat：

    [root@node1 bin]# ./startup.sh

停止tomcat：

```
[root@node1 bin]# ./shutdown.sh
```



## 登录

### 浏览器访问系统

登录网址为：

```
http://workbench自己部署的服务器ip:8080/gworkbench/views/user/login.html
```

![gWorkbench登录实例](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench/login.png)

### 连接gStore实例

设置远端服务器ip和端口保存到远端的gStore图数据库管理系统，注意远端服务器要安装gStore并启动ghttp服务

输入用户名、密码和验证码登录到已保存服务器上的gStore图数据库管理系统（gstore默认用户名为root，密码为12345）

![gWorkbench连接数据库实例](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/image/gWorkbench%E8%BF%9E%E6%8E%A5%E6%95%B0%E6%8D%AE%E5%BA%93%E5%AE%9E%E4%BE%8BV2.png)







## 查询功能


###  数据库管理

- **查看已加载数据库的信息**

![workbenchmanage](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench/%E6%95%B0%E6%8D%AE%E5%BA%93%E7%AE%A1%E7%90%86.png)

选择要查看的数据库，点击数据库下方的“详情”按钮，会看到数据库的具体信息。

- **新建数据库**

1.输入新建的数据库名称，如lubm

2.有两种方式上传文件：

一种是从服务器上传，输入正确的nt文件或n3文件路径，可以输入绝对路径或相对路径，若是想输入相对路径，注意当前路径为安装gstore的根目录。

例如：路径选择

    /root/gStore/data/lubm.nt   绝对路径
    ./data/lubm.nt              相对路径

另外一种是从本地上传，注意使用这种方式必须保证**workbench部署的服务器与安装gStore的服务器是同一台**。首先从本地选择nt或n3文件,然后点击上传文件。

3.点击提交

![wkbuild](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench/%E6%96%B0%E5%BB%BA%E6%95%B0%E6%8D%AE%E5%BA%93.png)

- **删除数据库**

点击数据库右下角的“删除”按钮，选择删除或者完全删除都会删除数据库。**system数据库不能删除。**

- **导入数据**

点击【数据库管理】，选择要导入的数据库，点击左下角“导入”按钮；文件类型可选择服务器文件和本地文件两种；导入的本地文件需要选择nt或者n3格式的文件，点击【选择上传文件】后，继续点击【导入数据】即可。

![导入数据](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench/%E5%AF%BC%E5%85%A5%E6%95%B0%E6%8D%AE%E5%BA%93.png)

- **导出数据库**

把数据库导出为nt文件，点击数据库左下角的“导出”按钮，选择导出的nt文件所在文件夹路径，可以输入绝对路径或相对路径，若是想输入相对路径，注意当前路径为安装gstore的根目录。

例如：路径选择

    /root/gStore/data   绝对路径
    ./data              相对路径

输入正确的路径后点击立即导出。**system数据库不能导出。**

![wkexport](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench/%E5%AF%BC%E5%87%BA%E6%95%B0%E6%8D%AE%E5%BA%93.png)

- **备份数据库**

点击想要备份的数据库下方的“备份”按钮，弹出如下对话框：

![wkbackup](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench/%E5%A4%87%E4%BB%BD%E6%95%B0%E6%8D%AE%E5%BA%93.png)

- **还原数据库**

![wkrestore](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench/%E8%BF%98%E5%8E%9F%E6%95%B0%E6%8D%AE%E5%BA%93.png)

###  图数据库查询

点击【图数据库查询】，这里包含普通查询、事务操作和高级查询三种功能。

**普通查询**

- 默认界面为普通查询，选择要查询的数据库
- 按照sparql文档输入查询语句，然后点击【查询】，就会在页面展示出详细的查询结果可视化界面
- 查询结束后，**上方会显示菜单栏，里面包含实体类型、布局、分析、图显示设置、选择等功能**。点击右上角下载图标，选择JSON列表/数据列表可以以json/表格的形式查看结果。

**图形化展示**

![image-20220929163045138](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929163045138.png)

**Json展示**

![Json展示](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929164227725.png)

**表格展示**

![表格](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929164346254.png)

**（2）事务操作**

点击【事务操作】，选择数据库，写入相应的事务SPARQL语句，点击【查询】。

![wktransaction](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929165750096.png)

针对三元组数据的插入和删除均可通过写SPARQL语句实现。

**（3）高级查询**

点击【高级查询】，选择数据库、执行函数等信息，点击【查询】，即可得到相应结果。高级查询模块能够降低用户使用难度，不需要用户写对应的SPARQL语句。

![image-20220929170724740](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929170724740.png)

## 高级设置

用户可以通过高级设置模块自定义函数，这些函数也能够在图数据库查询模块中直接调用。

### 查询自定义函数

点击【高级设置】—【自定义函数】模块，输入待查询自定义函数名称，选择函数状态，点击【搜索】，即可查找到目标自定义函数。

![查询自定义函数](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929180053731.png)

### 新增自定义函数

点击【高级设置】—【自定义函数】模块，点击【新增】。

![image-20220930102757815](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220930102757815.png)

填写函数名称、功能描述、参数类型等信息。![image-20220930102823012](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220930102823012.png)

点击【预览】，可对整个函数进行查看。

![image-20220930101423984](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220930101423984.png)

点击【编译】，对自定义函数进行编译。

![image-20220930102124487](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220930102124487.png)

用户也可以点击【删除】，管理自定义函数。

![image-20220930101711773](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220930101711773.png)

### 执行自定义函数

点击【高级设置】—【自定义函数】模块，点击【执行】，输入数据库、执行函数、节点信息、K跳值等信息，点击【执行】即可获得结果。

![excute](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929180639035.png)

## 系统管理

###  IP黑白名单

* 用户可以通过IP黑白名单功能限制能够访问的IP地址。通过黑名单功能阻止黑名单用户使用系统，或是通过白名单功能允许可访问IP地址。

输入黑白名单IP，用","分割，支持范围配置，使用"-"连接，如：ip1-1p2。

![黑白名单](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929171154646.png)

### 查询日志

- 用户可以在web界面上查看系统查询日志

点击【系统管理】—【查询日志】，在搜索栏选择指定日期，点击【搜索】，就可以查看到该日期的具体日志信息，包括客户端IP、SPARQL、查询时间、数据格式、耗时（毫秒）和结果数。

![查询日志](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929171701793.png)

### 事务日志

点击【系统管理】—【事务日志】查看具体事务信息，包括TID、数据库名、操作用户、状态、开始时间、结束时间等。

![事务日志](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929171900518.png)

同时，还可以对事务进行提交和回滚操作。

![事务日志1](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929172042161.png)

### 操作日志

点击【系统管理】—【操作日志】，在搜索栏选择指定日期，点击【搜索】，就可以查看到该日期的操作日志信息，包括客户端IP、操作类型、操作时间、操作结果和描述。

![操作日志](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929172302593.png)

### 定时备份

点击【定时备份】，然后点击【新增任务】，依次填入定时方式、任务名称、数据库名称和备份路径，就可以增加新的备份任务。

![wkbackup](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929172541859.png)

### 用户管理（只有root用户有该权限)

**（1）新增用户**

- 添加新用户

输入用户名和密码添加用户

![adduser](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929173640450.png)

**（2）用户授权**

- 对用户进行功能授权

选择需要授权的用户和数据库，添加或删除查询、加载、卸载、更新、备份、还原和导出权限。

![privilege](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929173819819.png)

**（3）账户编辑**

- 对用户账户的具体信息进行编辑

点击【用户管理】，选择某一用户账户，点击操作栏下的【修改密码】，输入相关信息后点击【提交】，即可对用户密码进行修改。

![edituser](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929174109182.png)

**（4）账户删除**

- 对用户账户进行删除

点击【用户管理】，选择某一用户账户，点击操作栏下的【删除】即可删除该用户。

![deluser](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929174139115.png)

<div STYLE="page-break-after: always;"></div>
