#include "ClusterCached.h"

namespace cluster
{
    bool ClusterDb::readFromNtFile(std::vector<TripleInfo>& triples, const std::string &file_name)
    {
        std::lock_guard<std::mutex> lock(cached_nt_mutex_);
        std::string file_path = ClusterManager::getClusterDir() + db_name_ + "/" + file_name;
        ifstream fp;
        fp.open(file_path, ios::in);
        if (!fp.is_open())
        {
            SLOG_ERROR("nt.log open fail, db_name:" << db_name_ << " , file name:" << file_name );
            return false;
        }

        std::string line;
        while (getline(fp, line))
        {
            if (line.empty())
                continue;
            std::vector<std::string> info;
		    Util::split(line, " ", info);
            TripleInfo triple;
            if (triple.convert(info))
                triples.push_back(triple);
            line.clear();
        }
        fp.close();
        return true;
    }

    bool ClusterDb::writeToNtFile(const std::vector<TripleInfo>& triples, const std::string &file_name, bool append)
    {
        std::lock_guard<std::mutex> lock(cached_nt_mutex_);
        std::string file_path = ClusterManager::getClusterDir() + db_name_ + "/" + file_name;
        ofstream fp;
        if (append)
        {
            fp.open(file_path,ios::app);
            if (!fp.is_open())
            {
                SLOG_ERROR("nt.log open fail, db_name:" << db_name_ << " , file name:" << file_name );
                return false;
            }
        }
        else
        {
            fp.open(file_path,ios::out);
            if (!fp.is_open())
            {
                SLOG_ERROR("nt.log open fail, db_name:" << db_name_ << " , file name:" << file_name );
                return false;
            }
        }
        std::string triple;
        for (const auto&m : triples)
        {
            triple = m.toString();
            if (triple.empty())
                continue;
            fp << triple << std::endl;
        }
        fp.close();
        return true;
    }

    // update.log
    bool ClusterDb::readFromUpdateFile(ClusterDbNameLogInfo &logInfo)
    {
        std::lock_guard<std::mutex> lock(update_log_mutex_);
        std::string file_path = ClusterManager::getClusterDir() + db_name_ + "/update.json";
        ifstream fp;
        fp.open(file_path,ios::in);
        if (!fp.is_open())
        {
            SLOG_ERROR("update.log open fail, db_name:" << db_name_ );
            return false;
        }
        nlohmann::json rjson;
        fp >> rjson;
        if (!ClusterDbNameLogInfo::from_json(rjson, logInfo))
        {
            SLOG_ERROR("json convert fail, db_name:" << db_name_);
            fp.close();
            return false;
        }
        fp.close();
        return true;
    }

    bool ClusterDb::writeToUpdateFile(ClusterDbNameLogInfo &logInfo)
    {
        std::lock_guard<std::mutex> lock(update_log_mutex_);
        nlohmann::json wjson;
        if (!ClusterDbNameLogInfo::to_json(wjson, logInfo))
        {
            SLOG_ERROR("json convert fail, db_name:" << db_name_);
            return false;
        }
        
        std::string file_path = ClusterManager::getClusterDir() + db_name_ + "/update.json";
        ofstream fp;
        fp.open(file_path,ios::out);
        if (!fp.is_open())
        {
            SLOG_ERROR("update.log open fail, db_name:" << db_name_);
            return false;
        }
        fp << wjson;
        fp.close();
        return true;
    }

    void ClusterDb::addLog(uint64 index, int status, ClusterOperation operation)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(log))
        {
            SLOG_ERROR("add log fail!" << db_name_ << index << status);
            return;
        }
        log.addLog(index, status, operation);
        if (!writeToUpdateFile(log))
        {
            SLOG_ERROR("add log fail!" << db_name_ << index << status);
            return;
        }
    }

    void ClusterDb::updateLogStatus(uint64 index, int status)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name_ << index << status);
            return;
        }
        log.updateLogStatus(index, status);
        if (!writeToUpdateFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name_ << index << status);
            return;
        }
    }

    void ClusterDb::addLogReplyNum(uint64 index)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name_ << index);
            return;
        }
        log.addLogReplyNum(index);
        if (!writeToUpdateFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name_ << index);
            return;
        }
    }

    void ClusterDb::addLogSyncNum(uint64 index)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name_ << index);
            return;
        }
        log.addLogSyncNum(index);
        if (!writeToUpdateFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name_ << index);
            return;
        }
    }

    uint32 ClusterDb::getLogReplyNum(uint64 index)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name_ << index);
            return 0;
        }
        return log.getLogReplyNum(index);
    }

    uint32 ClusterDb::getLogSyncNum(uint64 index)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name_ << index);
            return 0;
        }
        return log.getLogSyncNum(index);
    }

    void ClusterDb::addCachedNtFile(const std::vector<TripleInfo>& triples, const std::string file_name)
    {
        writeToNtFile(triples, file_name);
    }

    void ClusterDb::appendCachedNtData(const std::vector<TripleInfo>& triples, const std::string file_name)
    {
        writeToNtFile(triples, file_name, true);
    }

    void ClusterDb::getNtFileData(std::vector<TripleInfo>& triples, const std::string& file_name)
    {
        readFromNtFile(triples, file_name);
    }
}