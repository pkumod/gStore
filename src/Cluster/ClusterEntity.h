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
        std::mutex log_mutex_;
        static std::string cluster_dir_path_;
        ClusterEntity(){};
        virtual ~ClusterEntity(){}
        bool readFromUpdateFile(std::string name, ClusterDbNameLogInfo &logInfo);
        bool writeToUpdateFile(std::string name, ClusterDbNameLogInfo &logInfo);
        bool readFromTermFile(ClusterTermInfo &logInfo);
        bool writeToTermFile(ClusterTermInfo &logInfo);

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
        std::string getClusterDir()const{ return cluster_dir_path_; };
    };
}