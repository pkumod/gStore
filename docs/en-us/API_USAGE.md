## Introduction to API

gStore provides API services to users through http and Socket services, and its components are gRPC, ghttp and gServer.

### HTTP API

We now provide C++, Java, Python, PHP, and Node.js APIs for gRPC and ghttp. Please refer to the examples in `api/http/cpp`, `api/http/java`, `api/http/python`, `api/http/php` and `api/http/nodejs`. To use these examples, make sure you have generated the executable files of gRPC or ghttp. **Next, please use the `bin/grpc` or `bin/ghttp` command to start the service.** If you know of a running gRPC or ghttp server that's available for connection, you can also directly connect to it. Then, for the C++ and Java code, you need to compile the sample code in the directory `api/http/cpp/example` or`api/http/java/example`.

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

### Socket API

We now provide C++ (Java, Python, PHP, and Node.js will be supported in future versions) API for gServer, the socket API service. Please see api/socket/cpp for the example code. To use these examples, make sure you have generated an executable file for gServer. **Next, use the `bin/gserver -s` command to start the gserver service.** If you know of a running gServer that's available for connection, you can also directly connect to it. Then, for the C++ code, you need to compile the sample code in the directory `api/socket/cpp/example`. 

**For details on how to start and shut down gServer, please see [development documentation] - [Quick Start] - [Socket API service].**

**After the Socket API is started, you can connect through the Socket. The default port of gServer is 9000.**

<!-- <!-- <div STYLE="page-break-after: always;"></div> --> -->
## HTTP API framework

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
## ghttp API instruction

### API Interconnection Mode

The ghttp interface adopts the `HTTP` protocol and supports multiple ways to access the interface. If the ghttp is started on the port `9000`, the interface interconnection content is as follows

API address：

```http
http://ip:9000/
```

The interface supports both `GET` and `POST` requests, where `GET` requests place parameters in the URL and `POST` requests place parameters in the `body` request.

 

**Note: `GET` request parameters contain special characters, such as? , @,& and other characters, you need to use urlencode encoding, especially the `SPARQL` parameter must be encoded**

 

### API list

| API name             | Definition                                | Note                                                         |
| -------------------- | ----------------------------------------- | ------------------------------------------------------------ |
| build                | build graph database                      | The database file must be locally stored on the server       |
| load                 | load graph database                       | Load the database into memory                                |
| unload               | unload graph database                     | Unload the database from memory                              |
| monitor              | monitor graph database                    | Count information about the specified database (such as the number of triples, etc.) |
| drop                 | drop graph database                       | Logical deletion and physical deletion can be performed      |
| show                 | display graph database                    | Display a list of all databases                              |
| usermanage           | user management                           | Add, delete, or modify user information                      |
| showuser             | display all user list                     | Display a list of all users                                  |
| userprivilegemange   | user privilege management                 | Add, delete, or modify user's privilege information          |
| backup               | backup database                           | backup database information                                  |
| backuppath           | get the backup database path              | Returns a list of all backup files in the default backup path. |
| restore              | restore database                          | restore database information                                 |
| query                | query database                            | Including query, delete, and insert                          |
| export               | export database                           | Export database as NT file                                   |
| login                | login to database                         | It is used to authenticate user names and passwords          |
| ch                   | heartbeat signal                          | Detect ghttp heartbeat signal                                |
| init（abandon）      | Initialize system                         | This operation should not be initialized by ghttp            |
| refresh（abandon）   | reload database                           | This can be done by unload+ Load                             |
| parameter（abandon） | Set parameters for backup                 | All parameters are currently boiled down to the config.ini file and cannot be changed remotely |
| begin                | Start transaction                         | Transaction starts and needs to be used in conjunction with TQuery |
| tquery               | Querying the database (with transactions) | Data queries with transaction mode (insert and DELETE only)  |
| commit               | commit transactions                       | Commit the transaction after it completes                    |
| rollback             | rollback transaction                      | Roll back the transaction to begin state                     |
| txnlog               | Obtain transaction log information        | Return transcation log information as json                   |
| checkpoint           | write data to a disk                      | After an INSERT or delete operation is performed on the database, manually checkpoint is required |
| testConnect          | testing connectivity                      | Used to check whether GHTTP is connected                     |
| getCoreVersion       | get gStore version                        | Get the gStore version number                                |
| batchInsert          | batch insert data                         | Batch insert NT data                                         |
| batchRemove          | batch delete data                         | Batch delete NT data                                         |
| querylog（added）    | Get query log information                 | Query log information is returned as json                    |
| ipmanage（added）    | black/white list management               | Maintains a blacklist and whitelist of IP addresses that access gStore |
| querylogdate         | Get the date list of query logs           | query the date list of existing query logs                   |
| accesslog            | get the access log of API                 | return API access log information in json format             |
| accesslogdate        | get the date of API log                   | query the date list of existing API logs                     |
| funquery             | query operator function pagination        | get the custom operator function list                        |
| funcudb              | management operator function              | operator function's addition, modification, deletion, compilation |
| funreview            | preview operator function                 | when creating and updating, you can view the source code of the final generated operator function through preview interface |
| upload               | upload files                              | support file types to upload are nt, ttl, n3, rdf, txt.      |
| download             | download files                            | support to download files in gstore main directory and its sub directories. |
| rename               | rename graph database                     | modify graph database name.                                  |
| stat                 | query system resources                    | statistics CPU, memory, disk available space information.    |

<!-- <!-- <div STYLE="page-break-after: always;"></div> --> -->
### API specific intruction

> The input and output parameters of each interface are specified in this section. Assume that the IP address of the GHTTP server is 127.0.0.1 and the port is 9000

**(1) build -build datbase**

**Brief description**

- Create a database based on existing NT files
- Files must exist on the gStore server

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON `structure

**Parameter**

| parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **build**                     |
| db_name        | yes       | string | Database name (.db is not required)                          |
| db_path        | yes       | string | Database file path (can be an absolute path or a relative path. The relative path uses the gStore installation root directory as a reference directory) |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |

**Return value**

| Parameter name | Tpye   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| Failed_num     | int    | Number of build failures                                     |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Import RDF file to database done.",
    "failed_num": 0
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(2) check**

**Brief description**

- Check whether the GHTTP service is online

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- ###### GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is **check** |

**Return value**

| Parameter name | Type   | Note                                                         |
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

<!-- <div STYLE="page-break-after: always;"></div> -->
**(3) load (Updated in this version)**

**Brief description**  

- Loading a database into memory is a prerequisite for many operations, such as query and monitor.
- Updates in this version: Added the load_csr parameter, default is false

**Request URL**

- `http://127.0.0.1:9000/`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request: the parameters are passed directly as the URL
- POST request: `raw` in `body` in `httprequest` , passed as `JSON`  

**Parameter**  

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `load`                        |
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

**(4) monitor (Updated in this version)**

**Brief description**

- Get database statistics (database needs to be loaded)
- Updates in this version: parameter names in the return values are modified (the parameters with spaces and underscores are changed to camel case form, e.g. triple num -> tripleNum)

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON `

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `monitor`                     |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |

**Return value** 

| Parameter name                     | Type   | Note                                                         |
| ---------------------------------- | ------ | ------------------------------------------------------------ |
| StatusCode                         | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg                          | string | Return specific information                                  |
| database                           | string | database name                                                |
| creator                            | string | creator                                                      |
| builtTime                          | string | create time                                                  |
| tripleNum                          | string | number of triples                                            |
| entityNum                          | int    | number of entities                                           |
| literalNum                         | int    | number of characters (attribute value)                       |
| subjectNum                         | int    | number of subjects                                           |
| predicateNum                       | int    | number of objects                                            |
| connectionNum                      | int    | number of connections                                        |
| <font color=red>diskUsed</font>    | int    | disk space（MB）                                             |
| <font color=red>subjectList</font> | Array  | Entity type statistics                                       |

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

<!-- <div STYLE="page-break-after: always;"></div> -->
**(5) unload**

**Brief description**

- Unload the database from memory (all changes are flushed back to hard disk)

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**unload** |
| db_name        | yes       | string | Database name (.db is not required)      |
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
    "StatusCode": 0,
    "StatusMsg": "Database unloaded."
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(6) drop**

**Brief description**

- Delete the database (either logically or physically)

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**drop**                       |
| db_name        | yes       | string | Database name (.db is not required)                          |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| is_backup      | no        | string | True: Logical deletion, false: represents physical deletion (default true)，if it's logical deletion, change the file folder to .bak file foder, user can change the folder name to. Db and add the folder to the system database by calling bin/ ginit-db database name |

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

<!-- <div STYLE="page-break-after: always;"></div> -->
**(7) show**

**Brief description**

- Display all database list

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**show**                       |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | No        | string | If it is empty, the password is clear text. If it is 1, it means it is encrypted with md5. |

**Return value**

| Parameter name      | Type      | Note                                                         |
| :------------------ | :-------- | ------------------------------------------------------------ |
| StatusCode          | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg           | string    | Return specific information                                  |
| ResponseBody        | JSONArray | JSON arrays (each of which is a database information)        |
| -------- database   | string    | database name                                                |
| ---------creator    | string    | creator                                                      |
| ---------built_time | string    | create time                                                  |
| ---------status     | string    | database status                                              |

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

<!-- <div STYLE="page-break-after: always;"></div> -->
**(8) usermanage**

**Brief description**

- Manage users (including adding, deleting, and changing users)

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**usermanage**                 |
| type           | yes       | string | Operation Type（1：adduser ，2：deleteUser 3：alterUserPassword） |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| op_username    | yes       | string | User name for the operation                                  |
| op_password    | yes       | string | Password of the operation (if the password is to be changed, the password is the password to be changed) (If the operation contains special characters and the get request is adopted, the value must be urlencode-encoded) |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{    "StatusCode": 1004,    "StatusMsg": "username already existed, add user failed."}
```

**Notes**

The default interface permissions for the new user are: `login`, `check`, `testConnect`, `getCoreVersion`, `show`, `funquery`, `funcudb`, `funreview`, `userpassword`.

Users with `query` rights also have the following interface rights: `query`, `monitor`.

Users with the `update` permission also have the following interface permissions: `batchInsert`, `batchRemove`, `begin`, `tquery`, `commit`, `rollback`.

Only the root user can invoke the interface rights that are not within the scope of authorization management, for example: `build`, `drop`, `usermanage`, `showuser`, `userprivilege`, `manage`, `txnlog`, `checkpoint`, `shutdown`, `querylog`, `accesslog`, `ipmanage`.

<!-- <div STYLE="page-break-after: always;"></div> -->
**(9) showuser**

**Brief description**

- Display all user information

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**showuser**                   |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | No        | string | If it is empty, the password is clear text. If it is 1, it means it is encrypted with md5. |

**Return value**

| Parameter name        | Type      | Note                                                         |
| :-------------------- | :-------- | ------------------------------------------------------------ |
| StatusCode            | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg             | string    | Return specific information                                  |
| ResponseBody          | JsonArray | JSON object array                                            |
| ------username        | string    | user name                                                    |
| ------password        | string    | password                                                     |
| -----query_privilege  | string    | Query permissions (database names separated by commas)       |
| ----update_privilege  | string    | Update permissions (database names separated by commas)      |
| ----load_privilege    | string    | Load permissions (database names separated by commas)        |
| ---unload_privilege   | string    | Unload permissions (database names separated by commas)      |
| ----backup_privilege  | string    | Back up permissions (database names separated by commas)     |
| ----restore_privilege | string    | Restore permissions (database names separated by commas)     |
| ---export_privilege   | string    | Export permissions (database names separated by commas)      |

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

<!-- <div STYLE="page-break-after: always;"></div> -->
**(10) userprivilegemanage**

**Brief description**

- Manage user permissions (including adding, deleting, and changing users)

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**userprivilegemanage**        |
| type           | yes       | string | Operation type（1：add privilege，2：delete privilege 3：clear Privilege ） |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
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
{    "StatusCode": 0,    "StatusMsg": "add privilege query successfully. \r\nadd privilege load successfully. \r\nadd privilege unload successfully. \r\nadd privilege update successfully. \r\nadd privilege backup successfully. \r\n"}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(11) userpassword (New in this version)**

**Brief description**

- Modify the user password.

**Request URL**

- `http://127.0.0.1:9000/`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`  

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `userpassword`                |
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
	"StatusCode": 1004,
	"StatusMsg": "change password done."
}
```



**(12) backup**

**Brief description**

- Back up database

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**backup**                     |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | database that need operaions                                 |
| backup_path    | no        | string | The backup file path can be relative or absolute. The relative path uses the gStore root directory as reference. The default path is the backup directory in the gStore root directory |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| backupfilepath | string | Backup file path (this value can be used as the input parameter value for restore) |

**Return sample** 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database backup successfully.",
    "backupfilepath": "./backups/lubm.db_210828211529"
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(13) backuppath**

**Summary Description**

- Obtain all backup files of the database located under the default backup path.

**Request Methods**

- GET/POST

**Parameter Passing Methods**

- For GET requests, parameters are passed directly in the URL.
- For POST requests, the `raw` field in the `body` of the `httprequest` is passed in JSON structure.

**Parameters**

| Parameter Name | Required | Type   | Description                                                  |
| -------------- | -------- | ------ | ------------------------------------------------------------ |
| operation      | Yes      | string | Operation name, fixed value is **backuppath**.               |
| username       | Yes      | string | Username.                                                    |
| password       | Yes      | string | Password (plaintext).                                        |
| encryption     | No       | string | If it is empty, the password is plaintext. If it is 1, it means encryption with MD5. |
| db_name        | Yes      | string | Database name that needs to be queried.                      |

**Return Values**

| Parameter Name | Type    |                                                  Description |
| -------------- | ------- | -----------------------------------------------------------: |
| StatusCode     | integer | Return value code (please refer to the attached table for specific values). |
| StatusMsg      | string  |                                 Specific return information. |
| paths          | Array   | Backup file paths (this value can be used as input for restore operation). |

**Return sample** 

```
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "paths": [
        "./backups/lubm.db_220828211529",
        "./backups/lubm.db_221031094522"
    ]
}
```



**(14) restore**

**Brief description**

- Restore database

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**restore**                    |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | database that need operations                                |
| backup_path    | no        | string | The full time-stamped path of the backup file (can be a relative path or an absolute path. The relative path is based on the gStore root directory). The default path is the backup directory in the gStore root directory |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{    "StatusCode": 0,    "StatusMsg": "Database restore successfully."}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(15) query**

**Brief description**

- query the database

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**query**                      |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | database that need operations                                |
| format         | no        | string | The result set returns in json, HTML, and file format. The default is JSOn |
| sparql         | yes       | string | Sparql statement to execute (SPARQL requires URL encoding if it is a GET request) |

**Return value**

| Parameter Name | Type | Description                                                  |
| -------------- | ---- | ------------------------------------------------------------ |
| StatusCode     | int  | Return value code (please refer to the attached table for specific values) |
| StatusMsg      | str  | Specific return information                                  |
| head           | JSON | Head information                                             |
| results        | JSON | Result information (please refer to the return example for details) |
| AnsNum         | int  | Number of results                                            |
| OutputLimit    | int  | Maximum number of return results (-1 for no limit)           |
| ThreadId       | str  | Query thread ID                                              |
| QueryTime      | str  | Query time (milliseconds)                                    |

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
    "StatusMsg": "success",
    "AnsNum": 15,
    "OutputLimit": -1,
    "ThreadId": "140595527862016",
    "QueryTime": "1"
}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(16) export**

**Brief description**

- export database

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                      |
| :------------- | :-------- | :----- | ----------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**restore** |
| username       | yes       | string | user name                                 |
| password       | yes       | string | Password (plain text)                     |
| db_name        | yes       | string | database that need operations             |
| db_path        | no        | string | Export path (gstore root by default)      |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| filepath       | string | Path for exporting files                                     |

**Return sample** 

``` json
{    "StatusCode": 0,    "StatusMsg": "Export the database successfully.",    "filepath": "export/lubm_210828214603.nt"}
```



<!-- <div STYLE="page-break-after: always;"></div> -->
**(17) login (Updated in this version)**

**Brief description**

- User login (verify username and password) 
- Updates in this version: the full path information of RootPath will be returned after successful login

**Request URL**

- `http://127.0.0.1:9000/`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `login`                       |
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

**(18) begin**

**Brief description**

- start transaction

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**begin**                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
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

<!-- <div STYLE="page-break-after: always;"></div> -->
**(19) tquery**

**Brief description**

- query the transaction type

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**tquery** |
| username       | yes       | string | User name                                |
| password       | yes       | string | Password (plain text)                    |
| db_name        | yes       | string | Database name                            |
| tid            | yes       | string | Transaction ID                           |
| sparql         | yes       | string | sparql statement                         |

**Return value**

| Parameter Name |  Type  |                         Description                          |
| :------------: | :----: | :----------------------------------------------------------: |
|   StatusCode   |  int   | Return value code (please refer to the attached table for specific values) |
|   StatusMsg    | string |                 Specific return information                  |
|      head      |  JSON  | Head information (only returned when executing query-type statements) |
|    results     |  JSON  | Result information (only returned when executing query-type statements) |

**Return sample** 

``` json
{    "result": "",    "StatusCode": 0,    "StatusMsg": "success"}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(20) commit** 

**Brief description**

- submit transaction

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**commit** |
| username       | yes       | string | User name                                |
| password       | yes       | string | Password (plain text)                    |
| db_name        | yes       | string | Database name                            |
| tid            | yes       | string | Transaction ID                           |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{    "StatusCode": 0,    "StatusMsg": "transaction commit success. TID: 1"}
```

 <!-- <div STYLE="page-break-after: always;"></div> -->
**(21) rollback** 

**Brief description**

- Rollback trasnsaction

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                       |
| :------------- | :-------- | :----- | ------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**rollback** |
| username       | yes       | string | User name                                  |
| password       | yes       | string | Password (plain text)                      |
| db_name        | yes       | string | Database name                              |
| tid            | yes       | string | Transaction ID                             |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{    "StatusCode": 0,    "StatusMsg": "transaction rollback success. TID: 2"}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(22) txnlog (Updated in this version)**

**Brief description**

- Get transaction logs (this function is only available for the root user)  
- Updates in this version: added paging query parameters

**Request URL**

- `http://127.0.0.1:9000/`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `txnlog`                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| pageNo         | yes       | int    | Page number, the value ranges from 1 to N, the default value is 1 |
| pageSize       | yes       | int    | The number of entries per page, the value ranges from 1 to N, the default value is 10 |

**Return value**

| Parameter name  | Type      | Note                                                         |
| --------------- | --------- | ------------------------------------------------------------ |
| StatusCode      | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg       | string    | Return specific information                                  |
| totalSize       | int       | Total size                                                   |
| totalPage       | int       | The total number of pages                                    |
| pageNo          | int       | The current page number                                      |
| pageSize        | int       | The number of entries per page                               |
| list            | JSONArray | Log array                                                    |
| ---- db_name    | string    | Database name                                                |
| ---- TID        | string    | Transaction ID                                               |
| ---- user       | string    | User                                                         |
| ---- state      | string    | State (COMMITED/RUNNING/ROLLBACK)                            |
| ---- begin_time | string    | Start time                                                   |
| ---- end_time   | string    | End time                                                     |

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

<!-- <div STYLE="page-break-after: always;"></div> -->
**(23) checkpoint**

**Brief description**

- Received Flush data back to hard disk (to make data final)

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                         |
| :------------- | :-------- | :----- | -------------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**checkpoint** |
| username       | yes       | string | User name                                    |
| password       | yes       | string | Password (plain text)                        |
| db_name        | yes       | string | Database name                                |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
{    "StatusCode": 0,    "StatusMsg": "Database saved successfully."}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(24) testConnect (Updated in this version)**

**Brief description**

- Test whether the server can connect (for Workbench)
- Updates in this version: added HTTP service type in the return value

**Request URL**

- `http://127.0.0.1:9000/`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `testConnect`                 |
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

**(25) getCoreVersion (Updated in this version)**

**Brief description**

- Get the server version number (for Workbench)
- Updates in this version: added HTTP service type in the return value

**Request URL**

- `http://127.0.0.1:9000/`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `getCoreVersion`              |
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

**(26) batchInsert**

**Brief description**

- batch insert data

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**batchInsert**                |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | Database name                                                |
| file           | yes       | string | The data NT file to insert (can be a relative or absolute path) |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| success_num    | string | Number of successful executions                              |

**Return sample** 

``` json
{    "StatusCode": 0,    "StatusMsg": "Batch Insert Data  Successfully.",    "success_num": "25"}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(27) batchRemove**

**Brief description**

- batch remove data

**Request URL**

- ` http://127.0.0.1:9000/ `

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**batchRemove**                |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | Database name                                                |
| file           | yes       | string | Data NT files to be deleted (can be relative or absolute paths) |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| success_num    | int    | Number of successful executions                              |

**Return sample** 

``` json
{    "StatusCode": 0,    "StatusMsg": "Batch Remove Data  Successfully.",    "success_num": "25"}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(28) shutdown**

**Brief description**

- close ghttp

**Request URL**

- ` http://127.0.0.1:9000/shutdown `[Note, address change]  

**Request mode**

- GET/POST 

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| username       | yes       | string | user name（default user name is system)                      |
| password       | yes       | string | Password（This password need to be viewed in the server's system.db/password[port].txt file，for example, if the port is 9000，then check password in password9000.txt file) |

**Return value**

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample** 

``` json
If no value is returned, no information is received by default on success, and an error JSON message is returned on failure
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(29) querylog**

**Brief description**

- Obtaining query Logs

**Request URL**

`http://127.0.0.1:9000`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**querylog**                   |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| date           | yes       | string | Date format is yyyyMMdd                                      |
| pageNo         | yes       | int    | Page number. The value ranges from 1 to N. The default value is 1 |
| pageSize       | yes       | int    | The number of entries per page. The value ranges from 1 to N. The default value is 10 |

**Return value**

| Parameter     | Type   | Note                                                         |
| :------------ | :----- | ------------------------------------------------------------ |
| StatusCode    | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg     | string | Return specific information                                  |
| totalSize     | int    | Total number                                                 |
| totalPage     | int    | Total page number                                            |
| pageNo        | int    | Current page number                                          |
| pageSize      | int    | The number of entries per page                               |
| list          | Array  | Log array                                                    |
| QueryDateTime | string | Query date/time                                              |
| Sparql        | string | SPARQL statement                                             |
| Format        | string | Query return format                                          |
| RemoteIP      | string | Request IP                                                   |
| FileName      | string | Query result set files                                       |
| QueryTime     | int    | Time (ms)                                                    |
| AnsNum        | int    | Result number                                                |

**Return sample**

```json
{	"StatusCode":0,    "StatusMsg":"Get query log success",	"totalSize":64,	"totalPage":13,	"pageNo":2,	"pageSize":5,	"list":[		{			"QueryDateTime":"2021-11-16 14:55:52:90ms:467microseconds",			"Sparql":"select ?name where { ?name <dislike> <Eve>. }",			"Format":"json",			"RemoteIP":"183.67.4.126",			"FileName":"140163774674688_20211116145552_847890509.txt",			"QueryTime":0,			"AnsNum":2		}        ......    ]}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(30) querylogdate**

**Brief description**

- Get the date of gStore's query log (for the date parameter of the query log interface)

**Request URL**

`http://127.0.0.1:9000`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as `JSON ` 

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `querylogdate`                |
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

**(31) accesslog**

**Brief description**

- Get the access log of the API

**Request URL**

`http://127.0.0.1:9000`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as `JSON `

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `accesslog`                   |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| date           | yes       | string | Date format is yyyyMMdd                                      |
| pageNo         | yes       | int    | Page number. The value ranges from 1 to N. The default value is 1 |
| pageSize       | yes       | int    | The number of entries per page. The value ranges from 1 to N. The default value is 10 |

**Return value**

| Parameter       | Type   | Note                                                         |
| :-------------- | :----- | ------------------------------------------------------------ |
| StatusCode      | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg       | string | Return specific information                                  |
| totalSize       | int    | Total number                                                 |
| totalPage       | int    | Total page number                                            |
| pageNo          | int    | Current page number                                          |
| pageSize        | int    | The number of entries per page                               |
| list            | Array  | Log array                                                    |
| ---- ip         | string | The IP that makes the access                                 |
| ----operation   | string | The type of operation performed                              |
| ---- createtime | string | The time of the operation                                    |
| ---- code       | string | The result of the operation (refer to Appendix: Return value code table for details) |
| ---- msg        | string | Log description                                              |

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

**(32) accesslogdate**

**Brief description**

- Get the date of gStore's access log (for the date parameter of the access log interface)

**Request URL**

`http://127.0.0.1:9000`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as `JSON `

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `querylogdate`                |
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

**(33) ipmanage**

**Brief description**

- Blacklist and whitelist management

**Request URL**

`http://127.0.0.1:9000`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as `JSON ` structure

**Parameter**

Querying the blacklist and whitelist:

| Parameter name | Mandatory | Type   | Note                                       |
| -------------- | --------- | ------ | ------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**ipmanage** |
| username       | yes       | string | User name                                  |
| password       | yes       | string | Password (plain text)                      |
| type           | yes       | string | Operation type, fixed value is**1**        |

Saving the blacklist and whitelist:

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**ipmanage**                   |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| type           | yes       | string | Operation type, fixed value is**2**                          |
| whiteIPs       | yes       | string | Whitelist (multiple with **,** split, support range configuration, use **-** connection such as: IP1-1P2) |
| blackIPs       | yes       | string | Blacklist (multiple with **,** split, support range configuration, use **-** connection such as: IP1-1P2) |

```json
//保存POST示例{    "operation": "ipmanage",    "username": "root",    "password": "123456",    "type": "2",    "whiteIPs":"127.0.0.1,183.67.4.126-183.67.4.128",    "blackIPs": "192.168.1.141"}
```

**Return value**

| Parameter    | Type   | Note                                                         |
| :----------- | :----- | ------------------------------------------------------------ |
| StatusCode   | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg    | string | Return specific information                                  |
| ResponseBody | Object | Return data (only for queries)                               |
| whiteIPs     | array  | Whitelist                                                    |
| blackIPs     | array  | Blacklist                                                    |

**Return sample**

```json
// 查询黑白名单返回{    "StatusCode": 0,    "StatusMsg": "success",    "ResponseBody": {        "whiteIPs": [            "127.0.0.1",            "183.67.4.126-183.67.4.128"        ],        "blackIPs": [            "192.168.1.141"        ]    }}// 保存黑白名单返回{    "StatusCode": 0,    "StatusMsg": "success"}
```

<!-- <div STYLE="page-break-after: always;"></div> -->
**(34) funquery (New in this version)**

**Brief description**

- Show user-defined graph analysis functions

**Request URL**

- `http://127.0.0.1:9000/`

**Request mode**

- POST

**Parameter transfer mode**

- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name | Mandatory | Type       | Note                                                         |
| -------------- | --------- | ---------- | ------------------------------------------------------------ |
| operation      | yes       | string     | Operation name, fixed value is `funquery`                    |
| username       | yes       | string     | User name                                                    |
| password       | yes       | string     | Password (plain text)                                        |
| encryption     | no        | string     | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| funInfo        | no        | JSONObject | Query parameters                                             |
| ---- funName   | no        | string     | Function name                                                |
| ---- funStatus | no        | string     | Status (1- to compile; 2- compiled; 3- exception)            |

**Return value**

| Parameter name | Type      | Note                                                         |
| -------------- | --------- | ------------------------------------------------------------ |
| StatusCode     | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string    | Return specific information                                  |
| list           | JSONArray | JSON array (if there is no data, no array is returned)       |
| ---- funName   | string    | Name                                                         |
| ---- funDesc   | string    | Description                                                  |
| ---- funArgs   | string    | Parameter type (1- no K hop parameter; 2- with K hop parameter) |
| ---- funBody   | string    | Function content                                             |
| ---- funSubs   | string    | Child functions (can be called in funBody)                   |
| ---- funStatus | string    | Status (1- to compile; 2- compiled; 3- exception)            |
| ---- lastTime  | string    | Last edit time (yyyy-MM-dd HH:mm:ss)                         |

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

**(35) funcudb (New in this version)**

**Brief description**

- User-defined graph analysis function management (add, modify, delete, compile)

**Request URL**

- `http://127.0.0.1:9000/`

**Request mode**

- POST

**Parameter transfer mode**

- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name | Mandatory | Type       | Note                                                         |
| -------------- | --------- | ---------- | ------------------------------------------------------------ |
| operation      | yes       | string     | Operation name, fixed value is `funcudb`                     |
| username       | yes       | string     | User name                                                    |
| password       | yes       | string     | Password (plain text)                                        |
| encryption     | no        | string     | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| type           | yes       | string     | 1: add, 2: modify, 3: delete, 4: compile                     |
| funInfo        | yes       | JSONObject | Operator function                                            |
| ---- funName   | yes       | string     | Function name                                                |
| ---- funDesc   | no        | string     | Description                                                  |
| ---- funArgs   | no        | string     | Parameter type (1- no K hop parameter; 2- with K hop parameter). Required for add or modify |
| ---- funBody   | no        | string     | Function contents (wrapped in {}). Required for add or modify |
| ---- funSubs   | no        | string     | Child functions (can be called in funBody)                   |
| ---- funReturn | no        | string     | Return type. Required for add or modify                      |

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

**(36) funreview (New in this version)**

**Brief description**

- Preview the user-defined graph analysis function

**Request URL**

- `http://127.0.0.1:9000/`

**Request mode**

- POST

**Parameter transfer mode**

- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name | Mandatory | Type       | Note                                                         |
| -------------- | --------- | ---------- | ------------------------------------------------------------ |
| operation      | yes       | string     | Operation name, fixed value is `funreview`                   |
| username       | yes       | string     | User name                                                    |
| password       | yes       | string     | Password (plain text)                                        |
| encryption     | no        | string     | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| funInfo        | yes       | JSONObject | Operator function                                            |
| ---- funName   | yes       | string     | Function name                                                |
| ---- funDesc   | no        | string     | Description                                                  |
| ---- funArgs   | no        | string     | Parameter type (1- no K hop parameter; 2- with K hop parameter) |
| ---- funBody   | no        | string     | Function contents (contents wrapped in {})                   |
| ---- funSubs   | no        | string     | Child function (can be called in funBody)                    |
| ---- funReturn | no        | string     | Return type (`path`: the path class result; `value`: value class result) |

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

**(37) upload**

**Brief Description**

- Uploading files. Currently supported file formats include nt, ttl, and txt

**Request URL**

- ` http://127.0.0.1:9000/file/upload ` (note that the address has changed)

**Request method**

- POST

**Parameter Transfer Methods**

- POST request, 'form-data' in the 'body' of 'httprequest' (requires the RequestHeader parameter Content-Type: multipart/form-data

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

**(38) download**

**Brief Description**

- Download files, currently supports downloading files from the gStore root directory.

**Request URL**

- ` http://127.0.0.1:9000/file/download ` (Note that the address has changed)

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

```
Content-Range: bytes 0-389/389
Content-Type: application/octet-stream
Date: Tue, 01 Nov 2022 17:21:40 GMT
Content-Length: 389
Connection: Keep-Alive
```

**(39) rename**

**Brief Description**

- Rename the database.

**Request URL**

- `http://127.0.0.1:9000/`

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

**(40) stat**

**Brief Description**

- Statistics system resource information.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request Method**

- GET/POST

**Parameter Passing Method**

- For GET requests, parameters are passed directly in the URL.
- For POST requests, the `raw` field in the `body` of the `httpRequest` is used to pass parameters in JSON structure.

**Parameters**

| Parameter Name | Required |  Type  |                       Description                       |
| :------------: | :------: | :----: | :-----------------------------------------------------: |
|   operation    |   Yes    | string |        Operation name, fixed value is **stat**.         |
|    username    |   Yes    | string |                        Username                         |
|    password    |   Yes    | string |                  Password (plaintext)                   |
|   encryption   |    No    | string | Empty if password is plaintext, 1 means MD5 encryption. |

**Return Values**

| Parameter Name |  Type  |                         Description                          |
| :------------: | :----: | :----------------------------------------------------------: |
|   StatusCode   |  int   | Return value code value (please refer to the attached table: Return Value Code Table) |
|   StatusMsg    | string |                 Specific return information                  |
|   cup_usage    | string |                       CPU usage ratio                        |
|   mem_usage    | string |                    Memory usage (unit MB)                    |
| disk_available | string |                Available disk space (unit MB)                |

**Return Sample**

```json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "cup_usage": "10.596026",
    "mem_usage": "2681.507812",
    "disk_available": "12270"
}
```

**Appendix: return value code table**

| Code value | Definition                                  |
| ---------- | ------------------------------------------- |
| 0          | Success                                     |
| 1000       | The method type is not supported            |
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
## grpc API instruction

### API Connection Mode

The grpc interface adopts the HTTP protocol and supports multiple ways to access the interface. If the grpc is started on the port 9000, the interface connection information is as follows.

API address:

```http
http://ip:9000/grpc
```

The interface supports both `GET` and `POST` requests, where `GET` requests place parameters in the URL and `POST` requests place parameters in the `body` request or use `form` to express a request.

Post request method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

Post request method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

**Note: If the `GET` request parameters contain special characters, such as? , @, and &, you need to use the urlencode encoding, especially for the `SPARQL` parameter.**

### API list

| API name                             | Definition                                | Note                                                         |
| ------------------------------------ | ----------------------------------------- | ------------------------------------------------------------ |
| check                                | Detect heartbeat signal                   | Detects service heartbeat signal                             |
| login                                | login to database                         | Used to authenticate user names and passwords                |
| testConnect                          | testing connectivity                      | Used to check whether GRPC is connected                      |
| getCoreVersion                       | get gStore version                        | Gets the gStore version number                               |
| ipmanage                             | black/white list management               | Maintains a blacklist and whitelist of IP addresses that access gStore |
| show                                 | display graph database                    | Displays the list of all databases                           |
| load                                 | load graph database                       | Loads the database into memory                               |
| unload                               | unload graph database                     | Unloads the database from memory                             |
| monitor                              | monitor graph database                    | Displays information about the specified database (such as the number of triples, etc.) |
| build                                | build graph database                      | Requires the database file to be stored locally on the server |
| drop                                 | drop graph database                       | Logical deletion or physical deletion can be performed       |
| backup                               | backup database                           | Backups database information                                 |
| restore                              | restore database                          | Restores database information                                |
| query                                | query database                            | Includes query, delete, and insert                           |
| export                               | export database                           | Exports database as NT file                                  |
| begin                                | Start transaction                         | Transaction starts and needs to be used in conjunction with TQuery |
| tquery                               | Querying the database (with transactions) | Data queries with transaction mode (insert and delete only)  |
| commit                               | commit transactions                       | Commits the transaction after it completes                   |
| rollback                             | rollback transaction                      | Rolls back the transaction to begin state                    |
| checkpoint                           | write data to a disk                      | After an insert or delete operation is performed on the database, manual checkpoint is required |
| batchInsert                          | batch insert data                         | Batch inserts NT data                                        |
| batchRemove                          | batch delete data                         | Batch deletes NT data                                        |
| usermanage                           | user management                           | Adds, deletes, or modifies user information                  |
| showuser                             | display all user list                     | Displays a list of all users                                 |
| userprivilegemanage                  | user privilege management                 | Adds, deletes, or modifies user's privilege information      |
| userpassword                         | modify user's password                    |                                                              |
| txnlog                               | Obtain transaction log information        | Returns transcation log information as JSON                  |
| querylog                             | Get query log information                 | Returns query log information as JSON                        |
| querylogdate                         | Gets the date list of query logs          | Returns the date list of existing query logs                 |
| accesslog                            | Get API access logs                       | Returns API access log information as JSON                   |
| accesslogdate                        | Get the date of the API log               | Returns the date list of existing API logs                   |
| funquery                             | Query operator function                   | Gets a list of user-defined graph analysis functions         |
| funcudb                              | Manage operator function                  | Adds, modifies, deletes, or compiles operator function       |
| funreview                            | Preview operator function                 | View the latest generated graph analysis function source code |
| shutdown                             | Stop gRPC service                         |                                                              |
| <font color=red>upload(new)</font>   | upload files                              | The supported file types for uploading are nt, ttl, n3, rdf, txt |
| <font color=red>download(new)</font> | download files                            | supports downloading files under the main directory and subdirectories of gstore |
| <font color=red>rename(new)</font>   | Rename the graph database                 | Modify the name of the graph database                        |
| <font color=red>stat(new)</font>     | Query system resources                    | Statistics CPU, memory, disk available space information     |

### API specific intruction

> The input and output parameters of each interface are specified in this section. Assume that the IP address of the GRPC server is 127.0.0.1 and the port is 9000.

**(1) check**

**Brief description**

- Check whether the gRPC service is online

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

Request mode

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

  method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

**Parameter**

| Parameter name | Mandatory | type   | Note                                     |
| -------------- | --------- | ------ | ---------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is **check** |

**Return value**

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "the grpc server is running..."
}
```

**(2) login**

**Brief description**

- User login (verify username and password)

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

  method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

**Parameter**

| Parameter name | Mandatory | type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **login**                     |
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
| type           | string | HTTP service type. The fixed value is grpc                   |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "login successfully",
	"CoreVersion": "1.2",
	"licensetype": "opensource",
	"Rootpath": "/data/gstore",
	"type": "grpc"
}
```

**(3) testConnect**

**Brief description**

- Test whether the server can connect (for Workbench)

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

  method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

**Parameter**

| Parameter name | Mandatory | type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **testConnect**               |
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
| type           | string | HTTP service type. The fixed value is grpc                   |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"CoreVersion": "1.2",
	"licensetype": "opensource",
	"type": "grpc"
}
```

**(4) getCoreVersion**

**Brief description**

- Get the server version number (for Workbench)

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

  method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

**Parameter**

| Parameter name | Mandatory | type   | Note                                                         |
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
| type           | string | HTTP service type. The fixed value is grpc                   |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"CoreVersion": "1.2",
	"type": "grpc"
}
```

**(5)  ipmanage**

**Brief description**

- Blacklist and whitelist management

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

  method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

**Parameter**

Querying blacklist and whitelist:

| Parameter name | Mandatory | type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **ipmanage**                  |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| type           | yes       | string | Operation type, fixed value is 1                             |

Saving blacklist and whitelist:

| Parameter name | Mandatory | type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **ipmanage**                  |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| type           | yes       | string | Operation type, fixed value is 2                             |
| ip_type        | yes       | string | List type: 1-blacklist; 2- whitelist                         |
| ips            | yes       | string | IP list (use`,` to split; support range configuration with `-`, e.g. ip1-1p2) |

```json
//Example of Saving blacklist and whitelist
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

| Parameter name | Type       | Note                                                         |
| -------------- | ---------- | ------------------------------------------------------------ |
| StatusCode     | int        | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string     | Return specific information                                  |
| ResponseBody   | JSONObject | Data returned (only returned when queried)                   |
| ---- ip_type   | string     | List type: 1- blacklist; 2- whitelist                        |
| ---- ips       | array      | list                                                         |

**Return sample**

```json
//Query the blacklist and whitelist
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
//Save the blacklist and whitelist
{
	"StatusCode": 0,
	"StatusMsg": "success"
}
```

**(6) show**

**Brief description**

- Display the list of all databases

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

  method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

**Parameter**

| Parameter name | Mandatory | type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **show**                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

**Return value**

| Parameter name  | Type      | Note                                                         |
| --------------- | --------- | ------------------------------------------------------------ |
| StatusCode      | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg       | string    | Return specific information                                  |
| ResponseBody    | JSONArray | JSON arrays (each of which is a database information)        |
| ---- database   | string    | database name                                                |
| ---- creator    | string    | creator                                                      |
| ---- built_time | string    | create time                                                  |
| ---- status     | string    | database status                                              |

**Return sample**

```json
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

**(7) load**

**Brief description**

- Load the database into memory. Loading a database is a prerequisite for many operations, such as query and monitor.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

  method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `load`                        |
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

**(8) unload**

**Brief description**

- Unload the database from memory (all changes are flushed to disk).

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

  method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `unload`                      |
| db_name        | yes       | string | Database name (.db is not required)                          |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Database unloaded."
}
```

**(9) monitor**

**Brief description**

- Get database statistics (requires the database to be loaded)

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

  method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `monitor`                     |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |

**Return value**  

| Parameter name                     | Type   | Note                                                         |
| ---------------------------------- | ------ | ------------------------------------------------------------ |
| StatusCode                         | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg                          | string | Return specific information                                  |
| database                           | string | database name                                                |
| creator                            | string | creator                                                      |
| builtTime                          | string | create time                                                  |
| tripleNum                          | string | number of triples                                            |
| entityNum                          | int    | number of entities                                           |
| literalNum                         | int    | number of literals (i.e., attributes)                        |
| subjectNum                         | int    | number of subjects                                           |
| predicateNum                       | int    | number of objects                                            |
| connectionNum                      | int    | number of connections                                        |
| <font color=red>diskUsed</font>    | int    | disk space（MB）                                             |
| <font color=red>subjectList</font> | Array  | Entity type statistics                                       |

**Return sample**

```json
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

**(10) build**

**Brief description**

- Create a database based on an existing NT file.
- The file must be store locally on the gStore server.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `build`                       |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| db_path        | yes       | string | Database file path (can be an absolute path or a relative path. The relative path uses the gStore installation root directory as a reference directory) |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| failed_num     | int    | 构建失败数量                                                 |

**Return sample**

```json
{
    "StatusCode": 0,
    "StatusMsg": "Import RDF file to database done.",
    "failed_num": 0
}
```

**(11) drop**

**Brief description**

- Delete the database (either logically or physically)

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `drop`                        |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| is_backup      | no        | string | Database file path (can be an absolute path or a relative path. The relative path uses the gStore installation root directory as a reference directory) |

**Return value**

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Database test_lubm dropped."
}
```

**(12) backup**

**Brief description**

- Back up database

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `backup`                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| backup_path    | no        | string | Database file path (can be an absolute path or a relative path. The relative path uses the gStore installation root directory as a reference directory) |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| backupfilepath | string | Backup file path (this value can be used as the input parameter value for restore) |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Database backup successfully.",
	"backupfilepath": "./backups/lubm.db_210828211529"
}
```

**(13) backuppath**

**Brief Description**

- Get all backup files of the database in the default backup path.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request Method**

- GET/POST

**Parameter Transmission Method**

- For GET requests, parameters are transmitted directly in the URL.

- For POST requests, there are two ways to transmit parameters:

  Method 1: Transmit parameters in JSON structure through the `raw` field in the `body` of `httpRequest` (requires RequestHeader parameter Content-Type:application/json).

  Method 2: Transmit parameters in Form form (requires RequestHeader parameter Content-Type:application/x-www-form-urlencoded).

**patameter**

| Parameter Name | Required | Type   | Description                                                  |
| -------------- | -------- | ------ | ------------------------------------------------------------ |
| operation      | Yes      | string | Operation name, fixed value is **backuppath**                |
| username       | Yes      | string | Username                                                     |
| password       | Yes      | string | Password (plaintext)                                         |
| encryption     | No       | string | Empty means password is plaintext, 1 means encrypted with MD5 |
| db_name        | Yes      | string | Database name to be queried                                  |

**Return Values**

| Parameter Name | Type   | Description                                                  |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code (please refer to the attached table: Return Value Code Table) |
| StatusMsg      | string | Detailed return information                                  |
| paths          | Array  | Backup file paths (this value can be used as the input parameter for restore) |

**Return sample：**

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

**(14) restore**

**Brief description**

- Restore database

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `restore`                     |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| backup_path    | no        | string | The full time-stamped path of the backup file (can be a relative path or an absolute path. The relative path is based on the gStore root directory). The default path is the backup directory in the gStore root directory |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Database lumb restore successfully."
}
```

**(15) query**

**Brief description**

- Query the database

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `query`                       |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| format         | no        | string | The result set format (JSON/file/JSON+file; the default is JSON) |
| sparql         | yes       | string | The SPARQL query to execute. (Requires URL encoding for GET requests) |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| head           | JSON   | Head information                                             |
| results        | JSON   | Result information (see Return Sample for details)           |
| AnsNum         | int    | Number of results                                            |
| OutputLimit    | int    | Maximum number of results returned (-1 is not limited)       |
| ThreadId       | string | Query thread Number                                          |
| QueryTime      | string | Query time (ms)                                              |
| FileName       | string | Result file name (when the format is file or JSON+file)      |

**Return sample**

```json
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
// Result file root directory：%gstore_home%/query_result
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

**(16) export**

**Brief description**

- Export the database

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `export`                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| db_path        | no        | string | The export file path can be relative or absolute. The relative path uses the gStore root directory as reference. |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| filepath       | string | Export file path                                             |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Export the database successfully.",
	"filepath": "export/lubm_210828214603.nt"
}
```

**(17) begin**

**Brief description**

- start transaction

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `begin`                       |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| isolevel       | yes       | string | Transaction isolation level. 1: RC (read committed); 2: SI (snapshot isolation); 3: SR (seriablizable） |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| TID            | string | Transaction ID                                               |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "transaction begin success",
	"TID": "1"
}
```

**(18) tquery**

**Brief description**

- Query as transaction

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `tquery`                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| tid            | yes       | string | Transaction ID                                               |
| sparql         | yes       | string | The SPARQL query to execute                                  |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| head           | JSON   | Header information (this field is returned when a query statement is executed, but not an update statement) |
| results        | JSON   | Result information (this field is returned when a query statement is executed, but not an update statement) |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "success"
}
```

**(19) commit**

**Brief description**

- Commit a transaction

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `commit`                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| tid            | yes       | string | Transaction ID                                               |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Transaction commit success. TID: 1"
}
```

**(20) rollback**

**Brief description**

- Rollback trasnsaction.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `rollback`                    |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| tid            | yes       | string | Transaction ID                                               |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Transaction rollback success. TID: 2"
}
```

**(21) checkpoint**

**Brief description**

- Flush data back to hard disk to make it persistent.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `checkpoint`                  |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Database saved successfully."
}
```

**(22) batchInsert**

**Brief description**

- Batch insert data.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `batchInsert`                 |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| file           | yes       | string | The NT data file to insert (can be a relative or absolute path) |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| success_num    | string | Number of successfully inserted triples                      |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Batch insert data successfully.",
	"success_num": "25"
}
```

**(23) batchRemove**

**Brief description**

- Batch remove data.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `batchRemove`                 |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| file           | yes       | string | The data NT file to be deleted (can be a relative or absolute path) |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| success_num    | string | Number of successfully removed triples                       |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Batch remove data successfully.",
	"success_num": "25"
}
```

**(24)  usermanage**

**Brief description**

- Manage users (including adding and deleting users, and modifying user information)

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `usermanage`                  |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| type           | yes       | string | Operation type（1: adduser, 2: deleteUser, 3: alterUserPassword） |
| op_username    | yes       | string | The name of the user to operate on                           |
| op_password    | no        | string | Password of the user operated on, which can be left empty when deleting the user. If the password is to be changed, input the new password here. (If the password contains special characters and the GET request is used, it must be urlencode-encoded.) |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Add user done."
}
```

**Notes**

The default interface permissions for a new user are: `login`, `check`, `testConnect`, `getCoreVersion`, `show`, `funquery`, `funcudb`, `funreview`, `userpassword`.

Users with `query` rights also have the following interface rights: `query`, `monitor`.

Users with the `update` permission also have the following interface permissions: `batchInsert`, `batchRemove`, `begin`, `tquery`, `commit`, `rollback`.

Only the root user can invoke the interface rights that are not within the scope of authorization management, for example: `build`, `drop`, `usermanage`, `showuser`, `userprivilege`, `manage`, `txnlog`, `checkpoint`, `shutdown`, `querylog`, `accesslog`, `ipmanage`.

**(25) showuser**

**Brief description**

- Display all users' information.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `showuser`                    |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

**Return value**  

| Parameter name         | Type   | Note                                                         |
| ---------------------- | ------ | ------------------------------------------------------------ |
| StatusCode             | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg              | string | Return specific information                                  |
| ResponseBody           | string | JSON object array                                            |
| ---- username          | string | user name                                                    |
| ---- password          | string | password                                                     |
| ---- query_privilege   | string | Query permissions (database names separated by commas)       |
| ---- update_privilege  | string | Update permissions (database names separated by commas)      |
| ---- load_privilege    | string | Load permissions (database names separated by commas)        |
| ---- unload_privilege  | string | Unload permissions (database names separated by commas)      |
| ---- backup_privilege  | string | Backup permissions (database names separated by commas)      |
| ---- restore_privilege | string | Restore permissions (database names separated by commas)     |
| ---- export_privilege  | string | Export permissions (database names separated by commas)      |

**Return sample**

```json
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

**(26) userprivilegemanage**

**Brief description**

- Manage user permissions (including adding, deleting, and modifying them).

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `userprivilegemanage`         |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| type           | yes       | string | Operation type (1: add permission，2: delete permission, 3: clear permission) |
| op_username    | yes       | string | The name of the user to operate on                           |
| privileges     | no        | string | Permissions to operate on (multiple permissions are separated by commas; if it is a clear operation, this can be an empty string.) 1: query, 2: load, 3: unload, 4: update, 5: backup, 6: restore, 7: export |
| db_name        | no        | string | The database to operate on (this can be empty if it is a clear operation) |

**Return value**  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "add privilege query successfully. \r\nadd privilege load successfully. \r\nadd privilege unload successfully. \r\nadd privilege update successfully. \r\nadd privilege backup successfully. \r\n"
}
```

**(27) userpassword**

**Brief description**

- Modify the user password.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `userpassword`                |
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
	"StatusCode": 0,
	"StatusMsg": "Change password done."
}
```

**(28) txnlog**

**Brief description**

- Get transaction logs.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `txnlog`                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| pageNo         | yes       | int    | Page number, the value ranges from 1 to N, the default value is 1 |
| pageSize       | yes       | int    | The number of entries per page, the value ranges from 1 to N, the default value is 10 |

**Return value**

| Parameter name  | Type      | Note                                                         |
| --------------- | --------- | ------------------------------------------------------------ |
| StatusCode      | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg       | string    | Return specific information                                  |
| totalSize       | int       | Total size                                                   |
| totalPage       | int       | The total number of pages                                    |
| pageNo          | int       | The current page number                                      |
| pageSize        | int       | The number of entries per page                               |
| list            | JSONArray | Log array                                                    |
| ---- db_name    | string    | Database name                                                |
| ---- TID        | string    | Transaction ID                                               |
| ---- user       | string    | User                                                         |
| ---- state      | string    | State (COMMITED/RUNNING/ROLLBACK)                            |
| ---- begin_time | string    | Start time                                                   |
| ---- end_time   | string    | End time                                                     |

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

**(29) querylog**

**Brief description**

- Obtaining query logs.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type         | Note                                                         |
| -------------- | --------- | ------------ | ------------------------------------------------------------ |
| operation      | yes       | string       | Operation name, fixed value is `querylog`                    |
| username       | yes       | string       | User name                                                    |
| password       | yes       | string       | Password (plain text)                                        |
| encryption     | noyes     | stringstring | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| date           | yes       | string       | Date format is yyyyMMdd                                      |
| pageNo         | yes       | int          | Page number, the value ranges from 1 to N, the default value is 1 |
| pageSize       | yes       | int          | The number of entries per page, the value ranges from 1 to N, the default value is 10 |

**Return value**

| Parameter name     | Type      | Note                                                         |
| ------------------ | --------- | ------------------------------------------------------------ |
| StatusCode         | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg          | string    | Return specific information                                  |
| totalSize          | int       | Total size                                                   |
| totalPage          | int       | The total number of pages                                    |
| pageNo             | int       | The current page number                                      |
| pageSize           | int       | The number of entries per page                               |
| list               | JSONArray | Log array                                                    |
| ---- QueryDateTime | string    | Query time                                                   |
| ---- RemoteIP      | string    | The IP that sends the request                                |
| ---- Sparql        | string    | SPARQL query                                                 |
| ---- AnsNum        | int       | Result number                                                |
| ---- Format        | string    | Query return format                                          |
| ---- FileName      | string    | Query result set files                                       |
| ---- QueryTime     | int       | Time (ms)                                                    |
| ---- StatusCode    | int       | Status code                                                  |
| ---- DbName        | string    | Database name                                                |

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
			"Sparql":"select ?name where { ?name <dislike> <Eve>. }",
			"Format":"json",
			"RemoteIP":"183.67.4.126",
			"FileName":"140163774674688_20211116145552_847890509.txt",
			"QueryTime":0,
			"AnsNum":2,
			"StatusCode": 0,
			"DbName": "demo"
		}
		......
	]
}
```

**(30) querylogdate**

**Brief description**

- Get the date of gStore's query log (for the date parameter of the querylog interface).

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `querylogdate`                |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password                                                     |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

**Return value**

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| list           | array  | Date list                                                    |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Get query log date success",
	"list":[
		"20220828",
		"20220826",
		"20220825",
		"20220820"
	]
}
```

**(31) accesslog**

**Brief description**

- Get gStore access logs.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `accesslog`                   |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password                                                     |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| date           | yes       | string | Date format is yyyyMMdd                                      |
| pageNo         | yes       | int    | Page number, the value ranges from 1 to N, the default value is 1 |
| pageSize       | yes       | int    | The number of entries per page, the value ranges from 1 to N, the default value is 10 |

**Return value**

| Parameter name  | Type      | Note                                                         |
| --------------- | --------- | ------------------------------------------------------------ |
| StatusCode      | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg       | string    | Return specific information                                  |
| totalSize       | int       | Total size                                                   |
| totalPage       | int       | The total number of pages                                    |
| pageNo          | int       | The current page number                                      |
| pageSize        | int       | The number of entries per page                               |
| list            | JSONArray | Log array                                                    |
| ---- ip         | string    | The IP of the access                                         |
| ---- operation  | string    | Operation type                                               |
| ---- createtime | string    | Operation time                                               |
| ---- code       | string    | Operation results (refer to Appendix: Return value code table for details) |
| ---- msg        | string    | Log descriptions                                             |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Get access log success",
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

**(32) accesslogdate**

**Brief description**

- Get the date of API log (for the date parameter of the accesslog interface).

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL

- POST request

  method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

  method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `accesslogdate`               |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password                                                     |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

**Return value**

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| list           | array  | Date list                                                    |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Get access log date success",
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

**(33) funquery**

**Brief description**

- Show user-defined graph analysis functions.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json.)

**Parameter**

| Parameter name | Mandatory | Type       | Note                                                         |
| -------------- | --------- | ---------- | ------------------------------------------------------------ |
| operation      | yes       | string     | Operation name, fixed value is `funquery`                    |
| username       | yes       | string     | User name                                                    |
| password       | yes       | string     | Password (plain text)                                        |
| encryption     | no        | string     | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| funInfo        | no        | JSONObject | Function information                                         |
| ---- funName   | no        | string     | Function name                                                |
| ---- funStatus | no        | string     | Status (1- to compile; 2- compiled; 3- exception)            |

**Return value**

| Parameter name | Type      | Note                                                         |
| -------------- | --------- | ------------------------------------------------------------ |
| StatusCode     | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string    | Return specific information                                  |
| list           | JSONArray | JSON array (if there is no data, no array is returned)       |
| ---- funName   | string    | Function name                                                |
| ---- funDesc   | string    | Function description                                         |
| ---- funArgs   | string    | Parameter type (1- no K-hop parameter; 2- with K-hop parameter) |
| ---- funBody   | string    | Function content                                             |
| ---- funSubs   | string    | Child functions (can be called from funBody)                 |
| ---- funStatus | string    | Status (1- to compile; 2- compiled; 3- exception)            |
| ---- lastTime  | string    | Last edit time (yyyy-MM-dd HH:mm:ss)                         |

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
			"funBody": "{\nstd::cout<<\"uid=\"<<uid<<endl;\nstd::cout<<\"vid=\"<<vid<<endl;\nstd::cout<<\"k=\"<<k<<endl;\nreturn \"success\";\n}",
			"funSubs": "",
			"funStatus": "1",
			"lastTime": "2022-03-15 11:32:25"
		}
	]
}
```

**5.4.3.34 funcudb**

**Brief description**

- User-defined graph analysis function management (add, modify, delete, compile).

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json.)

Parameter

| Parameter name | Mandatory | Type       | Note                                                         |
| -------------- | --------- | ---------- | ------------------------------------------------------------ |
| operation      | yes       | string     | Operation name, fixed value is `funcudb`                     |
| username       | yes       | string     | User name                                                    |
| password       | yes       | string     | Password (plain text)                                        |
| encryption     | no        | string     | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| type           | yes       | string     | 1: add, 2: modify, 3: delete, 4: compile                     |
| funInfo        | yes       | JSONObject | Function information                                         |
| ---- funName   | yes       | string     | Function name                                                |
| ---- funDesc   | no        | string     | Function description                                         |
| ---- funArgs   | no        | string     | Parameter type (1- no K hop parameter; 2- with K hop parameter). Required for add or modify |
| ---- funBody   | no        | string     | Function contents (wrapped in {}). Required for add or modify |
| ---- funSubs   | no        | string     | Child functions (can be called in funBody)                   |
| ---- funReturn | no        | string     | Return type (`path`: single-path result; `value`: constant value result; `multipath`: multiple-path result). Required for add or modify |

**Return value**

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Function create success"
}
```

**(35) funreview**

**Brief description**

- Preview the user-defined graph analysis function.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request mode**

- POST

**Parameter transfer mode**

- POST request, the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json.)

**Parameter**

| Parameter name | Mandatory | Type       | Note                                                         |
| -------------- | --------- | ---------- | ------------------------------------------------------------ |
| operation      | yes       | string     | Operation name, fixed value is `funreview`                   |
| username       | yes       | string     | User name                                                    |
| password       | yes       | string     | Password (plain text)                                        |
| encryption     | no        | string     | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| funInfo        | yes       | JSONObject | Function information                                         |
| ---- funName   | yes       | string     | Function name                                                |
| ---- funDesc   | no        | string     | Function description                                         |
| ---- funArgs   | no        | string     | Parameter type (1- no K-hop parameter; 2- with K-hop parameter) |
| ---- funBody   | no        | string     | Function contents (wrapped in {})                            |
| ---- funSubs   | no        | string     | Child functions (can be called in funBody)                   |
| ---- funReturn | no        | string     | Return type (`path`: single-path result; `value`: constant value result; `multipath`: multiple-path result) |

**Return value**

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| result         | string | Function source code (decoding is required)                  |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
  "Result":"%23include+%3Ciostream%3E%0A%23include+%22..%2F..%2FDatabase%2FCSRUtil.h%22%0A%0Ausing+..."
}
```

**(36) shutdown**

**Brief description**

- Shut down the gRPC service.

**Request URL**

- `http://127.0.0.1:9000/grpc/shutdown`

**Request mode**

- GET/POST

**Parameter transfer mode**

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest` , passed as ` JSON` 

**Parameter**

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| username       | yes       | string | User name (default user name is system)                      |
| password       | yes       | string | Password（This password need to be viewed in the server's system.db/password[port].txt file. For example, if the port is 9000, then the corresponding password is in password9000.txt) |

**Return value**

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

**Return sample**

```json
{
	"StatusCode": 0,
	"StatusMsg": "Server stopped successfully."
}
```

**(37) upload**

**Brief Description**

- Upload files. Currently, the supported file formats for uploading are nt, ttl, and txt.

**Request URL**

- `http://127.0.0.1:9000/grpc/file/upload` <font color="red">(Note: the address has changed)</font>

**Request Method**

- POST

**Parameter Transfer Method**

- POST request. The `body` in `httprequest` uses `form-data` (Requires RequestHeader parameter Content-Type:multipart/form-data)

**Parameters**

| Parameter Name | Required | Type     | Description                                                 |
| :------------: | :------- | :------- | ----------------------------------------------------------- |
|    username    | Yes      | String   | Username                                                    |
|    password    | Yes      | String   | Password (plaintext)                                        |
|   encryption   | No       | String   | If empty, the password is plaintext; 1 means MD5 encryption |
|      file      | Yes      | Boundary | The binary file stream of the file to be uploaded           |

**Return Values**

| Parameter Name | Type    | Description                                                  |
| :------------: | :------ | ------------------------------------------------------------ |
|   StatusCode   | Integer | Return value code value (please refer to the table of return value codes) |
|   StatusMsg    | String  | Specific information returned                                |
|    filepath    | String  | The relative path address returned after the upload is successful |

**Return Example**

```json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "filepath": "./upload/test_20221101164622.nt"
}
```

**(38) download**

**Brief Description**

- Download files. Currently, only files under the gStore root directory are supported.

**Request URL**

- `http://127.0.0.1:9000/grpc/file/download`  <font color="red">(Note: the address has changed)</font>

**Request Method**

- POST

**Parameter Transfer Method**

- POST request. The parameters are passed in Form表单 format (requires RequestHeader parameter Content-Type:application/x-www-form-urlencoded)

**Parameters**

| Parameter Name | Required | Type   | Description                                                  |
| :------------: | :------- | :----- | ------------------------------------------------------------ |
|    username    | Yes      | String | Username (this username is defaulted to system)              |
|    password    | Yes      | String | Password (plaintext)                                         |
|   encryption   | No       | String | If empty, the password is plaintext; 1 indicates MD5 encryption |
|    filepath    | Yes      | String | The file path of the file to be downloaded (only supports files under the gstore main directory and its subdirectories) |

**Return Values**

- Binary stream response

**Example of Response Headers**

The example of Response Headers is as follows:

```
Content-Range: bytes 0-389/389
Content-Type: application/octet-stream
Date: Tue, 01 Nov 2022 17:21:40 GMT
Content-Length: 389
Connection: Keep-Alive
```

**(39) Rename**

**Brief Description**

- Rename the database.

**Request URL**

- `http://127.0.0.1:9000/grpc/api`

**Request Method**

- GET/POST

**Parameter Transmission Method**

- GET request, parameters transmitted directly in the URL.
- POST request
  - Method 1: parameters transmitted in JSON structure via the `raw` field in the `body` of the `httprequest` (requires RequestHeader parameter Content-Type:application/json)
  - Method 2: parameters transmitted in Form表单 mode (requires RequestHeader parameter Content-Type:application/x-www-form-urlencoded)

**Parameters**

| Parameter Name | Mandatory |  Type  |                    Description                     |
| :------------: | :-------: | :----: | :------------------------------------------------: |
|   operation    |    Yes    | string |     Operation name, fixed value of **rename**      |
|    username    |    Yes    | string |                      Username                      |
|    password    |    Yes    | string |                Password (plaintext)                |
|   encryption   |    No     | string | Empty for plaintext password, 1 for md5 encryption |
|    db_name     |    Yes    | string |                   Database name                    |
|    new_name    |    Yes    | string |                 New database name                  |

**Return Values**

| Parameter Name |  Type  |                         Description                          |
| :------------: | :----: | :----------------------------------------------------------: |
|   StatusCode   |  int   | Return value code value (refer to the table of return value codes) |
|   StatusMsg    | string |                 Return specific information                  |
|   cup_usage    | string |                     CPU usage percentage                     |
|   mem_usage    | string |                    Memory usage (unit MB)                    |
| disk_available | string |                Available disk space (unit MB)                |

**Return Example**

```json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "cup_usage": "10.596026",
    "mem_usage": "2681.507812",
    "disk_available": "12270"
}
```

**(40) stat**

**Brief Description**

- Statistics system resource information

**Request URL**

- 'http://127.0.0.1:9000/ الا/api'

**Request Method**

- GET/POST

**Parameter Passing Methods**

- GET request, parameters are passed directly in the URL
- POST request

Method 1: The parameters are passed in JSON structure through the 'raw' method in the 'body' of the 'httprequest' (requires the RequestHeader parameter Content-Type: application/json)

Method 2: Pass parameters in a Form (requires the RequestHeader parameter Content-Type: application/x-www-form-urlencoded)

**Parameters**

| Parameter name | Required | Type   | Description                                                  |
| :------------- | :------- | :----- | ------------------------------------------------------------ |
| operation      | Yes      | string | Operation name, fixed value is **stat**                      |
| username       | Yes      | string | Username                                                     |
| password       | Yes      | string | Password (clear text)                                        |
| encryption     | No       | string | If it is empty, the password is clear text. If it is 1, it means md5 encryption is used |

**Return Value**

| Parameter name | Type   | Description                                                  |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (please refer to the attached table: Return Value Code Table) |
| StatusMsg      | string | Returns specific information                                 |
| cup_usage      | string | CPU usage ratio                                              |
| mem_usage      | string | Memory usage (unit: MB)                                      |
| disk_available | string | Available disk space (in MB)                                 |

**Return example**

```json
{
" StatusCode": 0,
"StatusMsg": "success",
"cup_usage": "10.596026",
"mem_usage": "2681.507812",
"disk_available": "12270"
}
```

 

**Appendix 1: return value code table**

| Code value | Definition                                 |
| ---------- | ------------------------------------------ |
| 0          | Success                                    |
| 14         | Route not fund                             |
| 1000       | The method type is not supported           |
| 1001       | Authentication Failed                      |
| 1002       | Check Privilege Failed                     |
| 1003       | Param is illegal                           |
| 1004       | The operation conditions are not satisfied |
| 1005       | Operation failed                           |
| 1006       | Add privilege Failed                       |
| 1007       | Loss of lock                               |
| 1008       | Transcation manage Failed                  |
| 1100       | The operation is not defined               |
| 1101       | IP Blocked                                 |



## C++ HTTP API

To use the C++ API, put the phrase '#include "client.h" in your CPP code, as shown below：

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

	std::string monitor(std::string db_name, std::string request_type);
	Function：Displays information for a specific database.
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.monitor("lubm");

**Save the database: checkpoint**

	std::string checkpoint(std::string db_name, std::string request_type);
	Function：If you change the database, save the database 
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.checkpoint("lubm");

**Show the database: show**

	std::string show(std::string request_type);
	Function：Displays all created databases 
	Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.show();

**The kernel version information is displayed: getCoreVersion**

	std::string getCoreVersion(std::string request_type);
	Function：Get kernel version information 
	Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
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
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.checkpoint("lubm");

**Show database：show**

	public String show(String request_type);
	Function：Displays all created databases
	Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.show();

**The kernel version information is displayed：getCoreVersion**

	public String getCoreVersion(String request_type);
	Function：Get kernel version information 
	Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.getCoreVersion();

**Display API version：getAPIVersion**

	public String getAPIVersion(String request_type);
	Function：Get API version 
	Parameter Parameter：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.getAPIVersion();

**Query the database and save the file: fquery**

	public void fquery(String db_name, String format, String sparql, String filename, String request_type);
	Function：Query the database and save the result to a file 
	Parameter definition：[database name], [query result type JSON, HTML or text], [SPARQL statement], [file name]，[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.fquery("lubm", "json", sparql, "ans.txt");

**Export database**

	public String exportDB(String db_name, String dir_path, String request_type);
	Function：Export database to a folder
	Parameter definition：[database name], [database export folder path]，[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.exportDB("lubm", "/root/gStore/");

<!-- <div STYLE="page-break-after: always;"></div> -->
## Python HTTP API

To use the Python API, please refer to the gStore/API/HTTP/Python/SRC/GstoreConnector. Py. Specific use is as follows:

**Construct the initialization function**

	def __init__(self, ip, port, username, password, http_type='ghttp'):
	Function: Initialize
	Parameter definition：[Server IP], [HTTP port on the server], [Username], [password], [HTTP service types "ghttp" and "grpc" can be omitted if the http type is "ghttp"]
	Example：gc = GstoreConnector.GstoreConnector("127.0.0.1", 9000, "root","123456")

**Check service: check**

	def check(self, request_type='GET'):
	Function: check the HTTP service status
	Parameter definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example: res = gc.check()

**login authorization: login**

	def login(self, username, password, request_type='GET'):
	Function: login HTTP API Server. If a message is displayed indicating that "the ip address has been locked" when you access another API, you can unlock the IP address through this interface
	Parameter definition：[username],[password],[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example: res = gc.login("root","123456")

**Build database: build**

	def build(self, db_name, db_path, request_type='GET'):
	Function：Create a new database from an RDF file
	Parameter definition：[Database name], [REF file path], [Request types "GET" and "POST" can be omitted if the request type is "GET"】
	Example：res = gc.build("lubm", "data/lubm/lubm.nt")

**Database deletion: drop**

	def drop(self, db_name, is_backup, request_type='GET'):
	Function：Delete the database directly or delete the database while leaving a backup
	Parameter definition：[database name], [false not backup, true backup], [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.drop("lubm", false)

**Load database: load**

	def load(self, db_name, csr='0', request_type='GET'):
	Function：load the database you have created
	Parameter definition：[database name], ["0" not loading csr resource "1" loading csr resource default "0"] [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.load("lubm")

**Unload database: unload**

	def unload(self, db_name, request_type='GET'):
	Function：Unload database
	Parameter definition：[database name], [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.unload("lubm")

**Display database：show**

	def show(self, request_type='GET'):
	Function: Display the meanings of all created databases
	Parameter definition: [Request types "GET" and "POST" can be omitted if the request type is "GET"] 
	Example：res = gc.show()
	
**Database Monitor ：monitor**   	

	def monitor(self, db_name, request_type='GET'):
	Function：Displays information for a specific database 
	Parameter definition：[Database name], [Request types "GET" and "POST" can be omitted if the request type is "GET"] 
	Example：res = gc.monitor("lubm")
	
**Query Database: query**

	def query(self, db_name, format, sparql, request_type='GET'):
	Function：Query the database
	Parameter definition：[Database name], [Query result type JSON, HTML or text], [SPARQL statement], [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：
	sparql = "select ?x ?p where { ?x ?p <FullProfessor0>. }"
	res = gc.query("lubm", "json", sparql)
	print(res)

**Display users: showUser**

	def showuser(self, request_type='GET'):
	Function：Display all users
	Parameter definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.showUser()

**User management: usermanage**

	def usermanage(self, type, op_username, op_password, equest_type='GET'):
	Function：The root user can add, delete user, or modify the user's password only.
	Parameter definition：["1" adds a user, "2" deletes a user, "3" modify user's password], [username], [password], [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.usermanage("1", "test", "123456")
	
**Modify user's privileges: userprivilegemanage**

	def userprivilegemanage(self, type, op_username, privileges, db_name, request_type='GET'):
	Function：modify user's privileges
	Parameter definition：["1" add privilege, "2" delete privilege, "3" clear privilege], [user name], ["1" query permission, "2" load permission, "3" unload permission, "4" update permission, "5" backup permission, "6" restore permission,  "7" export permission], [database name],[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：
	Add all privileges of "lubm" to user "test"
	res = gc.userprivilegemanage("1", "test", "1,2,3,4,5,6,7", "lubm")
	Remove "backup" "restore" privileges of "lubm" from user "test"
	res = gc.userprivilegemanage("2", "test", "5,7", "lubm")
	Clear all privileges 
	res = gc.userprivilegemanage("3", "test", "", "")

**The kernel version information is displayed: getCoreVersion**

	def getCoreVersion(self, request_type='GET'):
	Function: Get the definition of kernel version
	Parameter definition: [Request types "GET" and "POST" can be omitted if the request type is "GET"] 
	Example：res = gc.getCoreVersion()

**Open transaction operation: begin**

	def begin(self, db_name, isolevel, request_type='GET'):
	Function：open transaction operation 
	Parameter definition：[database name], ["1" seriablizable "2" snapshot isolation "3" read committed], [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.begin("lubm", "1")

**Execute transaction query: fquery**

	def tquery(self, db_name, tid, sparql, request_type='GET'):
	Function：execute transaction query 
	Parameter definition：[database name], [transaction ID], [SPARQL statement], [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.fquery("lubm", "1", sparql)

**commit transaction: commit**

	def commit(self, db_name, tid, request_type='GET'):
	Function: commit transaction 
	Parameter definition：[database name], [transaction ID], [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.commit("lubm", "1")

**rollback transaction: rollback**

	def rollback(self, db_name, tid, request_type='GET'):
	Function: rollback transaction 
	Parameter definition：[database name], [transaction ID], [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.rollback("lubm", "1")

**get transaction log: getTransLog**

	def getTransLog(self, page_no=1, page_size=10, request_type='GET'):
	Function: get transaction log information
	Parameter definition：[database name], [page number], [size per page], [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.getTransLog(1, 10)

**Save database: checkpoint**

	def checkpoint(self, db_name, request_type='GET'):
	Function：If you change the database, save the database 
	Parameter definition: [database name], [request type "GET" and "POST ", if request type "GET" can be omitted] 
	Example：res = gc.checkpoint("lubm")

**Export databse: exportDB**

	def exportDB(self, db_name, db_path, request_type='GET'):
	Function：exprot database to folders
	Parameter definition：[database name], [database export folder path]，[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.exportDB("lubm", "./export")

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
	The defintion of parameters are as follows: [database name], [.nt file path], [Request types "GET" and "POST" can be omitted if the request type is "GET"]
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

	async monitor(db_name = '', request_type);
	Function：Displays information for a specific database 
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：res = gc.monitor("lubm");

**Save database: checkpoint**

	async checkpoint(db_name = '', request_type);
	Function：If you change the database, save the database 
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：res = gc.checkpoint("lubm");

**Display database: show**

	async show(request_type);
	Function：Displays all created databases
	Parameter definition: [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.show();

**Display kernel version information: getCoreVersion**	

	async getCoreVersion(request_type);
	Function：Get kernel version information 
	Parameter definition: [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.getCoreVersion();

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
	参数含义：["Add_query" adds query permission, "delete_query" deletes query permission, "add_load" adds load permission, "delete_load" deletes load permission, "add_unload" adds no load permission, "delete_unload" deletes no load permission,  "Add_update" adds update permission, "delete_update" deletes update permission, "add_backup" adds backup permission, "delete_bakup" deletes backup permission, "add_restore" adds restore permission,  "Delete_restore" deletes restore permission,"add_export" adds export permission," delete_export" deletes export permission], [user name], [database name], [Request types "GET" and "POST" can be omitted if the Request type is "GET"]
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

	function monitor($db_name, $request_type)
	Function：Displays information for a specific database 
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：$res = $gc->monitor("lubm");echo $res. PHP_EOL;

**Save database: checkpoint**

	function checkpoint($db_name, $request_type)
	Function：If you change the database, save the database 
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：$res = $gc->checkpoint("lubm");echo $res. PHP_EOL;

**Display database: show**

	function show($request_type)
	Function：Displays all created databases
	Parameter definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：$res = $gc->show();echo $res. PHP_EOL;

**Display kernel version information: getCoreVersion**

	function getCoreVersion($request_type)
	Function：get kernel version information
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
## gServer API instruction

### API Interconnection Mode

The gServer interface uses the `socket` protocol and supports multiple ways to access the interface. If the port `9000` is started from the gServer in the Main directory, the contents of the interface interconnection are as follows:

API address：

```http
http://ip:9000/
```

The API supports the input of a parameter list in JSON format, as shown below：

```json
{"op": "[op_type]", "[paramname1]": "[paramvalue1]", "[paramname2]": "[paramvalue2]"}
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
|          |                                |                                                         |

<!-- <div STYLE="page-break-after: always;"></div> -->
### API specific instruction

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
| op             | yes       | string | Operation name, fixed value is**build**                      |
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

| Parameter name | Mandatory | Type   | Note                                   |
| :------------- | :-------- | :----- | -------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**load** |
| db_name        | yes       | string | Database name (.db is not required)    |

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

| Parameter name | Mandatory | Type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**unload** |
| db_name        | yes       | string | Database name (.db is not required)      |

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

| Parameter name | Mandatory | Type   | Note                                   |
| :------------- | :-------- | :----- | -------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**drop** |
| db_name        | yes       | string | Database name (.db is not required)    |

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

| Parameter name | Mandatory | Type   | Note                                   |
| :------------- | :-------- | :----- | -------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**show** |

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
| op             | yes       | string | Operation name, fixed value is**query**         |
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

| Parameter name | Mandatory | Type   | Note                                    |
| :------------- | :-------- | :----- | --------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**login** |
| username       | yes       | string | user name                               |
| password       | yes       | string | Password (plain text)                   |

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

| Parameter name | Mandatory | Type   | Note                                   |
| :------------- | :-------- | :----- | -------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**stop** |

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

<!-- <div STYLE="page-break-after: always;"></div> -->
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

| Parameter name | Mandatory | Type   | Note                                    |
| :------------- | :-------- | :----- | --------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**close** |

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

<!-- <div STYLE="page-break-after: always;"></div> -->
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
|            |                                             |



<!-- <div STYLE="page-break-after: always;"></div> -->
<!-- <div STYLE="page-break-after: always;"></div> -->