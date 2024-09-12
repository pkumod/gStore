#include "ClusterManager.h"

namespace cluster
{
    CluterManager::CluterManager()
    {
        on_   = false;
        log_  = nullptr;
        role_ = nullptr;
    }

    CluterManager::~CluterManager()
    {
    }

    void CluterManager::init()
    {
        string cluster_on = Util::getConfigureValue("cluster_on");
        if (cluster_on == "yes")
        {
            string cluster_role = Util::getConfigureValue("cluster_role");
            if (cluster_role == "leader")
            {
                role_ = std::make_shared<ClusterEntityLeader>();
            }
            else if (cluster_role == "follow")
            {
                role_ = std::make_shared<ClusterEntityFollow>();
            }
            else
            {
                SLOG_ERROR("cluster_role config is error");
                return;
            }
            on_ = true;
            log_ = std::make_shared<ClusterLog>();
            role_->init();
            log_->init();
        }
    }

    bool CluterManager::isLeader()
    {
        if (getCluterRole() == cluster::ClusterRoleType_Leader)
            return true;
        return false;
    }

    uint32 CluterManager::startNotify(uint32 term, uint32 index)
    {
        return role_->startNotify(term, index);
    }

    uint32 CluterManager::startSync(uint32 term, uint32 index, const std::string& file_path)
    {
        return role_->startSync(term, index, file_path);
    }

    void CluterManager::addLog(uint32 term, uint32 index, uint32 status)
    {

    }

    void CluterManager::updateLogStatus(uint32 term, uint32 index, uint32 status)
    {

    }

    void CluterManager::addLogReplyNum(uint32 index, uint32 replyNum)
    {

    }

    void CluterManager::addLogSyncNum(uint32 index, uint32 replyNum)
    {

    }

    uint32 CluterManager::getLogReplyNum(uint32 index)
    {

    }

    uint32 CluterManager::getLogSyncNum(uint32 index)
    {

    }
}