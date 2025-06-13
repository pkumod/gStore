#include "ApiProvider.h"

namespace server
{
    void ApiHandler::reason_manage(shared_ptr<APIUtil> &apiUtil, server::MessageReasonManageResponse &response, nlohmann::json &json_data)
    {
        try
        {
            std::string db_name = JsonUtil::jsonParam(json_data, "db_name");
            std::string type = JsonUtil::jsonParam(json_data, "type");
            std::string operation = "AddReason";
            std::string _db_home = GlobalTypedef::db_home();
            std::string _db_suffix = GlobalTypedef::db_suffix();
            std::string _db_path = _db_home + db_name + _db_suffix;
            std::string msg;
            response.type = type;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            if (apiUtil->check_param_value("type", type, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            if (type == "1") // add Reason Rule
            {
                operation = "AddReasonRule";
                // check user number
                if (json_data.contains("ruleinfo") == false)
                {
                    msg = "the data has not the rule information";
                    response.Error(StatusParamIsIllegal, msg);
                    return;
                }
                nlohmann::json reasonInfo = json_data["ruleinfo"];
                reasonInfo["status"] = "新建";
                reasonInfo["createtime"] = gutil::TimeUtil::now(NORM_DATETIME_PATTERN);
                ReasonOperationResult resultInfo = ReasonHelper::saveReasonRuleInfo(reasonInfo, _db_path);
                if (resultInfo.issuccess == 1)
                {
                    response.StatusMsg = "Add Reason Rule done." + resultInfo.error_message;
                }
                else
                {
                    msg = "Add Reason Rule Fail. " + resultInfo.error_message;
                    response.Error(StatusOperationFailed, msg);
                }
            }
            else if (type == "2") // listReason
            {
                operation = "listReasonRules";
                string db_path = _db_home + db_name + _db_suffix;
                vector<string> liststr = ReasonHelper::getReasonRuleList(db_path);
                stringstream str_stream;
                str_stream << "[";
                for (int i = 0; i < liststr.size(); i++)
                {
                    if (i > 0)
                        str_stream << ",";
                    str_stream << liststr[i];
                }
                str_stream << "]";
                string arrayStr = str_stream.str();
                try
                {
                    nlohmann::json list = nlohmann::json::parse(arrayStr);
                    response.StatusMsg = "ok";
                    response.list = list;
                    if (list.is_array())
                    {
                        response.num = list.size();
                    }
                }
                catch (nlohmann::json::exception &e)
                {
                    msg = "pasrse rulefiles error:" + arrayStr;
                    response.Error(StatusOperationFailed, msg);
                }
            }
            else if (type == "3") // Compile Reason Rule
            {
                operation = "compileReasonRule";
                string rulename = JsonUtil::jsonParam(json_data, "rulename");
                ReasonSparql resultInfo = ReasonHelper::compileReasonRule(rulename, _db_path);
                if (resultInfo.issuccess == 0)
                {
                    response.Error(StatusOperationFailed, resultInfo.error_message);
                }
                else
                {
                    response.insert_sparql = resultInfo.insert_sparql;
                    response.delete_sparql = resultInfo.delete_sparql;
                    response.check_sparql = resultInfo.check_sparql;
                    response.StatusMsg = "ok";
                }
            }
            else if (type == "4") // execute Reason
            {
                operation = "executeReasonRule";
                string rulename = JsonUtil::jsonParam(json_data, "rulename");
                string username = JsonUtil::jsonParam(json_data, "username");
                ReasonSparql resultInfo = ReasonHelper::executeReasonRule(rulename, _db_path);
                if (resultInfo.issuccess == 0)
                {
                    response.Error(StatusOperationFailed, resultInfo.error_message);
                    return;
                }
                if (apiUtil->check_db_built(db_name) == false)
                {
                    msg = "the database is not exist!";
                    response.Error(StatusOperationFailed, msg);
                    return;
                }
                if (apiUtil->check_db_loaded(db_name) == false)
                {
                    msg = "Database not load yet!";
                    response.Error(StatusOperationFailed, msg);
                    return;
                }
                shared_ptr<DatabaseInfo> db_info;
                apiUtil->get_databaseinfo(db_name, db_info);
                if (apiUtil->trywrlock_databaseinfo(db_info) == false)
                {
                    msg = "get current database write lock fail.";
                    response.Error(StatusOperationFailed, msg);
                    return;
                }
                ResultSet rs;
                int ret_val;
                FILE *output = NULL;
                string sparql = resultInfo.insert_sparql;
                bool update_flag_bool = true;
                try
                {
                    // SLOG_DEBUG("begin query...");
                    rs.setUsername(username);
                    ret_val = db_info->getDatabase()->query(sparql, rs, output, update_flag_bool, false, nullptr);
                    db_info->getDatabase()->save();
                    apiUtil->unlock_databaseinfo(db_info);
                }
                catch (string exception_msg)
                {
                    string content = exception_msg;
                    apiUtil->unlock_databaseinfo(db_info);
                    response.Error(StatusOperationFailed, content);
                    return;
                }
                catch (const std::runtime_error &e2)
                {
                    string content = e2.what();
                    apiUtil->unlock_databaseinfo(db_info);
                    response.Error(StatusOperationFailed, content);
                    return;
                }
                catch (...)
                {
                    string content = "unknow error";
                    apiUtil->unlock_databaseinfo(db_info);
                    response.Error(StatusOperationFailed, content);
                    return;
                }
                ReasonHelper::updateReasonRuleStatus(rulename, "已执行", _db_path);
                response.insert_sparql = resultInfo.insert_sparql;
                response.select_sparql = resultInfo.select_sparql;
                response.num = ret_val;
                response.StatusMsg = "ok";
            }
            else if (type == "5")
            {
                operation = "disableReasonRule";
                string rulename = JsonUtil::jsonParam(json_data, "rulename");
                ReasonSparql resultInfo = ReasonHelper::disableReasonRule(rulename, _db_path);
                if (resultInfo.issuccess == 0)
                {
                    response.Error(StatusOperationFailed, resultInfo.error_message);
                    return;
                }
                string username = JsonUtil::jsonParam(json_data, "username");
                if (apiUtil->check_db_built(db_name) == false)
                {
                    msg = "the database is not exist!";
                    response.Error(StatusOperationFailed, msg);
                    return;
                }
                // check database load status
                if (apiUtil->check_db_loaded(db_name) == false)
                {
                    msg = "Database not load yet!";
                    response.Error(StatusOperationFailed, msg);
                    return;
                }
                shared_ptr<DatabaseInfo> db_info;
                apiUtil->get_databaseinfo(db_name, db_info);
                bool update_flag_bool = true;
                if (apiUtil->trywrlock_databaseinfo(db_info) == false)
                {
                    msg = "get current database write lock fail.";
                    response.Error(StatusOperationFailed, msg);
                    return;
                }
                ResultSet rs;
                int ret_val;
                FILE *output = NULL;
                string sparql = resultInfo.delete_sparql;
                try
                {
                    // SLOG_DEBUG("begin query...");
                    rs.setUsername(username);
                    ret_val = db_info->getDatabase()->query(sparql, rs, output, update_flag_bool, false, nullptr);
                    db_info->getDatabase()->save();
                    apiUtil->unlock_databaseinfo(db_info);
                }
                catch (string exception_msg)
                {
                    string content = exception_msg;
                    apiUtil->unlock_databaseinfo(db_info);
                    response.Error(StatusOperationFailed, content);
                    return;
                }
                catch (const std::runtime_error &e2)
                {
                    string content = e2.what();
                    apiUtil->unlock_databaseinfo(db_info);
                    response.Error(StatusOperationFailed, content);
                    return;
                }
                catch (...)
                {
                    string content = "unknow error";
                    apiUtil->unlock_databaseinfo(db_info);
                    response.Error(StatusOperationFailed, content);
                    return;
                }

                ReasonHelper::updateReasonRuleStatus(rulename, "已失效", _db_path);
                response.delete_sparql = resultInfo.delete_sparql;
                response.num = ret_val;
                response.StatusMsg = "ok";
            }
            else if (type == "6")
            {
                operation = "showReasonRule";
                string rulename = JsonUtil::jsonParam(json_data, "rulename");
                ReasonOperationResult resultInfo = ReasonHelper::getReasonInfo(rulename, _db_path);
                if (resultInfo.issuccess == 0)
                {
                    response.Error(StatusOperationFailed, resultInfo.error_message);
                }
                else
                {
                    // 输出格式化的JSON
                    try
                    {
                        response.ruleinfo = nlohmann::json::parse(resultInfo.error_message);
                        response.StatusMsg = "ok";
                    }
                    catch (nlohmann::json::exception &e)
                    {
                        msg = "pasrse ruleinfo error:" + resultInfo.error_message;
                        response.Error(StatusOperationFailed, msg);
                    }
                }
            }
            else if (type == "7")
            {
                operation = "deleteReasonRule";
                string rulename = JsonUtil::jsonParam(json_data, "rulename");
                ReasonOperationResult resultInfo = ReasonHelper::getReasonInfo(rulename, _db_path);
                if (resultInfo.issuccess == 0)
                {
                    response.Error(StatusOperationFailed, resultInfo.error_message);
                    return;
                }
                else
                {
                    // 输出格式化的JSON
                    nlohmann::json doc = nlohmann::json::parse(resultInfo.error_message);
                    if (doc.contains("status") && JsonUtil::jsonParam(doc, "status") == "已执行")
                    {
                        string error_msg = "该规则已经执行，请先让该规则失效后再执行";
                        response.Error(StatusOperationFailed, error_msg);
                        return;
                    }
                    ReasonOperationResult resultInfo2 = ReasonHelper::removeReasonRule(rulename, _db_path);
                    if (resultInfo2.issuccess == 1)
                    {
                        response.StatusMsg = resultInfo2.error_message;
                    }
                    else
                    {
                        response.Error(StatusOperationFailed, resultInfo2.error_message);
                    }
                }
            }
            else if (type == "8")
            {
                operation = "checkReasonRule";
                string rulename = JsonUtil::jsonParam(json_data, "rulename");
                ReasonSparql resultInfo = ReasonHelper::getCheckSparql(rulename, _db_path);
                if (resultInfo.issuccess == 0)
                {
                    response.Error(StatusOperationFailed, resultInfo.error_message);
                    return;
                }
                if (apiUtil->check_db_built(db_name) == false)
                {
                    msg = "the database is not exist!";
                    response.Error(StatusOperationFailed, msg);
                    return;
                }
                if (apiUtil->check_db_loaded(db_name) == false)
                {
                    msg = "Database not load yet.";
                    response.Error(StatusOperationFailed, msg);
                    return;
                }

                shared_ptr<DatabaseInfo> db_info;
                apiUtil->get_databaseinfo(db_name, db_info);
                if (apiUtil->rdlock_databaseinfo(db_info) == false)
                {
                    msg = "get current database read lock fail.";
                    response.Error(StatusOperationFailed, msg);
                    return;
                }
                ResultSet rs;
                bool update_flag_bool = true;
                int ret_val;
                FILE *output = NULL;
                string username = JsonUtil::jsonParam(json_data, "username");
                string sparql = resultInfo.check_sparql;
                try
                {
                    rs.setUsername(username);
                    ret_val = db_info->getDatabase()->query(sparql, rs, output, update_flag_bool, false, nullptr);
                    apiUtil->unlock_databaseinfo(db_info);
                }
                catch (string exception_msg)
                {
                    string content = exception_msg;
                    apiUtil->unlock_databaseinfo(db_info);
                    response.Error(StatusOperationFailed, content);
                    ReasonHelper::updateReasonRuleEffectNum(rulename, _db_path, 0, content);
                    return;
                }
                catch (const std::runtime_error &e2)
                {
                    string content = e2.what();
                    apiUtil->unlock_databaseinfo(db_info);
                    response.Error(StatusOperationFailed, content);
                    ReasonHelper::updateReasonRuleEffectNum(rulename, _db_path, 0, content);
                    return;
                }
                catch (...)
                {
                    string content = "unknow error";
                    apiUtil->unlock_databaseinfo(db_info);
                    response.Error(StatusOperationFailed, content);
                    ReasonHelper::updateReasonRuleEffectNum(rulename, _db_path, 0, content);
                    return;
                }
                int effectNum = 0;
                string checkMsg = "ok";
                try
                {
                    std::string countNum = rs.answer[0][0];
                    SLOG_TRACE("countNum: " + countNum);
                    size_t pos = countNum.find("\"^^<");
                    if (pos != string::npos) 
                    {
                        countNum = countNum.substr(1, pos - 1);
                    }
                    effectNum = std::stoi(countNum);
                }
                catch(const std::exception& e)
                {
                    SLOG_ERROR("parse countNum error: " << e.what());
                }
                
                // nlohmann::json json;
                // if (rs.to_JSON(json))
                // {
                //     checkMsg = "query result is not json format!";
                //     effectNum = 0;
                // }
                // else
                // {
                //     if (json.contains("results"))
                //     {
                //         nlohmann::json results = json["results"];
                //         if (results.contains("bindings"))
                //         {
                //             nlohmann::json bindings = results["bindings"];
                //             if (bindings.size() > 0)
                //             {
                //                 nlohmann::json resultobj = bindings[0]["result"];
                //                 if (resultobj.contains("value"))
                //                 {
                //                     string result_value;
                //                     resultobj.at("value").get_to(result_value);
                //                     effectNum = stoi(result_value);
                //                 }
                //                 else
                //                 {
                //                     effectNum = 0;
                //                 }
                //             }
                //             else
                //             {
                //                 effectNum = 0;
                //             }
                //         }
                //     }
                // }
                ReasonHelper::updateReasonRuleEffectNum(rulename, _db_path, effectNum, checkMsg);
                ReasonHelper::updateReasonRuleStatus(rulename, "已校验", _db_path);
                response.check_sparql = resultInfo.check_sparql;
                response.num = effectNum;
                response.checkMsg = checkMsg;
                response.StatusMsg = "ok";
            }
            else
            {
                msg = "The operation is not support.";
                response.Error(StatusParamIsIllegal, msg);
            }
        }
        catch (const std::exception &e)
        {
            string error = "Reason Manage fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }
}