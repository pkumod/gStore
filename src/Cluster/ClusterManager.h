#pragma once
#include "ClusterTypedef.h"
#include "ClusterDefined.h"
#include "ClusterCached.h"
#include "ClusterEntity.h"
#include "ClusterEntityLeader.h"
#include "ClusterEntityFollower.h"
#include "ClusterLog.h"
#include "../Util/Util.h"
#include "ClusterTask.h"

namespace cluster
{
    class ClusterManager
    {
        private:
        bool on_;
        ClusterEntityPtr role_;
        ConcurrenceQueue<ClusterEvent> g_queue;

        public:
        ClusterManager();
        ~ClusterManager();

        // 从配置表读取数据, 初始化主从节点
        void init();

        public:
        // 是否启用集群
        bool isEnable(){ return on_; }
        // 集群身份
        ClusterRoleType getCluterRole(){ return role_->getCluterRoleType(); }
        // 是否是主节点
        bool isLeader();
        // 窘机恢复, 尝试检查所有库
        bool tryRecover(const std::vector<std::string>& dbs);

        //主从互通模块
        // 启动心跳超时检测
        void startHeartBeat();
        // 启动更新通知, 返回应答数量
        int startNotify(std::string db_name, uint32 index);
        // 启动同步通知, 返回应答数量
        int startSync(std::string db_name, uint32 index, ClusterOperation operation, const std::string& file_path);
        // IP是否来自Leader节点
        bool fromLeader(const std::string& ip);
        // IP是否来自Follower节点
        bool fromFollower(const std::string& ip);
        // 获取从节点url列表, http://127.0.0.1:9000
        std::vector<std::string> getFollowrUrlArray();
        // 获取主节点url
        std::string getLeaderUrl();
        // 获取主节点
        ClusterNode getLearrNode();
        // 获取从节点列表
        std::vector<ClusterNode> getFollowNodeL();
        // 添加任务
        void addTask(std::string db_name, uint32 index, ClusterOperation operation, const std::string& file_name, ClusterLogStatus status);

        //日志模块
        //新增日志
        void addLog(std::string db_name, uint64 index, int status, ClusterOperation operation);
        // 更新日志状态
        void updateLogStatus(std::string db_name, uint64 index, int status);
        // 增加响应节点数量
        void addLogReplyNum(std::string db_name, uint64 index);
        // 增加同步节点数量
        void addLogSyncNum(std::string db_name, uint64 index);
        // 获取通知响应数量
        uint32 getLogReplyNum(std::string db_name, uint64 index);
        // 获取同步数量
        uint32 getLogSyncNum(std::string db_name, uint64 index);
        // 更换主节点
        void updateTerm(uint32 term);
        // 更换主节点索引
        void updateDbIndex(std::string db_name, uint64 index);
        // 获取任期
        uint32 getTerm();
        // 获取数据库成功提交的最新日志索引
        void getDbIndex(std::string db_name, uint64 index);

        // nt数据存储模块
        // 普通数据更新，每次操作，单独文件进行存储
        // 事务操作, 一次完整操作, 存储一个文件
        void addCachedNtFile(const std::vector<TripleInfo>& triples, const std::string& db_name, const std::string file_name);
        // 事务操作进行追加
        void appendCachedNtData(const std::vector<TripleInfo>& triples, const std::string& db_name,  const std::string file_name);
        // 存储follower接收到的nt文件, 返回文件路径
        std::string saveFromFollowerFile(const std::pair<std::string, std::string>& file_info, const std::string& db_name);
        // 获取操作文件nt数据
        void getNtFileData(std::vector<TripleInfo>& triples, const std::string& db_name, const std::string& file_name);
    };
}