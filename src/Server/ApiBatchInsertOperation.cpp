#include "ApiProvider.h"

namespace server
{
    bool ApiHandler::batch_insert_check(shared_ptr<APIUtil>& apiUtil, const MessageBatchInsertRequest& request, MessageBatchInsertResponse& response, std::vector<std::string>& file_paths, std::vector<std::string>& temp_paths)
    {
        std::string db_name = request.db_name;
        std::string msg;
        if (apiUtil->check_param_value("db_name", db_name, msg) == false)
        {
            response.StatusCode = StatusParamIsIllegal;
            response.StatusMsg = msg;
            return false;
        }
        if (apiUtil->check_db_built(db_name) == false)
        {
            response.StatusMsg = "Database not built yet.";
            response.StatusCode = StatusOperationConditionsAreNotSatisfied;
            return false;
        }
        if (apiUtil->check_db_loaded(db_name) == false)
        {
            response.StatusMsg = "Database not load yet.";
            response.StatusCode = StatusOperationConditionsAreNotSatisfied;
            return false;
        }
        std::vector<std::string> files = request.file;
        if (files.empty())
        {
            response.StatusMsg = "the value of file can not be empty!";
            response.StatusCode = StatusParamIsIllegal;
            return false;
        } 
        else
        {
            bool db_paths_rt = true;
            std::map<std::string, unsigned long long> uncompress_files;
            for (auto& file : files) {
                std::string local_path = GlobalTypedef::upload_path();
                // download file from remote
                if (request.remote) 
                {
                    CURLcode rt = HttpUtil::DownloadFile(file, local_path);
                    if (rt != CURLcode::CURLE_OK || FileUtil::pathExists(local_path) == false) {
                        response.StatusMsg = "Download file '" + file + "' failed.";
                        response.StatusCode = StatusOperationFailed;
                        db_paths_rt = false;
                        break;
                    }
                    temp_paths.push_back(local_path);
                }
                else
                {
                    if (FileUtil::pathExists(file) == false)
                    {
                        response.StatusMsg = "The file '" + file + "' is not exist.";
                        response.StatusCode = StatusOperationFailed;
                        db_paths_rt = false;
                        break;
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
                    db_paths_rt = false;
                    break;
                }
                if (is_zip)
                {
                    // uncompress zip
                    size_t pos = file_name.size() - file_suffix.size() - 1;
                    std::string uncompress_path = GlobalTypedef::upload_path() + file_name.substr(0, pos) + "_" + gs::TimeUtil::now();
                    temp_paths.push_back(uncompress_path);
                    bool unzip = uncompress_zip(apiUtil, local_path, uncompress_files, uncompress_path, response);
                    if (!unzip)
                    {
                        db_paths_rt = false;
                        break;
                    }
                }
                else
                {
                    // add file to file_paths
                    uncompress_files[local_path] = 0ull;
                }
            }
            if (!db_paths_rt)
            {
                // delete temp files: uncompress_path and download_path
                for (auto& path : temp_paths)
                {
                    FileUtil::removePath(path);
                }
                return false;
            }
            for (auto& item: uncompress_files)
            {
                file_paths.push_back(item.first);
            }
        }
        return true;
    }

    void ApiHandler::batch_insert(shared_ptr<APIUtil>& apiUtil, const MessageBatchInsertRequest& request, MessageBatchInsertResponse& response)
    { 
        shared_ptr<DatabaseInfo> db_info = nullptr;
        int64_t t = gs::TimeUtil::timestamp();
        std::vector<std::string> file_paths; // local file path
        std::vector<std::string> temp_paths; // download files or uncompress files
        try
        {
            if (!batch_insert_check(apiUtil, request, response, file_paths, temp_paths))
                return;
            StatusCode statusCode;
            std::string statusMsg;
            if (!apiUtil->validate_databaseinfo(request.db_name, db_info, statusCode, statusMsg, true, DatabaseLock::W, 180))
            {
                response.StatusCode = statusCode;
                response.StatusMsg = statusMsg;
                return;
            }
            unsigned success_num = 0;
            unsigned total_num = 0;
            unsigned parse_error_num = 0;
            string error_log = db_info->getPath() + "/parse_error.log";
            total_num = FileUtil::fileLines(error_log);
            db_info->setStatus(DatabaseStatus::INSERTING);
            shared_ptr<Database> db_ptr = db_info->getDatabase();
            for (std::string rdf_file : file_paths)
            {
                SLOG_DEBUG("begin insert data from " + rdf_file);
                success_num += db_ptr->batch_insert(rdf_file, false, nullptr);
            }
            if (success_num > 0)
            {
                SLOG_DEBUG("update schema: " << db_ptr->getSchemaFlag());
                db_ptr->updateSchema();
            }
            // exclude Info line
            parse_error_num = FileUtil::fileLines(error_log) - total_num - file_paths.size();
            // save data and unlock
            int64_t t1 = gs::TimeUtil::timestamp();
            if (Util::getConfigureValue("check_point") == "on")
            {
                if (!db_ptr->save())
                {
                    statusMsg = "disk or memory is not enough";
                    throw new std::runtime_error(statusMsg);
                }
                int64_t t2 = gs::TimeUtil::timestamp();
                SLOG_DEBUG("auto checkpoint used: " << t2 - t1);
            }
            db_info->setStatus(DatabaseStatus::LOADED);
            apiUtil->unlock_databaseinfo(db_info);
            db_ptr.reset();
            db_info.reset();
            // remove temp files
            remove_temp_files(temp_paths);
            response.StatusCode = StatusOK;
            response.StatusMsg = "Batch insert data successfully.";
            response.successNum = success_num;
            response.failedNum = parse_error_num;
        }
        catch (const std::exception &e)
        {
            if (db_info)
                db_info->setStatus(DatabaseStatus::LOADED);
            apiUtil->unlock_databaseinfo(db_info);
            // remove temp files
            remove_temp_files(temp_paths);
            response.StatusCode = StatusOperationFailed;
            response.StatusMsg = string(e.what());
            SLOG_ERROR("Batch insert fail: " << response.StatusMsg << "(code " << response.StatusCode << ")");
        }
    }

    void ApiHandler::batch_insert_cluster(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBatchInsertRequest& request, MessageBatchInsertResponse& response)
    {
        shared_ptr<DatabaseInfo> db_info = nullptr;
        std::vector<std::string> file_paths; // local file path
        std::vector<std::string> temp_paths; // download files or uncompress files
        try
        {
            if (!batch_insert_check(apiUtil, request, response, file_paths, temp_paths))
                return;

            shared_ptr<ofstream> clusterlog = nullptr;
            std::string cluster_db_path;
            std::string logpath;
            uint64 log_index;
            std::string db_name = request.db_name;

            // send [prepare] heartbeat and wait response
            ClusterUpdateType cluster_update_type = ClusterUpdateType::ClusterUpdateType_Insert;
            log_index = gs::IdUtil::nextUID();
            clusterManagerPtr->addLog(db_name, log_index, ClusterOperation_Prepare, cluster_update_type);
            bool prepare_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Prepare), true);
            if (!prepare_result)
            {
                clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
                response.StatusCode = StatusOperationFailed;
                response.StatusMsg = "Less than half of the cluster nodes are confirmed.";
                return;
            }
            cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
            logpath = cluster_db_path + to_string(log_index) + ".log";
            clusterlog = make_shared<ofstream>();
            clusterlog->open(logpath.c_str());

            StatusCode statusCode;
            std::string statusMsg;
            if (!apiUtil->validate_databaseinfo(db_name, db_info, statusCode, statusMsg, true, DatabaseLock::W, 180))
            {
                response.StatusCode = statusCode;
                response.StatusMsg = statusMsg;
                return;
            }
            unsigned success_num = 0;
            unsigned total_num = 0;
            unsigned parse_error_num = 0;
            db_info->setStatus(DatabaseStatus::INSERTING);
            shared_ptr<Database> db_ptr = db_info->getDatabase();
            string error_log = db_info->getPath() + "/parse_error.log";
            total_num = FileUtil::fileLines(error_log);
            for (std::string rdf_file : file_paths)
            {
                SLOG_DEBUG("begin insert data from " + rdf_file);
                success_num += db_ptr->batch_insert(rdf_file, false, nullptr, clusterlog);
            }
            if (success_num > 0)
            {
                SLOG_DEBUG("update schema: " << db_ptr->getSchemaFlag());
                db_ptr->updateSchema();
            }
            // exclude Info line
            parse_error_num = FileUtil::fileLines(error_log) - total_num - file_paths.size();
            // save data and unlock
            if (Util::getConfigureValue("check_point") == "on")
            {
                if (!db_ptr->save())
                {
                    statusMsg = "disk or memory is not enough";
                    throw new runtime_error(statusMsg);
                }
            }
            db_info->setStatus(DatabaseStatus::LOADED);
            apiUtil->unlock_databaseinfo(db_info);
            db_ptr.reset();
            db_info.reset();
            clusterlog->close();

            // respnse data
            response.StatusCode = StatusOK;
            response.StatusMsg = "Batch insert data successfully.";
            response.successNum = success_num;
            response.failedNum = parse_error_num;
            std::string json_str;
            response.toJsonString(json_str);
            
            // cluster sync task begin
            string log_file_name = to_string(log_index) + ".log";
            if (success_num > 0)
            {
                SLOG_DEBUG("add log appendEntities task, copy num " + to_string(success_num));
                bool append_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Append, ClusterUpdateType_Insert, log_file_name), true);
                if (append_result)
                {
                    SLOG_DEBUG("response result:\n" << json_str);
                    clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Commit));
                }
                else
                {
                    // try get wrlock timeout 180 senconds
                    StatusCode statusCode;
                    std::string statusMsg;
                    if (apiUtil->validate_databaseinfo(db_name, db_info, statusCode, statusMsg, true, DatabaseLock::W, 180))
                    {
                        uint64_t num = 0;
                        db_ptr = db_info->getDatabase();
                        for (std::string rdf_file : file_paths)
                        {
                            num += db_ptr->batch_remove(rdf_file);
                        }
                        SLOG_INFO("restore " + db_name + " data: batch_remove num " << num);
                        apiUtil->unlock_databaseinfo(db_info);
                        db_ptr.reset();
                        db_info.reset();
                    }
                    else
                    {
                        SLOG_ERROR("restore " + db_name + " data failed: unable get wrlock, log[" + log_file_name + "], operation[1]");
                    }
                    clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Cancel));
                    response.StatusMsg = "Less than half of the cluster nodes reply.";
                    response.StatusCode = StatusOperationFailed;
                    SLOG_ERROR(response.StatusMsg);
                }
            }
            else
            {
                SLOG_DEBUG("No data needs to be synchronized, update log stauts to failed");
                clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
                FileUtil::removePath(clusterManagerPtr->getDbDirPath(db_name)+log_file_name);
            }
            // cluster sync task end

            // remove temp files
            remove_temp_files(temp_paths);
        }
        catch (const std::exception &e)
        {
            if (db_info)
                db_info->setStatus(DatabaseStatus::LOADED);
            apiUtil->unlock_databaseinfo(db_info);
            // remove temp files
            remove_temp_files(temp_paths);
            response.StatusMsg = string(e.what());
            response.StatusCode = StatusOperationFailed;
            SLOG_ERROR("Batch insert fail: " << response.StatusMsg);
        }
    }
}