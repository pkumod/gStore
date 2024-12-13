#include "ApiProvider.h"
#include "../Api/WFHttpUtil.h"

namespace server
{
    bool ApiHandler::query_check(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& request, MessageQueryResponse& response)
    {
        int32_t min_memory = Util::getConfigureIntValue("min_memory", 512); // MB
        int32_t memoryLeft = gutil::ResourceUtil::memoryLeft();
        if (memoryLeft < (min_memory >> 10))
        {
            response.StatusMsg = "memory not enough, available:" + std::to_string(memoryLeft) + "MB, need minimum:" + std::to_string(min_memory) + "MB";
            response.StatusCode = StatusOperationFailed;
            return false;
        }

        // check db_name paramter
        std::string msg;
        if (apiUtil->check_param_value("db_name", request.db_name, msg) == false)
        {
            response.StatusMsg = msg;
            response.StatusCode = StatusParamIsIllegal;
            return false;
        }

        // check sparql paramter
        if (apiUtil->check_param_value("sparql", request.sparql, msg) == false)
        {
            response.StatusMsg = msg;
            response.StatusCode = StatusParamIsIllegal;
            return false;
        }

        // check database exist
        if (apiUtil->check_db_built(request.db_name) == false)
        {
            response.StatusMsg = "Database not build yet.";
            response.StatusCode = StatusOperationConditionsAreNotSatisfied;
            return false;
        }

        // check database load status
        if (apiUtil->check_db_loaded(request.db_name) == false)
        {
            response.StatusMsg = "Database not load yet.";
            response.StatusCode = StatusOperationConditionsAreNotSatisfied;
            return false;
        }

        return true;
    }

    void ApiHandler::query_format_response_data(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& request, MessageQueryResponse& response, ResultSet& rs)
    {
        response.fileName = "";
        if (request.format == "json")
        {
            rs.to_JSON(response.query_json);
        }
        else if (request.format == "file")
        {
            
            string file_name = request.db_name + "_" + response.threadId + "_" + gutil::TimeUtil::now() + ".txt";
            string file_path = apiUtil->get_query_result_path() + file_name;
            nlohmann::json json_data;
            rs.to_JSON(json_data);
            ofstream outfile;
            outfile.open(file_path);
            outfile << json_data.dump();
            outfile.close();
            response.fileName = file_name;
        }
        else if (request.format == "n-triple")
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
            response.query_json = json_data;
        }
        else
        {
            response.StatusMsg = "Unknown result format.";
            response.StatusCode = StatusOperationFailed;
        }
    }

    void ApiHandler::query_result_notify(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& request, MessageQueryResponse& response)
    {
        int state = response.StatusCode == StatusOK ? 1:-1;
        std::string strPost;
        response.toJsonString(strPost);
        // save result to file
        string file_name = response.opt_id + ".json";
        string file_path = apiUtil->get_query_result_path() + file_name;
        ofstream outfile;
        outfile.open(file_path);
        outfile << strPost;
        outfile.close();
        // update access log
        apiUtil->update_access_log(response.StatusCode, response.StatusMsg, response.opt_id, state, response.ansNum, 0, file_path);
        // send result to callback
        if (!request.callback.empty()) 
        {
            std::string strResponse;
            HttpUtil::Post(request.callback, strPost, strResponse);
            SLOG_DEBUG("async query callback: " + request.callback + ", response: " + strResponse);
        }
        else
        {
            SLOG_DEBUG("async query no callback");
        }
    }

    void ApiHandler::query(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& request, MessageQueryResponse& response, const query_call& cb, bool format_check)
    {
        try
        {
            if (!query_check(apiUtil, request, response))
                return;
            std::string db_name = request.db_name;
            std::string username = request.username;
            std::string sparql = request.sparql;
            // check db_name paramter
            std::string msg;

            shared_ptr<DatabaseInfo> db_info;
            apiUtil->get_databaseinfo(db_name, db_info);
            // check database read lock
            if (apiUtil->rdlock_databaseinfo(db_info) == false)
            {
                response.StatusMsg = "get current database read lock fail.";
                response.StatusCode = StatusLossOfLock;
                return;
            }
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
                response.StatusMsg = "Unknown query type:" + string(e.what());
                response.StatusCode = StatusOperationFailed;
                return;
            }
            FILE *output = NULL;
            ResultSet rs;
            int ret_val;
            long query_time = gutil::TimeUtil::timestamp();
            std::string query_start_time;
            try
            {
                SLOG_DEBUG("begin query...\n" + sparql);
                rs.setUsername(username);
                query_start_time = gutil::TimeUtil::now(NORM_DATETIME_MS_PATTERN);
                ret_val = db_info->getDatabase()->query(sparql, rs, output, update_flag_bool, false, nullptr, nullptr);
                query_time = gutil::TimeUtil::timestamp() - query_time;
                // unlock rdlock
                apiUtil->unlock_databaseinfo(db_info);
            } catch (const std::exception &e) {
                apiUtil->unlock_databaseinfo(db_info);
                response.StatusMsg = "Query fail: " + string(e.what());
                response.StatusCode = StatusOperationFailed;
                return;
            }
            string query_time_s = to_string(query_time);
            long rs_ansNum = 0;
            response.threadId = gutil::ThreadUtil::getThreadID();

            if (!is_update && (ret_val == -100))
            {
                rs_ansNum = max((long)rs.ansNum - rs.output_offset, 0L);
                long rs_outputlimit = (long)rs.output_limit;
                if (rs_outputlimit != -1)
                {
                    rs_ansNum = min(rs_ansNum, rs_outputlimit);
                }	

                int StatusCode = 0;
                // to void someone downloading all the data file by sparql query on purpose and to protect the data
                // if the ansNum too large, for example, larger than 100000, we limit the return ans.
                if (rs_ansNum > apiUtil->get_max_output_size())
                {
                    if (rs_outputlimit == -1 || rs_outputlimit > apiUtil->get_max_output_size())
                    {
                        rs_outputlimit = apiUtil->get_max_output_size();
                    }
                }
                response.StatusCode = StatusOK;
                response.StatusMsg = "success";
                response.ansNum = rs_ansNum;
                response.outputLimit = rs_outputlimit;
                response.queryTime = query_time_s;
                if (format_check)
                {
                    query_format_response_data(apiUtil, request, response, rs);
                }
                else
                {
                    rs.to_JSON(response.query_json);
                }
            }
            else if (is_update)
            {
                rs_ansNum = ret_val;
                SLOG_DEBUG("update query returns true. update num " + to_string(ret_val));
                response.StatusCode = StatusOK;
                response.StatusMsg = "update query returns true.";
                response.ansNum = rs_ansNum;
                response.queryTime = query_time_s;
                response.isUpdate = true;
            }
            else
            {
                msg = "query returns false.";
                SLOG_DEBUG(msg);
                response.StatusMsg = msg;
                response.StatusCode = StatusOperationFailed;   
            }
            // add callback task for query log start
            std::shared_ptr<DBQueryLogInfo> query_log_ptr = std::make_shared<DBQueryLogInfo>(query_start_time, request.remote_ip, sparql, 
                rs_ansNum, request.format, response.fileName, response.StatusCode, query_time, db_name);
            cb(query_log_ptr);
            // release ResultSet
            rs.release();
            return;
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Query fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
            return;
        }
    }

    void ApiHandler::query_cluster(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageQueryRequest& request, MessageQueryResponse& response, bool &is_update, const query_call& cb)
    {
        try
        {
            if (!query_check(apiUtil, request, response))
                return;
            std::string db_name = request.db_name;
            std::string username = request.username;
            std::string sparql = request.sparql;
            // check db_name paramter
            std::string msg;
            shared_ptr<DatabaseInfo> db_info;
            apiUtil->get_databaseinfo(db_name, db_info);
            // check database read lock
            if (apiUtil->rdlock_databaseinfo(db_info) == false)
            {
                response.StatusMsg = "get current database read lock fail.";
                response.StatusCode = StatusLossOfLock;
                return;
            }
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
            if(clusterManagerPtr->isFollower() && is_update)
            {
                apiUtil->unlock_databaseinfo(db_info);
                response.StatusMsg = "Redirect";
                response.StatusCode = StatusOK;
                return;
            }
            FILE *output = NULL;
            ResultSet rs;
            int ret_val;
            long query_time = gutil::TimeUtil::timestamp();
            shared_ptr<ofstream> clusterlog = nullptr;
            std::string cluster_db_path;
            std::string logpath;
            uint64 log_index;
            std::string query_start_time;
            ClusterUpdateType cluster_update_type = ClusterUpdateType::ClusterUpdateType_None;
            // update waiting follower reply
            if (is_update)
            {
                // send [prepare] heartbeat and wait response
                if (update_type == QueryTree::UpdateType::Insert_Data || update_type  == QueryTree::UpdateType::Insert_Clause) 
                    cluster_update_type = ClusterUpdateType::ClusterUpdateType_Insert;
                else
                    cluster_update_type = ClusterUpdateType::ClusterUpdateType_Delete;
                log_index = gutil::IdUtil::nextUID();
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
            try
            {
                SLOG_DEBUG("begin query...\n" + sparql);
                rs.setUsername(username);
                query_start_time = gutil::TimeUtil::now(NORM_DATETIME_MS_PATTERN);
                ret_val = db_info->getDatabase()->query(sparql, rs, output, update_flag_bool, false, nullptr, clusterlog);
                query_time = gutil::TimeUtil::timestamp() - query_time;
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
            response.threadId = gutil::ThreadUtil::getThreadID();
            string query_time_s = to_string(query_time);
            long rs_ansNum = 0;
            string file_name = "";
            if (!is_update && (ret_val == -100))
            {
                rs_ansNum = max((long)rs.ansNum - rs.output_offset, 0L);
                long rs_outputlimit = (long)rs.output_limit;
                if (rs_outputlimit != -1)
                {
                    rs_ansNum = min(rs_ansNum, rs_outputlimit);
                }

                // to void someone downloading all the data file by sparql query on purpose and to protect the data
                // if the ansNum too large, for example, larger than 100000, we limit the return ans.
                if (rs_ansNum > apiUtil->get_max_output_size())
                {
                    if (rs_outputlimit == -1 || rs_outputlimit > apiUtil->get_max_output_size())
                    {
                        rs_outputlimit = apiUtil->get_max_output_size();
                    }
                }

                response.StatusCode = StatusOK;
                response.StatusMsg = "success";
                response.ansNum = rs_ansNum;
                response.outputLimit = rs_outputlimit;
                response.queryTime = query_time_s;
                query_format_response_data(apiUtil, request, response, rs);
            }
            else if (is_update)
            {
                SLOG_DEBUG("update query returns true. update num " + to_string(ret_val));
                response.StatusCode = StatusOK;
                response.StatusMsg = "update query returns true.";
                response.ansNum = ret_val;
                response.queryTime = query_time_s;
                response.isUpdate = true;

                std::string json_str;
                response.toJsonString(json_str);
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
                            FileUtil::removePath(nt_file_path);
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
                    FileUtil::removePath(clusterManagerPtr->getDbDirPath(db_name)+log_file_name);
                }
            }
            else
            {
                msg = "search query returns false.";
                SLOG_DEBUG(msg);
                response.StatusMsg = msg;
                response.StatusCode = StatusOperationFailed;
            }
            // add callback task for query log start
            std::shared_ptr<DBQueryLogInfo> query_log_ptr = std::make_shared<DBQueryLogInfo>(query_start_time, request.remote_ip, sparql, 
                rs_ansNum, request.format, file_name, response.StatusCode, query_time, db_name);
            cb(query_log_ptr);
            // release ResultSet
            rs.release();
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Query fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }
}