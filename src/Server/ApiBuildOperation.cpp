#include "ApiProvider.h"

namespace server
{
    bool ApiHandler::build_check(shared_ptr<APIUtil>& apiUtil, const MessageBuildRequest& request, MessageBuildResponse& response)
    {
        std::string db_name = request.db_name;
        std::string msg;
        if (!request.db_path.empty() && FileUtil::fileExists(request.db_path) == false)
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
            std::string file_suffix = FileUtil::fileSuffix(db_path);
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
            try
            {
                if (!flag) 
                {
                    result = "build failed.";
                    throw std::runtime_error(result);
                }
                // if zip file then excuse batchInsert
                if (nt_files.size() > 0)
                {
                    current_database = make_shared<Database>(db_name);
                    bool rt  = current_database->load(false);
                    if (!rt)
                    {
                        result = "unable to load database.";
                        throw std::runtime_error(result);
                    }
                    for (std::string rdf_zip : nt_files)
                    {
                        SLOG_DEBUG("batch insert rdf file: " + rdf_zip);
                        current_database->batch_insert(rdf_zip, false, nullptr);
                    }
                    nt_file_num += nt_files.size();
                    if (!current_database->save())
                    {
                        result = "disk or memory is not enough.";
                        throw std::runtime_error(result);
                    }
                    success_num = current_database->getTripleNum();
                    current_database.reset();
                }
            }
            catch(const std::exception& e)
            {
                result = "Import RDF file to database failed:" + string(e.what());
                FileUtil::removePath(_db_path);
                if (!unz_dir_path.empty())
                {
                    FileUtil::removePath(unz_dir_path);
                }
                response.StatusMsg = result;
                response.StatusCode = StatusOperationFailed;
                current_database.reset();
                SLOG_ERROR(result);
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
            size_t parse_error_num = FileUtil::fileLines(error_log);
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
                FileUtil::removePath(unz_dir_path);
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
            std::string username = request.username;
            std::string db_path = request.db_path;
            apiUtil->init_databaseinfo(db_name, username, gutil::TimeUtil::now(NORM_DATETIME_PATTERN), DatabaseStatus::BUILDING);
            std::vector<std::string> nt_files;
            std::string unz_dir_path;
            std::string file_suffix = FileUtil::fileSuffix(db_path);
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
                        FileUtil::removePath(_db_path);
                        if (!unz_dir_path.empty())
                        {
                            FileUtil::removePath(unz_dir_path);
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
                FileUtil::removePath(_db_path);
                if (!unz_dir_path.empty())
                {
                    FileUtil::removePath(unz_dir_path);
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
            size_t parse_error_num = FileUtil::fileLines(error_log);
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
                FileUtil::removePath(unz_dir_path);
            }

            // Util::add_backuplog(db_name);
            response.StatusCode = StatusOK;
            response.StatusMsg = result;
            response.failed_num = parse_error_num;
            response.successNum = success_num;

            // build follower database
            ClusterOperation cluster_operation = ClusterOperation_Build;
            ClusterUpdateType cluster_update_type = ClusterUpdateType::ClusterUpdateType_Build;
            uint64 log_index = gutil::IdUtil::nextUID();
            clusterManagerPtr->addLog(db_name, log_index, cluster_operation, cluster_update_type);
            clusterManagerPtr->addCommitLog(db_name, log_index, cluster_update_type, "");
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Build fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }
}