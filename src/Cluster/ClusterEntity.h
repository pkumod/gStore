#pragma once
#include "ClusterDefined.h"
#include "../Util/Util.h"
#include "../Api/TimerProvider.h"
#include "../Api/NlohmanJson.hpp"
#include "../Api/HttpUtil.h"
#include "ClusterLog.h"
#include "../Api/NlohmanJson.hpp"
#include <map>
#include <thread>

namespace cluster
{
    class ClusterEntity : public std::enable_shared_from_this<ClusterEntity>
    {
        public:
        std::mutex term_mutex_;
        std::map<std::string, ClusterDbPtr> databaseL_; // db_name
        ClusterEntity(){};
        virtual ~ClusterEntity(){}
        bool readFromTermFile(ClusterTermInfo &logInfo);
        bool writeToTermFile(ClusterTermInfo &logInfo);
        ClusterDbPtr findDb(const std::string& db_name);

        // virtual function in here
        public:
        virtual void init() = 0;
        virtual ClusterRoleType getCluterRoleType()const = 0;
        void addLog(std::string db_name, uint64 index, int status, ClusterOperation operation);
        void updateLogStatus(std::string db_name, uint64 index, int status);
        void addLogReplyNum(std::string db_name, uint64 index);
        void addLogSyncNum(std::string db_name, uint64 index);
        uint32 getLogReplyNum(std::string db_name, uint64 index);
        uint32 getLogSyncNum(std::string db_name, uint64 index);
        void updateTerm(uint32 term);
        void updateDbIndex(std::string db_name, uint64 index);
        uint32 getTerm();
        uint64 getDbIndex(const std::string& db_name);
        void addCachedNtFile(const std::vector<TripleInfo>& triples, const std::string& db_name, const std::string file_name);
        void appendCachedNtData(const std::vector<TripleInfo>& triples, const std::string& db_name,  const std::string file_name);
        void getNtFileData(std::vector<TripleInfo>& triples, const std::string& db_name, const std::string& file_name);
    };
}