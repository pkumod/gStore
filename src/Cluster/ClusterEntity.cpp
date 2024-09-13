#include "ClusterEntity.h"
#include "ClusterEntityLeader.h"
#include "ClusterEntityFollower.h"

namespace cluster
{
    bool ClusterEntity::readFromFile(std::string db_name, ClusterLogInfo &logInfo)
    {
        std::lock_guard<std::mutex> lock(log_mutex_);
        ifstream fp;
        fp.open("update.log",ios::in);
        if (!fp.is_open())
        {
            SLOG_ERROR("update.log open fail!");
            return false;
        }
        nlohmann::json rjson;
        fp >> rjson;
        if (!ClusterLogInfo::from_json(rjson, logInfo))
        {
            SLOG_ERROR("json convert fail!");
            fp.close();
            return false;
        }
        fp.close();
        return true;
    }

    bool ClusterEntity::writeToFile(std::string db_name, ClusterLogInfo &logInfo)
    {
        std::lock_guard<std::mutex> lock(log_mutex_);
        nlohmann::json wjson;
        if (!ClusterLogInfo::to_json(wjson, logInfo))
        {
            SLOG_ERROR("json convert fail!");
            return false;
        }
        
        ofstream fp;
        fp.open("update.log",ios::out);
        if (!fp.is_open())
        {
            SLOG_ERROR("update.log open fail!");
            fp.close();
            return false;
        }
        fp << wjson;
        fp.close();
        return true;
    }

    void ClusterEntity::addLog(std::string db_name, uint64 index, int status)
    {
        ClusterLogInfo log;
        if (!readFromFile(db_name, log))
        {
            SLOG_ERROR("add log fail!" << db_name << index << status);
            return;
        }
        log.addLog(index, status);
        if (!writeToFile(db_name, log))
        {
            SLOG_ERROR("add log fail!" << db_name << index << status);
            return;
        }
    }

    void ClusterEntity::updateLogStatus(std::string db_name, uint64 index, int status)
    {
        ClusterLogInfo log;
        if (!readFromFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index << status);
            return;
        }
        log.updateLogStatus(index, status);
        if (!writeToFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index << status);
            return;
        }
    }

    void ClusterEntity::addLogReplyNum(std::string db_name, uint64 index)
    {
        ClusterLogInfo log;
        if (!readFromFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index);
            return;
        }
        log.addLogReplyNum(index);
        if (!writeToFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index);
            return;
        }
    }

    void ClusterEntity::addLogSyncNum(std::string db_name, uint64 index)
    {
        ClusterLogInfo log;
        if (!readFromFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index);
            return;
        }
        log.addLogSyncNum(index);
        if (!writeToFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index);
            return;
        }
    }

    uint32 ClusterEntity::getLogReplyNum(std::string db_name, uint64 index)
    {
        ClusterLogInfo log;
        if (!readFromFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index);
            return 0;
        }
        return log.getLogReplyNum(index);
    }

    uint32 ClusterEntity::getLogSyncNum(std::string db_name, uint64 index)
    {
        ClusterLogInfo log;
        if (!readFromFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index);
            return 0;
        }
        return log.getLogSyncNum(index);
    }
}
