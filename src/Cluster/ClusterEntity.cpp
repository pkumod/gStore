#include "ClusterEntity.h"
#include "ClusterEntityLeader.h"
#include "ClusterEntityFollower.h"

namespace cluster
{
    ClusterDbPtr ClusterEntity::addClusterDb(const std::string& db_name)
    {
        auto it = databaseL_.find(db_name);
        if (it != databaseL_.end())
        {
            SLOG_CORE("cluster db is exit, not repeated add, db name:" << db_name);
            return nullptr;
        }
        ClusterDbPtr db = std::make_shared<ClusterDb>(db_name);
        db->init();
        databaseL_.insert(std::make_pair(db_name, db));
        return db;
    }
    bool ClusterEntity::readFromTermFile(ClusterTermInfo &logInfo)
    {
        std::string file_path = ClusterDb::getClusterDir() + "term.json";
        if (!Util::file_exist(file_path))
        {
            SLOG_TRACE("init term log file");
            logInfo.setTerm(1);
            return writeToTermFile(logInfo);
        }
        std::lock_guard<std::mutex> lock(term_mutex_);
        ifstream fp;
        fp.open(file_path,ios::in);
        if (!fp.is_open())
        {
            SLOG_ERROR("term.log open fail!");
            return false;
        }
        nlohmann::json rjson;
        fp >> rjson;
        if (!ClusterTermInfo::from_json(rjson, logInfo))
        {
            SLOG_ERROR("json convert fail!");
            fp.close();
            return false;
        }
        fp.close();
        return true;
    }

    bool ClusterEntity::writeToTermFile(ClusterTermInfo &logInfo)
    {
        std::lock_guard<std::mutex> lock(term_mutex_);
        nlohmann::json wjson;
        if (!ClusterTermInfo::to_json(wjson, logInfo))
        {
            SLOG_ERROR("json convert fail!");
            return false;
        }
        
        std::string file_path = ClusterDb::getClusterDir() + "term.json";
        ofstream fp;
        fp.open(file_path,ios::out);
        if (!fp.is_open())
        {
            SLOG_ERROR("term.log open fail!");
            return false;
        }
        fp << wjson;
        fp.close();
        return true;
    }

    ClusterDbPtr ClusterEntity::findDb(const std::string& db_name)
    {
        auto it = databaseL_.find(db_name);
        if (it == databaseL_.end())
        {
            SLOG_ERROR("db is not exist, db name:" << db_name);
            return nullptr;
        }
        return it->second;
    }

    void ClusterEntity::addLog(std::string db_name, uint64 index, int status, ClusterOperation operation)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return;
        db->addLog(index, status, operation);
    }

    void ClusterEntity::updateLogStatus(std::string db_name, uint64 index, int status)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return;
        db->updateLogStatus(index, status);
    }

    void ClusterEntity::addLogReplyNum(std::string db_name, uint64 index, const std::string& ip)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return;
        db->addLogReplyNum(index, ip);
    }

    void ClusterEntity::addLogSyncNum(std::string db_name, uint64 index, const std::string& ip)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return;
        db->addLogSyncNum(index, ip);
    }

    uint32 ClusterEntity::getLogReplyNum(std::string db_name, uint64 index)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return 0;
        return db->getLogReplyNum(index);
    }

    uint32 ClusterEntity::getLogSyncNum(std::string db_name, uint64 index)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return 0;
        return db->getLogSyncNum(index);
    }

    // term.log
    void ClusterEntity::updateTerm(uint32 term)
    {
        ClusterTermInfo log;
        if (!readFromTermFile(log))
        {
            SLOG_ERROR("term log status fail! ,term:" << term);
            return;
        }
        log.setTerm(term);
        term_ = term;
        if (!writeToTermFile(log))
        {
            SLOG_ERROR("term log status fail! ,term:" << term);
            return;
        }
    }

    void ClusterEntity::updateDbIndex(std::string db_name, uint64 index)
    {
        ClusterTermInfo log;
        if (!readFromTermFile(log))
        {
            SLOG_ERROR("term log status fail!" << db_name << " ,index:" << index);
            return;
        }
        log.setDbIndex(db_name, index);
        if (!writeToTermFile(log))
        {
            SLOG_ERROR("term log status fail!" << db_name << " ,index:" << index);
            return;
        }
    }

    uint32 ClusterEntity::getTerm()
    {
        if (term_ != 0)
            return term_;
        ClusterTermInfo log;
        if (!readFromTermFile(log))
        {
            SLOG_ERROR("term log status fail!");
            return 0;
        }
        term_ = log.getTerm();
        return term_;
    }

    uint64 ClusterEntity::getDbIndex(const std::string& db_name)
    {
        ClusterTermInfo log;
        if (!readFromTermFile(log))
        {
            SLOG_ERROR("term log status fail!");
            return 0;
        }
        return log.getDbIndex(db_name);
    }

    // nt file
    void ClusterEntity::addCachedNtFile(const std::vector<TripleInfo>& triples, const std::string& db_name, const std::string file_name)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return;
        db->addCachedNtFile(triples, file_name);
    }

    void ClusterEntity::appendCachedNtData(const std::vector<TripleInfo>& triples, const std::string& db_name,  const std::string file_name)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return;
        db->appendCachedNtData(triples, file_name);
    }

    void ClusterEntity::getNtFileData(std::vector<TripleInfo>& triples, const std::string& db_name, const std::string& file_name)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return;
        db->getNtFileData(triples, file_name);
    }

    std::string ClusterEntity::getNtFilePath(const std::string& db_name, const std::string& file_name)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return std::string();
        return db->getNtFilePath(file_name);
    }
}
