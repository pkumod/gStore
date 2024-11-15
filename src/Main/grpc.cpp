#include <stdio.h>
#include "workflow/WFFacilities.h"
#include "../GRPC/grpc_server.h"
#include "../GRPC/grpc_status_code.h"
#include "../GRPC/grpc_operation.h"
// #include "../Api/APIConnector.h"
#include "../Api/APIUtil.h"
#include "../Api/HttpUtil.h"
#include "../Pfn/PFNUtil.h"
#include "../Util/CompressFileUtil.h"
#include "../Reason/Reason.h"
#include "../Cluster/ClusterManager.h"
#include "../Cluster/ClusterDefined.h"
#include "../Server/ApiProvider.h"

#define HTTP_TYPE "http"
#define BASE_URL "http://127.0.0.1:" + _server_port
#define API_URL BASE_URL + "/api"
#define OFF_URL BASE_URL + "/shutdown"

using namespace std;
using namespace grpc;
using namespace cluster;
typedef unsigned short uint8;

static WFFacilities::WaitGroup wait_group(1);

shared_ptr<APIUtil> apiUtil = nullptr;

shared_ptr<PFNUtil> pfnUtil = nullptr;

std::shared_ptr<cluster::ClusterManager> clusterManagerPtr =  nullptr;

Latch latch;

bool _is_server_running = false;

std::string _server_port;

std::string _server_deamon;

useconds_t _max_wait_time = 3 * 1000 * 1000;

bool startServer(bool background = false);
bool stopServer();
void initialServer(uint16_t port, bool background = false);
void releaseGlobalPtr(bool renew = false);
void register_service(GRPCServer &grpcServer);

void parseRequest(const GRPCReq *request, nlohmann::json &json_data);
bool checkRequest(const GRPCReq *request, GRPCResp *response, operation_type& op_type, nlohmann::json &json_data, bool check_license=true);

void shutdown(const GRPCReq *request, GRPCResp *response);
void cluster_api(const GRPCReq *request, GRPCResp *response, const cluster::ClusterOperation& operation);
void api(const GRPCReq *request, GRPCResp *response, SeriesWork *series);
void sys_api(const GRPCReq *request, GRPCResp *response, const operation_type& operation);
void upload_file(const GRPCReq *request, GRPCResp *response, SeriesWork *series);
void download_file(const GRPCReq *request, GRPCResp *response);
void redirect_handler(const GRPCReq *request, GRPCResp *response, SeriesWork *series);
void waiting_handler(const useconds_t microseconds, uint16_t &sync_status, const std::string& msg, useconds_t max_wait_timeout);
// for server
void check_task(const GRPCReq *request, GRPCResp *response);
void login_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void test_connect_task(const GRPCReq *request, GRPCResp *response);
void core_version_task(const GRPCReq *request, GRPCResp *response);
void ip_manage_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void refresh_conf_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
// for db
void init_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void show_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void load_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void unload_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void monitor_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void build_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, nlohmann::json &json_data);
void drop_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, nlohmann::json &json_data);
void backup_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void backup_path_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void restore_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void query_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, nlohmann::json &json_data);
void export_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void begin_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void tquery_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void commit_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void rollback_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void checkpoint_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void batch_insert_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, nlohmann::json &json_data);
void batch_remove_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, nlohmann::json &json_data);
void rename_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
// for user
void user_manage_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void user_show_task(const GRPCReq *request, GRPCResp *response);
void user_privilege_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void user_password_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
// for log
void txn_log_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void query_log_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void query_log_date_task(const GRPCReq *request, GRPCResp *response);
void access_log_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void access_log_date_task(const GRPCReq *request, GRPCResp *response);
void checkOperationState_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
// for personalized function
void fun_query_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void fun_cudb_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
void fun_review_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
// for system stat
void stat_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
// for reason engine
void reason_manage_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data);
// for cluster
void cluster_heartbeat_task(const GRPCReq *request, GRPCResp *response);
void cluster_append_task(const GRPCReq *request, GRPCResp *response);
void cluster_reply_task(const GRPCReq *request, GRPCResp *response);
void cluster_check_task(const GRPCReq *request, GRPCResp *response);
void cluster_recover_task(const GRPCReq *request, GRPCResp *response);
// for license
void license_import(const GRPCReq *request, GRPCResp *response);
void license_info(const GRPCReq *request, GRPCResp *response);
void license_remove(const GRPCReq *request, GRPCResp *response);

void parseRequest(const GRPCReq *request, nlohmann::json &json_data)
{
	if (request->contentType() == APPLICATION_JSON) //for application/json
	{
		json_data = request->json();
	}
	else if (request->contentType() == APPLICATION_URLENCODED) //for applicaiton/x-www-form-urlencoded
	{
		std::map<std::string, std::string> &form_data = request->formData();
		std::map<std::string, std::string>::iterator iter = form_data.begin();
		std::string v;
		while (iter != form_data.end())
		{
			v = iter->second;
			if (UrlEncode::is_url_encode(v))
			{
				v = gutil::StringUtil::url_decode(iter->second);
			}
			json_data[iter->first] = v;
			iter++;
		}
	}
	else if (request->contentType() == MULTIPART_FORM_DATA) //for multipart/form-data
	{
		Form &form = request->form();
		if (form.empty())
		{
			return;
		}
		for (Form::iterator iter = form.begin(); iter != form.end(); iter++)
		{
			if (iter->first == "file")
			{
				json_data["filename"] = form.at(iter->first).first;
				string hex_v = form.at(iter->first).second;
				size_t hex_size = hex_v.size();
				std::vector<uint8_t> file_data;
				file_data.resize(hex_size);
				std::copy(hex_v.begin(), hex_v.end(), file_data.begin());
				json_data["file"] = nlohmann::json::binary(file_data, hex_size);
			}
			else
			{
				string v = form.at(iter->first).second;
				json_data[iter->first] = v;
			}
		}
	}
	else // for get
	{
		std::map<std::string, std::string> params = request->queryList();
		if (params.empty())
		{
			return;
		}
		std::map<std::string, std::string>::iterator iter = params.begin();
		std::string v;
		while (iter != params.end())
		{
			v = iter->second;
			if (UrlEncode::is_url_encode(v))
			{
				v = gutil::StringUtil::url_decode(iter->second);
			}
			json_data[iter->first] = v;
			iter++;
		}
	}
}

bool checkRequest(const GRPCReq *request, GRPCResp *response, operation_type& op_type, nlohmann::json &json_data, bool check_license)
{			
	if (request->hasHeader("Accept-Encoding")) 
	{
		std::string accept_encoding = request->header("Accept-Encoding");
		if (accept_encoding.find("gzip") != std::string::npos)
		{
			response->headers["Content-Encoding"] = "gzip";
		}
	}
	// check ip address
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	std::string ipCheckResult;
	if (apiUtil->check_access_ip(ip_addr, 1, ipCheckResult) == false)
	{
		SLOG_WARN("request from " << ip_addr << " is blocked");
		response->Error(StatusIPBlocked, ipCheckResult);
		return false;
	}
	std::string ss;
	ss += "\n==================== http-api ====================";
	ss += "\n  Content-Type: " + ContentType::to_str(request->contentType());
	ss += "\n  Accept-Encoding: " + request->header("Accept-Encoding");
	ss += "\n  method: " +  string(request->get_method());
	ss += "\n  httpVersion: " +  string(request->get_http_version());
	ss += "\n  requestUri: " +  string(request->get_request_uri());
	if (request->contentType() != MULTIPART_FORM_DATA && !request->body().empty())
	{
		std::string body = gutil::StringUtil::clear_linebreak(request->body());
		ss += "\n  request_body: ";
		if (body.length() > 1024) 
		{
			ss += body.substr(0, 1024) + "...";
		}
		else
		{
			ss += body;
		}
	}
	ss += "\n==================================================";
	SLOG_DEBUG(ss);
	std::string operation;
	if (json_data.contains("operation")) 
	{
		operation = JsonUtil::jsonParam(json_data, "operation");
	}
	// parse request
	parseRequest(request, json_data);
	SLOG_DEBUG("Parse request params: \n" << json_data.dump(4));
	if (operation.empty() && json_data.contains("operation"))
	{
		operation = JsonUtil::jsonParam(json_data, "operation");
	}
	SLOG_INFO("receive [" << operation << "] request from " << ip_addr);
	// check license
	// if (operation != "login" && operation != "check" && operation != "testConnect" )
	// {
	// 	string msg;
	// 	if (check_license && apiUtil->check_license(msg) == false)
	// 	{
	// 		SLOG_INFO("License is invalid: " << msg);
	// 		response->Error(server::StatusCode::StatusLicenseInvalid, msg);
	// 		return false;
	// 	}
	// }
	// add remote_ip param
	json_data["remote_ip"] = ip_addr;
	if (operation.empty()) 
	{
		operation = JsonUtil::jsonParam(json_data, "operation", "unknown");
	}
	op_type = OperationType::to_enum(operation);
	if (op_type != OP_LOGIN && op_type != OP_TEST_CONNECT)
	{
		if (apiUtil->check_access_ip(ip_addr, 2, ipCheckResult) == false)
		{
			SLOG_WARN("request from " << ip_addr << " is blocked");
			response->Error(StatusIPBlocked, ipCheckResult);
			return false;
		}
	}
	if (operation == "unknown")
	{
		response->Error(StatusOperationUndefined);
		return false;
	}
	// add callback task for access log start
	bool async = JsonUtil::jsonBoolParam(json_data, "async", false);
	if (async == false)
	{
		struct DBAccessLogInfo *access_log_info_ptr = new DBAccessLogInfo(ip_addr, operation);
		rpc_task->add_callback([access_log_info_ptr](GRPCTask *task) {
			GRPCResp *resp = task->get_resp();
			apiUtil->write_access_log(access_log_info_ptr->operation, access_log_info_ptr->ip, resp->resp_code, resp->resp_msg, "", resp->success_num, resp->failed_num);
			delete access_log_info_ptr;
		});
	}
	// add callback task for access log end
	if (op_type == OP_CHECK)
	{
		return true;
	}
	if (json_data.contains("username") == false || json_data.contains("password") == false)
	{
		response->Error(StatusParamIsIllegal, "username or password is empty");
		return false;
	}
	std::string username = JsonUtil::jsonParam(json_data, "username");
	std::string password = JsonUtil::jsonParam(json_data, "password");
	std::string encryption = JsonUtil::jsonParam(json_data, "encryption");
	std::string db_name = JsonUtil::jsonParam(json_data, "db_name");
	bool is_inner = JsonUtil::jsonBoolParam(json_data, "inner", false);
	bool need_check_privilege = true;
	// skip check privilege for inner request
	if (is_inner && "127.0.0.1" == ip_addr)
	{
		need_check_privilege = false;
	}
	// check username and password
	if(need_check_privilege) 
	{
		std::string checkidentityresult;
		if (apiUtil->check_indentity(username, password, encryption, checkidentityresult) == false)
		{
			apiUtil->update_access_ip_error_num(ip_addr);
			response->Error(StatusAuthenticationFailed, checkidentityresult);
			return false;
		}
	}
	// check privilege
	if (username != GlobalTypedef::root_uname())
	{
		need_check_privilege = true;
	}
	if (need_check_privilege && apiUtil->check_privilege(username, operation, db_name) == 0)
	{
		std::string msg = "You have no " + operation + " privilege";
		response->Error(StatusOperationConditionsAreNotSatisfied, msg);
		return false;
	}
	return true;
}

void waiting_handler(const useconds_t microseconds, uint16_t &sync_status, const std::string& msg, useconds_t max_wait_timeout)
{
	std::string waiting = ".";
	useconds_t curr_wait_time = 0;
	while (sync_status)
	{
		usleep(microseconds);
		curr_wait_time += microseconds;
		if (curr_wait_time % 1000000 == 0)
		{
			SLOG_DEBUG(msg + waiting);
			waiting.append(".");
		}
		if(sync_status > 1 || curr_wait_time > max_wait_timeout)
		{
			break;
		}
	}
	return;
}

void sig_handler(int sig)
{
	SLOG_INFO("Capture system signal：" + to_string(sig));
	stopServer();
	std::cout.flush();
	_exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[])
{
	Util util;
	on_exit([](int status, void *arg) {
		releaseGlobalPtr();
	}, NULL);
	_server_port = util.getConfigureValue("port");
	_server_deamon = util.getConfigureValue("deamon");
	string _website = GlobalTypedef::product_website;
	srand(time(NULL));
	string command = "-s";
	if (argc>1)
	{
		command = argv[1];
	}
	if (command == "-h" || command == "--help")
	{
		cout << endl;
		cout << "gStore API Server(gserver)" << endl;
		cout << endl;
		cout << "Usage:\tbin/gserver [option]" << endl;
		cout << endl;
		cout << "Options:" << endl;
		cout << "\t-h,--help\t\tDisplay this message." << endl;
		cout << "\t-s,--start[default]\t\tStart gServer." << endl;
		// cout << "\t-db,--database[option],\t\tthe database name.Default value is empty."<< endl;
		// cout << "\t-c,--csr[option],\t\tEnable CSR Struct or not. 0 denote that false, 1 denote that true. Default value is 0." << endl;
		cout << "\t-t,--stop\t\tSafe shutdow gServer." << endl;
		cout << "\t-r,--restart\t\tRestart gServer." << endl;
		cout << "\t-k,--kill\t\tForce shutdow gServer." << endl;
		cout << "\t-b,--background\t\tStart gServer in the background." << endl;
		cout << "\t-S,--status\t\tShow gServer status." << endl;
		cout << endl;
		return 0;
	}
	else if ((command == "-s" || command == "--start"))
	{
		// check server thread
		// server::MessageCheckRequest check_request;
		// server::MessageCheckResponse check_response = APIConnector::check(API_URL, check_request);
		httpentities::CheckRequest check_request;
		httpentities::CheckResponse check_response = HttpUtil::check(API_URL, check_request);
		if (check_response.success())
		{
			cout << "the server already running." << endl;
			return 1;
		}
		startServer(false);
	}
	else if (command == "-t" || command == "--stop")
	{
		// server::MessageCheckRequest check_request;
		// server::MessageCheckResponse check_response = APIConnector::check(API_URL, check_request);
		httpentities::CheckRequest check_request;
		httpentities::CheckResponse check_response = HttpUtil::check(API_URL, check_request);
		if(!check_response.success()) {
			cout << "server is inactive (dead)." << endl;
			return 0;
		}
		// stop server
		if(!stopServer() || _server_deamon == "on")
			execl("/usr/bin/killall", "killall", Util::getExactPath(argv[0]).c_str(), NULL);
		return 0;
	}
	else if (command == "-r" || command == "--restart")
	{
		bool background = false;
		// server::MessageCheckRequest check_request;
		// server::MessageCheckResponse check_response = APIConnector::check(API_URL, check_request);
		httpentities::CheckRequest check_request;
		httpentities::CheckResponse check_response = HttpUtil::check(API_URL, check_request);
		if(check_response.success()) {
			// read pid file
			ifstream ifp(GlobalTypedef::pid_path.c_str());
			std::string line;
			if (ifp.is_open()) {
				int current_line = 0;
				while (current_line < 3 && std::getline(ifp, line))
				{
					current_line++;
				}
				ifp.close();
				if (current_line == 3 && line == "1")
				{
					background = true;
				}
			}
			cout << "server is active (running)." << endl;
			cout << "stop server..." << endl;
			if(!stopServer() || _server_deamon == "on")
			{
				execl("/usr/bin/killall", "killall", Util::getExactPath(argv[0]).c_str(), NULL);
			}
		}
		// start server
		cout << "start server..." << endl;
		if (startServer(background))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else if (command  == "-k" || command == "--kill")
	{
		// server::MessageCheckRequest check_request;
		// server::MessageCheckResponse check_response = APIConnector::check(API_URL, check_request);
		httpentities::CheckRequest check_request;
		httpentities::CheckResponse check_response = HttpUtil::check(API_URL, check_request);
		if(!check_response.success()) {
			cout << "server is inactive (dead)." << endl;
			return 0;
		}
		// kill server
		cout << "The service will be forcibly stopped!" << endl;
		execl("/usr/bin/killall", "killall", Util::getExactPath(argv[0]).c_str(), NULL);
		// remove pid file
		Util::remove_path(GlobalTypedef::pid_path);
		return 0;
	}
	else if (command == "-S" || command == "--status")
	{
		// show server status
		// server::MessageCheckRequest check_request;
		// server::MessageCheckResponse check_response = APIConnector::check(API_URL, check_request);
		httpentities::CheckRequest check_request;
		httpentities::CheckResponse check_response = HttpUtil::check(API_URL, check_request);
		cout << "gStore API Server(gserver)" << endl;
		if (check_response.success())
		{
			cout << "\tActive: active (running)" << endl;
			cout << "\tMain PID: " << check_response.getStatusMsg() << endl;
		}
		else
		{
			cout << "\tActive: inactive (dead)" << endl;
			cout << "\tProcess: bin/gserver" << endl;
		}
		cout << "\tDocs: " + _website << endl;
		return 0;
	}
	else if ((command == "-b" || command == "--background"))
	{
		// check server thread
		// server::MessageCheckRequest check_request;
		// server::MessageCheckResponse check_response = APIConnector::check(API_URL, check_request);
		httpentities::CheckRequest check_request;
		httpentities::CheckResponse check_response = HttpUtil::check(API_URL, check_request);
		if (check_response.success())
		{
			cout << "the server already running." << endl;
			return -1;
		}
		if (startServer(true))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		cout << "Invalid arguments! Input \"bin/gserver -h\" for help." << endl;
		return 1;
	}
}

bool checkServerPort(uint16_t port)
{
	int max_try = 20;
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int bind_return = -1; 
	
	if (bind_return == -1)
	{
		std::cout << "waiting" << std::flush;
		while (bind_return == -1 && max_try > 0) {
			cout<<"."<<std::flush;
			sleep(3);
			bind_return = bind(sock, (struct sockaddr*) &addr,sizeof(addr));
			max_try --;
		}
		std::cout << std::endl;
		if (bind_return == -1)
		{			
			SLOG_INFO("Server port " + _server_port + " is already in use.");
			return false;
		}
	} 
	// relase bind
	max_try = 20;
	int close_status = close(sock);
	while (close_status != 0 && max_try > 0) {
		close_status = close(sock);
		// shutdown_status = shutdown(sock, SHUT_RDWR);
		if (close_status != 0)
		{
			SLOG_DEBUG("close I/O result:" + to_string(close_status));
			sleep(3);
		}
		max_try --;
	}
	sock = -1;
	std::memset(&addr, 0, sizeof(addr));
	return true;
}

bool startServer(bool background) 
{	uint64_t _port_ul =  stoul(_server_port);
	if (_port_ul > 65535)
	{
		SLOG_ERROR("Invalid port number: " + _server_port);
		return false;
	}
	uint16_t port =  static_cast<uint16_t>(_port_ul);
	// check port
	if (!checkServerPort(port))
	{
		return false;
	}
	pid_t fpid;
	int status;
	if (background)
	{
		// run in background
		fpid = fork();
		// child
		if (fpid == 0)
		{
			while (true)
			{
				if (_server_deamon == "on")
					fpid = fork();
				else
					fpid = 0;
				// child, main process
				if (fpid == 0)
				{
					// initialize server
					initialServer(port, background);
					return true;
				}
				// parent, deamon process
				else if (fpid > 0)
				{
					int status;
					waitpid(fpid, &status, 0);
					if (WIFEXITED(status))
					{
						return true;
					}
					else
					{
						SLOG_WARN("Stopped abnormally, restarting server...");
						sleep(1);
						releaseGlobalPtr(true);
					}
				}
				// fork failure
				else
				{
					SLOG_ERROR("Failed to start server: deamon fork failure.");
					return false;
				}
			}
		}
		// parent
		else if (fpid > 0)
		{
			return true;
		}
		// fork failure
		else 
		{
			SLOG_ERROR("Failed to start server: fork failure.");
			return false;
		}
	}
	else
	{
		//run in foreground
		while(true)
		{
			if (_server_deamon == "on")
				fpid = fork();
			else
				fpid = 0;
			// child, main process
			if (fpid == 0)
			{
				// initialize server
				initialServer(port, background);
				return true;
			}
			// parent, deamon process
			else if (fpid > 0)
			{
				int status;
				waitpid(fpid, &status, 0);
				if (WIFEXITED(status))
				{
					return true;
				}
				else
				{
					SLOG_WARN("Stopped abnormally, restarting server...");
					sleep(1);
					releaseGlobalPtr(true);
				}
			}
			// fork failure
			else
			{
				SLOG_ERROR("Failed to start server: fork failure.");
				return false;
			}
		}
	}
}

void initialServer(uint16_t port, bool background)
{
	apiUtil = make_shared<APIUtil>();
	pfnUtil = make_shared<PFNUtil>();
	clusterManagerPtr = make_shared<ClusterManager>();
	// init config
	int rt = apiUtil->initialize();			
	if (rt == -1)
	{
		exit(EXIT_FAILURE);
	}
	// save start type to pid file
	fstream ofp;
	std::string pid_path = GlobalTypedef::pid_path;
	ofp.open(pid_path.c_str(), ios::app);
	ofp << background;
	ofp << '\n';
	ofp.flush();
	ofp.close();
	sleep(1);
	if (clusterManagerPtr->isEnable()) {
		SLOG_INFO("cluster status on");
		clusterManagerPtr->init();
		std::vector<std::string> headers = {"name", "value"};
		std::vector<std::vector<std::string>> rows;
		std::string role = clusterManagerPtr->getCluterRole() == ClusterRoleType_Leader ? "leader" : "follower";
		rows.push_back({"role", role});
		rows.push_back({"heartbeat", Util::getConfigureValue("cluster_heartbeat") + " s"});
		rows.push_back({"relpy_timeout", Util::getConfigureValue("cluster_relpy_timeout") + " s"});
		rows.push_back({"data_path", Util::getConfigureValue("cluster_data_path")});
		if (clusterManagerPtr->isLeader()) {
			uint16_t node_idx = 1;
			for (auto& follower : clusterManagerPtr->getFollowrUrlArray()) {		    
				rows.push_back({"node" + to_string(node_idx), follower});
				node_idx++ ;
			}
		} else {
			rows.push_back({"node", clusterManagerPtr->getLeaderUrl()});
		}
		Util::printConsole(headers, rows);
	} else {
		SLOG_INFO("cluster status off");
	}
	GRPCServer grpcServer;
	// register rest service
	register_service(grpcServer);
	int max_try = 30;
	int start_status = -1;
	do
	{
		// try starting until success of more than max_try
		start_status = grpcServer.start(port);
		if(start_status != 0)
		{
			SLOG_INFO("Server try starting " + to_string(start_status));
			sleep(1);
		}
		max_try--;
	} while (start_status == -1 && max_try > 0);
	if(start_status != 0)
	{
		SLOG_ERROR("Server start failed.");
		exit(EXIT_FAILURE);
	}
	std::string lic_msg;
	if (apiUtil->check_license(lic_msg))
	{
		apiUtil->print_license();
	}
	else
	{
		SLOG_DEBUG("check license result: " + lic_msg);
		SLOG_WARN("The license is invalid. Please import a new license in time");
	}
	SLOG_INFO("Server started at port " + _server_port);
	_is_server_running = true;
	// handle the Ctrl+C signal
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	wait_group.wait();
	grpcServer.stop();
	// apiUtil.reset();
	// pfnUtil.reset();
	// clusterManagerPtr.reset();
	SLOG_INFO("Server stoped.");
	std::cout.flush();
	exit(EXIT_SUCCESS);
}

void releaseGlobalPtr(bool renew)
{
	SLOG_DEBUG("release global pointer begin...");
	latch.lockExclusive();
	if (apiUtil) {
		apiUtil.reset();
		if(renew) {
			apiUtil = make_shared<APIUtil>();
		}
	}
	if (pfnUtil) 
	{
		pfnUtil.reset();
		if(renew) {
			pfnUtil = make_shared<PFNUtil>();
		}
	}
	if (clusterManagerPtr)
	{
		clusterManagerPtr->stopHeart();
		clusterManagerPtr.reset();
		if(renew) {
			clusterManagerPtr = make_shared<ClusterManager>();
		}
	}
	latch.unlock();
	SLOG_DEBUG("release global pointer ok");
}

bool stopServer()
{
	string pid_path = GlobalTypedef::pid_path;
	SLOG_DEBUG("pid path: " + pid_path);
	if (!Util::file_exist(pid_path))
	{
		return false;
	}
	string system_user = GlobalTypedef::sys_uname();
	string pid;
	string system_password;
	ifstream in;
	in.open(pid_path.c_str(), ios::in);
	getline(in, pid, '\n');
	getline(in, system_password, '\n');
	in.close();
	SLOG_DEBUG("port: " + _server_port + ", system user: " + system_user + ", password: " + system_password);
	// server::MessageShutdownRequest shutdown_request(system_user, system_password);
	// server::MessageShutdownResponse shutdown_response = APIConnector::shutdown(OFF_URL, shutdown_request);
	httpentities::ShutdownRequest shutdown_request(system_user, system_password);
	httpentities::ShutdownResponse shutdown_response = HttpUtil::shutdown(OFF_URL, shutdown_request);
	if (shutdown_response.success())
	{
		SLOG_INFO("the Server [" + pid + "] stop successfully.");
		Util::remove_file(pid_path);
		return true;
	}
	else
	{
		SLOG_INFO("the Server [" + pid + "] stop failed: " + shutdown_response.StatusMsg);
		return false;
	}
}

void register_service(GRPCServer &svr)
{
	std::vector<std::string> methods = {"GET", "POST"};
	svr.ROUTE(
		"/shutdown", [](const GRPCReq *request, GRPCResp *response)
		{ 
			shutdown(request, response);
		},
		methods);

	svr.ROUTE(
		"/cluster/heartbeat", [](const GRPCReq *request, GRPCResp *response)
		{ 
			cluster_api(request, response, cluster::ClusterOperation::ClusterOperation_HeartBeat);
		},
		ReqMethod::POST);

	svr.ROUTE(
		"/cluster/appendEntries", [](const GRPCReq *request, GRPCResp *response)
		{ 
			cluster_api(request, response, cluster::ClusterOperation::ClusterOperation_Append);
		},
		ReqMethod::POST);

	svr.ROUTE(
		"/cluster/appendEntries", [](const GRPCReq *request, GRPCResp *response)
		{
			response->add_header_pair("Access-Control-Allow-Origin", "*");
			response->add_header_pair("Access-Control-Allow-Methods", "POST");
			response->String("ok");
		},
		ReqMethod::OPTIONS);

	svr.ROUTE(
		"/cluster/reply", [](const GRPCReq *request, GRPCResp *response)
		{ 
			cluster_api(request, response, cluster::ClusterOperation::ClusterOperation_Replly);
		},
		ReqMethod::POST);

	svr.ROUTE(
		"/cluster/check", [](const GRPCReq *request, GRPCResp *response)
		{ 
			cluster_api(request, response, cluster::ClusterOperation::ClusterOperation_Check);
		},
		ReqMethod::POST);
	svr.ROUTE(
		"/cluster/recover", [](const GRPCReq *request, GRPCResp *response)
		{ 
			cluster_api(request, response, cluster::ClusterOperation::ClusterOperation_Recover);
		},
		ReqMethod::POST);

	svr.ROUTE(
		"/sys/query", [](const GRPCReq *request, GRPCResp *response)
		{ 
			sys_api(request, response, OP_QUERY);
		},
		methods);
		
	svr.ROUTE(
		"/", [](const GRPCReq *request, GRPCResp *response, SeriesWork *series)
		{ 
			api(request, response, series);
		},
		ReqMethod::POST);

	svr.ROUTE(
		"/api", [](const GRPCReq *request, GRPCResp *response, SeriesWork *series)
		{ 
			api(request, response, series);
		},
		methods);

	svr.ROUTE(
		"/file/upload", [](const GRPCReq *request, GRPCResp *response, SeriesWork *series)
		{
			upload_file(request, response, series);
		},
		ReqMethod::POST);

	svr.ROUTE(
		"/file/upload", [](const GRPCReq *request, GRPCResp *response)
		{
			response->add_header_pair("Access-Control-Allow-Origin", "*");
			response->add_header_pair("Access-Control-Allow-Methods", "POST");
			response->String("ok");
		},
		ReqMethod::OPTIONS);

	svr.ROUTE(
		"/file/download", [](const GRPCReq *request, GRPCResp *response)
		{
			download_file(request, response);
		},
		ReqMethod::POST);

	svr.ROUTE(
		"/file/download", [](const GRPCReq *request, GRPCResp *response)
		{
			response->add_header_pair("Access-Control-Allow-Origin", "*");
			response->add_header_pair("Access-Control-Allow-Methods", "POST");
			response->String("ok");
		},
		ReqMethod::OPTIONS);

	svr.ROUTE(
		"/lic/import", [](const GRPCReq *request, GRPCResp *response)
		{
			license_import(request, response);
		},
		ReqMethod::POST);
	svr.ROUTE(
		"/lic/import", [](const GRPCReq *request, GRPCResp *response)
		{
			response->add_header_pair("Access-Control-Allow-Origin", "*");
			response->add_header_pair("Access-Control-Allow-Methods", "POST");
			response->String("ok");
		},
		ReqMethod::OPTIONS);
	svr.ROUTE(
		"/lic/info", [](const GRPCReq *request, GRPCResp *response)
		{
			license_info(request, response);
		},
		methods);
	svr.ROUTE(
		"/lic/remove", [](const GRPCReq *request, GRPCResp *response)
		{
			license_remove(request, response);
		},
		ReqMethod::POST);
}

void shutdown(const GRPCReq *request, GRPCResp *response)
{
	if (!_is_server_running)
	{
		std::string msg = "server is stopping, please waitting !";
		SLOG_DEBUG(msg);
		response->Error(StatusIPBlocked, msg);
		return;
	}
	// check ip address
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	std::string ipCheckResult;
	if (!apiUtil || apiUtil->check_access_ip(ip_addr, 0, ipCheckResult) == false)
	{
		SLOG_DEBUG(ipCheckResult);
		response->Error(StatusIPBlocked, ipCheckResult);
		return;
	}
	SLOG_INFO("receive [shutdown] request from " << ip_addr);
	std::string msg;
	std::string username = request->header("username");
	std::string password = request->header("password");
	if (apiUtil->check_param_value("username", username, msg) == false)
	{
		response->Error(StatusParamIsIllegal, msg);
		return;
	}
	if (apiUtil->check_param_value("password", password, msg) == false)
	{
		response->Error(StatusParamIsIllegal, msg);
		return;
	}
	if (username != GlobalTypedef::sys_uname())
	{
		msg =  "You have no rights to stop the server.";
        response->Error(StatusAuthenticationFailed, msg);
		return;
	}
	if (apiUtil->check_server_indentity(password, msg) == false)
	{
		apiUtil->update_access_ip_error_num(ip_addr);
		response->Error(StatusAuthenticationFailed, msg);
		return;
	}
	// bool flag = apiUtil->db_checkpoint_all();
	rpc_task->add_callback([](GRPCTask *grpcTask){
		// free apiUtil
		_is_server_running = false;
		releaseGlobalPtr(false);
		std::cout.flush();
		_exit(EXIT_SUCCESS);
	});
	msg = "Server stopped successfully.";
	apiUtil->write_access_log("shutdown", ip_addr, StatusOK, msg);
	response->Success(msg);
}

void upload_file(const GRPCReq *request, GRPCResp *response, SeriesWork *series)
{
	operation_type op_type;
	nlohmann::json json_data = nlohmann::json{
		{"operation", "uploadfile"}
	};
	if (checkRequest(request, response, op_type, json_data) == false)
	{
		return;
	}
	// filename : filecontent
	std::string filename = JsonUtil::jsonParam(json_data, "filename");
	string msg;
	if(filename.empty() || JsonUtil::hasJsonParam(json_data, "file") == false)
	{
		msg = "Upload file can not be empty!";
		response->Error(StatusParamIsIllegal, msg);
		return;
	}
	if(request->has_content_length_header())
	{
		size_t content_length = stoul(request->header("Content-Length"));
		size_t max_body_size = apiUtil->get_upload_max_body_size();
		if (content_length > max_body_size)
		{
			SLOG_DEBUG("File size is " + to_string(content_length) + " byte, allowed max size " + to_string(max_body_size) + " byte!");
			msg = "Upload file more than max_body_size!";
			response->Error(StatusOperationFailed, msg);
			return;
		}
	}
	std::string file_suffix = GRPCUtil::fileSuffix(filename);
	if (!apiUtil->check_upload_allow_compress_packages(file_suffix) && apiUtil->check_upload_allow_extensions(file_suffix) == false)
	{
		msg = "The type of upload file is not supported!";
		response->Error(StatusOperationFailed, msg);
		return;
	}
	// redirect to cluster
	if (clusterManagerPtr->isEnable() && clusterManagerPtr->isFollower())
	{
		redirect_handler(request, response, series);
		return;
	}
	// remove path info, only return base filename
	std::string file_name = GRPCUtil::fileName(filename);
	size_t pos = file_name.size() - file_suffix.size() - 1;
	std::string file_dst = GlobalTypedef::upload_path() + file_name.substr(0, pos) + "_" + gutil::TimeUtil::now() + "." + file_suffix;
	std::string notify_msg = "{\"StatusCode\":0, \"StatusMsg\":\"success\", \"filepath\": \""+file_dst+"\"}";
	nlohmann::byte_container_with_subtype<std::vector<uint8_t>> file_binary =  json_data["file"].get_binary();
	response->Save(file_dst, static_cast<const void *>(file_binary.data()), file_binary.size(), notify_msg);
}

void download_file(const GRPCReq *request, GRPCResp *response)
{
	operation_type op_type;
	nlohmann::json json_data = nlohmann::json{
		{"operation", "downloadfile"}
	};
	if (checkRequest(request, response, op_type, json_data) == false)
	{
		return;
	}
	std::string error;
	std::string filepath = JsonUtil::jsonParam(json_data, "filepath");
	apiUtil->check_param_value("filepath", filepath, error);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	if (Util::is_file(filepath))
	{
		// the file must in the gstore home dir
		std::string exact_path = Util::getExactPath(filepath.c_str());
		std::string cur_path = Util::get_cur_path();
		SLOG_DEBUG("download file path: " + filepath);
		SLOG_DEBUG("file exact path: " + exact_path);
		if (gutil::StringUtil::start_with(exact_path, cur_path) == false)
		{
			error = "Download file must in the "+ GlobalTypedef::product_name +" home dir";
			response->Error(StatusOperationFailed, error);
			return;
		}
		bool compress = JsonUtil::jsonBoolParam(json_data, "compress", false);
		if (compress) // compress to zip file
		{
			std::string* zip_file_path = new string(exact_path + ".zip");
			CompressUtil::CompressZip compress_util;
			if (compress_util.compressDirExportZip(exact_path, *zip_file_path)) {
				task_of(response)->add_callback([zip_file_path](GRPCTask *_task){
					Util::remove_path(*zip_file_path);
					delete zip_file_path;
				});
				response->File(*zip_file_path);
			}
			else
			{
				response->File(exact_path);
			}
		}
		else 
		{
			response->File(exact_path);
		}
	}
	else
	{
		error = "Download file is not exist";
		response->Error(StatusOperationFailed, error);
	}
}

void redirect_handler(const GRPCReq *request, GRPCResp *response, SeriesWork *series)
{
	// redirect to leader
	WFHttpTask *leader_task;
	string leader_url =  clusterManagerPtr->getLeaderUrl();
	const string redirect_url = leader_url + request->get_request_uri() ;
	SLOG_DEBUG("cluster follower redirect to: " + redirect_url);
	leader_task = WFTaskFactory::create_http_task(redirect_url, 0, 0, [response](WFHttpTask *task) {
		int state = task->get_state();
		if (state == WFT_STATE_SUCCESS)
		{
			const void *body;
			size_t len;
			task->get_resp()->get_parsed_body(&body, &len);
			char* null_terminated_string = new char[len + 1];
			std::memcpy(null_terminated_string, body, len);
			null_terminated_string[len] = '\0'; 
			SLOG_DEBUG("leader response body: " << null_terminated_string);
			response->headers["Content-Type"] = ContentType::to_str(APPLICATION_JSON);
			response->String(null_terminated_string);
			task_of(response)->add_callback([null_terminated_string](GRPCTask *_task){
				delete []null_terminated_string;
			});
		}
		else
		{
			response->Error(StatusProxyError);
		}
	});
	// copy client request to the leader_task request
	const void *body;
	size_t len;
	request->get_parsed_body(&body, &len);

	auto *leader_req = leader_task->get_req();
	leader_req->set_method(request->get_method());
	// copy client request header
	protocol::HttpHeaderCursor req_cursor(request);
	std::vector<std::string> headerNames = {"Content-Type", "Accept-Encoding", "Accept", "Content-Length", "Connection", "Cache-Control"};
	std::string header_name;
	std::string header_value;
	while (req_cursor.next(header_name, header_value))
	{
		SLOG_DEBUG(header_name + ": " + header_value);
		if (std::find(headerNames.begin(), headerNames.end(), header_name) != headerNames.end())
		{
			leader_req->set_header_pair(header_name.c_str(), header_value.c_str());
		}
	}
	leader_req->append_output_body_nocopy(body, len);
	*series << leader_task;
}

void cluster_api(const GRPCReq *request, GRPCResp *response, const cluster::ClusterOperation& operation)
{
	if (!clusterManagerPtr) {
		response->Error(StatusOperationFailed, "The cluster is nullptr");
		return;
	}
	if (!clusterManagerPtr->isEnable()) {
		response->Error(StatusOperationFailed, "The cluster config is turned off");
		return;
	}
	// check ip address
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	// check cluster ip
	string cluster_ip_check = Util::getConfigureValue("cluster_ip_check", "off");
	if (cluster_ip_check == "on")
	{
		bool ipCheckResult;
		if (clusterManagerPtr->isLeader())
			ipCheckResult = clusterManagerPtr->fromFollowerIp(ip_addr);
		else
			ipCheckResult = clusterManagerPtr->fromLeader(ip_addr);
		if (ipCheckResult == false)
		{
			SLOG_DEBUG(ip_addr + " does not belong to the cluster whitelist");
			response->Error(StatusIPBlocked, "The ip address does not belong to the cluster whitelist");
			return;
		}
	}
	std::string op_str = cluster::ClusterOperationHandle::to_str(operation);
	grpc::content_type content_type = request->contentType();
	SLOG_INFO("receive [" << op_str << "] request from " << ip_addr);
	std::string ss;
	ss += "\n==================== cluster-api ====================";
	ss += "\n  Content-Type: " + ContentType::to_str(content_type);
	ss += "\n  Accept-Encoding: " + request->header("Accept-Encoding");
	ss += "\n  method: " +  string(request->get_method());
	ss += "\n  httpVersion: " +  string(request->get_http_version());
	ss += "\n  requestUri: " +  string(request->get_request_uri());
	if (content_type != MULTIPART_FORM_DATA && !request->body().empty())
	{
		ss += "\n  request_body: " + request->body();
	}
	ss += "\n==================================================";
	SLOG_DEBUG(ss);
	std::string username = request->header("username");
	std::string password = request->header("password");
	// check username and password
	std::string checkidentityresult;
	std::string encryption = "0";
	if (apiUtil->check_indentity(username, password, encryption, checkidentityresult) == false)
	{
		response->Error(StatusAuthenticationFailed, checkidentityresult);
		return;
	}
	// operation
	switch (operation)
	{
	case cluster::ClusterOperation_HeartBeat:
		// from leader heartbeat
		cluster_heartbeat_task(request, response);
		break;
	case cluster::ClusterOperation_Append:
		// from leader append entries
		cluster_append_task(request, response);
		break;
	case cluster::ClusterOperation_Replly:
		// from follower reply
		cluster_reply_task(request, response);
		break;
	case cluster::ClusterOperation_Check:
		cluster_check_task(request, response);
		break;
	case cluster::ClusterOperation_Recover:
		cluster_recover_task(request, response);
		break;
	default:
		SLOG_ERROR("Unkown operation:" + op_str);
		response->Error(StatusOperationUndefined);
		break;
	}
}

void sys_api(const GRPCReq *request, GRPCResp *response, const operation_type& operation)
{
	nlohmann::json json_data;
	parseRequest(request, json_data);
	bool is_inner = JsonUtil::jsonBoolParam(json_data, "inner", false);
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	string msg;
	if (!is_inner || ip_addr != "127.0.0.1")
	{
		SLOG_DEBUG("inner: " + std::to_string(is_inner) + ", ip: " + ip_addr);
		msg = "You are not allowed to access sys api";
		response->Error(StatusIPBlocked, msg);
		return;
	}
	if (operation == OP_QUERY)
	{
		string sparql;
		ResultSet rs;
		sparql = JsonUtil::jsonParam(json_data, "sparql");
		if (!apiUtil->check_param_value("sparql", sparql, msg))
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		bool query_rt = false;
		uint64_t query_time = gutil::TimeUtil::timestamp();
		query_rt = apiUtil->query_sys_db(sparql, rs);
		query_time = gutil::TimeUtil::timestamp() - query_time;
		if (!query_rt)
		{
			response->Error(StatusOperationFailed, "Query failed");
			return;
		}
		if (!query_rt)
		{
			response->Error(StatusOperationFailed, "Query failed");
			return;
		}
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
		server::MessageQueryResponse resp_data;
		resp_data.query_json = json_data;
		resp_data.StatusCode = StatusOK;
		resp_data.StatusMsg = "success";
		resp_data.ansNum = rs.ansNum;
		resp_data.outputLimit = -1;
		resp_data.threadId = gutil::ThreadUtil::getThreadID();
		resp_data.queryTime = to_string(query_time);
		rs.release();
		nlohmann::json resp_json;
		resp_data.toJson(resp_json);
		response->Json(resp_json);
	}
	else
	{
		response->Error(StatusOperationUndefined);
	}
}



void api(const GRPCReq *request, GRPCResp *response, SeriesWork *series)
{
	operation_type op_type;
	nlohmann::json json_data = nlohmann::json::object();
	if (checkRequest(request, response, op_type, json_data) == false) 
	{
		return;
	}
	// api operation
	switch (op_type)
	{
	case OP_CHECK:
		check_task(request, response);
		break;
	case OP_LOGIN:
		login_task(request, response, json_data);
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
	case OP_REFRESH_CONF:
		refresh_conf_task(request, response, json_data);
		break;
	case OP_INIT:
		init_task(request, response, json_data);
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
		build_task(request, response, series, json_data);
		break;
	case OP_DROP:
		drop_task(request, response, series, json_data);
		break;
	case OP_BACKUP:
		backup_task(request, response, json_data);
		break;
	case OP_BACKUP_PATH:
		backup_path_task(request, response, json_data);
		break;
	case OP_RESTORE:
		restore_task(request, response, json_data);
		break;
	case OP_QUERY:
		query_task(request, response, series, json_data);
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
		batch_insert_task(request, response, series, json_data);
		break;
	case OP_BATCH_REMOVE:
		batch_remove_task(request, response, series, json_data);
		break;
	case OP_RENAME:
		rename_task(request, response, json_data);
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
	case OP_STAT:
		stat_task(request, response, json_data);
		break;
	case OP_CHECKOPERATIONSTATE:
		checkOperationState_task(request, response, json_data);
		break;
	case OP_REASON_MANAGE:
	    reason_manage_task(request,response,json_data);
		break;
	default:
		SLOG_ERROR("Unkown operation, request body:\n" + request->body());
		response->Error(StatusOperationUndefined);
		break;
	}
}

/**
 * check the server activity
 * 
 * @param request 
 * @param response 
 */
void check_task(const GRPCReq *request, GRPCResp *response)
{
	// std::string success = "the server is running...";
	std::string success = to_string(getpid());
	response->Success(success);
}

/**
 * login server
 * 
 * @param request 
 * @param response 
 */
void login_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	try
	{
		nlohmann::json resp_data = nlohmann::json{
			{"StatusCode", 0},
			{"StatusMsg", "login successfully"}
		};
		string licensetype;
		if (apiUtil->get_license().type == "0") 
		{
			licensetype = "trial";
		}
		else
		{
			licensetype = "official";
		}
		resp_data["licensetype"] = licensetype;
		resp_data["CoreVersion"] = GlobalTypedef::product_version;
		resp_data["RootPath"] = Util::get_cur_path();
		resp_data["type"] = HTTP_TYPE;
		string remote_ip = JsonUtil::jsonParam(json_data, "remote_ip");
		apiUtil->reset_access_ip_error_num(remote_ip);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "login fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * login server
 * 
 * @param request 
 * @param response
 */
void test_connect_task(const GRPCReq *request, GRPCResp *response)
{
	try
	{
		nlohmann::json resp_data = nlohmann::json{
			{"StatusCode", 0},
			{"StatusMsg", "success"}
		};
		string licensetype;
		if (apiUtil->get_license().type == "0") 
		{
			licensetype = "trial";
		}
		else
		{
			licensetype = "official";
		}
		resp_data["licensetype"] = licensetype;
		resp_data["CoreVersion"] = GlobalTypedef::product_version;
		resp_data["type"] = HTTP_TYPE;
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
		nlohmann::json resp_data = nlohmann::json{
			{"StatusCode", 0},
			{"StatusMsg", "success"}
		};
		resp_data["CoreVersion"] = GlobalTypedef::product_version;
		resp_data["type"] = HTTP_TYPE;
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
void ip_manage_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	try
	{
		std::string error;
		string type = json_data["type"];
		if (type == "1")
		{
			string IPtype = apiUtil->ip_enabled_type();
			if (IPtype == "3")
			{
				error = "please configure ip_deny_path or ip_allow_path in the conf.ini file first.";
				response->Error(StatusOperationFailed, error);
				return;
			}
			vector<string> ip_list;
			apiUtil->ip_list(IPtype, ip_list);
			size_t count = ip_list.size();
			nlohmann::json ips = nlohmann::json::array();
			for (size_t i = 0; i < count; i++)
			{
				ips.push_back(std::move(ip_list[i]));
			}
			nlohmann::json resp_data = nlohmann::json{
				{"StatusCode", 0},
				{"StatusMsg", "success"}
			};
			resp_data["ResponseBody"] = nlohmann::json{
				{"ip_type", IPtype},
				{"ips", ips}
			};
			response->Json(resp_data);
		}
		else if (type == "2")
		{
			std::string ips = json_data["ips"];
			std::string ip_type = json_data["ip_type"];
			if (ips.empty())
			{
				error = "the ips can't be empty";
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
 * refresh the configuration file
 * 
 * @param request 
 * @param response 
 * @param json_data
 */
void refresh_conf_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	apiUtil->refresh_conf();
	clusterManagerPtr->refresh();
	response->Success("refreshing configuration success");
}

/**
 * init an exist database
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {username: "the user who is the owner of database or has rights to access the database"}
 * {database: "the name of database"}
 */
void init_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	std::string db_names = JsonUtil::jsonParam(json_data, "db_names");
	if (db_names.empty())
	{
		response->Error(StatusParamIsIllegal, "db_names can't be empty");
		return;
	}
	std::string username = json_data["username"];
	std::string built_time = gutil::TimeUtil::now(NORM_DATETIME_PATTERN);
	std::vector<std::string> db_name_vector;
	Util::split(db_names, ",", db_name_vector);
	nlohmann::json response_data = nlohmann::json{
		{"StatusCode", 0},
		{"StatusMsg", "init database successfully!"},
		{"data", {}}
	};
	for(auto db_name : db_name_vector)
	{
		nlohmann::json db_info;
		db_info["db_name"] = db_name;
		if (apiUtil->check_db_built(db_name))
		{
			db_info["status"] = "1";
			db_info["msg"] = "exist";
			response_data["data"].push_back(db_info);
			continue;
		} 
		std::string db_path = GlobalTypedef::db_home() + db_name + GlobalTypedef::db_suffix();
		if(!Util::dir_exist(db_path))
		{
			db_info["status"] = "1";
			db_info["msg"] = db_name + GlobalTypedef::db_suffix() + " is not exist.";
			response_data["data"].push_back(db_info);
			continue;
		}
		if(apiUtil->init_databaseinfo(db_name, username, built_time, DatabaseStatus::AREADY_BUILT))
		{
			db_info["status"] = "0";
			db_info["msg"] = "success";
		}
		else
		{
			db_info["status"] = "1";
			db_info["msg"] = "fail";
		}
		response_data["data"].push_back(db_info);
	}
	response->Json(response_data);
}

/**
 * show the all database list (except system database)
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {username: "the user who is the owner of database or has rights to access the database"}
 */
void show_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	try
	{
		std::string username = json_data["username"];

		vector<shared_ptr<DatabaseInfo>> array;
		apiUtil->get_databaseinfos(username, array);

		nlohmann::json resp_data = nlohmann::json{
			{"StatusCode", 0},
			{"StatusMsg", "Get the database list successfully!"}
		};
		resp_data["ResponseBody"] = nlohmann::json::array();
		size_t count = array.size();
		for (size_t i = 0; i < count; i++)
		{
			shared_ptr<DatabaseInfo> dbInfo = array[i];
			resp_data["ResponseBody"].push_back(dbInfo->toJSON());
		}
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
void load_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageLoadRequest request_data(json_data);
	server::MessageLoadResponse response_data; 
	server::ApiHandler::load(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void unload_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	try
	{
		std::string db_name = JsonUtil::jsonParam(json_data, "db_name");
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusOperationFailed, msg);
			return;
		}
		if (apiUtil->check_db_built(db_name) == false)
		{
			msg = "the database [" + db_name + "] not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		if (apiUtil->check_db_loaded(db_name) == false)
		{
			msg = "the database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			msg = "Unable to unload due to loss of lock.";
			response->Error(StatusLossOfLock, msg);
			return;
		}
		else
		{
			apiUtil->remove_txn_manager(db_name, true);
			db_info->unloadDatabase();
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
void monitor_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageMonitorRequest request_data(json_data);
	server::MessageMonitorResponse response_data; 
	string remote_ip = JsonUtil::jsonParam(json_data, "remote_ip");
	server::ApiHandler::monitor(apiUtil, clusterManagerPtr, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void build_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, nlohmann::json &json_data)
{
	if (clusterManagerPtr->isEnable() && clusterManagerPtr->isFollower())
	{
		redirect_handler(request, response, series);
		return;
	}

	server::MessageBuildRequest request_data(json_data);
	server::MessageBuildResponse response_data; 
	if (request_data.async)
	{
		grpc::GRPCServerTask* sub_task = task_of(response);
		std::string opt_id;
        gutil::IdUtil::nextUID(opt_id);
		response_data.opt_id = opt_id;
		response_data.StatusCode = StatusOK;
		response_data.StatusMsg = "Operation Success.";
		sub_task->add_callback([request_data, opt_id](GRPCTask *)
		{
			server::MessageBuildResponse response_data;
			response_data.opt_id = opt_id;
			apiUtil->write_access_log(request_data.op, request_data.remote_ip, 0, "Operation success", opt_id);
			if (clusterManagerPtr->isEnable())
				server::ApiHandler::build_cluster(apiUtil, clusterManagerPtr, request_data, response_data);
			else
				server::ApiHandler::build(apiUtil, request_data, response_data);

			if (response_data.StatusCode == StatusOK)
				apiUtil->update_access_log(0, response_data.StatusMsg, response_data.opt_id, 1, response_data.successNum, response_data.failed_num);
			else
				apiUtil->update_access_log(response_data.StatusCode, response_data.StatusMsg, response_data.opt_id, -1, 0, 0);
			std::string callback = request_data.callback;
            if (!callback.empty())
            {
                std::string json_str;
                response_data.toJsonString(json_str);
                string res;
                HttpUtil::Post(callback, json_str, res);
            }
		});
	}
	else
	{
		if (clusterManagerPtr->isEnable())
			server::ApiHandler::build_cluster(apiUtil, clusterManagerPtr, request_data, response_data);
		else
			server::ApiHandler::build(apiUtil, request_data, response_data);
	}
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void drop_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, nlohmann::json &json_data)
{
	if(clusterManagerPtr->isEnable() && clusterManagerPtr->isFollower())
	{
		redirect_handler(request, response, series);
		return;
	}
	server::MessageDropRequest request_data(json_data);
	server::MessageDropResponse response_data; 
	server::ApiHandler::drop(apiUtil, clusterManagerPtr, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		response->Success(response_data.StatusMsg);
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
void backup_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageBackupRequest request_data(json_data);
	server::MessageBackupResponse response_data;
	if (request_data.async)
	{
		grpc::GRPCServerTask* sub_task = task_of(response);
		std::string opt_id;
        gutil::IdUtil::nextUID(opt_id);
		response_data.opt_id = opt_id;
		response_data.StatusCode = StatusOK;
		response_data.StatusMsg = "Operation Success.";
		sub_task->add_callback([request_data, opt_id](GRPCTask *)
		{
			server::MessageBackupResponse response_data;
			response_data.opt_id = opt_id;
			apiUtil->write_access_log(request_data.op, request_data.remote_ip, 0, "Operation success", opt_id);
			server::ApiHandler::backup_async(apiUtil, request_data, response_data);
		});
	}
	else
	{
		server::ApiHandler::backup(apiUtil, request_data, response_data);
	}
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
	}
}

/**
 * query backup path
 * 
 * @param request 
 * @param response 
 * @param json_data 
 */
void backup_path_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageBackupPathRequest request_data(json_data);
	server::MessageBackupPathResponse response_data;
	server::ApiHandler::backup_path(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void restore_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageRestoreRequest request_data(json_data);
	server::MessageRestoreResponse response_data;

	if (request_data.async)
	{
		grpc::GRPCServerTask* sub_task = task_of(response);
		std::string opt_id;
        gutil::IdUtil::nextUID(opt_id);
		response_data.opt_id = opt_id;
		response_data.StatusCode = StatusOK;
		response_data.StatusMsg = "Operation Success.";
		sub_task->add_callback([request_data, opt_id](GRPCTask *)
		{
			server::MessageRestoreResponse response_data;
			response_data.opt_id = opt_id;
			apiUtil->write_access_log(request_data.op, request_data.remote_ip, 0, "Operation success", opt_id);
			server::ApiHandler::restore_async(apiUtil, request_data, response_data);
		});
	}
	else
	{
		server::ApiHandler::restore(apiUtil, request_data, response_data);
	}
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void query_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, nlohmann::json &json_data)
{
	server::MessageQueryRequest request_data(json_data);
	server::MessageQueryResponse response_data; 
	string remote_ip = JsonUtil::jsonParam(json_data, "remote_ip");
	bool async = JsonUtil::jsonBoolParam(json_data, "async", false);
	GRPCServerTask *sub_task = task_of(response);
	bool is_update = false;
	if (clusterManagerPtr->isEnable())
	{
		server::ApiHandler::query_cluster(apiUtil, clusterManagerPtr, request_data, response_data, is_update, [sub_task](struct DBQueryLogInfo* query_log_ptr)
		{
			sub_task->add_callback([query_log_ptr](GRPCTask *t)
			{	
				apiUtil->write_query_log(query_log_ptr);
				delete query_log_ptr;
			});
		});
		if (response_data.StatusCode == StatusOK && clusterManagerPtr->isFollower() && is_update)
		{
			redirect_handler(request, response, series);
			return;
		}
	}
	else if (async)
	{
		std::string opt_id;
		gutil::IdUtil::nextUID(opt_id);
		response_data.opt_id = opt_id;
		response_data.StatusCode = StatusOK;
		response_data.StatusMsg = "Operation Success.";
		sub_task->add_callback([request_data, opt_id](GRPCTask *)
		{
			server::MessageQueryResponse response;
			response.opt_id = opt_id;
			apiUtil->write_access_log(request_data.op, request_data.remote_ip, StatusOK, "Operation Success.", opt_id);
			server::ApiHandler::query(apiUtil, request_data, response, [](struct DBQueryLogInfo* query_log_ptr)
			{
				apiUtil->write_query_log(query_log_ptr);
				delete query_log_ptr;
			});
			server::ApiHandler::query_result_notify(apiUtil, request_data, response);
		});
	}
	else
	{
		server::ApiHandler::query(apiUtil, request_data, response_data, [sub_task](struct DBQueryLogInfo* query_log_ptr)
		{
			sub_task->add_callback([query_log_ptr](GRPCTask *)
			{
				apiUtil->write_query_log(query_log_ptr);
				delete query_log_ptr;
			});
		}, true);
	}
	
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		response->set_header_pair("Cache-Control", "no-cache");
		response->set_header_pair("Pragma", "no-cache");
		response->set_header_pair("Expires", "0");
		std::string json_str;
		if (async)
			response_data.toAsyncJsonString(json_str);
		else
			response_data.toJsonString(json_str);
		response->Json(json_str);
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
void export_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageExportRequest request_data(json_data);
	server::MessageExportResponse response_data;
	server::ApiHandler::export_db(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void begin_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageBeginRequest request_data(json_data);
	server::MessageBeginResponse response_data; 
	server::ApiHandler::begin(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void tquery_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageTqueryRequest request_data(json_data);
	server::MessageTqueryResponse response_data; 
	server::ApiHandler::tquery(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void commit_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageCommitRequest request_data(json_data);
	server::MessageResponse response_data; 
	server::ApiHandler::commit(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void rollback_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageCommitRequest request_data(json_data);
	server::MessageResponse response_data; 
	server::ApiHandler::rollback(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
	}
}

/**
 * this is used for checkpoint, we must ensure that modification is written to disk
 * 
 * @param request 
 * @param response 
 * @param json_data 
 */
void checkpoint_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageCheckPointRequest request_data(json_data);
	server::MessageResponse response_data; 
	server::ApiHandler::checkpoint(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void batch_insert_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, nlohmann::json &json_data)
{
	if (clusterManagerPtr->isEnable() && clusterManagerPtr->isFollower())
	{
		redirect_handler(request, response, series);
		return;
	}

	server::MessageBatchInsertRequest request_data(json_data);
	server::MessageBatchInsertResponse response_data; 
	if (request_data.async)
	{
		grpc::GRPCServerTask* sub_task = task_of(response);
		std::string opt_id;
		gutil::IdUtil::nextUID(opt_id);
		response_data.opt_id = opt_id;
		response_data.StatusCode = StatusOK;
		response_data.StatusMsg = "Operation Success.";
		sub_task->add_callback([request_data, opt_id](GRPCTask *)
		{
			server::MessageBatchInsertResponse response_data;
			response_data.opt_id = opt_id;
			apiUtil->write_access_log(request_data.op, request_data.remote_ip, 0, "Operation success", opt_id);
			if (clusterManagerPtr->isEnable())
				server::ApiHandler::batch_insert_cluster(apiUtil, clusterManagerPtr, request_data, response_data);
			else
				server::ApiHandler::batch_insert(apiUtil, request_data, response_data);

			if (response_data.StatusCode == StatusOK)
				apiUtil->update_access_log(0, response_data.StatusMsg, response_data.opt_id, 1, response_data.successNum, response_data.failedNum);
			else
				apiUtil->update_access_log(response_data.StatusCode, response_data.StatusMsg, response_data.opt_id, -1, 0, 0);
			std::string callback = request_data.callback;
			if (!callback.empty())
			{
				std::string json_str;
				response_data.toJsonString(json_str);
				string res;
				HttpUtil::Post(callback, json_str, res);
			}
		});
	}
	else
	{
		if (clusterManagerPtr->isEnable())
			server::ApiHandler::batch_insert_cluster(apiUtil, clusterManagerPtr, request_data, response_data);
		else
			server::ApiHandler::batch_insert(apiUtil, request_data, response_data);
	}
	
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void batch_remove_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, nlohmann::json &json_data)
{
	if (clusterManagerPtr->isEnable() && clusterManagerPtr->isFollower())
	{
		redirect_handler(request, response, series);
		return;
	}

	server::MessageBatchRemoveRequest request_data(json_data);
	server::MessageBatchRemoveResponse response_data; 
	if (request_data.async)
	{
		grpc::GRPCServerTask* sub_task = task_of(response);
		std::string opt_id;
        gutil::IdUtil::nextUID(opt_id);
		response_data.opt_id = opt_id;
		response_data.StatusCode = StatusOK;
		response_data.StatusMsg = "Operation Success.";
		sub_task->add_callback([request_data, opt_id](GRPCTask *)
		{
			server::MessageBatchRemoveResponse response_data;
			response_data.opt_id = opt_id;
			apiUtil->write_access_log(request_data.op, request_data.remote_ip, 0, "Operation success", opt_id);
			if (clusterManagerPtr->isEnable())
				server::ApiHandler::batch_remove_cluster(apiUtil, clusterManagerPtr, request_data, response_data);
			else
				server::ApiHandler::batch_remove(apiUtil, request_data, response_data);

			if (response_data.StatusCode == StatusOK)
				apiUtil->update_access_log(0, response_data.StatusMsg, response_data.opt_id, 1, response_data.successNum, response_data.failedNum);
			else
				apiUtil->update_access_log(response_data.StatusCode, response_data.StatusMsg, response_data.opt_id, -1, 0, 0);
			std::string callback = request_data.callback;
            if (!callback.empty())
            {
                std::string json_str;
                response_data.toJsonString(json_str);
                string res;
                HttpUtil::Post(callback, json_str, res);
            }
		});
	}
	else
	{
		if (clusterManagerPtr->isEnable())
			server::ApiHandler::batch_remove_cluster(apiUtil, clusterManagerPtr, request_data, response_data);
		else
			server::ApiHandler::batch_remove(apiUtil, request_data, response_data);
	}
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
	}
}

void rename_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	try
	{
		std::string db_name = JsonUtil::jsonParam(json_data, "db_name");
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		std::string new_name = JsonUtil::jsonParam(json_data, "new_name");
		if (apiUtil->check_param_value("new_name", new_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if (apiUtil->rename_databaseinfo(db_name, new_name, msg)) {
			msg = "Database rename successfully.";
			response->Success(msg);
		} else {
			response->Error(StatusOperationFailed, msg);
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "Rename fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

void reason_manage_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageReasonManageResponse response_data; 
	server::ApiHandler::reason_manage(apiUtil, response_data, json_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
	server::MessageShowUserResponse response_data; 
	server::ApiHandler::show_users(apiUtil, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void user_manage_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageUserManageRequest request_data(json_data);
	server::MessageUserManageResponse response_data; 
	server::ApiHandler::user_manage(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void user_privilege_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageUserPrivilegeManageRequest request_data(json_data);
	server::MessageUserPrivilegeManageResponse response_data; 
	server::ApiHandler::user_privilege_manage(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void user_password_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageUserPasswordRequest request_data(json_data);
	server::MessageUserPasswordResponse response_data; 
	server::ApiHandler::user_passworrd(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void txn_log_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageTxnLogRequest request_data(json_data);
	server::MessageTxnLogResponse response_data; 
	server::ApiHandler::txn_log(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void query_log_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageQueryLogRequest request_data(json_data);
	server::MessageQueryLogResponse response_data; 
	server::ApiHandler::query_log(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
	server::MessageQueryLogDateRequest request_data;
	server::MessageQueryLogDateResponse response_data; 
	server::ApiHandler::query_log_date(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void access_log_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageAccessLogRequest request_data(json_data);
	server::MessageAccessLogResponse response_data; 
	server::ApiHandler::access_log(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
	server::MessageAccessLogDateRequest request_data;
	server::MessageAccessLogDateResponse response_data; 
	server::ApiHandler::access_log_date(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void fun_query_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageFunQueryRequest request_data;
	server::MessageFunQueryResponse response_data;
	request_data.funInfo = PFNInfo(json_data["funInfo"]);
	server::ApiHandler::funquery(apiUtil, pfnUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void fun_cudb_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageFunCudbRequest request_data;
	server::MessageFunCudbResponse response_data;
	request_data.type = json_data["type"];
	request_data.funInfo = PFNInfo(json_data["funInfo"]);
	server::ApiHandler::funcudb(apiUtil, pfnUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
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
void fun_review_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageFunReviewRequest request_data;
	server::MessageFunReviewResponse response_data; 
	request_data.funInfo = PFNInfo(json_data["funInfo"]);
	server::ApiHandler::funreview(apiUtil, pfnUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
	}
}

void stat_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	try
	{
		int pid = getpid();
		float cup_usage = gutil::ResourceUtil::get_app_cpu_usage(pid) * 100; // %
		char cup_usage_char[32];
		sprintf(cup_usage_char, "%f", cup_usage);
		float mem_usage = gutil::ResourceUtil::get_app_mem_usage(pid); // MB
		char mem_usage_char[32];
		sprintf(mem_usage_char, "%f", mem_usage);
		uint64_t disk_available = gutil::ResourceUtil::get_disk_free(); // MB
		char disk_available_char[32];
		sprintf(disk_available_char, "%lu", disk_available);
		nlohmann::json resp_data = nlohmann::json {
			{"StatusCode", 0},
			{"StatusMsg", "success"},
			{"cup_usage", cup_usage_char},
			{"mem_usage", mem_usage_char},
			{"disk_available", disk_available_char}
		};
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		std::string error = "stat fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

void checkOperationState_task(const GRPCReq *request, GRPCResp *response, nlohmann::json &json_data)
{
	server::MessageCheckOperationStateRequest request_data(json_data);
	server::MessageCheckOperationStateResponse response_data; 
	server::ApiHandler::checkOperationState(apiUtil, request_data, response_data);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else if (response_data.operation == "query")
	{
		if (response_data.state == 0)
		{
			response->Success("The query task is not complete");
		} 
		else
		{
			ifstream file(response_data.queryfilepath);
			if (!file.is_open())
			{
				SLOG_ERROR("open result file failed: " + response_data.queryfilepath);
				response->Error(server::StatusFileReadError, "Read result file failed.");
			}
			else
			{
				nlohmann::json json_result;
				file >> json_result;
				file.close();
				response->Json(json_result);
			}
		}
	} 
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->Json(json_str);
	}
}

void cluster_heartbeat_task(const GRPCReq *request, GRPCResp *response)
{
	nlohmann::json json_data;
	parseRequest(request, json_data);
	std::string expection = JsonUtil::jsonParam(json_data, "operation");
	const cluster::ClusterOperation expectionEnum = cluster::ClusterOperationHandle::to_enum(expection);
	server::MessageClusterRequest request_data(json_data);
	switch (expectionEnum)
	{
		case cluster::ClusterOperation_Compare:
			// compare term and index with leader
			server::ApiHandler::cluster_heartbeat_compare(apiUtil, clusterManagerPtr, request_data);
			response->Success("ok");
			break;
		case cluster::ClusterOperation_Prepare:
			// prepare for log append
			// check local db is available
			server::ApiHandler::cluster_heartbeat_prepare(apiUtil, clusterManagerPtr, request_data);
			response->Success("ok");
			break;
		case cluster::ClusterOperation_Commit:
			server::ApiHandler::cluster_heartbeat_commit(apiUtil, clusterManagerPtr, request_data);
			response->Success("ok");
			break;
		case cluster::ClusterOperation_Cancel:
			server::ApiHandler::cluster_heartbeat_cancel(apiUtil, clusterManagerPtr, request_data);
			response->Success("ok");
			break;
		case cluster::ClusterOperation_Fail:
			server::ApiHandler::cluster_heartbeat_fail(apiUtil, clusterManagerPtr, request_data);
			response->Success("ok");
			break;
		case cluster::ClusterOperation_Drop:
			server::ApiHandler::cluster_heartbeat_drop(apiUtil, clusterManagerPtr, request_data);
			response->Success("ok");
			break;
		default:
			response->Success("ok");
			break;
	}
}

void cluster_append_task(const GRPCReq *request, GRPCResp *response)
{
	Form &form = request->form();
	server::MessageResponse response_data;
	server::ApiHandler::cluster_append(apiUtil, clusterManagerPtr, form, response_data, _server_port);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		response->Success("ok");
	}
}

void cluster_reply_task(const GRPCReq *request, GRPCResp *response)
{
	nlohmann::json json_data;
	parseRequest(request, json_data);
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	server::MessageClusterReplyRequest resquest_data(json_data);
	server::ApiHandler::cluster_reply(apiUtil, clusterManagerPtr, resquest_data, ip_addr);
	response->Success("ok");
}

void cluster_check_task(const GRPCReq *request, GRPCResp *response)
{
	nlohmann::json json_data;
	parseRequest(request, json_data);
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	server::MessageClusterCheckRequest resquest_data(json_data);
	server::ApiHandler::cluster_check(apiUtil, clusterManagerPtr, resquest_data, ip_addr);
	response->Success("ok");
}

void cluster_recover_task(const GRPCReq *request, GRPCResp *response)
{
	Form &form = request->form();
	server::MessageResponse response_data;
	server::ApiHandler::cluster_recover(apiUtil, clusterManagerPtr, form, response_data, _server_port);
	if (response_data.StatusCode != server::StatusOK)
	{
		response->Error(response_data.StatusCode, response_data.StatusMsg);
	}
	else
	{
		response->Success("ok");
	}
}

void license_import(const GRPCReq *request, GRPCResp *response)
{
	operation_type op_type;
	nlohmann::json json_data = nlohmann::json {
		{"operation", "importLicense"}
	};
	if (checkRequest(request, response, op_type, json_data, false) == false)
	{
		return;
	}
	// filename : filecontent
	std::string filename = JsonUtil::jsonParam(json_data, "filename");
	std::string msg;
	if(filename.empty() || JsonUtil::hasJsonParam(json_data, "file") == false)
	{
		msg = "Upload file can not be empty!";
		response->Error(StatusParamIsIllegal, msg);
		return;
	}
	std::string file_suffix = GRPCUtil::fileSuffix(filename);
	if (file_suffix != "lic")
	{
		msg = "The type of license file is not supported!";
		response->Error(StatusOperationFailed, msg);
		return;
	}
	nlohmann::byte_container_with_subtype<std::vector<uint8_t>> file_binary =  json_data["file"].get_binary();
	// remove path info, only return base filename
	std::string file_name = GRPCUtil::fileName(filename);
	size_t pos = file_name.size() - file_suffix.size() - 1;
	std::string file_save_path = GlobalTypedef::upload_path() + file_name.substr(0, pos) + "_" + gutil::TimeUtil::now() + "." + file_suffix;
    WFFileIOTask *pwrite_task = WFTaskFactory::create_pwrite_task(
		file_save_path, static_cast<const void *>(file_binary.data()), file_binary.size(), 0, [file_save_path](WFFileIOTask *pwrite_task){
			long ret = pwrite_task->get_retval();
			GRPCServerTask *server_task = task_of(pwrite_task);
			GRPCResp *resp = server_task->get_resp();
			resp->headers["Access-Control-Allow-Origin"] = "*";
			if (pwrite_task->get_state() != WFT_STATE_SUCCESS || ret < 0)
			{
				resp->Error(StatusFileWriteError);
			} 
			else
			{
				string msg;
				if(apiUtil->import_license(file_save_path, msg))
				{
					server::MessageLicenseResponse respData(server::StatusCode::StatusOK, msg);
					respData.json = apiUtil->get_license();
					resp->Json(respData.json);
				}
				else 
				{
					Util::remove_path(file_save_path);
					resp->Error(server::StatusCode::StatusLicenseInvalid, msg);
				}
			}
	});
	auto* rpc_task = task_of(response);
    **rpc_task << pwrite_task;
}

void license_info(const GRPCReq *request, GRPCResp *response)
{
	operation_type op_type;
	nlohmann::json json_data = nlohmann::json {
		{"operation", "queryLicense"}
	};
	if (checkRequest(request, response, op_type, json_data, false) == false)
	{
		return;
	}
	LicenseInfo lic = apiUtil->get_license();
	server::MessageLicenseResponse respData(server::StatusCode::StatusOK, "success");
	if (lic.isvalid) 
	{
		respData.json = lic;
	}
	else
	{
		respData.json["isvalid"] = lic.isvalid;
		respData.json["desc"] = lic.desc;
	}
	nlohmann::json respJson;
	respData.toJson(respJson);
	response->Json(respJson);
}

void license_remove(const GRPCReq *request, GRPCResp *response)
{
	operation_type op_type;
	nlohmann::json json_data = nlohmann::json {
		{"operation", "removeLicense"}
	};
	if (checkRequest(request, response, op_type, json_data, false) == false)
	{
		return;
	}
	std::string msg;
	if(apiUtil->remove_license(msg))
	{
		response->Success(msg);
	}
	else
	{
		response->Error(server::StatusCode::StatusOperationFailed, msg);
	}
}