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

    class ClusterLogInfo
    {
        TermInfo termInfo_;
        std::map<uint64, LogInfo> logs_; //index, logInfo
        std::map<int, uint64> posL_; // pos, index;

        public:
        void setTermInfo(const nlohmann::json& s);
        void setLogs(const nlohmann::json& s);
        void setTerm(uint32 value){ termInfo_.setTerm(value); }
        void setIndex(uint64 value){ termInfo_.setIndex(value); }
        void covertJson(nlohmann::json& s)const;
        bool addLog(uint64 index, int status, ClusterOperation operation);
        void updateLogStatus(uint64 index, int status);
        void addLogReplyNum(uint64 index);
        void addLogSyncNum(uint64 index);
        uint32 getLogReplyNum(uint64 index)const;
        uint32 getLogSyncNum(uint64 index)const;
        void updateTerm(uint32 term);
        void updateTermIndex(std::string db_name, uint64 index);

        static bool from_json(const nlohmann::json& s, ClusterLogInfo& t);
        static bool to_json(nlohmann::json& s, const ClusterLogInfo& t);
    };
}