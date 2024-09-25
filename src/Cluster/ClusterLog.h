/*
 * @Author: hexuejiang
 * @Date: 2024-9-12 14:52:50
 * @LastEditTime: 2024-09-22 14:50:20
 * @LastEditors: hexuejiang 1632802996@qq.com
 * @Description: cluster log

/*
    ClusterTermInfo: term.json
    {
    "db_logs": [
        {
        "db_name": "debug",
        "index": 1,         //finish index
        "nextIndex": 2      // executing or pending
        "firstIndex": 1     // first finish index
        }
    ],
    "term": 1
    }
*/
#pragma once
#include "ClusterDefined.h"
#include "../Util/Util.h"
#include "../Api/NlohmanJson.hpp"
#include "ClusterCached.h"
// #include "ClusterOperation.h"

namespace cluster
{
    struct uint64StringPair
    {
        uint64 parm1;
        std::string parm2;
        // uint64StringPair()=delete;
        uint64StringPair(uint64 parm1_, const std::string& parm2_)
        {
            parm1 = parm1_;
            parm2 = parm2_;
        }
    };
    // update.json
    struct LogInfo
    {
        uint64 index;
        uint64 nextIndex;
        ClusterOperation operation;
        ClusterUpdateType updateType;
        std::string fileName;
        std::set<std::string> replyIpPort;
        std::set<std::string> appendIpPort;
        std::string createTime;
        std::string commitTime;
        public:
        LogInfo()
        {
            index     = 0;
            nextIndex = 0;
            operation    = ClusterOperation_None;
            updateType = ClusterUpdateType_None;
            fileName = "";
            replyIpPort = std::set<std::string>();
            appendIpPort = std::set<std::string>();
            createTime = "";
            commitTime = "";
        }
        void setIndex(uint64 value){ index = value; }
        void setNextIndex(uint64 value){ nextIndex = value; }
        void setOperation(ClusterOperation value){ operation = value; }
        void setUpdateType(ClusterUpdateType value){ updateType = value; }
        void setFileName(const std::string& value){ fileName = value; }
        void setCreateTime(const std::string& value){ createTime = value; }
        void setCommitTime(const std::string& value){ commitTime = value; }
        uint64 getIndex()const{ return index; }
        uint64 getNextIndex()const{ return nextIndex; }
        ClusterOperation getOperation()const{ return operation; }
        void setReplyIpPort(const nlohmann::json& s);
        void setAppenEntriesIpPort(const nlohmann::json& s);
        void covertReplyIpsJson(nlohmann::json& s)const;
        void covertAppenEntriesIpsJson(nlohmann::json& s)const;
        void addReplyIpProt(const std::string& value){ replyIpPort.insert(value); }
        void addAppenEntriesIpProt(const std::string& value){ appendIpPort.insert(value); }
        uint32 getReplyNum()const{ return replyIpPort.size(); }
        uint32 getAppenEntriesNum()const{ return appendIpPort.size(); }
        ClusterUpdateType getUpdateType()const{ return updateType; }
        std::string getFileName()const{ return fileName; }
    };

    class ClusterDbNameLogInfo
    {
        std::map<uint64, LogInfo> logs_; //index, logInfo

        public:
        void setLogs(const nlohmann::json& s);
        void covertJson(nlohmann::json& s)const;
        bool addLog(uint64 index, ClusterOperation status, ClusterUpdateType operation, uint64 last_index);
        void updateLogOperation(uint64 index, ClusterOperation status);
        void setLogUpdateType(uint64 index, ClusterUpdateType operation);
        void setLogFileName(uint64 index, std::string file_name);
        void addLogReplyNum(uint64 index, const std::string& ip_port);
        void addLogSyncNum(uint64 index, const std::string& ip_port);
        uint32 getLogReplyNum(uint64 index)const;
        uint32 getLogSyncNum(uint64 index)const;
        ClusterUpdateType getUpdateType(uint64 index)const;
        ClusterOperation getOperation(uint64 index)const;
        std::string getFileName(uint64 index)const;
        void getNextIndexL(uint64 index, std::vector<uint64StringPair>& indexl)const;

        static bool from_json(const nlohmann::json& s, ClusterDbNameLogInfo& t);
        static bool to_json(nlohmann::json& s, const ClusterDbNameLogInfo& t);
    };

    // term.json
    struct TermDbLog
    {
        std::string db_name;
        uint64 index;
        uint64 nextIndex;
        uint64 firstIndex;
        std::string fail_num;
        public:
        TermDbLog()
        {
            db_name = "";
            index   = 0;
            nextIndex = 0;
            firstIndex = 0;
        }

        TermDbLog(const std::string& db_name_, uint64 index_, uint64 nextIndex_, uint64 firstIndex_)
        {
            db_name = db_name_;
            index   = index_;
            nextIndex = nextIndex_;
            firstIndex = firstIndex_;
        }
        void setDbName(const std::string& value){ db_name = value; }
        void setIndex(uint64 value){ index = value; }
        void setNextIndex(uint64 value){ nextIndex = value; }
        uint64 getIndex()const{ return index; }
        uint64 getNextIndex()const{ return nextIndex; }
        uint64 getFirstIndex()const{ return firstIndex; }
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
        uint64 getFirstIndex(const std::string& db_name);
        void eraseDb(const std::string& db_name);

        static bool from_json(const nlohmann::json& s, ClusterTermInfo& t);
        static bool to_json(nlohmann::json& s, const ClusterTermInfo& t);
    };

    // triple nt info log
    struct TripleInfo
    {
        std::string subject;
        std::string predicate;
        std::string object;
        ClusterUpdateType operation;
        uint64 batch_index;
        public:
        TripleInfo()
        {
            subject = "";
            predicate = "";
            object = "";
            operation = ClusterUpdateType_None;
            batch_index = 0;
        }
        std::string toString()const
        {
            if (operation == ClusterUpdateType_None)
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
                operation = (ClusterUpdateType)atoi(triple[3].c_str());
            }
            else
            {
                subject = triple[0];
                predicate = triple[1];
                object = triple[2];
                operation = (ClusterUpdateType)atoi(triple[3].c_str());
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