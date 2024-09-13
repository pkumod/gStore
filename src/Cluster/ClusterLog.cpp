#include "ClusterLog.h"

namespace cluster
{
    // TermInfo
    void from_json(const nlohmann::json& s, TermInfo& t)
    {
        s.at("term").get_to(t.term);
        s.at("index").get_to(t.index);
    }

    void to_json(nlohmann::json& s, const TermInfo& t)
    {
        s["term"]  = t.term;
        s["index"] = t.index;
    }

    // LogInfo
    void from_json(const nlohmann::json& s, LogInfo& t)
    {
        s.at("index").get_to(t.index);
        s.at("status").get_to(t.status);
        s.at("nodeNum").get_to(t.nodeNum);
        s.at("operation").get_to(t.operation);
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

    // ClusterLogInfo
    void from_json(const nlohmann::json& s, ClusterLogInfo& t)
    {
        t.setTermInfo(s["termInfo"]);
        t.setLogs(s["logs"]);
    }

    void to_json(nlohmann::json& s, const ClusterLogInfo& t)
    {
        t.covertJson(s);
    }

    bool ClusterLogInfo::from_json(const nlohmann::json& s, ClusterLogInfo& t)
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

    bool ClusterLogInfo::to_json(nlohmann::json& s, const ClusterLogInfo& t)
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

    void ClusterLogInfo::setTermInfo(const nlohmann::json& s)
    {
        termInfo_ = s;
    }

    void ClusterLogInfo::setLogs(const nlohmann::json& s)
    {
        int size = s.size();
        for (int i = 0; i < size; i++)
        {
            uint64 index = s[i].at("index");
            logs_[index] = s[i];
            posL_[i] = index;
        }
    }

    void ClusterLogInfo::covertJson(nlohmann::json& s)const
    {
        s["termInfo"]  = termInfo_;
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

    bool ClusterLogInfo::addLog(uint64 index, int status, ClusterOperation operation)
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
        logs_[index] = log;
    }

    void ClusterLogInfo::updateLogStatus(uint64 index, int status)
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

    void ClusterLogInfo::addLogReplyNum(uint64 index)
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

    void ClusterLogInfo::addLogSyncNum(uint64 index)
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

    uint32 ClusterLogInfo::getLogReplyNum(uint64 index)const
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

    uint32 ClusterLogInfo::getLogSyncNum(uint64 index)const
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

    void ClusterLogInfo::updateTerm(uint32 term)
    {
        termInfo_.setTerm(term);
    }

    void ClusterLogInfo::updateTermIndex(std::string db_name, uint64 index)
    {
        termInfo_.setIndex(index);
    }
}
