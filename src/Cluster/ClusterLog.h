#pragma once
#include "ClusterDefined.h"
#include "../Util/Util.h"
#include "../Api/NlohmanJson.hpp"

namespace cluster
{
    struct TermInfo
    {
        uint32 term;
        uint64 index;
        public:
        TermInfo()
        {
            term  = 0;
            index = 0;
        }
        void setTerm(uint32 value){ term = value; }
        void setIndex(uint64 value){ index = value; }
        uint32 getTerm()const{ return term; }
        uint64 getIndex()const{ return index; }
    };

    struct LogInfo
    {
        uint64 index;
        int status;
        uint32 nodeNum;
        ClusterOperation operation;
        std::string file_path;
        public:
        LogInfo()
        {
            index   = 0;
            status  = 0;
            nodeNum = 0;
            operation = ClusterOperation_None;
            file_path = "";
        }
        void setIndex(uint64 value){ index = value; }
        void setStatus(int value){ status = value; }
        void setNodeNum(uint32 value){ nodeNum = value; }
        void setOperation(ClusterOperation value){ operation = value; }
        void setFilePath(const std::string& value){ file_path = value; }
        uint64 getIndex()const{ return index; }
        int    getStatus()const{ return status; }
        uint32 getNodeNum()const{ return nodeNum; }
        void addNodeNum(){ nodeNum += 1; }
    };

    class ClusterDbNameLogInfo
    {
        std::map<uint64, LogInfo> logs_; //index, logInfo
        std::map<int, uint64> posL_; // pos, index;

        public:
        void setLogs(const nlohmann::json& s);
        void covertJson(nlohmann::json& s)const;
        bool addLog(uint64 index, int status, ClusterOperation operation);
        void updateLogStatus(uint64 index, int status);
        void addLogReplyNum(uint64 index);
        void addLogSyncNum(uint64 index);
        uint32 getLogReplyNum(uint64 index)const;
        uint32 getLogSyncNum(uint64 index)const;
        void updateTerm(uint32 term);
        void updateTermIndex(std::string db_name, uint64 index);

        static bool from_json(const nlohmann::json& s, ClusterDbNameLogInfo& t);
        static bool to_json(nlohmann::json& s, const ClusterDbNameLogInfo& t);
    };

    struct TermDbLog
    {
        std::string db_name;
        uint64 index;
        public:
        TermDbLog()
        {
            db_name = "";
            index   = 0;
        }

        TermDbLog(const std::string& db_name_, uint64 index_)
        {
            db_name = db_name_;
            index   = index_;
        }
        void setDbName(const std::string& value){ db_name = value; }
        void setIndex(uint64 value){ index = value; }
        uint64 getIndex()const{ return index; }
    };

    class ClusterTermInfo
    {
        uint32 term_;
        std::map<std::string, TermDbLog> db_logs_; //db_name, logInfo
        public:
        ClusterTermInfo()
        {
            term_  = 0;
        }
        void covertJson(nlohmann::json& s)const;
        void setLogs(const nlohmann::json& s);
        void setTerm(uint32 value){ term_ = value; }
        void setDbIndex(const std::string& db, uint64 index);
        uint32 getTerm()const{ return term_; }
        uint64 getDbIndex(const std::string& db_name);

        static bool from_json(const nlohmann::json& s, ClusterTermInfo& t);
        static bool to_json(nlohmann::json& s, const ClusterTermInfo& t);
    };
}