/*
 * @Author: your name
 * @Date: 2022-02-28 10:31:06
 * @LastEditTime: 2022-04-19 17:15:57
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /gStore/GRPC/grpcImpl.cpp
 */
#include "grpcImpl.h"
#define TEST_IP ""
#define HTTP_TYPE "grpc"

GrpcImpl::GrpcImpl(int argc, char *argv[])
{
    apiUtil = new APIUtil();
    string db_name = "";
    string port = "";
    db_name = Util::getArgValue(argc, argv, "db", "database");
    port = Util::getArgValue(argc, argv, "p", "port", "9000");
    bool load_csr = false;
    load_csr = Util::string2int(Util::getArgValue(argc, argv, "c", "csr", "0"));
    int ret = apiUtil->initialize("grpc", port, db_name, load_csr);
    if(ret == -1)
    {
        _exit(1);
    }
}

GrpcImpl::~GrpcImpl()
{  
    if (apiUtil)
    {
        delete apiUtil;
    }
}

void GrpcImpl::api(CommonRequest *request, CommonResponse *response, srpc::RPCContext *ctx)
{
    string operation = request->operation();
    string remoteIP = ctx->get_remote_ip();
    WFGoTask *task;

    std::cout << "------------------------ grpc-api ------------------------" << std::endl;
    std::cout << "seq_id: " << ctx->get_seqid() << std::endl;
    std::cout << "remote_ip: " <<  remoteIP << std::endl;
    std::cout << "operation: " << operation << std::endl;
    std::cout << "request_time: " << Util::get_date_time() << std::endl;
    std::cout << "post content: " << request->ShortDebugString() << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;

    if (operation == "show")
    {
        task = WFTaskFactory::create_go_task("show_task", &GrpcImpl::show_task, this, request, response, ctx);
    }
    else if (operation == "load")
    {
        task = WFTaskFactory::create_go_task("load_task", &GrpcImpl::load_task, this, request, response, ctx);
    }
    else if (operation == "query")
    {
        task = WFTaskFactory::create_go_task("query_task", &GrpcImpl::query_task, this, request, response, ctx);
    }
    else if (operation == "build")
    {
        task = WFTaskFactory::create_go_task("build_task", &GrpcImpl::build_task, this, request, response, ctx);
    }
    else if (operation == "check")
    {
        task = WFTaskFactory::create_go_task("check_task", &GrpcImpl::check_task, this, request, response, ctx);
    }
    else if (operation == "monitor")
    {
        task = WFTaskFactory::create_go_task("monitor_task", &GrpcImpl::monitor_task, this, request, response, ctx);
    }
    else if (operation == "unload")
    {
        task = WFTaskFactory::create_go_task("unload_task", &GrpcImpl::unload_task, this, request, response, ctx);
    }
    else if (operation == "drop")
    {
        task = WFTaskFactory::create_go_task("drop_task", &GrpcImpl::drop_task, this, request, response, ctx);
    }
    else if ( operation == "backup")
    {
        task = WFTaskFactory::create_go_task("backup_task", &GrpcImpl::backup_task, this, request, response, ctx);
    }
    else if ( operation == "restore")
    {
        task = WFTaskFactory::create_go_task("restore_task", &GrpcImpl::restore_task, this, request, response, ctx);
    }
    else if ( operation == "export")
    {
        task = WFTaskFactory::create_go_task("export_task", &GrpcImpl::export_task, this, request, response, ctx);
    }
    else if ( operation == "login")
    {
        task = WFTaskFactory::create_go_task("login_task", &GrpcImpl::login_task, this, request, response, ctx);
    }
    else if ( operation == "begin")
    {
        task = WFTaskFactory::create_go_task("begin_task", &GrpcImpl::begin_task, this, request, response, ctx);
    }
    else if ( operation == "tquery")
    {
        task = WFTaskFactory::create_go_task("tquery_task", &GrpcImpl::tquery_task, this, request, response, ctx);
    }
    else if ( operation == "commit")
    {
        task = WFTaskFactory::create_go_task("commit_task", &GrpcImpl::commit_task, this, request, response, ctx);
    }
    else if ( operation == "rollback")
    {
        task = WFTaskFactory::create_go_task("rollback_task", &GrpcImpl::rollback_task, this, request, response, ctx);
    }
    else if ( operation == "checkpoint")
    {
        task = WFTaskFactory::create_go_task("checkpoint_task", &GrpcImpl::checkpoint_task, this, request, response, ctx);
    }
    else if ( operation == "testConnect")
    {
        task = WFTaskFactory::create_go_task("test_connect_task", &GrpcImpl::test_connect_task, this, request, response, ctx);
    }
    else if ( operation == "getCoreVersion")
    {
        task = WFTaskFactory::create_go_task("core_version_task", &GrpcImpl::core_version_task, this, request, response, ctx);
    }
    else if ( operation == "batchInsert")
    {
        task = WFTaskFactory::create_go_task("batch_insert_task", &GrpcImpl::batch_insert_task, this, request, response, ctx);
    }
    else if ( operation == "batchRemove")
    {
        task = WFTaskFactory::create_go_task("batch_remove_task", &GrpcImpl::batch_remove_task, this, request, response, ctx);
    }
    else if ( operation == "usermanage")
    {
        task = WFTaskFactory::create_go_task("user_manage_task", &GrpcImpl::user_manage_task, this, request, response, ctx);
    }
    else if ( operation == "showuser")
    {
        task = WFTaskFactory::create_go_task("user_show_task", &GrpcImpl::user_show_task, this, request, response, ctx);
    }
    else if ( operation == "userprivilegemanage")
    {
        task = WFTaskFactory::create_go_task("user_privilege_task", &GrpcImpl::user_privilege_task, this, request, response, ctx);
    }
    else if ( operation == "userpassword")
    {
        task = WFTaskFactory::create_go_task("user_password_task", &GrpcImpl::user_password_task, this, request, response, ctx);
    }
    else if ( operation == "txnlog")
    {
        task = WFTaskFactory::create_go_task("txn_log_task", &GrpcImpl::txn_log_task, this, request, response, ctx);
    }
    else if ( operation == "querylog")
    {
        task = WFTaskFactory::create_go_task("query_log_task", &GrpcImpl::query_log_task, this, request, response, ctx);
    }
    else if ( operation == "accesslog")
    {
        task = WFTaskFactory::create_go_task("access_log_task", &GrpcImpl::access_log_task, this, request, response, ctx);
    }
    else if ( operation == "ipmanage")
    {
        task = WFTaskFactory::create_go_task("ip_manage_task", &GrpcImpl::ip_manage_task, this, request, response, ctx);
    }
    else if (operation == "funquery")
    {
        task = WFTaskFactory::create_go_task("fun_query_task", &GrpcImpl::fun_query_task, this, request, response, ctx);
    }
    else if (operation == "funcudb")
    {
        task = WFTaskFactory::create_go_task("fun_cudb_task", &GrpcImpl::fun_cudb_task, this, request, response, ctx);
    }
    else if (operation == "funreview")
    {
        task = WFTaskFactory::create_go_task("fun_review_task", &GrpcImpl::fun_review_task, this, request, response, ctx);
    }
    else
    {
        task = WFTaskFactory::create_go_task("default_task", &GrpcImpl::default_task, this, request, response, ctx);
    }
    task->set_callback([&](WFGoTask *task) {
        string resJson = response->ShortDebugString();
        if (response->statuscode() == 0)
        {
            Util::formatPrint("response result:\n" + resJson);
        }
        else
        {
            Util::formatPrint("response result:\n" + resJson, "ERROR");
        }
        apiUtil->write_access_log(operation, remoteIP, response->statuscode(), response->statusmsg());
    });
    ctx->get_series()->push_back(task);
}

void GrpcImpl::shutdown(CommonRequest *request, CommonResponse *response, srpc::RPCContext *ctx)
{
    string remoteIP = ctx->get_remote_ip();
    string operation = "shutdown";
    std::cout << "------------------------ grpc-api ------------------------" << std::endl;
    std::cout << "seq_id: " << ctx->get_seqid() << std::endl;
    std::cout << "remote_ip: " <<  remoteIP << std::endl;
    std::cout << "operation: " << operation << std::endl;
    std::cout << "request_time: " << Util::get_date_time() << std::endl;
    std::cout << "post content:" << request->ShortDebugString() << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
    shutdown_task(request, response, ctx);
}

void GrpcImpl::build_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(check_result.empty() == false)
        {
            response->set_statuscode(1101);
            response->set_statusmsg(check_result);
            return;
        }
        check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string db_path = request->db_path();
        if (db_path == apiUtil->get_system_path())
        {
            response->set_statuscode(1002);
            response->set_statusmsg("You have no rights to access system files");
            return;
        }
        string result = apiUtil->check_param_value("db_name", request->db_name());
        if (result.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(result);
            return;
        }
        //check if database named [db_name] is already built
        if(apiUtil->check_db_exist(request->db_name()))
        {
            response->set_statuscode(1004);
            response->set_statusmsg("database already built.");
            return;
        }
        string dataset = request->db_path();
        string database = request->db_name();
        cout << "Import dataset to build database..." << endl;
        cout << "DB_store: " << database << "\tRDF_data: " << dataset << endl;
        int len = database.length();
        Database* current_database = new Database(database);
        bool flag = current_database->build(dataset);
        delete current_database;
        current_database = NULL;
        if (!flag)
        {
            string error = "Import RDF file to database failed.";
            string cmd = "rm -r " + database + ".db";
            system(cmd.c_str());
            response->set_statuscode(1005);
            response->set_statusmsg(error);
            return;
        }

        ofstream f;
        f.open("./" + database + ".db/success.txt");
        f.close();
        //by default, one can query or load or unload the database that is built by itself, so add the database name to the privilege set of the user
        if (apiUtil->add_privilege(request->username(), "query", request->db_name()) == 0 || 
        apiUtil->add_privilege(request->username(), "load", request->db_name()) == 0 || 
        apiUtil->add_privilege(request->username(), "unload", request->db_name()) == 0 || 
        apiUtil->add_privilege(request->username(), "backup", request->db_name()) == 0 || 
        apiUtil->add_privilege(request->username(), "restore", request->db_name()) == 0 || 
        apiUtil->add_privilege(request->username(), "export", request->db_name()) == 0)
        {
            string error = "add query or load or unload or backup or restore or export privilege failed.";
            Util::formatPrint(error, "ERROR");
            response->set_statuscode(1006);
            response->set_statusmsg(error);
            return;
        }
        Util::formatPrint("add query and load and unload and backup and restore privilege succeed after build.");

        //add database information to system.db
        if(apiUtil->build_db_user_privilege(request->db_name() ,request->username()))
        {
            string success = "Import RDF file to database done.";
            response->set_statuscode(0);
            response->set_statusmsg(success);
            Util::add_backuplog(request->db_name());
        }
        else 
        {
            string error = "add database information to system failed.";
            Util::formatPrint(error, "ERROR");
            response->set_statuscode(1006);
            response->set_statusmsg(error);
        }
    }
    catch (std::exception &e)
    {
        string content = "Build failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::load_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string result = apiUtil->check_param_value("db_name", request->db_name());
        if (result.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(result);
            return;
        }
        string db_name = request->db_name();
        // check database build
        if (!apiUtil->check_already_build(db_name))
        {
            throw runtime_error("the database [" + db_name + "] not build yet.");
        }
        // check database load status
        Database *current_database = apiUtil->get_database(db_name);
        if (current_database == NULL)
        {
            if (!apiUtil->trywrlock_database(db_name))
            {
                throw runtime_error("Unable to load due to loss of lock.");
            }
            Database *current_database = new Database(db_name);
            cout << "begin loading..." << endl;
            bool load_csr = false;
            if (request->has_load_csr() && request->load_csr() == "true")
            {
                load_csr = true;
            }
            bool rt = current_database->load(load_csr);
            cout << "end loading..." << endl;
            
            if (rt)
            {
                apiUtil->add_database(db_name, current_database);
                //todo insert txn
                if(apiUtil->insert_txn_managers(current_database, request->db_name()) == false)
                {
                    cout<<"when load insert_txn_managers fail"<<endl;
                }
                response->set_statuscode(0);
                response->set_statusmsg("Database loaded successfully.");
                if (current_database->csr != NULL)
                {
                    response->set_csr("true");
                }
                else
                {
                    response->set_csr("false");
                }
            }
            else
            {
                response->set_statuscode(1005);
                response->set_statusmsg("load failed: unknow error.");
            }
            // rt = apiUtil->unlock_database_map();
            // cout << "unlock database map: " << rt << endl;
            rt = apiUtil->unlock_database(db_name);
            cout << "unlock database: " << rt << endl;
        }
        else
        {
            response->set_statuscode(0);
            response->set_statusmsg("the database already load yet.");
            if (current_database->csr != NULL)
            {
                response->set_csr("true");
            }
            else
            {
                response->set_csr("false");
            }
        }
    }
    catch (std::exception &e)
    {
        string content = "Load failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::check_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string success="the grpc server is running...";
        response->set_statuscode(0);
        response->set_statusmsg(success);
    }
    catch (std::exception &e)
    {
        string content = "Check failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::monitor_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string db_name = request->db_name();
        string error = apiUtil->check_param_value("db_name", db_name);
        if( error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        if(apiUtil->check_db_exist(db_name) == false)
        {
            error = "the database [" + db_name + "] not built yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        Database *current_database = apiUtil->get_database(db_name);
		if(current_database == NULL)
        {
            error = "Database not load yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        DatabaseInfo *database_info = apiUtil->get_databaseinfo(db_name);
		if(apiUtil->tryrdlock_databaseinfo(database_info) == false)
        {
            error = "Unable to monitor due to loss of lock";
            response->set_statuscode(1007);
            response->set_statusmsg(error);
            return;
        }
        string creator = database_info->getCreator();
		string time = database_info->getTime();
		apiUtil->unlock_databaseinfo(database_info);
        response->set_statuscode(0);
        response->set_statusmsg("success");
        response->set_creator(creator);
        response->set_database(current_database->getName());
        response->set_builttime(time);
        char tripleNumString[128];
        sprintf(tripleNumString, "%lld", current_database->getTripleNum());
        response->set_triplenum(tripleNumString);
        response->set_entitynum(current_database->getEntityNum());
        response->set_literalnum(current_database->getLiteralNum());
        response->set_subjectnum(current_database->getSubNum());
        response->set_predicatenum(current_database->getPreNum());
        response->set_connectionnum(apiUtil->get_connection_num());
    }
    catch (std::exception &e)
    {
        string content = "Moniter failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::unload_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string db_name = request->db_name();
        string error = apiUtil->check_param_value("db_name", db_name);
        if( error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        if(apiUtil->check_db_exist(db_name) == false)
        {
            error = "the database [" + db_name + "] not built yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        if(apiUtil->check_already_load(db_name) == false)
        {
            error = "the database not load yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        //here lock the databaseinfo
        struct DatabaseInfo* db_info = apiUtil->get_databaseinfo(db_name);
        if (apiUtil->trywrlock_databaseinfo(db_info) == false)
        {
            error = "the operation can not been excuted due to loss of lock.";
            response->set_statuscode(1007);
            response->set_statusmsg(error);
            return;
        }
        else{
            //  apiUtil->trywrlock_database_map();//lock the databases_map_lock
            if(apiUtil->find_txn_managers(db_name) == false)
            {
                error = "transaction manager can not find the database";
                apiUtil->unlock_database_map();
                apiUtil->unlock_databaseinfo(db_info);
                response->set_statuscode(1008);
                response->set_statusmsg(error);
                return;
            }
            apiUtil->db_checkpoint(db_name);
            apiUtil->delete_from_databases(db_name);
            response->set_statuscode(0);
            string success = "Database ["+ db_name +"] unloaded.";
            response->set_statusmsg(success);
            // apiUtil->unlock_database_map();
            apiUtil->unlock_databaseinfo(db_info);
            return;
        }
    }
    catch (std::exception &e)
    {
        string content = "Unload failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::drop_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string db_name = request->db_name();
        string error = apiUtil->check_param_value("db_name", db_name);
        if( error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        if(apiUtil->check_already_build(db_name) == false)
        {
            error = "the database [" + db_name + "] not built yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        struct DatabaseInfo* db_info = apiUtil->get_databaseinfo(db_name);
        if(apiUtil->trywrlock_databaseinfo(db_info) == false)
        {
            error = "the operation can not been excuted due to loss of lock.";
            response->set_statuscode(1007);
            response->set_statusmsg(error);
            return;
        }
        else
        {
            //@ the database has loaded, unload it firstly
            if (apiUtil->check_already_load(db_name))
            {
                apiUtil->delete_from_databases(db_name);
                Util::formatPrint("remove " + db_name + " from loaded database list");
                
            }
            apiUtil->unlock_databaseinfo(db_info);
            apiUtil->delete_from_already_build(db_name);
            Util::formatPrint("remove " + db_name + " from the already build database list");
            //@ delete the database info from  the system database
            string update = "DELETE WHERE {<" + db_name + "> ?x ?y.}";
            apiUtil->update_sys_db(update);
            string cmd;
            string is_back = request->is_backup();
            if (is_back.empty())
            {
                is_back = "true";
            }
            if (is_back == "false")
            {
                cmd = "rm -r " + db_name + ".db";
            }
            else if (is_back == "true")
            {
                cmd = "mv " + db_name + ".db " + db_name + ".bak";
            }
            cout<<"delete the file: "<< cmd << endl;
            system(cmd.c_str());

            Util::delete_backuplog(db_name);
            string success = "Database " + db_name + " dropped.";
            response->set_statuscode(0);
            response->set_statusmsg(success);
        }
    }
    catch (std::exception &e)
    {
        string content = "Drop failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::show_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        vector<struct DatabaseInfo *> array;
        apiUtil->get_already_builds(username, array);
        size_t count = array.size();
        for (size_t i = 0; i < count; i++)
        {
            DatabaseInfo * db_ptr = array[i];
            DBInfo dbInfo;
            dbInfo.set_database(db_ptr->getName());
            dbInfo.set_creator(db_ptr->getCreator());
            dbInfo.set_builttime(db_ptr->getTime());
            dbInfo.set_status(db_ptr->getStatus());
            google::protobuf::Any *any = response->add_responsebody();
            any->PackFrom(dbInfo);
        }
        // set response status and message
        response->set_statuscode(0);
        response->set_statusmsg("Get the database list successfully!");
    }
    catch (std::exception &e)
    {
        string content = "Show failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::backup_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string db_name = request->db_name();
        string error = apiUtil->check_param_value("db_name", db_name);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
        }
        if(apiUtil->check_already_build(request->db_name()) == false)
        {
            error = "the database [" + request->db_name() + "] not built yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        struct DatabaseInfo *db_info = apiUtil->get_databaseinfo(request->db_name());
        if(apiUtil->trywrlock_databaseinfo(db_info) == false)
        {
            error = "the operation can not been excuted due to loss of lock.";
            response->set_statuscode(1007);
            response->set_statusmsg(error);
            return;
        }
        //begin backup database
        string path = request->backup_path();
        if(path == "") {
            path = apiUtil->get_backup_path();
        }
        if(path == "." ){
            string error = "Failed to backup the database. Backup Path Can not be root or empty.";
            apiUtil->unlock_databaseinfo(db_info);
            response->set_statuscode(1003);
            response->set_statusmsg(error);		
            return;
        }
        if(path[path.length() - 1] == '/') 
        {
            path = path.substr(0, path.length() - 1);
        }
        Util::formatPrint("backup path:" + path);
        string db_path = db_name + ".db";
        // APIUtil::trywrlock_database_map(); //lock the databases_map_lock
        apiUtil->rw_wrlock_database_map();
        int ret = apiUtil->db_copy(db_path, path);
        apiUtil->unlock_database_map();
        // APIUtil::unlock_database_map(); //unlock the databases_map_lock
        string timestamp="";
        if(ret == 1)
        {
            string error = "Failed to backup the database. Database Folder Misssing.";
            apiUtil->unlock_databaseinfo(db_info);
            response->set_statuscode(1005);
            response->set_statusmsg(error);
        }
        else
        {
            timestamp = Util::get_timestamp();
            cout << "db_path " << db_path << endl;
            // path = path.substr(0, path.length() - 1);
            path = path + "/" + db_path;
            string _path = path + "_" + timestamp;
            cout << "_path " << _path << endl;
            string sys_cmd = "mv " + path + " " + _path;
            system(sys_cmd.c_str());

            Util::formatPrint("database backup done: " + db_name);
            string success = "Database backup successfully.";
            apiUtil->unlock_databaseinfo(db_info);
            
            response->set_statuscode(0);
            response->set_statusmsg(success);
            response->set_backupfilepath(_path);
        }
    }
    catch (std::exception &e)
    {
        string content = "Backup failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::restore_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string db_name = request->db_name();
        string error = apiUtil->check_param_value("db_name", db_name);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        string path = request->backup_path();
        if(path[path.length() - 1] == '/') 
        {
            path = path.substr(0, path.length()-1);
        }
        Util::formatPrint("backup path: " + path);
        if(Util::dir_exist(path)==false){
            string error = "Backup path not exist, restore failed.";
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        string database = db_name;
        Util::formatPrint("restore database: " + database);
        //@ check database build?
        if( apiUtil->check_already_build(db_name) == false){
            string error = "Database not built yet. Rebuild Now";
            string time = Util::get_backup_time(path, db_name);
            if(time.size() == 0){
                string error = "Backup path does not match database name, restore failed";
                response->set_statuscode(1003);
                response->set_statusmsg(error);
                return;
            }
            if (apiUtil->add_privilege(username, "query", db_name) == 0 ||
                apiUtil->add_privilege(username, "load", db_name) == 0 ||
                apiUtil->add_privilege(username, "unload", db_name) == 0 ||
                apiUtil->add_privilege(username, "backup", db_name) == 0 ||
                apiUtil->add_privilege(username, "restore", db_name) == 0 ||
                apiUtil->add_privilege(username, "export", db_name) == 0)
            {
                string error = "add query or load or unload or backup or restore or export privilege failed.";
                response->set_statuscode(1006);
                response->set_statusmsg(error);
                return;
            }
            if(apiUtil->build_db_user_privilege(db_name, username))
            {
                Util::add_backuplog(db_name);
            }
            else
            {
                error = "Database not built yet. Rebuild failed.";
                response->set_statuscode(1005);
                response->set_statusmsg(error);
                return;
            }
        }
        struct DatabaseInfo *db_info = apiUtil->get_databaseinfo(db_name);
        if(apiUtil->trywrlock_databaseinfo(db_info) == false)
        {
            error = "Unable to restore due to loss of lock";
            response->set_statuscode(1007);
            response->set_statusmsg(error);
            return;
        }
        //restore
        string sys_cmd = "rm -rf " + db_name + ".db";
        system(sys_cmd.c_str());

        // TODO why need lock the database_map?
        // APIUtil::trywrlock_database_map();
        int ret = apiUtil->db_copy(path, apiUtil->get_Db_path());
        // APIUtil::unlock_database_map();

        if (ret == 1)
        {
            string error = "Failed to restore the database. Backup path error";
            apiUtil->unlock_databaseinfo(db_info);
            response->set_statuscode(1005);
            response->set_statusmsg(error);
        }
        else
        {
            //TODO update the in system.db
            path = Util::get_folder_name(path, db_name);
            sys_cmd = "mv " + path + " " + db_name + ".db";
            system(sys_cmd.c_str());
            apiUtil->unlock_databaseinfo(db_info);
            string success = "Database " + db_name + " restore successfully.";
            response->set_statuscode(0);
            response->set_statusmsg(success);
        }
    }
    catch (std::exception &e)
    {
        string content = "Restore failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::query_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string thread_id = Util::getThreadID();
        Database *current_database;
        string db_name = request->db_name();
        string sparql = request->sparql();
        string format = request->format();
        if (format.empty())
        {
            format="json";
        }
        try
        {
            // check db_name paramter
            string error="";
            error = apiUtil->check_param_value("db_name",db_name);
            if (error.empty() == false)
            {
                throw runtime_error(error);
            }
            // check sparql paramter
            error = apiUtil->check_param_value("sparql",sparql);
            if (error.empty() == false)
            {
                throw runtime_error(error);
            }
            // check database exist
            if(apiUtil->check_db_exist(db_name) == false)
            {
                throw runtime_error("Database not build yet.");
            }
            // check database load status
            current_database = apiUtil->get_database(db_name);
            if (current_database == NULL)
            {
                throw runtime_error("Database not load yet.");
            }
            bool lock_rt = apiUtil->rdlock_database(db_name);
            if (lock_rt)
            {
                Util::formatPrint("get current database read lock success: " + db_name);
            }
            else
            {
                throw runtime_error("get current database read lock fail.");
            }
            
        }
        catch (const std::exception &e)
        {
            string content = "query failed: " + string(e.what());
            response->set_statuscode(1005);
            response->set_statusmsg(content);
            return;
        }
        FILE *output = NULL;

        ResultSet rs;
        int ret_val;
        bool update_flag_bool = true;
        size_t query_time = Util::get_cur_time();

        // set query_start_time
        std::string query_start_time;
        struct timeval tv;
        gettimeofday(&tv, NULL);
        int s = tv.tv_usec / 1000;
        int y = tv.tv_usec % 1000;
        query_start_time = Util::get_date_time() + ":" + Util::int2string(s) + "ms" + ":" + Util::int2string(y) + "microseconds";
        try
        {
            Util::formatPrint("begin query...");
            rs.setUsername(username);
            ret_val = current_database->query(sparql, rs, output, update_flag_bool, false, nullptr);
            query_time = Util::get_cur_time() - query_time;
        }
        catch (const std::string &exception_msg)
        {
            apiUtil->unlock_database(db_name);
            response->set_statuscode(1005);
            response->set_statusmsg(exception_msg);
            return;
        }
        catch (const std::runtime_error &e1)
        {
            string content = e1.what();
            apiUtil->unlock_database(db_name);
            response->set_statuscode(1005);
            response->set_statusmsg(content);
            return;
        }
        catch (...)
        {
            string content = "unknow error";
            Util::formatPrint("query failed:unknow error", "ERROR");
            apiUtil->unlock_database(db_name);
            response->set_statuscode(1005);
            response->set_statusmsg("query failed: " + content);
            return;
        }
        bool ret = false, update = false;
        if (ret_val < -1) // non-update query
        {
            ret = (ret_val == -100);
        }
        else // update query, -1 for error, non-negative for num of triples updated
        {
            update = true;
        }
        if (Util::dir_exist("./query_result") == false)
        {
            Util::create_dir("./query_result");
        }
        string filename = thread_id+"_"+Util::getTimeString2()+"_"+Util::int2string(Util::getRandNum())+".txt";
        string localname = "./query_result/" + filename;
        if (ret)
        {
            cout << thread_id << ": search query returned successfully." << endl;
            long rs_ansNum = max((long)rs.ansNum - rs.output_offset, 0L);
			long rs_outputlimit = (long)rs.output_limit;
			if (rs_outputlimit != -1)
				rs_ansNum = min(rs_ansNum, rs_outputlimit);
            if (rs_ansNum > apiUtil->get_max_output_size())
            {
                if (rs_outputlimit == -1 || rs_outputlimit > apiUtil->get_max_output_size())
                {
                    rs_outputlimit = apiUtil->get_max_output_size();
                }
            }
            std::string query_time_s = Util::int2string(query_time);
            ofstream outfile;
            std::string json_result = rs.to_JSON();
            if (format == "json")
            {
                parse_query_result(request, response, rs);
            }
            else if (format == "file")
            {
                outfile.open(localname);
                outfile << json_result;
                outfile.close();
                response->set_filename(filename);
            }
            else if (format == "json+file" || format == "file+json")
            {
                parse_query_result(request, response, rs);
                outfile.open(localname);
                outfile << json_result;
                outfile.close();
                response->set_filename(filename);
            }
            else if (format == "sparql-results+json")
            {
                response->set_result(json_result);
            }
            else
            {
                apiUtil->unlock_database(db_name);
                Util::formatPrint("query fail: unkown result format.", "ERROR");
                response->set_statuscode(1005);
                response->set_statusmsg("Unkown result format.");
                return;
            }
            response->set_statuscode(0);
            response->set_statusmsg("success");
            response->set_ansnum(rs_ansNum);
            response->set_outputlimit(rs_outputlimit);
            response->set_querytime(query_time_s);
            response->set_threadid(thread_id);
            
            // record each query operation, including the sparql and the answer number
            // accurate down to microseconds
            std::string remote_ip = ctx->get_remote_ip();
            if (remote_ip != TEST_IP)
            {
                int status_code = 0;
                string file_name = "";
                if (format.find("file") != string::npos)
                {
                    file_name = string(filename.c_str());
                }
                struct DBQueryLogInfo queryLogInfo(query_start_time, remote_ip, sparql, rs_ansNum, format, file_name, status_code, query_time);
                apiUtil->write_query_log(&queryLogInfo);
            }
        }
        else
        {
            string status_msg;
            int status_code;
            if (update)
            {
                status_code = 0;
                status_msg = "update query returns true.";
                Util::formatPrint(status_msg);
            }
            else
            {
                status_code = 1005;
                status_msg = "search query returns false.";
                std::cout << status_msg << endl;
                Util::formatPrint(status_msg, "WARN");
            }
            response->set_statuscode(status_code);
            response->set_statusmsg(status_msg);
        }
        apiUtil->unlock_database(db_name);
        Util::formatPrint("query complete!");
    }
    catch (std::exception &e)
    {
        string content = "Query failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::export_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string db_name = request->db_name();
        string db_path = request->db_path();
        string error = apiUtil->check_param_value("db_name", db_name);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
        }
        error = apiUtil->check_param_value("db_path", db_path);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
        }
        if(apiUtil->check_already_build(db_name) == false)
        {
            error = "the database not built yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        if(db_path[db_path.length()-1] != '/')
        {
            db_path = db_path + "/";
        }
        if(Util::dir_exist(db_path) == false)
        {
            Util::create_dir(db_path);
        }	
        db_path = db_path + db_name +"_" + Util::get_timestamp() + ".nt";
        //check if database named [db_name] is already load
        Database *current_database = apiUtil->get_database(db_name);
        if(current_database == NULL)
        {
            error = "Database not load yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        apiUtil->rdlock_database(db_name);//lock database
        Util::formatPrint("export_path: " + db_path);
        FILE* ofp = fopen(db_path.c_str(), "w");
        current_database->export_db(ofp);
        fflush(ofp);
        fclose(ofp);
        ofp = NULL;
        current_database = NULL;
        apiUtil->unlock_database(db_name);//unlock database
        string success = "Export the database successfully.";
        
        response->set_statuscode(0);
        response->set_statusmsg(success);
        response->set_filepath(db_path);
    }
    catch (std::exception &e)
    {
        string content = "Export failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::login_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if (ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string success = "login successfully.";

        response->set_statuscode(0);
        response->set_statusmsg(success);
        string version = Util::getConfigureValue("version");
        response->set_coreversion(version);
        string licensetype = Util::getConfigureValue("licensetype");
        response->set_licensetype(licensetype);
        string cur_path = Util::get_cur_path();
        response->set_rootpath(cur_path);
        response->set_type(HTTP_TYPE);
    }
    catch (std::exception &e)
    {
        string content = "Login failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::begin_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string db_name = request->db_name();
        string error = apiUtil->check_param_value("db_name", db_name);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
        }
        string isolevel = request->isolevel();
        error = apiUtil->check_param_value("isolevel", isolevel);
        int level=Util::string2int(isolevel);
        if(level<=0||level>3)
        {
            error="the Isolation level's value only can been 1/2/3";
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        if(apiUtil->check_already_build(db_name) == false)
        {
            error = "Database not built yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        if(apiUtil->check_already_load(db_name) == false)
        {
            error = "Database not load yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        if(apiUtil->get_Txn_ptr(db_name) == NULL)
        {
            error = "Database transaction manager error.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        string result_tid = apiUtil->begin_process(db_name, level, request->username());
        if( result_tid.empty())
        {
            error = "transaction begin failed.";
            response->set_statuscode(1005);
            response->set_statusmsg(error);
            return;
        }
        
        response->set_statuscode(0);
        response->set_statusmsg("transaction begin success");
        response->set_tid(result_tid);
    }
    catch (std::exception &e) 
    {
        string content = "Transaction begin failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::tquery_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string db_name = request->db_name();
        string sparql = request->sparql();
        string error = apiUtil->check_param_value("db_name", db_name);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
        }
        error = apiUtil->check_param_value("TID", request->tid());
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
        }
        error = apiUtil->check_param_value("sparql", sparql);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
        }
        auto TID = apiUtil->check_txn_id(request->tid());
        if(TID == NULL)
        {
            error = "TID is not a pure number. TID: " + request->tid();
            response->set_statuscode(1003);
            response->set_statusmsg(error);
        }
        if(apiUtil->check_already_load(db_name) == false)
        {
            error = "Database not load yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        auto txn_m = apiUtil->get_Txn_ptr(db_name);
        if( txn_m == NULL)
        {
            error = "Database transaction manager error.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        Util::formatPrint("tquery sparql: " + sparql);
        string res;
        int ret = txn_m->Query(TID, sparql ,res);
        if(ret == -1)
        {
            error = "Transaction query failed due to wrong TID";
            response->set_statuscode(1005);
            response->set_statusmsg(error);
        }
        else if(ret == -10)
        {
            error = "Transaction query failed due to wrong database status";
            apiUtil->rollback_process(txn_m, TID);
            response->set_statuscode(1005);
            response->set_statusmsg(error);
        }
        else if(ret == -99)
        {
            error = "Transaction query failed. This transaction is not in running status!";
            apiUtil->rollback_process(txn_m, TID);
            response->set_statuscode(1005);
            response->set_statusmsg(error);
        }
        else if(ret == -100)
        {
            
            Document resDoc;
            cout<<"res:"<<res<<endl;
            resDoc.Parse(res.c_str());
            if(resDoc.HasParseError())
            {
                response->set_statuscode(0);
                response->set_statusmsg("success");
                response->set_result(res);
            }
            else
            {
                QueryHeadInfo *head = new QueryHeadInfo();
                QueryResultInfo *results = new QueryResultInfo();
                if (resDoc.HasMember("head") && resDoc["head"].IsObject())
                {
                    Value& resHead = resDoc["head"];
                    if (resHead.HasMember("vars") && resHead["vars"].IsArray())
                    {
                        Value& vars = resHead["vars"];
                        for(int i = 0; i<vars.Size(); i++)
                        {
                            head->add_vars(vars[i].GetString());
                        }
                    }
                    if (resHead.HasMember("link") && resHead["link"].IsArray())
                    {
                        Value& link = resHead["link"];
                        for (size_t i = 0; i < link.Size(); i++)
                        {
                            head->add_link(link[i].GetString());
                        }
                    }
                }
                if (resDoc.HasMember("results") && resDoc["results"].IsObject()
                    && resDoc["results"].HasMember("bindings"))
                {
                    Value& bindings = resDoc["results"]["bindings"];
                    if (bindings.IsArray())
                    {
                        for(int i = 0; i<bindings.Size(); i++)
                        {
                            Value& obj = bindings[i];
                            if (obj.IsObject())
                            {
                                StringBuffer resBuffer;
                                rapidjson::Writer<rapidjson::StringBuffer> resWriter(resBuffer);
                                obj.Accept(resWriter);
                                string binding_str = resBuffer.GetString();
                                cout << "bindings["<< i << "]=" << binding_str << endl;
                                results->add_bindings(binding_str);
                            } 
                            else if (obj.IsString())
                            {
                                results->add_bindings(obj.GetString());
                            }
                            else
                            {
                                cout << "bindings["<< i << "] unknown type" << endl;
                            }
                        }
                    } 
                    else if (bindings.IsObject())
                    {
                        StringBuffer resBuffer;
                        rapidjson::Writer<rapidjson::StringBuffer> resWriter(resBuffer);
                        bindings.Accept(resWriter);
                        results->add_bindings(resBuffer.GetString());
                    }
                    else if (bindings.IsString())
                    {
                        results->add_bindings(bindings.GetString());
                    }
                    else 
                    {
                        Util::formatPrint("Unknown bindings type", "ERROR");
                    }
                }
                response->set_statuscode(0);
                response->set_statusmsg("success");
                response->set_allocated_head(head);
                response->set_allocated_results(results);
            }
        }
        else if(ret == -20)
        {
            error = "Transaction query failed. This transaction is set abort due to conflict!";
            apiUtil->rollback_process(txn_m, TID);
            response->set_statuscode(1005);
            response->set_statusmsg(error);
        }
        else if(ret == -101)
        {
            error = "Transaction query failed. Unknown query error";
            response->set_statuscode(1005);
            response->set_statusmsg(error);
        }
        else
        {
            string success = "Transaction query success, update num: " + Util::int2string(ret);
            response->set_statuscode(0);
            response->set_statusmsg(success);
        }
    }
    catch (std::exception &e) 
    {
        string content = "Transaction query failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::commit_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string db_name = request->db_name();
        string error = apiUtil->check_param_value("db_name", db_name);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        error = apiUtil->check_param_value("TID", request->tid());
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        string TID_s = request->tid();
        auto TID = apiUtil->check_txn_id(TID_s);
        if( TID == NULL)
        {
            error = "TID is not a pure number. TID: " + TID_s;
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        Database *current_database = apiUtil->get_database(db_name);
		if(current_database == NULL)
        {
            error = "Database not built yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        if(apiUtil->check_already_load(db_name) == false)
        {
            error = "Database not load yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        auto txn_m = apiUtil->get_Txn_ptr(db_name);
        if(txn_m == NULL)
        {
            error = "Database transaction manager error.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        int ret = txn_m->Commit(TID);
        if (ret == -1)
        {
            error = "transaction not found, commit failed. TID: " + TID_s;
            response->set_statuscode(1005);
            response->set_statusmsg(error);
        }
        else if (ret == 1)
        {
            error = "transaction not in running state! commit failed. TID: " + TID_s;
            apiUtil->rollback_process(txn_m, TID);
            response->set_statuscode(1005);
            response->set_statusmsg(error);
        }
        else
        {
            apiUtil->commit_process(txn_m, TID);
            auto latest_tid = txn_m->find_latest_txn();
			cout << "latest TID: " << latest_tid <<  endl;
			if (latest_tid == 0)
			{
				Util::formatPrint("this is latest TID, auto checkpoint and save.");
				txn_m->Checkpoint();
                Util::formatPrint("transaction checkpoint done.");
				if (apiUtil->trywrlock_database(db_name))
				{
					current_database->save();
					apiUtil->unlock_database(db_name);
				}
				else
				{
					Util::formatPrint("the save operation can not been excuted due to loss of lock.", "ERROR");
				}
			}
            string success = "transaction commit success. TID: " + TID_s;
            response->set_statuscode(0);
            response->set_statusmsg(success);
        }
    }
    catch (std::exception &e) 
    {
        string content = "Commit failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::rollback_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string db_name = request->db_name();
        string error = apiUtil->check_param_value("db_name", db_name);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
        }
        error = apiUtil->check_param_value("TID", request->tid());
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
        }
        string TID_s = request->tid();
        auto TID = apiUtil->check_txn_id(TID_s);
        if(TID == NULL)
        {
            error = "TID is not a pure number. TID: " + TID_s;
            response->set_statuscode(1003);
            response->set_statusmsg(error);
        }
        if(apiUtil->check_db_exist(request->db_name()) == false)
        {
            error = "the database [" + request->db_name() + "] not built yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        if(!apiUtil->check_already_load(request->db_name()))
        {
            error = "Database not load yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        auto txn_m = apiUtil->get_Txn_ptr(db_name);
        if(txn_m == NULL)
        {
            error = "Database transaction manager error.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        int ret = txn_m->Rollback(TID);
        if (ret == 1)
        {
            error = "transaction not in running state! rollback failed. TID: " + TID_s;
            response->set_statuscode(1005);
            response->set_statusmsg(error);
            return;
        }
        else if (ret == -1)
        {
            error = "transaction not found, rollback failed. TID: " + TID_s;
            response->set_statuscode(1005);
            response->set_statusmsg(error);
            return;
        }
        else
        {
            apiUtil->rollback_process(txn_m, TID);
            string success = "transaction rollback success. TID: " + TID_s;
            response->set_statuscode(0);
            response->set_statusmsg(success);
            return;
        }
    }
    catch (std::exception &e) 
    {
        string content = "Rollback failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::checkpoint_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string db_name = request->db_name();
        string error = apiUtil->check_param_value("db_name", db_name);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        if(apiUtil->check_db_exist(db_name) == false)
        {
            error = "Database not built yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        if(apiUtil->check_already_load(db_name) == false)
        {
            error = "Database not load yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        Database *current_database = apiUtil->get_database(db_name);
        if (apiUtil->trywrlock_database(db_name) == false)
        {
            error = "the operation can not been excuted due to loss of lock.";
            response->set_statuscode(1007);
            response->set_statusmsg(error);
        }
        else
        {
            Util::formatPrint("get Txn_ptr");
            auto txn_m = apiUtil->get_Txn_ptr(db_name);
            if(txn_m == NULL)
            {
                error = "Database transaction manager error.";
                apiUtil->unlock_database(db_name);
                response->set_statuscode(1004);
                response->set_statusmsg(error);
            }
            else
            {
                Util::formatPrint("begin checkpoint");
                txn_m->Checkpoint();
                Util::formatPrint("begin save");
                current_database->save();
                apiUtil->unlock_database(db_name);
                response->set_statuscode(0);
                response->set_statusmsg("Database saved successfully.");
            }
        }
    }
    catch (std::exception &e) 
    {
        string content = "Checkpoint failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::test_connect_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        response->set_statuscode(0);
        response->set_statusmsg("success");
        string version = Util::getConfigureValue("version");
        response->set_coreversion(version);
        string licensetype=Util::getConfigureValue("licensetype");
        response->set_licensetype(licensetype);
        response->set_type(HTTP_TYPE);
    }
    catch (std::exception &e) 
    {
        string content = "Test connect failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::core_version_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        response->set_statuscode(0);
        response->set_statusmsg("success");
        string version = Util::getConfigureValue("version");
        response->set_coreversion(version);
        response->set_type(HTTP_TYPE);
    }
    catch (std::exception &e) 
    {
        string content = "Get CoreVersion failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
    
}

void GrpcImpl::batch_insert_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string db_name = request->db_name();
        string file = request->file();
        string error = apiUtil->check_param_value("db_name", db_name);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        error = apiUtil->check_param_value("file", file);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        if(Util::file_exist(file)==false)
        {
        error="The data file is not exist";
        response->set_statuscode(1003);
        response->set_statusmsg(error);
        return;
        }
        if(apiUtil->check_db_exist(db_name) == false)
        {
            error = "Database not built yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        if(apiUtil->check_already_load(db_name) == false)
        {
            error = "Database not load yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        Database *current_database = apiUtil->get_database(db_name);
        if (apiUtil->trywrlock_database(db_name) == false)
        {
            error = "the operation can not been excuted due to loss of lock.";
            response->set_statuscode(1007);
            response->set_statusmsg(error);
        }
        else
        {
            unsigned success_num = current_database->batch_insert(file, false, nullptr);
            apiUtil->unlock_database(db_name);
            response->set_statuscode(0);
            response->set_statusmsg("Batch insert data successfully.");
            response->set_successnum(Util::int2string(success_num));
            apiUtil->write_access_log("batchInsert",ctx->get_remote_ip(),0,"Batch insert data successfully.");
            return;
        }
    }
    catch (std::exception &e) 
    {
        string content = "batchInsert failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::batch_remove_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string db_name = request->db_name();
        string file = request->file();
        string error = apiUtil->check_param_value("db_name", db_name);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        error = apiUtil->check_param_value("file", file);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        if(Util::file_exist(file) == false)
        {
        error="the data file is not exist";
        response->set_statuscode(1003);
        response->set_statusmsg(error);
        return;
        }
        if(apiUtil->check_db_exist(db_name) == false)
        {
            error = "Database not built yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        if(apiUtil->check_already_load(db_name) == false)
        {
            error = "Database not load yet.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        Database *current_database = apiUtil->get_database(db_name);
        if (apiUtil->trywrlock_database(db_name) == false)
        {
            error = "the operation can not been excuted due to loss of lock.";
            response->set_statuscode(1007);
            response->set_statusmsg(error);
        }
        else
        {
            unsigned success_num = current_database->batch_remove(file, false, nullptr);
            apiUtil->unlock_database(db_name);
            response->set_statuscode(0);
            response->set_statusmsg("Batch remove data successfully.");
            response->set_successnum(Util::int2string(success_num));
            apiUtil->write_access_log("batchRemove",ctx->get_remote_ip(),0,"Batch remove data successfully.");
        }
    }
    catch (std::exception &e) 
    {
        string content = "batchRemove failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::shutdown_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string remote_ip = ctx->get_remote_ip();
    string ipCheckResult = apiUtil->check_access_ip(remote_ip);
    if(!ipCheckResult.empty())
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
        return;
    }
    string username = request->username();
    string password = request->password();
    if (username != apiUtil->get_system_username())
	{
		string msg = "You have no rights to stop the server.";
        response->set_statuscode(1101);
        response->set_statusmsg(msg);
		return;
	}
    string checkidentityresult = apiUtil->check_server_indentity(password);
	if (checkidentityresult.empty() == false)
	{
        response->set_statuscode(1001);
        response->set_statusmsg(checkidentityresult);
		return;
	}
    bool flag = apiUtil->db_checkpoint_all();
    if (flag)
    {
        string msg = "Server stopped successfully.";
        response->set_statuscode(0);
        response->set_statusmsg(msg);
        apiUtil->write_access_log("shutdown", remote_ip, 0, msg);
        Util::formatPrint(msg);
        delete apiUtil;
        _exit(1);
    }
    else
    {
        string msg = "Server stopped failed.";
        response->set_statuscode(1005);
        response->set_statusmsg(msg);
        apiUtil->write_access_log("shutdown", remote_ip, 1005, msg);
    }
}

void GrpcImpl::user_manage_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(!ipCheckResult.empty())
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string type = request->type();
        string msg = "";
        string op_username = request->op_username();
        string op_password = request->op_password();
        if(type == "1")//insert user
        {
            if(op_username.empty() || op_password.empty())
            {
                msg = "the user name and password can not be empty while adding user.";
                response->set_statuscode(1003);
                response->set_statusmsg(msg);
            }
            else if(apiUtil->user_add(op_username, op_password))
            {
                msg = "user add done.";
                response->set_statuscode(0);
                response->set_statusmsg(msg);
            }
            else
            {
                msg = "username already existed, add user failed.";
                response->set_statuscode(1004);
                response->set_statusmsg(msg);
            }
        }
        else if(type == "2") //delete user
        {
            if(op_username == apiUtil->get_root_username())
            {
                msg = "you cannot delete root, delete user failed.";
                response->set_statuscode(1004);
                response->set_statusmsg(msg);
            }
            else if(apiUtil->user_delete(op_username, op_password))
            {
                response->set_statuscode(0);
                response->set_statusmsg("delete user done.");
            }
            else
            {
                response->set_statuscode(1004);
                response->set_statusmsg("username not exist, delete user failed.");
            }
        }
        else if (type =="3")//alert password
        {
            if(apiUtil->user_pwd_alert(op_username, op_password))
            {
                response->set_statuscode(0);
                response->set_statusmsg("change password done.");
            }
            else
            {
                response->set_statuscode(1004);
                response->set_statusmsg("username not exist, change password failed.");
            }
        }
        else
        {
            response->set_statuscode(1003);
            response->set_statusmsg("the operation is not support.");
        }
    }
    catch (std::exception &e) 
    {
        string content = "usermanage failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::user_show_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        vector<struct DBUserInfo *> userList;
		apiUtil->get_user_info(&userList);
		if (userList.empty())
		{
			response->set_statuscode(0);
            response->set_statusmsg("No Users");
            return;
		}
        size_t count = userList.size();
        for (size_t i = 0; i < count; i++)
        {
            DBUserInfo * user_info = userList[i];
            UserInfo userInfo;
            userInfo.set_username(user_info->getUsernname());
            userInfo.set_password(user_info->getPassword());
            userInfo.set_queryprivilege(user_info->getQuery());
            userInfo.set_updateprivilege(user_info->getUpdate());
            userInfo.set_loadprivilege(user_info->getLoad());
            userInfo.set_unloadprivilege(user_info->getUnload());
            userInfo.set_backupprivilege(user_info->getBackup());
            userInfo.set_restoreprivilege(user_info->getRestore());
            userInfo.set_exportprivilege(user_info->getExport());
            google::protobuf::Any *any = response->add_responsebody();
            any->PackFrom(userInfo);
        }
        // set response status and message
        response->set_statuscode(0);
        response->set_statusmsg("success");
    }
    catch  (std::exception &e) 
    {
        string content = "usershow failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::user_privilege_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string error ="";
        string type = request->type();
        string db_name = request->db_name();
        string op_username = request->op_username();
        string privilege = request->privileges();
        error = apiUtil->check_param_value("type", type);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        error = apiUtil->check_param_value("op_username", op_username);
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        if (type !="3")
        {
            error = apiUtil->check_param_value("db_name", db_name);
            if (error.empty() == false)
            {
                response->set_statuscode(1003);
                response->set_statusmsg(error);
                return;
            }
            error = apiUtil->check_param_value("privilege", privilege);
            if (error.empty() == false)
            {
                response->set_statuscode(1003);
                response->set_statusmsg(error);
                return;
            }
        }
        if (op_username == apiUtil->get_root_username())
        {
            error = "you can't add privilege to root user.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
            return;
        }
        string result = "";
        if (type == "3")
        {
            int resultint = apiUtil->clear_user_privilege(op_username);
            if(resultint == -1)
            {
                error="the username is not exists or the username is root.";
                response->set_statuscode(1004);
                response->set_statusmsg(error);
            }
            else
            {
                result="clear the all privileges for the user successfully!";
                response->set_statuscode(0);
                response->set_statusmsg(result);
            }
        }
        else
        {
            vector<string> privileges;
            if (privilege.substr(privilege.length() - 1, 1) != ",")
            {
                privilege = privilege + ",";
            }
            Util::split(privilege, ",", privileges);
            for (int i = 0; i < privileges.size(); i++)
            {
                string temp_privilege_int = privileges[i];
                string temp_privilege = "";
                if (temp_privilege_int.empty())
                {
                    continue;
                }
                if (temp_privilege_int == "1")
                {
                    temp_privilege = "query";
                }
                else if (temp_privilege_int == "2")
                {
                    temp_privilege = "load";
                }
                else if (temp_privilege_int == "3")
                {
                    temp_privilege = "unload";
                }
                else if (temp_privilege_int == "4")
                {
                    temp_privilege = "update";
                }
                else if (temp_privilege_int == "5")
                {
                    temp_privilege = "backup";
                }
                else if (temp_privilege_int == "6")
                {
                    temp_privilege = "restore";
                }
                else if (temp_privilege_int == "7")
                {
                    temp_privilege = "export";
                }
                if (temp_privilege.empty() == false)
                {
                    if (type == "1")
                    {
                        if (apiUtil->add_privilege(op_username, temp_privilege, db_name) == 0)
                        {
                            result = result + "add privilege " + temp_privilege + " failed. \r\n";
                        }
                        else
                        {
                            result = result + "add privilege " + temp_privilege + " successfully. \r\n";
                        }
                    }
                    else if (type == "2")
                    {
                        if (apiUtil->del_privilege(op_username, temp_privilege, db_name) == 0)
                        {
                            result = result + "delete privilege " + temp_privilege + " failed. \r\n";
                        }
                        else
                        {
                            result = result + "delete privilege " + temp_privilege + " successfully. \r\n";
                        }
                    }
                    else
                    {
                        result = "the operation type is not support.";
                        response->set_statuscode(1003);
                        response->set_statusmsg(result);
                        return;
                    }
                }
            }
            response->set_statuscode(0);
            response->set_statusmsg(result);
        }
    }
    catch  (std::exception &e) 
    {
        string content = "userprivilege fail:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::user_password_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(!ipCheckResult.empty())
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string op_password = request->op_password();
        
        if(apiUtil->user_pwd_alert(username, op_password))
        {
            response->set_statuscode(0);
            response->set_statusmsg("change password done.");
        }
        else
        {
            response->set_statuscode(1004);
            response->set_statusmsg("username not exist, change password failed.");
        }
    }
    catch (std::exception &e) 
    {
        string content = "change password failed:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::txn_log_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        int page_no = request->pageno();
        int page_size = request->pagesize();
        struct TransactionLogs transactionLogs;
        apiUtil->get_transactionlog(page_no, page_size, &transactionLogs);
        response->set_statuscode(0);
        response->set_statusmsg("Get transaction log success");
        response->set_totalsize(transactionLogs.getTotalSize());
        response->set_totalpage(transactionLogs.getTotalPage());
        response->set_pageno(request->pageno());
        response->set_pagesize(request->pagesize());
        vector<struct TransactionLogInfo> logList = transactionLogs.getTransactionLogInfoList();
        size_t count = logList.size();
        for (size_t i = 0; i < count; i++)
        {
            struct TransactionLogInfo item = logList[i];
            TxnLogInfo *logInfo = new TxnLogInfo();
            logInfo->set_dbname(item.getDbName());
            logInfo->set_tid(item.getTID());
            logInfo->set_user(item.getUser());
            logInfo->set_state(item.getState());
            logInfo->set_begintime(item.getBeginTime());
            logInfo->set_endtime(item.getEndTime());
            google::protobuf::Any *any = response->add_list();
            any->PackFrom(*logInfo);
        }
    }
    catch (std::exception &e) 
    {
        string content = "Get transaction log fail:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::query_log_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string date = request->date();
        int page_no = request->pageno();
        int page_size = request->pagesize();
        struct DBQueryLogs dbQueryLogs;
        apiUtil->get_query_log(date, page_no, page_size, &dbQueryLogs);
        response->set_statuscode(0);
        response->set_statusmsg("Get query log success");
        response->set_totalsize(dbQueryLogs.getTotalSize());
        response->set_totalpage(dbQueryLogs.getTotalPage());
        response->set_pageno(request->pageno());
        response->set_pagesize(request->pagesize());
        vector<struct DBQueryLogInfo> logList = dbQueryLogs.getQueryLogInfoList();
        size_t count = logList.size();
        for (size_t i = 0; i < count; i++)
        {
            struct DBQueryLogInfo item = logList[i];
            QueryLogInfo *queryLogInfo = new QueryLogInfo();
            queryLogInfo->set_querydatetime(item.getQueryDateTime());
            queryLogInfo->set_remoteip(item.getRemoteIP());
            queryLogInfo->set_sparql(item.getSparql());
            queryLogInfo->set_ansnum(item.getAnsNum());
            queryLogInfo->set_format(item.getFormat());
            queryLogInfo->set_filename(item.getFileName());
            queryLogInfo->set_querytime(item.getQueryTime());
            google::protobuf::Any *any = response->add_list();
            any->PackFrom(*queryLogInfo);
        }
    }
    catch (std::exception &e) 
    {
        string msg = "Get query log fail:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(msg);
    }
}

void GrpcImpl::access_log_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string date = request->date();
        int page_no = request->pageno();
        int page_size = request->pagesize();
        struct DBAccessLogs dbAccessLogs;
        apiUtil->get_access_log(date, page_no, page_size, &dbAccessLogs);

        response->set_statuscode(0);
        response->set_statusmsg("Get query log success");
        response->set_totalsize(dbAccessLogs.getTotalSize());
        response->set_totalpage(dbAccessLogs.getTotalPage());
        response->set_pageno(request->pageno());
        response->set_pagesize(request->pagesize());
        vector<struct DBAccessLogInfo> logList = dbAccessLogs.getAccessLogInfoList();
        size_t count = logList.size();
        for (size_t i = 0; i < count; i++)
        {
            DBAccessLogInfo item = logList[i];
            AccessLogInfo *accessLogInfo = new AccessLogInfo();
            accessLogInfo->set_ip(item.getIP());
            accessLogInfo->set_operation(item.getOperation());
            accessLogInfo->set_code(item.getCode());
            accessLogInfo->set_msg(item.getMsg());
            accessLogInfo->set_createtime(item.getCreateTime());
            google::protobuf::Any *any = response->add_list();
            any->PackFrom(*accessLogInfo);
        }
    }
    catch (std::exception &e) 
    { 
        string msg = "Get access log fail:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(msg);
    }
}

void GrpcImpl::ip_manage_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        if(request->type() == "1")
        {
            string IPtype = apiUtil->ip_enabled_type();
            if(IPtype == "3"){
                response->set_statuscode(1005);
                response->set_statusmsg("please configure ip_deny_path or ip_allow_path in the conf.ini file first.");
                return;
            }
            IPManageInfo* ipManageInfo = new IPManageInfo();
            ipManageInfo->set_iptype(IPtype);
            vector<string>ip_list = apiUtil->ip_list(IPtype);
            for(int i = 0 ; i<ip_list.size(); i++)
            {
                ipManageInfo->add_ips(ip_list[i]);
            }
            google::protobuf::Any *any = response->add_responsebody();
            any->PackFrom(*ipManageInfo);
            response->set_statuscode(0);
            response->set_statusmsg("success");
        }
        else if ( request->type() == "2")
        {
            string ips = "";
            ips = request->ips();
            if(ips.empty())
            {
                response->set_statuscode(1003);
                response->set_statusmsg("the ips can't be empty");
                return;
            }
            vector<string> ipVector;
            Util::split(ips,",", ipVector);
            if(request->ip_type() == "1" || request->ip_type() == "2")
            {
                if(apiUtil->ip_save(request->ip_type(),ipVector) == false)
                {
                    if(request->ip_type() == "1")
                    {
                        response->set_statuscode(1005);
                        response->set_statusmsg("ip_deny_path is not configured, please configure it in the conf.ini file first.");
                    }
                    else
                    {
                        response->set_statuscode(1005);
                        response->set_statusmsg("ip_allow_path is not configured, please configure it in the conf.ini file first.");
                    }
                }
                else
                {    
                    response->set_statuscode(0);
                    response->set_statusmsg("success");
                }
            }
            else
            {
                response->set_statuscode(1003);
                response->set_statusmsg("ip_type is invalid, please look up the api document.");
            }
        }
        else
        {
            response->set_statuscode(1003);
            response->set_statusmsg("type is invalid, please look up the api document.");
        }
    }
    catch (const std::exception &e)
    {
        string content = "ipmanage fail:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::parse_query_result(CommonRequest *&request, CommonResponse *&response, ResultSet &rs)
{
    QueryHeadInfo *head = new QueryHeadInfo();
    QueryResultInfo *results = new QueryResultInfo();

    for (int i = 0; i < rs.true_select_var_num; i++)
    {
        head->add_vars(rs.var_name[i].substr(1));
    }

    regex dataTypePattern("\\^\\^<(\\S*?)[^>]*>.*?|<.*? />");
    smatch matchResult;
    string::const_iterator iterStart;
    string::const_iterator iterEnd;
    for (long long i = rs.output_offset; i < rs.ansNum; i++)
    {
        if (rs.output_limit != -1 && i == rs.output_offset + rs.output_limit)
        {
            cout << "the size is out than the output_limit" << endl;
            break;
        }

        if (i >= rs.output_offset)
        {
            string *binding = results->add_bindings();
            // rapidjson::Document doc;
            // rapidjson::Document::AllocatorType &allocator = doc.GetAllocator(); 
            // doc.SetObject();
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            writer.StartObject();
            for (int j = 0; j < rs.true_select_var_num; j++)
            {
                string ans_key, ans_type, ans_str, data_type;
                ans_str = rs.answer[i][j];
                cout << "rs.answer[" << i << "][" << j << "]: " << ans_str << endl;
                if (ans_str.length() == 0)
                    continue;
                ans_key = rs.var_name[j].substr(1);
                if (ans_str[0] == '<')
                {
                    ans_type = "uri";
                    ans_str = ans_str.substr(1, ans_str.length() - 2);
                }
                else if (ans_str[0] == '"')
                {
                    if (ans_str.find("\"^^<") == string::npos)
                    {
                        // no has type string
                        ans_type = "literal";
                        ans_str = ans_str.substr(1, ans_str.rfind('"') - 1);
                    }
                    else
                    {
                        string data_type = "";
                        if (ans_str[ans_str.length() - 1] == '>')
                        {
                            ans_type = "typed-literal";
                            int pos = ans_str.find("\"^^<");
                            // string data_type = ans_str.substr(pos + 4, ans_str.length() - pos - 5);
                            iterStart = ans_str.begin();
                            iterEnd = ans_str.end();
                            while (regex_search(iterStart, iterEnd, matchResult, dataTypePattern))
                            {
                                data_type = matchResult[0];
                                break;
                            }
                            if (data_type.length() > 4)
                            {
                                // remove ^^<>
                                data_type = data_type.substr(3, data_type.length() - 4);
                                if (data_type.find("^^<") != string::npos)
                                {
                                    data_type = "http://www.w3.org/2001/XMLSchema#string-complete";
                                }
                            }
                            else
                            {
                                data_type = "http://www.w3.org/2001/XMLSchema#string-complete";
                            }
                            ans_str = ans_str.substr(0, pos + 1);
                        }
                        else
                        {
                            // the entity value is not complete
                            ans_type = "typed-literal";
                            int pos = ans_str.find("\"^^<");
                            data_type = "http://www.w3.org/2001/XMLSchema#string-not-complete";
                            ans_str = ans_str.substr(0, pos + 1);
                        }
                    }
                }
                else
                {
                    ans_type = "unknown";
                    ans_str = "the information is not complete!";
                }
                writer.Key(StringRef(ans_key.c_str()));
                writer.StartObject();
                writer.Key("type");
                writer.String(StringRef(ans_type.c_str()));
                writer.Key("value");
                writer.String(StringRef(ans_str.c_str()));
                if (!data_type.empty())
                {
                    writer.Key("datatype");
                    writer.String(StringRef(data_type.c_str()));
                }
                writer.EndObject();
                
                // rapidjson::Document item;
                // item.SetObject();
                // item.AddMember("type", StringRef(ans_type.c_str()), allocator);
                // item.AddMember("value", StringRef(ans_str.c_str()), allocator);
                // if (!data_type.empty())
                // {
                //     item.AddMember("datatype", StringRef(data_type.c_str()), allocator);
                // }
                // doc.AddMember(StringRef(ans_key.c_str()), item, allocator);
            }
            writer.EndObject();

            // doc.Accept(writer);
            (*binding) = buffer.GetString();
        }
    }
    response->set_allocated_head(head);
    response->set_allocated_results(results);
}

void GrpcImpl::fun_query_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        FunInfo fun_info = request->funinfo();
        string fun_name = fun_info.funname();
        string fun_status = fun_info.funstatus();
        struct PFNInfos *pfn_infos = new PFNInfos();

        apiUtil->fun_query(fun_name, fun_status, username, pfn_infos);

        vector<struct PFNInfo> list = pfn_infos->getPFNInfoList();
        size_t count = list.size();
        for (size_t i = 0; i < count; i++)
        {
            struct PFNInfo item = list[i];
            fun_copy_from_pfn_info(&fun_info, &item);
            google::protobuf::Any *any = response->add_list();
            any->PackFrom(fun_info);
        }
        response->set_statuscode(0);
        response->set_statusmsg("success");
    }
    catch (const std::exception &e)
    {
        string content = "Fun query fail:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::fun_cudb_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }
        string type = request->type();
        FunInfo fun_info = request->funinfo();
        struct PFNInfo pfn_info;
        fun_copy_from_fun_info(&pfn_info, &fun_info);
        if (type == "1")
        {
            request->set_operation("funcreate");
            try
            {
                apiUtil->fun_create(username, &pfn_info);
                response->set_statuscode(0);
                response->set_statusmsg("success");
            }
            catch(const std::exception& e)
            {
                string content = "Fun create fail:" + string(e.what());
                response->set_statuscode(1005);
                response->set_statusmsg(content);
            }
        }
        else if (type == "2")
        {
            request->set_operation("funupdate");
            try
            {
                apiUtil->fun_update(username, &pfn_info);
                response->set_statuscode(0);
                response->set_statusmsg("success");
                
            }
            catch (const std::exception &e)
            {
                string content = "Fun update fail:" + string(e.what());
                response->set_statuscode(1005);
                response->set_statusmsg(content);
            }
        }
        else if (type == "3")
        {
            request->set_operation("fundelete");
            try
            {
                apiUtil->fun_delete(username, &pfn_info);
                response->set_statuscode(0);
                response->set_statusmsg("success");
            }
            catch (const std::exception &e)
            {
                string content = "Fun delete fail:" + string(e.what());
                response->set_statuscode(1005);
                response->set_statusmsg(content);
            }
        }
        else if (type == "4")
        {
            request->set_operation("funbuild");
            try
            {
                string fun_name = fun_info.funname();
                string result = apiUtil->fun_build(username, fun_name);
                if (result == "")
                {
                    response->set_statuscode(0);
                    response->set_statusmsg("success");
                }
                else
                {
                    Util::formatPrint("Fun build fail:\n" + result, "ERROR");
                    response->set_statuscode(1005);
                    response->set_statusmsg(result);
                }
            }
            catch (const std::exception &e)
            {
                string content = "Fun build fail:" + string(e.what());
                response->set_statuscode(1005);
                response->set_statusmsg(content);
            }
        }
        else
        {
            response->set_statuscode(1003);
            response->set_statusmsg("type is invalid, please look up the api document.");
        }
    }
    catch(const std::exception& e)
    {
        string content = "Fun cudb fail:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
    
}

void GrpcImpl::fun_review_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    struct PFNInfo *pfn_info = nullptr;
    try
    {
        string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
        if(ipCheckResult != "")
        {
            response->set_statuscode(1101);
            response->set_statusmsg(ipCheckResult);
            return;
        }
        string username = request->username();
        string password = request->password();
        string encryption = request->encryption();
        string check_result = apiUtil->check_indentity(username, password, encryption);
        if (check_result.empty() == false)
        {
            response->set_statuscode(1001);
            response->set_statusmsg(check_result);
            return;
        }
        if (apiUtil->check_privilege(username, request->operation(), request->db_name()) == 0)
        {
            check_result = "You have no " + request->operation() + " privilege, operation failed";
            response->set_statuscode(1002);
            response->set_statusmsg(check_result);
            return;
        }

        string content;
        FunInfo fun_info = request->funinfo();
        pfn_info = new PFNInfo();
        fun_copy_from_fun_info(pfn_info, &fun_info);
        apiUtil->fun_review(username, pfn_info);
        content = pfn_info->getFunBody();
        content = Util::urlEncode(content);

        if (pfn_info != nullptr)
        {
            delete pfn_info;
        }
        response->set_statuscode(0);
        response->set_statusmsg("success");
        response->set_result(content);
    }
    catch (const std::exception &e)
    {
        if (pfn_info != nullptr)
        {
            delete pfn_info;
        }
        string content = "Fun review fail:" + string(e.what());
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

 void GrpcImpl::fun_copy_from_fun_info(struct PFNInfo * target, FunInfo * source)
 {
     target->setFunName(source->funname());
     target->setFunArgs(source->funargs());
     target->setFunDesc(source->fundesc());
     target->setFunBody(source->funbody());
     target->setFunSubs(source->funsubs());
     target->setFunReturn(source->funreturn());
     target->setFunStatus(source->funstatus());
     target->setLastTime(source->lasttime());
 }

void GrpcImpl::fun_copy_from_pfn_info(FunInfo * target, struct PFNInfo * source)
{
    target->set_funname(source->getFunName());
    target->set_funargs(source->getFunArgs());
    target->set_fundesc(source->getFunDesc());
    target->set_funbody(source->getFunBody());
    target->set_funsubs(source->getFunSubs());
    target->set_funreturn(source->getFunReturn());
    target->set_funstatus(source->getFunStatus());
    target->set_lasttime(source->getLastTime());
}

void GrpcImpl::default_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string msg = "the operation '" + request->operation() + "' has not match handler function";
    Util::formatPrint(msg,"ERROR");
    response->set_statuscode(1100);
    response->set_statusmsg(msg);
}