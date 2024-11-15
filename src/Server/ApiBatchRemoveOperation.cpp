#include "ApiProvider.h"

namespace server
{
    bool ApiHandler::batch_remove_check(shared_ptr<APIUtil>& apiUtil, const MessageBatchRemoveRequest& request, MessageBatchRemoveResponse& response)
    {
        std::string msg;
        if (apiUtil->check_param_value("db_name", request.db_name, msg) == false)
        {
            response.StatusMsg = msg;
            response.StatusCode = StatusParamIsIllegal;
            return false;
        }
        if (apiUtil->check_param_value("file", request.file, msg) == false)
        {
            response.StatusMsg = msg;
            response.StatusCode = StatusParamIsIllegal;
            return false;
        }
        if (Util::file_exist(request.file) == false)
        {
            response.StatusMsg = "The data file is not exist";
            response.StatusCode = StatusParamIsIllegal;
            return false;
        }
        if (apiUtil->check_db_built(request.db_name) == false)
        {
            response.StatusMsg = "Database not built yet.";
            response.StatusCode = StatusOperationConditionsAreNotSatisfied;
            return false;
        }
        if (apiUtil->check_db_loaded(request.db_name) == false)
        {
            response.StatusMsg = "Database not load yet.";
            response.StatusCode = StatusOperationConditionsAreNotSatisfied;
            return false;
        }
        return true;
    }

    void ApiHandler::batch_remove(shared_ptr<APIUtil>& apiUtil, const MessageBatchRemoveRequest& request, MessageBatchRemoveResponse& response)
    {
        try
        {
            if (!batch_remove_check(apiUtil, request, response))
                return;
            
            std::string file = request.file;
            std::vector<std::string> nt_files;
            std::string unz_dir_path;
            std::string file_suffix = Util::fileSuffix(file);
            bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
            if (is_zip)
            {
                if (!uncompress_zip(apiUtil, file, nt_files, unz_dir_path, response))
                    return;
            }
            else
            {
                nt_files.push_back(file);
            }
            std::string db_name = request.db_name;
            std::string remote_ip = request.remote_ip;
            shared_ptr<DatabaseInfo> db_info;
            apiUtil->get_databaseinfo(db_name, db_info);
            if (!apiUtil->trywrlock_databaseinfo(db_info, 300))
            {
                response.StatusMsg = "Unable to batch remove due to loss of lock.";
                response.StatusCode = StatusLossOfLock;
                return;
            }

            unsigned success_num = 0;
            unsigned total_num = 0;
            size_t parse_error_num = 0;
            string error_log = GlobalTypedef::db_path(db_name) + "/parse_error.log";
            total_num = Util::count_lines(error_log);
            for (std::string rdf_file : nt_files)
            {
                SLOG_DEBUG("begin remove data from " + rdf_file);
                success_num += db_info->getDatabase()->batch_remove(rdf_file, false, nullptr);
            }
            // exclude Info line
            parse_error_num = Util::count_lines(error_log) - total_num - nt_files.size();
            // save data and unlock
            db_info->getDatabase()->save();
            apiUtil->unlock_databaseinfo(db_info);

            string msg = "Batch remove data successfully.";
            response.StatusCode = StatusOK;
            response.StatusMsg = msg;
            response.successNum = success_num;
            response.failedNum = parse_error_num;
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Batch remove fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }

    void ApiHandler::batch_remove_cluster(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBatchRemoveRequest& request, MessageBatchRemoveResponse& response)
    {
        try
        {
            if (!batch_remove_check(apiUtil, request, response))
                return;
            
            std::string cluster_db_path;
            std::string logpath;
            uint64 log_index;
            std::string db_name = request.db_name;
            string msg;

            // send [prepare] heartbeat and wait response
            ClusterUpdateType cluster_update_type = ClusterUpdateType::ClusterUpdateType_Delete;
            log_index = gutil::IdUtil::nextUID();
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
            shared_ptr<ofstream> clusterlog = make_shared<ofstream>();
            clusterlog->open(logpath.c_str());

            std::string file = request.file;
            std::vector<std::string> nt_files;
            std::string unz_dir_path;
            std::string file_suffix = Util::fileSuffix(file);
            bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
            if (is_zip)
            {
                if (!uncompress_zip(apiUtil, file, nt_files, unz_dir_path, response))
                    return;
            }
            else
            {
                nt_files.push_back(file);
            }
            std::string remote_ip = request.remote_ip;
            shared_ptr<DatabaseInfo> db_info;
            apiUtil->get_databaseinfo(db_name, db_info);
            if (!apiUtil->trywrlock_databaseinfo(db_info, 300))
            {
                response.StatusMsg = "Unable to batch remove due to loss of lock.";
                response.StatusCode = StatusLossOfLock;
                return;
            }

            unsigned success_num = 0;
            unsigned total_num = 0;
            size_t parse_error_num = 0;
            string error_log = GlobalTypedef::db_path(db_name) + "/parse_error.log";
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
            clusterlog->close();
            msg = "Batch remove data successfully.";

            MessageBatchRemoveResponse resp_data;
            resp_data.StatusCode = StatusOK;
            resp_data.StatusMsg = msg;
            resp_data.successNum = success_num;
            resp_data.failedNum = parse_error_num;
            resp_data.opt_id = response.opt_id;
            std::string json_str;
            resp_data.toJsonString(json_str);
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
                    response = resp_data;
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
                    response = resp_data;
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
                response = resp_data;
            }
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Batch remove fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }
}