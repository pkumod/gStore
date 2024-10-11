#include "ApiProvider.h"

namespace server
{
    void ApiHandler::batch_insert(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBatchInsertRequest& resquest, MessageBatchInsertResponse& response, const string& remote_ip, const std::string& _db_home, const std::string& _db_suffix)
    {
        try
        {
            std::string db_name = resquest.db_name;
            std::string dir = resquest.dir;
            std::string file = resquest.file;
            bool async = resquest.async;
            std::string callback = resquest.callback;
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.status_code = StatusParamIsIllegal;
                response.status_msg = msg;
                return;
            }
            bool is_file = true;
            if (apiUtil->check_param_value("file", file, msg) == false)
            {
                is_file = false;
                if (apiUtil->check_param_value("dir", dir, msg) == false)
                {
                    response.status_msg = "file and dir cannot be empty at the same time!";
                    response.status_code = StatusParamIsIllegal;
                    return;
                }
            }
            if (is_file && Util::file_exist(file) == false)
            {
                response.status_msg = "The data file is not exist";
                response.status_code = StatusParamIsIllegal;
                return;
            }
            if (!is_file && Util::file_exist(dir) == false)
            {
                response.status_msg = "The data directory is not exist";
                response.status_code = StatusParamIsIllegal;
                return;
            }
            if (apiUtil->check_db_built(db_name) == false)
            {
                response.status_msg = "Database not built yet.";
                response.status_code = StatusOperationConditionsAreNotSatisfied;
                return;
            }
            if (apiUtil->check_db_loaded(db_name) == false)
            {
                msg = "Database not load yet.";
                response.status_code = StatusOperationConditionsAreNotSatisfied;
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
                    response.status_msg = "Less than half of the cluster nodes are confirmed.";
                    response.status_code = StatusOperationFailed;
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
                        response.status_code = (StatusCode)code;
                        response.status_msg = "uncompress is failed error.";
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
                        response.status_code = (StatusCode)code;
                        response.status_msg = "uncompress is failed error.";
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
            auto insert_helper = [&db_name, &nt_files, &unz_dir_path, opt_id, async, callback, &log_index, &clusterlog, apiUtil, remote_ip, clusterManagerPtr, _db_home, _db_suffix](MessageBatchInsertResponse *response) {
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
                        response->status_code = StatusLossOfLock;
                        response->status_msg = msg;
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
                    clusterlog.reset();
                }
                // update access log
                msg = "Batch insert data successfully.";
                apiUtil->update_access_log(StatusOK, "Batch insert data successfully.", opt_id, 1, success_num, parse_error_num);
                // respnse data
                MessageBatchInsertResponse resp_data;
                resp_data.status_code = StatusOK;
                resp_data.status_msg = msg;
                resp_data.success_num = success_num;
                resp_data.failed_num = parse_error_num;
                resp_data.opt_id = opt_id;
                std::string json_str;
                resp_data.toJsonString(json_str);
                
                if (clusterManagerPtr->isEnable()) 
                {
                    // cluster sync task begin
                    if (success_num > 0)
                    {
                        SLOG_DEBUG("add log appendEntities task, copy num " + to_string(success_num));
                        string log_file_name = to_string(log_index) + ".log";
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
                            if (response)
                            {
                                response->status_code = StatusOperationFailed;
                                response->status_msg = msg;
                            }
                            if (!callback.empty())
                            {
                                string res;
                                resp_data.toJsonString(json_str);
                                HttpUtil::Post(callback, json_str, res);
                            }
                        }
                    }
                    else
                    {
                        SLOG_DEBUG("No data needs to be synchronized, update log stauts to committed");
                        clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Commit));
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
                response.status_code = StatusOK;
                response.status_msg = "Operation success";
                response.status_msg = "opt_id";
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
            response.status_msg = "Batch insert fail: " + string(e.what());
            response.status_code = StatusOperationFailed;
        }
    }
}