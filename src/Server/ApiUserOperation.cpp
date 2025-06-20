#include "ApiProvider.h"

namespace gs
{
    void ApiHandler::show_users(shared_ptr<APIUtil>& apiUtil, gs::MessageShowUserResponse& response)
    {
        try
        {
            vector<shared_ptr<struct DBUserInfo>> userList;
            apiUtil->get_user_info(userList);
            if (userList.empty())
            {
                response.StatusMsg = "No Users";
                response.StatusCode = StatusOK;
                return;
            }
            size_t count = userList.size();
            for (size_t i = 0; i < count; i++)
            {
                shared_ptr<struct DBUserInfo> useInfo = userList[i];
                MessageShowUserResponseBody user;
                user.username = useInfo->getUsernname();
                user.password = useInfo->getPassword();
                user.query_privilege = useInfo->getQuery();
                user.update_privilege = useInfo->getUpdate();
                user.load_privilege = useInfo->getUnload();
                user.unload_privilege = useInfo->getUnload();
                user.backup_privilege = useInfo->getBackup();
                user.restore_privilege = useInfo->getRestore();
                user.export_privilege = useInfo->getExport();
                response.ResponseBody.push_back(user);
            }
            response.StatusCode = 0;
            response.StatusMsg = "success";
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Show user fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }

    void ApiHandler::user_manage(shared_ptr<APIUtil>& apiUtil, gs::MessageUserManageRequest& request, gs::MessageUserManageResponse& response)
    {
        try
        {
            std::string op_username = request.op_username;
            std::string msg;
            if (apiUtil->check_param_value("op_username", op_username, msg) == false)
            {
                response.StatusMsg = msg;
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            std::string op_password = request.op_password;
            std::string type = request.type;
            if (type != "2")
            {
                if (apiUtil->check_param_value("op_password", op_password, msg) == false)
                {
                    response.StatusMsg = msg;
                    response.StatusCode = StatusParamIsIllegal;
                    return;
                }
            }
            
            if (type == "1") // add user
            {
                // check user number
                if (apiUtil->check_user_count() == false)
                {
                    response.StatusMsg = "The total number of users more than max_user_num.";
                    response.StatusCode = StatusOperationConditionsAreNotSatisfied;
                    return;
                }
                if (apiUtil->user_add(op_username, op_password))
                {
                    response.StatusMsg = "Add user done.";
                }
                else
                {
                    response.StatusMsg = "Username already existed, add user failed.";
                    response.StatusCode = StatusOperationFailed;
                }
            }
            else if (type == "2") // delete user
            {
                if (op_username == GlobalTypedef::root_uname())
                {
                    response.StatusMsg = "You cannot delete root, delete user failed.";
                    response.StatusCode = StatusOperationFailed;
                }
                else if (apiUtil->user_delete(op_username))
                {
                    response.StatusMsg = "Delete user done.";

                }
                else
                {
                    response.StatusMsg = "Username not exist, delete user failed.";
                    response.StatusCode = StatusOperationFailed;
                }
            }
            else if (type == "3") // alert password
            {
                if (apiUtil->user_pwd_alert(op_username, op_password))
                {
                    response.StatusMsg = "Change password done.";
                }
                else
                {
                    response.StatusMsg = "Username not exist, change password failed.";
                    response.StatusCode = StatusOperationFailed;
                }
            }
            else
            {
                response.StatusMsg = "The operation is not support.";
                response.StatusCode = StatusParamIsIllegal;
            }
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "User manage fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }

    void ApiHandler::user_privilege_manage(shared_ptr<APIUtil>& apiUtil, gs::MessageUserPrivilegeManageRequest& request, gs::MessageUserPrivilegeManageResponse& response)
    {
        try
        {
            std::string type = request.type;
            std::string msg;
            if (apiUtil->check_param_value("type", type, msg) == false)
            {
                response.StatusMsg = msg;
                response.StatusCode = StatusParamIsIllegal;
                return;
            }  
            else if (type != "1" && type != "2" && type != "3")
            {
                response.StatusMsg =  "The type " + type + " is not support.";
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            std::string op_username = request.op_username;
            if (apiUtil->check_param_value("op_username", op_username, msg) == false)
            {
                response.StatusMsg = msg;
                response.StatusCode = StatusParamIsIllegal;
                return;
            } 
            else if (apiUtil->check_user_exist(op_username) == false)
            {
                response.StatusMsg =  "The username is not exists.";
                response.StatusCode = StatusOperationConditionsAreNotSatisfied;
                return;
            }
            else if (op_username == GlobalTypedef::root_uname())
            {
                response.StatusMsg =  "You can't change privileges for root user.";
                response.StatusCode = StatusOperationConditionsAreNotSatisfied;
                return;
            } 
            
            std::string db_name = request.db_name;
            std::string privileges = request.privileges;
            // check db_name and built status and privileges if not clear privilege
            if (type != "3")
            {
                if (apiUtil->check_param_value("db_name", db_name, msg) == false)
                {
                    response.StatusMsg = msg;
                    response.StatusCode = StatusParamIsIllegal;
                    return;
                }
                // check database exist
                if (apiUtil->check_db_built(db_name) == false)
                {
                    response.StatusMsg =  "Database not build yet.";
                    response.StatusCode = StatusOperationConditionsAreNotSatisfied;
                    return;
                }
                if (apiUtil->check_param_value("privileges", privileges, msg) == false)
                {
                    response.StatusMsg = msg;
                    response.StatusCode = StatusParamIsIllegal;
                    return;
                }
            } 
            if (type == "3")
            {
                // clear the user all privileges
                if (apiUtil->clear_privilege(op_username))
                {
                    response.StatusMsg = "Clear the all privileges for the user successfully!";
                }
                else
                {
                    response.StatusMsg =  "Clear the all privileges for the user fail.";
                    response.StatusCode = StatusOperationFailed;
                }
            }
            else
            {
                vector<string> privilege_vector;
                if (privileges.substr(privileges.length() - 1, 1) != ",")
                {
                    privileges = privileges + ",";
                }
                Util::split(privileges, ",", privilege_vector);
                vector<string> privilegeTypes;
                for (unsigned i = 0; i < privilege_vector.size(); i++)
                {
                    std::string temp_privilege_int = privilege_vector[i];
                    if (temp_privilege_int.empty())
                    {
                        continue;
                    }
                    if (temp_privilege_int == "1")
                    {
                        privilegeTypes.push_back("query");
                    }
                    else if (temp_privilege_int == "2")
                    {
                        privilegeTypes.push_back("load");
                    }
                    else if (temp_privilege_int == "3")
                    {
                        privilegeTypes.push_back("unload");
                    }
                    else if (temp_privilege_int == "4")
                    {
                        privilegeTypes.push_back("update");
                    }
                    else if (temp_privilege_int == "5")
                    {
                        privilegeTypes.push_back("backup");
                    }
                    else if (temp_privilege_int == "6")
                    {
                        privilegeTypes.push_back("restore");
                    }
                    else if (temp_privilege_int == "7")
                    {
                        privilegeTypes.push_back("export");
                    } 
                    else
                    {
                        SLOG_WARN("The privilege " + temp_privilege_int + " undefined.");
                        continue;
                    } 
                }
                string result = "";
                if (privilegeTypes.size() > 0) 
                {
                    string privilegeNames="";
                    for (size_t i = 0; i < privilegeTypes.size(); i++)
                    {
                        if (i > 0) 
                        {
                            privilegeNames = privilegeNames + ",";
                        }
                        privilegeNames = privilegeNames + privilegeTypes[i];
                    }
                    SLOG_DEBUG(type + "=" + privilegeNames);
                    if (type == "1")
                    {
                        if (apiUtil->add_privilege(op_username, privilegeTypes, db_name) == 0)
                        {
                            result = result + "add privilege " + privilegeNames + " failed.";
                            response.StatusMsg = result;
                            response.StatusCode = StatusOperationFailed;
                        }
                        else
                        {
                            result = result + "add privilege " + privilegeNames + " successfully.";
                            response.StatusMsg = result;
                        }
                    }
                    else if (type == "2")
                    {
                        if (apiUtil->del_privilege(op_username, privilegeTypes, db_name) == 0)
                        {
                            result += "delete privilege " + privilegeNames + " failed.";
                            response.StatusMsg = result;
                            response.StatusCode = StatusOperationFailed;
                        }
                        else
                        {
                            result += "delete privilege " + privilegeNames + " successfully.";
                            response.StatusMsg = result;
                        }
                    }
                    else
                    {
                        result = "the operation type is not support.";
                        response.StatusMsg = result;
                        response.StatusCode = StatusParamIsIllegal;
                    }
                }
                else
                {
                    response.StatusMsg = "not match any valid privilege, valid values between 1 and 7.";
                    response.StatusCode = StatusParamIsIllegal;
                }
            }
        }
        catch (const std::exception &e)
        {
            std::string error = "User privilege manage fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::user_passworrd(shared_ptr<APIUtil>& apiUtil, gs::MessageUserPasswordRequest& request, gs::MessageUserPasswordResponse& response)
    {
        try
        {
            std::string op_password = request.op_password;
            std::string msg;
            if (apiUtil->check_param_value("op_password", op_password, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            std::string username = request.username;
            if (apiUtil->check_user_exist(username) == false)
            {
                msg =  "Username does not exist.";
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            if (apiUtil->user_pwd_alert(username, op_password))
            {
                response.StatusMsg = "Change password done.";
            }
            else
            {
                response.StatusMsg = "Change password fail.";
            }
        }
        catch (const std::exception &e)
        {
            string error = "Change password fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }
}