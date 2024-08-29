## Introduction to API

gStore provides API services to users through http and Socket services, and its components are gRPC, ghttp and gServer.

### Introduction to HTTP API

We now provide C++, Java, Python, PHP, and Node.js APIs for gRPC and ghttp. Please refer to the examples in `api/http/cpp`, `api/http/java`, `api/http/python`, `api/http/php` and `api/http/nodejs`. To use these examples, make sure you have generated the executable files of gRPC or ghttp. **Next, please use the `bin/grpc` or `bin/ghttp` command to start the service.** If you know of a running gRPC or ghttp server that's available for connection, you can also directly connect to it. Then, for the C++ and Java code, you need to compile the sample code in the directory `api/http/cpp/example` or`api/http/java/example`.

<!-- <div STYLE="page-break-after: always;"></div> -->

#### HTTP API Framework

The gStore HTTP API is placed in the API/HTTP directory of the gStore root directory and contains the following: 

- gStore/api/http/
  - cpp/ (the C++ API)
    - example/ (Example program using the C++ API)
      - Benchmark.cpp
      - GET-example.cpp
      - POST-example.cpp
      - Transaction-example.cpp
      - Makefile (for compiling the sample code)
    - src/
      - GstoreConnector.cpp (C++ API's source code)
      - GstoreConnector.h
      - Makefile (for compiling the lib)
  - java/ (the Java API)
    - example/ (Example program using the Java API)
      - Benckmark.java
      - GETexample.java
      - POSTexample.java
      - Makefile
    - src/
      - jgsc/
        - GstoreConnector.java (Java API's source code)
      - Makefile
  - python/ (the Python API)
    - example/ (Example program using the Python API)
      - Benchmark.py
      - GET-example.py
      - POST-example.py
    - src/
      - GstoreConnector.py
  - nodejs/ (the Node.js API)
    - example/ (Example program using the Node.js API)
      - POST-example.js
      - GET-example.js
    - src
      - GstoreConnector.js (Node.js API's source code)
      - LICENSE
      - package.json
  - php/ (the PHP API)
    - example/ (Example program using the PHP API)
      - Benchmark.php
      - POST-example.php
      - GET-example.php
    - src/
      - GstoreConnector.php (the PHP API's source code)  

- - -

<!-- <div STYLE="page-break-after: always;"></div> -->

**For details on how to start and shut down gRPC or ghttp, please see [development documentation] - [Quick Start] - [HTTP API service].**

**After the API service is started, the gRPC access address is as follows:**

```
http://serverip:port/grpc/
```

**The ghttp access address is as follows:**

```
http://serverip:port/
```

`ServerIP` is the IP address of the gStore server, and `port` is the port on which gRPC or ghttp is started.

### Introduction to Socket API

We now provide C++ (Java, Python, PHP, and Node.js will be supported in future versions) API for gServer, the socket API service. Please see api/socket/cpp for the example code. To use these examples, make sure you have generated an executable file for gServer. **Next, use the `bin/gserver -s` command to start the gserver service.** If you know of a running gServer that's available for connection, you can also directly connect to it. Then, for the C++ code, you need to compile the sample code in the directory `api/socket/cpp/example`. 

**For details on how to start and shut down gServer, please see [development documentation] - [Quick Start] - [Socket API service].**

**After the Socket API is started, you can connect through the Socket. The default port of gServer is 9000.**

<!-- <div STYLE="page-break-after: always;"></div> -->

#### Socket API Framework

gStore的Socket API放在gStore根目录的API/Socket目录中，其内容如下:
The gStore Socket API is placed in the API/Socket directory of the gStore root directory and contains the following: 

- gStore/api/socket/

  - cpp/ (the C++ API)

    - example/ (Example program using the C++ API)
      - CppAPIExample.cpp
      - Makefile  ((for compiling the sample code)
    - src/
      - Client.cpp  (C++ API's source code)
      - Client.h
      - Makefile (for compiling the lib)



- - -

<!-- <div STYLE="page-break-after: always;"></div> -->


## http API Instruction

### ghttp Service Interconnection Mode

>The ghttp interface adopts the `HTTP` protocol and supports multiple ways to access the interface. If the ghttp is started on the port `9000`, the interface interconnection content is as follows
>API address：
> 
>```http
>http://ip:9000/
>```
> 
>The interface supports both `GET` and `POST` requests, where `GET` requests place > > parameters in the URL and `POST` requests place parameters in the `body` request.
> 
>**Note: `GET` request parameters contain special characters, such as? , @,& and other characters, you need to use urlencode encoding, especially the `SPARQL` parameter must be encoded**

### grpc Service Interconnection Mode

>The grpc interface adopts the HTTP protocol and supports multiple ways to access the interface. If the grpc is started on the port 9000, the interface connection information is as follows.
> 
>API address:
> 
>```http
>http://ip:9000/grpc
>```
> 
>The interface supports both `GET` and `POST` requests, where `GET` requests place parameters in the URL and `POST` requests place parameters in the `body` request or use `form` to express a request.
> 
>Post request method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)
> 
>Post request method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)
>
>**Note: If the `GET` request parameters contain special characters, such as? , @, and &, you need to use the urlencode encoding, especially for the `SPARQL` parameter.**

 

### API List
>The red font represents the latest version additions, while the blue font represents the latest version modifications

| API name                                              | Definition                      | Note                                                         |
| ------------------------------------------------------ | ------------------------- | ------------------------------------------------------------ |
| Interface of system                                           |                           |                                                              |
| &emsp;check                                            | heartbeat signal                  | Detect ghttp heartbeat signal                                            |
| &emsp;login                                            | login to database | It is used to authenticate user names and passwords |
| &emsp;testConnect                                      | testing connectivity | Used to check whether GHTTP is connected |
| &emsp;getCoreVersion                                   | get gStore version | Get the gStore version number                |
| &emsp;ipmanage                                         | black/white list management | Maintains a blacklist and whitelist of IP addresses that access gStore |
| &emsp;upload                                           | upload files                        | support file types to upload are nt, ttl, n3, rdf, txt. |
| &emsp;download                                         | download files    | support to download files in gstore main directory and its sub directories. |
| &emsp;stat                                             | query system resources | statistics CPU, memory, disk available space information.    |
| &emsp;shutdown                                         | close ghttp  |                                                              |
| Interface of system operations     |                           |                                                              |
| &emsp;show                                             | display graph database | Display a list of all databases            |
| &emsp;load                                             | load graph database | Load the database into memory            |
| &emsp;unload                                           | unload graph database | Unload the database from memory          |
| &emsp;monitor                                          | monitor graph database | Count information about the specified database (such as the number of triples, etc.) |
| &emsp;<font color=blue>build(update)</font>        | build graph database | The database file must be locally stored on the server |
| &emsp;drop                                             | drop graph database | Logical deletion and physical deletion can be performed |
| &emsp;<font color=blue>backup(update)</font>       | backup database | backup database information                    |
| &emsp;backuppath                                       | get the backup database path | Returns a list of all backup files in the default backup path. |
| &emsp;<font color=blue>restore(update)</font>      | restore database | restore database information                   |
| &emsp;query                                            | query database                      | Including query, delete, and insert      |
| &emsp;export                                           | export database | Export database as NT file                 |
| &emsp;<font color=blue>batchInsert(update)</font> | batch insert data | Batch insert NT data                           |
| &emsp;<font color=blue>batchRemove(update)</font>  | batch delete data | Batch delete NT data                           |
| &emsp;rename                                           | rename graph database | modify graph database name.                  |
| &emsp;<font color=red>checkOperationState(new)</font> | Query asynchronous operation status | Query the asynchronous execution status of build, backup, restore, batch_insert, batch_remove |
| Interface of database transaction |                                     |                                                              |
| &emsp;begin                                            | Start transaction | Transaction starts and needs to be used in conjunction with TQuery |
| &emsp;tquery                                           | Querying the database (with transactions) | Data queries with transaction mode (insert and DELETE only) |
| &emsp;commit                                           | commit transactions | Commit the transaction after it completes  |
| &emsp;rollback                                         | rollback transaction                      | Roll back the transaction to begin state  |
| &emsp;checkpoint                                       | write data to a disk | After an INSERT or delete operation is performed on the database, manually checkpoint is required |
| Interface of user management             |                           |                                                              |
| &emsp;showuser                                         | display all user list | Display a list of all users              |
| &emsp;usermanage                                       | user management   | Add, delete, or modify user information |
| &emsp;userprivilegemanage                              | user privilege management | Add, delete, or modify user's privilege information |
| &emsp;userpassword                                     | modify user's password | modify user's password                           |
|Interface of custom function |    ||
| &emsp;funquery                                         | Query operator function | Gets a list of user-defined graph analysis functions |
| &emsp;funcudb                                          | Manage operator function | Adds, modifies, deletes, or compiles operator function |
| &emsp;funreview                                        | Preview operator function | View the latest generated graph analysis function source code |
| Interface of log                           |                           |                                                              |
| &emsp;txnlog                                           | Obtain transaction log information | Returns transcation log information as JSON |
| &emsp;querylogdate                                     | Gets the date list of query logs | Returns the date list of existing query logs |
| &emsp;querylog                                         | Get query log information | Returns query log information as JSON |
| &emsp;accesslogdate                                    | Get the date of the API log | Returns the date list of existing API logs |
| &emsp;accesslog                                        | Get API access logs | Returns API access log information as JSON |
| <font color=red>Interface of reason engine(new)</font> |                           |                                                              |
| &emsp;addReason                                     | add reason    |                                                              |
| &emsp;listReason                                       | list reason  |                                                              |
| &emsp;compileReason                                    | compile reason   |                                                              |
| &emsp;executeReason                                    | execute reason |                                                              |
| &emsp;disableReason                                    | disable reason |                                                              |
| &emsp;showReason                                       | show reason    |                                                              |
| &emsp;removeReason                                     | remove reason |                                                              |

<!-- <div STYLE="page-break-after: always;"></div> -->

### API Specific Instruction

> The input and output parameters of each interface are specified in this section. Assume that the IP address of the GHTTP server is 127.0.0.1 and the port is 9000
Interfaces of system

### Interfaces of System

#### check

**Brief description**

- Detect ghttp heartbeat signal

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON `structure

**Parameter**

| parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **check**                     |

**Return value**

| Parameter name | Tpye   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "the ghttp server is running..."
}
```

#### login

**Brief description**

- User login (verify username and password) 
- Updates in this version: the full path information of RootPath will be returned after successful login

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **login**                       |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

**Return value**

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| CoreVersion    | string | Kernel version                                               |
| licensetype    | string | Certificate type (Open Source or Enterprise)                 |
| RootPath       | string | Full path to the gStore root directory                       |
| type           | string | HTTP service type                                            |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "login successfully",
	"CoreVersion": "1.0.0",
	"licensetype": "opensource",
	"Rootpath": "/data/gstore",
	"type": "ghttp"
}
```

<div STYLE="page-break-after: always;"></div>
#### testConnect

**Brief description**

- Test whether the server can connect (for Workbench)
- Updates in this version: added HTTP service type in the return value

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **testConnect**                 |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

**Return value**

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| CoreVersion    | string | Kernel version                                               |
| licensetype    | string | Certificate type (Open Source or Enterprise)                 |
| type           | string | HTTP service type (fixed as ghttp)                           |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"CoreVersion": "1.0.0",
	"licensetype": "opensource",
	"type": "ghttp"
}
```



#### getCoreVersion

**Brief description**

- Get the server version number (for Workbench)

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **getCoreVersion**            |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

**Return value**

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| CoreVersion    | string | Kernel version                                               |
| type           | string | HTTP service type (fixed as ghttp)                           |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"CoreVersion": "1.0.0",
	"type": "ghttp"
}
```

#### ipmanage

**Brief description**

- Blacklist and whitelist management

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as `JSON ` structure

**Parameter**

Querying the blacklist and whitelist:

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **ipmanage**                  |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| type           | yes       | string | Operation type, fixed value is **1**                         |

Saving the blacklist and whitelist:

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **ipmanage**                  |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| type           | yes       | string | Operation type, fixed value is **2**                         |
| ip_type        | yes       | string | List type, 1-Blacklist 2-Whitelist                           |
| ips            | yes       | string | IP List (Multiple separated by **,** supporting range configuration, using  **-**  connections such as ip1-ip2) |

```json
//Post example
{
    "operation": "ipmanage",
    "username": "root",
    "password": "123456",
    "type": "2",
    "ip_type":"1",
    "ips": "192.168.1.111,192.168.1.120-192.168.1.129"
}
```

**Return value**

| Parameter           | Type   | Note                                                         |
| :------------------ | :----- | ------------------------------------------------------------ |
| StatusCode          | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg           | string | Return specific information                                  |
| ResponseBody        | Object | Return data (only for queries)                               |
| &emsp;&emsp;ip_type | string | List type, 1-Blacklist 2-Whitelist                           |
| &emsp;&emsp;ips     | array  | IP List                                                      |

**Return sample**

```json
//result of querying the black list and white list
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
// result of saving the black list and white list
{
    "StatusCode": 0,
    "StatusMsg": "success"
}
```

#### upload

**Brief Description**

- Uploading files. Currently supported file formats include nt, ttl, and txt

**Request URL**

- ghttp service:` http://127.0.0.1:9000/file/upload ` <font color="red">(note that the address has changed)</font>
- grpc service: ` http://127.0.0.1:9000/grpc/file/upload `<font color="red">(note that the address has changed)</font>

**Request method**

- POST

**Parameter Transfer Methods**

- POST request, `form-data` in the `body` of `httprequest`(requires the RequestHeader parameter Content-Type: multipart/form-data

**Parameters**

| Parameter name | Required | Type    | Description                                                  |
| :------------- | :------- | :------ | ------------------------------------------------------------ |
| username       | Yes      | string  | Username                                                     |
| password       | Yes      | string  | Password (clear text)                                        |
| encryption     | No       | string  | If it is empty, the password is clear text. If it is 1, it means md5 encryption is used |
| file           | Yes      | boudary | Binary file stream of the file to be uploaded                |

  **Return Value**

| Parameter name | Type   | Description                                                  |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (please refer to the attached table: Return Value Code Table) |
| StatusMsg      | string | Returns specific information                                 |
| filepath       | string | The relative path address returned after successful upload   |

  **Return Sample：**

```json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "filepath": "./upload/test_20221101164622.nt"
}
```

#### download

**Brief Description**

- Download files, currently supports downloading files from the gStore root directory.

**Request URL**

- ghttp service:` http://127.0.0.1:9000/file/download ` <font color=red>(Note that the address has changed)</font>
- grpc service: ` http://127.0.0.1:9000/grpc/file/upload `<font color="red">(Note that the address has changed)</font>

**Request method**

- POST

**Parameter Transfer Method**

- POST request: the parameters are passed in the form of Form data (requires the RequestHeader parameter Content-Type: application/x-www-form-urlencoded)

**Parameters**

| Parameter name | Required | Type   | Description                                                  |
| :------------- | :------- | :----- | ------------------------------------------------------------ |
| username       | Yes      | string | Username (the default username is system)                    |
| password       | Yes      | string | Password (clear text)                                        |
| encryption     | No       | string | If it is empty, the password is clear text. If it is 1, it means md5 encryption is used |
| filepath       | Yes      | string | The path of the file to be downloaded (only supports downloading files from the main directory and subdirectories of gstore) |

**Return Value**

- Binary stream response

**Return Sample：**

```shell
Content-Range: bytes 0-389/389
Content-Type: application/octet-stream
Date: Tue, 01 Nov 2022 17:21:40 GMT
Content-Length: 389
Connection: Keep-Alive
```

#### state

**Brief Description**

- Statistics on system resource information

**Request URL**

- ghttp service:` http://127.0.0.1:9000/ghttp/api`
- grpc service:` http://127.0.0.1:9000/grpc/api `

**Request method**

- GET/POST

**Parameter Transfer Method**

- GET request, the parameters are passed directly as the URL
- POST request: the parameters are passed in the form of Form data (requires the RequestHeader parameter Content-Type: application/x-www-form-urlencoded)

**Parameters**

| Parameter name | Required | Type   | Description                                                  |
| :------------- | :------- | :----- | ------------------------------------------------------------ |
| operation      | yes      | string | Operation name, fixed value is **stat**                      |
| username       | Yes      | string | Username (the default username is system)                    |
| password       | Yes      | string | Password (plain text)                                        |
| encryption     | No       | string | If it is empty, the password is clear text. If it is 1, it means md5 encryption is used |

**Return Value**

| Parameter name | Type   | Description                                                  |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (please refer to the attached table: Return Value Code Table) |
| StatusMsg      | string | Returns specific information                                 |
| cup-usage      | string | CPU usage ratio                                              |
| mem_usage      | string | Memory usage (in MB)                                         |
| disk_available | string | Available disk space (in MB)                                 |

**Return Sample：**

```shell
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "cpu_usage": "10.596026",
    "mem_usage": "2681.507812",
    "disk_available": "12270"
}
```

#### shutdown

**Brief description**

- close ghttp

**Request URL**

- ghttp service: ` http://127.0.0.1:9000/shutdown `<font color="red">(Note that the address has changed)</font>
- grpc service: ` http://127.0.0.1:9000/grpc/shutdown `<font color="red">(Note that the address has changed)</font>
**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| username       | yes       | string | user name（default user name is system)                      |
| password       | yes       | string | Password（This password need to be viewed in the server's system.db/password[port].txt file) |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Server stopped successfully."
}
```

### Interfaces of System Operation

#### show

**Brief description**

- Display all database list

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **show**                      |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | No        | string | If it is empty, the password is clear text. If it is 1, it means it is encrypted with md5. |

**Return value**

| Parameter name         | Type      | Note                                                         |
| :--------------------- | :-------- | ------------------------------------------------------------ |
| StatusCode             | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg              | string    | Return specific information                                  |
| ResponseBody           | JSONArray | JSON arrays (each of which is a database information)        |
| &emsp;&emsp;database   | string    | database name                                                |
| &emsp;&emsp;creator    | string    | creator                                                      |
| &emsp;&emsp;built_time | string    | create time                                                  |
| &emsp;&emsp;status     | string    | database status                                              |

**Return sample** 

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

#### load

**Brief description**  

- Loading a database into memory is a prerequisite for many operations, such as query and monitor.

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request: the parameters are passed directly as the URL
- POST request: `raw` in `body` in `httprequest` , passed as `JSON`  

**Parameter**  

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **load**                      |
| username       | yes       | string | User name                                                    |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | Database name (.db is not required)                          |
| csr            | no        | string | Whether to load CSR resources, default is 0 (set to 1 when using advanced query functions) |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| csr            | string | Whether to load CSR resources                                |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Database loaded successfully.",
	"csr": "1"
}
```

#### unload

**Brief description**

- Unload the database from memory (all changes are flushed back to hard disk)

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **unload**                    |
| db_name        | yes       | string | Database name (.db is not required)                          |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database unloaded."
}
```

#### monitor

**Brief description**

- Get database statistics (database needs to be loaded)


**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON `

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **monitor**                   |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |

**Return value** 

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| database       | string | database name                                                |
| creator        | string | creator                                                      |
| builtTime      | string | create time                                                  |
| tripleNum      | string | number of triples                                            |
| entityNum      | int    | number of entities                                           |
| literalNum     | int    | number of characters (attribute value)                       |
| subjectNum     | int    | number of subjects                                           |
| predicateNum   | int    | number of objects                                            |
| connectionNum  | int    | number of connections                                        |
| diskUsed       | int    | disk space（MB）                                             |
| subjectList    | Array  | Entity type statistics                                       |

**Return sample** 

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

#### build(update in this version)

**Brief description**

- Create a database based on existing NT files, or build an empty library (supported in version 1.2)
- The file must exist on the gStore server
- You can first upload data files to the gStore server through the upload interface

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name                    | Mandatory | Type   | Note                                                         |
| --------------------------------- | --------- | ------ | ------------------------------------------------------------ |
| operation                         | yes       | string | Operation name, fixed value is **build**                     |
| username                          | yes       | string | User name                                                    |
| password                          | yes       | string | Password (plain text)                                        |
| encryption                        | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name                           | yes       | string | database name(no need .db)                                   |
| db_path                           | no        | string | Database file path, supported file types are (can be absolute path or relative path, relative path is referenced from gStore installation root directory) |
| <font color =red>async</font>     | no        | string | If **async is "true"**, immediately return the query opd_id, the server's asynchronous processing logic, and the client can call the checkOperationState interface through opd_id to view the operation status information |
| <font color = red>callback</font> | no        | string | Callback interface, for example: http://127.0.0.1:8080/callback |

**Return value**

| Parameter name                 | Type   | Note                                                         |
| ------------------------------ | ------ | ------------------------------------------------------------ |
| StatusCode                     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg                      | string | Return specific information                                  |
| failed_num                     | int    | Number of failed constructions                               |
| <font color =red>opt_id</font> | string | Return the operation opd_id, the client can call the checkOperationState interface through opd_id to view the operation status information |

**Return sample**(defult)

```json
{
    "StatusCode": 0,
    "StatusMsg": "Import RDF file to database done.",
    "failed_num": 0,
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**Return sample**(async="true")

```json
{
    "StatusCode": 0,
    "StatusMsg": "Operation Success.",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**Return sample**(callback="http://127.0.0.1:8080/callback")

```json
#push data to callback address
{
    "operation": "build",
    "StatusCode": 0,
    "StatusMsg": "Import RDF file to database done.",
    "failed_num": 0,
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

#### drop

**Brief description**

- Delete the database (either logically or physically)

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **drop**                      |
| db_name        | yes       | string | Database name (.db is not required)                          |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| is_backup      | no        | string | True: Logical deletion, false: represents physical deletion (default true)，if it's logical deletion, change the file folder to .bak file foder, user can change the folder name to. db and add the folder to the system database by calling `bin/ ginit-db database name` |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database test_lubm dropped."
}
```

#### backup(update in this version)

**Brief description**

- Back up database

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name                   | Mandatory | Type   | Note                                                         |
| :------------------------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation                        | yes       | string | Operation name, fixed value is **backup**                    |
| username                         | yes       | string | user name                                                    |
| password                         | yes       | string | Password (plain text)                                        |
| encryption                       | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name                          | yes       | string | database that need operaions                                 |
| backup_path                      | no        | string | The backup file path can be relative or absolute. The relative path uses the gStore root directory as reference. The default path is the backup directory in the gStore root directory |
| <font color =red>async</font>    | no        | string | If **async is "true"**, immediately return the query opd_id, the server's asynchronous processing logic, and the client can call the checkOperationState interface through opd_id to view the operation status information |
| <font color =red>callback</font> | no        | string | Callback interface, for example: http://127.0.0.1:8080/callback |

**Return value**

| Parameter name                  | Type   | Note                                                         |
| :------------------------------ | :----- | ------------------------------------------------------------ |
| StatusCode                      | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg                       | string | Return specific information                                  |
| backupfilepath                  | string | Backup file path (this value can be used as the input parameter value for restore) |
| <font color = red>opt_id</font> | string | Return the operation opd_id, the client can call the checkOperationState interface through opd_id to view the operation status information |

**Return sample** (defult)

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database backup successfully.",
    "backupfilepath": "./backups/lubm.db_210828211529",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**Return sample** (async="true")

```json
{
    "StatusCode": 0,
    "StatusMsg": "Operation Success.",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**Return sample** (callback="http://127.0.0.1:8080/callback")

```json
{
    "operation": "backup",
    "StatusCode": 0,
    "StatusMsg": "Database backup successfully.",
    "backupfilepath": "./backups/lubm.db_210828211529",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

#### backuppath

**Brief description**

- Obtain all backup files of the database located under the default backup path.

**Request Methods**

- GET/POST

**Parameter Passing Methods**

- For GET requests, parameters are passed directly in the URL.
- For POST requests, the `raw` field in the `body` of the `httprequest` is passed in JSON structure.

**Parameters**

| Parameter Name | Required | Type   | Description                                                  |
| -------------- | -------- | ------ | ------------------------------------------------------------ |
| operation      | Yes      | string | Operation name, fixed value is **backuppath**                |
| username       | Yes      | string | Username                                                     |
| password       | Yes      | string | Password (plaintext)                                         |
| encryption     | No       | string | If it is empty, the password is plaintext. If it is 1, it means encryption with MD5 |
| db_name        | Yes      | string | Database name that needs to be queried                       |

**Return Values**

| Parameter Name | Type    |                                                  Description |
| -------------- | ------- | -----------------------------------------------------------: |
| StatusCode     | integer | Return value code (please refer to the attached table for specific values). |
| StatusMsg      | string  |                                 Specific return information. |
| paths          | Array   | Backup file paths (this value can be used as input for restore operation). |

**Return sample** 

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

#### restore（update in this version）

**Brief description**

- Restore database

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **restore**                   |
| username       | yes       | string | User name                                                   |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If it is empty, the password is plaintext. If it is 1, it means encryption with MD5. |
| db_name        | yes       | string | Database that need operations                               |
| backup_path    | no        | string | The full time-stamped path of the backup file (can be a relative path or an absolute path. The relative path is based on the gStore root directory). The default path is the backup directory in the gStore root directory |
| <font color =red>async</font>     | no        | string | If **async is "true"**, immediately return the query opd_id, the server's asynchronous processing logic, and the client can call the checkOperationState interface through opd_id to view the operation status information |
| <font color = red>callback</font> | no        | string | Callback interface, for example: http://127.0.0.1:8080/callback |

**Return value**

| Parameter name                 | Type   | Note                                                         |
| :----------------------------- | :----- | ------------------------------------------------------------ |
| StatusCode                     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg                      | string | Return specific information                                  |
| <font color =red>opt_id</font> | string | Return the operation opd_id, the client can call the checkOperationState interface through opd_id to view the operation status information |

**Return sample** (defult)

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database lumb restore successfully.",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**Return sample** (async="true")

```json
{
    "StatusCode": 0,
    "StatusMsg": "Operation Success.",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**Return sample** (callback=“http://127.0.0.1:8080/callback")

```json
{
    "StatusCode": 0,
    "StatusMsg": "Database lumb restore successfully.",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

#### query

**Brief description**

- query the database


**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **query**                     |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If it is empty, the password is plaintext. If it is 1, it means encryption with MD5. |
| db_name        | yes       | string | database that need operations                                |
| format         | no        | string | The result set returns in json, HTML, and file format. The default is JSOn |
| sparql         | yes       | string | Sparql statement to execute (SPARQL requires URL encoding if it is a GET request) |

**Return value**

| Parameter name       | Type   | Note                                                         |
| :------------------- | :----- | ------------------------------------------------------------ |
| StatusCode           | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg            | string | Return specific information                                  |
| head                 | object | Head information                                             |
| &emsp;&emsp;link     | array  |                                                              |
| &emsp;&emsp;vars     | array  |                                                              |
| results              | object | Result information (please refer to the return example for details) |
| &emsp;&emsp;bindings | array  |                                                              |
| AnsNum               | int    | Number of results                                            |
| OutputLimit          | int    | Maximum number of return results (-1 for no limit)           |
| ThreadId             | string | Query thread ID                                              |
| QueryTime            | string | Query time (milliseconds)                                    |


**Return sample** 

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
                    "value": "House of Flying Daggers"
                }
            },
            {
                "x": {
                    "type": "uri",
                    "value": "Blood Brothers"
                }
            },
            {
                "x": {
                    "type": "uri",
                    "value": "flower"
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

#### export

**Brief description**

- export database

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **restore**                   |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If it is empty, the password is plaintext. If it is 1, it means encryption with MD5. |
| db_name        | yes       | string | database that need operations                                |
| db_path        | no        | string | Export path (gstore root by default)                         |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| filepath       | string | Path for exporting files                                     |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Export the database successfully.",
    "filepath": "export/lubm_210828214603.nt"
}
```

#### batchInsert(update in this version)

**Brief description**

- batch insert data

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **batchInsert**               |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If it is empty, the password is plaintext. If it is 1, it means encryption with MD5. |
| db_name        | yes       | string | Database name                                                |
| file           | no        | string | The data to be inserted in the nt file or zip package (which can be a relative path or an absolute path) cannot be empty at the same time as the dir parameter. When both have values, take the file parameter |
| dir            | no        | string | The data to be inserted into the nt folder (which can be a relative path or an absolute path) cannot be empty at the same time as the file parameter. When both have values, take the file parameter |
| <font color =red>async</font>     | no        | string | If **async is "true"**, immediately return the query opd_id, the server's asynchronous processing logic, and the client can call the checkOperationState interface through opd_id to view the operation status information |
| <font color = red>callback</font> | no        | string | Callback interface, for example: http://127.0.0.1:8080/callback |


**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| success_num    | string | Number of successful executions                              |
| failed_num     | srting | Number of failed executions                                  |
| <font color =red>opt_id</font> | string | Return the operation opd_id, the client can call the checkOperationState interface through opd_id to view the operation status information |

**Return Sample** （defult）

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Batch insert data successfully.",
    "success_num": 25,
    "failed_num": 0,
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**Return Sample** (async=“true”)

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Operation Success.",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**Return Sample** （callback=“http://127.0.0.1:8080/callback”）

``` json
#push data to callback address
{
    "operation": "batchInsert",
    "StatusCode": 0,
    "StatusMsg": "Batch insert data successfully.",
    "success_num": 25,
    "failed_num": 0,
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```
#### batchRemove(update in this version)

**Brief description**

- batch remove data

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **batchRemove**       |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If it is empty, the password is plaintext. If it is 1, it means encryption with MD5. |
| db_name        | yes       | string | Database name                                                |
| file           | no        | string | Data NT files to be deleted (can be relative or absolute paths) |
| <font color =red>async</font>     | no        | string | If **async is "true"**, immediately return the query opd_id, the server's asynchronous processing logic, and the client can call the checkOperationState interface through opd_id to view the operation status information |
| <font color = red>callback</font> | no        | string | Callback interface, for example: http://127.0.0.1:8080/callback |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| success_num    | string | Number of successful executions                              |
| <font color =red>opt_id</font> | string | Return the operation opd_id, the client can call the checkOperationState interface through opd_id to view the operation status information |

**Return Sample** （defult）

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Batch remove data successfully.",
    "success_num": 25,
    "failed_num": 0,
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**Return Sample** (async=“true”)

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Operation Success.",
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

**Return Sample** （callback=“http://127.0.0.1:8080/callback”）

``` json
#push data to callback address
{
    "operation": "batchRemove",
    "StatusCode": 0,
    "StatusMsg": "Batch remove data successfully.",
    "success_num": 25,
    "opt_id": "XXXX-XXXX-XXXX-XXXX"
}
```

#### rename

**Brief Description**

- Rename the database.

**Request method**

- GET/POST

**Parameter Transfer Method**

- For GET requests, parameters are passed directly in the URL.
- For POST requests, the `raw` field in the `body` of the `httprequest` is used to pass parameters in JSON structure.

**Parameters**

| Parameter name | Required | Type   | Description                                                  |
| :------------- | :------- | :----- | ------------------------------------------------------------ |
| operation      | Yes      | string | Operation name, the fixed value is **rename**                |
| username       | Yes      | string | Username                                                     |
| password       | Yes      | string | Password (clear text)                                        |
| encryption     | No       | string | If it is empty, the password is clear text. If it is 1, it means that md5 encryption is used. |
| db_name        | Yes      | string | Database name                                                |
| new_name       | Yes      | string | New name of the database                                     |

**Return value**

| Parameter name | Type   | Description                                                  |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (please refer to the attached table: Return Value Code Table) |
| StatusMsg      | string | Returns specific information                                 |

**Return Sample：**

```json
{
    "StatusCode": 0,
    "StatusMsg": "success"
}
```

#### checkOperationState

**Brief description**

- Query asynchronous operation status (build, backup, restore, batchInsert, batchRemove）

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **checkOperationState** |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If it is empty, the password is plaintext. If it is 1, it means encryption with MD5. |
| <font color = red>opt_id</font> | yes    | string | Operation number |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| state | int | execution state(0 executing , -1 failed execution, 1 successfu execution) |
| success_num    | int | Number of successful executions                              |
| failed_num | int | Number of failed executions |

**Return Sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Batch insert data successfully.",
    "state": 1,
    "success_num": 100,
    "failed_num": 0
}
```

### Interface of Database Transaction

#### begin

**Brief description**

- start transaction

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **begin**                     |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If it is empty, the password is plaintext. If it is 1, it means encryption with MD5. |
| db_name        | yes       | string | Database name                                                |
| isolevel       | yes       | string | Transaction isolation level 1:RC(read committed)  2:SI(snapshot isolation) 3:SR(seriablizable） |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| TID            | string | Transaction ID(this ID is important enough to take as a parameter |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "transaction begin success",
    "TID": "1"
}
```

#### tquery

**Brief description**

- query the transaction type

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **tquery**                    |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If it is empty, the password is plaintext. If it is 1, it means encryption with MD5. |
| db_name        | yes       | string | Database name                                                |
| tid            | yes       | string | Transaction ID                                               |
| sparql         | yes       | string | sparql statement                                             |

**Return value**

| Parameter Name |  Type  |                         Description                          |
| :------------: | :----: | :----------------------------------------------------------: |
|   StatusCode   |  int   | Return value code (please refer to the attached table for specific values) |
|   StatusMsg    | string |                 Specific return information                  |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success"
}
```

#### commit

**Brief description**

- submit transaction

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **commit**                    |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If it is empty, the password is plaintext. If it is 1, it means encryption with MD5. |
| db_name        | yes       | string | Database name                                                |
| tid            | yes       | string | Transaction ID                                               |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "transaction commit success. TID: 1"
}
```

#### rollback

**Brief description**

- Rollback trasnsaction

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **rollback**                  |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If it is empty, the password is plaintext. If it is 1, it means encryption with MD5. |
| db_name        | yes       | string | Database name                                                |
| tid            | yes       | string | Transaction ID                                               |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "transaction rollback success. TID: 2"
}
```

#### checkpoint

**Brief description**

- Received Flush data back to hard disk (to make data final)

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                          |
| :------------- | :-------- | :----- | --------------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is **checkpoint** |
| username       | yes       | string | User name                                     |
| password       | yes       | string | Password (plain text)                         |
| encryption     | no        | string | If it is empty, the password is plaintext. If it is 1, it means encryption with MD5. |
| db_name        | yes       | string | Database name                                 |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database saved successfully."
}
```

### Interfaces of User Management

#### showuser

**Brief description**

- Display all user information

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **showuser**                  |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | No        | string | If it is empty, the password is clear text. If it is 1, it means it is encrypted with md5. |

**Return value**

| Parameter name                | Type      | Note                                                         |
| :---------------------------- | :-------- | ------------------------------------------------------------ |
| StatusCode                    | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg                     | string    | Return specific information                                  |
| ResponseBody                  | JsonArray | JSON object array                                            |
| &emsp;&emsp;username          | string    | user name                                                    |
| &emsp;&emsp;password          | string    | password                                                     |
| &emsp;&emsp;query_privilege   | string    | Query permissions (database names separated by commas)       |
| &emsp;&emsp;update_privilege  | string    | Update permissions (database names separated by commas)      |
| &emsp;&emsp;load_privilege    | string    | Load permissions (database names separated by commas)        |
| &emsp;&emsp;unload_privilege  | string    | Unload permissions (database names separated by commas)      |
| &emsp;&emsp;backup_privilege  | string    | Back up permissions (database names separated by commas)     |
| &emsp;&emsp;restore_privilege | string    | Restore permissions (database names separated by commas)     |
| &emsp;&emsp;export_privilege  | string    | Export permissions (database names separated by commas)      |

**Return sample** 

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

#### usermanage

**Brief description**

- Manage users (including adding, deleting, and changing users)

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **usermanage**                |
| type           | yes       | string | Operation Type（1：adduser ，2：deleteUser 3：alterUserPassword） |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | No        | string | If it is empty, the password is clear text. If it is 1, it means it is encrypted with md5. |
| op_username    | yes       | string | User name for the operation                                  |
| op_password    | yes       | string | Password of the operation (if the password is to be changed, the password is the password to be changed) (If the operation contains special characters and the get request is adopted, the value must be urlencode-encoded) |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 1004,
    "StatusMsg": "username already existed, add user failed."
}
```
**Notes**

- The default interface permissions for the new user are: `login`, `check`, `testConnect`, `getCoreVersion`, `show`, `funquery`, `funcudb`, `funreview`, `userpassword`.

- Users with `query` rights also have the following interface rights: `query`, `monitor`.

- Users with the `update` permission also have the following interface permissions: `batchInsert`, `batchRemove`, `begin`, `tquery`, `commit`, `rollback`.

- Only the root user can invoke the interface rights that are not within the scope of authorization management, for example: `build`, `drop`, `usermanage`, `showuser`, `userprivilege`, `manage`, `txnlog`, `checkpoint`, `shutdown`, `querylog`, `accesslog`, `ipmanage`.

#### userprivilegemanage

**Brief description**

- Manage user permissions (including adding, deleting, and changing users)

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **userprivilegemanage**       |
| type           | yes       | string | Operation type（1：add privilege，2：delete privilege 3：clear Privilege ） |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | No        | string | If it is empty, the password is clear text. If it is 1, it means it is encrypted with md5. |
| op_username    | yes       | string | User name for the operation                                  |
| privileges     | no        | string | Permissions to operate on (multiple permissions separated by commas) (can be null for clear Privilege)1:query,2:load,3:unload,4:update,5:backup,6:restore,7:export, you can set multi privileges by using , to split. |
| db_name        | no        | string | The database to operate on (this can be empty if it is clearPrivilege |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "add privilege query successfully. \r\nadd privilege load successfully. \r\nadd privilege unload successfully. \r\nadd privilege update successfully. \r\nadd privilege backup successfully. \r\n"
}
```

#### userpassword

**Brief description**

- Modify the user password.

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`  

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **userpassword**              |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password                                                     |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| op_password    | yes       | string | New password (plain text)                                    |

**Return value**

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample**

```json
{
	"StatusCode":0,
	"StatusMsg": "change password done."
}
```

### Interface of Custom Function

#### funquery

**Brief description**

- Show user-defined graph analysis functions

**Request mode**

- POST

**Parameter transfer mode**

- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name        | Mandatory | Type       | Note                                                         |
| --------------------- | --------- | ---------- | ------------------------------------------------------------ |
| operation             | yes       | string     | Operation name, fixed value is **funquery**                  |
| username              | yes       | string     | User name                                                    |
| password              | yes       | string     | Password (plain text)                                        |
| encryption            | no        | string     | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| funInfo               | no        | JSONObject | Query parameters                                             |
| &emsp;&emsp;funName   | no        | string     | Function name                                                |
| &emsp;&emsp;funStatus | no        | string     | Status (1- to compile; 2- compiled; 3- exception)            |

**Return value**

| Parameter name        | Type      | Note                                                         |
| --------------------- | --------- | ------------------------------------------------------------ |
| StatusCode            | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg             | string    | Return specific information                                  |
| list                  | JSONArray | JSON array (if there is no data, no array is returned)       |
| &emsp;&emsp;funName   | string    | Name                                                         |
| &emsp;&emsp; funDesc  | string    | Description                                                  |
| &emsp;&emsp; funArgs  | string    | Parameter type (1- no K hop parameter; 2- with K hop parameter) |
| &emsp;&emsp;funBody   | string    | Function content                                             |
| &emsp;&emsp; funSubs  | string    | Child functions (can be called in funBody)                   |
| &emsp;&emsp;funStatus | string    | Status (1- to compile; 2- compiled; 3- exception)            |
| &emsp;&emsp;lastTime  | string    | Last edit time (yyyy-MM-dd HH:mm:ss)                         |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"list": [
		{
			"funName": "demo",
			"funDesc": "this is demo",
			"funArgs": "2",
			"funBody": "{\nstd::cout<<\"uid=\<<uid<<endl;\nstd::cout<<\"vid=\<<vid<<endl;\nstd::cout<<\"k=\"<<k<<endl;\nreturn \"success\";\n}",
			"funSubs": "",
			"funStatus": "1",
			"lastTime": "2022-03-15 11:32:25"
		}
	]
}
```

#### funcudb

**Brief description**

- User-defined graph analysis function management (add, modify, delete, compile)

**Request mode**

- POST

**Parameter transfer mode**

- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name         | Mandatory | Type       | Note                                                         |
| ---------------------- | --------- | ---------- | ------------------------------------------------------------ |
| operation              | yes       | string     | Operation name, fixed value is **funcudb**                   |
| username               | yes       | string     | User name                                                    |
| password               | yes       | string     | Password (plain text)                                        |
| encryption             | no        | string     | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| type                   | yes       | string     | 1: add, 2: modify, 3: delete, 4: compile                     |
| funInfo                | yes       | JSONObject | Operator function                                            |
| &emsp;&emsp;funName    | yes       | string     | Function name                                                |
| &emsp;&emsp;funDesc    | no        | string     | Description                                                  |
| &emsp;&emsp;funArgs    | no        | string     | Parameter type (1- no K hop parameter; 2- with K hop parameter). |
| &emsp;&emsp;funBody    | no        | string     | Function contents (wrapped in {}).                           |
| &emsp;&emsp;funSubs    | no        | string     | Child functions (can be called in funBody)                   |
| &emsp;&emsp; funReturn | no        | string     | Return type(decode is required)                              |

**Return value**

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "success"
}
```

#### funreview

**Brief description**

- Preview the user-defined graph analysis function

**Request mode**

- POST

**Parameter transfer mode**

- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name        | Mandatory | Type       | Note                                                         |
| --------------------- | --------- | ---------- | ------------------------------------------------------------ |
| operation             | yes       | string     | Operation name, fixed value is **funreview**                 |
| username              | yes       | string     | User name                                                    |
| password              | yes       | string     | Password (plain text)                                        |
| encryption            | no        | string     | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| funInfo               | yes       | JSONObject | Operator function                                            |
| &emsp;&emsp;funName   | yes       | string     | Function name                                                |
| &emsp;&emsp;funDesc   | no        | string     | Description                                                  |
| &emsp;&emsp;funArgs   | yes       | string     | Parameter type (1- no K hop parameter; 2- with K hop parameter) |
| &emsp;&emsp;funBody   | yes       | string     | Function contents (contents wrapped in {})                   |
| &emsp;&emsp;funSubs   | yes       | string     | Child function (can be called in funBody)                    |
| &emsp;&emsp;funReturn | yes       | string     | Return type (`path`: the path class result; `value`: value class result) |

**Return value**

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| result         | string | Function source code (decode is required)                    |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
    "Result":"%23include+%3Ciostream%3E%0A%23include+%22..%2F..%2FDatabase%2FCSRUtil.h%22%0A%0Ausing+..."
}
```

### Interface of Log

#### txnlog

**Brief description**

- Get transaction logs (this function is only available for the root user)  
- Updates in this version: added paging query parameters

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **txnlog**                    |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| pageNo         | yes       | int    | Page number, the value ranges from 1 to N, the default value is 1 |
| pageSize       | yes       | int    | The number of entries per page, the value ranges from 1 to N, the default value is 10 |

**Return value**

| Parameter name         | Type      | Note                                                         |
| ---------------------- | --------- | ------------------------------------------------------------ |
| StatusCode             | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg              | string    | Return specific information                                  |
| totalSize              | int       | Total size                                                   |
| totalPage              | int       | The total number of pages                                    |
| pageNo                 | int       | The current page number                                      |
| pageSize               | int       | The number of entries per page                               |
| list                   | JSONArray | Log array                                                    |
| &emsp;&emsp;db_name    | string    | Database name                                                |
| &emsp;&emsp; TID       | string    | Transaction ID                                               |
| &emsp;&emsp;user       | string    | User                                                         |
| &emsp;&emsp;state      | string    | State (COMMITED/RUNNING/ROLLBACK)                            |
| &emsp;&emsp;begin_time | string    | Start time                                                   |
| &emsp;&emsp;end_time   | string    | End time                                                     |

**Return sample**

```json
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

#### querylogdate

**Brief description**

- Get the date of gStore's query log (for the date parameter of the query log interface)

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as `JSON ` 

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **querylogdate**              |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

**Return value**

| Parameter  | Type   | Note                                                         |
| :--------- | :----- | ------------------------------------------------------------ |
| StatusCode | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg  | string | Return specific information                                  |
| list       | array  | The list of dates                                            |

**Return sample**

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

#### querylog

**Brief description**

- Obtaining query Logs

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **querylog**                  |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| date           | yes       | string | Date format is yyyyMMdd                                      |
| pageNo         | yes       | int    | Page number. The value ranges from 1 to N. The default value is 1 |
| pageSize       | yes       | int    | The number of entries per page. The value ranges from 1 to N. The default value is 10 |

**Return value**

| Parameter           | Type   | Note                                                         |
| :------------------ | :----- | ------------------------------------------------------------ |
| StatusCode          | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg           | string | Return specific information                                  |
| totalSize           | int    | Total number                                                 |
| totalPage           | int    | Total page number                                            |
| pageNo              | int    | Current page number                                          |
| pageSize            | int    | The number of entries per page                               |
| list                | Array  | Log array                                                    |
| &emsp;QueryDateTime | string | Query date/time                                              |
| &emsp;Sparql        | string | SPARQL statement                                             |
| &emsp;Format        | string | Query return format                                          |
| &emsp;RemoteIP      | string | Request IP                                                   |
| &emsp;FileName      | string | Query result set files                                       |
| &emsp;QueryTime     | int    | Time (ms)                                                    |
| &emsp;AnsNum        | int    | Result number                                                |

**Return sample**

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

#### accesslogdate

**Brief description**

- Get the date of gStore's access log (for the date parameter of the access log interface)

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as `JSON `

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **querylogdate**              |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

**Return value**

| Parameter  | Type   | Note                                                         |
| :--------- | :----- | ------------------------------------------------------------ |
| StatusCode | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg  | string | Return specific information                                  |
| list       | array  | The list of dates                                            |

**Return sample**

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

#### accesslog

**Brief description**

- Get the access log of the API

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as `JSON `

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **accesslog**                 |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| date           | yes       | string | Date format is yyyyMMdd                                      |
| pageNo         | yes       | int    | Page number. The value ranges from 1 to N. The default value is 1 |
| pageSize       | yes       | int    | The number of entries per page. The value ranges from 1 to N. The default value is 10 |

**Return value**

| Parameter         | Type   | Note                                                         |
| :---------------- | :----- | ------------------------------------------------------------ |
| StatusCode        | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg         | string | Return specific information                                  |
| totalSize         | int    | Total number                                                 |
| totalPage         | int    | Total page number                                            |
| pageNo            | int    | Current page number                                          |
| pageSize          | int    | The number of entries per page                               |
| list              | Array  | Log array                                                    |
| &emsp;ip          | string | The IP that makes the access                                 |
| &emsp;operation   | string | The type of operation performed                              |
| &emsp; createtime | string | The time of the operation                                    |
| &emsp;code        | string | The result of the operation (refer to Appendix: Return value code table for details) |
| &emsp;msg         | string | Log description                                              |

**Return sample**

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

### Reason Engine

**There are 7 types of operation to parameter type**

- 1:addReason 
- 2:listReason 
- 3:compileReason 
- 4:executeReason 
- 5:disableReason 
- 6.showReason 
- 7.deleteReason

#### addReason

**Brief description**

- add reasoning rules

**Request mode**

- POST

**Parameter transfer mode**

- POST request, `raw` in `body` in `Httprequest`, passed as `JSON `

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **reasonManage**                   |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name                                                |
| type           | yes       | string | Operation type is "1"                                        |
| ruleinfo       | yes       | object | Rule information: "ruleinfo": {"rulename":"", ...}，Rule details |
| &emsp;&emsp;rulename                          | yes | string | Rule name（Same database ,unique rulename） |
| &emsp;&emsp;description                       | no | string | Rule description                       |
| &emsp;&emsp;isenable                          | yes | int    | Whether to enable, optional values: 1 enable ,0 disable |
| &emsp;&emsp;type                              | yes | int    | Reason type:optional values:1 relational reason , 0 attribute reason |
| &emsp;&emsp;logic                             | yes | int    | The relationship between conditions and conditions，optional values：1 logical and , 0 logical or |
| &emsp;&emsp;conditions                        | yes | array  | Rules set                                    |
| &emsp;&emsp;&emsp;&emsp;patterns              | yes | array  | Triples set                                |
| &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;subject   | yes | string | Subject                                        |
| &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;predicate | yes | string | Predicate                                        |
| &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;object    | yes | string | Object                                           |
| &emsp;&emsp;&emsp;&emsp;filters               | no | array  | Filter conditions                            |
| &emsp;&emsp;&emsp;&emsp;count_info            | no | Object | Aggregate function（undetermined）    |
| &emsp;&emsp;return                            | yes | Object | Return object                               |
| &emsp;&emsp;&emsp;&emsp;source                | yes | string | Source node                                |
| &emsp;&emsp;&emsp;&emsp;target                | no | string | Terminal node                |
| &emsp;&emsp;&emsp;&emsp;label                 | 否   | string | predicate（The system automatically adds the Rule: prefix) |
| &emsp;&emsp;&emsp;&emsp;value                 | 否   | string | Attribute value                |

**Return value**

| Parameter         | Type   | Note                                                         |
| :---------------- | :----- | ------------------------------------------------------------ |
| StatusCode        | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg         | string | Return specific information                                  |

**Relational reasoning rule sample**

```json
{
	"rulename": "uncle",
	"description": "The mother's male sibling was the uncle"
	"isenable": 1,
	"type": 1,
	"logic": 1,
	"conditions": [
		{
			"patterns": [
				{
					"subject": "?x ",
					"predicate": "<mother>",
					"object": "?y"
				},
				{
					"subject": "?y ",
					"predicate": "<father>",
					"object": "?z"
				},
				{
					"subject": "?k",
					"predicate": "<father>",
					"object": "?z"
				},
				{
					"subject": "?k",
					"predicate": "<gender>",
					"object": "\"male\"^^<http://www.w3.org/2001/XMLSchema#string>"
				}
			],
			"filters": [],
			"count_info": {}
		}
	],
	"return": {
		"source": "?x",
		"target": "?k",
		"label": "uncle"
	}
}
```

**Attribute reasoning rule sample**

```json
{
	"rulename": "orphan",
	"description": "Both parents died and were orphaned",
	"isenable": 1,
	"type": 0,
	"logic": 1,
	"conditions": [
		{
			"patterns": [
				{
					"subject": "?x ",
					"predicate": "<father>",
					"object": "?y"
				},
				{
					"subject": "?x",
					"predicate": "<mother>",
					"object": "?z"
				},
				{
					"subject": "?y",
					"predicate": "<state>",
					"object": "\"died\"^^<http://www.w3.org/2001/XMLSchema#string>"
				},
				{
					"subject": "?z",
					"predicate": "<state>",
					"object": "\"died\"^^<http://www.w3.org/2001/XMLSchema#string>"
				}
			],
			"filters": [],
			"count_info": {}
		}
	],
	"return": {
		"source": "?x",
		"label": "died",
		"value": "\"orphan\"^^<http://www.w3.org/2001/XMLSchema#string>"
	}
}
```

**Return sample**

```json
{
    "StatusCode": 0,
    "StatusMsg": "Save Successfully! the file path is ./dbhome/lubm.db/reason_rule_files/test.json"
}
```

#### listReason

**Brief description**

-  show rules list

**Request mode**

- POST

**Parameter transfer mode**

- POST request, `raw` in `body` in `Httprequest`, passed as `JSON `

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **reasonManage**                |
| username       | yes       | string | Username                                                     |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name                                                |
| type           | yes       | string | Operation type：“2”                                          |

**Return value** 

| Parameter name                    | Type      | Note                                                         |
| :-------------------------------- | :-------- | ------------------------------------------------------------ |
| StatusCode                        | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg                         | string    | Return specific information                                  |
| num                               | int       | Rule number                                                  |
| list                              | JSONArray | Rule name list                                               |
| &emsp;rulename                    | string    | Rule name（Same database，unique rule name）                 |
| &emsp;description                 | string    | Rule description                                             |
| &emsp;isenable                    | int       | Whether to enable, optional values: 1 enable ,0 disable      |
| &emsp;type                        | int       | Reason type:optional values:1 relational reason , 0 attribute reason |
| &emsp;&emsp;logic                 | int       | The relationship between conditions and conditions，optional values：1 logical and , 0 logical or |
| &emsp;conditions                  | array     | Rules set                                                    |
| &emsp;&emsp;condition             | arry      | Rule                                                         |
| &emsp;&emsp;&emsp;patterns        | array     | Triples set                                                  |
| &emsp;&emsp;&emsp;&emsp;subject   | string    | Subject                                                      |
| &emsp;&emsp;&emsp;&emsp;predicate | string    | Predicate                                                    |
| &emsp;&emsp;&emsp;&emsp;object    | string    | Object                                                       |
| &emsp;&emsp;&emsp;filters         | array     | Filter rule                                                  |
| &emsp;&emsp;&emsp;count_info      | Object    | Aggregate function（undetermined）                           |
| &emsp;return                      | Object    | Return object                                                |
| &emsp;&emsp;source                | string    | Source node                                                  |
| &emsp;&emsp;target                | string    | Terminal node                                                |
| &emsp;&emsp;label                 | string    | predicate（The system automatically adds the Rule: prefix)   |
| &emsp;&emsp;value                 | string    | Attribute value                                              |
| &emsp;status                      | string    | status                                                       |
| &emsp;insert_sparql               | string    | insert sparql                                                |
| &emsp;delete_sparql               | string    | delete sparql                                                |
| &emsp;createtime                  | string    | create time                                                  |

**Return value**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "ok",
    "list": [
        {
            "ruleid": "002",
            "rulename": "orphan",
            "description": "Both parents died and were orphaned",
            "conditions": [
                {
                    "condition": {
                        "patterns": [
                            {
                                "subject": "?x ",
                                "predicate": "<father>",
                                "object": "?y"
                            },
                            {
                                "subject": "?x",
                                "predicate": "<mother>",
                                "object": "?z"
                            },
                            {
                                "subject": "?y",
                                "predicate": "<state>",
                                "object": "\"died\"^^<http://www.w3.org/2001/XMLSchema#string>"
                            },
                            {
                                "subject": "?z",
                                "predicate": "<state>",
                                "object": "\"died\"^^<http://www.w3.org/2001/XMLSchema#string>"
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
                "label": "state",
                "value": "\"orphan\"^^<http://www.w3.org/2001/XMLSchema#string>"
            },
            "createtime": "2023-12-29 16:58:00",
            "status": "executed",
            "insert_sparql": "insert {?x <Rule:state> \"orphan\"^^<http://www.w3.org/2001/XMLSchema#string>. } where {  ?x  <father> ?y. ?x <mother> ?z. ?y <state> \"died\"^^<http://www.w3.org/2001/XMLSchema#string>. ?z <state> \"died\"^^<http://www.w3.org/2001/XMLSchema#string>. }",
            "delete_sparql": "delete where {?x <Rule:state> \"orphan\"^^<http://www.w3.org/2001/XMLSchema#string>.}"
        },
        {
            "ruleid": "001",
            "rulename": "uncle2",
            "description": "The mother's male sibling was the uncle",
            "conditions": [
                {
                    "condition": {
                        "patterns": [
                            {
                                "subject": "?x ",
                                "predicate": "<mother>",
                                "object": "?y"
                            },
                            {
                                "subject": "?y ",
                                "predicate": "<father>",
                                "object": "?z"
                            },
                            {
                                "subject": "?k",
                                "predicate": "<father>",
                                "object": "?z"
                            },
                            {
                                "subject": "?k",
                                "predicate": "<gender>",
                                "object": "\"male\"^^<http://www.w3.org/2001/XMLSchema#string>"
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
                "label": "uncle",
                "value": ""
            },
            "createtime": "2023-12-29 16:58:00",
            "status": "executed",
            "insert_sparql": "insert { ?x <Rule:uncle> ?k. } where {  ?x  <mother> ?y. ?y  <father> ?z. ?k <father> ?z. ?k <gender> \"male\"^^<http://www.w3.org/2001/XMLSchema#string>. }",
            "delete_sparql": "delete where {?x <Rule:unlce> ?y.}"
        }
    ],
    "num": 2
}
```

#### compileReason

**Brief description**  

- compile rules

**Request mode**

- POST

**Parameter transfer mode**

- POST request: `raw` in `body` in `httprequest` , passed as `JSON`  

**Parameter**

| parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name.fixed **reasonManage**                       |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name                                                |
| type           | yes       | string | Operation type：“3”                                          |
| rulename       | yes       | string | Rule name                                                    |

**Return value**

| parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| insert_sparql  | string | insert sparql                                                |
| delete_sparql  | string | delete sparql                                                |

**Return value**

``` json
{
    "insert_sparql": "insert {?x <Rule:state> \"orphan\"^^<http://www.w3.org/2001/XMLSchema#string>. } where {  ?x  <father> ?y. ?x <mother> ?z. ?y <state> \"died\"^^<http://www.w3.org/2001/XMLSchema#string>. ?z <state> \"died\"^^<http://www.w3.org/2001/XMLSchema#string>. }",
    "delete_sparql": "delete where {?x <Rule:state> \"orphan\"^^<http://www.w3.org/2001/XMLSchema#string>.}",
    "StatusCode": 0,
    "StatusMsg": "ok"
}
```

#### executeReason

**Brief description**

- execute rules

**Request mode**

- POST 

**Parameter transfer mode**

- POST request, `raw` in `body` in `Httprequest`, passed as`JSON `

**Parameter**

| parameter  | Mandatory | Type   | Note                                                         |
| :--------- | :-------- | :----- | ------------------------------------------------------------ |
| operation  | yes       | string | Operation name, fixed value is **reasonManage**             |
| username   | yes       | string | User name                                                    |
| password   | yes       | string | Password (plain text)                                        |
| encryption | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name    | yes       | string | Database name                                                |
| type       | yes       | string | Operation type ：“4”                                         |
| rulename   | yes       | string | Rule name                                                    |

**Return value**

| parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| insert_sparql  | string | insert sparql                                                |
| AnsNum         | int    | successful answer number                                     |

**return value**

``` json
{
    "insert_sparql": "insert {?x <Rule:state> \"orphan\"^^<http://www.w3.org/2001/XMLSchema#string>. } where {  ?x  <father> ?y. ?x <mother> ?z. ?y <state> \"died\"^^<http://www.w3.org/2001/XMLSchema#string>. ?z <state> \"died\"^^<http://www.w3.org/2001/XMLSchema#string>. }",
    "AnsNum": 0, 
    "StatusCode": 0,
    "StatusMsg": "ok"
}
```



#### disableReason

**Brief description**

- Invalidate the rule

**Request mode**

- POST 

**Parameter transfer mode**

- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed **reasonManage**                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name                                                |
| type           | yes       | string | Operation type：“5”                                          |
| rulename       | yes       | string | Rule name                                                    |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| delete_sparql  | string | Delete sparql                                                |
| AnsNum         | int    | successful answer number                                     |

**Return value**

``` json
{
    "delete_sparql": "delete where {?x <Rule:state> \"orphan\"^^<http://www.w3.org/2001/XMLSchema#string>.}",
    "AnsNum": 1, 
    "StatusCode": 0,
    "StatusMsg": "ok"
}
```

#### showReason

**Brief description**

- show rule details

**Request mode**

- POST 

**Parameter transfer mode**

- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value **reasonManage**                   |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password（plain test)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name                                                |
| type           | yes       | string | Operation type：“6”                                          |
| rulename       | yes       | string | Rule name                                                    |

**Return value**

| Parameter name              | Type      | Note                                                         |
| :-------------------------- | :-------- | ------------------------------------------------------------ |
| StatusCode                  | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg                   | string    | Return specific information                                  |
| ruleinfo                    | Object    | Object                                                       |
| insert_sparql               | string    | insert sparql                                                |
| delete_sparql               | string    | delete sparql                                                |
| status                      | string    | status                                                       |
| rulename                    | string    | Rule name（Same database，unique rule name）                 |
| description                 | string    | Rule description                                             |
| isenable                    | int       | Whether to enable                                            |
| type                        | int       | 1：relational reasoning   0：attribute reasoning             |
| &emsp;logic                 | int       | Relationship between conditions and conditions.  Optional values：1: logical and ; 0 : logical or |
| createtime                  | string    | Create time                                                  |
| conditions                  | JSONArray | Rule conditions（Array）                                     |
| &emsp;condition             | Object    | Rule                                                         |
| &emsp;&emsp;patterns        | array     | Triples set                                                  |
| &emsp;&emsp;&emsp;subject   | string    | Subject                                                      |
| &emsp;&emsp;&emsp;predicate | string    | Predicate                                                    |
| &emsp;&emsp;&emsp;object    | string    | Object                                                       |
| &emsp;&emsp;filters         | array     | Filter conditions                                            |
| &emsp;&emsp;count_info      | Object    | Aggregate function （undermined）                            |
| return                      | Object    | Return object                                                |
| &emsp;source                | string    | Source node                                                  |
| &emsp;target                | string    | Terminal node                                                |
| &emsp;label                 | string    | predicate（The system automatically adds the Rule: prefix)   |
| &emsp;value                 | string    | Attribute value                                              |

**Return value**

``` json
{
    "ruleid": "002",
    "rulename": "orphan",
    "description": "The person whose both parents died is orphan",
    "conditions": [
        {
            "condition": {
                "patterns": [
                    {
                        "subject": "?x ",
                        "predicate": "<father>",
                        "object": "?y"
                    },
                    {
                        "subject": "?x",
                        "predicate": "<mother>",
                        "object": "?z"
                    },
                    {
                        "subject": "?y",
                        "predicate": "<state>",
                        "object": "\"died\"^^<http://www.w3.org/2001/XMLSchema#string>"
                    },
                    {
                        "subject": "?z",
                        "predicate": "<state>",
                        "object": "\"died\"^^<http://www.w3.org/2001/XMLSchema#string>"
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
        "label": "state",
        "value": "\"orphan\"^^<http://www.w3.org/2001/XMLSchema#string>"
    },
    "createtime": "2023-12-29 16:58:00",
    "status": "disabled", 
    "insert_sparql": "insert {?x <Rule:state> \"orphan\"^^<http://www.w3.org/2001/XMLSchema#string>. } where {  ?x  <father> ?y. ?x <mother> ?z. ?y <state> \"died\"^^<http://www.w3.org/2001/XMLSchema#string>. ?z <state> \"died\"^^<http://www.w3.org/2001/XMLSchema#string>. }",
    "delete_sparql": "delete where {?x <Rule:state> \"orphan\"^^<http://www.w3.org/2001/XMLSchema#string>.}",
    "StatusCode": 0,
    "StatusMsg": "ok"
}
```

#### deleteReason

**Brief description**

- Delete rule

**Request mode**

- POST 

**Parameter transfer mode**

- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter  | Mandatory | Type   | Note                                                         |
| :--------- | :-------- | :----- | ------------------------------------------------------------ |
| operation  | yes       | string | Operation name, fixed value is **reasonManage**              |
| username   | yes       | string | User name                                                    |
| password   | yes       | string | Password（plain test)                                        |
| encryption | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name    | yes       | string | Database name                                                |
| type       | yes       | string | Operation type：“7”                                          |
| rulename   | yes       | string | Rule name                                                    |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return value**

``` json
{
    "StatusCode": 0,
    "StatusMsg": "the reason file has been remove successfully! file path:./dbhome/reason.db/reason_rule_files/orphan.json"
}
```

#### Appendix 1: Return value code table for details

|            Code             | Details                                     |
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

## Socket API Instruction

### API Interconnection Mode

The gServer interface uses the `socket` protocol and supports multiple ways to access the interface. If the port `9000` is started from the gServer in the Main directory, the contents of the interface interconnection are as follows:

API address：

```http
http://ip:9000/
```

The API supports the input of a parameter list in JSON format, as shown below：

```json
{"op": "[op_type]", "[paramname1]": "[paramvalue1]", "[paramname2]": "[paramvalue2]"……}
```
### API List

| API name | Definition                     | Note                                                    |
| -------- | ------------------------------ | ------------------------------------------------------- |
| build    | Build graph database           | The database file must be locally stored on the server  |
| load     | Load graph database            | Load the database into memory                           |
| unload   | Unload graph database          | Unload the database from memory                         |
| drop     | Delete graph database          | Logical deletion and physical deletion can be performed |
| show     | Display graph database         | Display list of all databases                           |
| query    | Query graph database           | Including query, delete, and insert                     |
| stop     | Close server                   | Only root user root can perform this operation          |
| close    | Close client server connection | Process client connection closure requests              |
| login    | login to database              | authenticate user names and password                    |

### API Specific Instruction

> This section describes the input and output parameters of each interface. Assume that the IP address of the gserver is 127.0.0.1 and the port is 9000

**(1) build- build database**

**Brief description**

- Create a database based on existing NT file
- Files must exist on the gStore server

**Request ip**

- ` 127.0.0.1 `

**Request port number**

- ` 9000 `

**Parameter transfer mode**

- Pass it as a `JSON` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| op             | yes       | string | Operation name, fixed value is **build**                     |
| db_name        | yes       | string | Database name (.db is not required)                          |
| db_path        | yes       | string | Database file path (can be an absolute path or a relative path. The relative path uses the gStore installation root directory as a reference directory) |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Import RDF file to database done."
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(2) load**

**Brief description**

- To load a database into memory, a load operation is a prerequisite for many operations, such as Query

**Request ip**

- ` 127.0.0.1 `

**Request port number**

- ` 9000 `

**Parameter transfer mode**

- Pass it as a `JSON` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                    |
| :------------- | :-------- | :----- | --------------------------------------- |
| op             | yes       | string | Operation name, fixed value is **load** |
| db_name        | yes       | string | Database name (.db is not required)     |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Load database successfully."
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(3) unload**

**Brief description**

- Unmount the database from memory (all changes are flushed back to hard disk)

**Request ip**

- ` 127.0.0.1 `

**Request port number**

- ` 9000 `

**Parameter transfer mode**

- Pass it as a `JSON` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                      |
| :------------- | :-------- | :----- | ----------------------------------------- |
| op             | yes       | string | Operation name, fixed value is **unload** |
| db_name        | yes       | string | Database name (.db is not required)       |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Unload database done."
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(4) drop**

**Brief description**

- Delete the database

**Request ip**

- ` 127.0.0.1 `

**Request port number**

- ` 9000 `

**Parameter transfer mode**

- Pass it as a `JSON` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                    |
| :------------- | :-------- | :----- | --------------------------------------- |
| op             | yes       | string | Operation name, fixed value is **drop** |
| db_name        | yes       | string | Database name (.db is not required)     |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Drop database done."
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(5) show**

**Brief description**

- Display all database list

**Request ip**

- ` 127.0.0.1 `

**Request port number**

- ` 9000 `

**Parameter transfer mode**

- Pass it as a `JSON` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                    |
| :------------- | :-------- | :----- | --------------------------------------- |
| op             | yes       | string | Operation name, fixed value is **show** |

**Return value**

| Parameter name    | Type      | Note                                                         |
| :---------------- | :-------- | ------------------------------------------------------------ |
| StatusCode        | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg         | string    | Return specific information                                  |
| ResponseBody      | JSONArray | JSON arrays (each of which is a database information0        |
| -------- database | string    | database name                                                |
| ---------status   | string    | database status                                              |

**Return sample** 

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
**(6) query**

**Brief description**

- query database

**Request ip**

- ` 127.0.0.1 `

**Request port number**

- ` 9000 `

**Parameter transfer mode**

- Pass it as a `JSON` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                            |
| :------------- | :-------- | :----- | ----------------------------------------------- |
| op             | yes       | string | Operation name, fixed value is **query**        |
| db_name        | yes       | string | database that need operations                   |
| format         | no        | string | The result set return format is json by default |
| sparql         | yes       | string | The SPARQL statement to execute                 |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| head           | JSON   | Head information                                             |
| results        | JSON   | Result information (see Return Sample for details)           |

**Return sample** 

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
**(7) login**

**Brief description**

- Login user (verify username and password)

**Request ip**

- ` 127.0.0.1 `

**Request port number**

- ` 9000 `

**Parameter transfer mode**

- Pass it as a `JSON` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| op             | yes       | string | Operation name, fixed value is **login** |
| username       | yes       | string | user name                                |
| password       | yes       | string | Password (plain text)                    |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 1001,
    "StatusMsg": "wrong password."
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(8) stop**

**Brief description**

- Close server

**Request ip**

- ` 127.0.0.1 `

**Request port number**

- ` 9000 `

**Parameter transfer mode**

- Pass it as a `JSON` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                    |
| :------------- | :-------- | :----- | --------------------------------------- |
| op             | yes       | string | Operation name, fixed value is **stop** |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Server stopped."
}
```

<div STYLE="page-break-after: always;"></div>
**(9) close**

**Brief description**

- Close the connection to the client

**Request ip**

- ` 127.0.0.1 `

**Request port number**

- ` 9000 `

**Parameter transfer mode**

- Pass it as a `JSON` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| op             | yes       | string | Operation name, fixed value is **close** |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Connection disconnected."
}
```

<div STYLE="page-break-after: always;"></div>
**Appendix 1 return value code table**

| Code value | Definition                                  |
| ---------- | ------------------------------------------- |
| 0          | Success                                     |
| 1000       | The method type is  not support             |
| 1001       | Authentication Failed                       |
| 1002       | Check Privilege  Failed                     |
| 1003       | Param is illegal                            |
| 1004       | The operation conditions  are not satisfied |
| 1005       | Operation failed                            |
| 1006       | Add privilege Failed                        |
| 1007       | Loss of lock                                |
| 1008       | Transcation manage Failed                   |
| 1100       | The operation is  not defined               |
| 1101       | IP Blocked                                  |

<!-- <div STYLE="page-break-after: always;"></div> -->

## C++ HTTP API

To use the C++ API, put the phrase `'#include "client.h"` in your CPP code, as shown below：

**Construct the initialization function**

	GstoreConnector(std::string serverIP, int serverPort, std::string httpType, std::string username, std::string password);
	Function: Initialize
	Parameter Definition：[Server IP], [HTTP port on the server], [HTTP service type], [Username], [password]
	Example：GstoreConnector gc("127.0.0.1", 9000, "ghttp", "root", "123456");

**Build database: build**
​	

	std::string build(std::string db_name, std::string rdf_file_path, std::string request_type);
	Function：Create a new database from an RDF file
	Parameter Definition：[database name], [.nt file path], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.build("lubm", "data/lubm/lubm.nt");

**Load database: load**

	std::string load(std::string db_name, std::string request_type);
	Function：Load the database you created
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.load("lubm");

**Stopping database loading: unload**
​	

	std::string unload(std::string db_name, std::string request_type);
	Function：Stopping database loading
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.unload("lubm");

**User management: user**

	std::string user(std::string type, std::string username2, std::string addition, std::string request_type);
	Function：The root user can add, delete, or modify the user's permission only
	1.Add or delete users：
	Parameter Definition：["add_user" adds user, "delete_user" deletes user],[user name],[password],[request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.user("add_user", "user1", "111111");
	2.Modify user's privilege：
	Parameter Definition：["Add_query" adds query permission, "delete_query" deletes query permission, "add_load" adds load permission, "delete_load" deletes load permission, "add_unload" adds no load permission, "delete_unload" deletes no load permission,  "Add_update" adds update permission, "delete_update" deletes update permission, "add_backup" adds backup permission, "delete_bakup" deletes backup permission, "add_restore" adds restore permission,  "Delete_restore" deletes restore permission,"add_export" adds export permission," delete_export" deletes export permission],[user name],[database name],[Request type "GET" and "POST ", if the request type is "GET", it can be omitted.]
	Example：gc.user("add_query", "user1", "lubm");

**Display user: showUser**

	std::string showUser(std::string request_type);
	Function：Display all Users
	Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.showUser();

**Database query: query**

	std::string query(std::string db_name, std::string format, std::string sparql, std::string request_type);
	Function：Query the database
	Parameter Definition：[database name], [query result type JSON, HTML or text], [SPARQL statement], [Request type "GET" and "POST ", if the request type is "GET", it can be omitted]
	Example：
	std::string res = gc.query("lubm", "json", sparql);
	std::cout << res << std::endl;  //output result

**Deleting a Database: drop**

	std::string drop(std::string db_name, bool is_backup, std::string request_type);
	Function：Delete the database directly or delete the database while leaving a backup
	Parameter Definition：[database name], [false not backup, true backup], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.drop("lubm", false);  //Delete the database without leaving a backup

**Monitoring database: monitor**

	std::string monitor(std::string db_name, std::string request_type);Function：Displays information for a specific database.
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]Example：gc.monitor("lubm");

**Save the database: checkpoint**

	std::string checkpoint(std::string db_name, std::string request_type);Function：If you change the database, save the databas Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]Example：gc.checkpoint("lubm");

**Show the database: show**

	std::string show(std::string request_type);Function：Displays all created databases Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"] Example：gc.show();

**The kernel version information is displayed: getCoreVersion**

	std::string getCoreVersion(std::string request_type);Function：Get kernel version information Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.getCoreVersion();

**The API version information is displaye: getAPIVersion**

	std::string getAPIVersion(std::string request_type);
	Function：Get the API version information
	Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.getAPIVersion();

**Query the database and save the file: fquery**

	void fquery(std::string db_name, std::string format, std::string sparql, std::string filename, std::string request_type);
	Function：Query the database and save the results to a file
	Parameter Definition：[database name], [query result type JSON, HTML or text], [SPARQL statement], [file name], [request type "GET" and "POST ", if the request type is "GET", it can be omitted]
	Example：gc.fquery("lubm", "json", sparql, "ans.txt");

**Exporting the Database**

	std::string exportDB(std::string db_name, std::string dir_path, std::string request_type);
	Function：Export the database to a folder
	Parameter Definition：[database name], [path to database export folder], [request type "GET" and "POST ", if the request type is "GET", can be omitted]
	Example：gc.exportDB("lubm", "/root/gStore/");



<!-- <div STYLE="page-break-after: always;"></div> -->
## Java  HTTP API

To use the Java API, please refer to the gStore/API/HTTP/Java/SRC/JGSC/GstoreConnector. Java. Specific use is as follows:

**Construct the initialization function**

```java
public class GstoreConnector(String serverIP, int serverPort, String httpType,
String username, String password);
Function: Initialize
Parameter Definition：[Server IP], [HTTP port on the server], [HTTP service type], [Username], [password]
Example：GstoreConnector gc = new GstoreConnector("127.0.0.1", 9000, "ghttp",
"root", "123456");
```

**Building a database: build**

	public String build(String db_name, String rdf_file_path, String request_type);
	Function：Create a new database from an RDF file
	Parameter Definition：[database name], [.nt file path], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.build("lubm", "data/lubm/lubm.nt");

**Loading a database: load**

	public String load(String db_name, String request_type);
	Function：Load the database you created
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.load("lubm");

**Stopping database loading: unload**

	public String unload(String db_name, String request_type);
	Function：Stopping database loading
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.unload("lubm");

**User management：user**

	public String user(String type, String username2, String addition, String request_type);
	Function：The root user can add, delete, or modify the user's permission only.
	1.Add or delete users：
	Parameter Definition：["add_user" adds user, "delete_user" deletes user],[user name],[password],[request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.user("add_user", "user1", "111111");
	2.Modify user's privilege：
	Parameter Definition：["Add_query" adds query permission, "delete_query" deletes query permission, "add_load" adds load permission, "delete_load" deletes load permission, "add_unload" adds no load permission, "delete_unload" deletes no load permission,  "Add_update" adds update permission, "delete_update" deletes update permission, "add_backup" adds backup permission, "delete_bakup" deletes backup permission, "add_restore" adds restore permission,  "Delete_restore" deletes restore permission,"add_export" adds export permission," delete_export" deletes export permission],[user name],[database name],[Request type "GET" and "POST ", if the request type is "GET", it can be omitted.]
	Example：gc.user("add_query", "user1", "lubm");

**Display user：showUser**

	public String showUser(String request_type);
	Function：Display all users
	Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.showUser();

**Database query: query**

	public String query(String db_name, String format, String sparql, String request_type);
	Function：query databse
	Parameter Definition：[database name], [query result type JSON, HTML or text], [SPARQL statement], [Request type "GET" and "POST ", if the request type is "GET", it can be omitted]
	Example：
	String res = gc.query("lubm", "json", sparql);
	System.out.println(res); //output result

**Database deletion ：drop**

	public String drop(String db_name, boolean is_backup, String request_type);
	Function：Delete the database directly or delete the database while leaving a backup.
	Parameter Definition：[database name], [false not backup, true backup], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.drop("lubm", false);  //Delete the database without leaving a backup

**Monitoring database: monitor**

	public String monitor(String db_name, String request_type);
	Function：Displays information for a specific database
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]Example：gc.monitor("lubm");

**Save the database: checkpoint**

	public String checkpoint(String db_name, String request_type);
	Function：If you change the database, save the database 
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]Example：gc.checkpoint("lubm");

**Show database：show**

	public String show(String request_type);Function：Displays all created databasesParameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]Example：gc.show();

**The kernel version information is displayed：getCoreVersion**

	public String getCoreVersion(String request_type);Function：Get kernel version information Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]Example：gc.getCoreVersion();

**Display API version：getAPIVersion**

	public String getAPIVersion(String request_type);Function：Get API version Parameter：[Request types "GET" and "POST" can be omitted if the request type is "GET"]Example：gc.getAPIVersion();

**Query the database and save the file: fquery**

	public void fquery(String db_name, String format, String sparql, String filename, String request_type);Function：Query the database and save the result to a file Parameter definition：[database name], [query result type JSON, HTML or text], [SPARQL statement], [file name]，[Request types "GET" and "POST" can be omitted if the request type is "GET"]Example：gc.fquery("lubm", "json", sparql, "ans.txt");

**Export database**

	public String exportDB(String db_name, String dir_path, String request_type);Function：Export database to parameter definition under file folder：[database name], [database export folder path]，[Request types "GET" and "POST" can be omitted if the request type is "GET"】Example：gc.exportDB("lubm", "/root/gStore/");

<!-- <div STYLE="page-break-after: always;"></div> -->
## Python HTTP API

To use the Python API, please refer to the gStore/API/HTTP/Python/src/GstoreConnector. Py. Specific use is as follows:

**Construct the initialization function**

```python
def __init__(self, ip, port, username, password, http_type='ghttp'):
Function: Initialize
Parameter Definition：[Server IP], [HTTP port on the server], [Username], [password],[HTTP service type]
Example：gc = GstoreConnector.GstoreConnector("127.0.0.1", 9000, "ghttp", "root",
"123456")
```

**Build database: build**

	def build(self, db_name, rdf_file_path, request_type):
	Function：Create a new database from an RDF file
	Parameter definition：[Database name]，[.nt文件路径]，[Request types "GET" and "POST" can be omitted if the request type is "GET"】
	Example：res = gc.build("lubm", "data/lubm/lubm.nt")

**Load database: load**

	def load(self, db_name, request_type):
	Function：load the database you have created
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：res = gc.load("lubm")

**Unload database: unload**

	def unload(self, db_name, request_type):
	Function：Unload database
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：res = gc.unload("lubm")

**User management: user**

	def user(self, type, username2, addition, request_type):
	Function：The root user can add, delete, or modify the user's permission only.
	1.Add or delete users：
	Parameter definition：["add_user" adds a user, "delete_user" deletes a user],  [username],[password],[Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：res = gc.user("add_user", "user1", "111111")
	2.Modify user's privilege：
	Parameter definition：["Add_query" adds query permission, "delete_query" deletes query permission, "add_load" adds load permission, "delete_load" deletes load permission, "add_unload" adds no load permission, "delete_unload" deletes no load permission,  "Add_update" adds update permission, "delete_update" deletes update permission, "add_backup" adds backup permission, "delete_bakup" deletes backup permission, "add_restore" adds restore permission,  "Delete_restore" deletes restore permission,"add_export" adds export permission," delete_export" deletes export permission],  [user name],[database name],[Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：res = gc.user("add_query", "user1", "lubm")

**Display users: showUser**

	def showUser(self, request_type):
	Function：Display all users
	Parameter definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.showUser()

**Query Database: query**

	def query(self, db_name, format, sparql, request_type):
	Function：Query the database
	Parameter definition：[Database name]，[Query result type JSON, HTML or text], [SPARQL statement], [Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：
	res = gc.query("lubm", "json", sparql)
	print(res) //output result

**Database deletion: drop**

	def drop(self, db_name, is_backup, request_type):
	Function：Delete the database directly or delete the database while leaving a backup
	Parameter definition：[database name], [false not backup, true backup],[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.drop("lubm", false)  //Delete the database without leaving a backup

**Database Monitor ：monitor**   	

	def monitor(self, db_name, request_type):    Function：Displays information for a specific database Parameter definition：[Database name]，[Request types "GET" and "POST" can be omitted if the request type is "GET"] 
	Example：res = gc.monitor("lubm")

**Save database: checkpoint**

	def checkpoint(self, db_name, request_type):Function：If the database is changed, save the meaning of the database parameters：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted] Example：res = gc.checkpoint("lubm")

**Display database：show**

	def show(self, request_type):Function：Display the meanings of all created databases：[Request types "GET" and "POST" can be omitted if the request type is "GET"] 
	Example：res = gc.show()

**The kernel version information is displayed: getCoreVersion**

	def getCoreVersion(self, request_type):Function：Get the definition of kernel version parameter ：[Request types "GET" and "POST" can be omitted if the request type is "GET"] 
	Example：res = gc.getCoreVersion()

**Display API version: getAPIVersion**	

	def getAPIVersion(self, request_type):Function：Get the API version information parameter definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.getAPIVersion()

**Query the database and save the file: fquery**

	def fquery(self, db_name, format, sparql, filename, request_type):Function：Query the database and save the result to a file Parameter definition：[database name], [query result type JSON, HTML or text], [SPARQL statement], [file name]，[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.fquery("lubm", "json", sparql, "ans.txt")

**Export databse**

	def exportDB(self, db_name, dir_path, request_type): Function：parameter definition of exprotin database to folders：[database name], [database export folder path]，[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.exportDB("lubm", "/root/gStore/")

<!-- <div STYLE="page-break-after: always;"></div> -->
## Node.js HTTP API

Before using the Nodejs API, type `NPM install Request` and `NPM Install request-promise` to add the required modules under the Nodejs folder.

To use Nodejs API, please refer to the `gStore/API/http/Nodejs/GstoreConnector.js`. Specific use is as follows:

**Construct the initialization function**

	class GstoreConnector(ip = '', port, httpType = 'ghttp', username = '', password
	= '');
	Function: Initialize
	Parameter Definition：[Server IP], [HTTP port on the server], [HTTP service type], [Username], [password]
	Example：gc = new GstoreConnector("127.0.0.1", 9000, "ghttp", "root", "123456");

**Build database: build**

	async build(db_name = '', rdf_file_path = '', request_type);
	Function：Create a new database from an RDF file
	The defintion of parameters are as follows: [database name], [.nt file path],[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.build("lubm", "data/lubm/lubm.nt");

**Load database：load**

	async load(db_name = '', request_type);
	Function：Load the database you have created
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：res = gc.load("lubm");

**Unload databse：unload**

	async unload(db_name = '', request_type);
	Function：Unload databse
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：res = gc.unload("lubm");

**User management：user**

	async user(type = '', username2 = '' , addition = '' , request_type);
	Function：The root user can add, delete, or modify the user's permission only
	1.Add or delete users：
	Parameter definition：["add_user" adds a user, "delete_user" deletes a user],  [user name],[password],[Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：res = gc.user("add_user", "user1", "111111");
	2.Privilege to modify user：
	Parameter definition：["Add_query" adds query permission, "delete_query" deletes query permission, "add_load" adds load permission, "delete_load" deletes load permission, "add_unload" adds no load permission, "delete_unload" deletes no load permission,  "Add_update" adds update permission, "delete_update" deletes update permission, "add_backup" adds backup permission, "delete_bakup" deletes backup permission, "add_restore" adds restore permission,  "Delete_restore" deletes restore permission,"add_export" adds export permission," delete_export" deletes export permission],  [user name],[database name],[Request types "GET" and "POST" can be omitted if the Request type is "GET"]]
	Example：res = gc.user("add_query", "user1", "lubm");

**Display user：showUser**

	async showUser(request_type);
	Function：Display all users
	Parameter definition: [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.showUser();

**Query database：query**

	async query(db_name = '', format = '' , sparql = '' , request_type);
	Function：Query database
	Parameter definition：[Database name], [query result type JSON, HTML or text], [SPARQL statement], [Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：
	res = gc.query("lubm", "json", sparql);
	console.log(JSON.stringify(res,",")); //output result

**Database deletion：drop**

	async drop(db_name = '', is_backup , request_type);
	Function：Delete the database directly or delete the database while leaving a backup
	Parameter definition：[database name]，[false no backup, true backup]，[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.drop("lubm", false);  //Delete the database without leaving a backup

**Database monitor: monitor**   	

	async monitor(db_name = '', request_type);     Function：Parameter definition for displaying information about a specific database：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]Example：res = gc.monitor("lubm");

**Save database: checkpoint**

	async checkpoint(db_name = '', request_type);Function：If the database is changed, the parameter definition of saving database：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]Example：res = gc.checkpoint("lubm");

**Display database: show**

	async show(request_type);Function：Displays all created databases Parameter definition: [Request types "GET" and "POST" can be omitted if the request type is "GET"]Example：res = gc.show();

**Display kernel version information: getCoreVersion**	

	async getCoreVersion(request_type);Function：Get kernel version information 
	Parameter definition: [Request types "GET" and "POST" can be omitted if the request type is "GET"]Example：res = gc.getCoreVersion();

**Display API version: getAPIVersion**			

	async getAPIVersion(request_type);
	Function：Get the API version information	
	Parameter definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.getAPIVersion();

**Query the database and save the file: fquery**

	async fquery(db_name = '', format = '' , sparql = '' , filename = '' , request_type);
	Function：Query the database and save the results to a file
	Parameter definition：[database name], [query result type JSON, HTML or text], [SPARQL statement], [file name],  [Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：gc.fquery("lubm", "json", sparql, "ans.txt");

**Export database**

	async exportDB(db_name = '' , dir_path = '' , request_type); 
	Function：export database to folders
	parameter definition：[database name], [directory where the database is exported], [Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：res = gc.exportDB("lubm", "/root/gStore/");

<!-- <div STYLE="page-break-after: always;"></div> -->
## PHP HTTP API

To use the Php API, please refer to the gStore/API/HTTP/Php/SRC/GstoreConnector. Php. Specific use is as follows:

**Construct the initialization function**

```php
class GstoreConnector($ip, $port, $httpType, $username, $password)
Function: Initialize
Parameter Definition：[Server IP], [HTTP port on the server], [HTTP service type], [Username], [password]
Example：$gc = new GstoreConnector("127.0.0.1", 9000, "ghttp", "root", "123456");
```

**Build databse：build**

	function build($db_name, $rdf_file_path, $request_type)
	Function：Create a new database from an RDF file
	Parameter definition：[database name], [.nt file path],[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：
	$res = $gc->build("lubm", "data/lubm/lubm.nt");
	echo $res . PHP_EOL;

**Load database：load**

	function load($db_name, $request_type)
	Function：Load the database you have created
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：
	$ret = $gc->load("test");
	echo $ret . PHP_EOL;

**Unload databse：unload**

	function unload($db_name, $request_type)
	Function：Unload databse
	Parameter definition：[database name]，[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：
	$ret = $gc->unload("test");
	echo $ret . PHP_EOL;

**User management：user**

	function user($type, $username2, $addition, $request_type)
	Function：Only the root user can add, delete, or modify the user's permission
	1.Add or delete users：
	Parameter definition：["add_user" adds a user, "delete_user" deletes a user],  [user name],[password],[Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：
	$res = $gc->user("add_user", "user1", "111111");
	echo $res . PHP_EOL;
	2.Privilege to modify user：
	参数含义：["Add_query" adds query permission, "delete_query" deletes query permission, "add_load" adds load permission, "delete_load" deletes load permission, "add_unload" adds no load permission, "delete_unload" deletes no load permission,  "Add_update" adds update permission, "delete_update" deletes update permission, "add_backup" adds backup permission, "delete_bakup" deletes backup permission, "add_restore" adds restore permission,  "Delete_restore" deletes restore permission,"add_export" adds export permission," delete_export" deletes export permission],  [user name],[database name],[Request types "GET" and "POST" can be omitted if the Request type is "GET"]]
	Example：
	$res = $gc->user("add_user", "user1", "lubm");
	echo $res . PHP_EOL;

**Display user：showUser**

	function showUser($request_type)
	Function：Display all users
	Parameter definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：
	$res = $gc->showUser();
	echo $res. PHP_EOL;

**Query database：query**

	function query($db_name, $format, $sparql, $request_type)
	Parameter definition：[database name], [query result type JSON, HTML or text], [SPARQL statement],[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：
	$res = $gc->query("lubm", "json", $sparql);
	echo $res. PHP_EOL; //output result

**Database deletion：drop**

	function drop($db_name, $is_backup, $request_type)
	Function：Delete the database directly or delete the database while leaving a backup
	Parameter definition：[database name], [false not backup, true backup],[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：
	$res = $gc->drop("lubm", false); //Delete the database without leaving a backup
	echo $res. PHP_EOL;             

**Database monitor: monitor**  	

	function monitor($db_name, $request_type)Function：Displays information for a specific database 
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：$res = $gc->monitor("lubm");echo $res. PHP_EOL;

**Save database: checkpoint**

	function checkpoint($db_name, $request_type)Function：the Parameter definition of save database if it has been changed：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]Example：$res = $gc->checkpoint("lubm");echo $res. PHP_EOL;

**Display database: show**

	function show($request_type)Function：Displays all created databases
	Parameter definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：$res = $gc->show();echo $res. PHP_EOL;

**Display kernel version information: getCoreVersion**

	function getCoreVersion($request_type)Function：get kernel version information
	Parameter definition: [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：$res = $gc->getCoreVersion();echo $res. PHP_EOL;

**Display API version: getAPIVersion**	

	function getAPIVersion($request_type)
	Function：Get API version	
	Parameter definition: [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：
	$res = $gc->getAPIVersion();
	echo $res. PHP_EOL;

**Query the database and save the file：fquery**

	function fquery($db_name, $format, $sparql, $filename, $request_type)
	Function：Query the database and save the results to a file
	Parameter definition：[database name], [query result type JSON, HTML or text], [SPARQL statement], [file name],[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：$gc->fquery("lubm", "json", $sparql, "ans.txt");

**Export database**

	function exportDB($db_name, $dir_path, $request_type)
	Function：Export the database to a folder
	Parameter definition：[database name], [database export folder path]，[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：$res = $gc->exportDB("lubm", "/root/gStore/");

<!-- <div STYLE="page-break-after: always;"></div> -->