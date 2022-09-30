#include <stdio.h>
#include "workflow/WFFacilities.h"
#include "../GRPC/grpc_server.h"
#include "../GRPC/grpc_status_code.h"
#include "../GRPC/grpc_operation.h"
#include "../GRPC/APIUtil.h"

#define HTTP_TYPE "grpc"

using namespace std;
using namespace grpc;

static WFFacilities::WaitGroup wait_group(1);

APIUtil *apiUtil = nullptr;

int initialize(int argc, char *argv[]);

void register_service(GRPCServer &grpcServer);

void shutdown(const GRPCReq *request, GRPCResp *response);
void api(const GRPCReq *request, GRPCResp *response);
// for server
void check_task(const GRPCReq *request, GRPCResp *response);
void login_task(const GRPCReq *request, GRPCResp *response);
void test_connect_task(const GRPCReq *request, GRPCResp *response);
void core_version_task(const GRPCReq *request, GRPCResp *response);
void ip_manage_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
// for db
void show_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void load_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void unload_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void monitor_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void build_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void drop_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void backup_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void restore_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void query_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void export_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void begin_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void tquery_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void commit_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void rollback_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void checkpoint_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void batch_insert_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void batch_remove_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
// for user
void user_manage_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void user_show_task(const GRPCReq *request, GRPCResp *response);
void user_privilege_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void user_password_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
// for log
void txn_log_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void query_log_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void query_log_date_task(const GRPCReq *request, GRPCResp *response);
void access_log_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void access_log_date_task(const GRPCReq *request, GRPCResp *response);
// for personalized function
void fun_query_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void fun_cudb_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void fun_review_task(const GRPCReq *request, GRPCResp *response, Json &json_data);

std::string jsonParam(const Json &json, const std::string &key)
{
	if (json.HasMember(key.c_str()) && json[key.c_str()].IsString())
	{
		return json[key.c_str()].GetString();
	}
	else
	{
		return "";
	}
}

std::string jsonParam(const Json &json, const std::string &key, const std::string &default_val)
{
	string value;
	if (json.HasMember(key.c_str()) && json[key.c_str()].IsString())
	{
		value = json[key.c_str()].GetString();
	}
	if (value.empty())
	{
		return default_val;
	}
	else
	{
		return value;
	}
}

int jsonParam(const Json &json, const std::string &key, const int &default_val)
{
	if (json.HasMember(key.c_str()))
	{
		if (json[key.c_str()].IsInt())
		{
			return json[key.c_str()].GetInt();
		}
		else if (json[key.c_str()].IsString())
		{
			return atoi(json[key.c_str()].GetString());
		} else {
			return default_val;
		}
	}
	else
	{
		return default_val;
	}
}

bool hasJsonParam(const Json &json, const std::string &key)
{
	return json.HasMember(key.c_str());
}

void sig_handler(int signo)
{
	if (apiUtil)
	{
		delete apiUtil;
	}
	SLOG_DEBUG("grpc server stopped.");
	wait_group.done();
	std::cout.flush();
	_exit(signo);
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	apiUtil = new APIUtil();
	while (true)
	{
		pid_t fpid = fork();
		// cout << "fpid:" << fpid << endl;
		if (fpid == 0)
		{
			int ret = initialize(argc, argv);
			std::cout.flush();
			_exit(ret);
		}
		else if (fpid > 0)
		{
			int status;
			waitpid(fpid, &status, 0);
			if (WIFEXITED(status))
			{
				return 0;
			}
			else
			{
				if (Util::file_exist("system.db/port.txt"))
				{
					string cmd = "rm system.db/port.txt";
					system(cmd.c_str());
				}
				SLOG_WARN("Stopped abnormally, restarting server...");
			}
		}
		else
		{
			SLOG_ERROR("Failed to start server: deamon fork failure.");
			return -1;
		}
	}

	return 0;
}

int initialize(int argc, char *argv[])
{
	unsigned short port = 9000;
	string db_name = "";
	string port_str = apiUtil->get_default_port();
	bool loadCSR = 0; // DO NOT load CSR by default
	stringstream ss;
	if (argc < 2)
	{
		SLOG_DEBUG("Server will use the default port: " + port_str);
		SLOG_DEBUG("Not load any database!");
		port = atoi(port_str.c_str());
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore RPC Server(grpc)" << endl;
			cout << endl;
			cout << "Usage:\tbin/grpc -p [port]" << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database[option],\t\tthe database name.Default value is empty. Notice that the name can not end with .db" << endl;
			cout << "\t-p,--port[option],\t\tthe listen port. Default value is 9000." << endl;
			cout << "\t-c,--csr[option],\t\tEnable CSR Struct or not. 0 denote that false, 1 denote that true. Default value is 0." << endl;

			cout << endl;
			return 0;
		}
		else
		{
			cout << "Invalid arguments! Input \"bin/grpc -h\" for help." << endl;
			return 0;
		}
	}
	else
	{
		db_name = Util::getArgValue(argc, argv, "db", "database");
		if (db_name.length() > 3 && db_name.substr(db_name.length() - 3, 3) == ".db")
		{
			SLOG_ERROR("Your db name to be built should not end with \".db\".");
			return -1;
		}
		else if (db_name == "system")
		{
			SLOG_ERROR("You can not load system files.");
			return -1;
		}
		port_str = Util::getArgValue(argc, argv, "p", "port", port_str);
		port = atoi(port_str.c_str());
		loadCSR = Util::string2int(Util::getArgValue(argc, argv, "c", "csr", "0"));
	}
	// check port.txt exist
	if (Util::file_exist("system.db/port.txt"))
	{
		SLOG_ERROR("Server port " + port_str + " is already in use.");
		return -1;
	}

	// call apiUtil initialized
	if (apiUtil->initialize("grpc", port_str, db_name, loadCSR) == -1)
	{
		return -1;
	}

	GRPCServer grpcServer;
	// register rest service
	register_service(grpcServer);

	if(grpcServer.start(port) == 0)
	{
		SLOG_DEBUG("grpc server port " + port_str);
	}
	else
	{
		SLOG_ERROR("grpc server start failed.");
		delete apiUtil;
		return -1;
	}

	// handle the Ctrl+C signal
	signal(SIGINT, sig_handler);

	wait_group.wait();
	grpcServer.stop();
	SLOG_DEBUG("grpc server stoped.");
	return 0;
}

void register_service(GRPCServer &svr)
{
	std::vector<std::string> methods = {"GET", "POST"};
	svr.ROUTE(
		"/grpc/shutdown", [](const GRPCReq *request, GRPCResp *response)
		{ 
			shutdown(request, response);
		},
		methods);

	svr.ROUTE(
		"/grpc/api", [](const GRPCReq *request, GRPCResp *response)
		{ 
			api(request, response);
		},
		methods);
}

void shutdown(const GRPCReq *request, GRPCResp *response)
{
	// check ip address
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	std::string ipCheckResult = apiUtil->check_access_ip(ip_addr);
	if (ipCheckResult.empty() == false)
	{
		SLOG_DEBUG(ipCheckResult);
		response->Error(StatusIPBlocked, ipCheckResult);
		return;
	}
	Json json_data;
	json_data.SetObject();
	Json::AllocatorType &allocator = json_data.GetAllocator();
	SLOG_DEBUG("Content-Type:" + ContentType::to_str(request->contentType()));
	if (request->contentType() == APPLICATION_JSON) //for application/json
	{
		Json &json = request->json();
		json_data.CopyFrom(json, allocator, true);
	}
	else if (request->contentType() == APPLICATION_URLENCODED) //for applicaiton/x-www-form-urlencoded
	{
		std::map<std::string, std::string> &form_data = request->formData();
		std::map<std::string, std::string>::iterator iter = form_data.begin();
		std::stringstream ss;
		std::string v;
		ss << "{";
		int count = 0;
		while (iter != form_data.end())
		{
			if (count > 0)
			{
				ss << ",";
			}
			v = iter->second;
			if (UrlEncode::is_url_encode(v))
			{
				StringUtil::url_decode(v);
			}
			ss << "\"" << iter->first << "\":" << "\"" << v << "\"";
			count++;
			iter++;
		}
		ss << "}";
		json_data.Parse(ss.str().c_str());
	}
	else // for get
	{
		std::map<std::string, std::string> params = request->queryList();
		if (params.empty() == false)
		{
			std::map<std::string, std::string>::iterator iter = params.begin();
			std::stringstream ss;
			std::string v;
			ss << "{";
			int count = 0;
			while (iter != params.end())
			{
				if (count > 0)
				{
					ss << ",";
				}
				v = iter->second;
				if (UrlEncode::is_url_encode(v))
				{
					StringUtil::url_decode(v);
				}
				ss << "\"" << iter->first << "\":" << "\"" << v << "\"";
				count++;
				iter++;
			}
			ss << "}";
			json_data.Parse(ss.str().c_str());
		}
	}
	std::stringstream ss;
	ss << "\n------------------------ grpc-api ------------------------";
	ss << "\nremote_ip: " << ip_addr;
	ss << "\noperation: shutdown";
	ss << "\nmethod: " << request->get_method();
	ss << "\nrequest_path: " << request->current_path();
	ss << "\nhttp_version: " << request->get_http_version();
	ss << "\nrequest_body: \n" << request->body();
	ss << "\n----------------------------------------------------------";
	SLOG_DEBUG(ss.str());
	std::string error;
	std::string username = jsonParam(json_data, "username");
	std::string password = jsonParam(json_data, "password");
	error = apiUtil->check_param_value("username", username);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	error = apiUtil->check_param_value("password", password);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	if (username != apiUtil->get_system_username())
	{
		error = "You have no rights to stop the server.";
        response->Error(StatusAuthenticationFailed, error);
		return;
	}
	error = apiUtil->check_server_indentity(password);
	if (error.empty() == false)
	{
		apiUtil->update_access_ip_error_num(ip_addr);
		response->Error(StatusAuthenticationFailed, error);
		return;
	}
	bool flag = apiUtil->db_checkpoint_all();
	if (flag)
	{
		// exit async
		rpc_task->add_callback([](GRPCTask *grpcTask){
			SLOG_DEBUG("Server stopped successfully.");
			std::cout.flush();
			_exit(1);
		});
		std::string msg = "Server stopped successfully.";
		apiUtil->write_access_log("shutdown", ip_addr, StatusOK, msg);
		// free apiUtil
		delete apiUtil;
		response->Success(msg);
	}
	else
	{
		error = "Server stopped failed.";
		apiUtil->write_access_log("shutdown", ip_addr, StatusOperationFailed, error);
		response->Error(StatusOperationFailed, error);
	}
}

void api(const GRPCReq *request, GRPCResp *response)
{
	// check ip address
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	std::string ipCheckResult = apiUtil->check_access_ip(ip_addr);
	if (ipCheckResult.empty() == false)
	{
		SLOG_DEBUG(ipCheckResult);
		response->Error(StatusIPBlocked, ipCheckResult);
		return;
	}

	Json json_data;
	json_data.SetObject();
	Json::AllocatorType &allocator = json_data.GetAllocator();
	SLOG_DEBUG("Content-Type:" + ContentType::to_str(request->contentType()));
	if (request->contentType() == APPLICATION_JSON) //for application/json
	{
		Json &json = request->json();
		json_data.CopyFrom(json, allocator, true);
	}
	else if (request->contentType() == APPLICATION_URLENCODED) //for applicaiton/x-www-form-urlencoded
	{
		std::map<std::string, std::string> &form_data = request->formData();
		std::map<std::string, std::string>::iterator iter = form_data.begin();
		std::stringstream ss;
		std::string v;
		ss << "{";
		int count = 0;
		while (iter != form_data.end())
		{
			if (count > 0)
			{
				ss << ",";
			}
			v = iter->second;
			if (UrlEncode::is_url_encode(v))
			{
				StringUtil::url_decode(v);
			}
			ss << "\"" << iter->first << "\":" << "\"" << v << "\"";
			count++;
			iter++;
		}
		ss << "}";
		json_data.Parse(ss.str().c_str());
	}
	else // for get
	{
		std::map<std::string, std::string> params = request->queryList();
		if (params.empty() == false)
		{
			std::map<std::string, std::string>::iterator iter = params.begin();
			std::stringstream ss;
			std::string v;
			ss << "{";
			int count = 0;
			while (iter != params.end())
			{
				if (count > 0)
				{
					ss << ",";
				}
				v = iter->second;
				if (UrlEncode::is_url_encode(v))
				{
					StringUtil::url_decode(v);
				}
				ss << "\"" << iter->first << "\":" << "\"" << v << "\"";
				count++;
				iter++;
			}
			ss << "}";
			json_data.Parse(ss.str().c_str());
		}
	}
	// add remote_ip param
	json_data.AddMember("remote_ip", StringRef(ip_addr.c_str()), allocator);
	std::string operation = jsonParam(json_data, "operation", "");
	operation_type op_type = OperationType::to_enum(operation);
	std::stringstream ss;
	ss << "\n------------------------ grpc-api ------------------------";
	ss << "\nremote_ip: " << ip_addr;
	ss << "\noperation: " << operation;
	ss << "\nmethod: " << request->get_method();
	ss << "\nrequest_path: " << request->current_path();
	ss << "\nhttp_version: " << request->get_http_version();
	ss << "\nrequest_body: \n" << request->body();
	ss << "\n----------------------------------------------------------";
	SLOG_DEBUG(ss.str());
	// add callback task for access log start
	auto *operation_ptr = new std::string(operation);
	auto *ip_ptr = new std::string(ip_addr);
	rpc_task->add_callback([operation_ptr,ip_ptr](GRPCTask *task) {
		GRPCResp *resp = task->get_resp();
		apiUtil->write_access_log(*operation_ptr, *ip_ptr, resp->resp_code, resp->resp_msg);
		delete operation_ptr;
		delete ip_ptr;
	});
	// end
	if (operation.empty())
	{
		SLOG_DEBUG("unkown operation");
		response->Error(StatusOperationUndefined);
		return;
	}
	if (op_type == OP_CHECK)
	{
		check_task(request, response);
		return;
	}

	if (json_data.HasMember("username") == false || json_data.HasMember("password") == false)
	{
		response->Error(StatusParamIsIllegal, "username or password is empty");
		return;
	}
	std::string username = jsonParam(json_data, "username");
	std::string password = jsonParam(json_data, "password");
	std::string encryption = jsonParam(json_data, "encryption", "");
	std::string db_name =jsonParam(json_data, "db_name");
	// check username and password
	std::string checkidentityresult = apiUtil->check_indentity(username, password, encryption);
	if (checkidentityresult.empty() == false)
	{
		apiUtil->update_access_ip_error_num(ip_addr);
		response->Error(StatusAuthenticationFailed, checkidentityresult);
		return;
	}
	// check privilege
	if (apiUtil->check_privilege(username, operation, db_name) == 0)
	{
		std::string msg = "You have no " + operation + " privilege, operation failed";
		response->Error(StatusOperationConditionsAreNotSatisfied, msg);
		return;
	}
	// api operation
	switch (op_type)
	{
	case OP_LOGIN:
		login_task(request, response);
		break;
	case OP_TEST_CONNECT:
		test_connect_task(request, response);
		break;
	case OP_CORE_VERSION:
		core_version_task(request, response);
		break;
	case OP_IP_MANAGE:
		ip_manage_task(request, response, json_data);
		break;
	case OP_SHOW:
		show_task(request, response, json_data);
		break;
	case OP_LOAD:
		load_task(request, response, json_data);
		break;
	case OP_UNLOAD:
		unload_task(request, response, json_data);
		break;
	case OP_MONITOR:
		monitor_task(request, response, json_data);
		break;
	case OP_BUILD:
		build_task(request, response, json_data);
		break;
	case OP_DROP:
		drop_task(request, response, json_data);
		break;
	case OP_BACKUP:
		backup_task(request, response, json_data);
		break;
	case OP_RESTORE:
		restore_task(request, response, json_data);
		break;
	case OP_QUERY:
		query_task(request, response, json_data);
		break;
	case OP_EXPORT:
		export_task(request, response, json_data);
		break;
	case OP_BEGIN:
		begin_task(request, response, json_data);
		break;
	case OP_TQUERY:
		tquery_task(request, response, json_data);
		break;
	case OP_COMMIT:
		commit_task(request, response, json_data);
		break;
	case OP_ROLLBACK:
		rollback_task(request, response, json_data);
		break;
	case OP_CHECKPOINT:
		checkpoint_task(request, response, json_data);
		break;
	case OP_BATCH_INSERT:
		batch_insert_task(request, response, json_data);
		break;
	case OP_BATCH_REMOVE:
		batch_remove_task(request, response, json_data);
		break;
	case OP_USER_MANAGE:
		user_manage_task(request, response, json_data);
		break;
	case OP_USER_SHOW:
		user_show_task(request, response);
		break;
	case OP_USER_PRIVILEGE:
		user_privilege_task(request, response, json_data);
		break;
	case OP_USER_PASSWORD:
		user_password_task(request, response, json_data);
		break;
	case OP_TXN_LOG:
		txn_log_task(request, response, json_data);
		break;
	case OP_QUERY_LOG:
		query_log_task(request, response, json_data);
		break;
	case OP_QUERY_LOG_DATE:
		query_log_date_task(request, response);
		break;
	case OP_ACCESS_LOG:
		access_log_task(request, response, json_data);
		break;
	case OP_ACCESS_LOG_DATE:
		access_log_date_task(request, response);
		break;
	case OP_FUN_QUERY:
		fun_query_task(request, response, json_data);
		break;
	case OP_FUN_CUDB:
		fun_cudb_task(request, response, json_data);
		break;
	case OP_FUN_REVIEW:
		fun_review_task(request, response, json_data);
		break;
	default:
		SLOG_ERROR("Unkown operation, request body:\n" + request->body());
		response->Error(StatusOperationUndefined);
		break;
	}
}

/**
 * check the grpc server activity
 * 
 * @param request 
 * @param response 
 */
void check_task(const GRPCReq *request, GRPCResp *response)
{
	std::string success = "the grpc server is running...";
	response->Success(success);
}

/**
 * login grpc server
 * 
 * @param request 
 * @param response 
 */
void login_task(const GRPCReq *request, GRPCResp *response)
{
	try
	{
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "login successfully", allocator);
		string version = Util::getConfigureValue("version");
		resp_data.AddMember("CoreVersion", StringRef(version.c_str()), allocator);
		string licensetype = Util::getConfigureValue("licensetype");
		resp_data.AddMember("licensetype", StringRef(licensetype.c_str()), allocator);
		string cur_path = Util::get_cur_path();
		resp_data.AddMember("RootPath", StringRef(cur_path.c_str()), allocator);
		resp_data.AddMember("type", HTTP_TYPE, allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "login fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * login grpc server
 * 
 * @param request 
 * @param response
 */
void test_connect_task(const GRPCReq *request, GRPCResp *response)
{
	try
	{
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		string version = Util::getConfigureValue("version");
		resp_data.AddMember("CoreVersion", StringRef(version.c_str()), allocator);
		string licensetype = Util::getConfigureValue("licensetype");
		resp_data.AddMember("licensetype", StringRef(licensetype.c_str()), allocator);
		resp_data.AddMember("type", HTTP_TYPE, allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "Test connect fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}
/**
 * get core version
 * 
 * @param request 
 * @param response
 */
void core_version_task(const GRPCReq *request, GRPCResp *response)
{
	try
	{
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		string version = Util::getConfigureValue("version");
		resp_data.AddMember("CoreVersion", StringRef(version.c_str()), allocator);
		resp_data.AddMember("type", HTTP_TYPE, allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "Get core version fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * IP manage
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {type: "1 for query, 2 for save", ip_type: "1 for black list, 2 for white list", ips:"ip list, split with ','"}
 */
void ip_manage_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string error;
		string type = json_data["type"].GetString();
		if (type == "1")
		{
			string IPtype = apiUtil->ip_enabled_type();
			if (IPtype == "3")
			{
				error = "please configure ip_deny_path or ip_allow_path in the conf.ini file first.";
				response->Error(StatusOperationFailed, error);
				return;
			}
			vector<string> ip_list = apiUtil->ip_list(IPtype);
			size_t count = ip_list.size();
			stringstream str_stream;
			str_stream << "[";
			for (size_t i = 0; i < count; i++)
			{
				if (i > 0)
				{
					str_stream << ",";
				}
				str_stream << "\"" << ip_list[i] << "\"";
			}
			str_stream << "]";
			Json listDoc;
			listDoc.SetArray();
			listDoc.Parse(str_stream.str().c_str());
			
			Json responseBody;
			responseBody.SetObject();
			responseBody.AddMember("ip_type", StringRef(IPtype.c_str()), responseBody.GetAllocator());
			responseBody.AddMember("ips", listDoc, responseBody.GetAllocator());

			Json resp_data;
			resp_data.SetObject();
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", "success", allocator);
			resp_data.AddMember("ResponseBody", responseBody, allocator);
			response->Json(resp_data);
		}
		else if (type == "2")
		{
			std::string ips = json_data["ips"].GetString();
			std::string ip_type = json_data["ip_type"].GetString();
			if (ips.empty())
			{
				std::string error = "the ips can't be empty";
				response->Error(StatusParamIsIllegal, error);
				return;
			}
			vector<string> ipVector;
			Util::split(ips, ",", ipVector);
			if (ip_type == "1" || ip_type == "2")
			{
				bool rt = apiUtil->ip_save(ip_type, ipVector);
				if (rt)
				{
					response->Success("success");
				}
				else
				{
					if (ip_type == "1")
					{
						error = "ip_deny_path is not configured, please configure it in the conf.ini file first.";
						response->Error(StatusOperationFailed, error);
					}
					else
					{
						error = "ip_allow_path is not configured, please configure it in the conf.ini file first.";
						response->Error(StatusOperationFailed, error);
					}
				}
			}
			else
			{
				error = "ip_type is invalid, please look up the api document.";
				response->Error(StatusParamIsIllegal, error);
			}
		}
		else
		{
			error = "type is invalid, please look up the api document.";
			response->Error(StatusParamIsIllegal, error);
		}
	}
	catch (const std::exception &e)
	{
		string error = "IP manger fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * show the all database list (except system database)
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {username: "the user who is the owner of database or has rights to access the database"}
 */
void show_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string username = json_data["username"].GetString();

		vector<struct DatabaseInfo *> array;
		apiUtil->get_already_builds(username, array);

		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		size_t count = array.size();
		std::string line;
		std::stringstream str_stream;
		str_stream << "[";
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			DatabaseInfo *dbInfo = array[i];
			line = dbInfo->toJSON();
			str_stream << line;
		}
		str_stream << "]";
		Json array_data;
		array_data.SetArray();
		line = str_stream.str();
		array_data.Parse(line.c_str());

		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Get the database list successfully!", allocator);
		resp_data.AddMember("ResponseBody", array_data, allocator);

		// set response status and message
		response->Json(resp_data);
	}
	catch (std::exception &e)
	{
		std::string error = "Show failed:" + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * load the database to memory.
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the name of database", csr: "load csr resource flag, default '0'"}
 */
void load_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (!apiUtil->check_already_build(db_name))
		{
			error = "The database [" + db_name + "] not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}

		Database *current_database = apiUtil->get_database(db_name);
		if (current_database == NULL)
		{
			if (!apiUtil->trywrlock_database(db_name))
			{
				error = "Unable to load due to loss of lock.";
				response->Error(StatusOperationFailed, error);
				return;
			}
			Database *current_database = new Database(db_name);
			SLOG_DEBUG("begin loading...");
			bool load_csr = false;
			if (jsonParam(json_data, "csr", "0") == "1")
			{
				load_csr = true;
			}
			// TODO progress notification
			bool rt  = current_database->load(load_csr);
			SLOG_DEBUG("end loading.");
			if (rt)
			{
				apiUtil->add_database(db_name, current_database);
				// todo insert txn
				if (apiUtil->insert_txn_managers(current_database, db_name) == false)
				{
					SLOG_WARN("when load insert_txn_managers fail.");
				}
				std::string csr_str = "0";
				if (current_database->csr != NULL)
				{
					csr_str = "1";
				}
				Json resp_data;
				resp_data.SetObject();
				Json::AllocatorType &allocator = resp_data.GetAllocator();
				resp_data.AddMember("StatusCode", 0, allocator);
				resp_data.AddMember("StatusMsg", "Database loaded successfully.", allocator);
				resp_data.AddMember("csr", StringRef(csr_str.c_str()), allocator);
				response->Json(resp_data);
			}
			else
			{
				error = "load failed: unknow error.";
				response->Error(StatusOperationFailed, error);
			}
			rt = apiUtil->unlock_database(db_name);
		}
		else
		{
			std::string csr_str = "0";
			if (current_database->csr != NULL)
			{
				csr_str = "1";
			}
			Json resp_data;
			resp_data.SetObject();
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", "The database already load yet.", allocator);
			resp_data.AddMember("csr", StringRef(csr_str.c_str()), allocator);
			response->Json(resp_data);
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "load fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}
/**
 * unload a database from memory
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the name of database"}
 */
void unload_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusOperationFailed, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "the database [" + db_name + "] not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "the database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		struct DatabaseInfo *db_info = apiUtil->get_databaseinfo(db_name);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "the operation can not been excuted due to loss of lock.";
			response->Error(StatusLossOfLock, error);
			return;
		}
		else
		{
			if (apiUtil->find_txn_managers(db_name) == false)
			{
				error = "transaction manager can not find the database";

				apiUtil->unlock_database_map();
				apiUtil->unlock_databaseinfo(db_info);
				response->Error(StatusTranscationManageFailed, error);
				return;
			}
			apiUtil->db_checkpoint(db_name);
			apiUtil->delete_from_databases(db_name);
			apiUtil->unlock_databaseinfo(db_info);

			response->Success("Database unloaded.");
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "Unload fail" + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * get the database monitor info 
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the name of database"}
 */
void monitor_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		// check the param value is legal or not.
		string error = apiUtil->check_param_value("db_name", db_name);

		if (error.empty() == false)
		{
			response->Error(StatusOperationFailed, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "the database [" + db_name + "] not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		Database *current_database = apiUtil->get_database(db_name);
		if (current_database == NULL)
		{
			error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		DatabaseInfo *database_info = apiUtil->get_databaseinfo(db_name);
		if (apiUtil->tryrdlock_databaseinfo(database_info) == false)
		{
			string error = "Unable to monitor due to loss of lock";
			response->Error(StatusLossOfLock, error);
			return;
		}
		std::string creator = database_info->getCreator();
		std::string time = database_info->getTime();
		apiUtil->unlock_databaseinfo(database_info);
		// /use JSON format to send message
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		resp_data.AddMember("database", StringRef(db_name.c_str()), allocator);
		resp_data.AddMember("creator", StringRef(creator.c_str()), allocator);
		resp_data.AddMember("builtTime", StringRef(time.c_str()), allocator);
		char tripleNumString[128];
		sprintf(tripleNumString, "%lld", current_database->getTripleNum());
		resp_data.AddMember("tripleNum", StringRef(tripleNumString), allocator);
		resp_data.AddMember("entityNum", current_database->getEntityNum(), allocator);
		resp_data.AddMember("literalNum", current_database->getLiteralNum(), allocator);
		resp_data.AddMember("subjectNum", current_database->getSubNum(), allocator);
		resp_data.AddMember("predicateNum", current_database->getPreNum(), allocator);
		resp_data.AddMember("connectionNum", apiUtil->get_connection_num(), allocator);

		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		std::string error = "Monitor fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * build the database
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the name of database that would build", db_path: "the data file path"}
 */
void build_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_path = jsonParam(json_data, "db_path");
		std::string error = apiUtil->check_param_value("db_path", db_path);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (db_path == apiUtil->get_system_path())
		{
			error = "You have no rights to access system files.";
			response->Error(StatusCheckPrivilegeFailed, error);
			return;
		}
		if (Util::file_exist(db_path) == false)
		{
			error = "RDF file not exist.";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string db_name = jsonParam(json_data, "db_name");
		error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}

		// check if database named [db_name] is already built
		if (apiUtil->check_db_exist(db_name))
		{
			error = "database already built.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		// check databse number
		if (apiUtil->check_db_count() == false)
		{
			string error = "The total number of databases more than max_databse_num.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		Socket socket;
		string dataset = db_path;
		string database = db_name;
		SLOG_DEBUG("Import dataset to build database...");
		SLOG_DEBUG("DB_store: " + database + "\tRDF_data: " + dataset);
		Database *current_database = new Database(database);
		// TODO progress notification
		bool flag = current_database->build(dataset);
		delete current_database;
		current_database = NULL;
		if (!flag)
		{
			error = "Import RDF file to database failed.";
			std::string cmd = "rm -r " + database + ".db";
			system(cmd.c_str());
			response->Error(StatusOperationFailed, error);
			return;
		}

		ofstream f;
		f.open("./" + database + ".db/success.txt");
		f.close();

		// by default, one can query or load or unload the database that is built by itself, so add the database name to the privilege set of the user
		std::string username = jsonParam(json_data, "username");
		if (apiUtil->add_privilege(username, "query", db_name) == 0 ||
			apiUtil->add_privilege(username, "load", db_name) == 0 ||
			apiUtil->add_privilege(username, "unload", db_name) == 0 ||
			apiUtil->add_privilege(username, "backup", db_name) == 0 ||
			apiUtil->add_privilege(username, "restore", db_name) == 0 ||
			apiUtil->add_privilege(username, "export", db_name) == 0)
		{
			error = "add query or load or unload privilege failed.";
			response->Error(StatusAddPrivilegeFaied, error);
			return;
		}
		SLOG_DEBUG("add query and load and unload privilege succeed after build.");

		// add database information to system.db
		if (apiUtil->build_db_user_privilege(db_name, username))
		{
			response->Success("Import RDF file to database done.");
			Util::add_backuplog(db_name);
		}
		else
		{
			error = "add database information to system failed.";
			response->Error(StatusOperationFailed, error);
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "Build fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * drop a database
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the name of database that would drop", "is_backup": "'true' for logic delete, 'false' for physically delete"}
 */
void drop_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string is_backup = jsonParam(json_data, "is_backup", "true");
		std::string error = apiUtil->check_param_value("db_name", db_name);

		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "the database [" + db_name + "] not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		struct DatabaseInfo *db_info = apiUtil->get_databaseinfo(db_name);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "the operation can not been excuted due to loss of lock.";
			response->Error(StatusLossOfLock, error);
		}
		else
		{
			if (apiUtil->check_already_load(db_name))
			{
				//@ the database has loaded, unload it firstly
				apiUtil->delete_from_databases(db_name);
				SLOG_DEBUG("remove " + db_name + " from loaded database list");
			}
			apiUtil->unlock_databaseinfo(db_info);
			//@ delete the database info from the system database
			bool rt = apiUtil->delete_from_already_build(db_name);
			if (!rt)
			{
				SLOG_DEBUG("remove " + db_name + " from the already build database list fail.");
				error = "the operation can not been excuted due to loss of lock.";
				response->Error(StatusLossOfLock, error);
				return;
			}
			SLOG_DEBUG("remove " + db_name + " from the already build database list success.");

			std::string cmd;
			if (is_backup == "false")
				cmd = "rm -r " + db_name + ".db";
			else
				cmd = "mv " + db_name + ".db " + db_name + ".bak";
			SLOG_DEBUG(cmd);
			system(cmd.c_str());
			Util::delete_backuplog(db_name);
			string success = "Database " + db_name + " dropped.";
			response->Success(success);
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "Drop fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * backup a database
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the name of database that would backup", "backup_path": "the backup path, default path ./backups"}
 */
void backup_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string backup_path = jsonParam(json_data, "backup_path");
		std::string error = apiUtil->check_param_value("db_name", db_name);

		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "the database [" + db_name + "] not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		struct DatabaseInfo *db_info = apiUtil->get_databaseinfo(db_name);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "the operation can not been excuted due to loss of lock.";
			response->Error(StatusLossOfLock, error);
			return;
		}
		// begin backup database
		string path = backup_path;
		if (path.empty())
		{
			path = apiUtil->get_backup_path();
			SLOG_DEBUG("backup_path is empty, set to default path: " + path);
		}
		if (path == ".")
		{
			string error = "Failed to backup the database. Backup Path Can not be root or empty.";
			apiUtil->unlock_databaseinfo(db_info);
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (path[path.length() - 1] == '/')
		{
			path = path.substr(0, path.length() - 1);
		}
		SLOG_DEBUG("backup path: " + path);
		string db_path = db_name + ".db";
		apiUtil->rw_wrlock_database_map();
		int ret = apiUtil->db_copy(db_path, path);
		apiUtil->unlock_database_map();
		string timestamp = "";
		if (ret == 1)
		{
			string error = "Failed to backup the database. Database Folder Misssing.";
			apiUtil->unlock_databaseinfo(db_info);
			response->Error(StatusOperationFailed, error);
		}
		else
		{
			timestamp = Util::get_timestamp();
			path = path + "/" + db_path;
			string _path = path + "_" + timestamp;
			string sys_cmd = "mv " + path + " " + _path;
			system(sys_cmd.c_str());

			SLOG_DEBUG("database backup done: " + db_name);
			string success = "Database backup successfully.";
			apiUtil->unlock_databaseinfo(db_info);

			Document resp_data;
			resp_data.SetObject();
			Document::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
			resp_data.AddMember("backupfilepath", StringRef(_path.c_str()), allocator);
			response->Json(resp_data);
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "Backup fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * restore the database
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", backup_path: "the backup path", username: "the operation username"}
 */
void restore_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string backup_path = jsonParam(json_data, "backup_path");
		std::string username = jsonParam(json_data, "username");

		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string path = backup_path;
		if (path[path.length() - 1] == '/')
		{
			path = path.substr(0, path.length() - 1);
		}
		SLOG_DEBUG("backup path:" + path);
		if (Util::dir_exist(path) == false)
		{
			error = "Backup path not exist, restore failed.";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		string database = db_name;
		SLOG_DEBUG("restore database:" + database);
		if (apiUtil->check_already_build(db_name) == false)
		{
			error = "Database not built yet. Rebuild Now";
			string time = Util::get_backup_time(path, db_name);
			if (time.size() == 0)
			{
				error = "Backup path does not match database name, restore failed";
				response->Error(StatusParamIsIllegal, error);
				return;
			}
			if (apiUtil->add_privilege(username, "query", db_name) == 0 ||
				apiUtil->add_privilege(username, "load", db_name) == 0 ||
				apiUtil->add_privilege(username, "unload", db_name) == 0 ||
				apiUtil->add_privilege(username, "backup", db_name) == 0 ||
				apiUtil->add_privilege(username, "restore", db_name) == 0 ||
				apiUtil->add_privilege(username, "export", db_name) == 0)
			{
				error = "add query or load or unload or backup or restore or export privilege failed.";
				response->Error(StatusAddPrivilegeFaied, error);
				return;
			}
			if (apiUtil->build_db_user_privilege(db_name, username))
			{
				Util::add_backuplog(db_name);
			}
			else
			{
				error = "Database not built yet. Rebuild failed.";
				response->Error(StatusOperationFailed, error);
				return;
			}
		}
		struct DatabaseInfo *db_info = apiUtil->get_databaseinfo(db_name);

		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "Unable to restore due to loss of lock";
			response->Error(StatusLossOfLock, error);
			return;
		}

		// restore
		std::string sys_cmd = "rm -rf " + db_name + ".db";
		std::system(sys_cmd.c_str());

		// TODO why need lock the database_map?
		// apiUtil->trywrlock_database_map();
		int ret = apiUtil->db_copy(path, apiUtil->get_Db_path());
		// apiUtil->unlock_database_map();

		if (ret == 1)
		{
			error = "Failed to restore the database. Backup path error";
			apiUtil->unlock_databaseinfo(db_info);
			response->Error(StatusOperationFailed, error);
		}
		else
		{
			// update the in system.db
			path = Util::get_folder_name(path, db_name);
			sys_cmd = "mv " + path + " " + db_name + ".db";
			std::system(sys_cmd.c_str());
			apiUtil->unlock_databaseinfo(db_info);
			std::string success = "Database " + db_name + " restore successfully.";
			response->Success(success);
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "Restore fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * query the database.
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", format: "json/html/file", sparql: "the sparql"}
 */
void query_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string sparql = jsonParam(json_data, "sparql");
		std::string format = jsonParam(json_data, "format", "json");
		std::string username = jsonParam(json_data, "username");
		// check db_name paramter
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		// check sparql paramter
		error = apiUtil->check_param_value("sparql", sparql);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		string thread_id = Util::getThreadID();
		Database *current_database;
		bool update_flag_bool = true;
		if (apiUtil->check_privilege(username, "update", db_name) == 0)
		{
			update_flag_bool = false;
		}
		try
		{
			// check database exist
			if (apiUtil->check_db_exist(db_name) == false)
			{
				error = "Database not build yet.";
				response->Error(StatusOperationConditionsAreNotSatisfied, error);
				return;
			}
			// check database load status
			current_database = apiUtil->get_database(db_name);
			if (current_database == NULL)
			{
				error = "Database not load yet.";
				response->Error(StatusOperationConditionsAreNotSatisfied, error);
				return;
			}
			bool lock_rt = apiUtil->rdlock_database(db_name);
			if (lock_rt)
			{
				SLOG_DEBUG("get current database read lock success: " + db_name);
			}
			else
			{
				error = "get current database read lock fail.";
				response->Error(StatusLossOfLock, error);
				return;
			}
		}
		catch (const std::exception &e)
		{
			error = "Query fail: " + string(e.what());
			response->Error(StatusOperationFailed, error);
			return;
		}

		FILE *output = NULL;

		ResultSet rs;
		int ret_val;
		int query_time = Util::get_cur_time();

		// set query_start_time
		std::string query_start_time;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		int s = tv.tv_usec / 1000;
		int y = tv.tv_usec % 1000;
		query_start_time = Util::get_date_time() + ":" + Util::int2string(s) + "ms" + ":" + Util::int2string(y) + "microseconds";
		try
		{
			SLOG_DEBUG("begin query...\n" + sparql);
			rs.setUsername(username);
			ret_val = current_database->query(sparql, rs, output, update_flag_bool, false, nullptr);
			query_time = Util::get_cur_time() - query_time;
		}
		catch (const std::exception &e)
		{
			error = "Query fail: " + string(e.what());
			apiUtil->unlock_database(db_name);
			response->Error(StatusOperationFailed, error);
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

		string filename = thread_id + "_" + Util::getTimeString2() + "_" + Util::int2string(Util::getRandNum()) + ".txt";
		string localname = apiUtil->get_query_result_path() + filename;
		if (ret)
		{
			// SLOG_DEBUG(thread_id + ":search query returned successfully.");

			// record each query operation, including the sparql and the answer number
			// accurate down to microseconds
			// filter the IP from the test server
			std::string remote_ip = jsonParam(json_data, "remote_ip");
			long rs_ansNum = max((long)rs.ansNum - rs.output_offset, 0L);
			long rs_outputlimit = (long)rs.output_limit;
			if (rs_outputlimit != -1)
				rs_ansNum = min(rs_ansNum, rs_outputlimit);
			// if (remote_ip != TEST_IP)
			// {
			int status_code = 0;
			string file_name = "";
			if (format.find("file") != string::npos)
			{
				file_name = string(filename.c_str());
			}
			// add callback task for query log start
			struct DBQueryLogInfo* query_log_info = new DBQueryLogInfo(query_start_time, remote_ip, sparql, rs_ansNum, format, file_name, status_code, query_time, db_name);
			auto * query_log_task = task_of(response);
			query_log_task->add_callback([query_log_info](GRPCTask *) {
				apiUtil->write_query_log(query_log_info);
				delete query_log_info;
			});
			// end
			// }

			// to void someone downloading all the data file by sparql query on purpose and to protect the data
			// if the ansNum too large, for example, larger than 100000, we limit the return ans.
			if (rs_ansNum > apiUtil->get_max_output_size())
			{
				if (rs_outputlimit == -1 || rs_outputlimit > apiUtil->get_max_output_size())
				{
					rs_outputlimit = apiUtil->get_max_output_size();
				}
			}
			string query_time_s = Util::int2string(query_time);

			ofstream outfile;
			string ans = "";
			string success = rs.to_JSON();
			// TODO: if result is stored in Stream instead of memory?  (if out of memory to use to_str)
			// BETTER: divide and transfer, in multiple times, getNext()
			if (format == "json")
			{
				Json resp_data;
				Json::AllocatorType &allocator = resp_data.GetAllocator();
				/* code */
				resp_data.Parse(success.c_str());
				if (resp_data.HasParseError())
				{
					string filename2 = "error_" + filename;
					string localname2 = apiUtil->get_query_result_path() + filename2;
					outfile.open(localname2);
					outfile << success;
					outfile.close();
					SLOG_ERROR("result parse error:\n" + localname2);
					error = "Query fail: the result parse error.";
					response->Error(StatusOperationFailed, error);
				}
				else
				{
					resp_data.AddMember("StatusCode", 0, allocator);
					resp_data.AddMember("StatusMsg", "success", allocator);
					resp_data.AddMember("AnsNum", rs_ansNum, allocator);
					resp_data.AddMember("OutputLimit", rs_outputlimit, allocator);
					resp_data.AddMember("ThreadId", StringRef(thread_id.c_str()), allocator);
					resp_data.AddMember("QueryTime", StringRef(query_time_s.c_str()), allocator);

					response->set_header_pair("Cache-Control", "no-cache");
					response->set_header_pair("Pragma", "no-cache");
					response->set_header_pair("Expires", "0");
					response->Json(resp_data);
				}
			}
			else if (format == "file")
			{
				outfile.open(localname);
				outfile << success;
				outfile.close();

				Json resp_data;
				resp_data.SetObject();
				Json::AllocatorType &allocator = resp_data.GetAllocator();
				resp_data.AddMember("StatusCode", 0, allocator);
				resp_data.AddMember("StatusMsg", "success", allocator);
				resp_data.AddMember("AnsNum", rs_ansNum, allocator);
				resp_data.AddMember("OutputLimit", rs_outputlimit, allocator);
				resp_data.AddMember("ThreadId", StringRef(thread_id.c_str()), allocator);
				resp_data.AddMember("QueryTime", StringRef(query_time_s.c_str()), allocator);
				resp_data.AddMember("FileName", StringRef(filename.c_str()), allocator);

				response->set_header_pair("Cache-Control", "no-cache");
				response->set_header_pair("Pragma", "no-cache");
				response->set_header_pair("Expires", "0");
				response->Json(resp_data);
			}
			else if (format == "json+file" || format == "file+json")
			{
				outfile.open(localname);
				outfile << success;
				outfile.close();

				Json resp_data;
				Json::AllocatorType &allocator = resp_data.GetAllocator();
				/* code */
				resp_data.Parse(success.c_str());
				if (resp_data.HasParseError())
				{
					string filename2 = "error_" + filename;
					string localname2 = apiUtil->get_query_result_path() + filename2;
					outfile.open(localname2);
					outfile << success;
					outfile.close();
					SLOG_ERROR("result parse error:\n" + localname2);
					error = "Query fail: the result parse error.";
					response->Error(StatusOperationFailed, error);
				}
				else
				{
					resp_data.AddMember("StatusCode", 0, allocator);
					resp_data.AddMember("StatusMsg", "success", allocator);
					resp_data.AddMember("AnsNum", rs_ansNum, allocator);
					resp_data.AddMember("OutputLimit", rs_outputlimit, allocator);
					resp_data.AddMember("ThreadId", StringRef(thread_id.c_str()), allocator);
					resp_data.AddMember("QueryTime", StringRef(query_time_s.c_str()), allocator);
					resp_data.AddMember("FileName", StringRef(filename.c_str()), allocator);

					response->set_header_pair("Cache-Control", "no-cache");
					response->set_header_pair("Pragma", "no-cache");
					response->set_header_pair("Expires", "0");
					response->Json(resp_data);
				}
			}
			else if (format == "sparql-results+json")
			{
				response->set_header_pair("Content-Type", "application/sparql-results+json");
				response->set_header_pair("Cache-Control", "no-cache");
				response->set_header_pair("Pragma", "no-cache");
				response->set_header_pair("Expires", "0");

				response->String(success);
			}
			else
			{
				error = "Unkown result format.";
				response->Error(StatusOperationFailed, error);
			}
		}
		else if (update)
		{
			SLOG_DEBUG("update query returns true.");
			response->Success("update query returns true.");	
		}
		else
		{
			error = "search query returns false.";
			SLOG_DEBUG(error);
			response->Error(StatusOperationFailed, error);
		}
		apiUtil->unlock_database(db_name);
		SLOG_DEBUG("query complete!");
	}
	catch (const std::exception &e)
	{
		std::string error = "Query fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * export the database
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", "db_path": "the export path"}
 */
void export_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string db_path = jsonParam(json_data, "db_path");
		error = apiUtil->check_param_value("db_path", db_path);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not build yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		if (db_path[db_path.length() - 1] != '/')
		{
			db_path = db_path + "/";
		}
		if (Util::dir_exist(db_path) == false)
		{
			Util::create_dirs(db_path);
		}
		db_path = db_path + db_name + "_" + Util::get_timestamp() + ".nt";
		// check if database named [db_name] is already load
		Database *current_database = apiUtil->get_database(db_name);
		if (current_database == NULL)
		{
			string error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		apiUtil->rdlock_database(db_name); // lock database
		SLOG_DEBUG("export_path: " + db_path);
		FILE *ofp = fopen(db_path.c_str(), "w");
		current_database->export_db(ofp);
		fflush(ofp);
		fclose(ofp);
		ofp = NULL;
		current_database = NULL;
		apiUtil->unlock_database(db_name); // unlock
		std::string success = "Export the database successfully.";

		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
		resp_data.AddMember("filepath", StringRef(db_path.c_str()), allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		std::string error = "Export fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * begin a transaction
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", isolevel: "the Isolation level， 1:RC(read committed) 2:SI(snapshot isolation) 3:SR(seriablizable）"}
 */
void begin_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string isolevel = jsonParam(json_data, "isolevel");
		error = apiUtil->check_param_value("isolevel", isolevel);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		int level = Util::string2int(isolevel);
		if (level <= 0 || level > 3)
		{
			error = "The isolation level's value only can been 1/2/3";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		if (apiUtil->get_Txn_ptr(db_name) == NULL)
		{
			error = "Database transaction manager error.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		std::string username = jsonParam(json_data, "username");
		string TID_s = apiUtil->begin_process(db_name, level, username);
		if (TID_s.empty())
		{
			error = "Transaction begin failed.";
			response->Error(StatusTranscationManageFailed, error);
			return;
		}
		StringBuffer s;
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Transaction begin success", allocator);
		resp_data.AddMember("TID", StringRef(TID_s.c_str()), allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		std::string error = "Transaction begin fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * query database with transcation
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", sparql: "the sparql", tid: "the transcation id while begin transcation"}
 */
void tquery_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string TID_s = jsonParam(json_data, "tid");
		error = apiUtil->check_param_value("tid", TID_s);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}

		txn_id_t TID;
		TID = apiUtil->check_txn_id(TID_s);
		if (TID == (unsigned long long)0)
		{
			error = "TID "+TID_s+" is not a pure number.";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string sparql = jsonParam(json_data, "sparql");
		error = apiUtil->check_param_value("sparql", sparql);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		auto txn_m = apiUtil->get_Txn_ptr(db_name);
		if (txn_m == NULL)
		{
			error = "Get database transaction manager error.";
			response->Error(StatusTranscationManageFailed, error);
			return;
		}
		SLOG_DEBUG("tquery sparql: " + sparql);
		std::string res;
		int ret = txn_m->Query(TID, sparql, res);
		if (ret == -1)
		{
			error = "Transaction query failed due to wrong TID";
			response->Error(StatusOperationFailed, error);
		}
		else if (ret == -10)
		{
			error = "Transaction query failed due to wrong database status";
			apiUtil->rollback_process(txn_m, TID);
			response->Error(StatusOperationFailed, error);
		}
		else if (ret == -99)
		{
			error = "Transaction query failed. This transaction is not in running status!";
			apiUtil->rollback_process(txn_m, TID);
			response->Error(StatusOperationFailed, error);
		}
		else if (ret == -100)
		{
			Json resp_data;
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.Parse(res.c_str());
			if (resp_data.HasParseError())
			{
				SLOG_ERROR("tquery result parse error.\n" + res);
				resp_data.Parse("{}");
				resp_data.AddMember("result", StringRef(res.c_str()), allocator);
			}
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", "success", allocator);
			response->Json(resp_data);
		}
		else if (ret == -20)
		{
			error = "Transaction query failed. This transaction is set abort due to conflict!";
			apiUtil->rollback_process(txn_m, TID);
			response->Error(StatusOperationFailed, error);
		}
		else if (ret == -101)
		{
			error = "Transaction query failed. Unknown query error";
			response->Error(StatusOperationFailed, error);
		}
		else
		{
			string success = "Transaction query success, update num: " + Util::int2string(ret);
			response->Success(success);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Transaction query fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * commit a transcation
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", tid: "the transcation id while begin transcation"}
 */
void commit_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string TID_s = jsonParam(json_data, "tid");
		error = apiUtil->check_param_value("TID", TID_s);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		auto TID = apiUtil->check_txn_id(TID_s);
		if (TID == (unsigned long long)0)
		{
			error = "TID " + TID_s + " is not a pure number.";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		Database *current_database = apiUtil->get_database(db_name);
		if (current_database == NULL)
		{
			error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		auto txn_m = apiUtil->get_Txn_ptr(db_name);
		if (txn_m == NULL)
		{
			error = "Get database transaction manager error.";
			response->Error(StatusTranscationManageFailed, error);
			return;
		}
		int ret = txn_m->Commit(TID);
		if (ret == -1)
		{
			error = "Transaction not found, commit failed. TID: " + TID_s;
			response->Error(StatusOperationFailed, error);
		}
		else if (ret == 1)
		{
			error = "Transaction not in running state! commit failed. TID: " + TID_s;
			apiUtil->rollback_process(txn_m, TID);
			response->Error(StatusOperationFailed, error);
		}
		else
		{
			apiUtil->commit_process(txn_m, TID);
			// TODO auto checkpoint are sometimes blocked
			// auto latest_tid = txn_m->find_latest_txn();
			// SLOG_DEBUG("latest TID: "+ to_string(latest_tid));
			// if (latest_tid == 0)
			// {
			// 	SLOG_DEBUG("This is latest TID, auto checkpoint and save.");
			// 	txn_m->Checkpoint();
			// 	SLOG_DEBUG("Transaction checkpoint done.");
			// 	if (apiUtil->trywrlock_database(db_name))
			// 	{
			// 		current_database->save();
			// 		apiUtil->unlock_database(db_name);
			// 	}
			// 	else
			// 	{
			// 		SLOG_ERROR("The save operation can not been excuted due to loss of lock.");
			// 	}
			// }
			string success = "Transaction commit success. TID: " + TID_s;
			response->Success(success);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Transaction commit fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * rollback a transcation
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", tid: "the transcation id while begin transcation"}
 */
void rollback_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string TID_s = jsonParam(json_data, "tid");
		error = apiUtil->check_param_value("TID", TID_s);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		auto TID = apiUtil->check_txn_id(TID_s);
		if (TID == (unsigned long long)0)
		{
			error = "TID " + TID_s + " is not a pure number.";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		Database *current_database = apiUtil->get_database(db_name);
		if (current_database == NULL)
		{
			error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		auto txn_m = apiUtil->get_Txn_ptr(db_name);
		if (txn_m == NULL)
		{
			error = "Get database transaction manager error.";
			response->Error(StatusTranscationManageFailed, error);
			return;
		}
		int ret = txn_m->Rollback(TID);
		if (ret == 1)
		{
			error = "Transaction not in running state! rollback failed. TID: " + TID_s;
			response->Error(StatusOperationFailed, error);
		}
		else if (ret == -1)
		{
			error = "Transaction not found, rollback failed. TID: " + TID_s;
			response->Error(StatusOperationFailed, error);
		}
		else
		{
			apiUtil->rollback_process(txn_m, TID);
			string success = "Transaction rollback success. TID: " + TID_s;
			response->Success(success);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Transaction rollback fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * this is used for checkpoint, we must ensure that modification is written to disk
 * 
 * @param request 
 * @param response 
 * @param json_data 
 */
void checkpoint_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		Database *current_database = apiUtil->get_database(db_name);
		if (current_database == NULL)
		{
			error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		if (apiUtil->trywrlock_database(db_name) == false)
		{
			error = "The operation can not been excuted due to loss of lock.";
			response->Error(StatusLossOfLock, error);
		}
		else
		{
			auto txn_m = apiUtil->get_Txn_ptr(db_name);
			if (txn_m == NULL)
			{
				error = "Get database transaction manager error.";
				apiUtil->unlock_database(db_name);
				response->Error(StatusTranscationManageFailed, error);
			}
			else
			{
				txn_m->Checkpoint();
				current_database->save();
				apiUtil->unlock_database(db_name);
				string success = "Database saved successfully.";
				response->Success(success);
			}
		}
	}
	catch (const std::exception &e)
	{
		string error = "Checkpoint fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}

}

/**
 * batch insert data
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", file: "the insert data file"}
 */
void batch_insert_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string file = jsonParam(json_data, "file");
		error = apiUtil->check_param_value("file", file);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (Util::file_exist(file) == false)
		{
			error = "The data file is not exist";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		Database *current_database = apiUtil->get_database(db_name);
		if (apiUtil->trywrlock_database(db_name) == false)
		{
			error = "The operation can not been excuted due to loss of lock.";
			response->Error(StatusLossOfLock, error);
		}
		else
		{
			string success = "Batch insert data successfully.";
			unsigned success_num = current_database->batch_insert(file, false, nullptr);
			current_database->save();
			apiUtil->unlock_database(db_name);

			Json resp_data;
			resp_data.SetObject();
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
			resp_data.AddMember("success_num", StringRef(Util::int2string(success_num).c_str()), allocator);
			response->Json(resp_data);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Batch insert fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * batch remove data
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", file: "the insert data file"}
 */
void batch_remove_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string file = jsonParam(json_data, "file");
		error = apiUtil->check_param_value("file", file);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (Util::file_exist(file) == false)
		{
			error = "The data file is not exist";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		Database *current_database = apiUtil->get_database(db_name);
		if (apiUtil->trywrlock_database(db_name) == false)
		{
			error = "The operation can not been excuted due to loss of lock.";
			response->Error(StatusLossOfLock, error);
		}
		else
		{
			string success = "Batch remove data successfully.";
			unsigned success_num = current_database->batch_remove(file, false, nullptr);
			current_database->save();
			apiUtil->unlock_database(db_name);

			Json resp_data;
			resp_data.SetObject();
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
			resp_data.AddMember("success_num", StringRef(Util::int2string(success_num).c_str()), allocator);
			response->Json(resp_data);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Batch remove fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

// for user

/**
 * user manager (include add user,delete user,alter user),notice that only support alter user's password
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {type: "1 for add user, 2 for delete user, 3 for alert user password", op_username: "the user who be operated", op_password: "new password(for type 3)"}
 */
void user_manage_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string op_username = jsonParam(json_data, "op_username");
		std::string error = apiUtil->check_param_value("op_username", op_username);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string op_password = jsonParam(json_data, "op_password");
		std::string type = jsonParam(json_data, "type");
		if (type != "2")
		{
			error = apiUtil->check_param_value("op_password", op_password);
			if (error.empty() == false)
			{
				response->Error(StatusParamIsIllegal, error);
				return;
			}
		}
		
		if (type == "1") // add user
		{
			// check user number
			if (apiUtil->check_user_count() == false)
			{
				string error = "The total number of users more than max_user_num.";
				response->Error(StatusOperationConditionsAreNotSatisfied, error);
				return;
			}
			if (apiUtil->user_add(op_username, op_password))
			{
				response->Success("Add user done.");
			}
			else
			{
				error = "Username already existed, add user failed.";
				response->Error(StatusOperationFailed, error);
			}
		}
		else if (type == "2") // delete user
		{
			if (op_username == apiUtil->get_root_username())
			{
				error = "You cannot delete root, delete user failed.";
				response->Error(StatusOperationFailed, error);
			}
			else if (apiUtil->user_delete(op_username))
			{
				response->Success("Delete user done.");
			}
			else
			{
				error = "Username not exist, delete user failed.";
				response->Error(StatusOperationFailed, error);
			}
		}
		else if (type == "3") // alert password
		{
			if (apiUtil->user_pwd_alert(op_username, op_password))
			{
				response->Success("Change password done.");
			}
			else
			{
				error = "Username not exist, change password failed.";
				response->Error(StatusOperationFailed, error);
			}
		}
		else
		{
			error = "The operation is not support.";
			response->Error(StatusParamIsIllegal, error);
		}
	}
	catch (const std::exception &e)
	{
		string error = "User manage fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * show all user list
 * 
 * @param request 
 * @param response 
 */
void user_show_task(const GRPCReq *request, GRPCResp *response)
{
	try
	{
		vector<struct DBUserInfo *> userList;
		apiUtil->get_user_info(&userList);
		if (userList.empty())
		{
			response->Success("No Users");
			return;
		}
		size_t count = userList.size();
		std::stringstream str_stream;
		str_stream << "[";
		std::string line;
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			line = userList[i]->toJSON();
			str_stream << line;
		}
		str_stream << "]";
		Json array_data;
		array_data.SetArray();
		line = str_stream.str();
		array_data.Parse(line.c_str());

		Json resp_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.SetObject();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		resp_data.AddMember("ResponseBody", array_data, allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "Show user fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * manage the user privilege, include add user privilege, delete user privilege
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {
 *   type: "1 for add privilege, 2 for delete privilege, 3 for clear privilege", 
 *   op_username: "the user who be operated",
 *   privileges: "1:query,2:load,3:unload,4:update,5:backup,6:restore,7:export(multi privileges by using ',' to split)",
 *   db_name: "the operation database name"
 * }
 */
void user_privilege_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string type = jsonParam(json_data, "type");
		std::string error = apiUtil->check_param_value("type", type);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}  
		else if (type != "1" && type != "2" && type != "3")
		{
			error = "The type " + type + " is not support.";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string op_username = jsonParam(json_data, "op_username");
		error = apiUtil->check_param_value("op_username", op_username);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		} 
		else if (apiUtil->check_user_exist(op_username) == false)
		{
			error = "The username is not exists.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		else if (op_username == apiUtil->get_root_username())
		{
			error = "You can't change privileges for root user.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		} 
		
		std::string db_name = jsonParam(json_data, "db_name");
		std::string privileges = jsonParam(json_data, "privileges");
		if (type != "3")
		{
			error = apiUtil->check_param_value("db_name", db_name);
			if (error.empty() == false)
			{
				response->Error(StatusParamIsIllegal, error);
				return;
			}
			// check database exist
			if (apiUtil->check_db_exist(db_name) == false)
			{
				error = "Database not build yet.";
				response->Error(StatusOperationConditionsAreNotSatisfied, error);
				return;
			}
			error = apiUtil->check_param_value("privileges", privileges);
			if (error.empty() == false)
			{
				response->Error(StatusParamIsIllegal, error);
				return;
			}
		}
		
		if (type == "3")
		{
			// clear the user all privileges
			int resultint = apiUtil->clear_user_privilege(op_username);
			if (resultint == 1)
			{
				response->Success("Clear the all privileges for the user successfully!");
			}
			else
			{
				error = "Clear the all privileges for the user fail.";
				response->Error(StatusOperationFailed, error);
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
			std::stringstream result;
			for (unsigned i = 0; i < privilege_vector.size(); i++)
			{
				std::string temp_privilege_int = privilege_vector[i];
				std::string temp_privilege = "";
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
				else
				{
					SLOG_DEBUG("The privilege " + temp_privilege_int + " undefined.");
					continue;
				} 
				
				if (type == "1")
				{
					if (apiUtil->add_privilege(op_username, temp_privilege, db_name) == 0)
					{
						result << "add privilege " + temp_privilege + " failed. \r\n";
					}
					else
					{
						result << "add privilege " + temp_privilege + " successfully. \r\n";
					}
				}
				else if (type == "2")
				{
					if (apiUtil->del_privilege(op_username, temp_privilege, db_name) == 0)
					{
						result << "delete privilege " + temp_privilege + " failed. \r\n";
					}
					else
					{
						result << "delete privilege " + temp_privilege + " successfully. \r\n";
					}
				}
			}
			response->Success(result.str());
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "User privilege manage fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * change user password
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {op_password: "new password"}
 */
void user_password_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string op_password = jsonParam(json_data, "op_password");
		std::string error = apiUtil->check_param_value("op_password", op_password);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string username = jsonParam(json_data, "username");
		if (apiUtil->user_pwd_alert(username, op_password))
		{
			response->Success("Change password done.");
		}
		else
		{
			error = "Username not exist.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Change password fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}
// for log
/**
 * get transcation log
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {pageNo: "page number, default 1", pageSize: "number of per page, default 10"}
 */
void txn_log_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		// std::string username = jsonParam(json_data, "username");
		// if (username != apiUtil->get_root_username())
		// {
		// 	std::string error = "Root User Only!";
		// 	response->Error(StatusOperationConditionsAreNotSatisfied, error);
		// 	return;
		// }
		int page_no = jsonParam(json_data, "pageNo", 1);
		int page_size = jsonParam(json_data, "pageSize", 10);
		struct TransactionLogs transactionLogs;
		apiUtil->get_transactionlog(page_no, page_size, &transactionLogs);
		vector<struct TransactionLogInfo> logList = transactionLogs.getTransactionLogInfoList();
		size_t count = logList.size();
		std::string line = "";
		std::stringstream str_stream;
		str_stream << "[";
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			TransactionLogInfo log_info = logList[i];
			line = log_info.toJSON();
			str_stream << line;
		}
		str_stream << "]";

		Json resp_data;
		Json array_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.SetObject();
		array_data.SetArray();
		line = str_stream.str();
		array_data.Parse(line.c_str());

		int totalSize = transactionLogs.getTotalSize();
		int totalPage = transactionLogs.getTotalPage();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Get transaction log success.", allocator);
		resp_data.AddMember("totalSize", totalSize, allocator);
		resp_data.AddMember("totalPage", totalPage, allocator);
		resp_data.AddMember("pageNo", page_no, allocator);
		resp_data.AddMember("pageSize", page_size, allocator);
		resp_data.AddMember("list", array_data, allocator);

		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "Get transaction log fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * get query log
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {date:"log date, required format is yyyyMMdd", pageNo: "page number, default 1", pageSize: "number of per page, default 10"}
 */
void query_log_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string date = jsonParam(json_data, "date");
		std::string error = apiUtil->check_param_value("date", date);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		int page_no = jsonParam(json_data, "pageNo", 1);
		int page_size = jsonParam(json_data, "pageSize", 10);
		struct DBQueryLogs dbQueryLogs;
		apiUtil->get_query_log(date, page_no, page_size, &dbQueryLogs);
		vector<struct DBQueryLogInfo> logList = dbQueryLogs.getQueryLogInfoList();
		size_t count = logList.size();
		std::string line = "";
		std::stringstream str_stream;
		str_stream << "[";
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			DBQueryLogInfo log_info = logList[i];
			line = log_info.toJSON();
			str_stream << line;
		}
		str_stream << "]";
		Json resp_data;
		Json array_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.SetObject();
		array_data.SetArray();
		line = str_stream.str();
		array_data.Parse(line.c_str());
		int totalSize = dbQueryLogs.getTotalSize();
		int totalPage = dbQueryLogs.getTotalPage();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Get query log success", allocator);
		resp_data.AddMember("totalSize", totalSize, allocator);
		resp_data.AddMember("totalPage", totalPage, allocator);
		resp_data.AddMember("pageNo", page_no, allocator);
		resp_data.AddMember("pageSize", page_size, allocator);
		resp_data.AddMember("list", array_data, allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "Get query log fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * get query log date
 * 
 * @param request 
 * @param response 
 */
void query_log_date_task(const GRPCReq *request, GRPCResp *response)
{
	try
	{
		vector<string> logfiles;
		apiUtil->get_query_log_files(logfiles);
		sort(logfiles.begin(), logfiles.end(), [](const string& a, const string& b) {
			return a > b;
		});
		std::stringstream str_stream;
		str_stream << "[";
		size_t count = logfiles.size();
		std::string item;
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			item = logfiles[i];
			item = item.substr(0, item.length()-4); // file_name: yyyyMMdd.log
			str_stream << "\"" << item << "\"";
		}
		str_stream << "]";
		Json resp_data;
		Json array_data;
		resp_data.SetObject();
		array_data.SetArray();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		array_data.Parse(str_stream.str().c_str());
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Get query log date success", allocator);
		resp_data.AddMember("list", array_data, allocator);
		response->Json(resp_data);
	}
	catch(const std::exception& e)
	{
		string error = "Get query log date fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * get access log
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {date:"log date, required format is yyyyMMdd", pageNo: "page number, default 1", pageSize: "number of per page, default 10"}
 */
void access_log_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string date = jsonParam(json_data, "date");
		std::string error = apiUtil->check_param_value("date", date);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		int page_no = jsonParam(json_data, "pageNo", 1);
		int page_size = jsonParam(json_data, "pageSize", 10);
		struct DBAccessLogs dbAccessLogs;
		apiUtil->get_access_log(date, page_no, page_size, &dbAccessLogs);
		vector<struct DBAccessLogInfo> logList = dbAccessLogs.getAccessLogInfoList();
		size_t count = logList.size();
		std::string line = "";
		std::stringstream str_stream;
		str_stream << "[";
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			DBAccessLogInfo log_info = logList[i];
			line = log_info.toJSON();
			str_stream << line;
		}
		str_stream << "]";

		Json resp_data;
		Json array_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.SetObject();
		array_data.SetArray();
		line = str_stream.str();
		array_data.Parse(line.c_str());

		int totalSize = dbAccessLogs.getTotalSize();
		int totalPage = dbAccessLogs.getTotalPage();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Get access log success", allocator);
		resp_data.AddMember("totalSize", totalSize, allocator);
		resp_data.AddMember("totalPage", totalPage, allocator);
		resp_data.AddMember("pageNo", page_no, allocator);
		resp_data.AddMember("pageSize", page_size, allocator);
		resp_data.AddMember("list", array_data, allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "Get access log fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * get access log date
 * 
 * @param request 
 * @param response 
 */
void access_log_date_task(const GRPCReq *request, GRPCResp *response)
{
	try
	{
		vector<string> logfiles;
		apiUtil->get_access_log_files(logfiles);
		sort(logfiles.begin(), logfiles.end(), [](const string& a, const string& b) {
			return a > b;
		});
		std::stringstream str_stream;
		str_stream << "[";
		size_t count = logfiles.size();
		std::string item;
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			item = logfiles[i];
			item = item.substr(0, item.length()-4); // file_name: yyyyMMdd.log
			str_stream << "\"" << item << "\"";
		}
		str_stream << "]";
		Json resp_data;
		Json array_data;
		resp_data.SetObject();
		array_data.SetArray();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		array_data.Parse(str_stream.str().c_str());
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Get access log date success", allocator);
		resp_data.AddMember("list", array_data, allocator);
		response->Json(resp_data);
	}
	catch(const std::exception& e)
	{
		string error = "Get access log date fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

// for personalized function
void build_PFNInfo(rapidjson::Value &fun_info, struct PFNInfo &pfn_info)
{
	if (fun_info.HasMember("funName") && fun_info["funName"].IsString())
	{
		pfn_info.setFunName(fun_info["funName"].GetString());
	}
	if (fun_info.HasMember("funDesc") && fun_info["funDesc"].IsString())
	{
		pfn_info.setFunDesc(fun_info["funDesc"].GetString());
	}
	if (fun_info.HasMember("funArgs") && fun_info["funArgs"].IsString())
	{
		pfn_info.setFunArgs(fun_info["funArgs"].GetString());
	}
	if (fun_info.HasMember("funBody") && fun_info["funBody"].IsString())
	{
		pfn_info.setFunBody(fun_info["funBody"].GetString());
	}
	if (fun_info.HasMember("funSubs") && fun_info["funSubs"].IsString())
	{
		pfn_info.setFunSubs(fun_info["funSubs"].GetString());
	}
	if (fun_info.HasMember("funStatus") && fun_info["funStatus"].IsString())
	{
		pfn_info.setFunStatus(fun_info["funStatus"].GetString());
	}
	if (fun_info.HasMember("funReturn") && fun_info["funReturn"].IsString())
	{
		pfn_info.setFunReturn(fun_info["funReturn"].GetString());
	}
}

/**
 * query personalized function
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {
 *   funInfo:{
 *     funName: "search function name", 
 *     funStatus: "1:unbuild,2:builded,3:error"
 *   }
 * }
 */
void fun_query_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		struct PFNInfo pfn_info;
		if (hasJsonParam(json_data, "funInfo"))
		{
			rapidjson::Value &fun_info = json_data["funInfo"];
			build_PFNInfo(fun_info, pfn_info);
		}
		std::string username =  jsonParam(json_data, "username");
		struct PFNInfos *pfn_infos = new PFNInfos();
		apiUtil->fun_query(pfn_info.getFunName(), pfn_info.getFunStatus(), username, pfn_infos);
		vector<struct PFNInfo> list = pfn_infos->getPFNInfoList();
		size_t count = list.size();
		string line = "";
		stringstream str_stream;
		str_stream << "[";
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			PFNInfo pfn_info = list[i];
			line = pfn_info.toJSON();
			str_stream << line;
		}
		str_stream << "]";

		Json resp_data;
		Json array_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.SetObject();
		array_data.SetArray();
		line = str_stream.str();
		array_data.Parse(line.c_str());

		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		resp_data.AddMember("list", array_data, allocator);

		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		std::string error = "Fun query fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * personalized function manager, include create、update、delete and build
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * { 
 *   type: "1:create,2:update,3:delete", 
 *   funInfo: {
 *     funName: "function name",
 *     funDesc: "function description",
 *     funArgs: "1: no K-Hop param, 2: has K-Hop param",
 *     funBody: "function body",
 *     funSubs: "sub function",
 *     funReturn: "path/multipath/kv/value"
 *    }
 * }
 */
void fun_cudb_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	std::string type = jsonParam(json_data, "type");
	std::string error = apiUtil->check_param_value("type", type);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	if (hasJsonParam(json_data, "funInfo") == false)
	{
		error = "the value of funInfo can not be empty!";
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	std::string username = jsonParam(json_data, "username");
	struct PFNInfo pfn_info;
	rapidjson::Value &fun_info = json_data["funInfo"];
	build_PFNInfo(fun_info, pfn_info);
	if (type == "1")
	{
		try
		{
			apiUtil->fun_create(username, &pfn_info);
			response->Success("Function create success.");
		}
		catch(const std::exception& e)
		{
			std::string error = "Function create fail: " + string(e.what());
			response->Error(StatusOperationFailed, error);
		}
	}
	else if (type == "2")
	{
		try
		{
			apiUtil->fun_update(username, &pfn_info);
			response->Success("Function update success.");
		}
		catch(const std::exception& e)
		{
			std::string error = "Function update fail: " + string(e.what());
			response->Error(StatusOperationFailed, error);
		}
	}
	else if (type == "3")
	{
		try
		{
			apiUtil->fun_delete(username, &pfn_info);
			response->Success("Function delete success.");
		}
		catch(const std::exception& e)
		{
			std::string error = "Function delete fail: " + string(e.what());
			response->Error(StatusOperationFailed, error);
		}
	}
	else if (type == "4")
	{
		try
		{
			string result = apiUtil->fun_build(username, pfn_info.getFunName());
			if (result == "")
			{
				response->Success("Function build success.");
			}
			else
			{
				response->Error(StatusOperationFailed, result);
			}
		}
		catch(const std::exception& e)
		{
			std::string error = "Function build fail: " + string(e.what());
			response->Error(StatusOperationFailed, error);
		}
	}
	else
	{
		error = "The type is invalid, please look up the api document.";
		response->Error(StatusParamIsIllegal, error);
	}
}

/**
 * review personalized function
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {  
 *   funInfo: {
 *     funName: "function name",
 *     funDesc: "function description",
 *     funArgs: "1: no K-Hop param, 2: has K-Hop param",
 *     funBody: "function body",
 *     funSubs: "sub function",
 *     funReturn: "path/multipath/kv/value"
 *    }
 * }
 */
void fun_review_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string error;
		if (hasJsonParam(json_data, "funInfo") == false)
		{
			error = "the value of funInfo can not be empty!";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string username = jsonParam(json_data, "username");
		struct PFNInfo pfn_info;
		rapidjson::Value &fun_info = json_data["funInfo"];
		build_PFNInfo(fun_info, pfn_info);
		apiUtil->fun_review(username, &pfn_info);
		string content = pfn_info.getFunBody();
		content = Util::urlEncode(content);
		Json resp_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.SetObject();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		resp_data.AddMember("Result", StringRef(content.c_str()), allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		std::string error = "Function review fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}