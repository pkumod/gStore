#include "ClusterLog.h"

namespace cluster
{
    // LogInfo
    void from_json(const nlohmann::json& s, LogInfo& t)
    {
        if (s.contains("index"))
            s.at("index").get_to(t.index);
        if (s.contains("status"))
            s.at("status").get_to(t.status);
        if (s.contains("nodeNum"))
            s.at("nodeNum").get_to(t.nodeNum);
        if (s.contains("operation"))
            s.at("operation").get_to(t.operation);
        if (s.contains("file_path"))
            s.at("file_path").get_to(t.file_path);
    }

    void to_json(nlohmann::json& s, const LogInfo& t)
    {
        s["index"]   = t.index;
        s["status"]  = t.status;
        s["nodeNum"] = t.nodeNum;
        s["operation"] = t.operation;
        s["file_path"] = t.file_path;
    }

    // ClusterDbNameLogInfo
    void from_json(const nlohmann::json& s, ClusterDbNameLogInfo& t)
    {
        if (s.contains("logs"))
            t.setLogs(s["logs"]);
    }

    void to_json(nlohmann::json& s, const ClusterDbNameLogInfo& t)
    {
        t.covertJson(s);
    }

    bool ClusterDbNameLogInfo::from_json(const nlohmann::json& s, ClusterDbNameLogInfo& t)
    {
        try
        {
            t = s;
        }
        catch (nlohmann::json::exception& e)
        {
            SLOG_ERROR("update log format is error, message:" << e.what() << ", exception id: " << e.id );
            return false;
        }
        return true;
    }

    bool ClusterDbNameLogInfo::to_json(nlohmann::json& s, const ClusterDbNameLogInfo& t)
    {
        try
        {
            s = t;
        }
        catch (nlohmann::json::exception& e)
        {
            SLOG_ERROR("update log format is error, message:" << e.what() << ", exception id: " << e.id );
            return false;
        }
        return true;
    }

    void ClusterDbNameLogInfo::setLogs(const nlohmann::json& s)
    {
        int size = s.size();
        for (int i = 0; i < size; i++)
        {
            if (!s[i].contains("index"))
                continue;
            uint64 index = s[i].at("index");
            logs_[index] = s[i];
            posL_[i] = index;
        }
    }

    void ClusterDbNameLogInfo::covertJson(nlohmann::json& s)const
    {
        int size = logs_.size();
        for (int i = 0; i < size; i++)
        {
            auto it = posL_.find(i);
            if (it == posL_.end())
            {
                SLOG_ERROR("please checkout logs format is error pos:" + i);
                continue;
            }
            uint64 index = it->second;
            auto log = logs_.find(it->second);
            if (log == logs_.end())
            {
                SLOG_ERROR("please checkout logs format is error, index:" + index);
                continue;
            }
            s["logs"][i] = log->second;
        }
    }

    bool ClusterDbNameLogInfo::addLog(uint64 index, int status, ClusterOperation operation)
    {
        auto it = logs_.find(index);
        if (it != logs_.end())
        {
            SLOG_ERROR("index is exist, index:" + index);
            return false;
        }
        LogInfo log;
        log.setIndex(index);
        log.setStatus(status);
        log.setNodeNum(0);
        log.setOperation(operation);
        posL_[logs_.size()] = index;
        logs_[index] = log;
        return true;
    }

    void ClusterDbNameLogInfo::updateLogStatus(uint64 index, int status)
    {
        auto it = logs_.find(index);
        if (it == logs_.end())
        {
            SLOG_ERROR("index is not exist, index:" + index);
            return;
        }
        it->second.setStatus(status);
        it->second.setNodeNum(0);
    }

    void ClusterDbNameLogInfo::addLogReplyNum(uint64 index)
    {
        auto it = logs_.find(index);
        if (it == logs_.end())
        {
            SLOG_ERROR("index is not exist, index:" << index);
            return;
        }
        if (it->second.getStatus() != ClusterLogStatus_pending)
        {
            SLOG_ERROR("status not support, index:" << index << ", status:" << it->second.getStatus());
            return;
        }
        it->second.addNodeNum();
    }

    void ClusterDbNameLogInfo::addLogSyncNum(uint64 index)
    {
        auto it = logs_.find(index);
        if (it == logs_.end())
        {
            SLOG_ERROR("index is not exist, index:" << index);
            return;
        }
        if (it->second.getStatus() != ClusterLogStatus_sync)
        {
            SLOG_ERROR("status not support, index:" << index << ", status:" << it->second.getStatus());
            return;
        }
        it->second.addNodeNum();
    }

    uint32 ClusterDbNameLogInfo::getLogReplyNum(uint64 index)const
    {
        auto it = logs_.find(index);
        if (it == logs_.end())
        {
            SLOG_ERROR("index is not exist, index:" << index);
            return 0;
        }
        if (it->second.getStatus() != ClusterLogStatus_pending)
        {
            SLOG_ERROR("status not support, index:" << index << ", status:" << it->second.getStatus());
            return 0;
        }
        return it->second.getNodeNum();
    }

    uint32 ClusterDbNameLogInfo::getLogSyncNum(uint64 index)const
    {
        auto it = logs_.find(index);
        if (it == logs_.end())
        {
            SLOG_ERROR("index is not exist, index:" << index);
            return 0;
        }
        if (it->second.getStatus() != ClusterLogStatus_sync)
        {
            SLOG_ERROR("status not support, index:" << index << ", status:" << it->second.getStatus());
            return 0;
        }
        return it->second.getNodeNum();
    }

    // ClusterTermInfo
    // TermDbLog
    void from_json(const nlohmann::json& s, TermDbLog& t)
    {
        if (s.contains("db_name"))
            s.at("db_name").get_to(t.db_name);
        if (s.contains("index"))
            s.at("index").get_to(t.index);
    }

    void to_json(nlohmann::json& s, const TermDbLog& t)
    {
        s["db_name"]  = t.db_name;
        s["index"] = t.index;
    }

    // ClusterTermInfo
    void from_json(const nlohmann::json& s, ClusterTermInfo& t)
    {
        if (s.contains("term"))
            t.setTerm(s["term"]);
        if (s.contains("db_logs"))
            t.setLogs(s["db_logs"]);
    }

    void to_json(nlohmann::json& s, const ClusterTermInfo& t)
    {
        s["term"] = t.getTerm();
        t.covertJson(s);
    }

    void ClusterTermInfo::setLogs(const nlohmann::json& s)
    {
        int size = s.size();
        for (int i = 0; i < size; i++)
        {
            if (!s[i].contains("db_name"))
                continue;
            std::string db_name = s[i].at("db_name");
            db_logs_[db_name] = s[i];
        }
    }

    void ClusterTermInfo::covertJson(nlohmann::json& s)const
    {
        int i = 0;
        for (const auto& m : db_logs_)
        {
            s["db_logs"][i] = m.second;
            i++;
        }
    }

    bool ClusterTermInfo::from_json(const nlohmann::json& s, ClusterTermInfo& t)
    {
        try
        {
            t = s;
        }
        catch (nlohmann::json::exception& e)
        {
            SLOG_ERROR("update log format is error, message:" << e.what() << ", exception id: " << e.id );
            return false;
        }
        return true;
    }

    bool ClusterTermInfo::to_json(nlohmann::json& s, const ClusterTermInfo& t)
    {
        try
        {
            s = t;
        }
        catch (nlohmann::json::exception& e)
        {
            SLOG_ERROR("update log format is error, message:" << e.what() << ", exception id: " << e.id );
            return false;
        }
        return true;
    }

    void ClusterTermInfo::setDbIndex(const std::string& db_name, uint64 index)
    {
        auto it = db_logs_.find(db_name);
        if (it == db_logs_.end())
        {
            TermDbLog log(db_name, index);
            db_logs_.insert(std::make_pair(db_name, log));
            return;
        }
        it->second.setIndex(index);
    }

    uint64 ClusterTermInfo::getDbIndex(const std::string& db_name)
    {
        auto it = db_logs_.find(db_name);
        if (it == db_logs_.end())
            return 0;

        return it->second.getIndex();
    }

    // ClusterTripleArray
    void ClusterTripleArray::addTriple(const TripleInfo& triple)
    {
        triples_.push_back(triple);
    }
}
