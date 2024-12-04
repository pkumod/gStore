#include "ApiProvider.h"

namespace server
{
    bool ApiHandler::build_check(shared_ptr<APIUtil>& apiUtil, const MessageBuildRequest& request, MessageBuildResponse& response)
    {
        std::string db_name = request.db_name;
        std::string msg;
        if (!request.db_path.empty() && Util::file_exist(request.db_path) == false)
        {
            response.StatusMsg = "RDF file not exist.";
            response.StatusCode = StatusParamIsIllegal;
            return false;
        }
        if (apiUtil->check_param_value("db_name", db_name, msg) == false)
        {
            response.StatusMsg = msg;
            response.StatusCode = StatusParamIsIllegal;
            return false;
        }
        //check the db_name is system
        if (db_name == GlobalTypedef::system_db)
        {
            response.StatusMsg = "The database name can not be system.";
            response.StatusCode = StatusParamIsIllegal;
            return false;
        }
        // check if database named [db_name] is already built
        if (apiUtil->check_db_built(db_name))
        {
            response.StatusMsg = "database already built.";
            response.StatusCode = StatusOperationConditionsAreNotSatisfied;
            return false;
        }
        // check databse number
        if (apiUtil->check_db_count() == false)
        {
            response.StatusMsg = "The total number of databases more than max_databse_num.";
            response.StatusCode = StatusOperationConditionsAreNotSatisfied;
            return false;
        }
        return true;
    }

    void ApiHandler::build(shared_ptr<APIUtil>& apiUtil, const MessageBuildRequest& request, MessageBuildResponse& response)
    {
        try
        {
            if (!build_check(apiUtil, request, response))
                return;
            
            std::string db_name = request.db_name;
            std::string username = request.username;
            std::string db_path = request.db_path;
            apiUtil->init_databaseinfo(db_name, username, gutil::TimeUtil::now(NORM_DATETIME_PATTERN), DatabaseStatus::BUILDING);
            std::vector<std::string> nt_files;
            std::string unz_dir_path;
            std::string file_suffix = Util::fileSuffix(db_path);
            bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
            if (is_zip)
            {
                if (!uncompress_zip(apiUtil, db_path, nt_files, unz_dir_path, response, db_path))
                {
                    apiUtil->erase_databaseinfo(db_name);
                    return;
                }
            }

            string _db_path = GlobalTypedef::db_path(db_name);
            string database = db_name;
            SLOG_DEBUG("Import dataset to build database...");
            SLOG_DEBUG("db_name: " + database + "\tRDF_data: " + db_path);
            string result;
            shared_ptr<Database> current_database = make_shared<Database>(database);
            shared_ptr<DatabaseInfo> current_db_info;
            apiUtil->get_databaseinfo(db_name, current_db_info);
            current_db_info->setDatabase(current_database);
            // build empty database
            bool flag = true;
            int nt_file_num = 0;
            if (!db_path.empty())
            {
                flag = current_database->build(db_path);
                nt_file_num = 1;
            }
            else
                flag = current_database->BuildEmptyDB();
            int success_num = current_database->getTripleNum();
            current_db_info->setDatabase(nullptr);
            current_database.reset();
            if (flag)
            {
                // if zip file then excuse batchInsert
                if (nt_files.size() > 0)
                {
                    current_database = make_shared<Database>(db_name);
                    bool rt  = current_database->load(false);
                    if (!rt)
                    {
                        result = "Import RDF file to database failed: load error.";
                        Util::remove_path(_db_path);
                        if (!unz_dir_path.empty())
                        {
                            Util::remove_path(unz_dir_path);
                        }
                        response.StatusMsg = result;
                        response.StatusCode = StatusOperationFailed;
                        current_database.reset();
                        return;
                    }
                    for (std::string rdf_zip : nt_files)
                    {
                        current_database->batch_insert(rdf_zip, false, nullptr);
                    }
                    nt_file_num += nt_files.size();
                    if (!current_database->save())
					{
                        response.Error(StatusOperationFailed, "disk or memory is not enough");
                        return;
                    }
                    success_num = current_database->getTripleNum();
                    current_database.reset();
                }
            }
            else
            {
                result = "Import RDF file to database failed.";
                Util::remove_path(_db_path);
                if (!unz_dir_path.empty())
                {
                    Util::remove_path(unz_dir_path);
                }
                response.StatusMsg = result;
                response.StatusCode = StatusOperationFailed;
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
            response.StatusCode = StatusOK;
            response.StatusMsg = result;
            response.failed_num = parse_error_num;
            response.successNum = success_num;
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Build fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }

    void ApiHandler::build_cluster(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBuildRequest& request, MessageBuildResponse& response)
    {
        try
        {
            if (!build_check(apiUtil, request, response))
                return;
            
            std::string db_name = request.db_name;
            std::string cluster_db_path;
            std::string logpath;
            uint64 log_index;
            ClusterUpdateType cluster_update_type = ClusterUpdateType::ClusterUpdateType_Build;
            log_index = gutil::IdUtil::nextUID();
            clusterManagerPtr->addLog(db_name, log_index, ClusterOperation_Prepare, cluster_update_type);
            bool prepare_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Prepare), true);
            if (!prepare_result)
            {
                clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
                response.StatusMsg =  "Less than half of the cluster nodes are confirmed.";
                response.StatusCode = StatusOperationFailed;
                SLOG_ERROR(response.StatusMsg);
                return;
            }
            cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
            logpath = cluster_db_path + to_string(log_index) + ".log";
            shared_ptr<ofstream> clusterlog = make_shared<ofstream>();
            clusterlog->open(logpath.c_str());

            std::string username = request.username;
            std::string db_path = request.db_path;
            apiUtil->init_databaseinfo(db_name, username, gutil::TimeUtil::now(NORM_DATETIME_PATTERN), DatabaseStatus::BUILDING);
            std::vector<std::string> nt_files;
            std::string unz_dir_path;
            std::string file_suffix = Util::fileSuffix(db_path);
            bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
            if (is_zip)
            {
                if (!uncompress_zip(apiUtil, db_path, nt_files, unz_dir_path, response, db_path))
                {
                    apiUtil->erase_databaseinfo(db_name);
                    return;
                }
            }

            string _db_path = GlobalTypedef::db_path(db_name);
            string database = db_name;
            SLOG_DEBUG("Import dataset to build database...");
            SLOG_DEBUG("db_name: " + database + "\tRDF_data: " + db_path);
            string result;
            shared_ptr<Database> current_database = make_shared<Database>(database);
            shared_ptr<DatabaseInfo> current_db_info;
            apiUtil->get_databaseinfo(db_name, current_db_info);
            current_db_info->setDatabase(current_database);
            // build empty database
            bool flag = true;
            int nt_file_num = 0;
            if (!db_path.empty())
            {
                flag = current_database->build(db_path, clusterlog);
                nt_file_num = 1;
            }
            else
                flag = current_database->BuildEmptyDB();
            int success_num = current_database->getTripleNum();
            current_db_info->setDatabase(nullptr);
            current_database.reset();
            if (flag)
            {
                // if zip file then excuse batchInsert
                if (nt_files.size() > 0)
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
                        response.StatusMsg = result;
                        response.StatusCode = StatusOperationFailed;
                        current_database.reset();
                        return;
                    }
                    for (std::string rdf_zip : nt_files)
                    {
                        current_database->batch_insert(rdf_zip, false, nullptr, clusterlog);
                    }
                    nt_file_num += nt_files.size();
                    current_database->save();
                    if (!current_database->save())
					{
                        response.Error(StatusOperationFailed, "disk or memory is not enough");
                        return;
                    }
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
                response.StatusMsg = result;
                response.StatusCode = StatusOperationFailed;
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
            response.StatusCode = StatusOK;
            response.StatusMsg = result;
            response.failed_num = parse_error_num;
            response.successNum = success_num;
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
                    // follower recover by heartbeat compare
                    SLOG_DEBUG("build db follower recover by heartbeat compare:" << db_name);
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
            }
            // cluster sync task end
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Build fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }
}