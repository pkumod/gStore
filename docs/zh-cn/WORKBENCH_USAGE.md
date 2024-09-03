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

### 知识逐级探索

知识逐级探索主要通过用户选择数据库、实体、实体属性、属性值来进行查询，使得数据查询得到简化，除属性值需要输入外，前三向均在下拉选项中选择，从左到右逐级细化。

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E9%80%90%E7%BA%A7%E6%8E%A2%E7%B4%A21.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084916744&Signature=Jbt1iZLkiY4ENz8JMeyGgxMXPpM%3D)

将信息填写完整后可进行查询如下（最后一项可不填写）

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E9%80%90%E7%BA%A7%E6%8E%A2%E7%B4%A22.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084916870&Signature=UpyW1BkLAl8Y6X9kJG2MvmPw9y4%3D)

此处功能与上文普通查询描述的功能一致不再赘述

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E9%80%90%E7%BA%A7%E6%8E%A2%E7%B4%A23.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084916930&Signature=LhC0aCiGvrLuSfISCEVVuBnerO8%3D)

### 知识关联分析

在知识关联分析界面中，旨在帮助您深入探索实体与实体之间的复杂关系、实体的类型以及实体的属性值。，通过知识关联分析您不仅可以对比不同实体之间的关系，还能查询两个实体之间在指定跳数内存在的关联。揭示知识图谱中隐藏的深层关系。为方便说明此处用A和B来指代两个节点，此处分析的便是这两个节点间的关系。

 选择使用的数据库

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E5%85%B3%E8%81%94%E5%88%86%E6%9E%901.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084917104&Signature=KE0vPp%2BHwq2tivxFgzyn9wI%2FZvY%3D)

下面的两个红框框选的就是我们选择的节点（注意如果属性值不填写，那么就是有该属性的实体集合和另外的一个节点间的关系，此处两个节点都选择了属性的话，你也可以认为是两个都只有一个元素的集合间的关系）

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E5%85%B3%E8%81%94%E5%88%86%E6%9E%902.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084917126&Signature=Ifxwqx1cJkQDYFt9%2FD6scUm4GHI%3D)

这里的层数表示最多经过几次跳转到达目标节点

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E5%85%B3%E8%81%94%E5%88%86%E6%9E%903.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084917154&Signature=FgTDy2L6GB2fm%2FDfwKytIOZ0lXw%3D)

## 知识管理

### 知识更新

知识更新主要包括新增节点、更新关系、更新属性、切换布局四个功能，也可进行数据库查询和历史记录查看。

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B01.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919226&Signature=2IcrVi5pHMbtn9EU1cRu6FAJu4Y%3D)

点击新增节点，新增行可添加多个节点名称。

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B02.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919249&Signature=dq4MeraCttXOnQTzjhmzPLScU3Y%3D)

点击更新关系，可选新增关系和编辑关系，点击新增关系可添加多个节点关系。

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B03.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919273&Signature=avXGHxWudiHKNTYlIqhSOGDZcGI%3D)

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B04.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919291&Signature=MVT5XWUj7n3nWG290TCCIKIrgvE%3D)

点击更新关系中编辑关系，可以编辑查询到的数据库中节点、关系、节点信息。

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B05.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919312&Signature=k1jKnUNII%2F8yKw%2BIYM0CfOfzIHI%3D)

点击更新属性，可选新增属性和编辑属性，点击新增属性可添加多个节点，属性名称和属性值。

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B06.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919331&Signature=6T%2FgHo3VCMs7jfUQg%2BFLEhpjyj0%3D)

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B07.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919349&Signature=U7BFILl1o1MSiGW32MaAZ%2BIX5eM%3D)

点击更新属性，编辑属性，可查询和对节点、属性名称及属性值修改和删除。

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B08.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919366&Signature=FAabrKbII%2FRjlOdn9wP%2BncESGLs%3D)

弹性布局可支持多种形式图谱布局。

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B09.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919381&Signature=DvozsPXS3gtRZ6aP2dUMM%2BvjJVo%3D)

右键点击一个实体的节点，出现以下功能

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B010.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919403&Signature=C8rlxb2WaNVONK9%2BBLAI3P4KdrI%3D)

（1）节点信息

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B011.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919418&Signature=t8gRSdKQnhKQqOJEO7nvmQKAUHI%3D)

（2）编辑节点（修改节点名称）

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B012.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919434&Signature=r1RezU%2FX%2Fx2yjousrM3Nr9cKDwk%3D)

（3）新增关系（通过出现的线条指向一个实体节点）

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B013.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919525&Signature=dCbPnaATd9u8saAgrWJIyjHDVBI%3D)

输入关系名称并提交保存

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B014.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919539&Signature=mOYbxrGu%2BaT6iLtoaYG0XxYJP1w%3D)

（4）新增属性（和上文的新增属性操作一致）

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B015.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919554&Signature=VcTSiPFkpHDo0ImGTlhluE3czeQ%3D)

（5）删除节点（删除该实体节点）

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B016.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919568&Signature=vIbzYWPUckhWMmjEX7n%2FB6bn91Q%3D)

（6）展开关系（展示和该节点有关系的节点）

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B017.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919586&Signature=KepvusknNBhxaPyIGPo3VyUH03o%3D)

（7）展开属性（展开选择的节点的属性）

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B018.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919601&Signature=OeHjF2nBYe4bICtLB%2BTX6TfYzmo%3D)

右键点击一个实体节点的属性，出现以下功能

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B019.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919621&Signature=Y3WAo5P9nvMu1QQOCoK5oAiE2%2Fg%3D)

（1）编辑属性（对属性的名称和值做改变）

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B020.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919638&Signature=YoGPTRKa9AvOBKgxlfbG1TyyiY8%3D)

（2）删除属性（删除该节点的该属性）

搜索（输入节点名称，查找到对应节点在图中的位置）

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B021.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919764&Signature=btyjdHxAUbf5HFmsErpD0KB6spA%3D)

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B022.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919786&Signature=Ff%2FvKQmTGjZ7bl4g%2BXY%2BFGphjjg%3D)

查看更新

（1）操作按钮所在

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B023.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919804&Signature=hw%2FJpocGO6IO5rIlFOeLz3hVovk%3D)

（2）显示的内容

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B024.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919819&Signature=BW4uCK8dvmLjwGMz5MTj4XNGz1g%3D)

（3）保存

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B025.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919833&Signature=9XMPrMNoEFTOOeF1kmcq3SopLvw%3D)

知识更新保存

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B026.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919848&Signature=QzA8Fs%2BfBP3t0ayef4TSTCsMu5E%3D)

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B027.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919871&Signature=3NXJXJcYBfQ0VeQWO22AtJc8HRs%3D)

知识更新取消保存（取消之前未保存的更新操作）

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B028.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919889&Signature=qCgMl%2FLHm11O5UOVi0Z1jZx15hA%3D)



## 高级设置

用户可以通过高级设置模块自定义函数，这些函数也能够在图数据库查询模块中直接调用。

### 自定义函数

**查询自定义函数**

点击【高级设置】—【自定义函数】模块，输入待查询自定义函数名称，选择函数状态，点击【搜索】，即可查找到目标自定义函数。

![查询自定义函数](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929180053731.png)

**新增自定义函数**

点击【高级设置】—【自定义函数】模块，点击【新增】。

![image-20220930102757815](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220930102757815.png)

填写函数名称、功能描述、参数类型等信息。![image-20220930102823012](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220930102823012.png)

点击【预览】，可对整个函数进行查看。

![image-20220930101423984](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220930101423984.png)

点击【编译】，对自定义函数进行编译。

![image-20220930102124487](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220930102124487.png)

用户也可以点击【删除】，管理自定义函数。

![image-20220930101711773](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220930101711773.png)

**执行自定义函数**

点击【高级设置】—【自定义函数】模块，点击【执行】，输入数据库、执行函数、节点信息、K跳值等信息，点击【执行】即可获得结果。

![excute](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929180639035.png)

### 推理引擎管理

推理引擎会根据我们在规则中填写的一些信息自动地在数据库中进行数据库的更新，具体我会在下面的示例中给出。我准备了下面的数据

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E1.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921183&Signature=ANHwKkvIVjFJdJic2DC0KJin20U%3D)

选择数据库

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E2.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921204&Signature=RUFqVBWztp1mt4sghkLuMA3JoKE%3D)

新增规则

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E3.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921264&Signature=yxG6boMed58wJY78RlphyMlw2uA%3D)

前三个选项的作用如下

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E4.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921279&Signature=sAU8NmksTsIIpP%2Bsi96LATUEaaY%3D)

关系推理规则

（1）AND（两个推理条件都要满足）

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E5.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921296&Signature=Vw%2FxXCoAGj3xxKiObbK0QsTs4mM%3D)

先在此处编译，此时会生成将执行的语句

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E6.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921310&Signature=c6%2FLmVNrxRFejjYfUACns4yR97Q%3D)

预览

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E7.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921325&Signature=KINU1lL8PKCC%2B2hLiFZTiBh7w2A%3D)

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E8.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921339&Signature=82vropvhb86%2FVu1AJfIbmzOwpII%3D)

执行

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E9.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921354&Signature=XyhUmqyidEGZxQco1Qzjv0681Ow%3D)

结果如下

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E10.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921369&Signature=PY73ArKIpSA7%2BHjZpGsZM4zaiFw%3D)

点击失效将撤回这个操作

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E11.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921386&Signature=iMFBR%2FtfL%2FIumI1cqRU74xnMvYs%3D)

（2）OR（推理条件只需满足一个即可）

此处仅仅需要将之前规则上修改此处

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E12.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921398&Signature=yDRRC9Tw4YaVdOTm8IKjqBimBsY%3D)

其余操作与AND一致

结果如下

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E13.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921415&Signature=9Rs3bJebznH26YNVAyg%2BdTBzbc0%3D)

属性规则

属性规则是在满足条件后改变或新增对应节点的我们指定的属性，其余操作和上面的规则推理一致于是只展示改变的页面和结果。

选择推理类型、填写推理条件、填写返回结果。

这是原始的数据

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E14.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921435&Signature=aCXiDPiqo8nLjb9xE4XdqC8yvoI%3D)

这是规则

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E15.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921450&Signature=aFQ3g6wESVpnDEWjMfdxbgp22b4%3D)

结果如下

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E16.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921462&Signature=j%2FHcU%2B2lHiJYxly0DBfZS4B16X8%3D)

过滤条件的使用，例如y只能是张翠山

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E17.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921476&Signature=ykUQ3Y3P%2B4baEPrXCd1Dwxou6K8%3D)

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

### 用户管理

**说明：只有root用户有该权限**

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
