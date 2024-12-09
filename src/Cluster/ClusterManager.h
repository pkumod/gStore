/*
 * @Author: hexuejiang
 * @Date: 2024-9-12 14:52:50
 * @LastEditTime: 2024-09-22 14:50:20
 * @LastEditors: hexuejiang 1632802996@qq.com
 * @Description: cluster
 * @FilePath: ClusterManger.h, Cluster management interface, where all methods are defined
 * @FilePath: ClusterEntityLeader.h   -- all leader node interfaces are defined here
 * @FilePath: ClusterEntityFollower.h -- all Follower node interfaces are defined here
 * @FilePath: ClusterLog.h      -- all log info are defined here
 * @FilePath: ClusterCached.h   -- all Save log interfaces are defined here
 */
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

typedef std::function<void(bool)> timeoutCall;

namespace cluster
{
    class ClusterManager
    {
        private:
        bool on_;
        ClusterEntityPtr role_;
        ConcurrenceQueue<ClusterEventPtr> task_queueL;

        public:
        ClusterManager();
        ~ClusterManager();

        // 从配置表读取数据, 初始化主从节点
        void init();

        public:
        // 是否启用集群
        bool isEnable(){ return on_; }
        void refresh();
        // 集群身份
        ClusterRoleType getCluterRole(){ return role_->getCluterRoleType(); }
        // 是否是主节点
        bool isLeader();
        bool isFollower();
        // 窘机恢复, 尝试检查所有库
        bool tryRecover(const std::vector<std::string>& dbs);

        //主从互通模块
        // IP是否来自Leader节点
        bool fromLeader(const std::string& ip);
        // IP是否来自Follower节点
        bool fromFollower(const std::string& ip, const std::string& port);
        bool fromFollowerIp(const std::string& ip);
        // 获取从节点url列表, http://127.0.0.1:9000
        std::vector<std::string> getFollowrUrlArray();
        // 获取主节点url
        std::string getLeaderUrl();
        // 获取主节点
        ClusterNode getLearrNode();
        // 添加初始化数据库
        void addClusterDb(const std::string& db_name);
        // 获取从节点列表
        std::vector<ClusterNode> getFollowNodeL();
        // 添加任务
        bool addTask(ClusterTaskInfo info, bool sync = false);
        // 添加恢复任务任务
        bool addTask(ClusterRecoverInfo info);
        // 启动跑任务
        void runTask();
        // 是否心跳类任务
        bool IsSupportTask(ClusterOperation status);
        // 是否支持同步
        bool IsSupportSync(ClusterOperation status);
        // 删除集群信息
        void dropDb(const std::string& db_name);
        // 获取数据库同步信息
        TermDbLog getTermInfoDbLog(const std::string& db_name);
        // 停掉心跳
        void stopHeart();

        //日志模块
        //新增日志
        void addLog(std::string db_name, uint64 index, ClusterOperation operation, ClusterUpdateType update_type);
        void addCommitLog(std::string db_name, uint64 index, ClusterUpdateType update_type, const std::string& file_name);
        // 初始化建库, 生成集群日志
        void buildDb(std::string db_name, uint64 uid);
        // 更新日志操作
        void updateLogOperation(std::string db_name, uint64 index, ClusterOperation operation);
        void updateLogInfo(std::string db_name, uint64 index, ClusterOperation operation, ClusterUpdateType update_type = ClusterUpdateType_Defaut, std::string file_name = "");
        // 更新日志操作类型
        void setLogUpdateType(std::string db_name, uint64 index, ClusterUpdateType operation);
        // 更新日志操作文件
        void setLogFileName(std::string db_name, uint64 index, std::string file_name);
        // 增加响应节点数量
        void addLogReplyNum(std::string db_name, uint64 index, const std::string& ip, const std::string& port);
        // 增加同步节点数量
        void addLogSyncNum(std::string db_name, uint64 index, const std::string& ip, const std::string& port);
        // 获取通知响应数量
        uint32 getLogReplyNum(std::string db_name, uint64 index);
        // 获取同步数量
        uint32 getLogSyncNum(std::string db_name, uint64 index);
        //是否达到处理要求, 过半(k+1/2)
        bool enabelAttain(std::string db_name, uint64 index, ClusterOperation status);
        // 更换主节点
        void updateTerm(uint32 term);
        // 更新已完成节点索引
        void updateDbIndex(std::string db_name, uint64 index);
        // 更新需要处理的节点索引
        void updateDbNextIndex(std::string db_name, uint64 next_index);
        // 获取任期
        uint32 getTerm();
        // 获取数据库成功提交的最新日志索引
        uint64 getDbIndex(std::string db_name);
        // 获取数据库正在处理的日志索引
        uint64 getDbNextIndex(std::string db_name);
        // 获取nt文件路径
        std::string getNTFilePathByIndex(const std::string& db_name, uint64 index);
        // 获取日志操作状态
        ClusterOperation getDbLogOperation(const std::string& db_name, uint64 index);
        // 获取日志更新类型
        ClusterUpdateType getDbLogUpdateType(const std::string& db_name, uint64 index);
        // 获取数据库路径
        std::string getDbDirPath(std::string db_name);
        // 获取索引后面的索引和索引文件
        void getDbNextIndexL(const std::string& db_name, uint64 index, std::vector<uint64StringPair>& indexl);
        uint64 getDbFirstIndex(const std::string& db_name);
        uint64 getDbNextIndexByIndex(const std::string& db_name, uint64 follower_index);
        // 设置从节点ip(服务名称或则ip地址)
        void setFollowIpPort(const std::string& ip, const std::string& port);
        // 获取从节点ip
        std::string getFollowIp();
        std::string getFollowPort();
        void addRestoreDb(const std::string& db_name);
        void removeRestoreDb(const std::string& db_name);
        // 从节点是否正在恢复数据
        bool isFollowerRestoring(const std::string& db_name);

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
        // 获取操作文件nt数据
        std::string getNtFilePath(const std::string& db_name, const std::string& file_name);
        // 微妙
        uint32 getAppendTimeout(const std::string& db_name, const std::string& file_name);

        // 测试
        // 启动心跳超时检测(比对)
        void startHeartBeatTest();
        // 启动更新通知, 返回应答数量
        bool startNotifyTest(std::string db_name);
        // 启动同步通知, 返回应答数量
        bool startSyncTest(std::string db_name, ClusterUpdateType update_type, const std::string& file_name);
    };

    // task
    struct ClusterTaskEvent : public ClusterEvent
    {
        ClusterTaskInfo info_;
        ClusterEntityLeaderWeaker wer_;
        ClusterTaskEvent(const ClusterTaskInfo& info, ClusterEntityLeaderPtr per)
        {
            info_ = info;
            wer_ = per;
        }

        void runEvent()const override
        {
            ClusterEntityLeaderPtr per = wer_.lock();
            if (!per)
            {
                SLOG_TRACE("ClusterHeartBeatEvent fail, per is free");
                return;
            }
            else
            {
                per->runTask(info_);
            }
        }
    };

    struct ClusterRecoverTaskEvent : public ClusterEvent
    {
        ClusterRecoverInfo info_;
        ClusterEntityLeaderWeaker wer_;
        ClusterRecoverTaskEvent(const ClusterRecoverInfo& info, ClusterEntityLeaderPtr per)
        {
            info_ = info;
            wer_ = per;
        }

        void runEvent()const override
        {
            ClusterEntityLeaderPtr per = wer_.lock();
            if (!per)
            {
                SLOG_TRACE("ClusterRecoverTaskEvent recover fail, per is free");
                return;
            }

            per->runRestoreTask(info_);
        }
    };
}