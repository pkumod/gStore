#include <stdio.h>
#include "workflow/WFFacilities.h"
#include "../Api/HttpUtil.h"
#include "../GRPC/grpc_server.h"
#include "../GRPC/grpc_status_code.h"
#include "../GRPC/grpc_operation.h"
#include "../Api/APIUtil.h"
#include "../Api/PFNUtil.h"
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

std::string _db_home;

std::string _db_suffix;

std::string _server_port;

std::string _server_deamon;

useconds_t _max_wait_time = 3 * 1000 * 1000;

bool startServer(bool background = false);
bool stopServer();
void initialServer(uint16_t port, bool background = false);
void releaseGlobalPtr(bool renew = false);
void register_service(GRPCServer &grpcServer);

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
void login_task(const GRPCReq *request, GRPCResp *response, std::string &ip);
void test_connect_task(const GRPCReq *request, GRPCResp *response);
void core_version_task(const GRPCReq *request, GRPCResp *response);
void ip_manage_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void refresh_conf_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
// for db
void init_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void show_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void load_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void unload_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void monitor_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void build_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, Json &json_data);
void drop_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, Json &json_data);
void backup_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void backup_path_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void restore_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void query_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, Json &json_data);
void export_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void begin_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void tquery_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void commit_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void rollback_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void checkpoint_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void batch_insert_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, Json &json_data);
void batch_remove_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, Json &json_data);
void rename_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
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
void checkOperationState_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
// for personalized function
void fun_query_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void fun_cudb_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void fun_review_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
// for system stat
void stat_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
// for reason engine
void reason_manage_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
// for cluster
void cluster_heartbeat_task(const GRPCReq *request, GRPCResp *response);
void cluster_append_task(const GRPCReq *request, GRPCResp *response);
void cluster_reply_task(const GRPCReq *request, GRPCResp *response);
void cluster_check_task(const GRPCReq *request, GRPCResp *response);
void cluster_recover_task(const GRPCReq *request, GRPCResp *response);

// common function
std::string to_json_string(const Json& json);
std::string jsonParam(const Json &json, const std::string &key, const std::string& default_val = "");
int32_t jsonParam(const Json &json, const std::string &key, const int32_t &default_val);
uint32_t jsonParam(const Json &json, const std::string &key, const uint32_t &default_val);
int64_t jsonParam(const Json &json, const std::string &key, const int64_t &default_val);
uint64_t jsonParam(const Json &json, const std::string &key, const uint64_t &default_val);
bool jsonBoolParam(const Json &json, const std::string &key, const bool &default_val);
bool hasJsonParam(const Json &json, const std::string &key);
void parseRequest(const GRPCReq *request, Json &json_data);

std::string to_json_string(const Json& json)
{
	rapidjson::StringBuffer resBuffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> resWriter(resBuffer);
    json.Accept(resWriter);
	return resBuffer.GetString();
}

std::string jsonParam(const Json &json, const std::string &key, const std::string& default_val)
{
	if (json.HasMember(key.c_str()))
	{
		auto& value = json[key.c_str()];
		if (value.IsString()) {	    
			return value.GetString();
		} else if (value.IsInt()) {
			return std::to_string(value.GetInt());
		} else if (value.IsUint()) {
			return std::to_string(value.GetUint());
		} else if (value.IsInt64()) {
			return std::to_string(value.GetInt64());
		} else if (value.IsUint64()) {
			return std::to_string(value.GetUint64());
		} else if (value.IsDouble()) {
			return std::to_string(value.GetDouble());
		} else if(value.IsFloat()){
			return std::to_string(value.GetFloat());
		}else if (value.IsTrue()) {
			return "true";
		} else if (value.IsFalse()) {
			return "false";
		}
	}
	return default_val;
}

int32_t jsonParam(const Json &json, const std::string &key, const int32_t &default_val)
{
	if (json.HasMember(key.c_str()))
	{
		auto& value = json[key.c_str()];
		if (value.IsInt()) {
			return value.GetInt();
		} else if (value.IsString()) {
			return std::stoi(value.GetString());
		}
	}
	return default_val;
}

uint32_t jsonParam(const Json &json, const std::string &key, const uint32_t &default_val)
{
	if (json.HasMember(key.c_str()))
	{
		auto& value = json[key.c_str()];
		if (value.IsUint()) {
			return value.GetUint();
		} else if (value.IsString()) {
			uint32_t max = std::numeric_limits<uint32_t>::max();
			int64_t val = std::stoll(value.GetString());
			if (val > max) {
				return default_val;
			}
			return val;
		}
	}
	return default_val;
}

int64_t jsonParam(const Json &json, const std::string &key, const int64_t &default_val)
{
	if (json.HasMember(key.c_str()))
	{
		auto& value = json[key.c_str()];
		if (value.IsInt64()) {
			return value.GetInt64();
		} else if (value.IsString()) {
			int64_t max = std::numeric_limits<int64_t>::max();
			uint64_t val = std::stoll(value.GetString());
			if (val > max) {
				return default_val;
			}
			return val;
		}
	}
	return default_val;
}

uint64_t jsonParam(const Json &json, const std::string &key, const uint64_t &default_val)
{
	if (json.HasMember(key.c_str()))
	{
		auto& value = json[key.c_str()];
		if (value.IsInt64()) {
			return value.GetInt64();
		} else if (value.IsString()) {
			return std::stoul(value.GetString());
		}
	}
	return default_val;
}

bool jsonBoolParam(const Json &json, const std::string &key, const bool &default_val)
{
	if (json.HasMember(key.c_str())) {
		auto& value = json[key.c_str()];
		if (value.IsBool()) {
			SLOG_DEBUG("json[" + key + "]=" + to_string(value.GetBool()));
			return value.GetBool();
		} else if (value.IsString()) {
			std::string v = value.GetString();
			SLOG_DEBUG("json[" + key + "]=" + v);
			return v == "true" || v == "1";
		} else if (value.IsInt()) {
			SLOG_DEBUG("json[" + key + "]=" + to_string(value.GetInt()));
			return value.GetInt() == 1;
		}
	}
	return default_val;
}

bool hasJsonParam(const Json &json, const std::string &key)
{
	return json.HasMember(key.c_str());
}

void parseRequest(const GRPCReq *request, Json &json_data)
{
	json_data.SetObject();
	Json::AllocatorType &allocator = json_data.GetAllocator();
	if (request->contentType() == APPLICATION_JSON) //for application/json
	{
		Json &json = request->json();
		json_data.CopyFrom(json, allocator);
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
				StringUtil::url_decode(v);
			}
			json_data.AddMember(rapidjson::Value().SetString(iter->first.c_str(), allocator).Move(), rapidjson::Value().SetString(v.c_str(), allocator).Move(), allocator);
			iter++;
		}
	}
	else // for get
	{
		std::map<std::string, std::string> params = request->queryList();
		if (params.empty() == false)
		{
			std::map<std::string, std::string>::iterator iter = params.begin();
			std::string v;
			while (iter != params.end())
			{
				v = iter->second;
				if (UrlEncode::is_url_encode(v))
				{
					StringUtil::url_decode(v);
				}
				json_data.AddMember(rapidjson::Value().SetString(iter->first.c_str(), allocator).Move(), rapidjson::Value().SetString(v.c_str(), allocator).Move(), allocator);
				iter++;
			}
		}
	}
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
	_db_home = util.getConfigureValue("db_home");
	_db_suffix = util.getConfigureValue("db_suffix");
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
		cout << "\t-db,--database[option],\t\tthe database name.Default value is empty."<< endl;
		cout << "\t-c,--csr[option],\t\tEnable CSR Struct or not. 0 denote that false, 1 denote that true. Default value is 0." << endl;
		cout << "\t-t,--stop\t\tSafe shutdow gServer." << endl;
		cout << "\t-r,--restart\t\tRestart gServer." << endl;
		cout << "\t-k,--kill\t\tForce shutdow gServer." << endl;
		cout << "\t-b,--background\t\tStart gServer in the background." << endl;
		cout << "\t-S,--status\t\tShow gServer status." << endl;
		cout << endl;
		return 0;
	}
	else if ((command == "-s" || command == "--start") && (argc == 1 || argc == 2 || argc == 4 || argc == 6))
	{
		// check server thread
		httpentities::CheckRequest check_request;
		httpentities::CheckResponse check_response = HttpUtil::check(API_URL, check_request);
		if (check_response.success())
		{
			cout << "the server already running." << endl;
			return 1;
		}
		if (startServer(false))
		{
			sleep(1);
			// load db
			if(argc == 4 || argc == 6)
			{
				string db_name = Util::getArgValue(argc, argv, "db", "database");
				string csr = Util::getArgValue(argc, argv, "c", "csr", "0");
				httpentities::LoadRequest load_requst(db_name, csr);
				httpentities::LoadResponse load_response = HttpUtil::load(API_URL, true, load_requst);
				if (load_response.success())
				{
					SLOG_INFO("load " + db_name + " success.");
				}
				else
				{
					SLOG_INFO("load failed: unknow error.");
				}
			}
			else
			{
				SLOG_INFO("No database is loaded!");
			}
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else if (command == "-t" || command == "--stop")
	{
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
		httpentities::CheckRequest check_request;
		httpentities::CheckResponse check_response = HttpUtil::check(API_URL, check_request);
		if(check_response.success()) {
			// read pid file
			ifstream ifp(PID_PATH);
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
		Util::remove_path(PID_PATH);
		return 0;
	}
	else if (command == "-S" || command == "--status")
	{
		// show server status
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
		cout << "\tDocs: https://www.gstore.cn" << endl;
		return 0;
	}
	else if (command == "-b" || command == "--background")
	{
		// check server thread
		httpentities::CheckRequest check_request;
		httpentities::CheckResponse check_response = HttpUtil::check(API_URL, check_request);
		if (check_response.success())
		{
			cout << "the server already running." << endl;
			return -1;
		}
		if (startServer(true))
		{
			// load db
			if(argc == 4 || argc == 6)
			{
				string db_name = Util::getArgValue(argc, argv, "db", "database");
				string csr = Util::getArgValue(argc, argv, "c", "csr", "0");
				httpentities::LoadRequest load_requst(db_name, csr);
				httpentities::LoadResponse load_response = HttpUtil::load(API_URL, true, load_requst);
				if (load_response.success())
				{
					SLOG_INFO("load " + db_name + " success.");
				}
				else
				{
					SLOG_INFO("load failed: unknow error.");
				}
			}
			else
			{
				SLOG_INFO("No database is loaded!");
			}
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
			SLOG_INFO("Server started at port " + _server_port);
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
	std::string pid_path = PID_PATH;
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
		rows.push_back({"heartbeat", apiUtil->get_configure_value("cluster_heartbeat") + " s"});
		rows.push_back({"relpy_timeout", apiUtil->get_configure_value("cluster_relpy_timeout") + " s"});
		rows.push_back({"data_path", apiUtil->get_configure_value("cluster_data_path")});
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
	if (!background)
		SLOG_INFO("Server started at port " + _server_port);
	// handle the Ctrl+C signal
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	wait_group.wait();
	grpcServer.stop();
	apiUtil.reset();
	pfnUtil.reset();
	clusterManagerPtr.reset();
	SLOG_INFO("Server stoped.");
	std::cout.flush();
	exit(EXIT_SUCCESS);
}

void releaseGlobalPtr(bool renew)
{
	SLOG_DEBUG("release global pointer");
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
}

bool stopServer()
{
	string pid_path = PID_PATH;
	SLOG_DEBUG("pid path: " + pid_path);
	if (!Util::file_exist(pid_path))
	{
		return false;
	}
	string system_user = apiUtil->get_configure_value("system_username");
	string pid;
	string system_password;
	ifstream in;
	in.open(pid_path.c_str(), ios::in);
	getline(in, pid, '\n');
	getline(in, system_password, '\n');
	in.close();
	SLOG_DEBUG("port: " + _server_port + ", system user: " + system_user + ", password: " + system_password);
	httpentities::ShutdownRequest shutdwon_request(system_user, system_password);
	httpentities::ShutdownResponse shutdown_response = HttpUtil::shutdown(OFF_URL, shutdwon_request);
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
}

void shutdown(const GRPCReq *request, GRPCResp *response)
{
	// check ip address
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	std::string ipCheckResult;
	if (apiUtil->check_access_ip(ip_addr, 0, ipCheckResult) == false)
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
	if (username != apiUtil->get_system_username())
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
	// check ip address
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	std::string ipCheckResult;
	if (apiUtil->check_access_ip(ip_addr, 0, ipCheckResult) == false)
	{
		SLOG_DEBUG(ipCheckResult);
		response->Error(StatusIPBlocked, ipCheckResult);
		return;
	}
	SLOG_DEBUG("Content-Type:" + ContentType::to_str(request->contentType()));
	if (request->contentType() != MULTIPART_FORM_DATA) //for application/json
	{
		response->Error(StatusFileReadError, "Content-Type not match");
		return;
	}
	SLOG_INFO("receive [uploadfile] request from " << ip_addr);
	std::string ss;
	ss += "\n==================== http-api ====================";
	ss += "\n  Content-Type: " + ContentType::to_str(request->contentType());
	ss += "\n  Accept-Encoding: " + request->header("Accept-Encoding");
	ss += "\n  method: " +  string(request->get_method());
	ss += "\n  httpVersion: " +  string(request->get_http_version());
	ss += "\n  requestUri: " +  string(request->get_request_uri());
	ss += "\n  Content-Length: " + request->header("Content-Length");
	SLOG_DEBUG(ss);
	Form &form = request->form();
	if (form.empty())
	{   
		response->Error(StatusFileReadError, "Form data is empty");
		return;
	}
	std::string error;
	if (form.find("username") == form.end() || form.find("password") == form.end())
	{
		error = "username or password is empty";
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	std::string username = form.at("username").second;
	std::string password = form.at("password").second;
	apiUtil->check_param_value("username", username, error);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	apiUtil->check_param_value("password", password, error);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	// filename : filecontent
	std::pair<std::string, std::string>& fileinfo = form.at("file");
	if(fileinfo.first.empty())
	{
		error = "Upload file can not be empty!";
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	if(request->has_content_length_header())
	{
		size_t content_length = stoul(request->header("Content-Length"), nullptr, 0);
		size_t max_body_size = apiUtil->get_upload_max_body_size();
		if (content_length > max_body_size)
		{
			SLOG_DEBUG("File size is " + to_string(content_length) + " byte, allowed max size " + to_string(max_body_size) + " byte!");
			error = "Upload file more than max_body_size!";
			response->Error(StatusOperationFailed, error);
			return;
		}
	}
	std::string file_suffix = GRPCUtil::fileSuffix(fileinfo.first);
	if (!apiUtil->check_upload_allow_compress_packages(file_suffix) && apiUtil->check_upload_allow_extensions(file_suffix) == false)
	{
		error = "The type of upload file is not supported!";
		response->Error(StatusOperationFailed, error);
		return;
	}
	// redirect to cluster
	if (clusterManagerPtr->isEnable() && clusterManagerPtr->isFollower())
	{
		redirect_handler(request, response, series);
		return;
	}
	// remove path info, only return base filename
	std::string file_name = GRPCUtil::fileName(fileinfo.first);
	size_t pos = file_name.size() - file_suffix.size() - 1;
	std::string file_dst = apiUtil->get_upload_path() + file_name.substr(0, pos) + "_" + Util::getTimeString2() + "." + file_suffix;
	std::string notify_msg = "{\"StatusCode\":0, \"StatusMsg\":\"success\", \"filepath\": \""+file_dst+"\"}";
	response->Save(file_dst, std::move(fileinfo.second), notify_msg);
}

void download_file(const GRPCReq *request, GRPCResp *response)
{
	// check ip address
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	std::string ipCheckResult;
	if (apiUtil->check_access_ip(ip_addr, 0, ipCheckResult) == false)
	{
		SLOG_DEBUG(ipCheckResult);
		response->Error(StatusIPBlocked, ipCheckResult);
		return;
	}
	Json json_data;
	json_data.SetObject();
	Json::AllocatorType &allocator = json_data.GetAllocator();
	if (request->contentType() == MULTIPART_FORM_DATA) //for multipart/form-data
	{
		Form &form = request->form();
		if (form.empty())
		{   
			response->Error(StatusFileReadError, "Form data is empty");
			return;
		}
		for (Form::iterator iter = form.begin(); iter != form.end(); iter++)
		{
			string v = form.at(iter->first).second;
			json_data.AddMember(rapidjson::Value().SetString(iter->first.c_str(), allocator).Move(), rapidjson::Value().SetString(v.c_str(), allocator).Move(), allocator);
		}
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
				StringUtil::url_decode(v);
			}
			json_data.AddMember(rapidjson::Value().SetString(iter->first.c_str(), allocator).Move(), rapidjson::Value().SetString(v.c_str(), allocator).Move(), allocator);
			iter++;
		}
	}
	else // for get
	{
		std::map<std::string, std::string> params = request->queryList();
		if (params.empty() == false)
		{
			std::map<std::string, std::string>::iterator iter = params.begin();
			std::string v;
			while (iter != params.end())
			{
				v = iter->second;
				if (UrlEncode::is_url_encode(v))
				{
					StringUtil::url_decode(v);
				}
				json_data.AddMember(rapidjson::Value().SetString(iter->first.c_str(), allocator).Move(), rapidjson::Value().SetString(v.c_str(), allocator).Move(), allocator);
				iter++;
			}
		}
	}
	SLOG_INFO("receive [downloadfile] request from " << ip_addr);
	std::string ss;
	ss += "\n==================== http-api ====================";
	ss += "\n  Content-Type: " + ContentType::to_str(request->contentType());
	ss += "\n  Accept-Encoding: " + request->header("Accept-Encoding");
	ss += "\n  method: " +  string(request->get_method());
	ss += "\n  httpVersion: " +  string(request->get_http_version());
	ss += "\n  requestUri: " +  string(request->get_request_uri());
	if (!request->body().empty())
	{
		ss += "\n  request_body: " + request->body();
	}
	ss += "\n==================================================";
	SLOG_DEBUG(ss);
	std::string error;
	std::string username = jsonParam(json_data, "username");
	std::string password = jsonParam(json_data, "password");
	std::string filepath = jsonParam(json_data, "filepath");
	apiUtil->check_param_value("username", username, error);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	apiUtil->check_param_value("password", password, error);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
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
		if (StringUtil::start_with(exact_path, cur_path) == false)
		{
			error = "Download file must in the gstore home dir";
			response->Error(StatusOperationFailed, error);
			return;
		}
		// std::string compress = jsonParam(json_data, "compress", "0");
		// if (compress == "1") // compress to zip file
		// {
		// 	string filename = GRPCUtil::fileName(exact_path);
		// 	string zipfile = exact_path + ".zip";
		// 	string cmd = "zip " + zipfile + " " + exact_path;
		// 	system(cmd.c_str());
		// 	response->File(zipfile);
		// }
		// else 
		// {
		response->File(exact_path);
		// }
	}
	else
	{
		error = "Download file not exists";
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
	string cluster_ip_check = apiUtil->get_configure_value("cluster_ip_check", "off");
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
	if (apiUtil->check_indentity(username, password, "0", checkidentityresult) == false)
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
	Json json_data;
	parseRequest(request, json_data);
	bool is_inner = jsonBoolParam(json_data, "inner", false);
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
		sparql = jsonParam(json_data, "sparql");
		if (!apiUtil->check_param_value("sparql", sparql, msg))
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		bool query_rt = false;
		uint64_t query_time = Util::get_cur_time();
		query_rt = apiUtil->query_sys_db(sparql, rs);
		query_time = Util::get_cur_time() - query_time;
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
		rs.release();
		string json_data_str = json_data.dump();
		Json resp_data;
		Json::AllocatorType& allocator = resp_data.GetAllocator();
		resp_data.IsObject();
		resp_data.Parse(json_data_str.c_str());
		if (!resp_data.HasParseError())
		{
			uint32_t rs_ansNum = rs.ansNum;
			std::string thread_id = Util::getThreadID();
			rs.release();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", "success", allocator);
			resp_data.AddMember("AnsNum", rs_ansNum, allocator);
			resp_data.AddMember("OutputLimit", -1, allocator);
			resp_data.AddMember("ThreadId", StringRef(thread_id.c_str()), allocator);
			resp_data.AddMember("QueryTime", StringRef(to_string(query_time).c_str()), allocator);
		} 
		else
		{
			msg = "Query fail: the result parse error.";
			resp_data.AddMember("StatusCode", StatusOperationFailed, allocator);
			resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
		}
		response->Json(resp_data);
	}
	else
	{
		response->Error(StatusOperationUndefined);
	}
}

void api(const GRPCReq *request, GRPCResp *response, SeriesWork *series)
{
	// check ip address
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	std::string ipCheckResult;
	if (apiUtil->check_access_ip(ip_addr, 1, ipCheckResult) == false)
	{
		SLOG_DEBUG(ipCheckResult);
		response->Error(StatusIPBlocked, ipCheckResult);
		return;
	}
	Json json_data;
	parseRequest(request, json_data);
	Json::AllocatorType &allocator = json_data.GetAllocator();
	// add remote_ip param
	json_data.AddMember("remote_ip", StringRef(ip_addr.c_str()), allocator);
	std::string operation = jsonParam(json_data, "operation");
	operation_type op_type = OperationType::to_enum(operation);
	if (op_type != OP_LOGIN && op_type != OP_TEST_CONNECT)
	{
		if (apiUtil->check_access_ip(ip_addr, 2, ipCheckResult) == false)
		{
			SLOG_DEBUG(ipCheckResult);
			response->Error(StatusIPBlocked, ipCheckResult);
			return;
		}
	}
	SLOG_INFO("receive [" << operation << "] request from " << ip_addr);
	std::string ss;
	ss += "\n==================== http-api ====================";
	ss += "\n  Content-Type: " + ContentType::to_str(request->contentType());
	ss += "\n  Accept-Encoding: " + request->header("Accept-Encoding");
	ss += "\n  method: " +  string(request->get_method());
	ss += "\n  httpVersion: " +  string(request->get_http_version());
	ss += "\n  requestUri: " +  string(request->get_request_uri());
	if (!request->body().empty())
	{
		ss += "\n  request_body: " + request->body();
	}
	ss += "\n==================================================";
	SLOG_DEBUG(ss);
	// add callback task for access log start
	auto *operation_ptr = new std::string(operation);
	auto *ip_ptr = new std::string(ip_addr);
	rpc_task->add_callback([operation_ptr,ip_ptr](GRPCTask *task) {
		GRPCResp *resp = task->get_resp();
		if (*operation_ptr != "build" && *operation_ptr != "batchInsert" && *operation_ptr != "batchRemove" 
			&& *operation_ptr != "backup" && *operation_ptr != "restore")
			apiUtil->write_access_log(*operation_ptr, *ip_ptr, resp->resp_code, resp->resp_msg);
		else if (resp->resp_code != 0)
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
	std::string encryption = jsonParam(json_data, "encryption");
	std::string db_name = jsonParam(json_data, "db_name");
	bool is_inner = jsonBoolParam(json_data, "inner", false);
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
			return;
		}
	}
	// check privilege
	if (username != ROOT_USERNAME)
	{
		need_check_privilege = true;
	}
	if (need_check_privilege && apiUtil->check_privilege(username, operation, db_name) == 0)
	{
		std::string msg = "You have no " + operation + " privilege, operation failed";
		response->Error(StatusOperationConditionsAreNotSatisfied, msg);
		return;
	}
	// api operation
	switch (op_type)
	{
	case OP_LOGIN:
		login_task(request, response, ip_addr);
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
void login_task(const GRPCReq *request, GRPCResp *response, std::string &ip)
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
		string product_name = Util::getConfigureValue("product_name");
		std::transform(product_name.begin(), product_name.end(), product_name.begin(), ::tolower);
		if (product_name != "gstore") {
			licensetype = "";
		}
		resp_data.AddMember("licensetype", StringRef(licensetype.c_str()), allocator);
		string cur_path = Util::get_cur_path();
		resp_data.AddMember("RootPath", StringRef(cur_path.c_str()), allocator);
		resp_data.AddMember("type", HTTP_TYPE, allocator);
		apiUtil->reset_access_ip_error_num(ip);
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
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		string version = Util::getConfigureValue("version");
		resp_data.AddMember("CoreVersion", StringRef(version.c_str()), allocator);
		string licensetype = Util::getConfigureValue("licensetype");
		string product_name = Util::getConfigureValue("product_name");
		std::transform(product_name.begin(), product_name.end(), product_name.begin(), ::tolower);
		if (product_name != "gstore") {
			licensetype = "";
		}
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
			vector<string> ip_list;
			apiUtil->ip_list(IPtype, ip_list);
			size_t count = ip_list.size();
			Json resp_data;
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			rapidjson::Value responseBody(kObjectType);
			rapidjson::Value ips(kArrayType);
			for (size_t i = 0; i < count; i++)
			{
				ips.PushBack(rapidjson::Value().SetString(ip_list[i].c_str(), allocator).Move(), allocator);
			}

			responseBody.AddMember("ip_type", rapidjson::Value().SetString(IPtype.c_str(), allocator).Move(), allocator);
			responseBody.AddMember("ips", ips, allocator);

			resp_data.SetObject();
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
void refresh_conf_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
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
void init_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	std::string db_names = jsonParam(json_data, "db_names");
	if (db_names.empty())
	{
		response->Error(StatusParamIsIllegal, "db_names can't be empty");
		return;
	}
	std::string username = json_data["username"].GetString();
	std::string built_time = Util::get_date_time();
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
		std::string db_path = _db_home + db_name + _db_suffix;
		if(!Util::dir_exist(db_path))
		{
			db_info["status"] = "1";
			db_info["msg"] = db_name + _db_suffix + " not exist.";
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
	Json resp_data;
	resp_data.SetObject();
	resp_data.Parse(response_data.dump().c_str());
	response->Json(resp_data);
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

		vector<shared_ptr<DatabaseInfo>> array;
		apiUtil->get_databaseinfos(username, array);

		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		size_t count = array.size();
		
		rapidjson::Value array_data(rapidjson::kArrayType);
		for (size_t i = 0; i < count; i++)
		{
			shared_ptr<DatabaseInfo> dbInfo = array[i];
			array_data.PushBack(dbInfo->toJSON(allocator).Move(), allocator);
		}
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
	server::MessageLoadRequest request_data(json_data);
	server::MessageLoadResponse response_data; 
	server::ApiHandler::load(apiUtil, request_data, response_data);
	if (response_data.status_code != server::StatusOK)
	{
		response->Error(response_data.status_code, response_data.status_msg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->nlohmannJson(json_str);
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
			db_info->setStatus(DatabaseStatus::AREADY_BUILT);
			db_info->getDatabase()->unload();
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
		std::string disk = jsonParam(json_data, "disk");
		// check the param value is legal or not.
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
		shared_ptr<DatabaseInfo> database_info;
		apiUtil->get_databaseinfo(db_name, database_info);
		if (apiUtil->rdlock_databaseinfo(database_info) == false)
		{
			msg = "Unable to monitor due to loss of lock";
			response->Error(StatusLossOfLock, msg);
			return;
		}
		shared_ptr<Database> current_database = database_info->getDatabase();
		current_database->loadDBInfoFile();
		current_database->loadStatisticsInfoFile();
		apiUtil->unlock_databaseinfo(database_info);
		std::string creator = database_info->getCreator();
		std::string time = database_info->getTime();
		unordered_map<string, unsigned long long> umap = current_database->getStatisticsInfo();
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		rapidjson::Value subjectList(kArrayType);
		for (auto &kv : umap)
		{
			rapidjson::Value item(kObjectType);
			item.AddMember("name", rapidjson::Value().SetString(Util::clear_angle_brackets(kv.first).c_str(), allocator), allocator);
			item.AddMember("value", rapidjson::Value().SetUint64(kv.second), allocator);
			subjectList.PushBack(item.Move(), allocator);
		}
		// /use JSON format to send message
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
		unsigned diskUsed = 0;
		if (disk != "0") 
		{
			string db_path = _db_home + db_name + _db_suffix;
			string real_path = Util::getExactPath(db_path.c_str());
			if (!real_path.empty()) {
				long long unsigned count_size_byte = Util::count_dir_size(real_path.c_str());
				// byte to MB
				diskUsed = count_size_byte>>20;
			}
		}
		resp_data.AddMember("diskUsed", diskUsed, allocator);
		resp_data.AddMember("subjectList", subjectList, allocator);
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
void build_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, Json &json_data)
{
	try
	{
		if(clusterManagerPtr->isEnable() && clusterManagerPtr->isFollower())
		{
			redirect_handler(request, response, series);
			return;
		}
		std::string db_path = jsonParam(json_data, "db_path");
		std::string msg;
		if (!db_path.empty() && Util::file_exist(db_path) == false)
		{
			msg = "RDF file not exist.";
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		std::string db_name = jsonParam(json_data, "db_name");
		std::string username = jsonParam(json_data, "username");
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		//check the db_name is system
		if (db_name == Util::system_db)
		{
			msg = "The database name can not be system.";
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		// check if database named [db_name] is already built
		if (apiUtil->check_db_built(db_name))
		{
			msg = "database already built.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		// check databse number
		if (apiUtil->check_db_count() == false)
		{
			msg = "The total number of databases more than max_databse_num.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}

		shared_ptr<ofstream> clusterlog = nullptr;
		std::string cluster_db_path;
		std::string logpath;
		uint64 log_index;
		if (clusterManagerPtr->isEnable()) 
		{
			// send [prepare] heartbeat and wait response
			ClusterUpdateType cluster_update_type = ClusterUpdateType::ClusterUpdateType_Build;
			log_index = apiUtil->generateUID();
			clusterManagerPtr->addLog(db_name, log_index, ClusterOperation_Prepare, cluster_update_type);
			bool prepare_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Prepare), true);
			if (!prepare_result)
			{
				msg =  "Less than half of the cluster nodes are confirmed.";
				clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
				SLOG_ERROR(msg);
				response->Error(StatusOperationFailed, msg);
				return;
			}
			cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
			logpath = cluster_db_path + to_string(log_index) + ".log";
			clusterlog = make_shared<ofstream>();
			clusterlog->open(logpath.c_str());
		}
		apiUtil->init_databaseinfo(db_name, username, Util::get_date_time(), DatabaseStatus::BUILDING);
		std::vector<std::string> zip_files;
		std::string unz_dir_path;
		std::string file_suffix = GRPCUtil::fileSuffix(db_path);
		bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
		if (is_zip)
		{
			auto code = CompressUtil::FileHelper::foreachZip(db_path,[](std::string filename)->bool
				{
					if (apiUtil->check_upload_allow_extensions(GRPCUtil::fileSuffix(filename)) == false)
						return false;
					return true;
				});
			if (code != CompressUtil::UnZipOK)
			{
				apiUtil->erase_databaseinfo(db_name);
				msg = "uncompress is failed error.";
				response->Error(code, msg);
				return;
			}
			std::string file_name = GRPCUtil::fileName(db_path);
			size_t pos = file_name.size() - file_suffix.size() - 1;
            unz_dir_path = apiUtil->get_upload_path() + file_name.substr(0, pos) + "_" + Util::getTimeString2();
			mkdir(unz_dir_path.c_str(), 0775);
			CompressUtil::UnCompressZip upfile(db_path, unz_dir_path);
			code = upfile.unCompress();
			if (code != CompressUtil::UnZipOK)
			{
				Util::remove_path(unz_dir_path);
				apiUtil->erase_databaseinfo(db_name);
				msg = "uncompress is failed error.";
				response->Error(code, msg);
				return;
			}
			upfile.getFileList(zip_files, "");
		}
		std::string opt_id = apiUtil->generateUid();
		string remote_ip = task_of(response)->peer_addr();
		string operation = "build";
		msg = "Operation Success.";
		apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
		bool async = jsonBoolParam(json_data, "async", false);
		string callback = jsonParam(json_data, "callback");
		auto build_helper = [db_name,username,unz_dir_path,is_zip,zip_files,db_path,operation,opt_id,async,callback,log_index,clusterlog]
				(GRPCResp *response)
				{
					string _db_path = _db_home + db_name + _db_suffix;
					string database = db_name;
					SLOG_DEBUG("Import dataset to build database...");
					SLOG_DEBUG("db_name: " + database + "\tRDF_data: " + db_path);
					string result;
					shared_ptr<Database> current_database = make_shared<Database>(database);
					// build empty database
					bool flag = current_database->BuildEmptyDB();
					current_database.reset();
					int success_num = 0;
					int nt_file_num = 0;
					if (flag)
					{
						// if zip file then excuse batchInsert
						if (!db_path.empty() || zip_files.size() > 0)
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
								apiUtil->update_access_log(1005, result, opt_id, -1, 0, 0);
								if (!async)
									response->Error(StatusOperationFailed, result);
								current_database.reset();
								return;
							}
							if (!db_path.empty() && !is_zip)
							{
								current_database->batch_insert(db_path, false, nullptr, clusterlog);
								nt_file_num = 1;
							}
							for (std::string rdf_zip : zip_files)
							{
								current_database->batch_insert(rdf_zip, false, nullptr, clusterlog);
							}
							nt_file_num += zip_files.size();
							current_database->save();
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
						apiUtil->update_access_log(1005, result, opt_id, -1, 0, 0);
						if (response)
							response->Json(result);
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
					Util::add_backuplog(db_name);
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
					Util::add_backuplog(db_name);
					apiUtil->update_access_log(0, result, opt_id, 1, success_num, parse_error_num);
					// response data
					rapidjson::Document resp_data;
					resp_data.SetObject();
					rapidjson::Document::AllocatorType &allocator = resp_data.GetAllocator();
					resp_data.AddMember("StatusCode", 0, allocator);
					resp_data.AddMember("StatusMsg", StringRef(result.c_str()), allocator);
					resp_data.AddMember("failed_num", parse_error_num, allocator);
					resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
					if (clusterManagerPtr->isEnable()) 
					{
						// cluster sync task begin
						string log_file_name = to_string(log_index) + ".log";
						if (success_num > 0)
						{
							SLOG_DEBUG("add log appendEntities task, copy num " + to_string(success_num));
							string tmp_dir_path = unz_dir_path;
							bool append_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Append, ClusterUpdateType_Insert, log_file_name), true);
							if (append_result)
							{
								SLOG_DEBUG("response result:\n" << to_json_string(resp_data));
								clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Commit));
								if (response)
								{
									response->Json(resp_data);
								}
								if (!callback.empty())
								{
									string postdata;
									string res;
									postdata += "{\"StatusCode\":\"0\",";
									postdata += "\"StatusMsg\":\"" + result + "\",";
									postdata += "\"failed_num\":\"" + std::to_string(parse_error_num) + "\",";
									postdata += "\"opt_id\":\"" + opt_id + "\"}";
									HttpUtil::Post(callback, postdata, res);
								}
								if (!tmp_dir_path.empty())
								{
									Util::remove_path(tmp_dir_path);
								}
							}
							else
							{
								// follower recover by heartbeat compare
								SLOG_DEBUG("build db follower recover by heartbeat compare:" << db_name);
								response->Json(resp_data);
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
							if (response != nullptr)
							{
								response->Json(resp_data);
							}
						}
						// cluster sync task end
					}
					else
					{
						if (response)
						{
							response->Json(resp_data);
						}
						if (!callback.empty())
						{
							string postdata;
							string res;
							postdata += "{\"StatusCode\":\"0\",";
							postdata += "\"StatusMsg\":\"" + result + "\",";
							postdata += "\"failed_num\":\"" + std::to_string(parse_error_num) + "\",";
							postdata += "\"opt_id\":\"" + opt_id + "\"}";
							HttpUtil::Post(callback, postdata, res);
						}
					}
					
				};
		if (async)
		{
			rapidjson::Document resp_data;
			resp_data.SetObject();
			rapidjson::Document::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
			resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
			response->Json(resp_data);
			thread t(build_helper, nullptr);
			t.detach();
		}
		else
		{
			build_helper(response);
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
void drop_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, Json &json_data)
{
	try
	{
		if(clusterManagerPtr->isEnable() && clusterManagerPtr->isFollower())
		{
			redirect_handler(request, response, series);
			return;
		}
		std::string db_name = jsonParam(json_data, "db_name");
		bool is_backup = jsonBoolParam(json_data, "is_backup", true);
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if (apiUtil->check_db_built(db_name) == false)
		{
			msg = "the database [" + db_name + "] not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		if (apiUtil->check_db_loaded(db_name))
		{
			apiUtil->remove_txn_manager(db_name, false);
			SLOG_DEBUG("remove " + db_name + " from the txn managers.");
		}
		if (apiUtil->remove_databaseinfo(db_name, msg) == false)
		{
			response->Error(StatusOperationFailed, msg);
			return;
		}
		SLOG_DEBUG("remove " + db_name + " from the already build database list success.");
		string db_path = _db_home + db_name + _db_suffix;
		if (is_backup == false)
		{
			Util::remove_path(db_path);
			SLOG_DEBUG("remove_path"+db_path);
		}
		else
		{
			std::string cmd = "mv " + db_path + " " + _db_home + db_name + ".bak";
			SLOG_DEBUG(cmd);
			system(cmd.c_str());
		}
		string success = "Database " + db_name + " dropped.";
		clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Drop));
		clusterManagerPtr->dropDb(db_name);
		response->Success(success);
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
		bool compress = jsonBoolParam(json_data, "backup_zip", false);
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if (apiUtil->check_db_built(db_name) == false)
		{
			msg = "the database [" + db_name + "] not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		// check backup path
		if (backup_path.empty())
		{
			backup_path = Util::backup_path;
			SLOG_DEBUG("backup_path is empty, set to default path: " + backup_path);
		}
		if (backup_path == "." || backup_path == "./" || Util::getExactPath(backup_path.c_str()) == Util::getExactPath(_db_home.c_str()))
		{
			msg = "Backup path can not be root or \"" + _db_home + "\" .";
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		grpc::GRPCServerTask* sub_task = task_of(response);
		string remote_ip = sub_task->peer_addr();
		bool async = jsonBoolParam(json_data, "async", false);
		std::string callback = jsonParam(json_data, "callback");
		std::string operation = "backup";
		std::string opt_id = apiUtil->generateUid();
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
		if (async)
		{
			sub_task->add_callback([&opt_id, db_name, &backup_path, compress, &callback](GRPCTask *task){
				std::string msg;
				bool backup_rt = apiUtil->backup_databaseinfo(db_name, compress, backup_path, msg);
				nlohmann::json j = {
					{"opt_id", opt_id}
				};
				if (backup_rt)
				{
					j["StatusCode"] = 0;
					j["StatusMsg"] = "Backup success";
					j["backupfilepath"] = backup_path;
					apiUtil->update_access_log(StatusOK, msg, opt_id, 0, 0, 0, backup_path);
				}
				else
				{
					j["StatusCode"] = StatusOperationFailed;
					j["StatusMsg"] = msg;
					apiUtil->update_access_log(StatusOperationFailed, msg, opt_id, -1, 0, 0);
				}
				if (!callback.empty())
				{
					HttpUtil::Post(callback, j.dump(), msg);
				}
			});
			msg = "Operation success";
			apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
			response->Json(resp_data);
		}
		else
		{
			bool backup_rt = apiUtil->backup_databaseinfo(db_name, compress, backup_path, msg);
			if (backup_rt) {
				msg = "Database backup successfully.";
				apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
				resp_data.AddMember("StatusCode", 0, allocator);
				resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
				resp_data.AddMember("backupfilepath", StringRef(backup_path.c_str()), allocator);
				response->Json(resp_data);
			} else {
				apiUtil->write_access_log(operation, remote_ip, StatusOperationFailed, msg, opt_id);
				response->Error(StatusOperationFailed, msg);
			}
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "Backup fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * query backup path
 * 
 * @param request 
 * @param response 
 * @param json_data 
 */
void backup_path_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		std::vector<std::string> file_list;
		string backup_path = Util::backup_path;
		Util::dir_files(backup_path, db_name, file_list);
		Document resp_data;
		Document pathsDoc;
		resp_data.SetObject();
		pathsDoc.SetArray();
		Document::AllocatorType &allocator = resp_data.GetAllocator();
		for (size_t i = 0; i < file_list.size(); i++)
		{
			pathsDoc.PushBack(Value().SetString((backup_path + file_list[i]).c_str(), allocator), allocator);
		}
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		resp_data.AddMember("paths", pathsDoc, allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		std::string error = "Query backup path fail: " + string(e.what());
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
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if (apiUtil->check_param_value("backup_path", backup_path, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if (Util::is_file(backup_path)) {
			if (GRPCUtil::fileSuffix(backup_path) != "zip") {
				response->Error(StatusParamIsIllegal, "Backup file is not zip file.");
				return;
			} else if (Util::file_exist(backup_path) == false) {
				response->Error(StatusParamIsIllegal, "Backup file not exist.");
				return;
			}
		} else if (Util::is_dir(backup_path)) {
			if (Util::dir_exist(backup_path) == false) {
				response->Error(StatusParamIsIllegal, "Backup path not exist.");
				return;
			}
		} else {
			response->Error(StatusParamIsIllegal, "Backup path not exist.");
			return;
		}
		
		std::string opt_id = apiUtil->generateUid();
		grpc::GRPCServerTask* sub_task = task_of(response);
		string remote_ip = sub_task->peer_addr();
		string operation = "restore";
		bool async = jsonBoolParam(json_data, "async", false);
		std::string callback = jsonParam(json_data, "callback");
		
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
		if (async)
		{
			sub_task->add_callback([&opt_id, db_name, &username, &backup_path, &callback](GRPCTask *task){
				std::string msg;
				bool restore_rt = apiUtil->restore_databaseinfo(username, db_name, backup_path, msg);
				nlohmann::json j = {
					{"opt_id", opt_id}
				};
				if (restore_rt)
				{
					msg = "Restore success";
					j["StatusCode"] = 0;
					j["StatusMsg"] = msg;
					j["backupfilepath"] = backup_path;
					apiUtil->update_access_log(StatusOK, msg, opt_id, 0, 0, 0, backup_path);
				}
				else
				{
					j["StatusCode"] = StatusOperationFailed;
					j["StatusMsg"] = msg;
					apiUtil->update_access_log(StatusOperationFailed, msg, opt_id, -1, 0, 0);
				}
				if (!callback.empty())
				{
					HttpUtil::Post(callback, j.dump(), msg);
				}
			});
			msg = "Operation success";
			apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
			response->Json(resp_data);
		}
		else
		{
			bool backup_rt = apiUtil->restore_databaseinfo(username, db_name, backup_path, msg);
			if (backup_rt) {
				msg = "Database restore successfully.";
				apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
				resp_data.AddMember("StatusCode", 0, allocator);
				resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
				response->Json(resp_data);
			} else {
				apiUtil->write_access_log(operation, remote_ip, StatusOperationFailed, msg, opt_id);
				response->Error(StatusOperationFailed, msg);
			}
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
void query_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, Json &json_data)
{
	try
	{
		std::string min_memory = Util::getConfigureValue("min_memory").c_str();
		int memoryLeft = Util::memoryLeft();
		if (memoryLeft < atoi(min_memory.c_str()))
		{
			std::string error = "memory not enough, available:" + std::to_string(memoryLeft) + "GB, need minimum:" + min_memory + "GB";
			response->Error(StatusOperationFailed, error);
			return;
		}
		std::string db_name = jsonParam(json_data, "db_name");
		std::string format = jsonParam(json_data, "format", "json");
		std::string username = jsonParam(json_data, "username");
		std::string sparql = jsonParam(json_data, "sparql");
		// check db_name paramter
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		// check sparql paramter
		if (apiUtil->check_param_value("sparql", sparql, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		// check database exist
		if (apiUtil->check_db_built(db_name) == false)
		{
			msg = "Database not build yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		// check database load status
		if (apiUtil->check_db_loaded(db_name) == false)
		{
			msg = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		// check database read lock
		if (apiUtil->rdlock_databaseinfo(db_info) == false)
		{
			msg = "get current database read lock fail.";
			response->Error(StatusLossOfLock, msg);
			return;
		}
		string thread_id = Util::getThreadID();
		bool is_update = false;
		QueryTree::UpdateType update_type;
		bool update_flag_bool = apiUtil->check_privilege(username, "update", db_name);
		// check update operation
		is_update = db_info->getDatabase()->isUpdate(sparql, update_type);
		if(clusterManagerPtr->isEnable() && clusterManagerPtr->isFollower() && is_update)
		{
			apiUtil->unlock_databaseinfo(db_info);
			redirect_handler(request, response, series);
			return;
		}
		FILE *output = NULL;
		ResultSet rs;
		int ret_val;
		int query_time = Util::get_cur_time();
		shared_ptr<ofstream> clusterlog = nullptr;
		std::string cluster_db_path;
		std::string logpath;
		uint64 log_index;
		ClusterUpdateType cluster_update_type = ClusterUpdateType::ClusterUpdateType_None;
		// update waiting follower reply
		if (clusterManagerPtr->isEnable() && is_update) 
		{
			// send [prepare] heartbeat and wait response
			if (update_type == QueryTree::UpdateType::Insert_Data || update_type  == QueryTree::UpdateType::Insert_Clause) 
				cluster_update_type = ClusterUpdateType::ClusterUpdateType_Insert;
			else
				cluster_update_type = ClusterUpdateType::ClusterUpdateType_Delete;
			log_index = apiUtil->generateUID();
			clusterManagerPtr->addLog(db_name, log_index, ClusterOperation_Prepare, cluster_update_type);
			bool prepare_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Prepare), true);
			if (!prepare_result)
			{
				apiUtil->unlock_databaseinfo(db_info);
				msg = "Less than half of the cluster nodes are confirmed.";
				clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
				SLOG_ERROR(msg);
				response->Error(StatusOperationFailed, msg);
				return;
			}
			cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
			logpath = cluster_db_path + to_string(log_index) + ".log";
			clusterlog = make_shared<ofstream>();
			clusterlog->open(logpath.c_str());
		}

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
			ret_val = db_info->getDatabase()->query(sparql, rs, output, update_flag_bool, false, nullptr, clusterlog);
			query_time = Util::get_cur_time() - query_time;
			if (clusterlog) 
			{
				clusterlog->close();
				clusterlog.reset();
			}
			// unlock rdlock
			apiUtil->unlock_databaseinfo(db_info);
		} catch (const std::exception &e) {
			apiUtil->unlock_databaseinfo(db_info);
			msg = "Query fail: " + string(e.what());
			response->Error(StatusOperationFailed, msg);
			if (clusterlog)
				clusterlog->close();
			return;
		}
		string filename = thread_id + "_" + Util::getTimeString2() + "_" + Util::int2string(Util::getRandNum()) + ".txt";
		string localname = apiUtil->get_query_result_path() + filename;
		string query_time_s = Util::int2string(query_time);
		if (!is_update && (ret_val == -100))
		{
			// SLOG_DEBUG(thread_id + ":search query returned successfully.");

			// record each query operation, including the sparql and the answer number
			// accurate down to microseconds
			// filter the IP from the test server
			std::string remote_ip = jsonParam(json_data, "remote_ip");
			long rs_ansNum = max((long)rs.ansNum - rs.output_offset, 0L);
			long rs_outputlimit = (long)rs.output_limit;
			if (rs_outputlimit != -1)
			{
				rs_ansNum = min(rs_ansNum, rs_outputlimit);
			}	

			int status_code = 0;
			string file_name = "";
			if (format.find("file") != string::npos)
			{
				file_name = string(filename.c_str());
			}
			// add callback task for query log start
			struct DBQueryLogInfo* query_log_ptr = new DBQueryLogInfo(query_start_time, remote_ip, sparql, 
				rs_ansNum, format, file_name, status_code, query_time, db_name);
			task_of(response)->add_callback([query_log_ptr](GRPCTask *) {
				apiUtil->write_query_log(query_log_ptr);
				delete query_log_ptr;
			});

			// to void someone downloading all the data file by sparql query on purpose and to protect the data
			// if the ansNum too large, for example, larger than 100000, we limit the return ans.
			if (rs_ansNum > apiUtil->get_max_output_size())
			{
				if (rs_outputlimit == -1 || rs_outputlimit > apiUtil->get_max_output_size())
				{
					rs_outputlimit = apiUtil->get_max_output_size();
				}
			}

			ofstream outfile;
			string ans = "";
			Json resp_data;
			resp_data.SetObject();
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			if (format == "json")
			{
				string success = rs.to_JSON();
				rs.release();
				resp_data.Parse(success.c_str());
				if (!resp_data.HasParseError())
				{
					resp_data.AddMember("StatusCode", 0, allocator);
					resp_data.AddMember("StatusMsg", "success", allocator);
					resp_data.AddMember("AnsNum", rs_ansNum, allocator);
					resp_data.AddMember("OutputLimit", rs_outputlimit, allocator);
					resp_data.AddMember("QueryTime", StringRef(query_time_s.c_str()), allocator);
				} 
				else
				{
					string filename2 = "error_" + filename;
					string localname2 = apiUtil->get_query_result_path() + filename2;
					outfile.open(localname2);
					outfile << success;
					outfile.close();
					SLOG_ERROR("result parse error: ErrorCode=" + to_string(resp_data.GetParseError()) 
							+ ", ErrorPosition=" + to_string(resp_data.GetErrorOffset()) + ", ResultFile=" + localname2);
					msg = "Query fail: the result parse error.";
					resp_data.AddMember("StatusCode", StatusOperationFailed, allocator);
					resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
				}
			}
			else if (format == "file")
			{
				outfile.open(localname);
				outfile << rs.to_JSON();
				outfile.close();
				rs.release();
				resp_data.AddMember("StatusCode", 0, allocator);
				resp_data.AddMember("StatusMsg", "success", allocator);
				resp_data.AddMember("AnsNum", rs_ansNum, allocator);
				resp_data.AddMember("OutputLimit", rs_outputlimit, allocator);
				resp_data.AddMember("QueryTime", StringRef(query_time_s.c_str()), allocator);
				resp_data.AddMember("FileName", StringRef(filename.c_str()), allocator);
			}
			else if (format == "n-triple")
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
				rs.release();
				string json_data_str = json_data.dump();
				resp_data.Parse(json_data_str.c_str());
				if (!resp_data.HasParseError())
				{
					resp_data.AddMember("StatusCode", 0, allocator);
					resp_data.AddMember("StatusMsg", "success", allocator);
					resp_data.AddMember("AnsNum", rs_ansNum, allocator);
					resp_data.AddMember("OutputLimit", rs_outputlimit, allocator);
					resp_data.AddMember("ThreadId", StringRef(thread_id.c_str()), allocator);
					resp_data.AddMember("QueryTime", StringRef(query_time_s.c_str()), allocator);
				} 
				else
				{
					
					string filename2 = "error_" + filename;
					string localname2 = apiUtil->get_query_result_path() + filename2;
					outfile.open(localname2);
					outfile << json_data_str;
					outfile.close();
					SLOG_ERROR("result parse error: ErrorCode=" + to_string(resp_data.GetParseError()) + ", ErrorPosition=" + to_string(resp_data.GetErrorOffset()) + ", ResultFile=" + localname2);
					msg = "Query fail: the result parse error.";
					resp_data.AddMember("StatusCode", StatusOperationFailed, allocator);
					resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
				}
			}
			else
			{
				msg = "Unkown result format.";
				resp_data.AddMember("StatusCode", StatusOperationFailed, allocator);
				resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
			}
			// common data 
			resp_data.AddMember("ThreadId", StringRef(thread_id.c_str()), allocator);
			response->set_header_pair("Cache-Control", "no-cache");
			response->set_header_pair("Pragma", "no-cache");
			response->set_header_pair("Expires", "0");
			if (request->hasHeader("Accept-Encoding")) {
				std::string accept_encoding = request->header("Accept-Encoding");
				if (accept_encoding.find("gzip") != std::string::npos)
				{
					response->headers["Content-Encoding"] = "gzip";
				}
			}
			response->Json(resp_data);
		}
		else if (is_update)
		{
			SLOG_DEBUG("update query returns true. update num " + to_string(ret_val));
			Json resp_data;
			resp_data.SetObject();
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", "update query returns true.", allocator);
			resp_data.AddMember("AnsNum", ret_val, allocator);
			resp_data.AddMember("QueryTime", StringRef(query_time_s.c_str()), allocator);
			resp_data.AddMember("ThreadId", StringRef(thread_id.c_str()), allocator);
			if (clusterManagerPtr->isEnable())
			{
				// add log appendEntities task
				string log_file_name = to_string(log_index) + ".log";
				if (ret_val > 0)
				{
					SLOG_DEBUG("add log appendEntities task, copy num " + to_string(ret_val));
					bool append_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Append, cluster_update_type, log_file_name), true);
					if (append_result)
					{
						SLOG_DEBUG("response result:\n" << to_json_string(resp_data));
						clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Commit));
						response->Json(resp_data);
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
							Util::remove_path(nt_file_path);
						}
						else
						{
							SLOG_ERROR("restore " + db_name + " data failed: unable get wrlock, log[" + log_file_name + "], operation["+to_string(cluster_update_type)+"]");
						}
						msg = "Less than half of the cluster nodes reply.";
						SLOG_ERROR(msg);
						clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Cancel));
						response->Error(StatusOperationFailed, msg);
					}
				}
				else
				{
					SLOG_DEBUG("No data needs to be synchronized, update log stauts to failed");
					clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
					Util::remove_path(clusterManagerPtr->getDbDirPath(db_name)+log_file_name);
					response->Json(resp_data);
				}
			}
			else
			{
				response->Json(resp_data);
			}
		}
		else
		{
			msg = "search query returns false.";
			SLOG_DEBUG(msg);
			response->Error(StatusOperationFailed, msg);
		}
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
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		std::string db_path = jsonParam(json_data, "db_path");
		if (apiUtil->check_param_value("db_path", db_path, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if (apiUtil->check_db_built(db_name) == false)
		{
			msg = "Database not build yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		// check if database named [db_name] is already load
		if (apiUtil->check_db_loaded(db_name) == false)
		{
			msg = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		if (apiUtil->rdlock_databaseinfo(db_info) == false)
		{
			msg = "get current database read lock fail.";
			response->Error(StatusLossOfLock, msg);
			return;
		}
		Util::string_suffix(db_path, '/');
		if (Util::dir_exist(db_path) == false)
		{
			Util::create_dirs(db_path);
		}
		std::string export_path = db_path + db_name + "_" + Util::get_timestamp() + ".nt";
		bool compress = jsonBoolParam(json_data, "compress", false);
		SLOG_DEBUG("export_path: " + export_path);
		FILE *ofp = fopen(export_path.c_str(), "w");
		db_info->getDatabase()->export_db(ofp);
		fflush(ofp);
		fclose(ofp);
		ofp = NULL;
		// unlock
		apiUtil->unlock_databaseinfo(db_info);
		if (compress)
		{
			std::string zip_path = db_path + db_name + "_" + Util::get_timestamp() + ".zip";
			if (!CompressUtil::FileHelper::compressExportZip(export_path, zip_path))
			{
				Util::remove_path(export_path);
				Util::remove_path(zip_path);
				msg = "export compress fail.";
				response->Error(StatusCompressError, msg);
				return;
			}
			Util::remove_path(export_path);
			export_path = zip_path;
		}
		msg = "Export the database successfully.";
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
		resp_data.AddMember("filepath", StringRef(export_path.c_str()), allocator);
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
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		std::string isolevel = jsonParam(json_data, "isolevel");
		if (apiUtil->check_param_value("isolevel", isolevel, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		int level = stoi(isolevel);
		if (level <= 0 || level > 3)
		{
			msg = "The isolation level's value only can been 1/2/3";
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if (apiUtil->check_db_built(db_name) == false)
		{
			msg = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		if (apiUtil->check_db_loaded(db_name) == false)
		{
			msg = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		std::string username = jsonParam(json_data, "username");
		txn_id_t tid;
		if (apiUtil->begin_process(db_name, level, username, tid))
		{
			msg = "Transaction begin failed.";
			response->Error(StatusTranscationManageFailed, msg);
			return;
		}
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Transaction begin success", allocator);
		resp_data.AddMember("TID", StringRef(to_string(tid).c_str()), allocator);
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
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		std::string tid_s = jsonParam(json_data, "tid");
		if (apiUtil->check_param_value("tid", tid_s, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		txn_id_t tid;
		if (apiUtil->check_txn_id(tid_s, tid))
		{
			msg = "TID " + tid_s + " is not a pure number.";
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		std::string sparql = jsonParam(json_data, "sparql");
		if (apiUtil->check_param_value("sparql", sparql, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if (apiUtil->check_db_built(db_name) == false)
		{
			msg = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		if (apiUtil->check_db_loaded(db_name) == false)
		{
			msg = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		shared_ptr<Txn_manager> txn_m;
		if (apiUtil->get_txn_manager(db_name, txn_m))
		{
			msg = "Get database transaction manager error.";
			response->Error(StatusTranscationManageFailed, msg);
			return;
		}
		SLOG_DEBUG("tquery sparql: " + sparql);
		std::string res;
		int ret = txn_m->Query(tid, sparql, res);
		if (ret == -1)
		{
			msg = "Transaction query failed due to wrong TID";
			response->Error(StatusOperationFailed, msg);
		}
		else if (ret == -10)
		{
			msg = "Database has been flushed or removed";
			response->Error(StatusOperationFailed, msg);
		}
		else if (ret == -99)
		{
			msg = "Transaction is not in running status!";
			response->Error(StatusOperationFailed, msg);
		}
		else if (ret == -20)
		{
			apiUtil->aborted_process(txn_m, tid, msg);
			msg = "Transaction Abort due to Query failed!";
			response->Error(StatusOperationFailed, msg);
		}
		else if (ret == -101)
		{
			msg = "Transaction query failed. Unknown query error";
			response->Error(StatusOperationFailed, msg);
		}
		Json resp_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		if (ret == -100)
		{
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
		else
		{
			resp_data.AddMember("AnsNum", ret, allocator);
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", "Transaction query success", allocator);
			response->Json(resp_data);
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
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		std::string tid_s = jsonParam(json_data, "tid");
		if (apiUtil->check_param_value("TID", tid_s, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		txn_id_t tid;
		if (apiUtil->check_txn_id(tid_s, tid))
		{
			msg = "TID " + tid_s + " is not a pure number.";
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if (apiUtil->check_db_built(db_name) == false)
		{
			msg = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		if (apiUtil->check_db_loaded(db_name) == false)
		{
			msg = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			msg = "Unable to commit due to loss of lock.";
			response->Error(StatusLossOfLock, msg);
			return;
		}
		shared_ptr<Txn_manager> txn_m;
		if (apiUtil->get_txn_manager(db_name, txn_m))
		{
			apiUtil->unlock_databaseinfo(db_info);
			msg = "Get database transaction manager error.";
			response->Error(StatusTranscationManageFailed, msg);
			return;
		}
		if (apiUtil->commit_process(txn_m, tid, msg) ==  false)
		{
			response->Error(StatusOperationFailed, msg);
		}
		else
		{
			response->Success("Transaction commit success.");
		}
		apiUtil->unlock_databaseinfo(db_info);
	}
	catch (const std::exception &e)
	{
		string msg = "Transaction commit fail: " + string(e.what());
		response->Error(StatusOperationFailed, msg);
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
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		std::string tid_s = jsonParam(json_data, "tid");
		if (apiUtil->check_param_value("TID", tid_s, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		txn_id_t tid;
		if (apiUtil->check_txn_id(tid_s, tid))
		{
			msg = "TID " + tid_s + " is not a pure number.";
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if (apiUtil->check_db_built(db_name) == false)
		{
			msg = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		if (apiUtil->check_db_loaded(db_name) == false)
		{
			msg = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			msg = "Unable to rollback due to loss of lock.";
			response->Error(StatusLossOfLock, msg);
			return;
		}
		shared_ptr<Txn_manager> txn_m;
		if (apiUtil->get_txn_manager(db_name, txn_m))
		{
			apiUtil->unlock_databaseinfo(db_info);
			msg = "Get database transaction manager error.";
			response->Error(StatusTranscationManageFailed, msg);
			return;
		}
		if (apiUtil->rollback_process(txn_m, tid, msg) ==  false)
		{
			response->Error(StatusOperationFailed, msg);
		}
		else
		{
			response->Success("Transaction rollback success.");
		}
		apiUtil->unlock_databaseinfo(db_info);
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
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if (apiUtil->check_db_built(db_name) == false)
		{
			msg = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		if (apiUtil->check_db_loaded(db_name) == false)
		{
			msg = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			msg = "Unable to checkpoint due to loss of lock.";
			response->Error(StatusLossOfLock, msg);
			return;
		}
		shared_ptr<Txn_manager> txn_m;
		if(apiUtil->get_txn_manager(db_name, txn_m) == false)
		{
			msg = "Get database transaction manager error.";
			apiUtil->unlock_databaseinfo(db_info);
			response->Error(StatusTranscationManageFailed, msg);
			return;
		}
		txn_m->Checkpoint();
		db_info->getDatabase()->save();
		apiUtil->unlock_databaseinfo(db_info);
		string success = "Database saved successfully.";
		response->Success(success);
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
void batch_insert_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, Json &json_data)
{
	if(clusterManagerPtr->isEnable() && clusterManagerPtr->isFollower())
	{
		redirect_handler(request, response, series);
		return;
	}

	server::MessageBatchInsertRequest request_data(json_data);
	server::MessageBatchInsertResponse response_data; 
	string remote_ip = task_of(response)->peer_addr();
	server::ApiHandler::batch_insert(apiUtil, clusterManagerPtr, request_data, response_data, remote_ip, _db_home, _db_suffix);
	if (response_data.status_code != server::StatusOK)
	{
		response->Error(response_data.status_code, response_data.status_msg);
	}
	else
	{
		std::string json_str;
		response_data.toJsonString(json_str);
		response->nlohmannJson(json_str);
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
void batch_remove_task(const GRPCReq *request, GRPCResp *response, SeriesWork *series, Json &json_data)
{
	try
	{
		if(clusterManagerPtr->isEnable() && clusterManagerPtr->isFollower())
		{
			// TODO upload file to leader node
			redirect_handler(request, response, series);
			return;
		}
		std::string db_name = jsonParam(json_data, "db_name");
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		std::string file = jsonParam(json_data, "file");
		if (apiUtil->check_param_value("file", file, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if (Util::file_exist(file) == false)
		{
			msg = "The data file is not exist";
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if (apiUtil->check_db_built(db_name) == false)
		{
			msg = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		if (apiUtil->check_db_loaded(db_name) == false)
		{
			msg = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		shared_ptr<ofstream> clusterlog = nullptr;
		std::string cluster_db_path;
		std::string logpath;
		uint64 log_index;
		if (clusterManagerPtr->isEnable()) 
		{
			// send [prepare] heartbeat and wait response
			ClusterUpdateType cluster_update_type = ClusterUpdateType::ClusterUpdateType_Delete;
			log_index = apiUtil->generateUID();
			clusterManagerPtr->addLog(db_name, log_index, ClusterOperation_Prepare, cluster_update_type);
			bool prepare_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Prepare), true);
			if (!prepare_result)
			{
				msg = "Less than half of the cluster nodes are confirmed.";
				SLOG_ERROR(msg);
				clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
				response->Error(StatusOperationFailed, msg);
				return;
			}
			cluster_db_path = clusterManagerPtr->getDbDirPath(db_name);
			logpath = cluster_db_path + to_string(log_index) + ".log";
			clusterlog = make_shared<ofstream>();
			clusterlog->open(logpath.c_str());
		}
		std::vector<std::string> nt_files;
		std::string unz_dir_path;
		std::string file_suffix = GRPCUtil::fileSuffix(file);
		bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
		if (is_zip)
		{
			auto code = CompressUtil::FileHelper::foreachZip(file,[](std::string filename)->bool
				{
					if( apiUtil->check_upload_allow_extensions(GRPCUtil::fileSuffix(filename)) == false )
						return false;
					return true;
				});
			if( code != CompressUtil::UnZipOK )
			{
				msg = "uncompress is failed error.";
				response->Error(code, msg);
				if (clusterlog)
					clusterlog->close();
				return;
			}
			std::string file_name = GRPCUtil::fileName(file);
			size_t pos = file_name.size() - file_suffix.size() - 1;
			unz_dir_path = apiUtil->get_upload_path() + file_name.substr(0, pos) + "_" + Util::getTimeString2();
			Util::create_dirs(unz_dir_path);
			CompressUtil::UnCompressZip upfile(file, unz_dir_path);
			code = upfile.unCompress();
			if (code != CompressUtil::UnZipOK)
			{
				Util::remove_path(unz_dir_path);
				msg = "uncompress is failed error.";
				response->Error(code, msg);
				if (clusterlog)
					clusterlog->close();
				return;
			}
			upfile.getFileList(nt_files, "");
		}
		else
		{
			nt_files.push_back(file);
		}
		std::string opt_id = apiUtil->generateUid();
		bool async = jsonBoolParam(json_data, "async", false);
		std::string callback = jsonParam(json_data, "callback");
		auto remove_helper = [db_name, &nt_files, &unz_dir_path, opt_id, async, callback, &log_index, &clusterlog](GRPCResp *response){
			shared_ptr<DatabaseInfo> db_info;
			apiUtil->get_databaseinfo(db_name, db_info);
			// access log
			string msg = "Operation Success.";
			string operation = "batchRemove";
			string remote_ip = task_of(response)->peer_addr();
			if (!apiUtil->trywrlock_databaseinfo(db_info, 300))
			{
				msg = "Unable to batch remove due to loss of lock.";
				apiUtil->write_access_log(operation, remote_ip, StatusLossOfLock, msg, opt_id);
				response->Error(StatusLossOfLock, msg);
				return;
			}
			apiUtil->write_access_log(operation, remote_ip, StatusOK, msg, opt_id);
			unsigned success_num = 0;
			unsigned total_num = 0;
			size_t parse_error_num = 0;
			string error_log = _db_home +  "/" + db_name + _db_suffix + "/parse_error.log";
			total_num = Util::count_lines(error_log);
			for (std::string rdf_file : nt_files)
			{
				SLOG_DEBUG("begin remove data from " + rdf_file);
				success_num += db_info->getDatabase()->batch_remove(rdf_file, false, nullptr, clusterlog);
			}
			// exclude Info line
			parse_error_num = Util::count_lines(error_log) - total_num - nt_files.size();
			// save data and unlock
			db_info->getDatabase()->save();
			apiUtil->unlock_databaseinfo(db_info);
			// close cluster log
			if (clusterlog) 
			{
				clusterlog->close();
				clusterlog.reset();
			}
			// update access log
			msg = "Batch remove data successfully.";
			apiUtil->update_access_log(0, msg, opt_id, 1, success_num, parse_error_num);

			// respnse data
			Json resp_data;
			resp_data.SetObject();
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
			resp_data.AddMember("success_num", success_num, allocator);
			resp_data.AddMember("failed_num", parse_error_num, allocator);
			resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
			
			if (clusterManagerPtr->isEnable()) 
			{
				// cluster sync task begin
				string log_file_name = to_string(log_index) + ".log";
				if (success_num > 0)
				{
					SLOG_DEBUG("add log appendEntities task, copy num " + to_string(success_num));
					string tmp_dir_path = unz_dir_path;
					bool append_result = clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Append, ClusterUpdateType_Delete, log_file_name), true);
					if (append_result)
					{
						SLOG_DEBUG("response result:\n" << to_json_string(resp_data));
						clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Commit));
						if (response)
						{
							response->Json(resp_data);
						}
						if (!callback.empty())
						{
							string res;
							rapidjson::StringBuffer resBuffer;
							rapidjson::Writer<rapidjson::StringBuffer> resWriter(resBuffer);
							resp_data.Accept(resWriter);
							HttpUtil::Post(callback, resBuffer.GetString(), res);
						}
						if (!tmp_dir_path.empty())
						{
							Util::remove_path(tmp_dir_path);
						}
					}
					else
					{
						// restore data
						// try get wrlock timeout 600 senconds
						if (apiUtil->trywrlock_databaseinfo(db_info, 600))
						{
							uint64_t num = 0;
							for (std::string rdf_file : nt_files)
							{
								num += db_info->getDatabase()->batch_insert(rdf_file);
							}
							SLOG_INFO("restore " + db_name + " data: batch_insert num " << num);
							apiUtil->unlock_databaseinfo(db_info);
						}
						else
						{
							SLOG_ERROR("restore " + db_name + " data failed: unable get wrlock, log[" + log_file_name + "], operation[2]");
						}
						if (!tmp_dir_path.empty())
						{
							Util::remove_path(tmp_dir_path);
						}
						msg = "Less than half of the cluster nodes reply.";
						SLOG_ERROR(msg);
						clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Cancel));
						if (response)
						{
							response->Error(StatusOperationFailed, msg);
						}
						if (!callback.empty())
						{
							Json resp_error;
							resp_error.SetObject();
							Json::AllocatorType &allocator_err = resp_error.GetAllocator();
							resp_error.AddMember("StatusCode", 1005, allocator_err);
							resp_error.AddMember("StatusMsg", StringRef(msg.c_str()), allocator_err);
							string res;
							rapidjson::StringBuffer resBuffer;
							rapidjson::Writer<rapidjson::StringBuffer> resWriter(resBuffer);
							resp_error.Accept(resWriter);
							HttpUtil::Post(callback, resBuffer.GetString(), res);
						}
					}
				}
				else
				{
					SLOG_DEBUG("No data needs to be synchronized, update log stauts to failed");
					clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Fail));
					Util::remove_path(clusterManagerPtr->getDbDirPath(db_name)+log_file_name);
					// remove unzip files
					if (!unz_dir_path.empty())
					{
						Util::remove_path(unz_dir_path);
					}
					if (response != nullptr)
					{
						response->Json(resp_data);
					}
					if (!callback.empty())
					{
						string res;
						rapidjson::StringBuffer resBuffer;
						rapidjson::Writer<rapidjson::StringBuffer> resWriter(resBuffer);
						resp_data.Accept(resWriter);
						HttpUtil::Post(callback, resBuffer.GetString(), res);
						// TODO retry?
					}
				}
				// cluster sync task end
			}
			else
			{
				response->Json(resp_data);
			}
		};
		if (async)
		{
			Json resp_data;
			resp_data.SetObject();
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", "Operation Success.", allocator);
			resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
			response->Json(resp_data);
			thread t(remove_helper, nullptr);
			t.detach();
		}
		else
		{
			remove_helper(response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Batch remove fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

void rename_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		std::string new_name = jsonParam(json_data, "new_name");
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
		std::string msg;
		if (apiUtil->check_param_value("op_username", op_username, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		std::string op_password = jsonParam(json_data, "op_password");
		std::string type = jsonParam(json_data, "type");
		if (type != "2")
		{
			if (apiUtil->check_param_value("op_password", op_password, msg) == false)
			{
				response->Error(StatusParamIsIllegal, msg);
				return;
			}
		}
		
		if (type == "1") // add user
		{
			// check user number
			if (apiUtil->check_user_count() == false)
			{
				msg = "The total number of users more than max_user_num.";
				response->Error(StatusOperationConditionsAreNotSatisfied, msg);
				return;
			}
			if (apiUtil->user_add(op_username, op_password))
			{
				response->Success("Add user done.");
			}
			else
			{
				msg = "Username already existed, add user failed.";
				response->Error(StatusOperationFailed, msg);
			}
		}
		else if (type == "2") // delete user
		{
			if (op_username == apiUtil->get_root_username())
			{
				msg = "You cannot delete root, delete user failed.";
				response->Error(StatusOperationFailed, msg);
			}
			else if (apiUtil->user_delete(op_username))
			{
				response->Success("Delete user done.");
			}
			else
			{
				msg = "Username not exist, delete user failed.";
				response->Error(StatusOperationFailed, msg);
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
				msg = "Username not exist, change password failed.";
				response->Error(StatusOperationFailed, msg);
			}
		}
		else
		{
			msg = "The operation is not support.";
			response->Error(StatusParamIsIllegal, msg);
		}
	}
	catch (const std::exception &e)
	{
		string error = "User manage fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

void reason_manage_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string type= jsonParam(json_data, "type");
		std::string operation="AddReason";
		std::string msg;
		if (apiUtil->check_param_value("db_name", db_name, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if(apiUtil->check_param_value("type", type, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if (type == "1") // add Reason Rule
		{
			operation="AddReasonRule";
			// check user number
			if(json_data.HasMember("ruleinfo")==false)
			{
				msg = "the data has not the rule information";
			 	response->Error(StatusParamIsIllegal, msg);
				return;
			}
			Value reasonInfo=json_data["ruleinfo"].GetObject();
			Document::AllocatorType &allocator = json_data.GetAllocator();
			std::string createtime = Util::get_date_time();
			reasonInfo.AddMember("status","新建",allocator);
			reasonInfo.AddMember("createtime",StringRef(createtime.c_str()),allocator);
			ReasonOperationResult resultInfo= ReasonHelper::saveReasonRuleInfo(reasonInfo,db_name,_db_home,_db_suffix);
			if(resultInfo.issuccess==1)
			{
				response->Success("Add Reason Rule done."+resultInfo.error_message);
			}
			else
			{
				msg="Add Reason Rule Fail. " + resultInfo.error_message;
				response->Error(StatusOperationFailed, msg);
			}
		}
		else if (type == "2") // listReason
		{
			operation = "listReasonRules";
            string db_path=_db_home+db_name+_db_suffix;
			vector<string> liststr = ReasonHelper::getReasonRuleList(db_path);
			stringstream str_stream;
			str_stream << "[";
            for(int i=0;i<liststr.size();i++)
			{
				if (i > 0) 
					str_stream << ",";
				str_stream << liststr[i];
			}
			str_stream << "]";
			string arrayStr = str_stream.str();
			Document list;
			list.SetArray();
			list.Parse(arrayStr.c_str());
			if (list.HasParseError()) 
			{

				msg = "pasrse rulefiles error:" + arrayStr;
				response->Error(StatusOperationFailed, msg);
			} 
			else 
			{
				Json doc;
				Document::AllocatorType &allocator = doc.GetAllocator();
				unsigned num = list.Size();
				doc.SetObject();
				doc.AddMember("StatusCode", 0, allocator);
				doc.AddMember("StatusMsg", "ok", allocator);
				doc.AddMember("list", list.Move(), allocator);
				doc.AddMember("num",  num, allocator);
           		response->Json(doc);
			}
		}
		else if (type == "3") // Compile Reason Rule
		{
			operation = "compileReasonRule";
			string rulename=json_data["rulename"].GetString();
            ReasonSparql resultInfo=ReasonHelper::compileReasonRule(rulename,db_name,_db_home,_db_suffix);
			Document doc;
			doc.SetObject();
			Document::AllocatorType &allocator = doc.GetAllocator();
			if(resultInfo.issuccess==0)
			{
				response->Error(StatusOperationFailed,resultInfo.error_message);
			}
			else
			{
				doc.AddMember("StatusCode",0,allocator);
				doc.AddMember("insert_sparql",StringRef(resultInfo.insert_sparql.c_str()),allocator);
				doc.AddMember("delete_sparql",StringRef(resultInfo.delete_sparql.c_str()),allocator);
				doc.AddMember("check_sparql",StringRef(resultInfo.check_sparql.c_str()),allocator);
				response->Json(doc);
			}
		}
		else if(type=="4") //execute Reason
		{
			operation = "executeReasonRule";
			string rulename=json_data["rulename"].GetString();
			string username=json_data["username"].GetString();
			ReasonSparql resultInfo= ReasonHelper::executeReasonRule(rulename,db_name,_db_home,_db_suffix);
			if(resultInfo.issuccess==0)
			{
				response->Error(StatusOperationFailed,resultInfo.error_message);
				return;
			}
	        if (apiUtil->check_db_built(db_name) == false)
			{
				msg = "the database is not exist!";
				response->Error(StatusOperationFailed, msg);
			    return;
			}
			if (apiUtil->check_db_loaded(db_name) == false)
			{
				msg = "Database not load yet!";
				response->Error(StatusOperationFailed, msg);
			    return;
			}
            shared_ptr<DatabaseInfo> db_info;
			apiUtil->get_databaseinfo(db_name, db_info);
			if (apiUtil->trywrlock_databaseinfo(db_info) == false)
			{
				msg = "get current database write lock fail.";
				response->Error(StatusOperationFailed, msg);
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
				response->Error(StatusOperationFailed,content);
				return;
			}
			catch (const std::runtime_error &e2)
			{
				string content = e2.what();
				apiUtil->unlock_databaseinfo(db_info);
				response->Error(StatusOperationFailed,content);
				return;
			}
			catch (...)
			{
				string content = "unknow error";
				apiUtil->unlock_databaseinfo(db_info);
				response->Error(StatusOperationFailed,content);
				return;
			}
			
			Document doc;
			doc.SetObject();
			Document::AllocatorType &allocator = doc.GetAllocator();
			doc.AddMember("insert_sparql",StringRef(resultInfo.insert_sparql.c_str()),allocator);
			doc.AddMember("AnsNum", ret_val, allocator);
			doc.AddMember("StatusCode", 0, allocator);
			doc.AddMember("StatusMsg", "ok", allocator);
            ReasonHelper::updateReasonRuleStatus(rulename, db_name, "已执行",_db_home,_db_suffix);
		    response->Json(doc);
		}
		else if(type=="5")
		{
			operation = "disableReasonRule";
			string rulename=json_data["rulename"].GetString();
			ReasonSparql resultInfo= ReasonHelper::disableReasonRule(rulename,db_name,_db_home,_db_suffix);
			if(resultInfo.issuccess==0)
			{
				response->Error(StatusOperationFailed,resultInfo.error_message);
			    return;
			}
			string username = json_data["username"].GetString();
			if(apiUtil->check_db_built(db_name)==false)
			{
				msg = "the database is not exist!";
				response->Error(StatusOperationFailed, msg);
			    return;
			}
			// check database load status
			if (apiUtil->check_db_loaded(db_name) == false)
			{
				msg = "Database not load yet!";
				response->Error(StatusOperationFailed, msg);
			    return;
			}
            shared_ptr<DatabaseInfo> db_info;
			apiUtil->get_databaseinfo(db_name, db_info);
			bool update_flag_bool=true;
			if (apiUtil->trywrlock_databaseinfo(db_info) ==  false)
			{
				msg = "get current database write lock fail.";
				response->Error(StatusOperationFailed, msg);
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
				response->Error(StatusOperationFailed,content);
				return;
			}
			catch (const std::runtime_error &e2)
			{
				string content = e2.what();
				apiUtil->unlock_databaseinfo(db_info);
				response->Error(StatusOperationFailed,content);
				return;
			}
			catch (...)
			{
				string content = "unknow error";
				apiUtil->unlock_databaseinfo(db_info);
				response->Error(StatusOperationFailed,content);
				return;
			}
			Document doc;
			doc.SetObject();
			Document::AllocatorType &allocator = doc.GetAllocator();
			doc.AddMember("delete_sparql",StringRef(resultInfo.delete_sparql.c_str()),allocator);
			doc.AddMember("AnsNum",ret_val,allocator);
			doc.AddMember("StatusCode", 0, allocator);
			doc.AddMember("StatusMsg", "ok", allocator);
            ReasonHelper::updateReasonRuleStatus(rulename, db_name, "已失效",_db_home,_db_suffix);
		    response->Json(doc);
		}
		else if(type=="6")
		{
		     operation = "showReasonRule";
			string rulename=json_data["rulename"].GetString();
			ReasonOperationResult resultInfo= ReasonHelper::getReasonInfo(rulename,db_name,_db_home,_db_suffix);
			if(resultInfo.issuccess==0)
			{
				response->Error(StatusOperationFailed,resultInfo.error_message);
			}			
			else
			{
				// 输出格式化的JSON
				std::string ruleinfo = "{\"ruleinfo\":" + resultInfo.error_message + "}";
				Document doc;
				doc.SetObject();
				doc.Parse(ruleinfo.c_str());
				Document::AllocatorType &allocator = doc.GetAllocator();
				doc.AddMember("StatusCode",0,allocator);
				doc.AddMember("StatusMsg","ok",allocator);
				response->Json(doc);
			}
		}
		else if (type == "7")
		{
			operation = "deleteReasonRule";
			string rulename = json_data["rulename"].GetString();
			ReasonOperationResult resultInfo = ReasonHelper::getReasonInfo(rulename, db_name, _db_home, _db_suffix);
			if (resultInfo.issuccess == 0)
			{
				response->Error(StatusOperationFailed,resultInfo.error_message);
				return;
			}
			else
			{
				// 输出格式化的JSON
				Document doc;
				doc.SetObject();
				doc.Parse(resultInfo.error_message.c_str());
				Document::AllocatorType &allocator = doc.GetAllocator();
				if (doc.HasMember("status")&&doc["status"].GetString() == "已执行")
				{
					string error_msg = "该规则已经执行，请先让该规则失效后再执行";
					response->Error(StatusOperationFailed,error_msg);
					return;
				}
				ReasonOperationResult resultInfo2 = ReasonHelper::removeReasonRule(rulename, db_name, _db_home, _db_suffix);
				if (resultInfo2.issuccess == 1)
				{
					Document doc2;
				    doc2.SetObject();
					doc2.AddMember("StatusCode", 0, allocator);
					doc2.AddMember("StatusMsg", StringRef(resultInfo2.error_message.c_str()), allocator);
					response->Json(doc2);
				}
				else
				{
					response->Error(StatusOperationFailed,resultInfo2.error_message);
				}
			}
		}
		else if (type == "8")
		{
			operation = "checkReasonRule";
			string rulename=json_data["rulename"].GetString();
			ReasonSparql resultInfo= ReasonHelper::getCheckSparql(rulename,db_name,_db_home,_db_suffix);
			if(resultInfo.issuccess==0)
			{
				response->Error(StatusOperationFailed,resultInfo.error_message);
				return;
			}
			if(apiUtil->check_db_built(db_name)==false)
			{
				msg = "the database is not exist!";
				response->Error(StatusOperationFailed, msg);
			    return;
			}
			if(apiUtil->check_db_loaded(db_name)==false)
			{
				msg = "Database not load yet.";
				response->Error(StatusOperationFailed, msg);
			    return;
			}
			
            shared_ptr<DatabaseInfo> db_info;
			apiUtil->get_databaseinfo(db_name, db_info);
			if (apiUtil->rdlock_databaseinfo(db_info))
			{
				msg = "get current database read lock fail.";
				response->Error(StatusOperationFailed, msg);
			    return;
			}
			ResultSet rs;
			bool update_flag_bool = true;
			int ret_val;
			FILE *output = NULL;
			string username = jsonParam(json_data, "username");
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
				response->Error(StatusOperationFailed,content);
				ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,0,_db_home,_db_suffix,content);
				return;
			}
			catch (const std::runtime_error &e2)
			{
				string content = e2.what();
				apiUtil->unlock_databaseinfo(db_info);
				response->Error(StatusOperationFailed,content);
				ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,0,_db_home,_db_suffix,content);
				return;
			}
			catch (...)
			{
				string content = "unknow error";
				apiUtil->unlock_databaseinfo(db_info);
				response->Error(StatusOperationFailed,content);
				ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,0,_db_home,_db_suffix,content);
				return;
			}
			int effectNum = 0;
			string checkMsg = "ok";
			string json= rs.to_JSON();
		    Document doc2;
			doc2.SetObject();
			doc2.Parse(json.c_str());
			if(doc2.HasParseError())
			{
				checkMsg = "query result is not json format!";
				effectNum = 0;
			}
			else
			{
				if(doc2.HasMember("results"))
				{
					Value results=doc2["results"].GetObject();
					if(results.HasMember("bindings"))
					{
						Value bindings=results["bindings"].GetArray();
						if(bindings.Size()>0)
						{
							Value resultobj=bindings[0]["result"].GetObject();
							if(resultobj.HasMember("value"))
							{
								string result_value=resultobj["value"].GetString();
								// int result_value_int=Util::string2int(result_value);
								effectNum=Util::string2int(result_value);
							}
							else
							{
								effectNum=0;
							}
						}
						else
						{
							effectNum=0;
						}
					}
				}
			}
			Document doc;
			doc.SetObject();
			Document::AllocatorType &allocator = doc.GetAllocator();
			doc.AddMember("check_sparql", StringRef(resultInfo.check_sparql.c_str()), allocator);
			doc.AddMember("effectNum", effectNum, allocator);
			doc.AddMember("checkMsg", StringRef(checkMsg.c_str()), allocator);
			doc.AddMember("StatusCode", 0, allocator);
			doc.AddMember("StatusMsg", "ok", allocator);
			ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,effectNum,_db_home,_db_suffix,checkMsg);
            ReasonHelper::updateReasonRuleStatus(rulename, db_name, "已校验",_db_home,_db_suffix);
		    response->Json(doc);
		}
		else
		{
			msg =  "The operation is not support.";
			response->Error(StatusParamIsIllegal, msg);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Reason Manage fail: " + string(e.what());
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
		vector<shared_ptr<struct DBUserInfo>> userList;
		apiUtil->get_user_info(userList);
		if (userList.empty())
		{
			response->Success("No Users");
			return;
		}
		size_t count = userList.size();
		Json resp_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.SetObject();
		rapidjson::Value array_data(rapidjson::kArrayType);
		for (size_t i = 0; i < count; i++)
		{
			shared_ptr<struct DBUserInfo> useInfo = userList[i];
			array_data.PushBack(useInfo->toJSON(allocator).Move(), allocator);
		}

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
		std::string msg;
		if (apiUtil->check_param_value("type", type, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}  
		else if (type != "1" && type != "2" && type != "3")
		{
			msg =  "The type " + type + " is not support.";
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		std::string op_username = jsonParam(json_data, "op_username");
		if (apiUtil->check_param_value("op_username", op_username, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		} 
		else if (apiUtil->check_user_exist(op_username) == false)
		{
			msg =  "The username is not exists.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		}
		else if (op_username == apiUtil->get_root_username())
		{
			msg =  "You can't change privileges for root user.";
			response->Error(StatusOperationConditionsAreNotSatisfied, msg);
			return;
		} 
		
		std::string db_name = jsonParam(json_data, "db_name");
		std::string privileges = jsonParam(json_data, "privileges");
		if (type != "3")
		{
			if (apiUtil->check_param_value("db_name", db_name, msg) == false)
			{
				response->Error(StatusParamIsIllegal, msg);
				return;
			}
			// check database exist
			if (apiUtil->check_db_built(db_name) == false)
			{
				msg =  "Database not build yet.";
				response->Error(StatusOperationConditionsAreNotSatisfied, msg);
				return;
			}
			if (apiUtil->check_param_value("privileges", privileges, msg) == false)
			{
				response->Error(StatusParamIsIllegal, msg);
				return;
			}
		} 
		else if (type == "3")
		{
			// clear the user all privileges
			if (apiUtil->clear_privilege(op_username))
			{
				response->Success("Clear the all privileges for the user successfully!");
			}
			else
			{
				msg =  "Clear the all privileges for the user fail.";
				response->Error(StatusOperationFailed, msg);
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
				if (type == "1")
				{
					if (apiUtil->add_privilege(op_username, privilegeTypes, db_name) == 0)
					{
						result = result + "add privilege " + privilegeNames + " failed.";
						response->Error(StatusOperationFailed, result);
					}
					else
					{
						result = result + "add privilege " + privilegeNames + " successfully.";
						response->Success(result);
					}
				}
				else if (type == "2")
				{
					if (apiUtil->del_privilege(op_username, privilegeTypes, db_name) == 0)
					{
						result += "delete privilege " + privilegeNames + " failed.";
						response->Error(StatusOperationFailed, result);
					}
					else
					{
						result += "delete privilege " + privilegeNames + " successfully.";
						response->Success(result);
					}
				}
				else
				{
					result = "the operation type is not support.";
					response->Error(StatusParamIsIllegal, result);
				}
			}
			else
			{
				result = "not match any valid privilege, valid values between 1 and 7.";
				response->Error(StatusParamIsIllegal, result);
			}
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
		std::string msg;
		if (apiUtil->check_param_value("op_password", op_password, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		std::string username = jsonParam(json_data, "username");
		if (apiUtil->check_user_exist(username) == false)
		{
			msg =  "Username does not exist.";
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		if (apiUtil->user_pwd_alert(username, op_password))
		{
			response->Success("Change password done.");
		}
		else
		{
			response->Success("Change password fail.");
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
		// 	std::string msg =  "Root User Only!";
		// 	response->Error(StatusOperationConditionsAreNotSatisfied, error);
		// 	return;
		// }
		int page_no = jsonParam(json_data, "pageNo", 1);
		int page_size = jsonParam(json_data, "pageSize", 10);
		shared_ptr<struct TransactionLogs> transactionLogsPtr = make_shared<struct TransactionLogs>();
		apiUtil->get_transactionlog(page_no, page_size, transactionLogsPtr);
		vector<struct TransactionLogInfo> logList = transactionLogsPtr->getTransactionLogInfoList();
		size_t count = logList.size();
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		rapidjson::Value array_data(rapidjson::kArrayType);
		for (size_t i = 0; i < count; i++)
		{
			TransactionLogInfo log_info = logList[i];
			array_data.PushBack(log_info.toJSON(allocator).Move(), allocator);
		}
		int totalSize = transactionLogsPtr->getTotalSize();
		int totalPage = transactionLogsPtr->getTotalPage();
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
		std::string msg;
		if (apiUtil->check_param_value("date", date, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		int page_no = jsonParam(json_data, "pageNo", 1);
		int page_size = jsonParam(json_data, "pageSize", 10);
		shared_ptr<struct DBQueryLogs> dbQueryLogsPtr = make_shared<struct DBQueryLogs>();
		apiUtil->get_query_log(date, page_no, page_size, dbQueryLogsPtr);
		vector<struct DBQueryLogInfo> logList = dbQueryLogsPtr->getQueryLogInfoList();
		size_t count = logList.size();
		
		Json resp_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.SetObject();
		rapidjson::Value array_data(rapidjson::kArrayType);
		for (size_t i = 0; i < count; i++)
		{
			DBQueryLogInfo log_info = logList[i];
			array_data.PushBack(log_info.toJSON(allocator).Move(), allocator);
		}

		int totalSize = dbQueryLogsPtr->getTotalSize();
		int totalPage = dbQueryLogsPtr->getTotalPage();
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
		size_t count = logfiles.size();
		std::string item;
		Json resp_data;
		rapidjson::Value array_data(rapidjson::kArrayType);
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		for (size_t i = 0; i < count; i++)
		{
			item = logfiles[i];
			item = item.substr(0, item.length()-4); // file_name: yyyyMMdd.log
			array_data.PushBack(rapidjson::Value().SetString(item.c_str(), allocator).Move(), allocator);
		}
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
		std::string msg;
		if (apiUtil->check_param_value("date", date, msg) == false)
		{
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		int page_no = jsonParam(json_data, "pageNo", 1);
		int page_size = jsonParam(json_data, "pageSize", 10);
		shared_ptr<struct DBAccessLogs> dbAccessLogsPtr = make_shared<struct DBAccessLogs>();
		apiUtil->get_access_log(date, page_no, page_size, dbAccessLogsPtr);
		vector<struct DBAccessLogInfo> logList = dbAccessLogsPtr->getAccessLogInfoList();
		size_t count = logList.size();
		Json resp_data;
		rapidjson::Value array_data(rapidjson::kArrayType);
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		for (size_t i = 0; i < count; i++)
		{
			DBAccessLogInfo log_info = logList[i];
			array_data.PushBack(log_info.toJSON(allocator).Move(), allocator);
		}
		int totalSize = dbAccessLogsPtr->getTotalSize();
		int totalPage = dbAccessLogsPtr->getTotalPage();
		resp_data.SetObject();
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
		size_t count = logfiles.size();
		std::string item;
		Json resp_data;
		rapidjson::Value array_data(rapidjson::kArrayType);
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		for (size_t i = 0; i < count; i++)
		{
			item = logfiles[i];
			item = item.substr(0, item.length()-4); // file_name: yyyyMMdd.log
			array_data.PushBack(rapidjson::Value().SetString(item.c_str(), allocator).Move(), allocator);
		}
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
			pfnUtil->build_PFNInfo(fun_info, &pfn_info);
		}
		std::string username =  jsonParam(json_data, "username");
		struct PFNInfos *pfn_infos = new PFNInfos();
		pfnUtil->fun_query(pfn_info.getFunName(), pfn_info.getFunStatus(), username, pfn_infos);
		vector<struct PFNInfo> list = pfn_infos->getPFNInfoList();
		size_t count = list.size();
		Json resp_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		rapidjson::Value array_data(rapidjson::kArrayType);
		for (size_t i = 0; i < count; i++)
		{
			PFNInfo pfn_info = list[i];
			array_data.PushBack(pfn_info.toJSON(allocator).Move(), allocator);
		}
		resp_data.SetObject();
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
	std::string msg;
	if (apiUtil->check_param_value("type", type, msg) == false)
	{
		response->Error(StatusParamIsIllegal, msg);
		return;
	}
	if (hasJsonParam(json_data, "funInfo") == false)
	{
		msg =  "the value of funInfo can not be empty!";
		response->Error(StatusParamIsIllegal, msg);
		return;
	}
	std::string username = jsonParam(json_data, "username");
	struct PFNInfo pfn_info;
	rapidjson::Value &fun_info = json_data["funInfo"];
	pfnUtil->build_PFNInfo(fun_info, &pfn_info);
	if (type == "1")
	{
		try
		{
			pfnUtil->fun_create(username, &pfn_info);
			response->Success("Function create success.");
		}
		catch(const std::exception& e)
		{
			msg = "Function create fail: " + string(e.what());
			response->Error(StatusOperationFailed, msg);
		}
	}
	else if (type == "2")
	{
		try
		{
			pfnUtil->fun_update(username, &pfn_info);
			response->Success("Function update success.");
		}
		catch(const std::exception& e)
		{
			msg = "Function update fail: " + string(e.what());
			response->Error(StatusOperationFailed, msg);
		}
	}
	else if (type == "3")
	{
		try
		{
			pfnUtil->fun_delete(username, &pfn_info);
			response->Success("Function delete success.");
		}
		catch(const std::exception& e)
		{
			msg = "Function delete fail: " + string(e.what());
			response->Error(StatusOperationFailed, msg);
		}
	}
	else if (type == "4")
	{
		try
		{
			string result = pfnUtil->fun_build(username, pfn_info.getFunName());
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
			msg = "Function build fail: " + string(e.what());
			response->Error(StatusOperationFailed, msg);
		}
	}
	else
	{
		msg = "The type is invalid, please look up the api document.";
		response->Error(StatusParamIsIllegal, msg);
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
		std::string msg;
		if (hasJsonParam(json_data, "funInfo") == false)
		{
			msg =  "the value of funInfo can not be empty!";
			response->Error(StatusParamIsIllegal, msg);
			return;
		}
		std::string username = jsonParam(json_data, "username");
		struct PFNInfo pfn_info;
		rapidjson::Value &fun_info = json_data["funInfo"];
		pfnUtil->build_PFNInfo(fun_info, &pfn_info);
		pfnUtil->fun_review(username, &pfn_info);
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

void stat_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		int pid = getpid();
		float cup_usage = Util::get_cpu_usage(pid) * 100; // %
		char cup_usage_char[32];
		sprintf(cup_usage_char, "%f", cup_usage);
		float mem_usage = Util::get_memory_usage(pid); // MB
		char mem_usage_char[32];
		sprintf(mem_usage_char, "%f", mem_usage);
		unsigned long long disk_available = Util::get_disk_free(); // MB
		char disk_available_char[32];
		sprintf(disk_available_char, "%llu", disk_available);
		Json resp_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.SetObject();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		resp_data.AddMember("cup_usage", StringRef(cup_usage_char), allocator);
		resp_data.AddMember("mem_usage", StringRef(mem_usage_char), allocator);
		resp_data.AddMember("disk_available", StringRef(disk_available_char), allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		std::string error = "stat fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

void checkOperationState_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	string msg;
	string operation = "checkOperationState";
	try
	{
		std::string opt_id = jsonParam(json_data, "opt_id");
		if (apiUtil->check_param_value("opt_id", opt_id, msg))
		{
			response->Error(StatusOperationFailed, msg);
			return;
		}
		struct DBAccessLogInfo log;
		if (!apiUtil->getAccessLogByOptId(opt_id, log))
		{
			msg =  "opt_id not found.";
			response->Error(StatusOperationFailed, msg);
			return;
		}
		Json resp_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		string msg = log.getMsg();
		std::string backupfilepath = log.getBackupfilepath();
		resp_data.SetObject();
		resp_data.AddMember("StatusCode", log.getCode(), allocator);
		resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
		resp_data.AddMember("state", log.getState(), allocator);
		std::string log_operation = log.getOperation();
		if (log_operation == "backup")
		{
			resp_data.AddMember("backupfilepath", StringRef(backupfilepath.c_str()), allocator);
		}
		else if(log_operation != "restore")
		{
			resp_data.AddMember("success_num", log.getNum(), allocator);
			resp_data.AddMember("failed_num", log.getFailNum(), allocator);
		}
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "checkbatchInsertUid fail:" + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

void cluster_heartbeat_task(const GRPCReq *request, GRPCResp *response)
{
	Json json_data;
	parseRequest(request, json_data);
	std::string expection = jsonParam(json_data, "operation");
	const cluster::ClusterOperation expectionEnum = cluster::ClusterOperationHandle::to_enum(expection);
	server::MessageClusterRequest request_data(json_data, _server_port);
	switch (expectionEnum)
	{
		case cluster::ClusterOperation_Compare:
			// compare term and index with leader
			server::ApiHandler::cluster_heartbeat_compare(apiUtil, clusterManagerPtr, request_data);
			response->Json("ok");
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
	if (response_data.status_code != server::StatusOK)
	{
		response->Error(response_data.status_code, response_data.status_msg);
	}
	else
	{
		response->Success("ok");
	}
}

void cluster_reply_task(const GRPCReq *request, GRPCResp *response)
{
	Json json_data;
	parseRequest(request, json_data);
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	server::MessageClusterReplyRequest resquest_data(json_data, _server_port);
	server::ApiHandler::cluster_reply(apiUtil, clusterManagerPtr, resquest_data, ip_addr);
	response->Success("ok");
}

void cluster_check_task(const GRPCReq *request, GRPCResp *response)
{
	Json json_data;
	parseRequest(request, json_data);
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	server::MessageClusterCheckRequest resquest_data(json_data, _server_port);
	server::ApiHandler::cluster_check(apiUtil, clusterManagerPtr, resquest_data, ip_addr);
	response->Success("ok");
}

void cluster_recover_task(const GRPCReq *request, GRPCResp *response)
{
	Form &form = request->form();
	server::MessageResponse response_data;
	server::ApiHandler::cluster_recover(apiUtil, clusterManagerPtr, form, response_data, _server_port);
	if (response_data.status_code != server::StatusOK)
	{
		response->Error(response_data.status_code, response_data.status_msg);
	}
	else
	{
		response->Success("ok");
	}
}