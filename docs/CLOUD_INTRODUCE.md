## 简介

### gStore是什么？

gStore是一个由北京大学王选计算机所数据管理实验室研发的，基于图的RDF三元组存储的数据管理系统，可以用来管理庞大的互相联系的数据，拥有源头创新、标准系统、性能优越、自主可控四大优点。

### gStore云平台是什么？

gStore云平台是gStore系统的云端服务版本，可以在网上注册并审核通过后使用，不需要下载安装。

### gStore有什么用？

gStore可以用于大规模数据的处理，这让其拥有很广的用途，包括但不限于政府大数据、金融科技、智慧医疗、人工智能等。

### gStore如何在以上事务中发挥作用？

以金融科技为例，该系统可以通过图数据库的方式进行多级股权的查询，在本例中最多可查出五层的股权关系数据。



## 使用方式

### 注册与登录

云平台网址：http://cloud.gstore.cn  

云平台由gStore统一身份认证页面登录

![image-20221011104848163](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011104848163.png)
如没有gStore统一身份认证账号，需要进行注册,注册界面如下：

![image-20221011104939961](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011104939961.png)

注册后，即可通过图谱门户申请试用云平台：

![image-20221011105416451](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011105416451.png)

![image-20221011105249889](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011105249889.png)

如已有云平台账号，绑定账号后即可进入云平台。

![image-20221011105333411](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011105333411.png)![image-20221011105452584](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011105452584.png)

### 平台首页

平台首页如下图所示，将展示当前已构建数据库数量，三元组总数以及到期时间等，以及平台相关资讯信息（包括新闻资讯和版本信息等），点击相关资讯可以查看详情。另外，gStore 官网、github 地址、gitee 地址、社区论坛等一些常用链接信息可以在上方工具栏中点击进入。

![image-20221011105854860](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011105854860.png)



### 个人中心

个人中心在界面右上角。 

![3](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011110028585.png)
进入个人中心后可以查看用户基本信息和本周操作日志。  

用户基本信息中包括KeyID和Secret，这两个值用来在其他程序与gStore云平台的对接中作为密钥使用。

![image-20221011110134153](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011110134153.png)

### 数据库管理

左边区域是系统菜单，包括会员专区和帮助中心两大区域。  
会员专区又分为两大功能模块，数据库管理和数据库查询。  

**（1）启动数据库实例**

在数据库管理中有一个很重要的功能：实例。在第一次进入系统时，实例可能是停止状态，需要手动启动。  

![image-20221011114527409](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011114527409.png)



**（2）查看数据库信息**

启动实例后，可以从上面一行中看到实例的状态，已创建数据库与最多可创建数据库的个数，gStore过期时间，三元组的总数等信息。  
在下方，可以看到现在已创建的数据库，其中包括一个系统数据库（系统创建，不能操作，不算在最多可创建数据库个数里）和若干业务数据库（自己创建，可以操作）。  

![image-20221011114552601](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011114552601.png)

对业务数据库可以进行创建数据库，删除数据库，导出数据库，获取数据库相关信息等操作。  
点击某个数据库可以获取其相关信息，包括创建者、创建时间、三元组数量、实体数量、字符数量、主语数量、谓语数量、连接数量等。如下图为movie数据库的相关信息：  
![image-20221011114624096](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011114624096.png)

**（3）创建数据库**

点击【新建数据库】，可以创建数据库： （**由于资源有限，目前每个用户创建数据库数量限制为5个，每个数据库三元组数量限制为100万，如有需要可以向管理员申请扩容**） 

![image-20221011114109352](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011114109352.png)
创建数据库有三种方法，第一种是本地文件，即从本地上传一个文件到服务器上。目前系统只支持nt文件，将来可能会支持n3文件。  
![image-20221011114131559](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011114131559.png)

![image-20221011114148780](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011114148780.png)
注意，文件大小不能超过2GB，行数不能超过一百万  
第二种创建数据库的方法是远程关系型数据库，即远程访问网络上的数据库，将其导入到云平台上。目前云平台支持MySQL、Oracle、SQLServer、Postgre四种关系型数据库。创建数据库时需输入其相关信息，再生成D2RQ脚本，即可生成数据库。  

![image-20221011114245860](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011114245860.png)  
第三种创建数据库的方法是使用示例数据库。现在云平台的示例数据库中有电影数据库、基因数据库、诗歌数据库。以电影数据库为例，含有400多万三元组，里面包含电影、导演、演员、上映时间、电影评分等相关信息。（**示例数据库三元组数量不受单个数据库100万条三元组数量限制**)  

![image-20221011114221859](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011114221859.png)

**（4）导入数据库**

点击某个数据库右上角导入图标用户可以导入数据到已有数据库

![image-20221011115643940](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011115643940.png)

![image-20221011115624278](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011115624278.png)

选择文件上传，点击【导入数据】即可。

**（5） 导出数据库**

点击某个数据库右上角导出标志，可以导出该数据库。  
![image-20221011114753960](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011114753960.png)

![image-20221011114925800](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011114925800.png)点击导出后，会创建一个zip压缩文件，下载后解压便得到对应该数据库的nt文件。之后，数据库更名，数据库备份等更多的数据库功能也将可能上线。 

**（6）删除数据库**

点击某个数据库右上角左边的垃圾桶标志，可以删除该数据库。  
系统会为所有被删除的数据库提供15天的恢复期，以防止误删。
![image-20221011114723562](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011114723562.png)

### 数据库查询

在数据库查询方面，gStore云端系统提供了一种可视化的查询界面，在下图左侧的文本框中输入Sparql语句以得到结果。  (**注意：考虑系统性能，关系图和JSON数据默认展示100条数据，可以通过点击“下载”按钮获取所有返回数据**)
![7d38bdc7a5975dfab944d674bd4f81a](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/7d38bdc7a5975dfab944d674bd4f81a.png)
对Sparql语句不太熟悉的用户可以通过帮助中心区域中的SPARQL示例功能模块了解Sparql语句。

![image-20221011145754282](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011145754282.png)

以示例中第一个问题为例，查询刘亦菲的星座，得到结果如下：  
![image-20221011115320895](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221011115320895.png)
可以看到，左边是可视化的图形结果，右边是JSON数据的文字结果。  
同时，也可以用Sparql语句插入或删除数据库中的数据。  

### 帮助中心

为了更好的为用户提供服务，平台为用户提供了多种帮助文档信息，并在后续将不断完善和丰富文档信息。目前主要提供了如下文档信息：

**（1）平台使用手册**

平台使用手册主要是对gStore云平台的使用进行说明，让用户了解平台使用的相关问题

**（2） API帮助文档**

用户除了直接使用gStore云平台对图数据进行管理和查询以外，还可以使用API方式直接访问数据，API帮助文档中详细介绍了接口参数和返回值信息。

**（3） SPARQL示例**

部分用户可能对SPARQL不是很熟悉，为此平台提供了SPARQL示例文档，该文档以示例数据库Movie为例，详细介绍了目前平台支持的主要SPARQL语句，用户可以直接在该帮助文档中复杂SPARQL语句并在数据库查询功能中进行测试。



### API

应部分用户要求，我们将数据库查询操作封装成API接口，用户可以通过该接口实现远程数据库访问，方便用户嵌入其他系统，对接中需要使用KeyID和Secret。现在平台拥有三个数据接口，分别为获取当前数据列表、获取数据库详细信息、查询数据库。具体操作可参见帮助中心中的API帮助文档模块。  

## 结束

gStore云平台的帮助手册就此结束，如果您有什么使用上的问题，可以点击云平台右上角的“社区论坛”，在社区中提出意见。

<div STYLE="page-break-after: always;"></div>
