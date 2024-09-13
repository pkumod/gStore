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
        ClusterEntity(){};
        virtual ~ClusterEntity(){}
        bool readFromFile(std::string name, ClusterLogInfo &logInfo);
        bool writeToFile(std::string name, ClusterLogInfo &logInfo);

        // virtual function in here
        public:
        virtual void init() = 0;
        virtual ClusterRoleType getCluterRoleType()const = 0;
        virtual void addLog(std::string db_name, uint64 index, int status);
        virtual void updateLogStatus(std::string db_name, uint64 index, int status);
        virtual void addLogReplyNum(std::string db_name, uint64 index);
        virtual void addLogSyncNum(std::string db_name, uint64 index);
        virtual uint32 getLogReplyNum(std::string db_name, uint64 index);
        virtual uint32 getLogSyncNum(std::string db_name, uint64 index);
    };
}