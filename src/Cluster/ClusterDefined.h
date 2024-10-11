#pragma once
#include <string>
#include "ClusterTypedef.h"
#include "../Util/Util.h"

namespace cluster
{
    enum ClusterRoleType
    {
        ClusterRoleType_Leader = 1,
        ClusterRoleType_Follow = 2,
    };

    enum ClusterUpdateType
    {
        ClusterUpdateType_Defaut = -1,
        ClusterUpdateType_None   = 0,
        ClusterUpdateType_Insert = 1,
        ClusterUpdateType_Delete = 2,
        ClusterUpdateType_Build  = 3, //empty db
    };

    enum ClusterOperation
    {
        ClusterOperation_None      = 0,
        ClusterOperation_HeartBeat = 1,
        ClusterOperation_Compare   = 2,
        ClusterOperation_Prepare   = 3,
        ClusterOperation_Handle    = 4,
        ClusterOperation_Append    = 5,
        ClusterOperation_Commit    = 6,
        ClusterOperation_Cancel    = 7,
        ClusterOperation_Fail      = 8,
        ClusterOperation_Build     = 9,
        ClusterOperation_Drop      = 10,
        ClusterOperation_Recover   = 11,
        // .......
        ClusterOperation_Check     = 50,
        ClusterOperation_Replly    = 51,
        ClusterOperation_Undefine  = 52,
    };

    const std::map<ClusterUpdateType, std::string> ClusterUpdateType_str =
    {
        {ClusterUpdateType_None,   "none"},
        {ClusterUpdateType_Insert, "insert"},
        {ClusterUpdateType_Delete, "delete"},
        {ClusterUpdateType_Build,  "build"},
    };

    const std::map<ClusterOperation, std::string> ClusterOperation_str =
    {
        {ClusterOperation_None,         "none"},
        {ClusterOperation_HeartBeat,    "heartbeat"},
        {ClusterOperation_Compare,      "compare"},
        {ClusterOperation_Prepare,      "prepare"},
        {ClusterOperation_Handle,       "handle"},
        {ClusterOperation_Append,       "append"},
        {ClusterOperation_Commit,       "commit"},
        {ClusterOperation_Cancel,       "cancel"},
        {ClusterOperation_Fail,         "fail"},
        {ClusterOperation_Build,        "build"},
        {ClusterOperation_Drop,         "drop"},
        {ClusterOperation_Recover,      "recover"},
        // ...
        {ClusterOperation_Check,        "check"},
        {ClusterOperation_Replly,       "reply"},
        {ClusterOperation_Undefine,     "undiefine"},
    };

    class ClusterOperationHandle
    {
    public:
        static std::string to_str(const ClusterOperation &type);
        static ClusterOperation to_enum(const std::string &type_str);
    };

    class ClusterUpdateTypeHandle
    {
    public:
        static std::string to_str(const ClusterUpdateType &type);
        static ClusterUpdateType to_enum(const std::string &type_str);
    };

    struct ClusterTaskInfo
    {
        std::string db_name;
        ClusterOperation operation;
        ClusterUpdateType update_type;
        std::string file_name;
        uint64 index;
        uint64 nextIndex;
        uint64 uid;
        ClusterTaskInfo()
        {
            db_name   = "";
            operation    = ClusterOperation_None;
            update_type = ClusterUpdateType_None;
            file_name = "";
            index = 0;
            nextIndex = 0;
            uid = 0;
        }
        ClusterTaskInfo(const std::string& db_name_, ClusterOperation operation_)
        {
            db_name   = db_name_;
            operation    = operation_;
            update_type = ClusterUpdateType_None;
            file_name = "";
            index    = 0;
            nextIndex = 0;
            uid = 0;
        }
        ClusterTaskInfo(const std::string& db_name_, ClusterOperation operation_, ClusterUpdateType update_type_, const std::string& file_name_)
        {
            db_name   = db_name_;
            operation    = operation_;
            update_type = update_type_;
            file_name = file_name_;
            index    = 0;
            nextIndex = 0;
            uid = 0;
        }

        void setIndex(uint64 value){ index = value; }
        void setNextIndex(uint64 value){ nextIndex = value; }
        void setUid(uint64 value){ uid = value; }
    };

    struct ClusterRecoverInfo
    {
        std::string db_name;
        uint64 index;
        std::string ip;
        std::string port;
        ClusterRecoverInfo()
        {
            db_name = "";
            index = 0;
            ip = "";
            port = "";
        }
        ClusterRecoverInfo(const std::string& db_name_, uint64 index_, std::string ip_, std::string port_)
        {
            db_name = db_name_;
            index = index_;
            ip = ip_;
            port = port_;
        }
    };

    struct ClusterNode
    {
        private:
        std::string ip;
        std::string port;
        std::string username;
        std::string password;
        std::string base_url;
        public:
        ClusterNode()
        {
            ip       = "";
            port     = "";
            username = "";
            password = "";
            base_url = "";
        }
        ClusterNode(std::string ip_, std::string port_, std::string username_, std::string password_)
        {
            ip       = ip_;
            port     = port_;
            username = username_;
            password = password_;
            base_url = "http://" + ip_ + ":" + port_;
        }
        void setIp(std::string value){ ip = value; }
        void setPort(std::string value){ port = value; }
        void setUsername(std::string value){ username = value; }
        void setPassword(std::string value){ password = value; }
        void setBaseUrl(){ base_url = "http://" + ip + ":" + port; }
        std::string getIp()const{ return ip; }
        std::string getPort()const{ return port; }
        std::string getUsername()const{ return username; }
        std::string getPassword()const{ return password; }
        std::string getBaseUrl()const{ return base_url; }
        std::string getCheckUrl()const
        {
            return base_url + "/cluster/check";
        }
        std::string getReplyUrl()const
        {
            return base_url + "/cluster/reply";
        }
        std::string getHeartBeatUrl()const
        {
            return base_url + "/cluster/heartbeat";
        }
        std::string getAppendEntriesUrl()const
        {
            return base_url + "/cluster/appendEntries";
        }
        std::string getRecoverlUrl()const
        {
            return base_url + "/cluster/recover";
        }
        bool empty()const{ return ip.empty(); }
    };

    struct ClusterEvent
    {
        virtual void runEvent()const {}
    };
}