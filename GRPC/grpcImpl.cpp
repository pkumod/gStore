/*
 * @Author: your name
 * @Date: 2022-02-28 10:31:06
 * @LastEditTime: 2022-03-10 20:10:54
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /gStore/GRPC/grpcImpl.cpp
 */
#include "grpcImpl.h"

GrpcImpl::GrpcImpl(int argc, char *argv[])
{
    apiUtil = new APIUtil();
    string db_name = "";
    db_name = Util::getArgValue(argc, argv, "db", "database");
    bool load_csr = false;
    load_csr = Util::string2int(Util::getArgValue(argc, argv, "c", "csr", "0"));
    apiUtil->initialize(db_name, load_csr);
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
    std::cout << "----------------------------------------------------------" << std::endl;

    if (operation == "show")
    {
        cout << "in show" <<endl;
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
    else if ( operation == "stop")
    {
        task = WFTaskFactory::create_go_task("batch_remove_task", &GrpcImpl::shutdown_task, this, request, response, ctx);
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
    else if (operation == "fun_create")
    {
        task = WFTaskFactory::create_go_task("fun_create_task", &GrpcImpl::fun_create_task, this, request, response, ctx);
    }
    else if (operation == "fun_query")
    {
        task = WFTaskFactory::create_go_task("fun_retrieve_task", &GrpcImpl::fun_retrieve_task, this, request, response, ctx);
    }
    else if (operation == "fun_update")
    {
        task = WFTaskFactory::create_go_task("fun_update_task", &GrpcImpl::fun_update_task, this, request, response, ctx);
    }
    else if (operation == "fun_delete")
    {
        task = WFTaskFactory::create_go_task("fun_delete_task", &GrpcImpl::fun_delete_task, this, request, response, ctx);
    }
    else if (operation == "fun_build")
    {
        task = WFTaskFactory::create_go_task("fun_build_task", &GrpcImpl::fun_build_task, this, request, response, ctx);
    }
    else if (operation == "fun_review")
    {
        task = WFTaskFactory::create_go_task("fun_review_task", &GrpcImpl::fun_review_task, this, request, response, ctx);
    }
    else
    {
        task = WFTaskFactory::create_go_task("default_task", &GrpcImpl::default_task, this, request, response, ctx);
    }
    task->set_callback([&](WFGoTask *task) {
        apiUtil->write_access_log(operation, remoteIP, response->statuscode(), response->statusmsg());
    });
    ctx->get_series()->push_back(task);
}

void GrpcImpl::build_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
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

void GrpcImpl::load_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
        return;
    }
    try
    {
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
                if(apiUtil->insert_txn_managers(apiUtil->get_database(request->db_name()),request->db_name()) == false)
                {
                    cout<<"when load insert_txn_managers fail"<<endl;
                }
                response->set_statuscode(0);
                response->set_statusmsg("success");
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
        }
    }
    catch (const std::string &exception_msg)
    {
        std::cout << "load failed: " << exception_msg;
        response->set_statuscode(1005);
        response->set_statusmsg("load failed: " + exception_msg);
    }
    catch (const std::runtime_error &e1)
    {
        string content = e1.what();
        std::cout << "load failed:" << content << endl;
        response->set_statuscode(1005);
        response->set_statusmsg("load failed: " + content);
    }
    catch (...)
    {
        string content = "unknow error";
        std::cout << "load failed:" << content << endl;
        response->set_statuscode(1005);
        response->set_statusmsg("load failed: " + content);
    }
}

void GrpcImpl::check_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
        return;
    }
    string success="the ghttp server is running...";
    response->set_statuscode(0);
    response->set_statusmsg(success);
}

void GrpcImpl::monitor_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
        return;
    }
    string error = apiUtil->check_param_value("db_name", request->db_name());
    if( error.empty() == false)
    {
        response->set_statuscode(1003);
        response->set_statusmsg(error);
        return;
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
    if(apiUtil->trywrlock_databaseinfo(apiUtil->get_databaseinfo(request->db_name())) == false)
    {
        error = "Unable to monitor due to loss of lock";
        response->set_statuscode(1007);
        response->set_statusmsg(error);
        return;
    }
    string result = apiUtil->get_moniter_info(apiUtil->get_database(request->db_name()), apiUtil->get_databaseinfo(request->db_name()));
    apiUtil->unlock_databaseinfo(apiUtil->get_databaseinfo(request->db_name()));
    rapidjson::Document doc;
    doc.SetObject();
    doc.Parse(result.c_str());
    if (doc.HasParseError())
    {
        cout << "parse already build data error: " << doc.GetParseError() << endl;
        response->set_statuscode(1005);
        response->set_statusmsg("Get the database infomation error!");
        return;
    }
    Monitor monitor;
    if (doc.HasMember("database"))
    {
        monitor.set_database(doc["database"].GetString());
    }
    if (doc.HasMember("creator"))
    {
        monitor.set_creator(doc["creator"].GetString());
    }
    if (doc.HasMember("built_time"))
    {
        monitor.set_built_time(doc["built_time"].GetString());
    }
    if (doc.HasMember("triple num"))
    {
        monitor.set_triple_num(doc["triple num"].GetString());
    }
    if (doc.HasMember("literal num"))
    {
        monitor.set_literal_num(doc["literal num"].GetInt());
    }
    if (doc.HasMember("subject num"))
    {
        monitor.set_subject_num(doc["subject num"].GetInt());
    }
    if (doc.HasMember("predicate num"))
    {
        monitor.set_predicate_num(doc["predicate num"].GetInt());
    }
    if (doc.HasMember("connection num"))
    {
        monitor.set_connection_num(doc["connection num"].GetInt());
    }
    if (doc.HasMember("entity num"))
    {
        monitor.set_entity_num(doc["entity num"].GetInt());
    }
    google::protobuf::Any *any = response->add_responsebody();
    any->PackFrom(monitor);
    response->set_statuscode(0);
    response->set_statusmsg("success");
}

void GrpcImpl::unload_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{

    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
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
        error = "Database not load yet.";
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

void GrpcImpl::drop_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
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
		
		if (request->is_backup() == "false")
			cmd = "rm -r " + db_name + ".db";
		else if (request->is_backup() == "true")
			cmd = "mv " + db_name + ".db " + db_name + ".bak";
		cout<<"delete the file: "<<cmd<<endl;
		system(cmd.c_str());

        Util::delete_backuplog(db_name);
		string success = "Database " + db_name + " dropped.";
        response->set_statuscode(0);
        response->set_statusmsg(success);
    }
}

void GrpcImpl::show_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    cout << "show task begin" << endl;
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    cout << "check ip over" <<endl;
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
        return;
    }
    cout << "ip check over "<<endl;
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
    vector<struct DatabaseInfo *> array;
    apiUtil->get_already_builds(array);
    size_t count = array.size();
    for (size_t i = 0; i < count; i++)
    {
        DatabaseInfo * db_ptr = array[i];
        DBInfo dbInfo;
        dbInfo.set_database(db_ptr->getName());
        dbInfo.set_creator(db_ptr->getCreator());
        dbInfo.set_built_time(db_ptr->getTime());
        dbInfo.set_status(db_ptr->getStatus());
        google::protobuf::Any *any = response->add_responsebody();
        any->PackFrom(dbInfo);
    }
    // set response status and message
    response->set_statuscode(0);
    response->set_statusmsg("Get the database list successfully!");
}

void GrpcImpl::backup_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
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

void GrpcImpl::restore_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
        return;
    }
    string db_name = request->db_name();
    string username = request->username();
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
	Util::formatPrint("restore database:" + database);
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
        error = "the operation can not been excuted due to loss of lock.";
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
        string error = "Failed to restore the database. Backup Path Error";
        apiUtil->unlock_databaseinfo(db_info);
        response->set_statuscode(1007);
        response->set_statusmsg(error);
    }
    else
    {
        //TODO update the in system.db
        path = Util::get_folder_name(path, db_name);
        sys_cmd = "cp -r " + path + " " + db_name + ".db";
        system(sys_cmd.c_str());
        
        apiUtil->unlock_databaseinfo(db_info);

        string success = "Database " + db_name + " restore successfully.";
        response->set_statuscode(0);
        response->set_statusmsg(success);
    }
}

void GrpcImpl::query_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
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
        cout << thread_id << ":search query returned successfully." << endl;
        if (rs.ansNum > apiUtil->get_max_output_size())
        {
            if (rs.output_limit == -1 || rs.output_limit > apiUtil->get_max_output_size())
            {
                rs.output_limit = apiUtil->get_max_output_size();
            }
        }
        std::string query_time_s = Util::int2string(query_time);
        std::string ans_num_s = Util::int2string(rs.ansNum);
        std::string log_info = "queryTime: " + query_time_s + ", ansNum: " + ans_num_s;
        Util::formatPrint(log_info);
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
        response->set_ansnum(rs.ansNum);
        response->set_outputlimit(rs.output_limit);
        response->set_querytime(query_time_s);
        response->set_threadid(thread_id);
        
        // record each query operation, including the sparql and the answer number
        // accurate down to microseconds
        std::string remoteIP = ctx->get_remote_ip();
        long ansNum = rs.ansNum;
        int statusCode = response->statuscode();
        struct DBQueryLogInfo* queryLogInfo = new DBQueryLogInfo(query_start_time, remoteIP, sparql, ansNum, format, filename, statusCode, query_time);
        apiUtil->write_query_log(queryLogInfo);
        delete queryLogInfo;
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

void GrpcImpl::export_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
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
	if(Util::dir_exist(db_path)==false)
	{
        Util::create_dir(db_path);
    }	
	db_path = db_path + db_name +"_"+Util::get_timestamp()+ ".nt";
    //check if database named [db_name] is already load
    if(!apiUtil->check_already_load(db_name))
    {
        error = "Database not load yet.";
		response->set_statuscode(1004);
        response->set_statusmsg(error);
        return;
    }
    Database *current_database = apiUtil->get_database(db_name);
    apiUtil->rdlock_database(db_name);//lock database

    string sparql = "select * where {?x ?y ?z.} ";
	ResultSet rs;
    Util::formatPrint("db_path: " + db_path);
	FILE* ofp = fopen(db_path.c_str(), "w");
    int ret = current_database->query(sparql, rs, ofp, true, true);
    fflush(ofp);
	fclose(ofp);
	ofp = NULL;
    current_database = NULL;

	string success = "Export the database successfully.";
    apiUtil->unlock_database(db_name);//unlock database
    
    response->set_statuscode(0);
    response->set_statusmsg(success);
    response->set_filepath(db_path);
    return;
}

void GrpcImpl::login_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
        return;
    }
    string success="login successfully.";
    
    response->set_statuscode(0);
    response->set_statusmsg(success);
    string version = Util::getConfigureValue("version");
    response->set_coreversion(version);
    string licensetype=Util::getConfigureValue("licensetype");
    response->set_licensetype(licensetype);

}

void GrpcImpl::begin_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
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

void GrpcImpl::tquery_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
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
		response->set_statuscode(1005);
        response->set_statusmsg(error);
	}
	else if(ret == -99)
	{
		error = "Transaction query failed. This transaction is not in running status!";
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
            //TODO res = rs.to_JSON() need convert to query result object
            cout << res <<endl;
            Value& a = resDoc["head"]["vars"];
            QueryHeadInfo *head = new QueryHeadInfo();
            QueryResultInfo *results = new QueryResultInfo();
            for(int i = 0; i<a.Size(); i++)
            {
                head->add_vars(resDoc["head"]["vars"][i].GetString());
            }
            Value& b = resDoc["results"]["bindings"];
            for(int i = 0; i<b.Size(); i++)
            {
                results->add_bindings(resDoc["results"]["bindings"][i].GetString());
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

void GrpcImpl::commit_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
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
    auto txn_m = apiUtil->get_Txn_ptr(db_name);
    if(txn_m == NULL)
    {
        error = "Database transaction manager error.";
        response->set_statuscode(1004);
        response->set_statusmsg(error);
        return;
    }
    int ret = txn_m->Commit(TID);
    if (ret == 1)
	{
		error = "transaction not in running state! commit failed. TID: " + TID_s;
		response->set_statuscode(1005);
        response->set_statusmsg(error);
	}
	else if (ret == -1)
	{
		error = "transaction not found, commit failed. TID: " + TID_s;
		response->set_statuscode(1005);
        response->set_statusmsg(error);
	}
	else
	{
        apiUtil->commit_process(txn_m, TID);
		string success = "transaction commit success. TID: " + TID_s;
		response->set_statuscode(0);
        response->set_statusmsg(success);
	}

}

void GrpcImpl::rollback_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
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

void GrpcImpl::checkpoint_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
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
            txn_m->Checkpoint();
            current_database->save();
            apiUtil->unlock_database(db_name);
            response->set_statuscode(0);
            response->set_statusmsg("Database saved successfully.");
        }
    }
}

void GrpcImpl::test_connect_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
        return;
    }
    response->set_statuscode(0);
    response->set_statusmsg("success");
    string version = Util::getConfigureValue("version");
    response->set_coreversion(version);
    string licensetype=Util::getConfigureValue("licensetype");
    response->set_licensetype(licensetype);
}

void GrpcImpl::core_version_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
        return;
    }
    response->set_statuscode(0);
    response->set_statusmsg("success");
    string version = Util::getConfigureValue("version");
    response->set_coreversion(version);
}

void GrpcImpl::batch_insert_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
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
        response->set_success_num(Util::int2string(success_num));
        apiUtil->write_access_log("batchInsert",ctx->get_remote_ip(),0,"Batch insert data successfully.");
        return;
    }
}

void GrpcImpl::batch_remove_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
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
        response->set_success_num(Util::int2string(success_num));
        apiUtil->write_access_log("batchRemove",ctx->get_remote_ip(),0,"Batch remove data successfully.");
    }
}

void GrpcImpl::shutdown_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    exit(0);
}

void GrpcImpl::user_manage_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(!ipCheckResult.empty())
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
        return;
    }
    string type = request->type();
    string error = "";
    string username = request->username();
    string password = request->password();
    if(type == "1")//insert user
    {
        if(username.empty()||password.empty())
		{
            error="the user name and password can not be empty while adding user.";
			response->set_statuscode(1003);
            response->set_statusmsg(error);
		}
        else if(apiUtil->user_add(username,password))
        {
            error="user add done.";
            response->set_statuscode(0);
            response->set_statusmsg(error);
        }
        else
        {
            error = "username already existed, add user failed.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
        }
    }
    else if(type == "2") //delete user
    {
        if(username == ROOT_USERNAME)
        {
            error = "you cannot delete root, delete user failed.";
            response->set_statuscode(1004);
            response->set_statusmsg(error);
        }
        else if(apiUtil->user_delete(username, password))
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
        if(apiUtil->user_pwd_alert(username, password))
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

void GrpcImpl::user_show_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
        return;
    }
    string result = apiUtil->get_user_info();
    if(result != ""){
        rapidjson::Document doc;
        doc.SetArray();
        doc.Parse(result.c_str());
        size_t len = doc["ResponseBody"].Size();
        for (size_t i = 0; i < len; i++)
        {

            UserInfo userInfo;
            if (doc["ResponseBody"][i].HasMember("username"))
                userInfo.set_username(doc["ResponseBody"][i]["username"].GetString());

            if (doc["ResponseBody"][i].HasMember("password"))
                userInfo.set_password(doc["ResponseBody"][i]["password"].GetString());

            if (doc["ResponseBody"][i].HasMember("query_privilege"))
                userInfo.set_query_privilege(doc["ResponseBody"][i]["query_privilege"].GetString());

            if (doc["ResponseBody"][i].HasMember("update_privilege"))
                userInfo.set_update_privilege(doc["ResponseBody"][i]["update_privilege"].GetString());
            
            if (doc["ResponseBody"][i].HasMember("load_privilege"))
                userInfo.set_load_privilege(doc["ResponseBody"][i]["load_privilege"].GetString());
            
            if (doc["ResponseBody"][i].HasMember("unload_privilege"))
                userInfo.set_unload_privilege(doc["ResponseBody"][i]["unload_privilege"].GetString());
            
            if (doc["ResponseBody"][i].HasMember("backup_privilege"))
                userInfo.set_backup_privilege(doc["ResponseBody"][i]["backup_privilege"].GetString());

            if (doc["ResponseBody"][i].HasMember("restore_privilege"))
                userInfo.set_restore_privilege(doc["ResponseBody"][i]["restore_privilege"].GetString());

            if (doc["ResponseBody"][i].HasMember("export_privilege"))
                userInfo.set_export_privilege(doc["ResponseBody"][i]["export_privilege"].GetString());
            google::protobuf::Any *any = response->add_responsebody();
            any->PackFrom(userInfo);
        }
        // set response status and message
        response->set_statuscode(0);
        response->set_statusmsg("success");
    }
    else
    {
        string error = "No Users";
        response->set_statuscode(0);
        response->set_statusmsg(error);
    }
    return;
}

void GrpcImpl::user_privilege_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
        return;
    }
    string error ="";
    string privilege = request->privileges();
    error = apiUtil->check_param_value("type", request->type());
    if (error.empty() == false)
    {
        response->set_statuscode(1003);
        response->set_statusmsg(error);
        return;
    }
    error = apiUtil->check_param_value("username", request->username());
    if (error.empty() == false)
    {
        response->set_statuscode(1003);
        response->set_statusmsg(error);
        return;
    }
    if(request->type()!="3")
    {
        error = apiUtil->check_param_value("db_name", request->db_name());
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
        error = apiUtil->check_param_value("privilege", request->privileges());
        if (error.empty() == false)
        {
            response->set_statuscode(1003);
            response->set_statusmsg(error);
            return;
        }
    }

    if (request->username() == ROOT_USERNAME)
	{
		error = "you can't add privilege to root user.";
		response->set_statuscode(1004);
        response->set_statusmsg(error);
		return;
	}
    string result = "";
    if(request->type() == "3")
    {
        int resultint = apiUtil->clear_user_privilege(request->username());
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
				if (request->type() == "1")
				{
					if (apiUtil->add_privilege(request->username(), temp_privilege, request->db_name()) == 0)
					{
						result = result + "add privilege " + temp_privilege + " failed. \r\n";
					}
					else
					{
						result = result + "add privilege " + temp_privilege + " successfully. \r\n";
					}
				}
				else if (request->type() == "2")
				{
					if (apiUtil->del_privilege(request->username(), temp_privilege, request->db_name()) == 0)
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

void GrpcImpl::txn_log_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
        return;
    }
    if(request->username() == ROOT_USERNAME)
    {
        string result = Util::get_transactionlog();
        response->set_statuscode(0);
        response->set_statusmsg("Get Transaction log success");
        rapidjson::Document doc;
        doc.SetArray();
        doc.Parse(result.c_str());
        size_t len = doc["list"].Size();
        for (size_t i = 0; i < len; i++)
        {
            TxnLogInfo txnLogInfo;
            if (doc["list"][i].HasMember("db_name"))
                txnLogInfo.set_db_name(doc["list"][i]["db_name"].GetString());

            if (doc["list"][i].HasMember("TID"))
                txnLogInfo.set_tid(doc["list"][i]["TID"].GetString());
            
            if (doc["list"][i].HasMember("user"))
                txnLogInfo.set_user(doc["list"][i]["user"].GetString());
            
            if (doc["list"][i].HasMember("begin_time"))
                txnLogInfo.set_begin_time(doc["list"][i]["begin_time"].GetString());
            
            if (doc["list"][i].HasMember("state"))
                txnLogInfo.set_state(doc["list"][i]["state"].GetString());
            
            if (doc["list"][i].HasMember("end_time"))
                txnLogInfo.set_end_time(doc["list"][i]["end_time"].GetString());
            
            google::protobuf::Any *any = response->add_list();
            any->PackFrom(txnLogInfo);
        }
    }
    else
    {
        response->set_statuscode(1003);
        response->set_statusmsg("Root User Only!");
        return;
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
        string msg = "Get query log error: " + string(e.what());
        Util::formatPrint(msg, "ERROR");
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
        string msg = "Get access log error: " + string(e.what());
        Util::formatPrint(msg, "ERROR");
        response->set_statuscode(1005);
        response->set_statusmsg(msg);
    }
}

void GrpcImpl::ip_manage_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string ipCheckResult = apiUtil->check_access_ip(ctx->get_remote_ip());
    if(ipCheckResult != "")
    {
        response->set_statuscode(1101);
        response->set_statusmsg(ipCheckResult);
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
        ipManageInfo->set_ip_type(IPtype);
        vector<string>ip_list = apiUtil->ip_list(IPtype);
        for(int i = 0 ;i<ip_list.size();i++){
           ipManageInfo->add_ips(ip_list[i]);
        }
        response->set_statuscode(0);
        response->set_statusmsg("success");
        google::protobuf::Any *any = response->add_responsebody();
        any->PackFrom(*ipManageInfo);
        return;
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
        if(request->ip_type() == "1"|| request->ip_type() == "2")
        {
            if(apiUtil->ip_save(request->ip_type(),ipVector)==false)
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
            
        }
        else
        {
            response->set_statuscode(1003);
            response->set_statusmsg("ip_type is invalid, please look up the api document.");
            return;
        }
        response->set_statuscode(0);
        response->set_statusmsg("success");
        return;
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
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            writer.StartObject();
            for (int j = 0; j < rs.true_select_var_num; j++)
            {
                string ans_key, ans_type, ans_str;
                ans_str = rs.answer[i][j];
                cout << "rs.answer[" << i << "][" << j << "]: " << ans_str << endl;
                if (ans_str.length() == 0)
                    continue;
                ans_key = rs.var_name[j].substr(1);
                writer.Key(ans_key.c_str());

                writer.StartObject();
                if (ans_str[0] == '<')
                {
                    ans_type = "uri";
                    ans_str = ans_str.substr(1, ans_str.length() - 2);
                    writer.Key("type");
                    writer.String(ans_type.c_str());
                    writer.Key("value");
                    writer.String(ans_str.c_str());
                }
                else if (ans_str[0] == '"')
                {
                    if (ans_str.find("\"^^<") == string::npos)
                    {
                        // no has type string
                        ans_type = "literal";
                        ans_str = ans_str.substr(1, ans_str.rfind('"') - 1);
                        writer.Key("type");
                        writer.String(ans_type.c_str());
                        writer.Key("value");
                        writer.String(ans_str.c_str());
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
                            writer.Key("type");
                            writer.String(ans_type.c_str());
                            writer.Key("value");
                            writer.String(ans_str.c_str());
                            writer.Key("datatype");
                            writer.String(data_type.c_str());
                        }
                        else
                        {
                            // the entity value is not complete
                            ans_type = "typed-literal";
                            int pos = ans_str.find("\"^^<");
                            data_type = "http://www.w3.org/2001/XMLSchema#string-not-complete";
                            ans_str = ans_str.substr(0, pos + 1);
                            writer.Key("type");
                            writer.String(ans_type.c_str());
                            writer.Key("value");
                            writer.String(ans_str.c_str());
                            writer.Key("datatype");
                            writer.String(data_type.c_str());
                        }
                    }
                }
                else
                {
                    ans_type = "unknown";
                    ans_str = "the information is not complete!";
                    writer.Key("type");
                    writer.String(ans_type.c_str());
                    writer.Key("value");
                    writer.String(ans_str.c_str());
                }
                writer.EndObject();
            }
            writer.EndObject();
            (*binding) = buffer.GetString();
        }
    }
    response->set_allocated_head(head);
    response->set_allocated_results(results);
}

void GrpcImpl::fun_create_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    struct PFNInfo *pfn_info = nullptr;
    try
    {
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
        FunInfo fun_info = request->fun_info();
        pfn_info = new PFNInfo();
        fun_copy_from_fun_info(pfn_info, &fun_info);
        apiUtil->fun_create(username, pfn_info);
        delete pfn_info;
        response->set_statuscode(0);
        response->set_statusmsg("success");
    }
    catch (const std::exception &e)
    {
        if (pfn_info)
        {
            delete pfn_info;
        }
        string msg = string(e.what());
        Util::formatPrint(msg,"ERROR");
        response->set_statuscode(1005);
        response->set_statusmsg(msg);
    }
}

void GrpcImpl::fun_retrieve_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
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
        
        FunInfo fun_info = request->fun_info();
        string fun_name = fun_info.fun_name();
        string fun_status = fun_info.fun_status();
        struct PFNInfos *pfn_infos = new PFNInfos();

        apiUtil->fun_query(fun_name, fun_status, username, pfn_infos);

        vector<struct PFNInfo> list = pfn_infos->getPFNInfoList();
        size_t count = list.size();
        for (size_t i = 0; i < count; i++)
        {
            struct PFNInfo item = list[i];
            fun_copy_from_pfn_info(&fun_info, &item);
            google::protobuf::Any *any = response->add_responsebody();
            any->PackFrom(fun_info);
        }
        response->set_statuscode(0);
        response->set_statusmsg("success");
    }
    catch (const std::exception &e)
    {
        string content = "query fail: " + string(e.what());
        std::cout << content << '\n';
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::fun_update_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    struct PFNInfo *pfn_info = nullptr;
    try
    {
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
        FunInfo fun_info = request->fun_info();
        pfn_info = new PFNInfo();
        fun_copy_from_fun_info(pfn_info, &fun_info);
        apiUtil->fun_update(username, pfn_info);
        delete pfn_info;
        response->set_statuscode(0);
        response->set_statusmsg("success");
        
    }
    catch (const std::exception &e)
    {
        if (pfn_info)
        {
            delete pfn_info;
        }
        string content = string(e.what());
        Util::formatPrint(content, "ERROR");
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::fun_delete_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    struct PFNInfo *pfn_info = nullptr;
    try
    {
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
        FunInfo fun_info = request->fun_info();
        pfn_info = new PFNInfo();
        fun_copy_from_fun_info(pfn_info, &fun_info);
        apiUtil->fun_delete(username, pfn_info);
        delete pfn_info;
        response->set_statuscode(0);
        response->set_statusmsg("success");
    }
    catch (const std::exception &e)
    {
        if (pfn_info)
        {
            delete pfn_info;
        }
        string content = string(e.what());
        Util::formatPrint(content, "ERROR");
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::fun_build_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    try
    {
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
        FunInfo fun_info = request->fun_info();
        string fun_name = fun_info.fun_name();
        string result = apiUtil->fun_build(username, fun_name);
        if (result == "")
        {
            response->set_statuscode(0);
            response->set_statusmsg("success");
            return;
        }
        else
        {
            string content = string(result);
            Util::formatPrint(content, "ERROR");
            response->set_statuscode(1005);
            response->set_statusmsg(content);
            return;
        }
    }
    catch (const std::exception &e)
    {
        string content = string(e.what());
        Util::formatPrint(content, "ERROR");
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

void GrpcImpl::fun_review_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    struct PFNInfo *pfn_info = nullptr;
    try
    {
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

        string content;
        FunInfo fun_info = request->fun_info();
        pfn_info = new PFNInfo();
        fun_copy_from_fun_info(pfn_info, &fun_info);
        apiUtil->fun_review(username, pfn_info);
        content = pfn_info->getFunBody();
        content = Util::urlEncode(content);
        delete pfn_info;

        response->set_statuscode(0);
        response->set_statusmsg("success");
        response->set_result(content);
    }
    catch (const std::exception &e)
    {
        if (pfn_info)
        {
            delete pfn_info;
        }
        string content = "review fail: " + string(e.what());
        Util::formatPrint(content, "ERROR");
        response->set_statuscode(1005);
        response->set_statusmsg(content);
    }
}

 void GrpcImpl::fun_copy_from_fun_info(struct PFNInfo * target, FunInfo * source)
 {
     target->setFunName(source->fun_name());
     target->setFunArgs(source->fun_args());
     target->setFunDesc(source->fun_desc());
     target->setFunBody(source->fun_body());
     target->setFunSubs(source->fun_subs());
     target->setFunReturn(source->fun_return());
     target->setFunStatus(source->fun_status());
     target->setLastTime(source->last_time());
 }

void GrpcImpl::fun_copy_from_pfn_info(FunInfo * target, struct PFNInfo * source)
{
    target->set_fun_name(source->getFunName());
    target->set_fun_args(source->getFunArgs());
    target->set_fun_desc(source->getFunDesc());
    target->set_fun_body(source->getFunBody());
    target->set_fun_subs(source->getFunSubs());
    target->set_fun_return(source->getFunReturn());
    target->set_fun_status(source->getFunStatus());
    target->set_last_time(source->getLastTime());
}

void GrpcImpl::default_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx)
{
    string msg = "the operation '" + request->operation() + "' has not match handler function";
    Util::formatPrint(msg,"ERROR");
    response->set_statuscode(1100);
    response->set_statusmsg(msg);
}