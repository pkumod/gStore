#include "ClusterEntity.h"
#include "ClusterEntityLeader.h"
#include "ClusterEntityFollower.h"

namespace cluster
{
    std::string ClusterEntity::cluster_dir_path_ = "./Cluster/";
    bool ClusterEntity::readFromUpdateFile(std::string db_name, ClusterDbNameLogInfo &logInfo)
    {
        std::lock_guard<std::mutex> lock(log_mutex_);
        std::string file_path = getClusterDir() + db_name + "/update.json";
        ifstream fp;
        fp.open(file_path,ios::in);
        if (!fp.is_open())
        {
            SLOG_ERROR("update.log open fail, db_name:" << db_name );
            return false;
        }
        nlohmann::json rjson;
        fp >> rjson;
        if (!ClusterDbNameLogInfo::from_json(rjson, logInfo))
        {
            SLOG_ERROR("json convert fail, db_name:" << db_name);
            fp.close();
            return false;
        }
        fp.close();
        return true;
    }

    bool ClusterEntity::writeToUpdateFile(std::string db_name, ClusterDbNameLogInfo &logInfo)
    {
        std::lock_guard<std::mutex> lock(log_mutex_);
        nlohmann::json wjson;
        if (!ClusterDbNameLogInfo::to_json(wjson, logInfo))
        {
            SLOG_ERROR("json convert fail, db_name:" << db_name);
            return false;
        }
        
        std::string file_path = getClusterDir() + db_name + "/update.json";
        ofstream fp;
        fp.open(file_path,ios::out);
        if (!fp.is_open())
        {
            SLOG_ERROR("update.log open fail, db_name:" << db_name);
            fp.close();
            return false;
        }
        fp << wjson;
        fp.close();
        return true;
    }

    bool ClusterEntity::readFromTermFile(ClusterTermInfo &logInfo)
    {
        std::lock_guard<std::mutex> lock(log_mutex_);
        std::string file_path = getClusterDir() + "term.json";
        ifstream fp;
        fp.open(file_path,ios::in);
        if (!fp.is_open())
        {
            SLOG_ERROR("term.log open fail!");
            return false;
        }
        nlohmann::json rjson;
        fp >> rjson;
        if (!ClusterTermInfo::from_json(rjson, logInfo))
        {
            SLOG_ERROR("json convert fail!");
            fp.close();
            return false;
        }
        fp.close();
        return true;
    }

    bool ClusterEntity::writeToTermFile(ClusterTermInfo &logInfo)
    {
        std::lock_guard<std::mutex> lock(log_mutex_);
        nlohmann::json wjson;
        if (!ClusterTermInfo::to_json(wjson, logInfo))
        {
            SLOG_ERROR("json convert fail!");
            return false;
        }
        
        std::string file_path = getClusterDir() + "term.json";
        ofstream fp;
        fp.open(file_path,ios::out);
        if (!fp.is_open())
        {
            SLOG_ERROR("term.log open fail!");
            fp.close();
            return false;
        }
        fp << wjson;
        fp.close();
        return true;
    }

    void ClusterEntity::addLog(std::string db_name, uint64 index, int status, ClusterOperation operation)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(db_name, log))
        {
            SLOG_ERROR("add log fail!" << db_name << index << status);
            return;
        }
        log.addLog(index, status, operation);
        if (!writeToUpdateFile(db_name, log))
        {
            SLOG_ERROR("add log fail!" << db_name << index << status);
            return;
        }
    }

    void ClusterEntity::updateLogStatus(std::string db_name, uint64 index, int status)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index << status);
            return;
        }
        log.updateLogStatus(index, status);
        if (!writeToUpdateFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index << status);
            return;
        }
    }

    void ClusterEntity::addLogReplyNum(std::string db_name, uint64 index)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index);
            return;
        }
        log.addLogReplyNum(index);
        if (!writeToUpdateFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index);
            return;
        }
    }

    void ClusterEntity::addLogSyncNum(std::string db_name, uint64 index)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index);
            return;
        }
        log.addLogSyncNum(index);
        if (!writeToUpdateFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index);
            return;
        }
    }

    uint32 ClusterEntity::getLogReplyNum(std::string db_name, uint64 index)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index);
            return 0;
        }
        return log.getLogReplyNum(index);
    }

    uint32 ClusterEntity::getLogSyncNum(std::string db_name, uint64 index)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(db_name, log))
        {
            SLOG_ERROR("update log status fail!" << db_name << index);
            return 0;
        }
        return log.getLogSyncNum(index);
    }

    void ClusterEntity::updateTerm(uint32 term)
    {
        ClusterTermInfo log;
        if (!readFromTermFile(log))
        {
            SLOG_ERROR("update log status fail! ,term:" << term);
            return;
        }
        log.setTerm(term);
        if (!writeToTermFile(log))
        {
            SLOG_ERROR("update log status fail! ,term:" << term);
            return;
        }
    }

    void ClusterEntity::updateDbIndex(std::string db_name, uint64 index)
    {
        ClusterTermInfo log;
        if (!readFromTermFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name << " ,index:" << index);
            return;
        }
        log.setDbIndex(db_name, index);
        if (!writeToTermFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name << " ,index:" << index);
            return;
        }
    }

    uint32 ClusterEntity::getTerm()
    {
        ClusterTermInfo log;
        if (!readFromTermFile(log))
        {
            SLOG_ERROR("update log status fail!");
            return 0;
        }
        return log.getTerm();
    }

    uint64 ClusterEntity::getDbIndex(const std::string& db_name)
    {
        ClusterTermInfo log;
        if (!readFromTermFile(log))
        {
            SLOG_ERROR("update log status fail!");
            return 0;
        }
        return log.getDbIndex(db_name);
    }
}
