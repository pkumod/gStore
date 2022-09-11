#ifndef _GRPC_OPERATION_H
#define _GRPC_OPERATION_H

#include <cstdio>
#include <iostream>
#include <string>
#include <map>
#include <vector>

namespace grpc
{
// OP(name,string)
#define OPERATION_TYPES_MAP(OP)                       \
    OP(OP_CHECK,                               check) \
    OP(OP_LOGIN,                               login) \
    OP(OP_TEST_CONNECT,                  testConnect) \
    OP(OP_CORE_VERSION,               getCoreVersion) \
    OP(OP_IP_MANAGE,                        ipmanage) \
    OP(OP_SHOW,                                 show) \
    OP(OP_LOAD,                                 load) \
    OP(OP_UNLOAD,                             unload) \
    OP(OP_MONITOR,                           monitor) \
    OP(OP_BUILD,                               build) \
    OP(OP_DROP,                                 drop) \
    OP(OP_BACKUP,                             backup) \
    OP(OP_RESTORE,                           restore) \
    OP(OP_QUERY,                               query) \
    OP(OP_EXPORT,                             export) \
    OP(OP_BEGIN,                               begin) \
    OP(OP_TQUERY,                             tquery) \
    OP(OP_COMMIT,                             commit) \
    OP(OP_ROLLBACK,                         rollback) \
    OP(OP_CHECKPOINT,                     checkpoint) \
    OP(OP_BATCH_INSERT,                  batchInsert) \
    OP(OP_BATCH_REMOVE,                  batchRemove) \
    OP(OP_USER_MANAGE,                    usermanage) \
    OP(OP_USER_SHOW,                        showuser) \
    OP(OP_USER_PRIVILEGE,        userprivilegemanage) \
    OP(OP_USER_PASSWORD,                userpassword) \
    OP(OP_TXN_LOG,                            txnlog) \
    OP(OP_QUERY_LOG,                        querylog) \
    OP(OP_QUERY_LOG_DATE,               querylogdate) \
    OP(OP_ACCESS_LOG,                      accesslog) \
    OP(OP_ACCESS_LOG_DATE,             accesslogdate) \
    OP(OP_FUN_QUERY,                        funquery) \
    OP(OP_FUN_CUDB,                          funcudb) \
    OP(OP_FUN_REVIEW,                      funreview) \
    OP(OP_SHUTDOWN,                         shutdown)
}

enum operation_type
{
#define OP(name, string) name,
    OPERATION_TYPE_NONE,
    OPERATION_TYPES_MAP(OP)
    OPERATION_TYPE_UNDEFINE
#undef OP
};

class OperationType
{
public:
    static std::string to_str(enum operation_type type);
    static enum operation_type to_enum(const std::string &type_str);
};

std::string OperationType::to_str(enum operation_type type)
{
    switch (type)
    {
#define OP(name, string) case name: return #string;
        OPERATION_TYPES_MAP(OP)
#undef OP
        default:
            return "";
    }
}

enum operation_type OperationType::to_enum(const std::string &type_str)
{
    if (type_str.empty())
    {
        return OPERATION_TYPE_NONE;
    }
#define OP(name, string) \
    if (type_str == #string) { \
        return name; \
    }
    OPERATION_TYPES_MAP(OP)
#undef OP
    return OPERATION_TYPE_UNDEFINE;
}

#endif