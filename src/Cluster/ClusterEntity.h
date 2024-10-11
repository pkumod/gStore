#pragma once
#include "ClusterDefined.h"
#include "ClusterLog.h"
#include "../Api/TimerProvider.h"
#include "../Api/NlohmanJson.hpp"
#include "../Api/HttpUtil.h"
#include <map>
#include <thread>

namespace cluster
{
    class ClusterEntity : public std::enable_shared_from_this<ClusterEntity>
    {
        public:
        uint32 term_;
        std::mutex term_mutex_;
        std::mutex fail_ip_mutex_;
        std::map<std::string, ClusterDbPtr> databaseL_; // db_name
        ClusterEntity(){term_ = 0;}
        virtual ~ClusterEntity(){}
        bool readFromTermFile(ClusterTermInfo &logInfo);
        bool writeToTermFile(ClusterTermInfo &logInfo);
        ClusterDbPtr findDb(const std::string& db_name);
        ClusterDbPtr addClusterDb(const std::string& db_name);

        // update.log
        void buildDb(std::string db_name, uint64 uid);
        void addLog(std::string db_name, uint64 index, ClusterOperation operation, ClusterUpdateType update_type);
        void addCommitLog(std::string db_name, uint64 index, ClusterUpdateType update_type, const std::string& file_name);
        void updateLogOperation(std::string db_name, uint64 index, ClusterOperation operation, ClusterUpdateType update_type = ClusterUpdateType_Defaut, std::string file_name = "");
        void setLogUpdateType(std::string db_name, uint64 index, ClusterUpdateType update_type);
        void setLogFileName(std::string db_name, uint64 index, std::string file_name);
        void addLogReplyNum(std::string db_name, uint64 index, const std::string& ip, const std::string& port);
        void addLogSyncNum(std::string db_name, uint64 index, const std::string& ip, const std::string& port);
        uint32 getLogReplyNum(std::string db_name, uint64 index);
        uint32 getLogSyncNum(std::string db_name, uint64 index);
        ClusterOperation getDbLogOperation(const std::string& db_name, uint64 index);
        ClusterUpdateType getDbLogUpdateType(const std::string& db_name, uint64 index);
        void getDbNextIndexL(const std::string& db_name, uint64 index, std::vector<uint64StringPair>& indexl);
        void dropDb(std::string db_name);
        // nt log
        void addCachedNtFile(const std::vector<TripleInfo>& triples, const std::string& db_name, const std::string file_name);
        void appendCachedNtData(const std::vector<TripleInfo>& triples, const std::string& db_name,  const std::string file_name);
        void getNtFileData(std::vector<TripleInfo>& triples, const std::string& db_name, const std::string& file_name);
        std::string getNtFilePath(const std::string& db_name, const std::string& file_name);
        std::string getNTFilePathByIndex(const std::string& db_name, uint64 index);
        // term.json
        void setInlineTerm(uint32 term){ term_ = term; }
        void updateTerm(uint32 term);
        void updateDbIndex(std::string db_name, uint64 index);
        void updateDbNextIndex(std::string db_name, uint64 next_index);
        uint32 getTerm();
        uint64 getDbIndex(const std::string& db_name);
        uint64 getDbNextIndex(const std::string& db_name);
        uint64 getFirstIndex(const std::string& db_name);
        uint64 getDbNextIndexByindex(const std::string& db_name, uint64 index);
        bool getTermInfoDbLogs(ClusterTermInfo& info);
        TermDbLog getTermInfoDbLog(const std::string& db_name);
        static std::string getIpPort(const std::string& ip, const std::string& port);
        // virtual function in here
        public:
        virtual void init() = 0;
        virtual ClusterRoleType getCluterRoleType()const = 0;
        virtual std::string getLeaderIp()const { return std::string(); }
        virtual std::string getLeaderUrl()const{ return std::string(); }
        virtual ClusterNode getLearrNode()const{ return ClusterNode(); }
        virtual std::vector<std::string> getFollowrUrlArray()const{ return std::vector<std::string>(); }
        virtual std::vector<ClusterNode> getFollowNodeL()const{ return std::vector<ClusterNode>(); }
    };
}