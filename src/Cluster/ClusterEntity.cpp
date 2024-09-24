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
            if (!Util::file_exist(Util::getConfigureValue("db_home") + db_name + ".db"))
            {
                SLOG_ERROR("db is not exist, please check db name:" << db_name);
                return nullptr;
            }
            SLOG_TRACE("cluster init db name:" << db_name);
            return addClusterDb(db_name);
        }
        return it->second;
    }

    void ClusterEntity::addLog(std::string db_name, uint64 index, ClusterLogStatus status, ClusterOperation operation)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return;
        uint64 last_index = getDbIndex(db_name);
        if (last_index != 0)
        {
            SLOG_TRACE("please sure last index is finish:" << last_index);
            // updateDbIndex(db_name, last_index);
        }
        updateDbNextIndex(db_name, index);
        db->addLog(index, status, operation, getDbIndex(db_name));
    }

    void ClusterEntity::updateLogStatus(std::string db_name, uint64 index, ClusterLogStatus status)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return;
        
        if (status == ClusterLogStatus_commit)
        {
            updateDbIndex(db_name, index);
            updateDbNextIndex(db_name, 0);
        }
        else if (status == ClusterLogStatus_cancel || status == ClusterLogStatus_fail)
        {
            // this operation is failed
            updateDbNextIndex(db_name, 0);
        }
        db->updateLogStatus(index, status);
    }

    void ClusterEntity::setLogOperation(std::string db_name, uint64 index, ClusterOperation operation)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return;
        db->setLogOperation(index, operation);
    }

    void ClusterEntity::setLogFileName(std::string db_name, uint64 index, std::string file_name)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return;
        db->setLogFileName(index, file_name);
    }

    void ClusterEntity::addLogReplyNum(std::string db_name, uint64 index, const std::string& ip, const std::string& port)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return;
        db->addLogReplyNum(index, getIpPort(ip, port));
    }

    void ClusterEntity::addLogSyncNum(std::string db_name, uint64 index, const std::string& ip, const std::string& port)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return;
        db->addLogSyncNum(index, getIpPort(ip, port));
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

    std::string ClusterEntity::getIpPort(const std::string& ip, const std::string& port)
    {
        return ip + ":" + port;
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

    void ClusterEntity::updateDbNextIndex(std::string db_name, uint64 next_index)
    {
        ClusterTermInfo log;
        if (!readFromTermFile(log))
        {
            SLOG_ERROR("term log status fail!" << db_name << " ,next_index:" << next_index);
            return;
        }
        log.setDbNextIndex(db_name, next_index);
        if (!writeToTermFile(log))
        {
            SLOG_ERROR("term log status fail!" << db_name << " ,next_index:" << next_index);
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

    uint64 ClusterEntity::getDbNextIndex(const std::string& db_name)
    {
        ClusterTermInfo log;
        if (!readFromTermFile(log))
        {
            SLOG_ERROR("term log status fail!");
            return 0;
        }
        return log.getDbNextIndex(db_name);
    }

    uint64 ClusterEntity::getFirstIndex(const std::string& db_name)
    {
        ClusterTermInfo log;
        if (!readFromTermFile(log))
        {
            SLOG_ERROR("term log status fail!");
            return 0;
        }
        return log.getFirstIndex(db_name);
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

    std::string ClusterEntity::getDbDirPath(const std::string& db_name)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return "";
        return db->getDbDirPath();
    }

    std::string ClusterEntity::getNtFilePath(const std::string& db_name, const std::string& file_name)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return std::string();
        return db->getNtFilePath(file_name);
    }

    std::string ClusterEntity::getNTFilePathByIndex(const std::string& db_name, uint64 index)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return std::string();
        std::string file_name = db->getFileName(index);
        if (file_name.empty())
            return std::string();
        return db->getNtFilePath(file_name);
    }

    ClusterLogStatus ClusterEntity::getDbLogStatus(const std::string& db_name, uint64 index)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return ClusterLogStatus_None;
        return db->getStatus(index);
    }

    ClusterOperation ClusterEntity::getDbLogOperation(const std::string& db_name, uint64 index)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return ClusterOperation_None;
        return db->getOperation(index);
    }

    void ClusterEntity::getDbNextIndexL(const std::string& db_name, uint64 index, std::vector<uint64StringPair>& indexl)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return;
        return db->getNextIndexL(index, indexl);
    }

    void ClusterEntity::dropDb(std::string db_name)
    {
        ClusterTermInfo log;
        if (!readFromTermFile(log))
        {
            SLOG_ERROR("term log status fail! ,term:" << db_name);
            return;
        }
        log.eraseDb(db_name);
        Util::remove_dir(getDbDirPath(db_name));
        databaseL_.erase(db_name);
        if (!writeToTermFile(log))
        {
            SLOG_ERROR("term log status fail! ,term:" << db_name);
            return;
        }
    }
}
