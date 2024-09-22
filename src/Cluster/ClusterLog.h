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
        uint64 nextIndex;
        ClusterLogStatus status;
        ClusterOperation operation;
        std::string fileName;
        std::set<std::string> replyIpPort;
        std::set<std::string> appenEntriesIpPort;
        public:
        LogInfo()
        {
            index     = 0;
            nextIndex = 0;
            status    = ClusterLogStatus_None;
            operation = ClusterOperation_None;
            fileName = "";
            replyIpPort = std::set<std::string>();
            appenEntriesIpPort = std::set<std::string>();
        }
        void setIndex(uint64 value){ index = value; }
        void setNextIndex(uint64 value){ nextIndex = value; }
        void setStatus(ClusterLogStatus value){ status = value; }
        void setOperation(ClusterOperation value){ operation = value; }
        void setFileName(const std::string& value){ fileName = value; }
        uint64 getIndex()const{ return index; }
        ClusterLogStatus getStatus()const{ return status; }
        void setReplyIpPort(const nlohmann::json& s);
        void setAppenEntriesIpPort(const nlohmann::json& s);
        void covertReplyIpsJson(nlohmann::json& s)const;
        void covertAppenEntriesIpsJson(nlohmann::json& s)const;
        void addReplyIpProt(const std::string& value){ replyIpPort.insert(value); }
        void addAppenEntriesIpProt(const std::string& value){ appenEntriesIpPort.insert(value); }
        uint32 getReplyNum()const{ return replyIpPort.size(); }
        uint32 getAppenEntriesNum()const{ return appenEntriesIpPort.size(); }
        ClusterOperation getOperation()const{ return operation; }
        std::string getFileName()const{ return fileName; }
    };

    class ClusterDbNameLogInfo
    {
        std::map<uint64, LogInfo> logs_; //index, logInfo

        public:
        void setLogs(const nlohmann::json& s);
        void covertJson(nlohmann::json& s)const;
        bool addLog(uint64 index, ClusterLogStatus status, ClusterOperation operation, uint64 last_index);
        void updateLogStatus(uint64 index, ClusterLogStatus status);
        void setLogOperation(uint64 index, ClusterOperation operation);
        void setLogFileName(uint64 index, std::string file_name);
        void addLogReplyNum(uint64 index, const std::string& ip_port);
        void addLogSyncNum(uint64 index, const std::string& ip_port);
        uint32 getLogReplyNum(uint64 index)const;
        uint32 getLogSyncNum(uint64 index)const;
        ClusterOperation getOperation(uint64 index)const;
        ClusterLogStatus getStatus(uint64 index)const;
        std::string getFileName(uint64 index)const;

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
        std::string subject;
        std::string predicate;
        std::string object;
        ClusterOperation operation;
        uint64 batch_index;
        public:
        TripleInfo()
        {
            subject = "";
            predicate = "";
            object = "";
            operation = ClusterOperation_None;
            batch_index = 0;
        }
        std::string toString()const
        {
            if (operation == ClusterOperation_None)
                return std::string();
            std::string triple = "";
            if (batch_index == 0)
            {
                triple = subject
                + " " + predicate
                + " " + object
                + " " + std::to_string(operation);
            }
            else
            {
                triple = subject
                + " " + predicate
                + " " + object
                + " " + std::to_string(operation)
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
                subject = triple[0];
                predicate = triple[1];
                object = triple[2];
                operation = (ClusterOperation)atoi(triple[3].c_str());
            }
            else
            {
                subject = triple[0];
                predicate = triple[1];
                object = triple[2];
                operation = (ClusterOperation)atoi(triple[3].c_str());
                batch_index = strtoul(triple[4].c_str(), nullptr, 0);
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