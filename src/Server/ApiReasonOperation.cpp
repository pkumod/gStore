#include "ApiProvider.h"

namespace server
{
    void ApiHandler::reason_manage_task(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, server::MessageReasonManageResponse& response, rapidjson::Document& json_data)
    {
        // try
        // {
        //     std::string db_name = jsonParam(json_data, "db_name");
        //     std::string type= jsonParam(json_data, "type");
        //     std::string operation="AddReason";
        //     std::string msg;
        //     if (apiUtil->check_param_value("db_name", db_name, msg) == false)
        //     {
        //         response.Error(StatusParamIsIllegal, msg);
        //         return;
        //     }
        //     if(apiUtil->check_param_value("type", type, msg) == false)
        //     {
        //         response.Error(StatusParamIsIllegal, msg);
        //         return;
        //     }
        //     if (type == "1") // add Reason Rule
        //     {
        //         operation="AddReasonRule";
        //         // check user number
        //         if(json_data.HasMember("ruleinfo")==false)
        //         {
        //             msg = "the data has not the rule information";
        //             response.Error(StatusParamIsIllegal, msg);
        //             return;
        //         }
        //         Value reasonInfo=json_data["ruleinfo"].GetObject();
        //         Document::AllocatorType &allocator = json_data.GetAllocator();
        //         std::string createtime = Util::get_date_time();
        //         reasonInfo.AddMember("status","新建",allocator);
        //         reasonInfo.AddMember("createtime",StringRef(createtime.c_str()),allocator);
        //         ReasonOperationResult resultInfo= ReasonHelper::saveReasonRuleInfo(reasonInfo,db_name,_db_home,_db_suffix);
        //         if(resultInfo.issuccess==1)
        //         {
        //             response.StatusMsg = "Add Reason Rule done."+resultInfo.error_message;
        //         }
        //         else
        //         {
        //             msg="Add Reason Rule Fail. " + resultInfo.error_message;
        //             response.Error(StatusOperationFailed, msg);
        //         }
        //     }
        //     else if (type == "2") // listReason
        //     {
        //         operation = "listReasonRules";
        //         string db_path=_db_home+db_name+_db_suffix;
        //         vector<string> liststr = ReasonHelper::getReasonRuleList(db_path);
        //         stringstream str_stream;
        //         str_stream << "[";
        //         for(int i=0;i<liststr.size();i++)
        //         {
        //             if (i > 0) 
        //                 str_stream << ",";
        //             str_stream << liststr[i];
        //         }
        //         str_stream << "]";
        //         string arrayStr = str_stream.str();
        //         Document list;
        //         list.SetArray();
        //         list.Parse(arrayStr.c_str());
        //         if (list.HasParseError()) 
        //         {

        //             msg = "pasrse rulefiles error:" + arrayStr;
        //             response.Error(StatusOperationFailed, msg);
        //         } 
        //         else 
        //         {
        //             response.list = 
        //             Json doc;
        //             Document::AllocatorType &allocator = doc.GetAllocator();
        //             unsigned num = list.Size();
        //             doc.SetObject();
        //             doc.AddMember("StatusCode", 0, allocator);
        //             doc.AddMember("StatusMsg", "ok", allocator);
        //             doc.AddMember("list", list.Move(), allocator);
        //             doc.AddMember("num",  num, allocator);
        //             response->Json(doc);
        //         }
        //     }
        //     else if (type == "3") // Compile Reason Rule
        //     {
        //         operation = "compileReasonRule";
        //         string rulename=json_data["rulename"].GetString();
        //         ReasonSparql resultInfo=ReasonHelper::compileReasonRule(rulename,db_name,_db_home,_db_suffix);
        //         Document doc;
        //         doc.SetObject();
        //         Document::AllocatorType &allocator = doc.GetAllocator();
        //         if(resultInfo.issuccess==0)
        //         {
        //             response->Error(StatusOperationFailed,resultInfo.error_message);
        //         }
        //         else
        //         {
        //             doc.AddMember("StatusCode",0,allocator);
        //             doc.AddMember("insert_sparql",StringRef(resultInfo.insert_sparql.c_str()),allocator);
        //             doc.AddMember("delete_sparql",StringRef(resultInfo.delete_sparql.c_str()),allocator);
        //             doc.AddMember("check_sparql",StringRef(resultInfo.check_sparql.c_str()),allocator);
        //             response->Json(doc);
        //         }
        //     }
        //     else if(type=="4") //execute Reason
        //     {
        //         operation = "executeReasonRule";
        //         string rulename=json_data["rulename"].GetString();
        //         string username=json_data["username"].GetString();
        //         ReasonSparql resultInfo= ReasonHelper::executeReasonRule(rulename,db_name,_db_home,_db_suffix);
        //         if(resultInfo.issuccess==0)
        //         {
        //             response->Error(StatusOperationFailed,resultInfo.error_message);
        //             return;
        //         }
        //         if (apiUtil->check_db_built(db_name) == false)
        //         {
        //             msg = "the database is not exist!";
        //             response->Error(StatusOperationFailed, msg);
        //             return;
        //         }
        //         if (apiUtil->check_db_loaded(db_name) == false)
        //         {
        //             msg = "Database not load yet!";
        //             response->Error(StatusOperationFailed, msg);
        //             return;
        //         }
        //         shared_ptr<DatabaseInfo> db_info;
        //         apiUtil->get_databaseinfo(db_name, db_info);
        //         if (apiUtil->trywrlock_databaseinfo(db_info) == false)
        //         {
        //             msg = "get current database write lock fail.";
        //             response->Error(StatusOperationFailed, msg);
        //             return;
        //         }
        //         ResultSet rs;
        //         int ret_val;
        //         FILE *output = NULL;
        //         string sparql = resultInfo.insert_sparql;
        //         bool update_flag_bool = true;
        //         try
        //         {
        //             // SLOG_DEBUG("begin query...");
        //             rs.setUsername(username);
        //             ret_val = db_info->getDatabase()->query(sparql, rs, output, update_flag_bool, false, nullptr);
        //             db_info->getDatabase()->save();
        //             apiUtil->unlock_databaseinfo(db_info);
        //         }
        //         catch (string exception_msg)
        //         {
        //             string content = exception_msg;
        //             apiUtil->unlock_databaseinfo(db_info);
        //             response->Error(StatusOperationFailed,content);
        //             return;
        //         }
        //         catch (const std::runtime_error &e2)
        //         {
        //             string content = e2.what();
        //             apiUtil->unlock_databaseinfo(db_info);
        //             response->Error(StatusOperationFailed,content);
        //             return;
        //         }
        //         catch (...)
        //         {
        //             string content = "unknow error";
        //             apiUtil->unlock_databaseinfo(db_info);
        //             response->Error(StatusOperationFailed,content);
        //             return;
        //         }
                
        //         Document doc;
        //         doc.SetObject();
        //         Document::AllocatorType &allocator = doc.GetAllocator();
        //         doc.AddMember("insert_sparql",StringRef(resultInfo.insert_sparql.c_str()),allocator);
        //         doc.AddMember("AnsNum", ret_val, allocator);
        //         doc.AddMember("StatusCode", 0, allocator);
        //         doc.AddMember("StatusMsg", "ok", allocator);
        //         ReasonHelper::updateReasonRuleStatus(rulename, db_name, "已执行",_db_home,_db_suffix);
        //         response->Json(doc);
        //     }
        //     else if(type=="5")
        //     {
        //         operation = "disableReasonRule";
        //         string rulename=json_data["rulename"].GetString();
        //         ReasonSparql resultInfo= ReasonHelper::disableReasonRule(rulename,db_name,_db_home,_db_suffix);
        //         if(resultInfo.issuccess==0)
        //         {
        //             response->Error(StatusOperationFailed,resultInfo.error_message);
        //             return;
        //         }
        //         string username = json_data["username"].GetString();
        //         if(apiUtil->check_db_built(db_name)==false)
        //         {
        //             msg = "the database is not exist!";
        //             response->Error(StatusOperationFailed, msg);
        //             return;
        //         }
        //         // check database load status
        //         if (apiUtil->check_db_loaded(db_name) == false)
        //         {
        //             msg = "Database not load yet!";
        //             response->Error(StatusOperationFailed, msg);
        //             return;
        //         }
        //         shared_ptr<DatabaseInfo> db_info;
        //         apiUtil->get_databaseinfo(db_name, db_info);
        //         bool update_flag_bool=true;
        //         if (apiUtil->trywrlock_databaseinfo(db_info) ==  false)
        //         {
        //             msg = "get current database write lock fail.";
        //             response->Error(StatusOperationFailed, msg);
        //             return;
        //         }
        //         ResultSet rs;
        //         int ret_val;
        //         FILE *output = NULL;
        //         string sparql = resultInfo.delete_sparql;
        //         try
        //         {
        //             // SLOG_DEBUG("begin query...");
        //             rs.setUsername(username);
        //             ret_val = db_info->getDatabase()->query(sparql, rs, output, update_flag_bool, false, nullptr);
        //             db_info->getDatabase()->save();
        //             apiUtil->unlock_databaseinfo(db_info);
        //         }
        //         catch (string exception_msg)
        //         {
        //             string content = exception_msg;
        //             apiUtil->unlock_databaseinfo(db_info);
        //             response->Error(StatusOperationFailed,content);
        //             return;
        //         }
        //         catch (const std::runtime_error &e2)
        //         {
        //             string content = e2.what();
        //             apiUtil->unlock_databaseinfo(db_info);
        //             response->Error(StatusOperationFailed,content);
        //             return;
        //         }
        //         catch (...)
        //         {
        //             string content = "unknow error";
        //             apiUtil->unlock_databaseinfo(db_info);
        //             response->Error(StatusOperationFailed,content);
        //             return;
        //         }
        //         Document doc;
        //         doc.SetObject();
        //         Document::AllocatorType &allocator = doc.GetAllocator();
        //         doc.AddMember("delete_sparql",StringRef(resultInfo.delete_sparql.c_str()),allocator);
        //         doc.AddMember("AnsNum",ret_val,allocator);
        //         doc.AddMember("StatusCode", 0, allocator);
        //         doc.AddMember("StatusMsg", "ok", allocator);
        //         ReasonHelper::updateReasonRuleStatus(rulename, db_name, "已失效",_db_home,_db_suffix);
        //         response->Json(doc);
        //     }
        //     else if(type=="6")
        //     {
        //         operation = "showReasonRule";
        //         string rulename=json_data["rulename"].GetString();
        //         ReasonOperationResult resultInfo= ReasonHelper::getReasonInfo(rulename,db_name,_db_home,_db_suffix);
        //         if(resultInfo.issuccess==0)
        //         {
        //             response->Error(StatusOperationFailed,resultInfo.error_message);
        //         }			
        //         else
        //         {
        //             // 输出格式化的JSON
        //             std::string ruleinfo = "{\"ruleinfo\":" + resultInfo.error_message + "}";
        //             Document doc;
        //             doc.SetObject();
        //             doc.Parse(ruleinfo.c_str());
        //             Document::AllocatorType &allocator = doc.GetAllocator();
        //             doc.AddMember("StatusCode",0,allocator);
        //             doc.AddMember("StatusMsg","ok",allocator);
        //             response->Json(doc);
        //         }
        //     }
        //     else if (type == "7")
        //     {
        //         operation = "deleteReasonRule";
        //         string rulename = json_data["rulename"].GetString();
        //         ReasonOperationResult resultInfo = ReasonHelper::getReasonInfo(rulename, db_name, _db_home, _db_suffix);
        //         if (resultInfo.issuccess == 0)
        //         {
        //             response->Error(StatusOperationFailed,resultInfo.error_message);
        //             return;
        //         }
        //         else
        //         {
        //             // 输出格式化的JSON
        //             Document doc;
        //             doc.SetObject();
        //             doc.Parse(resultInfo.error_message.c_str());
        //             Document::AllocatorType &allocator = doc.GetAllocator();
        //             if (doc.HasMember("status")&&doc["status"].GetString() == "已执行")
        //             {
        //                 string error_msg = "该规则已经执行，请先让该规则失效后再执行";
        //                 response->Error(StatusOperationFailed,error_msg);
        //                 return;
        //             }
        //             ReasonOperationResult resultInfo2 = ReasonHelper::removeReasonRule(rulename, db_name, _db_home, _db_suffix);
        //             if (resultInfo2.issuccess == 1)
        //             {
        //                 Document doc2;
        //                 doc2.SetObject();
        //                 doc2.AddMember("StatusCode", 0, allocator);
        //                 doc2.AddMember("StatusMsg", StringRef(resultInfo2.error_message.c_str()), allocator);
        //                 response->Json(doc2);
        //             }
        //             else
        //             {
        //                 response->Error(StatusOperationFailed,resultInfo2.error_message);
        //             }
        //         }
        //     }
        //     else if (type == "8")
        //     {
        //         operation = "checkReasonRule";
        //         string rulename=json_data["rulename"].GetString();
        //         ReasonSparql resultInfo= ReasonHelper::getCheckSparql(rulename,db_name,_db_home,_db_suffix);
        //         if(resultInfo.issuccess==0)
        //         {
        //             response->Error(StatusOperationFailed,resultInfo.error_message);
        //             return;
        //         }
        //         if(apiUtil->check_db_built(db_name)==false)
        //         {
        //             msg = "the database is not exist!";
        //             response->Error(StatusOperationFailed, msg);
        //             return;
        //         }
        //         if(apiUtil->check_db_loaded(db_name)==false)
        //         {
        //             msg = "Database not load yet.";
        //             response->Error(StatusOperationFailed, msg);
        //             return;
        //         }
                
        //         shared_ptr<DatabaseInfo> db_info;
        //         apiUtil->get_databaseinfo(db_name, db_info);
        //         if (apiUtil->rdlock_databaseinfo(db_info))
        //         {
        //             msg = "get current database read lock fail.";
        //             response->Error(StatusOperationFailed, msg);
        //             return;
        //         }
        //         ResultSet rs;
        //         bool update_flag_bool = true;
        //         int ret_val;
        //         FILE *output = NULL;
        //         string username = jsonParam(json_data, "username");
        //         string sparql = resultInfo.check_sparql;
        //         try
        //         {
        //             rs.setUsername(username);
        //             ret_val = db_info->getDatabase()->query(sparql, rs, output, update_flag_bool, false, nullptr);
        //             apiUtil->unlock_databaseinfo(db_info);
        //         }
        //         catch (string exception_msg)
        //         {
        //             string content = exception_msg;
        //             apiUtil->unlock_databaseinfo(db_info);
        //             response->Error(StatusOperationFailed,content);
        //             ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,0,_db_home,_db_suffix,content);
        //             return;
        //         }
        //         catch (const std::runtime_error &e2)
        //         {
        //             string content = e2.what();
        //             apiUtil->unlock_databaseinfo(db_info);
        //             response->Error(StatusOperationFailed,content);
        //             ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,0,_db_home,_db_suffix,content);
        //             return;
        //         }
        //         catch (...)
        //         {
        //             string content = "unknow error";
        //             apiUtil->unlock_databaseinfo(db_info);
        //             response->Error(StatusOperationFailed,content);
        //             ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,0,_db_home,_db_suffix,content);
        //             return;
        //         }
        //         int effectNum = 0;
        //         string checkMsg = "ok";
        //         string json= rs.to_JSON();
        //         Document doc2;
        //         doc2.SetObject();
        //         doc2.Parse(json.c_str());
        //         if(doc2.HasParseError())
        //         {
        //             checkMsg = "query result is not json format!";
        //             effectNum = 0;
        //         }
        //         else
        //         {
        //             if(doc2.HasMember("results"))
        //             {
        //                 Value results=doc2["results"].GetObject();
        //                 if(results.HasMember("bindings"))
        //                 {
        //                     Value bindings=results["bindings"].GetArray();
        //                     if(bindings.Size()>0)
        //                     {
        //                         Value resultobj=bindings[0]["result"].GetObject();
        //                         if(resultobj.HasMember("value"))
        //                         {
        //                             string result_value=resultobj["value"].GetString();
        //                             // int result_value_int=Util::string2int(result_value);
        //                             effectNum=Util::string2int(result_value);
        //                         }
        //                         else
        //                         {
        //                             effectNum=0;
        //                         }
        //                     }
        //                     else
        //                     {
        //                         effectNum=0;
        //                     }
        //                 }
        //             }
        //         }
        //         Document doc;
        //         doc.SetObject();
        //         Document::AllocatorType &allocator = doc.GetAllocator();
        //         doc.AddMember("check_sparql", StringRef(resultInfo.check_sparql.c_str()), allocator);
        //         doc.AddMember("effectNum", effectNum, allocator);
        //         doc.AddMember("checkMsg", StringRef(checkMsg.c_str()), allocator);
        //         doc.AddMember("StatusCode", 0, allocator);
        //         doc.AddMember("StatusMsg", "ok", allocator);
        //         ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,effectNum,_db_home,_db_suffix,checkMsg);
        //         ReasonHelper::updateReasonRuleStatus(rulename, db_name, "已校验",_db_home,_db_suffix);
        //         response->Json(doc);
        //     }
        //     else
        //     {
        //         msg =  "The operation is not support.";
        //         response->Error(StatusParamIsIllegal, msg);
        //     }
        // }
        // catch (const std::exception &e)
        // {
        //     string error = "Reason Manage fail: " + string(e.what());
        //     response->Error(StatusOperationFailed, error);
        // }
    }
}