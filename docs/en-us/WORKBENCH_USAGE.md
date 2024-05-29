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

## Advanced settings

Users can define custom graph analysis functions through the advanced settings module, which, after definition, can also be called directly from the graph database query module.

### Find existing custom function

Click the [Advanced Settings] - [Customized Function] module, enter the name of the custom function to be queried, select the function status, and click [Search] to find the target custom function.

![image-20221031151815975](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221031151815975.png)

### Create new custom function

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

### Execute custom function

Click [Advanced Settings] - [Custom Function] module, click [Execute], input the database, function to be executed, source and/or destination nodes, K-hop value and other information, then click [Execute] to get the result.

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

### User management (Only for root user)

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