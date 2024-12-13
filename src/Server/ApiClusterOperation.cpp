#include "ApiProvider.h"

namespace server
{
    void ApiHandler::cluster_heartbeat_compare(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& resquest)
    {        
        std::thread([apiUtil, clusterManagerPtr, resquest]()
        {
            string db_name = resquest.db_name;
            if (db_name.empty()) 
                return;
            uint32_t leader_term = resquest.term;
            uint32_t local_term = clusterManagerPtr->getTerm(); // get local term
            uint64_t leader_index = resquest.index;
            uint64_t leader_uid = resquest.uid;
            std::string follow_ip = resquest.follow_ip;
            std::string follow_port = resquest.follow_port;

            // 通过主节点发送的从节点ip和端口号, 来设置从节点(目前从节点恢复数据需要告知主节点自己的ip和端口)
            // ip可能并不是一个ip地址, 也许是一个重定向的服务名称, 或则端口做过映射那么conf.ini的端口号就不是当前从节点的端口号
            clusterManagerPtr->setFollowIpPort(follow_ip, follow_port);
            
            // send ready response
            cluster::ClusterNode leader_node = clusterManagerPtr->getLearrNode();
            std::string check_url = leader_node.getCheckUrl();
            std::string username = leader_node.getUsername();
            std::string password = leader_node.getPassword();

            // result = 0, check ok, leader == follower
            // result = 1, recover by index
            // result = 2, recover by init
            // result = 3, follower is restoring
            uint16_t result = 0; // default check failed
            TermDbLog db_log = clusterManagerPtr->getTermInfoDbLog(db_name);
            if (clusterManagerPtr->isFollowerRestoring(db_name))
            {
                // follower is restoring, please wait
                result = 3;
                httpentities::ClusterCheckRequest check_request(local_term, db_name, db_log.index, db_log.nextIndex, leader_uid, result);
                check_request.setFollowIpPort(follow_ip, resquest.follow_port);
                HttpUtil::clusterCheck(check_url, check_request, username, password);
                SLOG_TRACE("heart compare, follower is restoring, please wait......" << db_name);
                return;
            }
            if (db_log.empty() || db_log.uid != leader_uid)
            {
                result = 2;
                httpentities::ClusterCheckRequest check_request(local_term, db_name, db_log.index, db_log.nextIndex, leader_uid, result);
                check_request.setFollowIpPort(follow_ip, resquest.follow_port);
                HttpUtil::clusterCheck(check_url, check_request, username, password);
                SLOG_TRACE("follower notify leader send data init database");
                return;
            }
            else
            {
                if (leader_term == local_term && leader_index == db_log.index && leader_uid == db_log.uid)
                {
                    // check ok
                    result = 0;
                }
                else
                    result = 1;
                httpentities::ClusterCheckRequest check_request(local_term, db_name, db_log.index, db_log.nextIndex, leader_uid, result);
                check_request.setFollowIpPort(follow_ip, follow_port);
                HttpUtil::clusterCheck(check_url, check_request, username, password);
            }
        }).detach();       
    }

    void ApiHandler::cluster_heartbeat_prepare(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& resquest)
    {
        std::thread([apiUtil, clusterManagerPtr, resquest]()
        {
            string db_name = resquest.db_name;
            if (apiUtil->check_db_built(db_name) == false)
            {
                SLOG_TRACE("cluster follower not build database, database name:" << db_name);
                return;
            }
            if (clusterManagerPtr->isFollowerRestoring(db_name))
            {
                SLOG_TRACE("heart prepare, follower is restoring, please wait......:" << db_name);
                return;
            }

            uint64_t leader_index = resquest.index;
            shared_ptr<DatabaseInfo> db_info = nullptr;
            apiUtil->get_databaseinfo(db_name, db_info);
            TermDbLog db_log = clusterManagerPtr->getTermInfoDbLog(db_name);
            if (leader_index != db_log.getIndex())
            {
                SLOG_ERROR("check term.json, db name data is different" << db_name);
                return;
            }

            uint32_t leader_term = resquest.term;
            uint64_t leader_nextIndex = resquest.nextIndex;
            ClusterUpdateType update_type = ClusterUpdateType_None;
            // check loaded
            if (apiUtil->check_db_loaded(db_name) == false)
            {
                db_info->getDatabase()->load();
                db_info->setStatus(DatabaseStatus::LOADED);
                apiUtil->insert_txn_manager(db_name, db_info);
            }
            // init cluster db path
            std::string cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
            FileUtil::createDirs(cluster_db_path);
            // add log
            clusterManagerPtr->addLog(db_name, leader_nextIndex, ClusterOperation::ClusterOperation_Prepare, update_type);
            
            // send ready response
            cluster::ClusterNode leader_node = clusterManagerPtr->getLearrNode();
            std::string reply_url = leader_node.getReplyUrl();
            std::string username = leader_node.getUsername();
            std::string password = leader_node.getPassword();
            std::string reply_operation = ClusterOperationHandle::to_str(ClusterOperation_Prepare);
            httpentities::ReplyRequest reply_request(leader_term, db_name, db_log.getIndex(), leader_nextIndex, db_log.getUid(), reply_operation, resquest.follow_port);
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
                    FileUtil::removePath(nt_file_path);
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
        std::string _db_home = GlobalTypedef::db_home();
        std::string _db_suffix = GlobalTypedef::db_suffix();
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
            FileUtil::removePath(db_path);				
            string success = "cluster Database " + db_name + " dropped.";
            clusterManagerPtr->dropDb(db_name);
        }
    }

    void ApiHandler::cluster_append(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, std::map<std::string, std::pair<std::string, std::string>>& form, MessageResponse& response, const string& local_port)
    {
        if (form.empty())
        {   
            response.StatusCode = StatusFileReadError;
            response.StatusMsg = "Form data is empty";
            return;
        }
        if (form.find("file") == form.end() || form.find("db_name") == form.end() 
            || form.find("term") == form.end() || form.find("index") == form.end() 
            || form.find("updateType") == form.end())
        {
            response.StatusCode = StatusFileReadError;
            response.StatusMsg = "Form data is illegal";
            return;
        }
        
        // filename : filecontent
        std::pair<std::string, std::string>& fileinfo = form.at("file");
        if(fileinfo.first.empty())
        {
            response.StatusCode = StatusParamIsIllegal;
            response.StatusMsg = "append file can not be empty!";
            return;
        }
        std::string file_suffix = FileUtil::fileSuffix(fileinfo.first);
        if (!apiUtil->check_upload_allow_compress_packages(file_suffix))
        {
            response.StatusMsg =  "The type of append file is not supported!";
            response.StatusCode = StatusOperationFailed;
            return;
        }
        std::string db_name = form.at("db_name").second;
        if (db_name.empty())
        {
            response.StatusMsg =  "db_name can not be empty!";
            response.StatusCode = StatusOperationFailed;
            return;
        }
        uint64_t leader_uid = std::stoul(form.at("uid").second);
        TermDbLog db_log = clusterManagerPtr->getTermInfoDbLog(db_name);
        uint64_t leader_index = std::stoul(form.at("index").second);
        uint64_t leader_nextIndex = std::stoul(form.at("nextIndex").second);
        if (leader_uid != db_log.getUid() || leader_index != db_log.getIndex() || leader_nextIndex != db_log.getNextIndex())
        {
            response.StatusMsg =  "follower different leader db name:" + db_name + " ,db uid:" + std::to_string(leader_uid) + " ,follower db uid:" + std::to_string(db_log.getUid());
            response.StatusCode = StatusOperationFailed;
            return;
        }

        uint32_t leader_term = std::stol(form.at("term").second);
        // TODO check leader term and index with local
        const std::string cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
        const std::string zip_file_path = cluster_db_path + fileinfo.first;
        const std::string update_type = form.at("updateType").second;
        std::string *save_content = new std::string;
        *save_content = std::move(fileinfo.second);
        WFFileIOTask *pwrite_task = WFTaskFactory::create_pwrite_task(zip_file_path, static_cast<const void *>((*save_content).c_str()), (*save_content).size(), 0, [save_content, apiUtil, clusterManagerPtr, leader_term, db_name, zip_file_path, cluster_db_path, update_type, db_log, local_port](WFFileIOTask *pwrite_task){
            SLOG_DEBUG("saveing log file callback. detete content file");
            delete save_content;
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
                FileUtil::removePath(zip_file_path);
                return;
            }
            std::vector<std::string> log_files;
            unzip.getFileList(log_files, "");
            if (log_files.empty())
            {
                SLOG_WARN("zip file is empty: " + zip_file_path);
                // remove zip file
                FileUtil::removePath(zip_file_path);
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
                FileUtil::removePath(zip_file_path);
                return;
            }
            std::string log_file_name = FileUtil::fileName(log_files[0]);
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
            FileUtil::removePath(zip_file_path);
            FileUtil::removePath(nt_file_path);
            apiUtil->unlock_databaseinfo(db_info);

            // update local log trem and index
            clusterManagerPtr->updateTerm(leader_term);
            clusterManagerPtr->updateLogInfo(db_name, db_log.getNextIndex(), ClusterOperation::ClusterOperation_Append, log_update_type, FileUtil::fileName(log_file_name));

            // send appendEntrites ok response
            cluster::ClusterNode leader_node = clusterManagerPtr->getLearrNode();
            std::string reply_url = leader_node.getReplyUrl();
            std::string username = leader_node.getUsername();
            std::string password = leader_node.getPassword();
            std::string expection = ClusterOperationHandle::to_str(cluster::ClusterOperation::ClusterOperation_Append);
            std::string follow_port = clusterManagerPtr->getFollowPort();
            if (follow_port.empty())
            {
                follow_port = local_port;
            }
            httpentities::ReplyRequest reply_request(leader_term, db_name, db_log.getIndex(), db_log.getNextIndex(), db_log.getUid(), expection, follow_port);
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
            clusterManagerPtr->addLogReplyNum(resquest.db_name, resquest.nextIndex, remote_ip, resquest.follow_port);
        } else if (expection_enum == cluster::ClusterOperation::ClusterOperation_Append) {
            clusterManagerPtr->addLogSyncNum(resquest.db_name, resquest.nextIndex, remote_ip, resquest.follow_port);
        }
    }

    void ApiHandler::cluster_check(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterCheckRequest& resquest, const string& remote_ip)
    {
        uint64_t follower_index = resquest.index;
        std::string db_name = resquest.db_name;
        if (resquest.follow_ip.empty())
        {
            SLOG_TRACE("follower ip do not null");
            return;
        }
        if (clusterManagerPtr->isFollowerRestoring(db_name))
        {
            SLOG_TRACE("cluster check, follower is restoring, please wait......" << db_name);
            return;
        }
        if (resquest.result == 1)
        {
            // add a new task that starting with follower index
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
                    ClusterRecoverInfo task_info(db_name, restore_index, resquest.follow_ip, resquest.follow_port);
                    clusterManagerPtr->addTask(task_info);
                }
            }
        }
        else if (resquest.result == 2)
        {
            shared_ptr<DatabaseInfo> db_info;
            apiUtil->get_databaseinfo(db_name, db_info);
            if (db_info == nullptr)
            {
                SLOG_TRACE("can't find [" << db_name << "] database info from already builts list");
                return;
            }
            if (apiUtil->trywrlock_databaseinfo(db_info, 1) ==  false)
            {
                SLOG_TRACE("Unable to drop due to loss of lock");
                return;
            }
            if (!db_info->getDatabase()->save())
            {
                apiUtil->unlock_databaseinfo(db_info);
                SLOG_TRACE("disk or memory not enough");
                return;
            }

            ClusterRecoverInfo task_info;
            task_info.db_name = db_name;
            task_info.index = clusterManagerPtr->getDbIndex(db_name);
            task_info.operation = ClusterOperation_Init;
            task_info.ip = resquest.follow_ip;
            task_info.port = resquest.follow_port;
            task_info.zip_path = clusterManagerPtr->compressInitDb(task_info);
            if (!FileUtil::fileExists(task_info.zip_path))
            {
                SLOG_TRACE("leader database dir compress not exist:" << db_name);
                apiUtil->unlock_databaseinfo(db_info);
                return;
            }
            apiUtil->unlock_databaseinfo(db_info);
            clusterManagerPtr->addTask(task_info);
        }
    }

    void ApiHandler::cluster_recover(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, std::map<std::string, std::pair<std::string, std::string>>& form, MessageResponse& response, const string& local_port)
    {
        if (form.empty())
        {   
            response.StatusCode = StatusFileReadError;
            response.StatusMsg = "Form data is empty";
            return;
        }
        if (form.find("file") == form.end() || form.find("db_name") == form.end() 
            || form.find("term") == form.end() || form.find("index") == form.end() 
            || form.find("updateType") == form.end())
        {
            response.StatusCode = StatusFileReadError;
            response.StatusMsg = "Form data is illegal";
            return;
        }
        // 注意文件file已经转义所有权, 不能再使用
        std::shared_ptr<MessageClusterRecoverRequest> request = std::make_shared<MessageClusterRecoverRequest>(form);
        if(request->file_name.empty())
        {
            response.StatusMsg =  "append file can not be empty!";
            response.StatusCode = StatusParamIsIllegal;
            return;
        }
        std::string file_suffix = FileUtil::fileSuffix(request->file_name);
        if (!apiUtil->check_upload_allow_compress_packages(file_suffix))
        {
            response.StatusMsg =  "The type of append file is not supported!";
            response.StatusCode = StatusOperationFailed;
            return;
        }
        if (request->db_name.empty())
        {
            response.StatusMsg =  "db_name can not be empty!";
            response.StatusCode = StatusOperationFailed;
            return;
        }
        if (clusterManagerPtr->isFollowerRestoring(request->db_name))
        {
            response.StatusMsg =  "cluster recover, follower is restoring, please wait......" + request->db_name;
            response.StatusCode = StatusOperationFailed;
            return;
        }
        TermDbLog db_info = clusterManagerPtr->getTermInfoDbLog(request->db_name);
        if (request->updateType == ClusterUpdateType::ClusterUpdateType_Init)
        {
            /*直接拷贝主节点的库进行从节点库的初始化
                1.主节点新建一个库
                2.新启一个从节点, 直接拷贝主节点的现有库进行同步恢复
                3.主节点库和从节点的建库id不一样(主节点做了删库, 重建操作)
            */
            if (request->uid == db_info.getUid())
            {
                response.StatusMsg =  "ok";
                response.StatusCode = StatusOK;
                return;
            }
            cluster_recover_by_init(apiUtil, clusterManagerPtr, request);
        }
        else
        {
            if (request->uid != db_info.getUid())
            {
                response.StatusMsg =  "leader database uid != follower uid, please check term.log, maybe follower is restoring";
                response.StatusCode = StatusOperationFailed;
                return;
            }
            if (request->index == db_info.getIndex())
            {
                response.StatusMsg =  "ok";
                response.StatusCode = StatusOK;
                return;
            }
            cluster_recover_by_index(apiUtil, clusterManagerPtr, request);            
        }
    }

    void ApiHandler::cluster_recover_by_init(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, std::shared_ptr<MessageClusterRecoverRequest> request)
    {
        clusterManagerPtr->addRestoringDb(request->db_name);
        const std::string cluster_init_dir = cluster::ClusterDb::getDbInitDir(request->db_name);
        if (!FileUtil::dirExists(cluster_init_dir))
            FileUtil::createDirs(cluster_init_dir);
        const std::string zip_file_path = cluster_init_dir + request->file_name;
        WFFileIOTask *pwrite_task = WFTaskFactory::create_pwrite_task(zip_file_path, static_cast<const void *>(request->file_content.c_str()), request->file_content.size(), 0, 
            [apiUtil, clusterManagerPtr, zip_file_path, cluster_init_dir, request](WFFileIOTask *pwrite_task)
        {
            std::string db_name = request->db_name;
            CompressUtil::UnCompressZip unzip(zip_file_path, cluster_init_dir);
            if (unzip.unCompress() != CompressUtil::UnZipOK) 
            {
                SLOG_ERROR("uncompress zip file fail: " + zip_file_path);
                // remove zip file
                FileUtil::removePath(zip_file_path);
                clusterManagerPtr->removeRestoringDb(db_name);
                return;
            }

            std::string cluster_db_dir = cluster_init_dir + '/' + db_name;
            std::string db_dir = cluster_db_dir + GlobalTypedef::db_suffix();
            if (!FileUtil::dirExists(db_dir) || !FileUtil::dirExists(cluster_db_dir))
            {
                SLOG_DEBUG("compress dir not datasase file" << db_name);
                // Util::remove_path(cluster_init_dir);
                clusterManagerPtr->removeRestoringDb(db_name);
                return;
            }
            string db_path = GlobalTypedef::db_home() + db_name + GlobalTypedef::db_suffix();
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
            }

            FileUtil::removePath(db_path);
            clusterManagerPtr->dropDb(db_name);

            FileUtil::movePath(db_dir, GlobalTypedef::db_home());

            std::string built_time = gutil::TimeUtil::now(NORM_DATETIME_PATTERN);
            if(!apiUtil->init_databaseinfo(db_name, GlobalTypedef::root_uname(), gutil::TimeUtil::now(NORM_DATETIME_PATTERN), DatabaseStatus::AREADY_BUILT))
            {
                SLOG_ERROR("cluster recover database " + db_name + " fail" << " ,zip name:" << zip_file_path);
                FileUtil::removePath(db_path);
            }
            else
            {
                FileUtil::movePath(cluster_db_dir, cluster::ClusterDb::getClusterDir());
                TermDbLog db_log(db_name, request->uid, request->index, 0, request->recoverIndex);
                clusterManagerPtr->initTermDbLog(db_log);
            }
            clusterManagerPtr->removeRestoringDb(db_name);

        });
        std::thread([pwrite_task](){
            SLOG_DEBUG("saveing recover init log file start...");
            pwrite_task->start();
        }).detach();
    }

    void ApiHandler::cluster_recover_by_index(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, std::shared_ptr<MessageClusterRecoverRequest> request)
    {
        clusterManagerPtr->addRestoringDb(request->db_name);
        const std::string cluster_db_path = clusterManagerPtr->getDbDirPath(request->db_name);
        const std::string zip_file_path = cluster_db_path + request->file_name;
        WFFileIOTask *pwrite_task = WFTaskFactory::create_pwrite_task(zip_file_path, static_cast<const void *>(request->file_content.c_str()), request->file_content.size(), 0, 
            [apiUtil, clusterManagerPtr, zip_file_path, cluster_db_path, request](WFFileIOTask *pwrite_task)
        {
            std::string db_name = request->db_name;
            SLOG_DEBUG("follower recover data ..., database name:" << db_name << " ,index:" << request->recoverIndex);
            // save success
            long ret = pwrite_task->get_retval();
            if (pwrite_task->get_state() != WFT_STATE_SUCCESS || ret < 0)
            {
                clusterManagerPtr->removeRestoringDb(db_name);
                return;
            }
            // unzip file
            CompressUtil::UnCompressZip unzip(zip_file_path, cluster_db_path);
            if (unzip.unCompress() != CompressUtil::UnZipOK) 
            {
                SLOG_ERROR("uncompress zip file fail: " + zip_file_path);
                // remove zip file
                FileUtil::removePath(zip_file_path);
                clusterManagerPtr->removeRestoringDb(db_name);
                return;
            }
            std::vector<std::string> log_files;
            unzip.getFileList(log_files, "");
            if (log_files.empty())
            {
                SLOG_WARN("zip file is empty: " + zip_file_path);
                // remove zip file
                FileUtil::removePath(zip_file_path);
                clusterManagerPtr->removeRestoringDb(db_name);
                return;
            }
            if (apiUtil->check_db_built(db_name) == false) 
            {
                SLOG_WARN("db[" + db_name + "] is not built.");
                clusterManagerPtr->removeRestoringDb(db_name);
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
                FileUtil::removePath(zip_file_path);
                clusterManagerPtr->removeRestoringDb(db_name);
                return;
            }
            std::string log_file_name = FileUtil::fileName(log_files[0]);
            std::string nt_file_path = clusterManagerPtr->getNtFilePath(db_name, log_file_name);
            if (request->updateType == ClusterUpdateType::ClusterUpdateType_Insert)
            {
                // batch insert
                db_info->getDatabase()->batch_insert(nt_file_path);
            }
            else if (request->updateType == ClusterUpdateType::ClusterUpdateType_Delete)
            {
                // batch remove
                db_info->getDatabase()->batch_remove(nt_file_path);
            }
            db_info->getDatabase()->save();
            FileUtil::removePath(zip_file_path);
            FileUtil::removePath(nt_file_path);
            apiUtil->unlock_databaseinfo(db_info);

            // update local log trem and index
            clusterManagerPtr->updateTerm(request->term);
            clusterManagerPtr->addCommitLog(db_name, request->recoverIndex, request->updateType, FileUtil::fileName(log_file_name));
            clusterManagerPtr->removeRestoringDb(db_name);
        });
        std::thread([pwrite_task](){
            SLOG_DEBUG("saveing recover index log file start...");
            pwrite_task->start();
        }).detach();
    }
}