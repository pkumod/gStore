## API Usage

​	

### API instruction

---

gStore provides API services to users through http and Socket services, and its components are ghttp and gServer.

#### HTTP API instruction

We now provide c++, Java, python, PHP, and nodejs apis for GHTTP. Please refer to the examples in `API/HTTP/CPP`, `API/HTTP/Java`, `API/HTTP /python`, `API/HTTP/PHP` and `API/HTTP /nodejs`. To use these examples, make sure you have generated an executable file. ** Next, use the `bin/ ghttp` command to start the ghttp service. ** If you know of a running available GHTTP server and try to connect to it, that's ok too. Then, for c++ and Java code, you need to compile the sample code in the directory `API/HTTP/CPP /example` and `API/HTTP/Java /example`.

**For details on how to start and shut down ghttp, see [development documentation] - [Quick Start] - [http API service]**

**After the API is started, the ghttp access address is as follow:**

```
http://serverip:port/
```

`ServerIP` is the IP address of the gStore server, and `port` is the port on which ghttp is started.

#### Socker API instruction

We now provide c++, Java (python, PHP, nodejs) apis for gserver. See API /socket/ CPP, API /socket/ Java for example code. To use these examples, make sure you have generated an executable file. **Next, use the `bin/gserver -s` command to start the gserver service.**If you know of a running available gServer and try to connect to it, this is also ok. Then, for c++ and Java code, you need to compile the sample code in the directory `API/HTTP/CPP /example` and `API/HTTP/Java /example`.  

**Start and close gServer**

**After the Socket API is started, you can connect through the Socket. The default port of gServer is 9000**

<div STYLE="page-break-after: always;"></div>

### HTTP API framework

---

The gStore HTTP API is placed in the API/HTTP directory of the gStore root directory and contains the following contents:

- gStore/api/http/

  - cpp/ (the C++ API)

    - client.cpp (C++ API's source code)

    - client.h

    - example/ (Example program using C++ API)
      - GET-example.cpp

      - Benchmark.cpp

      - POST-example.cpp

      - Makefile

    - Makefile (Compile and build the lib)

  - java/ (the Java API)

    - client.java

    - lib/

    - src/

      - Makefile

      - jgsc/

        - GstoreConnector.java  (Java API's source code)

    - example/  (A sample program using the Java API)

      - Benckmark.java

            - GETexample.java

      - POSTexample.java

      - Makefile

    - python/ (the Python API)

      - example/  (A sample program using the Python API)

        - Benchmark.py

        - GET-example.py

        - POST-example.py

      - src/ 

        - GstoreConnector.py  (A sample program using the Python API)

    - nodejs/ (the Nodejs API)

      - GstoreConnector.js (Nodejs API's source code)

      - LICENSE

      - package.json

      - README.md

      - example/ (A sample program using the Nodejs API)		

        - POST-example.js

        - GET-example.js		

    - php/ (the Php API)

      - example/  (A sample program using the php API)	

        - Benchmark.php

        - POST-example.php

        - GET-example.php

      - src/

        - GstoreConnector.php  (php API's source code)

- - -

<div STYLE="page-break-after: always;"></div>

### ghttp API instruction

---

#### API Interconnection Mode

> The ghttp interface adopts the `HTTP` protocol and supports multiple ways to access the interface. If the ghttp is started on the port `9000`, the interface interconnection content is as follows
>
> API address：
>
> ```json
> http://ip:9000/
> ```
>
> The interface supports both `GET` and `POST` requests, where `GET` requests place parameters in the URL and `POST` requests place parameters in the `body` request.
>
> 

> **Note: `GET` request parameters contain special characters, such as? , @,& and other characters, you need to use urlencode encoding, especially the `SPARQL` parameter must be encoded**
>
> 

#### API list

| API name                   | Definition                                | Note                                                         |
| -------------------------- | ----------------------------------------- | ------------------------------------------------------------ |
| build                      | build graph database                      | The database file must be locally stored on the server       |
| load                       | load graph database                       | Load the database into memory                                |
| unload                     | unload graph database                     | Unload the database from memory                              |
| monitor                    | monitor graph database                    | Count information about the specified database (such as the number of triples, etc.) |
| drop                       | drop graph database                       | Logical deletion and physical deletion can be performed      |
| show                       | display graph database                    | Display a list of all databases                              |
| usermanage（added）        | user management                           | Add, delete, or modify user information                      |
| showuser                   | display all user list                     | Display a list of all users                                  |
| userprivilegemanage(added) | user privilege management                 | Add, delete, or modify user's privilege information          |
| backup                     | backup database                           | backup database information                                  |
| restore                    | restore database                          | restore database information                                 |
| query                      | query database                            | Including query, delete, and insert                          |
| export                     | export database                           | Export database as NT file                                   |
| login                      | login to database                         | It is used to authenticate user names and passwords          |
| check（rewrite）           | Detect ghttp heartbeat signal             |                                                              |
| init（abandon）            | Initialize system                         | This operation should not be initialized by ghttp            |
| refresh（abandon）         | reload database                           | This can be done by unload+ Load                             |
| parameter（abandon）       | Set parameters for backup                 | All parameters are currently boiled down to the config.ini file and cannot be changed remotely |
| begin                      | Start transaction                         | Transaction starts and needs to be used in conjunction with TQuery |
| tquery                     | Querying the database (with transactions) | Data queries with transaction mode (insert and DELETE only)  |
| commit                     | commit transactions                       | Commit the transaction after it completes                    |
| rollback                   | rollback transaction                      | Roll back the transaction to begin state                     |
| txnlog                     | Obtain transaction log information        | Return transcation log information as json                   |
| checkpoint                 | write data to a disk                      | After an INSERT or delete operation is performed on the database, manually checkpoint is required |
| testConnect                | testing connectivity                      | Used to check whether GHTTP is connected                     |
| getCoreVersion             | get gStore version                        | Get the gStore version number                                |
| batchInsert                | batch insert data                         | Batch insert NT data                                         |
| batchRemove                | batch delete data                         | Batch delete NT data                                         |
| querylog（added）          | Get query log information                 | Query log information is returned as json                    |
| ipmanage（added）          | black/white list management               | Maintains a blacklist and whitelist of IP addresses that access gStore |

<div STYLE="page-break-after: always;"></div>

#### API specific intruction

> The input and output parameters of each interface are specified in this section. Assume that the IP address of the GHTTP server is 127.0.0.1 and the port is 9000

##### build -build datbase

###### Brief description

- Create a database based on existing NT files
- Files must exist on the gStore server

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON `structure

###### Parameter

| parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **build**                     |
| db_name        | yes       | string | Database name (.db is not required)                          |
| db_path        | yes       | string | Database file path (can be an absolute path or a relative path. The relative path uses the gStore installation root directory as a reference directory) |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |

###### Return value

| Parameter name | Tpye   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Import RDF file to database done."
}
```

<div STYLE="page-break-after: always;"></div>

##### check

###### Brief description

- Check whether the GHTTP service is online

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- ###### GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is **check** |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "the ghttp server is running..."
}
```

<div STYLE="page-break-after: always;"></div>

##### load

###### Brief description

- Loading a database into memory is a prerequisite for many operations, such as Query, monitor, and so on.

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- ###### GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                   |
| :------------- | :-------- | :----- | -------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**load** |
| db_name        | yes       | string | Database name (.db is not required)    |
| username       | yes       | string | user name                              |
| password       | yes       | string | Password (plain text)                  |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database loaded successfully."
}
```

<div STYLE="page-break-after: always;"></div>

##### monitor

###### Brief description

- et database statistics (load database first)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                      |
| :------------- | :-------- | :----- | ----------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**monitor** |
| db_name        | yes       | string | Database name (.db is not required)       |
| username       | yes       | string | user name                                 |
| password       | yes       | string | Password (plain text)                     |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| database       | string | database name                                                |
| creator        | string | creator                                                      |
| built_time     | string | create time                                                  |
| triple num     | string | number of triples                                            |
| entity num     | int    | number of entities                                           |
| literal num    | int    | number of characters (attribute value)                       |
| subject num    | int    | number of subjects                                           |
| predicate num  | int    | number of objects                                            |
| connection num | int    | number of connections                                        |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "database": "test_lubm",
    "creator": "root",
    "built_time": "2021-08-27 21:29:46",
    "triple num": "99550",
    "entity num": 28413,
    "literal num": 0,
    "subject num": 14569,
    "predicate num": 17,
    "connection num": 0
}
```

<div STYLE="page-break-after: always;"></div>

##### unload

###### Brief description

- Unload the database from memory (all changes are flushed back to hard disk)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**unload** |
| db_name        | yes       | string | Database name (.db is not required)      |
| username       | yes       | string | user name                                |
| password       | yes       | string | Password (plain text)                    |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database unloaded."
}
```

<div STYLE="page-break-after: always;"></div>

##### drop

###### Brief description

- Delete the database (either logically or physically)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**drop**                       |
| db_name        | yes       | string | Database name (.db is not required)                          |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| is_backup      | no        | string | True: Logical deletion, false: represents physical deletion (default true)，if it's logical deletion, change the file folder to .bak file foder, user can change the folder name to. Db and add the folder to the system database by calling bin/ ginit-db database name |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database test_lubm dropped."
}
```

<div STYLE="page-break-after: always;"></div>

##### show

###### Brief description

- Display all database list

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                   |
| :------------- | :-------- | :----- | -------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**show** |
| username       | yes       | string | user name                              |
| password       | yes       | string | Password (plain text)                  |

###### Return value

| Parameter name      | Type      | Note                                                         |
| :------------------ | :-------- | ------------------------------------------------------------ |
| StatusCode          | int       | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg           | string    | Return specific information                                  |
| ResponseBody        | JSONArray | JSON arrays (each of which is a database information)        |
| -------- database   | string    | database name                                                |
| ---------creator    | string    | creator                                                      |
| ---------built_time | string    | create time                                                  |
| ---------status     | string    | database status                                              |


###### Return sample 

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

<div STYLE="page-break-after: always;"></div>

##### usermanage（added）

###### Brief description

- Manage users (including adding, deleting, and changing users)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**usermanage**                 |
| type           | yes       | string | Operation Type（1：adduser ，2：deleteUser 3：alterUserPassword） |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| op_username    | yes       | string | User name for the operation                                  |
| op_password    | yes       | string | Password of the operation (if the password is to be changed, the password is the password to be changed) (If the operation contains special characters and the get request is adopted, the value must be urlencode-encoded) |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{    "StatusCode": 1004,    "StatusMsg": "username already existed, add user failed."}
```

<div STYLE="page-break-after: always;"></div>

##### showuser

###### Brief description

- Display all user information

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                       |
| :------------- | :-------- | :----- | ------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**showuser** |
| username       | yes       | string | user name                                  |
| password       | yes       | string | Password (plain text)                      |

###### Return value

| Parameter name        | Type      | Note                                                         |
| :-------------------- | :-------- | ------------------------------------------------------------ |
| StatusCode            | int       | Return value code value (refer to attached table: Return value code table for details) |
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


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "success",    "ResponseBody": [        {            "username": "liwenjie",            "password": "shuaige1982",            "query privilege": "",            "update privilege": "",            "load privilege": "",            "unload privilege": "",            "backup privilege": "",            "restore privilege": "",            "export privilege": ""        },        {            "username": "liwenjie2",            "password": "shuaige19888@&",            "query privilege": "lubm,movie,",            "update privilege": "lubm,movie,",            "load privilege": "lubm,movie,",            "unload privilege": "lubm,movie,",            "backup privilege": "lubm,movie,",            "restore privilege": "",            "export privilege": ""        },        {            "username": "root",            "password": "123456",            "query privilege": "all",            "update privilege": "all",            "load privilege": "all",            "unload privilege": "all",            "backup privilege": "all",            "restore privilege": "all",            "export privilege": "all"        }    ]}
```

<div STYLE="page-break-after: always;"></div>

##### userprivilegemanage（added）

###### Brief description

- Manage user permissions (including adding, deleting, and changing users)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**userprivilegemanage**        |
| type           | yes       | string | Operation type（1：add privilege，2：delete privilege 3：clear Privilege ） |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| op_username    | yes       | string | User name for the operation                                  |
| privileges     | no        | string | Permissions to operate on (multiple permissions separated by commas) (can be null for clear Privilege)1:query,2:load,3:unload,4:update,5:backup,6:restore,7:export, you can set multi privileges by using , to split. |
| db_name        | no        | string | The database to operate on (this can be empty if it is clearPrivilege |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "add privilege query successfully. \r\nadd privilege load successfully. \r\nadd privilege unload successfully. \r\nadd privilege update successfully. \r\nadd privilege backup successfully. \r\n"}
```

<div STYLE="page-break-after: always;"></div>

##### backup

###### Brief description

- Back up database

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**backup**                     |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | database that need operaions                                 |
| backup_path    | no        | string | The backup file path can be relative or absolute. The relative path uses the gStore root directory as reference. The default path is the backup directory in the gStore root directory |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| backupfilepath | string | Backup file path (this value can be used as the input parameter value for restore) |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "Database backup successfully.",    "backupfilepath": "testbackup/lubm.db_210828211529"}
```

<div STYLE="page-break-after: always;"></div>

##### restore

###### Brief description

- Restore database

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**restore**                    |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | database that need operations                                |
| backup_path    | no        | string | The full time-stamped path of the backup file (can be a relative path or an absolute path. The relative path is based on the gStore root directory). The default path is the backup directory in the gStore root directory |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "Database restore successfully."}
```

<div STYLE="page-break-after: always;"></div>

##### query

###### Brief description

- query the database

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**query**                      |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | database that need operations                                |
| format         | no        | string | The result set returns in json, HTML, and file format. The default is JSOn |
| sparql         | yes       | string | Sparql statement to execute (SPARQL requires URL encoding if it is a GET request) |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| head           | JSON   | head information                                             |
| results        | JSON   | Result information (see Return Sample for details)           |


###### Return sample 

``` json
{    "head": {        "link": [],        "vars": [            "x"        ]    },    "results": {        "bindings": [            {                "x": {                    "type": "uri",                    "value": "十面埋伏"                }            },            {                "x": {                    "type": "uri",                    "value": "投名状"                }            },            {                "x": {                    "type": "uri",                    "value": "如花"                }            }        ]    },    "StatusCode": 0,    "StatusMsg": "success"}
```

<div STYLE="page-break-after: always;"></div>

##### export

###### Brief description

- export database

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                      |
| :------------- | :-------- | :----- | ----------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**restore** |
| username       | yes       | string | user name                                 |
| password       | yes       | string | Password (plain text)                     |
| db_name        | yes       | string | database that need operations             |
| db_path        | no        | string | Export path (gstore root by default)      |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| filepath       | string | Path for exporting files                                     |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "Export the database successfully.",    "filepath": "export/lubm_210828214603.nt"}
```



<div STYLE="page-break-after: always;"></div>

#####  login

###### Brief description

- Login user (verify username and password)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                    |
| :------------- | :-------- | :----- | --------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**login** |
| username       | yes       | string | user name                               |
| password       | yes       | string | Password (plain text)                   |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{    "StatusCode": 1001,    "StatusMsg": "wrong password."}
```

<div STYLE="page-break-after: always;"></div>

##### begin

###### Brief description

- start transaction

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**begin**                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | Database name                                                |
| isolevel       | yes       | string | Transaction isolation level 1:RC(read committed)  2:SI(snapshot isolation) 3:SR(seriablizable） |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| TID            | string | Transaction ID(this ID is important enough to take as a parameter |


###### Return sample 

``` json
{    "StatusCode": 1001,    "StatusMsg": "wrong password."}
```

<div STYLE="page-break-after: always;"></div>

#####  tquery

###### Brief description

- query the transaction type

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**tquery** |
| username       | yes       | string | User name                                |
| password       | yes       | string | Password (plain text)                    |
| db_name        | yes       | string | Database name                            |
| tid            | yes       | string | Transaction ID                           |
| sparql         | yes       | string | sparql statement                         |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{    "result": "",    "StatusCode": 0,    "StatusMsg": "success"}
```

<div STYLE="page-break-after: always;"></div>

##### commit 

###### Brief description

- submit transaction

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**commit** |
| username       | yes       | string | User name                                |
| password       | yes       | string | Password (plain text)                    |
| db_name        | yes       | string | Database name                            |
| tid            | yes       | string | Transaction ID                           |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "transaction commit success. TID: 1"}
```

 <div STYLE="page-break-after: always;"></div>

##### rollback 

###### Brief description

- Rollback trasnsaction

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                       |
| :------------- | :-------- | :----- | ------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**rollback** |
| username       | yes       | string | User name                                  |
| password       | yes       | string | Password (plain text)                      |
| db_name        | yes       | string | Database name                              |
| tid            | yes       | string | Transaction ID                             |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "transaction rollback success. TID: 2"}
```

<div STYLE="page-break-after: always;"></div>

##### txnlog

###### Brief description

- Get transaction logs (this function only works for root user)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**txnlog** |
| username       | yes       | string | user name                                |
| password       | yes       | string | Password (plain text)                    |

###### Return value

| Parameter name | Type      | Note                                                         |
| :------------- | :-------- | ------------------------------------------------------------ |
| StatusCode     | int       | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string    | Return specific information                                  |
| list           | JSONArray | Log JSON array                                               |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "Get Transaction log success",    "list": [        {            "db_name": "lubm2",            "TID": "1",            "user": "root",            "begin_time": "1630376221590",            "state": "COMMITED",            "end_time": "1630376277349"        },        {            "db_name": "lubm2",            "TID": "2",            "user": "root",            "begin_time": "1630376355226",            "state": "ROLLBACK",            "end_time": "1630376379508"        }    ]}
```



<div STYLE="page-break-after: always;"></div>

##### checkpoint

###### Brief description

- Received Flush data back to hard disk (to make data final)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                         |
| :------------- | :-------- | :----- | -------------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**checkpoint** |
| username       | yes       | string | User name                                    |
| password       | yes       | string | Password (plain text)                        |
| db_name        | yes       | string | Database name                                |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "Database saved successfully."}
```

<div STYLE="page-break-after: always;"></div>

##### testConnect

###### Brief description

- Test whether the server can connect (for workbench)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                          |
| :------------- | :-------- | :----- | --------------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**testConnect** |
| username       | yes       | string | user name                                     |
| password       | yes       | string | Password (plain text)                         |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| CoreVersion    | string | Core version number                                          |
| licensetype    | string | License type (Open Source or Enterprise)                     |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "success",    "CoreVersion": "0.9.1",    "licensetype": "\"opensource\""}
```

<div STYLE="page-break-after: always;"></div>

##### getCoreVersion

###### Brief description

- Get the server version number (for Workbench)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                             |
| :------------- | :-------- | :----- | ------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**getCoreVersion** |
| username       | yes       | string | user name                                        |
| password       | yes       | string | Password (plain text)                            |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| CoreVersion    | string | Core version number                                          |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "success",    "CoreVersion": "0.9.1"}
```

<div STYLE="page-break-after: always;"></div>

#####  batchInsert

###### Brief description

- batch insert data

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**batchInsert**                |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | Database name                                                |
| file           | yes       | string | The data NT file to insert (can be a relative or absolute path) |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| success_num    | string | Number of successful executions                              |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "Batch Insert Data  Successfully.",    "success_num": "25"}
```

<div STYLE="page-break-after: always;"></div>

##### batchRemove

###### Brief description

- batch remove data

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**batchRemove**                |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | Database name                                                |
| file           | yes       | string | Data NT files to be deleted (can be relative or absolute paths) |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| success_num    | int    | Number of successful executions                              |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "Batch Remove Data  Successfully.",    "success_num": "25"}
```

<div STYLE="page-break-after: always;"></div>

##### shutdown

###### Brief description

- close ghttp

###### Request URL

- ` http://127.0.0.1:9000/shutdown `[Note, address change]  


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| username       | yes       | string | user name（default user name is system)                      |
| password       | yes       | string | Password（This password need to be viewed in the server's system.db/password[port].txt file，for example, if the port is 9000，then check password in password9000.txt file) |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
If no value is returned, no information is received by default on success, and an error JSON message is returned on failure
```

<div STYLE="page-break-after: always;"></div>

##### querylog

###### Brief description

- Obtaining query Logs

###### Request URL

`http://127.0.0.1:9000`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**querylog**                   |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| date           | yes       | string | Date format is yyyyMMdd                                      |
| pageNo         | yes       | int    | Page number. The value ranges from 1 to N. The default value is 1 |
| pageSize       | yes       | int    | Number of pages. The value ranges from 1 to N. The default value is 10 |

###### Return value

| Parameter     | Type   | Note                                                         |
| :------------ | :----- | ------------------------------------------------------------ |
| StatusCode    | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg     | string | Return specific information                                  |
| totalSize     | int    | Total number                                                 |
| totalPage     | int    | Total page number                                            |
| pageNo        | int    | Current page number                                          |
| pageSize      | int    | Each page                                                    |
| list          | Array  | Log array                                                    |
| QueryDateTime | string | Query date/time                                              |
| Sparql        | string | SPARQL statement                                             |
| Format        | string | Query return format                                          |
| RemoteIP      | string | Request IP                                                   |
| FileName      | string | Query result set files                                       |
| QueryTime     | int    | Time (ms)                                                    |
| AnsNum        | int    | Result number                                                |

###### Return sample

```json
{	"StatusCode":0,    "StatusMsg":"Get query log success",	"totalSize":64,	"totalPage":13,	"pageNo":2,	"pageSize":5,	"list":[		{			"QueryDateTime":"2021-11-16 14:55:52:90ms:467microseconds",			"Sparql":"select ?name where { ?name <不喜欢> <Eve>. }",			"Format":"json",			"RemoteIP":"183.67.4.126",			"FileName":"140163774674688_20211116145552_847890509.txt",			"QueryTime":0,			"AnsNum":2		}        ......    ]}
```

<div STYLE="page-break-after: always;"></div>

##### ipmanage

###### Brief description

- Blacklist and whitelist management

###### Request URL

`http://127.0.0.1:9000`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

查询黑白名单

| Parameter name | Mandatory | Type   | Note                                       |
| -------------- | --------- | ------ | ------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**ipmanage** |
| username       | yes       | string | User name                                  |
| password       | yes       | string | Password (plain text)                      |
| type           | yes       | string | Operation type, fixed value is**1**        |

保存黑白名单

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



###### Return value

| Parameter    | Type   | Note                                                         |
| :----------- | :----- | ------------------------------------------------------------ |
| StatusCode   | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg    | string | Return specific information                                  |
| ResponseBody | Object | Return data (only for queries)                               |
| whiteIPs     | array  | Whitelist                                                    |
| blackIPs     | array  | Blacklist                                                    |

###### Return sample

```json
// 查询黑白名单返回{    "StatusCode": 0,    "StatusMsg": "success",    "ResponseBody": {        "whiteIPs": [            "127.0.0.1",            "183.67.4.126-183.67.4.128"        ],        "blackIPs": [            "192.168.1.141"        ]    }}// 保存黑白名单返回{    "StatusCode": 0,    "StatusMsg": "success"}
```

<div STYLE="page-break-after: always;"></div>

##### Attached table 1 return value code table

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

<div STYLE="page-break-after: always;"></div>

### C++ HTTP API

---

To use the C++ API, put the phrase '#include "client.h" in your CPP code, as shown below：

**Construct the initialization function**
	

	GstoreConnector(std::string serverIP, int serverPort, std::string username, std::string password);
	Function: Initialize
	Parameter Definition：[Server IP], [GHTTP port on the server], [Username], [password]
	Example：GstoreConnector gc("127.0.0.1", 9000, "root", "123456");

**Build database: build**
	

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



<div STYLE="page-break-after: always;"></div>

###  Java  HTTP API

---

To use the Java API, please refer to the gStore/API/HTTP/Java/SRC/JGSC/GstoreConnector. Java. Specific use is as follows:

**Construct the initialization function**

	public class GstoreConnector(String serverIP, int serverPort, String username, String password);
	Function：Initialize 
	Parameter Definition：[Server IP], [GHTTP port on the server], [Username], [password]
	Example：GstoreConnector gc = new GstoreConnector("127.0.0.1", 9000, "root", "123456");

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

<div STYLE="page-break-after: always;"></div>

### Python HTTP API

---

To use the Python API, please refer to the gStore/API/HTTP/Python/SRC/GstoreConnector. Py. Specific use is as follows:

**Construct the initialization function**

	public class GstoreConnector(self, serverIP, serverPort, username, password):
	Function：Initialization   
	Parameter definition：[Server IP], [GHTTP port on the server], [Username], [password].
	Example：gc =  GstoreConnector.GstoreConnector("127.0.0.1", 9000, "root", "123456")

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

<div STYLE="page-break-after: always;"></div>

### Nodejs  HTTP API

---

Before using the Nodejs API, type `NPM install Request` and `NPM Install request-promise` to add the required modules under the Nodejs folder.

To use Nodejs API, please refer to the `gStore/API/http/Nodejs/GstoreConnector.js`. Specific use is as follows:

**Construct the initialization function**

	class GstoreConnector(ip = '', port, username = '', password = '');
	Function：Initialization   
	Parameter definition：[Server IP], [GHTTP port on the server], [Username], [password]
	Example：gc =  new GstoreConnector("127.0.0.1", 9000, "root", "123456");

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

<div STYLE="page-break-after: always;"></div>

### PHP HTTP API

---

To use the Php API, please refer to the gStore/API/HTTP/Php/SRC/GstoreConnector. Php. Specific use is as follows:

**Construct the initialization function**

	class GstoreConnector($ip, $port, $username, $password)
	Function：Initialization   
	Parameter definition：[Server IP], [GHTTP port on the server], [Username], [password]
	Example：$gc = new GstoreConnector("127.0.0.1", 9000, "root", "123456");

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

<div STYLE="page-break-after: always;"></div>

### gServer API instruction

---

#### API Interconnection Mode

> The gServer interface uses the `socket` protocol and supports multiple ways to access the interface. If the port `9000` is started from the gServer in the Main directory, the contents of the interface interconnection are as follows:
>
> API address：
>
> ```json
> http://ip:9000/
> ```
>
> The API supports the input of a parameter list in JSON format, as shown below：
>
> ```json
> {"op": "[op_type]", "[paramname1]": "[paramvalue1]", "[paramname2]": "[paramvalue2]"……}
> ```
>
> 



#### API List

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

<div STYLE="page-break-after: always;"></div>

#### API specific instruction

> This section describes the input and output parameters of each interface. Assume that the IP address of the gserver is 127.0.0.1 and the port is 9000

##### 5.9.3.1 build- build database

###### Brief description

- Create a database based on existing NT file
- Files must exist on the gStore server

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| op             | yes       | string | Operation name, fixed value is**build**                      |
| db_name        | yes       | string | Database name (.db is not required)                          |
| db_path        | yes       | string | Database file path (can be an absolute path or a relative path. The relative path uses the gStore installation root directory as a reference directory) |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Import RDF file to database done."
}
```

<div STYLE="page-break-after: always;"></div>

##### load

###### Brief description

- To load a database into memory, a load operation is a prerequisite for many operations, such as Query

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                   |
| :------------- | :-------- | :----- | -------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**load** |
| db_name        | yes       | string | Database name (.db is not required)    |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Load database successfully."
}
```

<div STYLE="page-break-after: always;"></div>

##### unload

###### Brief description

- Unmount the database from memory (all changes are flushed back to hard disk)

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**unload** |
| db_name        | yes       | string | Database name (.db is not required)      |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Unload database done."
}
```

<div STYLE="page-break-after: always;"></div>

##### drop

###### Brief description

- Delete the database

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                   |
| :------------- | :-------- | :----- | -------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**drop** |
| db_name        | yes       | string | Database name (.db is not required)    |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Drop database done."
}
```

<div STYLE="page-break-after: always;"></div>

##### show

###### Brief description

- Display all database list

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                   |
| :------------- | :-------- | :----- | -------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**show** |

###### Return value

| Parameter name    | Type      | Note                                                         |
| :---------------- | :-------- | ------------------------------------------------------------ |
| StatusCode        | int       | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg         | string    | Return specific information                                  |
| ResponseBody      | JSONArray | JSON arrays (each of which is a database information0        |
| -------- database | string    | database name                                                |
| ---------status   | string    | database status                                              |


###### Return sample 

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

<div STYLE="page-break-after: always;"></div>

##### query

###### Brief description

- query database

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                            |
| :------------- | :-------- | :----- | ----------------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**query**         |
| db_name        | yes       | string | database that need operations                   |
| format         | no        | string | The result set return format is json by default |
| sparql         | yes       | string | The SPARQL statement to execute                 |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| head           | JSON   | Head information                                             |
| results        | JSON   | Result information (see Return Sample for details)           |


###### Return sample 

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

<div STYLE="page-break-after: always;"></div>

##### login

###### Brief description

- Login user (verify username and password)

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                    |
| :------------- | :-------- | :----- | --------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**login** |
| username       | yes       | string | user name                               |
| password       | yes       | string | Password (plain text)                   |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 1001,
    "StatusMsg": "wrong password."
}
```

<div STYLE="page-break-after: always;"></div>

##### stop

###### Brief description

- Close server

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                   |
| :------------- | :-------- | :----- | -------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**stop** |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Server stopped."
}
```

<div STYLE="page-break-after: always;"></div>

##### close

###### Brief description

- Close the connection to the client

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                    |
| :------------- | :-------- | :----- | --------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**close** |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to attached table: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Connection disconnected."
}
```

<div STYLE="page-break-after: always;"></div>

##### Attached table 1 return value code table

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

