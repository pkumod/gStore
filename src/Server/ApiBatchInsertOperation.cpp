#include "ApiProvider.h"

namespace server
{
    bool ApiHandler::batch_insert_check(shared_ptr<APIUtil>& apiUtil, const MessageBatchInsertRequest& request, MessageBatchInsertResponse& response, bool& is_file)
    {
        std::string db_name = request.db_name;
        std::string msg;
        if (apiUtil->check_param_value("db_name", db_name, msg) == false)
        {
            response.StatusCode = StatusParamIsIllegal;
            response.StatusMsg = msg;
            return false;
        }
        std::string file = request.file;
        std::string dir = request.dir;
        if (apiUtil->check_param_value("file", file, msg) == false)
        {
            is_file = false;
            if (apiUtil->check_param_value("dir", dir, msg) == false)
            {
                response.StatusMsg = "file and dir cannot be empty at the same time!";
                response.StatusCode = StatusParamIsIllegal;
                return false;
            }
        }
        if (is_file && Util::file_exist(file) == false)
        {
            response.StatusMsg = "The data file is not exist";
            response.StatusCode = StatusParamIsIllegal;
            return false;
        }
        if (!is_file && Util::file_exist(dir) == false)
        {
            response.StatusMsg = "The data directory is not exist";
            response.StatusCode = StatusParamIsIllegal;
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
            msg = "Database not load yet.";
            response.StatusCode = StatusOperationConditionsAreNotSatisfied;
            return false;
        }
        return true;
    }

    void ApiHandler::batch_insert(shared_ptr<APIUtil>& apiUtil, const MessageBatchInsertRequest& request, MessageBatchInsertResponse& response)
    { 
        try
        {
            bool is_file = true;
            if (!batch_insert_check(apiUtil, request, response, is_file))
                return;

            std::string file = request.file;
            std::vector<std::string> nt_files;
            std::string unz_dir_path;
            if (is_file)
            {
                std::string file_suffix = Util::fileSuffix(file);
                bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
                if (is_zip)
                {
                    if (!uncompress_zip(apiUtil, file, nt_files, unz_dir_path, response))
                        return;
                }
                else
                    nt_files.push_back(file);
            }
            else
            {
                // is dirctory
                std::string dir = request.dir;
                gutil::StringUtil::append(dir, '/');
                Util::dir_files(dir, "", nt_files);
            }
            std::string db_name = request.db_name;
            shared_ptr<DatabaseInfo> db_info;
            apiUtil->get_databaseinfo(db_name, db_info);

            if (!apiUtil->trywrlock_databaseinfo(db_info, 300))
            {
                response.StatusCode = StatusLossOfLock;
                response.StatusMsg = "Unable to batch insert due to loss of lock.";
                return;
            }
            unsigned success_num = 0;
            unsigned total_num = 0;
            unsigned parse_error_num = 0;
            string error_log = GlobalTypedef::db_path(db_info->getName()) + "/parse_error.log";
            total_num = Util::count_lines(error_log);
            for (std::string rdf_file : nt_files)
            {
                SLOG_DEBUG("begin insert data from " + rdf_file);
                success_num += db_info->getDatabase()->batch_insert(rdf_file, false, nullptr);
            }
            // exclude Info line
            parse_error_num = Util::count_lines(error_log) - total_num - nt_files.size();
            // save data and unlock
            db_info->getDatabase()->save();
            apiUtil->unlock_databaseinfo(db_info);

            response.StatusCode = StatusOK;
            response.StatusMsg = "Batch insert data successfully.";
            response.successNum = success_num;
            response.failedNum = parse_error_num;
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Batch insert fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }

    void ApiHandler::batch_insert_cluster(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBatchInsertRequest& request, MessageBatchInsertResponse& response)
    {
        try
        {
            bool is_file = true;
            if (!batch_insert_check(apiUtil, request, response, is_file))
                return;

            shared_ptr<ofstream> clusterlog = nullptr;
            std::string cluster_db_path;
            std::string logpath;
            uint64 log_index;
            std::string db_name = request.db_name;

            // send [prepare] heartbeat and wait response
            ClusterUpdateType cluster_update_type = ClusterUpdateType::ClusterUpdateType_Insert;
            log_index = gutil::IdUtil::nextUID();
            clusterManagerPtr->addLog(db_name, log_index, ClusterOperation_Prepare, cluster_update_type);
            bool prepare_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Prepare), true);
            if (!prepare_result)
            {
                clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
                response.StatusMsg = "Less than half of the cluster nodes are confirmed.";
                response.StatusCode = StatusOperationFailed;
                SLOG_ERROR(response.StatusMsg);
                return;
            }
            cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
            logpath = cluster_db_path + to_string(log_index) + ".log";
            clusterlog = make_shared<ofstream>();
            clusterlog->open(logpath.c_str());

            std::string file = request.file;
            std::vector<std::string> nt_files;
            std::string unz_dir_path;
            if (is_file)
            {
                std::string file_suffix = Util::fileSuffix(file);
                bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
                if (is_zip)
                {
                    if (!uncompress_zip(apiUtil, file, nt_files, unz_dir_path, response))
                        return;
                }
                else
                    nt_files.push_back(file);
            }
            else
            {
                // is dirctory
                std::string dir = request.dir;
                gutil::StringUtil::append(dir, '/');
                Util::dir_files(dir, "", nt_files);
            }

            shared_ptr<DatabaseInfo> db_info;
            apiUtil->get_databaseinfo(db_name, db_info);
            if (!apiUtil->trywrlock_databaseinfo(db_info, 300))
            {
                response.StatusCode = StatusLossOfLock;
                response.StatusMsg = "Unable to batch insert due to loss of lock.";
                return;
            }
            unsigned success_num = 0;
            unsigned total_num = 0;
            unsigned parse_error_num = 0;
            string error_log = GlobalTypedef::db_path(db_info->getName()) + "/parse_error.log";
            total_num = Util::count_lines(error_log);
            for (std::string rdf_file : nt_files)
            {
                SLOG_DEBUG("begin insert data from " + rdf_file);
                success_num += db_info->getDatabase()->batch_insert(rdf_file, false, nullptr);
            }
            // exclude Info line
            parse_error_num = Util::count_lines(error_log) - total_num - nt_files.size();
            // save data and unlock
            db_info->getDatabase()->save();
            apiUtil->unlock_databaseinfo(db_info);
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
                string tmp_dir_path = unz_dir_path;
                bool append_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Append, ClusterUpdateType_Insert, log_file_name), true);
                if (append_result)
                {
                    SLOG_DEBUG("response result:\n" << json_str);
                    clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Commit));
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
                Util::remove_path(clusterManagerPtr->getDbDirPath(db_name)+log_file_name);
                // remove unzip files
                if (!unz_dir_path.empty())
                {
                    Util::remove_path(unz_dir_path);
                }
            }
            // cluster sync task end
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Batch insert fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }
}