#include "ApiProvider.h"

namespace gs
{
    bool ApiHandler::batch_remove_check(shared_ptr<APIUtil>& apiUtil, const MessageBatchRemoveRequest& request, MessageBatchRemoveResponse& response, std::vector<std::string>& file_paths, std::vector<std::string>& temp_paths)
    {
        std::string msg;
        if (apiUtil->check_param_value("db_name", request.db_name, msg) == false)
        {
            response.StatusMsg = msg;
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
        string file = request.file;
        if (apiUtil->check_param_value("file", file, msg) == false)
        {
            response.StatusMsg = msg;
            response.StatusCode = StatusParamIsIllegal;
            return false;
        }
        else
        {
            std::map<std::string, unsigned long long> uncompress_files;
            std::string local_path = GlobalTypedef::upload_path();
            // download file from remote
            if (request.remote) 
            {
                CURLcode rt = HttpUtil::DownloadFile(file, local_path);
                if (rt != CURLcode::CURLE_OK || FileUtil::pathExists(local_path) == false) {
                    response.StatusMsg = "Download file '" + file + "' failed.";
                    response.StatusCode = StatusOperationFailed;
                    return false;
                }
                temp_paths.push_back(local_path);
            }
            else
            {
                if (FileUtil::pathExists(file) == false)
                {
                    response.StatusMsg = "The file '" + file + "' is not exist.";
                    response.StatusCode = StatusOperationFailed;
                    return false;
                }
                local_path = file;
            }
            std::string file_name = FileUtil::fileName(local_path);
            std::string file_suffix = FileUtil::fileSuffix(local_path);
            bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
            // check file suffix
            if (!apiUtil->check_upload_allow_extensions(file_suffix) &&  !is_zip)
            {
                response.StatusMsg = "The file suffix '" + file_suffix + "' is not allowed.";
                response.StatusCode = StatusParamIsIllegal;
                if (request.remote)
                    FileUtil::removeFile(local_path);
                return false;
            }
            if (is_zip)
            {
                // uncompress zip
                size_t pos = file_name.size() - file_suffix.size() - 1;
                std::string uncompress_path = GlobalTypedef::upload_path() + file_name.substr(0, pos) + "_" + gutil::TimeUtil::now();
                temp_paths.push_back(uncompress_path);
                bool unzip = uncompress_zip(apiUtil, local_path, uncompress_files, uncompress_path, response);
                if (!unzip)
                {
                    return false;
                }
            }
            else
            {
                // add file to file_paths
                uncompress_files[local_path] = 0ull;
            }
            for (auto& item: uncompress_files)
            {
                file_paths.push_back(item.first);
            }
        }
        return true;
    }

    void ApiHandler::batch_remove(shared_ptr<APIUtil>& apiUtil, const MessageBatchRemoveRequest& request, MessageBatchRemoveResponse& response)
    {
        shared_ptr<DatabaseInfo> db_info = nullptr;
        std::vector<std::string> file_paths; // local file path
        std::vector<std::string> temp_paths; // download files or uncompress files
        try
        {
            if (!batch_remove_check(apiUtil, request, response, file_paths, temp_paths))
                return;
            std::string db_name = request.db_name;
            std::string remote_ip = request.remote_ip;
            apiUtil->get_databaseinfo(db_name, db_info);
            StatusCode statusCode;
            std::string statusMsg;
            if (!apiUtil->validate_databaseinfo(db_info, statusCode, statusMsg, true, true, true, 300))
            {
                response.StatusCode = statusCode;
                response.StatusMsg = statusMsg;
                return;
            }
            unsigned success_num = 0;
            unsigned total_num = 0;
            size_t parse_error_num = 0;
            string error_log = GlobalTypedef::db_path(db_name) + "/parse_error.log";
            total_num = FileUtil::fileLines(error_log);
            for (std::string rdf_file : file_paths)
            {
                SLOG_DEBUG("begin remove data from " + rdf_file);
                success_num += db_info->getDatabase()->batch_remove(rdf_file, false, nullptr);
            }
            // exclude Info line
            parse_error_num = FileUtil::fileLines(error_log) - total_num - file_paths.size();
            // save data and unlock
            if (Util::getConfigureValue("check_point") == "on")
            {
                if (!db_info->getDatabase()->save())
                {
                    statusMsg = "disk or memory not enough";
                    throw std::runtime_error(statusMsg);
                }
            }
            apiUtil->unlock_databaseinfo(db_info);

            statusMsg = "Batch remove data successfully.";
            response.StatusCode = StatusOK;
            response.StatusMsg = statusMsg;
            response.successNum = success_num;
            response.failedNum = parse_error_num;
        }
        catch (const std::exception &e)
        {
            apiUtil->unlock_databaseinfo(db_info);
            // remove temp files
            remove_temp_files(temp_paths);
            response.StatusMsg = "Batch remove fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }

    void ApiHandler::batch_remove_cluster(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBatchRemoveRequest& request, MessageBatchRemoveResponse& response)
    {
        shared_ptr<DatabaseInfo> db_info = nullptr;
        std::vector<std::string> file_paths; // local file path
        std::vector<std::string> temp_paths; // download files or uncompress files
        try
        {
            if (!batch_remove_check(apiUtil, request, response, file_paths, temp_paths))
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
                response.StatusCode = StatusOperationFailed;
                response.StatusMsg = "Less than half of the cluster nodes are confirmed.";
                throw new runtime_error(response.StatusMsg);
            }
            cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
            logpath = cluster_db_path + to_string(log_index) + ".log";
            shared_ptr<ofstream> clusterlog = make_shared<ofstream>();
            clusterlog->open(logpath.c_str());

            apiUtil->get_databaseinfo(db_name, db_info);
            StatusCode statusCode;
            if (!apiUtil->validate_databaseinfo(db_info, statusCode, msg, true, true, true, 180))
            {
                response.StatusMsg = msg;
                response.StatusCode = statusCode;
                return;
            }
            unsigned success_num = 0;
            unsigned total_num = 0;
            size_t parse_error_num = 0;
            string error_log = GlobalTypedef::db_path(db_name) + "/parse_error.log";
            total_num = FileUtil::fileLines(error_log);
            for (std::string rdf_file : file_paths)
            {
                SLOG_DEBUG("begin remove data from " + rdf_file);
                success_num += db_info->getDatabase()->batch_remove(rdf_file, false, nullptr, clusterlog);
            }
            // exclude Info line
            parse_error_num = FileUtil::fileLines(error_log) - total_num - file_paths.size();
            // save data and unlock
            if (Util::getConfigureValue("check_point") == "on")
            {
                if (!db_info->getDatabase()->save())
                {
                    msg = "disk or memory not enough";
                    throw std::runtime_error(msg);
                }
            }
            apiUtil->unlock_databaseinfo(db_info);
            db_info.reset();
            // close cluster log
            clusterlog->close();

            MessageBatchRemoveResponse resp_data;
            resp_data.StatusCode = StatusOK;
            resp_data.StatusMsg = "Batch remove data successfully.";
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
                bool append_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Append, ClusterUpdateType_Delete, log_file_name), true);
                if (append_result)
                {
                    SLOG_DEBUG("response result:\n" << json_str);
                    clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Commit));
                    response = resp_data;
                }
                else
                {
                    // restore data
                    // try get wrlock timeout 180 senconds
                    if (apiUtil->validate_databaseinfo(db_info, statusCode, msg, true, true, true, 180))
                    {
                        uint64_t num = 0;
                        for (std::string rdf_file : file_paths)
                        {
                            num += db_info->getDatabase()->batch_insert(rdf_file);
                        }
                        SLOG_INFO("restore " + db_name + " data: batch_insert num " << num);
                        apiUtil->unlock_databaseinfo(db_info);
                        db_info.reset();
                    }
                    else
                    {
                        SLOG_ERROR("restore failed: " + msg + ", log[" + log_file_name + "], operation[2]");
                    }
                    clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Cancel));
                    resp_data.StatusMsg = "Less than half of the cluster nodes reply.";
                    resp_data.StatusCode = StatusOperationFailed;
                    response = resp_data;
                    SLOG_ERROR(response.StatusMsg);
                }
            }
            else
            {
                SLOG_DEBUG("No data needs to be synchronized, update log stauts to failed");
                clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
                FileUtil::removePath(clusterManagerPtr->getDbDirPath(db_name)+log_file_name);
                response = resp_data;
            }
            // remove temp files
            remove_temp_files(temp_paths);
        }
        catch (const std::exception &e)
        {
            apiUtil->unlock_databaseinfo(db_info);
            // remove temp files
            remove_temp_files(temp_paths);
            response.StatusMsg = "Batch remove fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }
}