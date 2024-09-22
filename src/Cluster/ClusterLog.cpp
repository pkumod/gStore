#include "ClusterLog.h"

namespace cluster
{
    // LogInfo
    void from_json(const nlohmann::json& s, LogInfo& t)
    {
        if (s.contains("index"))
            s.at("index").get_to(t.index);
        if (s.contains("nextIndex"))
            s.at("nextIndex").get_to(t.nextIndex);
        if (s.contains("status"))
            s.at("status").get_to(t.status);
        if (s.contains("operation"))
            s.at("operation").get_to(t.operation);
        if (s.contains("fileName"))
            s.at("fileName").get_to(t.fileName);
        if (s.contains("replyIpPort"))
            t.setReplyIpPort(s["replyIpPort"]);
        if (s.contains("appenEntriesIpPort"))
            t.setAppenEntriesIpPort(s["appenEntriesIpPort"]);
    }

    void to_json(nlohmann::json& s, const LogInfo& t)
    {
        s["index"]     = t.index;
        s["nextIndex"] = t.nextIndex;
        s["status"]    = t.status;
        s["operation"] = t.operation;
        s["fileName"]  = t.fileName;
        t.covertReplyIpsJson(s);
        t.covertAppenEntriesIpsJson(s);
    }


    void LogInfo::setReplyIpPort(const nlohmann::json& s)
    {
        int size = s.size();
        for (int i = 0; i < size; i++)
        {
            if (!s[i].contains("ip_port"))
                continue;
            std::string ip_port = s[i].at("ip_port");
            replyIpPort.insert(ip_port);
        }
    }

    void LogInfo::setAppenEntriesIpPort(const nlohmann::json& s)
    {
        int size = s.size();
        for (int i = 0; i < size; i++)
        {
            if (!s[i].contains("ip_port"))
                continue;
            std::string ip_port = s[i].at("ip_port");
            appenEntriesIpPort.insert(ip_port);
        }
    }

    void LogInfo::covertReplyIpsJson(nlohmann::json& s)const
    {
        int i = 0;
        for (const auto& m : replyIpPort)
        {
            s["replyIpPort"][i]["ip_port"] = m;
            i++;
        }
    }

    void LogInfo::covertAppenEntriesIpsJson(nlohmann::json& s)const
    {
        int i = 0;
        for (const auto& m : appenEntriesIpPort)
        {
            s["appenEntriesIpPort"][i]["ip_port"] = m;
            i++;
        }
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
        }
    }

    void ClusterDbNameLogInfo::covertJson(nlohmann::json& s)const
    {
        int i = 0;
        for (const auto& m : logs_)
        {
            s["logs"][i] = m.second;
            i++;
        }
    }

    bool ClusterDbNameLogInfo::addLog(uint64 index, ClusterLogStatus status, ClusterOperation operation, uint64 last_index)
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
        log.setOperation(operation);
        logs_[index] = log;

        // set old next index is current index
        if (last_index != 0)
        {
             auto last_it = logs_.find(last_index);
            if (last_it != logs_.end())
            {
                last_it->second.setNextIndex(index);
            }
        }

        return true;
    }

    void ClusterDbNameLogInfo::updateLogStatus(uint64 index, ClusterLogStatus status)
    {
        auto it = logs_.find(index);
        if (it == logs_.end())
        {
            SLOG_ERROR("index is not exist, index:" + index);
            return;
        }
        it->second.setStatus(status);
    }

    void ClusterDbNameLogInfo::setLogOperation(uint64 index, ClusterOperation operation)
    {
        auto it = logs_.find(index);
        if (it == logs_.end())
        {
            SLOG_ERROR("index is not exist, index:" + index);
            return;
        }
        it->second.setOperation(operation);
    }

    void ClusterDbNameLogInfo::setLogFileName(uint64 index, std::string file_name)
    {
        auto it = logs_.find(index);
        if (it == logs_.end())
        {
            SLOG_ERROR("index is not exist, index:" + index);
            return;
        }
        it->second.setFileName(file_name);
    }

    void ClusterDbNameLogInfo::addLogReplyNum(uint64 index, const std::string& ip_port)
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
        it->second.addReplyIpProt(ip_port);
    }

    void ClusterDbNameLogInfo::addLogSyncNum(uint64 index, const std::string& ip_port)
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
        it->second.addAppenEntriesIpProt(ip_port);
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
        return it->second.getReplyNum();
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
        return it->second.getAppenEntriesNum();
    }

    ClusterOperation ClusterDbNameLogInfo::getOperation(uint64 index)const
    {
        auto it = logs_.find(index);
        if (it == logs_.end())
        {
            SLOG_ERROR("index is not exist, index:" << index);
            return ClusterOperation_None;
        }
        return it->second.getOperation();
    }

    ClusterLogStatus ClusterDbNameLogInfo::getStatus(uint64 index)const
    {
        auto it = logs_.find(index);
        if (it == logs_.end())
        {
            SLOG_ERROR("index is not exist, index:" << index);
            return ClusterLogStatus_None;
        }
        return it->second.getStatus();
    }

    std::string ClusterDbNameLogInfo::getFileName(uint64 index)const
    {
        auto it = logs_.find(index);
        if (it == logs_.end())
        {
            SLOG_ERROR("index is not exist, index:" << index);
            return std::string();
        }
        return it->second.getFileName();
    }

    // ClusterTermInfo
    // TermDbLog
    void from_json(const nlohmann::json& s, TermDbLog& t)
    {
        if (s.contains("db_name"))
            s.at("db_name").get_to(t.db_name);
        if (s.contains("index"))
            s.at("index").get_to(t.index);
        if (s.contains("nextIndex"))
            s.at("nextIndex").get_to(t.nextIndex);
    }

    void to_json(nlohmann::json& s, const TermDbLog& t)
    {
        s["db_name"]  = t.db_name;
        s["index"] = t.index;
        s["nextIndex"] = t.nextIndex;
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
            TermDbLog log(db_name, index, 0);
            db_logs_.insert(std::make_pair(db_name, log));
            return;
        }
        it->second.setIndex(index);
    }

    void ClusterTermInfo::setDbNextIndex(const std::string& db_name, uint64 next_index)
    {
        auto it = db_logs_.find(db_name);
        if (it == db_logs_.end())
        {
            TermDbLog log(db_name, 0, next_index);
            db_logs_.insert(std::make_pair(db_name, log));
            return;
        }
        it->second.setNextIndex(next_index);
    }

    uint64 ClusterTermInfo::getDbIndex(const std::string& db_name)
    {
        auto it = db_logs_.find(db_name);
        if (it == db_logs_.end())
            return 0;

        return it->second.getIndex();
    }

    uint64 ClusterTermInfo::getDbNextIndex(const std::string& db_name)
    {
        auto it = db_logs_.find(db_name);
        if (it == db_logs_.end())
            return 0;

        return it->second.getNextIndex();
    }

    // ClusterTripleArray
    void ClusterTripleArray::addTriple(const TripleInfo& triple)
    {
        triples_.push_back(triple);
    }
}
