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
        uint32 getTerm()const{ return term; }
        uint64 getStatus()const{ return index; }
    };

    struct LogInfo
    {
        uint64 index;
        int status;
        uint32 nodeNum;
        public:
        LogInfo()
        {
            index   = 0;
            status  = 0;
            nodeNum = 0;
        }
        void setIndex(uint64 value){ index = value; }
        void setStatus(int value){ status = value; }
        void setNodeNum(uint32 value){ nodeNum = value; }
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
        void covertJson(nlohmann::json& s)const;
        bool addLog(uint64 index, uint32 status);
        void updateLogStatus(uint64 index, int status);
        void addLogReplyNum(uint64 index);
        void addLogSyncNum(uint64 index);
        uint32 getLogReplyNum(uint64 index)const;
        uint32 getLogSyncNum(uint64 index)const;

        static void from_json(const nlohmann::json& s, ClusterLogInfo& t);
        static void to_json(nlohmann::json& s, const ClusterLogInfo& t);
    };
}