#pragma once
#include "ClusterDefined.h"
#include "../Util/Util.h"
#include "../Api/NlohmanJson.hpp"
#include "ClusterCached.h"

namespace cluster
{
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
        uint64 index; //当前已完成
        uint64 nextIndex; //正在执行或则待完成
        std::string fail_num;
        public:
        TermDbLog()
        {
            db_name = "";
            index   = 0;
            nextIndex = 0;
        }

        TermDbLog(const std::string& db_name_, uint64 index_, uint64 nextIndex_)
        {
            db_name = db_name_;
            index   = index_;
            nextIndex = nextIndex_;
        }
        void setDbName(const std::string& value){ db_name = value; }
        void setIndex(uint64 value){ index = value; }
        void setNextIndex(uint64 value){ nextIndex = value; }
        uint64 getIndex()const{ return index; }
        uint64 getNextIndex()const{ return nextIndex; }
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
        void setDbNextIndex(const std::string& db, uint64 NextIndex);
        uint32 getTerm()const{ return term_; }
        uint64 getDbIndex(const std::string& db_name);
        uint64 getDbNextIndex(const std::string& db_name);

        static bool from_json(const nlohmann::json& s, ClusterTermInfo& t);
        static bool to_json(nlohmann::json& s, const ClusterTermInfo& t);
    };

    struct TripleInfo
    {
        ClusterOperation operation;
        std::string subject;
        std::string predicate;
        std::string object;
        uint64 batch_index;
        public:
        TripleInfo()
        {
            operation = ClusterOperation_None;
            subject = "";
            predicate = "";
            object = "";
            batch_index = 0;
        }
        std::string toString()const
        {
            if (operation == ClusterOperation_None)
                return std::string();
            std::string triple = "";
            if (batch_index == 0)
            {
                triple = std::to_string(operation)
                + " " + subject
                + " " + predicate
                + " " + object;
            }
            else
            {
                triple = std::to_string(operation)
                + " " + subject
                + " " + predicate
                + " " + object
                + " " + std::to_string(batch_index);
            }
            return triple;
        }
        bool convert(const std::vector<std::string>& triple)
        {
            if (triple.size() < 4)
                return false;
            if (triple.size() == 4)
            {
                operation = (ClusterOperation)atoi(triple[0].c_str());
                subject = triple[1];
                predicate = triple[1];
                object = triple[2];
            }
            else
            {
                operation = (ClusterOperation)atoi(triple[0].c_str());
                subject = triple[1];
                predicate = triple[1];
                object = triple[2];
                batch_index = strtoul(triple[3].c_str(), nullptr, 0);
            }
            return true;
        }
    };

    class ClusterTripleArray
    {
        std::vector<TripleInfo> triples_;
        public:
        void addTriple(const TripleInfo& triple);
    };
}