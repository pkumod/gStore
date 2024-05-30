**This chapter introduces some useful tricks if you are using gStore to implement applications.**

---

#### Config

If you are using gStore to serve as a SPARQL Endpoint, you had better set the SPARQL_ENDPOINT macro in Util.h.
In addition, all DEBUG macros in Util.h should not be used.
What is more, if you do not use gStore as endpoint, but you do not need to update the database, ONLY_READ macro should be set in Util.h.

---

#### Backup

When running as a HTTP server, gStore has provided a backup function, and you can modify the time interval of backup procedure in Util.
However, this backup utility can not take effect if the whole disk is down.
As a result, you had better use multiple machines or clouds to do other backup procedures by yourself, if you are requiring a high security.

---

#### Query

When running as a HTTP server, gStore has set a time limit for query processing, i.e. 1 hour.
You can modify this parameter in Util as you wish, but we suggest that the lowest bound is 1 minute.

---

#### KVstore

The efficiency of KVstore has huge impact on the performance of the whole system, and you can modify related parameters in KVstore.h, according to your demand and memory capacity.

---

#### String Buffer

To speed up the process of reading Strings from disk when running into getFinalResult() function of the query processing, gStore has provided String Buffers for entities and literals.
You can set this parameters in setStringBuffer() of Database.h according to the memory capacity of your machine.

---

#### HTTP API

If you are using HTTP API, and use Java to visit it, then you must be care for the efficiency(due to the Garbage Collection process in jvm).
We strongly suggest that you read codes in api/http/java carefully, before you write your own program to use the REST API providing by HTTP protocol.

---

