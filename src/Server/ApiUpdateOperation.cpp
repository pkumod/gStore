#include "ApiProvider.h"

namespace server
{
    void ApiHandler::build(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBuildRequest& resquest, MessageBuildResponse& response, const string& remote_ip)
    {
        try
        {
            std::string db_name = resquest.db_name;
            std::string username = resquest.username;
            std::string db_path = resquest.db_path;
            bool async = resquest.async;
            string callback = resquest.callback;
            std::string msg;
            if (!db_path.empty() && Util::file_exist(db_path) == false)
            {
                response.StatusMsg = "RDF file not exist.";
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.StatusMsg = msg;
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            //check the db_name is system
            if (db_name == Util::system_db)
            {
                response.StatusMsg = "The database name can not be system.";
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            // check if database named [db_name] is already built
            if (apiUtil->check_db_built(db_name))
            {
                response.StatusMsg = "database already built.";
                response.StatusCode = StatusOperationConditionsAreNotSatisfied;
                return;
            }
            // check databse number
            if (apiUtil->check_db_count() == false)
            {
                response.StatusMsg = "The total number of databases more than max_databse_num.";
                response.StatusCode = StatusOperationConditionsAreNotSatisfied;
                return;
            }

            shared_ptr<ofstream> clusterlog = nullptr;
            std::string cluster_db_path;
            std::string logpath;
            uint64 log_index;
            if (clusterManagerPtr->isEnable()) 
            {
                // send [prepare] heartbeat and wait response
                ClusterUpdateType cluster_update_type = ClusterUpdateType::ClusterUpdateType_Build;
                log_index = apiUtil->generateUID();
                clusterManagerPtr->addLog(db_name, log_index, ClusterOperation_Prepare, cluster_update_type);
                bool prepare_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Prepare), true);
                if (!prepare_result)
                {
                    clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
                    SLOG_ERROR(msg);
                    response.StatusMsg =  "Less than half of the cluster nodes are confirmed.";
                    response.StatusCode = StatusOperationFailed;
                    return;
                }
                cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
                logpath = cluster_db_path + to_string(log_index) + ".log";
                clusterlog = make_shared<ofstream>();
                clusterlog->open(logpath.c_str());
            }
            apiUtil->init_databaseinfo(db_name, username, Util::get_date_time(), DatabaseStatus::BUILDING);
            std::vector<std::string> zip_files;
            std::string unz_dir_path;
            std::string file_suffix = Util::fileSuffix(db_path);
            bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
            if (is_zip)
            {
                auto code = CompressUtil::FileHelper::foreachZip(db_path,[apiUtil](std::string filename)->bool
                    {
                        if (apiUtil->check_upload_allow_extensions(Util::fileSuffix(filename)) == false)
                            return false;
                        return true;
                    });
                if (code != CompressUtil::UnZipOK)
                {
                    apiUtil->erase_databaseinfo(db_name);
                    response.StatusMsg = "uncompress is failed error.";
                    response.StatusCode = code;
                    return;
                }
                std::string file_name = Util::fileName(db_path);
                size_t pos = file_name.size() - file_suffix.size() - 1;
                unz_dir_path = apiUtil->get_upload_path() + file_name.substr(0, pos) + "_" + Util::getTimeString2();
                mkdir(unz_dir_path.c_str(), 0775);
                CompressUtil::UnCompressZip upfile(db_path, unz_dir_path);
                code = upfile.unCompress();
                if (code != CompressUtil::UnZipOK)
                {
                    Util::remove_path(unz_dir_path);
                    apiUtil->erase_databaseinfo(db_name);
                    response.StatusMsg = "uncompress is failed error.";
                    response.StatusCode = code;
                    return;
                }
                upfile.getFileList(zip_files, "");
            }
            std::string opt_id = apiUtil->generateUid();
            string operation = "build";
            msg = "Operation Success.";
            apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
            auto build_helper = [apiUtil,clusterManagerPtr,db_name,username,unz_dir_path,is_zip,zip_files,db_path,operation,opt_id,async,callback,log_index,clusterlog]
                    (MessageBuildResponse *response)
                    {
                        std::string _db_home = Util::getConfigureValue("db_home");
                        std::string _db_suffix = Util::getConfigureValue("db_suffix");
                        string _db_path = _db_home + db_name + _db_suffix;
                        string database = db_name;
                        SLOG_DEBUG("Import dataset to build database...");
                        SLOG_DEBUG("db_name: " + database + "\tRDF_data: " + db_path);
                        string result;
                        shared_ptr<Database> current_database = make_shared<Database>(database);
                        // build empty database
                        bool flag = current_database->BuildEmptyDB();
                        current_database.reset();
                        int success_num = 0;
                        int nt_file_num = 0;
                        if (flag)
                        {
                            // if zip file then excuse batchInsert
                            if (!db_path.empty() || zip_files.size() > 0)
                            {
                                current_database = make_shared<Database>(db_name);
                                bool rt  = current_database->load(false);
                                if (!rt)
                                {
                                    result = "Import RDF file to database failed: load error.";
                                    clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Drop));
                                    clusterManagerPtr->dropDb(db_name);
                                    Util::remove_path(_db_path);
                                    if (!unz_dir_path.empty())
                                    {
                                        Util::remove_path(unz_dir_path);
                                    }
                                    apiUtil->update_access_log(StatusOperationFailed, result, opt_id, -1, 0, 0);
                                    if (!async)
                                    {
                                        response->StatusMsg = result;
                                        response->StatusCode = StatusOperationFailed;
                                    }
                                    current_database.reset();
                                    return;
                                }
                                if (!db_path.empty() && !is_zip)
                                {
                                    current_database->batch_insert(db_path, false, nullptr, clusterlog);
                                    nt_file_num = 1;
                                }
                                for (std::string rdf_zip : zip_files)
                                {
                                    current_database->batch_insert(rdf_zip, false, nullptr, clusterlog);
                                }
                                nt_file_num += zip_files.size();
                                current_database->save();
                                success_num = current_database->getTripleNum();
                                current_database.reset();
                            }
                        }
                        else
                        {
                            result = "Import RDF file to database failed.";
                            clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Drop));
                            clusterManagerPtr->dropDb(db_name);
                            Util::remove_path(_db_path);
                            if (!unz_dir_path.empty())
                            {
                                Util::remove_path(unz_dir_path);
                            }
                            apiUtil->update_access_log(StatusOperationFailed, result, opt_id, -1, 0, 0);
                            if (response)
                            {
                                response->StatusMsg = result;
                                response->StatusCode = StatusOperationFailed;
                            }
                            return;
                        }	
                        // init databaseinfo
                        shared_ptr<DatabaseInfo> db_info;
                        apiUtil->get_databaseinfo(db_name, db_info);
                        db_info->setStatus(DatabaseStatus::AREADY_BUILT);
                        db_info->initDatabase();
                        // init user privilege
                        apiUtil->init_privilege(username, db_name);
                        ofstream f;
                        f.open(_db_path + "/success.txt");
                        f.close();
                        // add backup.log
                        // Util::add_backuplog(db_name);
                        // build response result
                        result = "Import RDF file to database done.";
                        string error_log = _db_path + "/parse_error.log";
                        size_t parse_error_num = Util::count_lines(error_log);
                        // exclude Info line
                        if (parse_error_num > 0)
                            parse_error_num = parse_error_num - nt_file_num;
                        if (parse_error_num > 0)
                        {
                            SLOG_ERROR("RDF parse error num " + to_string(parse_error_num));
                            SLOG_ERROR("See log file for details " + error_log);
                        }
                        // remove unzip dir
                        if (!unz_dir_path.empty())
                        {
                            Util::remove_path(unz_dir_path);
                        }
                        // Util::add_backuplog(db_name);
                        apiUtil->update_access_log(0, result, opt_id, 1, success_num, parse_error_num);
                        MessageBuildResponse resp_data;
                        resp_data.StatusCode = StatusOK;
                        resp_data.StatusMsg = result;
                        resp_data.failed_num = parse_error_num;
                        resp_data.opt_id = opt_id;
                        std::string json_str;
                        resp_data.toJsonString(json_str);
                        if (clusterManagerPtr->isEnable()) 
                        {
                            // cluster sync task begin
                            string log_file_name = to_string(log_index) + ".log";
                            if (success_num > 0)
                            {
                                SLOG_DEBUG("add log appendEntities task, copy num " + to_string(success_num));
                                string tmp_dir_path = unz_dir_path;
                                bool append_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Append, ClusterUpdateType_Insert, log_file_name), true);
                                if (append_result)
                                {
                                    SLOG_DEBUG("response result:\n" << json_str);
                                    clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Commit));
                                    if (response)
                                    {
                                        *response = resp_data;
                                    }
                                    if (!callback.empty())
                                    {
                                        string res;
                                        HttpUtil::Post(callback, json_str, res);
                                    }
                                    if (!tmp_dir_path.empty())
                                    {
                                        Util::remove_path(tmp_dir_path);
                                    }
                                }
                                else
                                {
                                    // follower recover by heartbeat compare
                                    SLOG_DEBUG("build db follower recover by heartbeat compare:" << db_name);
                                    *response = resp_data;
                                }
                            }
                            else
                            {
                                SLOG_DEBUG("No data needs to be synchronized, update log stauts to committed");
                                clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
                                Util::remove_path(clusterManagerPtr->getDbDirPath(db_name)+log_file_name);
                                // remove unzip files
                                if (!unz_dir_path.empty())
                                {
                                    Util::remove_path(unz_dir_path);
                                }
                                if (response != nullptr)
                                {
                                    *response = resp_data;
                                }
                            }
                            // cluster sync task end
                        }
                        else
                        {
                            if (response)
                            {
                                *response = resp_data;
                            }
                            if (!callback.empty())
                            {
                                string res;
                                HttpUtil::Post(callback, json_str, res);
                            }
                        }
                        
                    };
            if (async)
            {
                response.StatusCode = StatusOK;
                response.StatusMsg = msg;
                response.opt_id = "opt_id";
                thread t(build_helper, nullptr);
                t.detach();
            }
            else
            {
                build_helper(&response);
            }
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Build fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }

    void ApiHandler::batch_insert(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBatchInsertRequest& resquest, MessageBatchInsertResponse& response, const string& remote_ip)
    {
        try
        {
            std::string db_name = resquest.db_name;
            std::string dir = resquest.dir;
            std::string file = resquest.file;
            bool async = resquest.async;
            std::string callback = resquest.callback;
            std::string _db_home = Util::getConfigureValue("db_home");
            std::string _db_suffix = Util::getConfigureValue("db_suffix");
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.StatusCode = StatusParamIsIllegal;
                response.StatusMsg = msg;
                return;
            }
            bool is_file = true;
            if (apiUtil->check_param_value("file", file, msg) == false)
            {
                is_file = false;
                if (apiUtil->check_param_value("dir", dir, msg) == false)
                {
                    response.StatusMsg = "file and dir cannot be empty at the same time!";
                    response.StatusCode = StatusParamIsIllegal;
                    return;
                }
            }
            if (is_file && Util::file_exist(file) == false)
            {
                response.StatusMsg = "The data file is not exist";
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            if (!is_file && Util::file_exist(dir) == false)
            {
                response.StatusMsg = "The data directory is not exist";
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            if (apiUtil->check_db_built(db_name) == false)
            {
                response.StatusMsg = "Database not built yet.";
                response.StatusCode = StatusOperationConditionsAreNotSatisfied;
                return;
            }
            if (apiUtil->check_db_loaded(db_name) == false)
            {
                msg = "Database not load yet.";
                response.StatusCode = StatusOperationConditionsAreNotSatisfied;
                return;
            }
            shared_ptr<ofstream> clusterlog = nullptr;
            std::string cluster_db_path;
            std::string logpath;
            uint64 log_index;
            if (clusterManagerPtr->isEnable()) 
            {
                // send [prepare] heartbeat and wait response
                ClusterUpdateType cluster_update_type = ClusterUpdateType::ClusterUpdateType_Insert;
                log_index = apiUtil->generateUID();
                clusterManagerPtr->addLog(db_name, log_index, ClusterOperation_Prepare, cluster_update_type);
                bool prepare_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Prepare), true);
                if (!prepare_result)
                {
                    clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
                    response.StatusMsg = "Less than half of the cluster nodes are confirmed.";
                    response.StatusCode = StatusOperationFailed;
                    SLOG_ERROR(msg);
                    return;
                }
                cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
                logpath = cluster_db_path + to_string(log_index) + ".log";
                clusterlog = make_shared<ofstream>();
                clusterlog->open(logpath.c_str());
            }
            
            std::vector<std::string> nt_files;
            std::string unz_dir_path;
            if (is_file)
            {
                std::string file_suffix = Util::fileSuffix(file);
                bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
                if (is_zip)
                {
                    auto code = CompressUtil::FileHelper::foreachZip(file,[apiUtil](std::string filename)->bool
                        {
                            if( apiUtil->check_upload_allow_extensions(Util::fileSuffix(filename)) == false )
                                return false;
                            return true;
                        });
                    if( code != CompressUtil::UnZipOK )
                    {
                        response.StatusCode = (StatusCode)code;
                        response.StatusMsg = "uncompress is failed error.";
                        if (clusterlog)
                            clusterlog->close();
                        return;
                    }
                    std::string file_name = Util::fileName(file);
                    size_t pos = file_name.size() - file_suffix.size() - 1;
                    unz_dir_path = apiUtil->get_upload_path() + file_name.substr(0, pos) + "_" + Util::getTimeString2();
                    Util::create_dirs(unz_dir_path);
                    CompressUtil::UnCompressZip upfile(file, unz_dir_path);
                    code = upfile.unCompress();
                    if (code != CompressUtil::UnZipOK)
                    {
                        Util::remove_path(unz_dir_path);
                        response.StatusCode = (StatusCode)code;
                        response.StatusMsg = "uncompress is failed error.";
                        if (clusterlog)
                            clusterlog->close();
                        return;
                    }
                    upfile.getFileList(nt_files, "");
                }
                else
                {
                    nt_files.push_back(file);
                }
            }
            else
            {
                // is dirctory
                Util::string_suffix(dir, '/');
                Util::dir_files(dir, "", nt_files);
            }
            std::string opt_id = apiUtil->generateUid();
            auto insert_helper = [db_name, nt_files, unz_dir_path, opt_id, async, callback, log_index, clusterlog, apiUtil, remote_ip, clusterManagerPtr, _db_home, _db_suffix](MessageBatchInsertResponse *response) {
                shared_ptr<DatabaseInfo> db_info;
                apiUtil->get_databaseinfo(db_name, db_info);
                // access log
                string msg = "Operation Success.";
                string operation = "batchInsert";
                if (!apiUtil->trywrlock_databaseinfo(db_info, 300))
                {
                    msg = "Unable to batch insert due to loss of lock.";
                    apiUtil->write_access_log(operation, remote_ip, StatusLossOfLock, msg, opt_id);
                    if (response)
                    {
                        response->StatusCode = StatusLossOfLock;
                        response->StatusMsg = msg;
                    }
                    return;
                }
                apiUtil->write_access_log(operation, remote_ip, StatusOK, msg, opt_id);
                unsigned success_num = 0;
                unsigned total_num = 0;
                unsigned parse_error_num = 0;
                string error_log = _db_home +  "/" + db_info->getName() + _db_suffix + "/parse_error.log";
                total_num = Util::count_lines(error_log);
                for (std::string rdf_file : nt_files)
                {
                    SLOG_DEBUG("begin insert data from " + rdf_file);
                    success_num += db_info->getDatabase()->batch_insert(rdf_file, false, nullptr, clusterlog);
                }
                // exclude Info line
                parse_error_num = Util::count_lines(error_log) - total_num - nt_files.size();
                // save data and unlock
                db_info->getDatabase()->save();
                apiUtil->unlock_databaseinfo(db_info);
                // close cluster log
                if (clusterlog) 
                {
                    clusterlog->close();
                }
                // update access log
                msg = "Batch insert data successfully.";
                apiUtil->update_access_log(StatusOK, "Batch insert data successfully.", opt_id, 1, success_num, parse_error_num);
                // respnse data
                MessageBatchInsertResponse resp_data;
                resp_data.StatusCode = StatusOK;
                resp_data.StatusMsg = msg;
                resp_data.successNum = success_num;
                resp_data.failedNum = parse_error_num;
                resp_data.opt_id = opt_id;
                std::string json_str;
                resp_data.toJsonString(json_str);
                
                if (clusterManagerPtr->isEnable()) 
                {
                    // cluster sync task begin
                    string log_file_name = to_string(log_index) + ".log";
                    if (success_num > 0)
                    {
                        SLOG_DEBUG("add log appendEntities task, copy num " + to_string(success_num));
                        string tmp_dir_path = unz_dir_path;
                        bool append_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Append, ClusterUpdateType_Insert, log_file_name), true);
                        if (append_result)
                        {
                            SLOG_DEBUG("response result:\n" << json_str);
                            clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Commit));
                            if (response)
                            {
                                *response = resp_data;
                            }
                            if (!callback.empty())
                            {
                                string res;
                                HttpUtil::Post(callback, json_str, res);
                            }
                            if (!tmp_dir_path.empty())
                            {
                                Util::remove_path(tmp_dir_path);
                            }
                        }
                        else
                        {
                            // restore data
                            bool lock_status;
                            // try get wrlock timeout 600 senconds
                            if (apiUtil->trywrlock_databaseinfo(db_info, 600))
                            {

                                uint64_t num = 0;
                                for (std::string rdf_file : nt_files)
                                {
                                    num += db_info->getDatabase()->batch_remove(rdf_file);
                                }
                                SLOG_INFO("restore " + db_name + " data: batch_remove num " << num);
                                apiUtil->unlock_databaseinfo(db_info);
                            }
                            else
                            {
                                SLOG_ERROR("restore " + db_name + " data failed: unable get wrlock, log[" + log_file_name + "], operation[1]");
                            }
                            if (!tmp_dir_path.empty())
                            {
                                Util::remove_path(tmp_dir_path);
                            }
                            msg = "Less than half of the cluster nodes reply.";
                            SLOG_ERROR(msg);
                            clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Cancel));
                            resp_data.StatusMsg = msg;
                            resp_data.StatusCode = StatusOperationFailed;
                            resp_data.toJsonString(json_str);
                            if (response)
                            {
                                *response = resp_data;
                            }
                            if (!callback.empty())
                            {
                                string res;
                                HttpUtil::Post(callback, json_str, res);
                            }
                        }
                    }
                    else
                    {
                        SLOG_DEBUG("No data needs to be synchronized, update log stauts to failed");
                        clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
                        Util::remove_path(clusterManagerPtr->getDbDirPath(db_name)+log_file_name);
                        // remove unzip files
                        if (!unz_dir_path.empty())
                        {
                            Util::remove_path(unz_dir_path);
                        }
                        if (response != nullptr)
                        {
                            *response = resp_data;
                        }
                        if (!callback.empty())
                        {
                            string res;
                            resp_data.toJsonString(json_str);
                            HttpUtil::Post(callback, json_str, res);
                            // TODO retry?
                        }
                    }
                    // cluster sync task end
                }
                else
                {
                    *response = resp_data;
                }
            };
            if (async)
            {
                // grpc::GRPCServerTask* sub_task = task_of(response);
                // sub_task->add_callback([&](GRPCTask *task) {
                    
                // });
                response.StatusCode = StatusOK;
                response.StatusMsg = "Operation success";
                response.opt_id = "opt_id";
                thread t(insert_helper, nullptr);
                t.detach();
            }
            else
            {
                insert_helper(&response);
            }
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Batch insert fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }

    void ApiHandler::batch_remove(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBatchRemoveRequest& resquest, MessageBatchRemoveResponse& response, const string& remote_ip)
    {
        try
        {
            std::string db_name = resquest.db_name;
            std::string file = resquest.file;
            bool async = resquest.async;
            std::string callback = resquest.callback;
            std::string _db_home = Util::getConfigureValue("db_home");
            std::string _db_suffix = Util::getConfigureValue("db_suffix");
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.StatusMsg = msg;
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            if (apiUtil->check_param_value("file", file, msg) == false)
            {
                response.StatusMsg = msg;
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            if (Util::file_exist(file) == false)
            {
                response.StatusMsg = "The data file is not exist";
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            if (apiUtil->check_db_built(db_name) == false)
            {
                response.StatusMsg = "Database not built yet.";
                response.StatusCode = StatusOperationConditionsAreNotSatisfied;
                return;
            }
            if (apiUtil->check_db_loaded(db_name) == false)
            {
                response.StatusMsg = "Database not load yet.";
                response.StatusCode = StatusOperationConditionsAreNotSatisfied;
                return;
            }
            shared_ptr<ofstream> clusterlog = nullptr;
            std::string cluster_db_path;
            std::string logpath;
            uint64 log_index;
            if (clusterManagerPtr->isEnable()) 
            {
                // send [prepare] heartbeat and wait response
                ClusterUpdateType cluster_update_type = ClusterUpdateType::ClusterUpdateType_Delete;
                log_index = apiUtil->generateUID();
                clusterManagerPtr->addLog(db_name, log_index, ClusterOperation_Prepare, cluster_update_type);
                bool prepare_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Prepare), true);
                if (!prepare_result)
                {
                    clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
                    response.StatusMsg = "Less than half of the cluster nodes are confirmed.";
                    response.StatusCode = StatusOperationFailed;
                    SLOG_ERROR(msg);
                    return;
                }
                cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
                logpath = cluster_db_path + to_string(log_index) + ".log";
                clusterlog = make_shared<ofstream>();
                clusterlog->open(logpath.c_str());
            }
            std::vector<std::string> nt_files;
            std::string unz_dir_path;
            std::string file_suffix = Util::fileSuffix(file);
            bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
            if (is_zip)
            {
                auto code = CompressUtil::FileHelper::foreachZip(file,[apiUtil](std::string filename)->bool
                    {
                        if( apiUtil->check_upload_allow_extensions(Util::fileSuffix(filename)) == false )
                            return false;
                        return true;
                    });
                if( code != CompressUtil::UnZipOK )
                {
                    response.StatusMsg = "uncompress is failed error.";
                    response.StatusCode = code, msg;
                    if (clusterlog)
                        clusterlog->close();
                    return;
                }
                std::string file_name = Util::fileName(file);
                size_t pos = file_name.size() - file_suffix.size() - 1;
                unz_dir_path = apiUtil->get_upload_path() + file_name.substr(0, pos) + "_" + Util::getTimeString2();
                Util::create_dirs(unz_dir_path);
                CompressUtil::UnCompressZip upfile(file, unz_dir_path);
                code = upfile.unCompress();
                if (code != CompressUtil::UnZipOK)
                {
                    Util::remove_path(unz_dir_path);
                    response.StatusMsg = "uncompress is failed error.";
                    response.StatusCode = code;
                    if (clusterlog)
                        clusterlog->close();
                    return;
                }
                upfile.getFileList(nt_files, "");
            }
            else
            {
                nt_files.push_back(file);
            }
            std::string opt_id = apiUtil->generateUid();
            auto remove_helper = [apiUtil,clusterManagerPtr,db_name, nt_files, &unz_dir_path, opt_id, async, callback, log_index, clusterlog,remote_ip,_db_home,_db_suffix](MessageBatchRemoveResponse *response){
                shared_ptr<DatabaseInfo> db_info;
                apiUtil->get_databaseinfo(db_name, db_info);
                // access log
                string msg = "Operation Success.";
                string operation = "batchRemove";
                if (!apiUtil->trywrlock_databaseinfo(db_info, 300))
                {
                    apiUtil->write_access_log(operation, remote_ip, StatusLossOfLock, msg, opt_id);
                    if (response)
                    {
                        response->StatusMsg = "Unable to batch remove due to loss of lock.";
                        response->StatusCode = StatusLossOfLock;
                    }
                    return;
                }
                apiUtil->write_access_log(operation, remote_ip, StatusOK, msg, opt_id);
                unsigned success_num = 0;
                unsigned total_num = 0;
                size_t parse_error_num = 0;
                string error_log = _db_home +  "/" + db_name + _db_suffix + "/parse_error.log";
                total_num = Util::count_lines(error_log);
                for (std::string rdf_file : nt_files)
                {
                    SLOG_DEBUG("begin remove data from " + rdf_file);
                    success_num += db_info->getDatabase()->batch_remove(rdf_file, false, nullptr, clusterlog);
                }
                // exclude Info line
                parse_error_num = Util::count_lines(error_log) - total_num - nt_files.size();
                // save data and unlock
                db_info->getDatabase()->save();
                apiUtil->unlock_databaseinfo(db_info);
                // close cluster log
                if (clusterlog) 
                {
                    clusterlog->close();
                }
                // update access log
                msg = "Batch remove data successfully.";
                apiUtil->update_access_log(0, msg, opt_id, 1, success_num, parse_error_num);

                // respnse data
                MessageBatchRemoveResponse resp_data;
                resp_data.StatusCode = StatusOK;
                resp_data.StatusMsg = msg;
                resp_data.successNum = success_num;
                resp_data.failedNum = parse_error_num;
                resp_data.opt_id = opt_id;
                std::string json_str;
                resp_data.toJsonString(json_str);
                
                if (clusterManagerPtr->isEnable()) 
                {
                    // cluster sync task begin
                    string log_file_name = to_string(log_index) + ".log";
                    if (success_num > 0)
                    {
                        SLOG_DEBUG("add log appendEntities task, copy num " + to_string(success_num));
                        string tmp_dir_path = unz_dir_path;
                        bool append_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Append, ClusterUpdateType_Delete, log_file_name), true);
                        if (append_result)
                        {
                            SLOG_DEBUG("response result:\n" << json_str);
                            clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Commit));
                            if (response)
                            {
                                *response = resp_data;
                            }
                            if (!callback.empty())
                            {
                                string res;
                                HttpUtil::Post(callback, json_str, res);
                            }
                            if (!tmp_dir_path.empty())
                            {
                                Util::remove_path(tmp_dir_path);
                            }
                        }
                        else
                        {
                            // restore data
                            // try get wrlock timeout 600 senconds
                            if (apiUtil->trywrlock_databaseinfo(db_info, 600))
                            {
                                uint64_t num = 0;
                                for (std::string rdf_file : nt_files)
                                {
                                    num += db_info->getDatabase()->batch_insert(rdf_file);
                                }
                                SLOG_INFO("restore " + db_name + " data: batch_insert num " << num);
                                apiUtil->unlock_databaseinfo(db_info);
                            }
                            else
                            {
                                SLOG_ERROR("restore " + db_name + " data failed: unable get wrlock, log[" + log_file_name + "], operation[2]");
                            }
                            if (!tmp_dir_path.empty())
                            {
                                Util::remove_path(tmp_dir_path);
                            }
                            msg = "Less than half of the cluster nodes reply.";
                            SLOG_ERROR(msg);
                            clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Cancel));
                            resp_data.StatusMsg = msg;
                            resp_data.StatusCode = StatusOperationFailed;
                            resp_data.toJsonString(json_str);
                            if (response)
                            {
                                *response = resp_data;
                            }
                            if (!callback.empty())
                            {
                                string res;
                                HttpUtil::Post(callback, json_str, res);
                            }
                        }
                    }
                    else
                    {
                        SLOG_DEBUG("No data needs to be synchronized, update log stauts to failed");
                        clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
                        Util::remove_path(clusterManagerPtr->getDbDirPath(db_name)+log_file_name);
                        // remove unzip files
                        if (!unz_dir_path.empty())
                        {
                            Util::remove_path(unz_dir_path);
                        }
                        if (response != nullptr)
                        {
                            *response = resp_data;
                        }
                        if (!callback.empty())
                        {
                            string res;
                            HttpUtil::Post(callback, json_str, res);
                            // TODO retry?
                        }
                    }
                    // cluster sync task end
                }
                else
                {
                    *response = resp_data;
                }
            };
            if (async)
            {
                response.StatusCode = StatusOK;
                response.StatusMsg = "Operation success";
                response.opt_id = "opt_id";
                thread t(remove_helper, nullptr);
                t.detach();
            }
            else
            {
                remove_helper(&response);
            }
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Batch remove fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }

    void ApiHandler::drop(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageDropRequest& resquest, MessageDropResponse& response)
    {
        try
        {
            std::string db_name = resquest.db_name;
            bool is_backup = stringIsTrue(resquest.is_backup);
            std::string _db_home = Util::getConfigureValue("db_home");
            std::string _db_suffix = Util::getConfigureValue("db_suffix");
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.StatusMsg = msg;
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            if (apiUtil->check_db_built(db_name) == false)
            {
                response.StatusMsg = "the database [" + db_name + "] not built yet.";
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            if (apiUtil->check_db_loaded(db_name))
            {
                apiUtil->remove_txn_manager(db_name, false);
                SLOG_DEBUG("remove " + db_name + " from the txn managers.");
            }
            if (apiUtil->remove_databaseinfo(db_name, msg) == false)
            {
                response.StatusMsg = msg;
                response.StatusCode = StatusOperationFailed;
                return;
            }
            SLOG_DEBUG("remove " + db_name + " from the already build database list success.");
            string db_path = _db_home + db_name + _db_suffix;
            if (is_backup == false)
            {
                Util::remove_path(db_path);
                SLOG_DEBUG("remove_path"+db_path);
            }
            else
            {
                std::string cmd = "mv " + db_path + " " + _db_home + db_name + ".bak";
                SLOG_DEBUG(cmd);
                system(cmd.c_str());
            }
            string success = "Database " + db_name + " dropped.";
            clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Drop));
            clusterManagerPtr->dropDb(db_name);
            response.StatusMsg = success;
            response.StatusCode = StatusOK;
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Drop fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }

    void ApiHandler::query(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageQueryRequest& resquest, MessageQueryResponse& response, bool &redirect, bool &is_query, const DbQueryLogCall& db_queryLog_cb)
    {
        try
        {
            std::string min_memory = Util::getConfigureValue("min_memory").c_str();
            int memoryLeft = Util::memoryLeft();
            if (memoryLeft < atoi(min_memory.c_str()))
            {
                response.StatusMsg = "memory not enough, available:" + std::to_string(memoryLeft) + "GB, need minimum:" + min_memory + "GB";
                response.StatusCode = StatusOperationFailed;
                return;
            }
            std::string db_name = resquest.db_name;
            std::string format = resquest.format;
            std::string username = resquest.username;
            std::string sparql = resquest.sparql;
            // check db_name paramter
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.StatusMsg = msg;
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            // check sparql paramter
            if (apiUtil->check_param_value("sparql", sparql, msg) == false)
            {
                response.StatusMsg = msg;
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            // check database exist
            if (apiUtil->check_db_built(db_name) == false)
            {
                response.StatusMsg = "Database not build yet.";
                response.StatusCode = StatusOperationConditionsAreNotSatisfied;
                return;
            }
            // check database load status
            if (apiUtil->check_db_loaded(db_name) == false)
            {
                response.StatusMsg = "Database not load yet.";
                response.StatusCode = StatusOperationConditionsAreNotSatisfied;
                return;
            }
            shared_ptr<DatabaseInfo> db_info;
            apiUtil->get_databaseinfo(db_name, db_info);
            // check database read lock
            if (apiUtil->rdlock_databaseinfo(db_info) == false)
            {
                response.StatusMsg = "get current database read lock fail.";
                response.StatusCode = StatusLossOfLock;
                return;
            }
            string thread_id = Util::getThreadID();
            bool is_update = false;
            QueryTree::UpdateType update_type;
            bool update_flag_bool = apiUtil->check_privilege(username, "update", db_name);
            // check update operation
            try
            {
                is_update = db_info->getDatabase()->isUpdate(sparql, update_type);
            }
            catch(const std::exception& e)
            {
                apiUtil->unlock_databaseinfo(db_info);
                response.StatusMsg = e.what();
                response.StatusCode = StatusOperationFailed;
                return;
            }
            if(clusterManagerPtr->isEnable() && clusterManagerPtr->isFollower() && is_update)
            {
                apiUtil->unlock_databaseinfo(db_info);
                redirect = true;
                return;
            }
            FILE *output = NULL;
            ResultSet rs;
            int ret_val;
            int query_time = Util::get_cur_time();
            shared_ptr<ofstream> clusterlog = nullptr;
            std::string cluster_db_path;
            std::string logpath;
            uint64 log_index;
            ClusterUpdateType cluster_update_type = ClusterUpdateType::ClusterUpdateType_None;
            // update waiting follower reply
            if (clusterManagerPtr->isEnable() && is_update) 
            {
                // send [prepare] heartbeat and wait response
                if (update_type == QueryTree::UpdateType::Insert_Data || update_type  == QueryTree::UpdateType::Insert_Clause) 
                    cluster_update_type = ClusterUpdateType::ClusterUpdateType_Insert;
                else
                    cluster_update_type = ClusterUpdateType::ClusterUpdateType_Delete;
                log_index = apiUtil->generateUID();
                clusterManagerPtr->addLog(db_name, log_index, ClusterOperation_Prepare, cluster_update_type);
                bool prepare_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Prepare), true);
                if (!prepare_result)
                {
                    apiUtil->unlock_databaseinfo(db_info);
                    clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
                    response.StatusMsg = "Less than half of the cluster nodes are confirmed.";
                    response.StatusCode = StatusOperationFailed;
                    SLOG_ERROR(msg);
                    return;
                }
                cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
                logpath = cluster_db_path + to_string(log_index) + ".log";
                clusterlog = make_shared<ofstream>();
                clusterlog->open(logpath.c_str());
            }

            // set query_start_time
            std::string query_start_time;
            struct timeval tv;
            gettimeofday(&tv, NULL);
            int s = tv.tv_usec / 1000;
            int y = tv.tv_usec % 1000;
            query_start_time = Util::get_date_time() + ":" + Util::int2string(s) + "ms" + ":" + Util::int2string(y) + "microseconds";
            try
            {
                SLOG_DEBUG("begin query...\n" + sparql);
                rs.setUsername(username);
                ret_val = db_info->getDatabase()->query(sparql, rs, output, update_flag_bool, false, nullptr, clusterlog);
                query_time = Util::get_cur_time() - query_time;
                if (clusterlog) 
                {
                    clusterlog->close();
                    clusterlog.reset();
                }
                // unlock rdlock
                apiUtil->unlock_databaseinfo(db_info);
            } catch (const std::exception &e) {
                apiUtil->unlock_databaseinfo(db_info);
                response.StatusMsg = "Query fail: " + string(e.what());
                response.StatusCode = StatusOperationFailed;
                if (clusterlog)
                    clusterlog->close();
                return;
            }
            string filename = thread_id + "_" + Util::getTimeString2() + "_" + Util::int2string(Util::getRandNum()) + ".txt";
            string localname = apiUtil->get_query_result_path() + filename;
            string query_time_s = Util::int2string(query_time);
            if (!is_update && (ret_val == -100))
            {
                // SLOG_DEBUG(thread_id + ":search query returned successfully.");

                // record each query operation, including the sparql and the answer number
                // accurate down to microseconds
                // filter the IP from the test server
                is_query = true;
                std::string remote_ip = resquest.remote_ip;
                long rs_ansNum = max((long)rs.ansNum - rs.output_offset, 0L);
                long rs_outputlimit = (long)rs.output_limit;
                if (rs_outputlimit != -1)
                {
                    rs_ansNum = min(rs_ansNum, rs_outputlimit);
                }	

                int StatusCode = 0;
                string file_name = "";
                if (format.find("file") != string::npos)
                {
                    file_name = string(filename.c_str());
                }
                // add callback task for query log start
                struct DBQueryLogInfo* query_log_ptr = new DBQueryLogInfo(query_start_time, remote_ip, sparql, 
                    rs_ansNum, format, file_name, StatusCode, query_time, db_name);
                db_queryLog_cb(query_log_ptr);

                // to void someone downloading all the data file by sparql query on purpose and to protect the data
                // if the ansNum too large, for example, larger than 100000, we limit the return ans.
                if (rs_ansNum > apiUtil->get_max_output_size())
                {
                    if (rs_outputlimit == -1 || rs_outputlimit > apiUtil->get_max_output_size())
                    {
                        rs_outputlimit = apiUtil->get_max_output_size();
                    }
                }

                ofstream outfile;
                string ans = "";
                if (format == "json")
                {
                    rs.to_JSON(response.query_json);
                    string success = response.query_json.dump();
                    rs.release();
                    try
                    {
                        nlohmann::json parse_results_ = nlohmann::json::parse(success);
                        response.StatusCode = StatusOK;
                        response.StatusMsg = "success";
                        response.ansNum = rs_ansNum;
                        response.outputLimit = rs_outputlimit;
                        response.queryTime = query_time_s;
                    }
                    catch (nlohmann::json::exception& e)
                    {
                        string filename2 = "error_" + filename;
                        string localname2 = apiUtil->get_query_result_path() + filename2;
                        outfile.open(localname2);
                        outfile << success;
                        outfile.close();
                        SLOG_ERROR("result parse error: ErrorCode=" << e.id << ", ErrorPosition=" << e.what() << ", ResultFile=" << localname2);
                        response.StatusMsg = "Query fail: the result parse error.";
                        response.StatusCode = StatusOperationFailed;
                    }
                }
                else if (format == "file")
                {
                    outfile.open(localname);
                    outfile << rs.to_JSON();
                    outfile.close();
                    rs.release();
                    response.StatusMsg = "success";
                    response.StatusCode = StatusOK;
                    response.ansNum = rs_ansNum;
                    response.outputLimit = rs_outputlimit;
                    response.queryTime = query_time_s;
                    response.fileName = filename;
                }
                else if (format == "n-triple")
                {
                    // headers
                    nlohmann::json json_data;
                    json_data["head"] = nlohmann::json::array();
                    for(int i = 0; i < rs.true_select_var_num; i++)
                    {
                        json_data["head"].emplace_back(rs.var_name[i]);
                    }
                    // results
                    json_data["results"] = nlohmann::json::array();
                    for(int i = rs.output_offset; i < rs.ansNum; i++)
                    {
                        if (rs.output_limit != -1 && i == rs.output_offset + rs.output_limit)
                        {
                            break;
                        }	
                        if (i >= rs.output_offset)
                        {
                            std::vector<std::string> result_data;
                            for(int j = 0; j < rs.true_select_var_num; j++)
                            {
                                result_data.emplace_back(rs.answer[i][j]);
                            }
                            json_data["results"].emplace_back(result_data);
                        }
                    }
                    rs.release();
                    response.query_json = json_data;
                    response.StatusCode = StatusOK;
                    response.StatusMsg = "success";
                    response.ansNum = rs_ansNum;
                    response.outputLimit = rs_outputlimit;
                    response.queryTime = query_time_s;
                }
                else
                {
                    response.StatusMsg = "Unknown result format.";
                    response.StatusCode = StatusOperationFailed;
                }
                response.threadId = thread_id;
            }
            else if (is_update)
            {
                SLOG_DEBUG("update query returns true. update num " + to_string(ret_val));
                response.StatusCode = StatusOK;
                response.StatusMsg = "update query returns true.";
                response.ansNum = ret_val;
                response.queryTime = query_time_s;
                response.threadId = thread_id;
                response.isUpdate = true;
                std::string json_str;
                response.toJsonString(json_str);
                if (clusterManagerPtr->isEnable())
                {
                    // add log appendEntities task
                    string log_file_name = to_string(log_index) + ".log";
                    if (ret_val > 0)
                    {
                        SLOG_DEBUG("add log appendEntities task, copy num " + to_string(ret_val));
                        bool append_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Append, cluster_update_type, log_file_name), true);
                        if (append_result)
                        {
                            SLOG_DEBUG("response result:\n" << json_str);
                            clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Commit));
                        }
                        else
                        {
                            // restore data
                            SLOG_DEBUG("log appendEntities task failed, restore leader data.");
                            // try get wrlock timeout 600 senconds
                            if (apiUtil->trywrlock_databaseinfo(db_info, 600))
                            {
                                string nt_file_path = clusterManagerPtr->getNtFilePath(db_name, log_file_name);
                                if (cluster_update_type == ClusterUpdateType::ClusterUpdateType_Delete)
                                {
                                    uint32_t num = db_info->getDatabase()->batch_insert(nt_file_path);
                                    SLOG_INFO("restore " + db_name + " data: batch insert num " << num);
                                } 
                                else 
                                {
                                    uint32_t num = db_info->getDatabase()->batch_remove(nt_file_path);
                                    SLOG_INFO("restore " + db_name + " data: batch_remove num " << num);
                                }
                                apiUtil->unlock_databaseinfo(db_info);
                                Util::remove_path(nt_file_path);
                            }
                            else
                            {
                                SLOG_ERROR("restore " + db_name + " data failed: unable get wrlock, log[" + log_file_name + "], operation["+to_string(cluster_update_type)+"]");
                            }
                            msg = "Less than half of the cluster nodes reply.";
                            SLOG_ERROR(msg);
                            clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Cancel));
                            response.StatusMsg = msg;
                            response.StatusCode = StatusOperationFailed;
                        }
                    }
                    else
                    {
                        SLOG_DEBUG("No data needs to be synchronized, update log stauts to failed");
                        clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
                        Util::remove_path(clusterManagerPtr->getDbDirPath(db_name)+log_file_name);
                    }
                }
            }
            else
            {
                msg = "search query returns false.";
                SLOG_DEBUG(msg);
                response.StatusMsg = msg;
                response.StatusCode = StatusOperationFailed;
            }
            SLOG_DEBUG("query complete!");
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Query fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }

    void ApiHandler::checkpoint(shared_ptr<APIUtil>& apiUtil, const server::MessageCheckPointRequest& resquest, server::MessageResponse& response)
    {
        try
        {
            std::string db_name = resquest.db_name;
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            if (apiUtil->check_db_built(db_name) == false)
            {
                msg = "Database not built yet.";
                response.Error(StatusOperationConditionsAreNotSatisfied, msg);
                return;
            }
            if (apiUtil->check_db_loaded(db_name) == false)
            {
                msg = "Database not load yet.";
                response.Error(StatusOperationConditionsAreNotSatisfied, msg);
                return;
            }
            shared_ptr<DatabaseInfo> db_info;
            apiUtil->get_databaseinfo(db_name, db_info);
            if (apiUtil->trywrlock_databaseinfo(db_info) == false)
            {
                msg = "Unable to checkpoint due to loss of lock.";
                response.Error(StatusLossOfLock, msg);
                return;
            }
            shared_ptr<Txn_manager> txn_m;
            if(apiUtil->get_txn_manager(db_name, txn_m) == false)
            {
                msg = "Get database transaction manager error.";
                apiUtil->unlock_databaseinfo(db_info);
                response.Error(StatusTranscationManageFailed, msg);
                return;
            }
            txn_m->Checkpoint();
            db_info->getDatabase()->save();
            apiUtil->unlock_databaseinfo(db_info);
            response.StatusMsg = "Database saved successfully.";
        }
        catch (const std::exception &e)
        {
            string error = "Checkpoint fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }
}