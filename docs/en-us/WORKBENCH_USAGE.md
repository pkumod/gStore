## Installation and deployment

gStore Workbench is a web tool developed by gStore team for online management of gStore graph database and query visualization of gStore. Currently, gStore official website provides Workbench download, the download link is http://www.gstore.cn. Select [Product] - [gStore Workbench] and after filling in the relevant information, you will get a Workbench package, but you will need to install and deploy. The steps to install and deploy are described in detail below.

- **Download tomcat**

Workbench is a website that requires a Web server as a Web container to run, and we recommend tomcat8 as the Web server, which can be downloaded from https://tomcat.apache.org/download-80.cgi. After downloading the zip package, unzip it.

- **Put the Workbench package in tomcat's webapps directory and unzip it;**
- **Under the bin directory of tomcat**

Start tomcat:

```
[root@node1 bin]# ./startup.sh  
```

Stop tomcat:

```
[root@node1 bin]# ./shutdown.sh
```

## Login

### Browser Access to the System

Login address is：

```
http://workbench`Self-deployed servers`ip:8080/gworkbench/views/user/login.html
```

 ![gWorkbench登录界面](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/image/gWorkbench%E7%99%BB%E5%BD%95%E7%95%8C%E9%9D%A2.png)

### Connect to gStore instance

Set the IP address and port of the remote server and save the IP address and port number of the remote server to the gStore diagram database management system. Note that the remote server must install gStore and start the GHTTP service  

Enter the user name, password, and verification code to log in to the gStore graph database management system on the saved server (gStore default user name root, password 12345）

![gWorkbench连接数据库实例](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/image/gWorkbench%E8%BF%9E%E6%8E%A5%E6%95%B0%E6%8D%AE%E5%BA%93%E5%AE%9E%E4%BE%8B.png)







## Query functions 

### Database management  

- **View information about the loaded database**

![workbenchmanage](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20220929161106585.png)

Select the database to view, click the "Details" button below the database, and you will see the specific information of the database.

- **Create a database **

1.Enter the name of the new database, for example, lubm;

2.Upload files in one of the following two ways:
One is to upload from the server. Enter the correct path of the NT file or N3 file. You can enter an absolute path or a relative path; note that a relative path should be relative to the root directory of gStore.

For example, the following two paths are equivalent:

```
/root/gStore/data/lubm.nt 	 //absolute path
./data/lubm.nt				//relative path
```

The other option is to upload from the local machine. Note that using this approach you must ensure that the **Workbench server is the same server on which gStore is installed**. First select the NT or N3 file locally, then click "Upload File".

3.Click on the "Submit" button.

![image-20221031151425970](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151425970.png)

- **Database deletion**

Click the delete button in the upper right corner of the database and select "Delete" or "Delete Completely" to delete the database. **The system database cannot be deleted.**

- **Import data**

Click [Database Management], select the database to import, click the "Import" icon in the upper right corner; the file type can be server file or local file. To import local files, select nt or N3 format files, click [Upload File], and then click [Import Data].

![image-20221031151443124](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151443124.png)

- **Export data**

To export the database as an NT file, click the export button in the lower left corner of the database and select the directory where the exported NT file will reside. You can enter an absolute path or a relative path; note that the current path is the root directory of gStore.

For example, the following two paths are equivalent:

```
/root/gStore/data   //absolute path
./data 				//relative path
```

Enter the correct path and click "Export Now". **The system database cannot be exported.**

![image-20221031151455054](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151455054.png)

- **Backup database**

Click the [Back up] button on the database you want to back up, and the following dialog box will pop up.

![image-20221031151613155](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151613155.png)

- **Restore database** 

![image-20221031151620936](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151620936.png)

### Graph database query 

Click [Graph Database query], which contains three functions: ordinary query, transaction operation and advanced query.

**(2) Ordinary query**

- The default interface is ordinary query. Select the database to query.
- Input the query according to the SPARQL document, then click [Query], and the detailed visual interface of the query results will be displayed on the page.
- After the query, **a menu bar will be displayed, which includes functions such as entity type, layout, analysis, graph display settings, and selection.** Click the download icon in the upper right corner and select JSON list/data list to view the results in the form of json/table.

**Graphical display**

![image-20221031151656236](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151656236.png)

**Json display**

![image-20221031151710965](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151710965.png)

**Table display**

![image-20221031151730254](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151730254.png)

**(2) Transaction operation**

Click [Transaction Operation], select a database, input the SPARQL statement you want to run in transaction mode, and click [Query].

![image-20221031151746612](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151746612.png)

The insertion and deletion of triples can be achieved by SPARQL statements.

**(3) Advanced query**

Click [Advanced Query], select a database and a function to execute, fill in other information, and click [Query] to get the corresponding results. The advanced query module can reduce the difficulty of posing queries, since it does not require users to write a full SPARQL statement.![image-20221031151801506](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151801506.png)

### Knowledge hierarchical exploration

Knowledge hierarchical exploration allows users to query by selecting the database, entities, entity attributes, and attribute values, simplifying data retrieval. Except for attribute values, the other three fields are selected from dropdown menus, refining the query step by step from left to right.

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E9%80%90%E7%BA%A7%E6%8E%A2%E7%B4%A21.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084916744&Signature=Jbt1iZLkiY4ENz8JMeyGgxMXPpM%3D)

After completing the information, the query can be performed as shown below (the last item is optional).

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E9%80%90%E7%BA%A7%E6%8E%A2%E7%B4%A22.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084916870&Signature=UpyW1BkLAl8Y6X9kJG2MvmPw9y4%3D)

The functionality here is consistent with the one described in the basic query section, so it won't be elaborated further.

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E9%80%90%E7%BA%A7%E6%8E%A2%E7%B4%A23.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084916930&Signature=LhC0aCiGvrLuSfISCEVVuBnerO8%3D)

### Knowledge association analysis

The Knowledge Association Analysis interface is designed to help you explore the complex relationships between entities, entity types, and entity attributes. Through knowledge association analysis, you can compare relationships between different entities and query the associations between two entities within a specified number of hops, revealing hidden deep relationships in the knowledge graph. For clarity, we'll refer to the two nodes being analyzed as A and B, focusing on the relationship between them.

 Select the database to use:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E5%85%B3%E8%81%94%E5%88%86%E6%9E%901.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084917104&Signature=KE0vPp%2BHwq2tivxFgzyn9wI%2FZvY%3D)

The two nodes selected within the red boxes represent the nodes we've chosen (note that if no attribute values are entered, the relationship between the set of entities with that attribute and another node is analyzed. If both nodes are selected with attributes, you can consider it as a relationship between two sets, each containing only one element).

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E5%85%B3%E8%81%94%E5%88%86%E6%9E%902.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084917126&Signature=Ifxwqx1cJkQDYFt9%2FD6scUm4GHI%3D)

The number of hops indicates the maximum number of steps needed to reach the target node:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E5%85%B3%E8%81%94%E5%88%86%E6%9E%903.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084917154&Signature=FgTDy2L6GB2fm%2FDfwKytIOZ0lXw%3D)

## Knowledge Management

### Knowledge Update

Knowledge update includes adding new nodes, updating relationships, updating attributes, and switching layouts. It also allows for database queries and viewing of historical records.

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B01.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919226&Signature=2IcrVi5pHMbtn9EU1cRu6FAJu4Y%3D)

Click 'Add New Node' to add multiple node names:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B02.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919249&Signature=dq4MeraCttXOnQTzjhmzPLScU3Y%3D)

Click 'Update Relationships' to select 'Add Relationship' or 'Edit Relationship'. Click 'Add Relationship' to add multiple node relationships:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B03.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919273&Signature=avXGHxWudiHKNTYlIqhSOGDZcGI%3D)

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B04.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919291&Signature=MVT5XWUj7n3nWG290TCCIKIrgvE%3D)

In 'Edit Relationships,' you can edit the nodes, relationships, and node information in the queried database:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B05.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919312&Signature=k1jKnUNII%2F8yKw%2BIYM0CfOfzIHI%3D)

Click 'Update Attributes' to select 'Add Attributes' or 'Edit Attributes.' Click 'Add Attributes' to add multiple nodes, attribute names, and attribute values:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B06.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919331&Signature=6T%2FgHo3VCMs7jfUQg%2BFLEhpjyj0%3D)

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B07.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919349&Signature=U7BFILl1o1MSiGW32MaAZ%2BIX5eM%3D)

Click 'Update Attributes' to edit attributes, allowing you to query, modify, and delete nodes, attribute names, and attribute values:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B08.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919366&Signature=FAabrKbII%2FRjlOdn9wP%2BncESGLs%3D)

The elastic layout supports various graph layout forms:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B09.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919381&Signature=DvozsPXS3gtRZ6aP2dUMM%2BvjJVo%3D)

Right-click on an entity node to access the following functions:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B010.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919403&Signature=C8rlxb2WaNVONK9%2BBLAI3P4KdrI%3D)

（1）Node Information:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B011.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919418&Signature=t8gRSdKQnhKQqOJEO7nvmQKAUHI%3D)

（2）Edit Node (Modify Node Name):

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B012.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919434&Signature=r1RezU%2FX%2Fx2yjousrM3Nr9cKDwk%3D)

（3）Add Relationship (Draw a line pointing to another entity node):

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B013.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919525&Signature=dCbPnaATd9u8saAgrWJIyjHDVBI%3D)

Enter the relationship name and submit to save.

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B014.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919539&Signature=mOYbxrGu%2BaT6iLtoaYG0XxYJP1w%3D)

（4）Add Attribute (Same as the 'Add Attributes' operation described above):

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B015.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919554&Signature=VcTSiPFkpHDo0ImGTlhluE3czeQ%3D)

（5）Delete Node (Remove the entity node)

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B016.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919568&Signature=vIbzYWPUckhWMmjEX7n%2FB6bn91Q%3D)

（6）Expand Relationships (Display nodes related to the selected node):

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B017.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919586&Signature=KepvusknNBhxaPyIGPo3VyUH03o%3D)

（7）Expand Attributes (Show the attributes of the selected node):

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B018.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919601&Signature=OeHjF2nBYe4bICtLB%2BTX6TfYzmo%3D)

Right-click on an entity node's attribute to access the following functions:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B019.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919621&Signature=Y3WAo5P9nvMu1QQOCoK5oAiE2%2Fg%3D)

（1）Edit Attribute (Change the attribute name and value):

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B020.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919638&Signature=YoGPTRKa9AvOBKgxlfbG1TyyiY8%3D)

（2）Delete Attribute (Remove the attribute from the node):

Search (Enter a node name to find its location on the graph):

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B021.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919764&Signature=btyjdHxAUbf5HFmsErpD0KB6spA%3D)

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B022.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919786&Signature=Ff%2FvKQmTGjZ7bl4g%2BXY%2BFGphjjg%3D)

Check Updates:

（1）Where the action buttons are located:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B023.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919804&Signature=hw%2FJpocGO6IO5rIlFOeLz3hVovk%3D)

（2）Displayed content:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B024.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919819&Signature=BW4uCK8dvmLjwGMz5MTj4XNGz1g%3D)

（3）Save:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B025.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919833&Signature=9XMPrMNoEFTOOeF1kmcq3SopLvw%3D)

Save Knowledge Updates:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B026.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919848&Signature=QzA8Fs%2BfBP3t0ayef4TSTCsMu5E%3D)

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B027.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919871&Signature=3NXJXJcYBfQ0VeQWO22AtJc8HRs%3D)

Cancel Knowledge Updates (Revert unsaved update operations):

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E7%9F%A5%E8%AF%86%E6%9B%B4%E6%96%B028.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084919889&Signature=qCgMl%2FLHm11O5UOVi0Z1jZx15hA%3D)



## Advanced settings

Users can define custom graph analysis functions through the advanced settings module, which, after definition, can also be called directly from the graph database query module.

### Custom function

**Find existing custom function**

Click the [Advanced Settings] - [Customized Function] module, enter the name of the custom function to be queried, select the function status, and click [Search] to find the target custom function.

![image-20221031151815975](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151815975.png)

**Create new custom function**

Click the [Advanced Settings] - [Custom Function] module, and then click [New].

![image-20221031151829988](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151829988.png)

Enter the function name, function description, and parameter type.

![image-20221031151838820](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151838820.png)

Click [Preview] to view the whole function.

![image-20221031151849170](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151849170.png)

Click [Compile] to compile the custom function.

![image-20221031151858080](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151858080.png)

Users can also click [Delete] to delete custom functions.

![image-20221031151912507](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151912507.png)

**Execute custom function**

Click [Advanced Settings] - [Custom Function] module, click [Execute], input the database, function to be executed, source and/or destination nodes, K-hop value and other information, then click [Execute] to get the result.

### Inference engine management

The inference engine automatically updates the database based on the information we input into the rules. I will provide an example below. I have prepared the following data:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E1.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921183&Signature=ANHwKkvIVjFJdJic2DC0KJin20U%3D)

Select a database:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E2.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921204&Signature=RUFqVBWztp1mt4sghkLuMA3JoKE%3D)

Add a new rule:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E3.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921264&Signature=yxG6boMed58wJY78RlphyMlw2uA%3D)

The functions of the first three options are as follows:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E4.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921279&Signature=sAU8NmksTsIIpP%2Bsi96LATUEaaY%3D)

Relationship Inference Rules

（1）AND (both inference conditions must be met)

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E5.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921296&Signature=Vw%2FxXCoAGj3xxKiObbK0QsTs4mM%3D)

Compile here first, and a statement to be executed will be generated:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E6.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921310&Signature=c6%2FLmVNrxRFejjYfUACns4yR97Q%3D)

Preview:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E7.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921325&Signature=KINU1lL8PKCC%2B2hLiFZTiBh7w2A%3D)

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E8.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921339&Signature=82vropvhb86%2FVu1AJfIbmzOwpII%3D)

Execute:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E9.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921354&Signature=XyhUmqyidEGZxQco1Qzjv0681Ow%3D)

The result is as follows:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E10.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921369&Signature=PY73ArKIpSA7%2BHjZpGsZM4zaiFw%3D)

Click "Invalidate" to undo this operation:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E11.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921386&Signature=iMFBR%2FtfL%2FIumI1cqRU74xnMvYs%3D)

（2）OR (only one of the inference conditions needs to be met)

Simply modify this part of the previous rule:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E12.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921398&Signature=yDRRC9Tw4YaVdOTm8IKjqBimBsY%3D)

The remaining steps are the same as for AND.

The result is as follows:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E13.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921415&Signature=9Rs3bJebznH26YNVAyg%2BdTBzbc0%3D)

Attribute Rules

Attribute rules change or add the specified properties of the corresponding node when conditions are met. The other steps are the same as in the previous inference rules, so I will only show the modified pages and results.

Select inference type, input inference conditions, and specify return results.

This is the original data:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E14.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921435&Signature=aCXiDPiqo8nLjb9xE4XdqC8yvoI%3D)

This is the rule:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E15.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921450&Signature=aFQ3g6wESVpnDEWjMfdxbgp22b4%3D)

The result is as follows:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E16.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921462&Signature=j%2FHcU%2B2lHiJYxly0DBfZS4B16X8%3D)

Using a filter condition, for example, where y can only be Zhang Cuishan:

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/workbench%2F%E6%8E%A8%E7%90%86%E5%BC%95%E6%93%8E17.png?OSSAccessKeyId=LTAI4FqEvNC5Seewy6qYrjKp&Expires=2084921476&Signature=ykUQ3Y3P%2B4baEPrXCd1Dwxou6K8%3D)

## System management

### IP blacklist and whitelist

- Users can use the IP blacklist and whitelist function to limit the IP addresses that can initiate accesses. Users can use the blacklist function to prevent blacklisted users from using the system, or you can use the whitelist function to allow accessible IP addresses.

Enter the blacklist and whitelist IP address, separated by commas (,). Range configuration is supported using "-", such as: IP1-1P2.

![image-20221031151928723](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151928723.png)

### Log management

- User can view system logs on the web UI.

Click on the [system management] - [log management], select a specific date in the search bar, and click "Search" to view the specific log information of this date, including name of the log file, the client IP, SPARQL, query time, data format, the execution time (ms), and the number of results.

![image-20221031151936355](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151936355.png)

### Transaction management  

Click [System Management] - [Transaction Management] to view specific transaction information, including TID, database name, the user initiating the operation, transaction status, start time, end time, etc.

![image-20221031152044603](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152044603.png)

Meanwhile, transactions can also be committed and rolled back.

![image-20221031152052258](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152052258.png)

### Operation Log

Click [System Management] - [Operation Log], select a specific date in the search bar and click "Search" to view the specific log information of this date, including client IP address, operation type, operation time, operation result and description.

![image-20221031152147850](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152147850.png)

### Scheduled backup

Click [Scheduled Backup] and then [New Task] to add a new backup task by filling in the scheduling
mode, the task name, the database name and the backup path.

![image-20221031152200383](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152200383.png)

### User management 

**Tips: Only for root user**

**(1) Add user**

- Add new users

Enter the user name and password to add a user.

![image-20221031152216736](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152216736.png)

**(2) User authorization**

- Authorize functions for users.

Select the users and databases that you want to authorize, and add or remove query, load, unload, update, backup, restore, or export permissions.

![image-20221031152229551](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152229551.png)

**(3) Edit accounts**

- Edit user account details.

Click [User Management], select a User account, click [Change password] under the operation bar, input relevant information and click [Submit] to modify the user's password.

![image-20221031152240616](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152240616.png)

**(4) Delete accounts**

- Delete user accounts.

Click [User Management], select a User account, and click [Delete] under the operation bar to
delete the User.

![image-20221031152249581](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152249581.png)

<div STYLE="page-break-after: always;"></div>