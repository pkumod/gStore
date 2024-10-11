#include "ApiProvider.h"

namespace server
{
    void ApiHandler::cluster_heartbeat_compare(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& resquest)
    {        
        std::thread([apiUtil, clusterManagerPtr, resquest]()
        {
            uint32_t leader_term = resquest.term;
            uint32_t local_term = clusterManagerPtr->getTerm(); // get local term
            string db_name = resquest.db_name;
            uint64_t leader_index = resquest.index;
            uint64_t leader_nextIndex = resquest.nextIndex;
            uint64_t leader_uid = resquest.uid;
            std::string _db_home = Util::getConfigureValue("db_home");
            std::string _db_suffix = Util::getConfigureValue("db_suffix");
            if (!db_name.empty()) 
            {
                // send ready response
                cluster::ClusterNode leader_node = clusterManagerPtr->getLearrNode();
                std::string check_url = leader_node.getCheckUrl();
                std::string username = leader_node.getUsername();
                std::string password = leader_node.getPassword();
                uint16_t result = 0; // default check failed
                TermDbLog db_log = clusterManagerPtr->getTermInfoDbLog(db_name);
                if (db_log.empty() || db_log.uid != leader_uid)
                {
                    if (apiUtil->check_db_built(db_name))
                    {
                        if (apiUtil->check_db_loaded(db_name))
                        {
                            apiUtil->remove_txn_manager(db_name, false);
                            SLOG_DEBUG("remove " + db_name + " from the txn managers.");
                        }
                        shared_ptr<DatabaseInfo> db_info;
                        apiUtil->get_databaseinfo(db_name, db_info);
                        std::string msg;
                        if (apiUtil->remove_databaseinfo(db_name, msg) == false)
                        {
                            SLOG_DEBUG("remove " + db_name + " from the already build database list fail: " + msg);
                        }
                        SLOG_DEBUG("remove " + db_name + " from the already build database list success.");
                        string db_path = _db_home + db_name + _db_suffix;
                        Util::remove_path(db_path);				
                        string success = "cluster Database " + db_name + " dropped.";
                        clusterManagerPtr->dropDb(db_name);
                    }	

                    // build empty db
                    shared_ptr<DatabaseInfo> db_info = nullptr;
                    apiUtil->init_databaseinfo(db_name, ROOT_USERNAME, Util::get_date_time(), DatabaseStatus::BUILDING);
                    shared_ptr<Database> current_database = make_shared<Database>(db_name);
                    // build empty db
                    current_database->BuildEmptyDB();
                    current_database.reset();
                    // init dabaseinfo
                    apiUtil->get_databaseinfo(db_name, db_info);
                    db_info->initDatabase();
                    db_info->setStatus(DatabaseStatus::AREADY_BUILT);
                    // init privilege
                    apiUtil->init_privilege(ROOT_USERNAME, db_name);
                    string _db_path = _db_home + "/" + db_name + _db_suffix;
                    ofstream f;
                    f.open(_db_path + "/success.txt");
                    f.close();
                    // add backup.log
                    Util::add_backuplog(db_name);
                    // add log
                    clusterManagerPtr->buildDb(db_name, leader_uid);
                    httpentities::ClusterCheckRequest check_request(local_term, db_name, 0, 0, leader_uid, result, resquest.local_port);
                    HttpUtil::clusterCheck(check_url, check_request, username, password);
                }
                else
                {
                    if (leader_term == local_term && leader_index == db_log.index && leader_uid == db_log.uid)
                    {
                        // check ok
                        result == -1;
                    }
                    httpentities::ClusterCheckRequest check_request(local_term, db_name, db_log.index, db_log.nextIndex, leader_uid, result, resquest.local_port);
                    HttpUtil::clusterCheck(check_url, check_request, username, password);
                }
            }
        }).detach();       
    }

    void ApiHandler::cluster_heartbeat_prepare(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& resquest)
    {
        std::thread([apiUtil, clusterManagerPtr, resquest]()
        {
            uint32_t leader_term = resquest.term;
            string db_name = resquest.db_name;
            uint64_t leader_index = resquest.index;
            uint64_t leader_nextIndex = resquest.nextIndex;
            std::string _db_home = Util::getConfigureValue("db_home");
            std::string _db_suffix = Util::getConfigureValue("db_suffix");
            shared_ptr<DatabaseInfo> db_info = nullptr;
            ClusterUpdateType update_type = ClusterUpdateType_None;
            TermDbLog db_log;
            if (apiUtil->check_db_built(db_name) == false)
            {
                apiUtil->init_databaseinfo(db_name, ROOT_USERNAME, Util::get_date_time(), DatabaseStatus::BUILDING);
                shared_ptr<Database> current_database = make_shared<Database>(db_name);
                // build empty db
                current_database->BuildEmptyDB();
                current_database.reset();
                // init dabaseinfo
                apiUtil->get_databaseinfo(db_name, db_info);
                db_info->initDatabase();
                db_info->setStatus(DatabaseStatus::AREADY_BUILT);
                // init privilege
                apiUtil->init_privilege(ROOT_USERNAME, db_name);
                string _db_path = _db_home + "/" + db_name + _db_suffix;
                ofstream f;
                f.open(_db_path + "/success.txt");
                f.close();
                // add backup.log
                Util::add_backuplog(db_name);
                update_type = ClusterUpdateType_Build;
            } 
            else 
            {
                apiUtil->get_databaseinfo(db_name, db_info);
                db_log = clusterManagerPtr->getTermInfoDbLog(db_name);
                if (leader_index != db_log.getIndex())
                {
                    SLOG_ERROR("check term.json, db name data is different" << db_name);
                    return;
                }
            }
            // check loaded
            if (apiUtil->check_db_loaded(db_name) == false)
            {
                db_info->getDatabase()->load();
                db_info->setStatus(DatabaseStatus::LOADED);
                apiUtil->insert_txn_manager(db_name, db_info);
            }
            // init cluster db path
            std::string cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
            Util::create_dirs(cluster_db_path);
            // add log
            clusterManagerPtr->addLog(db_name, leader_nextIndex, ClusterOperation::ClusterOperation_Prepare, update_type);
            
            // send ready response
            cluster::ClusterNode leader_node = clusterManagerPtr->getLearrNode();
            std::string reply_url = leader_node.getReplyUrl();
            std::string username = leader_node.getUsername();
            std::string password = leader_node.getPassword();
            std::string reply_operation = ClusterOperationHandle::to_str(ClusterOperation_Prepare);
            httpentities::ReplyRequest reply_request(leader_term, db_name, db_log.getIndex(), leader_nextIndex, db_log.getUid(), reply_operation, resquest.local_port);
            HttpUtil::reply(reply_url, reply_request, username, password);
        }).detach();
    }

    void ApiHandler::cluster_heartbeat_commit(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& resquest)
    {
        string db_name = resquest.db_name;
        uint64_t leader_index = resquest.index;
        uint64_t leader_nextIndex = resquest.nextIndex;
        if (!db_name.empty())
        {
            // get current can be committed index， and compare with leader_index
            TermDbLog db_log = clusterManagerPtr->getTermInfoDbLog(db_name);
            if (leader_index == db_log.getIndex() && leader_nextIndex == db_log.getNextIndex())
            {
                clusterManagerPtr->updateLogOperation(db_name, leader_nextIndex, cluster::ClusterOperation::ClusterOperation_Commit);
            }
        }
    }

    void ApiHandler::cluster_heartbeat_cancel(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& resquest)
    {
        string db_name = resquest.db_name;
        uint64_t leader_index = resquest.index;
        uint64_t leader_nextIndex = resquest.nextIndex;
        if (!db_name.empty())
        {
            // get current index， and compare with leader_index
            TermDbLog db_log = clusterManagerPtr->getTermInfoDbLog(db_name);
            if (leader_index == db_log.getIndex() && leader_nextIndex == db_log.getNextIndex())
            {
                shared_ptr<DatabaseInfo> db_info;
                apiUtil->get_databaseinfo(db_name, db_info);
                apiUtil->wrlock_databaseinfo(db_info);
                std::string nt_file_path = clusterManagerPtr->getNTFilePathByIndex(db_name, leader_nextIndex);
                if (!nt_file_path.empty())
                {
                    cluster::ClusterUpdateType cluster_update_type = clusterManagerPtr->getDbLogUpdateType(db_name, leader_nextIndex);
                    if (cluster_update_type == ClusterUpdateType::ClusterUpdateType_Delete)
                    {
                        uint32_t num = db_info->getDatabase()->batch_insert(nt_file_path);
                        SLOG_DEBUG("follower restore " + db_name + " data: batch insert num " << num);
                    } 
                    else 
                    {
                        uint32_t num = db_info->getDatabase()->batch_remove(nt_file_path);
                        SLOG_DEBUG("follower restore " + db_name + " data: batch_remove num " << num);
                    }
                    db_info->getDatabase()->save();
                    Util::remove_path(nt_file_path);
                    clusterManagerPtr->updateLogOperation(db_name, leader_nextIndex, cluster::ClusterOperation::ClusterOperation_Cancel);
                }
                else
                {
                    SLOG_DEBUG("not found nt file path:" << db_name << " ,index:" << leader_nextIndex);
                }
                apiUtil->unlock_databaseinfo(db_info);
            }
        }
    }

    void ApiHandler::cluster_heartbeat_fail(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& resquest)
    {
        string db_name = resquest.db_name;
        uint64_t leader_index = resquest.index;
        uint64_t leader_nextIndex = resquest.nextIndex;
        if (!db_name.empty())
        {
            shared_ptr<DatabaseInfo> db_info;
            apiUtil->get_databaseinfo(db_name, db_info);
            apiUtil->wrlock_databaseinfo(db_info);
            // get current index， and compare with leader_index
            TermDbLog db_log = clusterManagerPtr->getTermInfoDbLog(db_name);
            if (leader_index == db_log.getIndex() && leader_nextIndex == db_log.getNextIndex())
            {
                clusterManagerPtr->updateLogOperation(db_name, leader_nextIndex, cluster::ClusterOperation::ClusterOperation_Fail);
            }
            apiUtil->unlock_databaseinfo(db_info);
        }
    }

    void ApiHandler::cluster_heartbeat_drop(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& resquest)
    {
        string db_name = resquest.db_name;
        uint64_t leader_index = resquest.index;
        uint64_t leader_nextIndex = resquest.nextIndex;
        std::string _db_home = Util::getConfigureValue("db_home");
        std::string _db_suffix = Util::getConfigureValue("db_suffix");
        if (!db_name.empty())
        {
            if (!apiUtil->check_db_built(db_name))
            {
                return;
            }	
            if (apiUtil->check_db_loaded(db_name))
            {
                apiUtil->remove_txn_manager(db_name, false);
                SLOG_DEBUG("remove " + db_name + " from the txn managers.");
            }
            shared_ptr<DatabaseInfo> db_info;
            apiUtil->get_databaseinfo(db_name, db_info);
            std::string msg;
            if (apiUtil->remove_databaseinfo(db_name, msg) == false)
            {
                SLOG_DEBUG("remove " + db_name + " from the already build database list fail: " + msg);
                return;
            }
            SLOG_DEBUG("remove " + db_name + " from the already build database list success.");
            string db_path = _db_home + db_name + _db_suffix;
            Util::remove_path(db_path);				
            string success = "cluster Database " + db_name + " dropped.";
            clusterManagerPtr->dropDb(db_name);
        }
    }

    void ApiHandler::cluster_append(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, std::map<std::string, std::pair<std::string, std::string>>& form, MessageResponse& response, const string& local_port)
    {
        if (form.empty())
        {   
            response.status_code = StatusFileReadError;
            response.status_msg = "Form data is empty";
            return;
        }
        if (form.find("file") == form.end() || form.find("db_name") == form.end() 
            || form.find("term") == form.end() || form.find("index") == form.end() 
            || form.find("updateType") == form.end())
        {
            response.status_code = StatusFileReadError;
            response.status_msg = "Form data is illegal";
            return;
        }
        
        std::string msg;
        // filename : filecontent
        std::pair<std::string, std::string>& fileinfo = form.at("file");
        if(fileinfo.first.empty())
        {
            response.status_code = StatusParamIsIllegal;
            response.status_msg = "append file can not be empty!";
            return;
        }
        std::string file_suffix = Util::fileSuffix(fileinfo.first);
        if (!apiUtil->check_upload_allow_compress_packages(file_suffix))
        {
            response.status_msg =  "The type of append file is not supported!";
            response.status_code = StatusOperationFailed;
            return;
        }
        std::string db_name = form.at("db_name").second;
        if (db_name.empty())
        {
            response.status_msg =  "db_name can not be empty!";
            response.status_code = StatusOperationFailed;
            return;
        }
        uint64_t leader_uid = std::stoul(form.at("uid").second);
        TermDbLog db_log = clusterManagerPtr->getTermInfoDbLog(db_name);
        uint64_t leader_index = std::stoul(form.at("index").second);
        uint64_t leader_nextIndex = std::stoul(form.at("nextIndex").second);
        if (leader_uid != db_log.getUid() || leader_index != db_log.getIndex() || leader_nextIndex != db_log.getNextIndex())
        {
            response.status_msg =  "follower different leader db name:" + db_name + " ,db uid:" + std::to_string(leader_uid) + " ,follower db uid:" + std::to_string(db_log.getUid());
            response.status_code = StatusOperationFailed;
            return;
        }

        uint32_t leader_term = std::stol(form.at("term").second);
        // TODO check leader term and index with local
        const std::string cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
        const std::string zip_file_path = cluster_db_path + fileinfo.first;
        const std::string update_type = form.at("updateType").second;
        const std::string content = std::move(fileinfo.second);
        WFFileIOTask *pwrite_task = WFTaskFactory::create_pwrite_task(zip_file_path, content.c_str(),content.size(), 0, [apiUtil, clusterManagerPtr, leader_term, db_name, zip_file_path, cluster_db_path, update_type, db_log, local_port](WFFileIOTask *pwrite_task){
            SLOG_DEBUG("saveing log file callback.");
            // save success
            long ret = pwrite_task->get_retval();
            if (pwrite_task->get_state() != WFT_STATE_SUCCESS || ret < 0) {
                return;
            }
            // unzip file
            CompressUtil::UnCompressZip unzip(zip_file_path, cluster_db_path);
            if (unzip.unCompress() != CompressUtil::UnZipOK) 
            {
                SLOG_ERROR("uncompress zip file fail: " + zip_file_path);
                // remove zip file
                Util::remove_path(zip_file_path);
                return;
            }
            std::vector<std::string> log_files;
            unzip.getFileList(log_files, "");
            if (log_files.empty())
            {
                SLOG_WARN("zip file is empty: " + zip_file_path);
                // remove zip file
                Util::remove_path(zip_file_path);
                return;
            }
            if (apiUtil->check_db_built(db_name) == false) 
            {
                SLOG_WARN("db[" + db_name + "] is not built.");
                return;
            }
            shared_ptr<DatabaseInfo> db_info = nullptr;
            apiUtil->get_databaseinfo(db_name, db_info);
            if(apiUtil->check_db_loaded(db_name) == false) 
            {
                SLOG_DEBUG("db[" + db_name + "] is not loaded, now begin loading.");
                // load db
                db_info->getDatabase()->load();
                apiUtil->insert_txn_manager(db_name, db_info);
            }
            if(!apiUtil->trywrlock_databaseinfo(db_info, 600)) {
                SLOG_WARN("unable to get write lock of " + db_name + ".");
                // remove zip file
                Util::remove_path(zip_file_path);
                return;
            }
            std::string log_file_name = Util::fileName(log_files[0]);
            std::string nt_file_path = clusterManagerPtr->getNtFilePath(db_name, log_file_name);
            ClusterUpdateType log_update_type = ClusterUpdateType_Defaut;
            if (update_type == "1") {
                // batch insert
                db_info->getDatabase()->batch_insert(nt_file_path);
                log_update_type = ClusterUpdateType::ClusterUpdateType_Insert;
            } else if (update_type == "2") {
                // batch remove
                db_info->getDatabase()->batch_remove(nt_file_path);
                log_update_type = ClusterUpdateType::ClusterUpdateType_Delete;
            }
            db_info->getDatabase()->save();
            Util::remove_path(zip_file_path);
            Util::remove_path(nt_file_path);
            apiUtil->unlock_databaseinfo(db_info);

            // update local log trem and index
            clusterManagerPtr->updateTerm(leader_term);
            clusterManagerPtr->updateLogInfo(db_name, db_log.getNextIndex(), ClusterOperation::ClusterOperation_Append, log_update_type, Util::fileName(log_file_name));

            // send appendEntrites ok response
            cluster::ClusterNode leader_node = clusterManagerPtr->getLearrNode();
            std::string reply_url = leader_node.getReplyUrl();
            std::string username = leader_node.getUsername();
            std::string password = leader_node.getPassword();
            std::string expection = ClusterOperationHandle::to_str(cluster::ClusterOperation::ClusterOperation_Append);
            httpentities::ReplyRequest reply_request(leader_term, db_name, db_log.getIndex(), db_log.getNextIndex(), db_log.getUid(), expection, local_port);
            HttpUtil::reply(reply_url, reply_request, username, password);
        });
        std::thread([pwrite_task](){
            SLOG_DEBUG("saveing log file start...");
            pwrite_task->start();
        }).detach();
    }

    void ApiHandler::cluster_reply(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterReplyRequest& resquest, const string& remote_ip)
    {
        cluster::ClusterOperation expection_enum = cluster::ClusterOperationHandle::to_enum(resquest.operation);
        // from follower reply, go into leader process 
        if (expection_enum == cluster::ClusterOperation::ClusterOperation_Prepare)
        {
            clusterManagerPtr->addLogReplyNum(resquest.db_name, resquest.nextIndex, remote_ip, resquest.port);
        } else if (expection_enum == cluster::ClusterOperation::ClusterOperation_Append) {
            clusterManagerPtr->addLogSyncNum(resquest.db_name, resquest.nextIndex, remote_ip, resquest.port);
        }
    }

    void ApiHandler::cluster_check(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterCheckRequest& resquest, const string& remote_ip)
    {
        uint64_t follower_index = resquest.index;
        std::string db_name = resquest.db_name;
        std::string port = resquest.port;
        if (resquest.result == 0)
        {
            // TODO add a new task that starting with follower index
            TermDbLog db_info = clusterManagerPtr->getTermInfoDbLog(db_name);
            if (db_info.index != follower_index)
            {
                uint64_t restore_index = 0;
                if (!db_info.empty() && db_info.getFirstIndex() != 0)
                {
                    if (follower_index == 0)
                    {
                        restore_index = db_info.firstIndex;
                    }
                    else
                    {
                        restore_index = clusterManagerPtr->getDbNextIndexByIndex(db_name, follower_index);
                    }
                }
                if (restore_index != 0)
                {
                    ClusterRecoverInfo task_info(db_name, restore_index, remote_ip, port);
                    clusterManagerPtr->addTask(task_info);
                }
            }
        }
    }

    void ApiHandler::cluster_recover(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, std::map<std::string, std::pair<std::string, std::string>>& form, MessageResponse& response, const string& local_port)
    {
        if (form.empty())
        {   
            response.status_code = StatusFileReadError;
            response.status_msg = "Form data is empty";
            return;
        }
        if (form.find("file") == form.end() || form.find("db_name") == form.end() 
            || form.find("term") == form.end() || form.find("index") == form.end() 
            || form.find("updateType") == form.end())
        {
            response.status_code = StatusFileReadError;
            response.status_msg = "Form data is illegal";
            return;
        }
        
        // filename : filecontent
        std::pair<std::string, std::string>& fileinfo = form.at("file");
        if(fileinfo.first.empty())
        {
            response.status_msg =  "append file can not be empty!";
            response.status_code = StatusParamIsIllegal;
            return;
        }
        std::string file_suffix = Util::fileSuffix(fileinfo.first);
        if (!apiUtil->check_upload_allow_compress_packages(file_suffix))
        {
            response.status_msg =  "The type of append file is not supported!";
            response.status_code = StatusOperationFailed;
            return;
        }
        std::string db_name = form.at("db_name").second;
        if (db_name.empty())
        {
            response.status_msg =  "db_name can not be empty!";
            response.status_code = StatusOperationFailed;
            return;
        }
        uint64_t leader_index = std::stoul(form.at("index").second);
        uint64_t leader_nextIndex = std::stoul(form.at("nextIndex").second);
        uint64_t leader_uid = std::stoul(form.at("uid").second);
        TermDbLog db_info = clusterManagerPtr->getTermInfoDbLog(db_name);
        if (leader_uid != db_info.getUid() || leader_index == db_info.getIndex())
        {
            response.status_msg =  "ok";
            response.status_code = StatusOK;
            return;
        }
        uint32_t leader_term = std::stol(form.at("term").second);
        std::string updateType = form.at("updateType").second;
        uint64_t recoverIndex = std::stoul(form.at("recoverIndex").second);
        // TODO check leader term and index with local
        const std::string cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
        const std::string zip_file_path = cluster_db_path + fileinfo.first;
        const std::string content = std::move(fileinfo.second);
        WFFileIOTask *pwrite_task = WFTaskFactory::create_pwrite_task(zip_file_path, content.c_str(),content.size(), 0, [apiUtil, clusterManagerPtr, leader_term, leader_index, db_name, zip_file_path, cluster_db_path, updateType, recoverIndex](WFFileIOTask *pwrite_task){
            SLOG_DEBUG("saveing log file callback.");
            // save success
            long ret = pwrite_task->get_retval();
            if (pwrite_task->get_state() != WFT_STATE_SUCCESS || ret < 0) {
                return;
            }
            // unzip file
            CompressUtil::UnCompressZip unzip(zip_file_path, cluster_db_path);
            if (unzip.unCompress() != CompressUtil::UnZipOK) 
            {
                SLOG_ERROR("uncompress zip file fail: " + zip_file_path);
                // remove zip file
                Util::remove_path(zip_file_path);
                return;
            }
            std::vector<std::string> log_files;
            unzip.getFileList(log_files, "");
            if (log_files.empty())
            {
                SLOG_WARN("zip file is empty: " + zip_file_path);
                // remove zip file
                Util::remove_path(zip_file_path);
                return;
            }
            if (apiUtil->check_db_built(db_name) == false) 
            {
                SLOG_WARN("db[" + db_name + "] is not built.");
                return;
            }
            shared_ptr<DatabaseInfo> db_info = nullptr;
            apiUtil->get_databaseinfo(db_name, db_info);
            if(apiUtil->check_db_loaded(db_name) == false) 
            {
                SLOG_DEBUG("db[" + db_name + "] is not loaded, now begin loading.");
                // load db
                db_info->getDatabase()->load();
                apiUtil->insert_txn_manager(db_name, db_info);
            }
            if(!apiUtil->trywrlock_databaseinfo(db_info, 600)) {
                SLOG_WARN("unable to get write lock of " + db_name + ".");
                // remove zip file
                Util::remove_path(zip_file_path);
                return;
            }
            std::string log_file_name = Util::fileName(log_files[0]);
            std::string nt_file_path = clusterManagerPtr->getNtFilePath(db_name, log_file_name);
            ClusterUpdateType log_updateType;
            if (updateType == "1") {
                // batch insert
                db_info->getDatabase()->batch_insert(nt_file_path);
                log_updateType = ClusterUpdateType::ClusterUpdateType_Insert;
            } else if (updateType == "2") {
                // batch remove
                db_info->getDatabase()->batch_remove(nt_file_path);
                log_updateType = ClusterUpdateType::ClusterUpdateType_Delete;
            }
            db_info->getDatabase()->save();
            Util::remove_path(zip_file_path);
            Util::remove_path(nt_file_path);
            apiUtil->unlock_databaseinfo(db_info);

            // update local log trem and index
            clusterManagerPtr->updateTerm(leader_term);
            clusterManagerPtr->addCommitLog(db_name, recoverIndex, log_updateType, Util::fileName(log_file_name));
        });
        std::thread([pwrite_task](){
            SLOG_DEBUG("saveing log file start...");
            pwrite_task->start();
        }).detach();
    }
}