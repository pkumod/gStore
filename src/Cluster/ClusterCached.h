#pragma once
#include "ClusterTypedef.h"
#include "../Util/Util.h"
#include "ClusterLog.h"

namespace cluster
{
    class ClusterDb
    {
        std::mutex update_log_mutex_;
        std::mutex cached_nt_mutex_;
        std::string db_name_;

        public:
        ClusterDb(const std::string& db_name)
        {
            db_name_ = db_name;
        };
        void init();
        void setDbname(const std::string& db_name){ db_name_ = db_name; }
        // 获取集群存放目录
        static std::string getClusterDir();
        static std::string getDbDirPath(const std::string& db_name);
        static std::string getUpdatePath(const std::string& db_name);
        // nt file
        bool readFromNtFile(std::vector<TripleInfo>& triples, const std::string &file_name);
        bool writeToNtFile(const std::vector<TripleInfo>& triples, const std::string &file_name, bool append = false);
        std::string readFromNtFilePath(const std::string &file_name);
        void addCachedNtFile(const std::vector<TripleInfo>& triples, const std::string file_name);
        void appendCachedNtData(const std::vector<TripleInfo>& triples, const std::string file_name);
        void getNtFileData(std::vector<TripleInfo>& triples, const std::string& file_name);
        std::string getNtFilePath(const std::string& file_name);

        // update.log
        bool readFromUpdateFile(ClusterDbNameLogInfo &logInfo);
        bool writeToUpdateFile(ClusterDbNameLogInfo &logInfo);
        void addLog(uint64 index, ClusterOperation operation, ClusterUpdateType update_type, uint64 last_index);
        void updateLogOperation(uint64 index, ClusterOperation operation);
        void setLogUpdateType(uint64 index, ClusterUpdateType update_type);
        void setLogFileName(uint64 index, std::string file_name);
        void addLogReplyNum(uint64 index, const std::string& ip_port);
        void addLogSyncNum(uint64 index, const std::string& ip_port);
        uint32 getLogReplyNum(uint64 index);
        uint32 getLogSyncNum(uint64 index);
        ClusterUpdateType getUpdateType(uint64 index);
        ClusterOperation getOperation(uint64 index);
        std::string getFileName(uint64 index);
        void getNextIndexL(uint64 index, std::vector<uint64StringPair>& indexl);
    };
}