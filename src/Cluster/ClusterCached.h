#pragma once
#include "ClusterDefined.h"
#include "../Util/Util.h"
#include "ClusterLog.h"

namespace cluster
{
    class ClusterDb
    {
        std::mutex update_log_mutex_;
        std::mutex cached_nt_mutex_;
        std::string db_name_;
        static std::string cluster_dir_path_;

        public:
        // 获取集群存放目录
        static std::string getClusterDir(){ return cluster_dir_path_; }
        // nt file
        bool readFromNtFile(std::vector<TripleInfo>& triples, const std::string &file_name);
        bool writeToNtFile(const std::vector<TripleInfo>& triples, const std::string &file_name, bool append = false);
        void addCachedNtFile(const std::vector<TripleInfo>& triples, const std::string file_name);
        void appendCachedNtData(const std::vector<TripleInfo>& triples, const std::string file_name);
        void getNtFileData(std::vector<TripleInfo>& triples, const std::string& file_name);

        // update.log
        bool readFromUpdateFile(ClusterDbNameLogInfo &logInfo);
        bool writeToUpdateFile(ClusterDbNameLogInfo &logInfo);
        void addLog(uint64 index, int status, ClusterOperation operation);
        void updateLogStatus(uint64 index, int status);
        void addLogReplyNum(uint64 index);
        void addLogSyncNum(uint64 index);
        uint32 getLogReplyNum(uint64 index);
        uint32 getLogSyncNum(uint64 index);
    };
}