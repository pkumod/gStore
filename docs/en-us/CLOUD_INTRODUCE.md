## Brief introduction

### What is gStore？

gStore is a graph-based RDF triplet storage data management system developed by the Data Management Laboratory of Wangxuan Institute of Computer Technology of Peking University . It can be used to manage huge interconnected data. It has four advantages: original innovation, standard system, superior performance, and independent control

### What is gStore cloud platform？

The gStore cloud platform is the cloud service version of the gStore system. It can be used after being registered online and approved. No download or installation is required.

### What is the use of gStore

gStore can be used for large-scale data processing, which gives it a wide range of uses, including but not limited to government big data, fintech, smart healthcare, artificial intelligence, etc.

### How does gStore play a role in the above transactions？

Taking fintech as an example, the system can query multi-level equity through graph database. In this case, up to five layers of equity relationship data can be found.

## How to use

### Registration and Login

Cloud Platform website：http://cloud.gstore.cn

Users can log in to the cloud platform through the gStore unified identity authentication page.

![image-20221031152309716](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152309716.png)

If you are using the gStore cloud platform for the first time, you need to register. The registration
page is as follows:

![image-20221031152318080](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152318080.png)

After registration, you can apply for a trial usage of the cloud platform through the portal:
![image-20221031152345445](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152345445.png)

If you already have a cloud platform account, please bind the account to log in to the cloud platform.

![image-20221031152355859](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152355859.png)
![image-20221031152404607](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152404607.png)

### Platform home page  

The homepage of the platform is shown in the figure below, which will display the number of currently built databases, the total number of triples and expiration time, as well as information related to the platform (including news and version information, etc.). Click "relevant information" to view the details. In addition, some common links such as the gStore official website, its GitHub and Gitee repository, and community forums can be accessed by clicking on the upper toolbar.

![image-20221031152422372](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152422372.png)

### Personal center

The personal center is in the upper right corner of the gStore page.

![image-20221031152444754](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152444754.png)

After logging in to the personal center, you can view the basic user information and operation logs of this week. The basic user information includes KeyID and Secret, which are used as keys when other programs connect with the gStore cloud platform.  

![image-20221031152457678](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152457678.png)

### Database management

The area on the left is the system menu, including the members area and help center.
The member area is divided into two functional modules, database management and database query.

**(1) Starting a database instance**

There is an important feature in database management: instances. When first entering the system, the instance may be stopped and need to be started manually.  

![image-20221031152506984](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152506984.png)

**(2) Viewing database Information**

After the instance is started, you can see the status of the instance, the number of databases created and the maximum number of databases that can be created, the expiration time of gStore, the total number of triples, and so on in the above row.

Below, you can see the databases that have been created, including a system database (created by the system, not operable, not included in the maximum number of databases that can be created) and several custom databases (created by yourself, operable).

![image-20221031152515776](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152515776.png)

You can create, delete, export, and obtain information about a custom database.
Click on a database to get information about it, including the creator, the creation time, the number of triples, the number of entities, the number of characters, the number of subjects, the number of predicates, the number of connections, and so on. The picture below shows information about the Movie database:

![image-20221031152529301](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152529301.png)

**(3) Creating a database**

Click [New Database] to create a database: (**Due to limited resources, the number of databases created by each user is limited to 5, and the number of triples of each database is limited to 1 million. If necessary, you can apply for space expansion from the administrator**)  

![image-20221031152537305](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152537305.png)

There are three ways to create a database. The first is a local file, that is, upload a file from the local PC to the server. Currently, the system only supports NT files (N3 files may be supported in the future).

![image-20221031152546485](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152546485.png)
![image-20221031152553638](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152553638.png)

Note that the file size cannot exceed 2GB and the number of lines cannot exceed 1 million.
The second way to create a database is a remote relational database, which remotely accesses a database on the network and imports it into the cloud platform. The cloud platform supports four relational databases: MySQL, Oracle, SQLServer, and Postgre. When creating a database, you need to input its related information and generate a D2RQ script to generate a database.  

![image-20221031152601742](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152601742.png)

The third way to create a database is to use example database. Currently, the movie database is the only example database in the cloud platform, but more will be added over time. The movie database contains more than 4 million triples containing information about movies, directors, actors, release dates, movie ratings, and more. (**Sample database triples are not limited to 1 million triples per database**)  

![image-20221031152619203](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152619203.png)

**(4) Importing a database**

You can import data to an existing database by clicking the up arrow icon in the upper right corner of the database.

![image-20221031152627516](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152627516.png)
![image-20221031152634734](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152634734.png)

Select file upload and click [Import Data].

**(5) Exporting a database**

You can export a database by clicking the down arrow icon in the upper right corner of the database.

![image-20221031152646096](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152646096.png)
![image-20221031152655330](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152655330.png)

After clicking Export, a zip file will be created, downloaded and decompressed to get the NT file corresponding to the database. After that, more database functions such as database rename and database backup will also come online.

**(6) Deleting a database**  

You can delete a database by clicking on the trash can icon in the upper right left corner of the database.
The system provides a 15-day recovery period for all deleted databases to prevent accidental deletion.

![image-20221031152702790](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031152702790.png)

### Database query

In terms of database query, gStore cloud system provides a visual query interface. Enter SPARQL statements in the text box below to get results. (Note: Considering system performance, the diagram and JSON data only show 100 result entries; you can click the [Download] button next to the JSON to get all the returned data)





## End

This is the end of the help manual for gStore Cloud platform. If you have any questions about the use of the cloud platform, you can click the "Submit questions" in the upper right corner of the cloud platform to give your opinion in the community.