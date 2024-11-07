#include "ClusterLog.h"

namespace cluster
{
    std::string TripleInfo::split_str = "|?<t>?|";
    std::string TripleInfo::getSplitStr()
    {
        return split_str;
    }
    // LogInfo
    void from_json(const nlohmann::json& s, LogInfo& t)
    {
        if (s.contains("index"))
            s.at("index").get_to(t.index);
        if (s.contains("nextIndex"))
            s.at("nextIndex").get_to(t.nextIndex);
        if (s.contains("operation"))
        {
            t.operation = ClusterOperationHandle::to_enum(s.at("operation"));
        }
        if (s.contains("updateType"))
        {
            t.updateType = ClusterUpdateTypeHandle::to_enum(s.at("updateType"));
        }
        if (s.contains("fileName"))
            s.at("fileName").get_to(t.fileName);
        if (s.contains("createTime"))
            s.at("createTime").get_to(t.createTime);
        if (s.contains("commitTime"))
            s.at("commitTime").get_to(t.commitTime);
        if (s.contains("replyIpPort"))
            t.setReplyIpPort(s["replyIpPort"]);
        if (s.contains("appendIpPort"))
            t.setAppenEntriesIpPort(s["appendIpPort"]);
    }

    void to_json(nlohmann::json& s, const LogInfo& t)
    {
        s["index"]      = t.index;
        s["nextIndex"]  = t.nextIndex;
        if (t.operation != ClusterOperation_None)
            s["operation"]  = ClusterOperationHandle::to_str(t.operation);
        if (t.updateType != ClusterUpdateType_None)
            s["updateType"] = ClusterUpdateTypeHandle::to_str(t.updateType);
        if (!t.fileName.empty())
            s["fileName"]   = t.fileName;
        if (!t.createTime.empty())
            s["createTime"] = t.createTime;
        if (!t.commitTime.empty())
            s["commitTime"] = t.commitTime;
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
            appendIpPort.insert(ip_port);
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
        for (const auto& m : appendIpPort)
        {
            s["appendIpPort"][i]["ip_port"] = m;
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

    bool ClusterDbNameLogInfo::addLog(uint64 index, ClusterOperation operation, ClusterUpdateType update_type, uint64 last_index, std::string file_name)
    {
        auto it = logs_.find(index);
        if (it != logs_.end())
        {
            SLOG_ERROR("index is exist, index:" + index);
            return false;
        }
        LogInfo log;
        log.setIndex(index);
        log.setOperation(operation);
        log.setUpdateType(update_type);
        log.setCreateTime(gutil::TimeUtil::now(NORM_DATETIME_PATTERN));
        log.setFileName(file_name);
        logs_[index] = log;

        // set old next index is current index
        if (last_index != 0)
        {
            auto last_it = logs_.find(last_index);
            if (last_it != logs_.end() && last_it->second.getOperation() == ClusterOperation_Commit)
            {
                last_it->second.setNextIndex(index);
            }
        }

        return true;
    }

    void ClusterDbNameLogInfo::updateLogOperation(uint64 index, ClusterOperation operation)
    {
        auto it = logs_.find(index);
        if (it == logs_.end())
        {
            SLOG_ERROR("index is not exist, index:" + index);
            return;
        }
        if (operation == ClusterOperation_Commit)
        {
            it->second.setCommitTime(gutil::TimeUtil::now(NORM_DATETIME_PATTERN));
        }
        it->second.setOperation(operation);
    }

    void ClusterDbNameLogInfo::setLogUpdateType(uint64 index, ClusterUpdateType update_type)
    {
        auto it = logs_.find(index);
        if (it == logs_.end())
        {
            SLOG_ERROR("index is not exist, index:" + index);
            return;
        }
        it->second.setUpdateType(update_type);
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
        if (it->second.getOperation() != ClusterOperation_Prepare)
        {
            SLOG_ERROR("status not support, index:" << index << ", status:" << it->second.getOperation());
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
        if (it->second.getOperation() != ClusterOperation_Append)
        {
            SLOG_ERROR("status not support, index:" << index << ", status:" << it->second.getOperation());
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
        if (it->second.getOperation() != ClusterOperation_Prepare)
        {
            SLOG_ERROR("status not support, index:" << index << ", status:" << it->second.getOperation());
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
        if (it->second.getOperation() != ClusterOperation_Append)
        {
            SLOG_ERROR("status not support, index:" << index << ", status:" << it->second.getOperation());
            return 0;
        }
        return it->second.getAppenEntriesNum();
    }

    uint64 ClusterDbNameLogInfo::getLogNextIndex(uint64 index)const
    {
        auto it = logs_.find(index);
        if (it == logs_.end())
        {
            SLOG_ERROR("index is not exist, index:" << index);
            return 0;
        }
        if (it->second.getOperation() != ClusterOperation_Commit)
        {
            SLOG_ERROR("status not support, index:" << index << ", status:" << it->second.getOperation());
            return 0;
        }
        return it->second.getNextIndex();
    }

    ClusterUpdateType ClusterDbNameLogInfo::getUpdateType(uint64 index)const
    {
        auto it = logs_.find(index);
        if (it == logs_.end())
        {
            SLOG_ERROR("index is not exist, index:" << index);
            return ClusterUpdateType_None;
        }
        return it->second.getUpdateType();
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

    void ClusterDbNameLogInfo::getNextIndexL(uint64 index, std::vector<uint64StringPair>& indexl)const
    {
        uint64 local_index = index;
        while (1)
        {
            auto it = logs_.find(local_index);
            if (it == logs_.end())
                return;
            uint64 next_index = it->second.getNextIndex();
            std::string file_name = it->second.getFileName();
            if (next_index == 0 || file_name.empty() || it->second.getOperation() != ClusterOperation_Commit)
                return;
            indexl.push_back(uint64StringPair(next_index, file_name));
            local_index = next_index;
        }
    }

    // ClusterTermInfo
    // TermDbLog
    void from_json(const nlohmann::json& s, TermDbLog& t)
    {
        if (s.contains("uid"))
            s.at("uid").get_to(t.uid);
        if (s.contains("dbName"))
            s.at("dbName").get_to(t.dbName);
        if (s.contains("index"))
            s.at("index").get_to(t.index);
        if (s.contains("nextIndex"))
            s.at("nextIndex").get_to(t.nextIndex);
        if (s.contains("firstIndex"))
            s.at("firstIndex").get_to(t.firstIndex);
    }

    void to_json(nlohmann::json& s, const TermDbLog& t)
    {
        s["uid"] = t.uid;
        if (!t.dbName.empty())
            s["dbName"]  = t.dbName;
        s["index"] = t.index;
        s["nextIndex"] = t.nextIndex;
        if (t.firstIndex != 0)
            s["firstIndex"] = t.firstIndex;
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
            if (!s[i].contains("dbName"))
                continue;
            std::string db_name = s[i].at("dbName");
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
            TermDbLog log(db_name, index, index, 0, index);
            db_logs_.insert(std::make_pair(db_name, log));
            return;
        }
        it->second.setIndex(index);
        it->second.setFirstIndex(index);
    }

    void ClusterTermInfo::setDbNextIndex(const std::string& db_name, uint64 next_index)
    {
        auto it = db_logs_.find(db_name);
        if (it == db_logs_.end())
        {
            TermDbLog log(db_name, next_index, 0, next_index, 0);
            db_logs_.insert(std::make_pair(db_name, log));
            return;
        }
        it->second.setNextIndex(next_index);
    }

    void ClusterTermInfo::initDbUid(const std::string& db_name, uint64 uid)
    {
        auto it = db_logs_.find(db_name);
        if (it != db_logs_.end())
        {
            db_logs_.erase(it);
        }
        TermDbLog log(db_name, uid, 0, 0, 0);
        db_logs_.insert(std::make_pair(db_name, log));
    }

    void ClusterTermInfo::eraseDb(const std::string& db_name)
    {
        auto it = db_logs_.find(db_name);
        if (it == db_logs_.end())
        {
            return;
        }
        db_logs_.erase(it);
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

    uint64 ClusterTermInfo::getFirstIndex(const std::string& db_name)
    {
        auto it = db_logs_.find(db_name);
        if (it == db_logs_.end())
            return 0;

        return it->second.getFirstIndex();
    }

    TermDbLog ClusterTermInfo::getLog(const std::string& db_name)
    {
        auto it = db_logs_.find(db_name);
        if (it == db_logs_.end())
            return TermDbLog();
        
        return it->second;
    }

    // ClusterTripleArray
    void ClusterTripleArray::addTriple(const TripleInfo& triple)
    {
        triples_.push_back(triple);
    }
}
