#include "ClusterEntity.h"
#include "ClusterEntityLeader.h"
#include "ClusterEntityFollower.h"

namespace cluster
{
    void ClusterEntity::readFromFile(std::string db_name, ClusterLogInfo &logInfo)
    {
        std::lock_guard<std::mutex> lock(log_mutex_);
        ifstream fp;
        fp.open("update.log",ios::in);
        if (!fp.is_open())
        {
            SLOG_ERROR("update.log open fail!");
            return;
        }
        nlohmann::json rjson;
        ClusterLogInfo::from_json(rjson, logInfo);
        fp.close();
    }

    void ClusterEntity::writeToFile(std::string db_name, ClusterLogInfo &logInfo)
    {
        std::lock_guard<std::mutex> lock(log_mutex_);
        ofstream fp;
        fp.open("update.log",ios::out);
        if (!fp.is_open())
        {
            SLOG_ERROR("update.log open fail!");
            return;
        }
        nlohmann::json wjson;
        ClusterLogInfo::to_json(wjson, logInfo);
        fp << wjson;
        fp.close();
    }

    void ClusterEntity::addLog(std::string db_name, uint64 index, int status)
    {
        ClusterLogInfo log;
        readFromFile(db_name, log);
        log.addLog(index, status);
        writeToFile(db_name, log);
    }

    void ClusterEntity::updateLogStatus(std::string db_name, uint64 index, int status)
    {
        ClusterLogInfo log;
        readFromFile(db_name, log);
        log.updateLogStatus(index, status);
        writeToFile(db_name, log);
    }

    void ClusterEntity::addLogReplyNum(std::string db_name, uint64 index)
    {
        ClusterLogInfo log;
        readFromFile(db_name, log);
        log.addLogReplyNum(index);
        writeToFile(db_name, log);
    }

    void ClusterEntity::addLogSyncNum(std::string db_name, uint64 index)
    {
        ClusterLogInfo log;
        readFromFile(db_name, log);
        log.addLogSyncNum(index);
        writeToFile(db_name, log);
    }

    uint32 ClusterEntity::getLogReplyNum(std::string db_name, uint64 index)
    {
        ClusterLogInfo log;
        readFromFile(db_name, log);
        return log.getLogReplyNum(index);
    }

    uint32 ClusterEntity::getLogSyncNum(std::string db_name, uint64 index)
    {
        ClusterLogInfo log;
        readFromFile(db_name, log);
        return log.getLogSyncNum(index);
    }
}
