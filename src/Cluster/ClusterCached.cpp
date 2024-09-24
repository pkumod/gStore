#include "ClusterCached.h"

namespace cluster
{
    void ClusterDb::init()
    {
        if (!Util::dir_exist(getDbDirPath()))
        {
            SLOG_TRACE("init db dir, db name:" << db_name_);
            Util::create_dir(getDbDirPath());
        }
    }

    bool ClusterDb::readFromNtFile(std::vector<TripleInfo>& triples, const std::string &file_name)
    {
        std::lock_guard<std::mutex> lock(cached_nt_mutex_);
        std::string file_path = getDbDirPath() + file_name;
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
        std::string file_path = getDbDirPath() + file_name;
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
        std::string file_path = getUpdatePath();
        std::lock_guard<std::mutex> lock(update_log_mutex_);
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
        
        std::string file_path = getUpdatePath();
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

    void ClusterDb::addLog(uint64 index, ClusterLogStatus status, ClusterOperation operation, uint64 last_index)
    {
        ClusterDbNameLogInfo log;
        std::string file_path = getUpdatePath();
        if (!Util::file_exist(file_path))
        {
            log.addLog(index, status, operation, 0);
            SLOG_TRACE("init update log file, db name:" << db_name_ << ", index:" << index << ", status:" << index << " ,operation:" << operation);
            if (!writeToUpdateFile(log))
            {
                SLOG_ERROR("add log fail!" << db_name_ << index << status);
            }
            return;
        }
        if (!readFromUpdateFile(log))
        {
            SLOG_ERROR("add log fail!" << db_name_ << index << status);
            return;
        }
        log.addLog(index, status, operation, last_index);
        if (!writeToUpdateFile(log))
        {
            SLOG_ERROR("add log fail!" << db_name_ << index << status);
            return;
        }
    }

    void ClusterDb::updateLogStatus(uint64 index, ClusterLogStatus status)
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

    void ClusterDb::setLogOperation(uint64 index, ClusterOperation operation)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(log))
        {
            SLOG_ERROR("update log operation fail!" << db_name_ << index << operation);
            return;
        }
        log.setLogOperation(index, operation);
        if (!writeToUpdateFile(log))
        {
            SLOG_ERROR("update log operation fail!" << db_name_ << index << operation);
            return;
        }
    }

    void ClusterDb::setLogFileName(uint64 index, std::string file_name)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(log))
        {
            SLOG_ERROR("update log file_name fail!" << db_name_ << index << file_name);
            return;
        }
        log.setLogFileName(index, file_name);
        if (!writeToUpdateFile(log))
        {
            SLOG_ERROR("update log file_name fail!" << db_name_ << index << file_name);
            return;
        }
    }

    void ClusterDb::addLogReplyNum(uint64 index, const std::string& ip_port)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name_ << index);
            return;
        }
        log.addLogReplyNum(index, ip_port);
        if (!writeToUpdateFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name_ << index);
            return;
        }
    }

    void ClusterDb::addLogSyncNum(uint64 index, const std::string& ip_port)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name_ << index);
            return;
        }
        log.addLogSyncNum(index, ip_port);
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
            SLOG_ERROR("get Log sync num fail!" << db_name_ << index);
            return 0;
        }
        return log.getLogSyncNum(index);
    }

    ClusterOperation ClusterDb::getOperation(uint64 index)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(log))
        {
            SLOG_ERROR("get operation fail!" << db_name_ << index);
            return ClusterOperation_None;
        }
        return log.getOperation(index);
    }

    ClusterLogStatus ClusterDb::getStatus(uint64 index)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name_ << index);
            return ClusterLogStatus_None;
        }
        return log.getStatus(index);
    }

    std::string ClusterDb::getFileName(uint64 index)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name_ << index);
            return std::string();
        }
        return log.getFileName(index);
    }

    void ClusterDb::getNextIndexL(uint64 index, std::vector<uint64StringPair>& indexl)
    {
        ClusterDbNameLogInfo log;
        if (!readFromUpdateFile(log))
        {
            SLOG_ERROR("update log status fail!" << db_name_ << index);
            return;
        }
        return log.getNextIndexL(index, indexl);
    }

    // nt or log
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

    std::string ClusterDb::readFromNtFilePath(const std::string &file_name)
    {
        std::lock_guard<std::mutex> lock(cached_nt_mutex_);
        std::string file_path = getDbDirPath() + file_name;
        std::string::size_type pos = file_name.find_last_of(".");
        if (pos == std::string::npos)
        {
            SLOG_ERROR("nt.log format is error:" << db_name_ << " , file name:" << file_name );
            return std::string();
        }
        std::string nt_path = getDbDirPath() + file_name.substr(0, pos) + ".nt";
        if (Util::file_exist(nt_path))
        {
            return nt_path;
        }

        ifstream r_fp;
        r_fp.open(file_path, ios::in);
        if (!r_fp.is_open())
        {
            SLOG_ERROR("nt.log open fail, db_name:" << db_name_ << " , file name:" << file_name );
            return std::string();
        }

        ofstream w_fp;
        w_fp.open(nt_path,ios::out);
        if (!w_fp.is_open())
        {
            SLOG_ERROR("nt.log open fail, db_name:" << db_name_ << " , file name:" << file_name );
            r_fp.close();
            return std::string();
        }

        std::string line;
        while (getline(r_fp, line))
        {
            if (line.empty())
                continue;
            std::vector<std::string> info;
		    Util::split(line, " ", info);
            TripleInfo triple;
            if (triple.convert(info))
            {

                w_fp << triple.subject << " " << triple.predicate << " " << triple.object << " ." << std::endl;
            }
            line.clear();
        }

        r_fp.close();
        w_fp.close();
        return nt_path;
    }

    std::string ClusterDb::getNtFilePath(const std::string& file_name)
    {
        return readFromNtFilePath(file_name);
    }
}