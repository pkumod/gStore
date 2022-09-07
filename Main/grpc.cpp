#include <stdio.h>
#include "workflow/WFFacilities.h"
#include "../GRPC/grpc_server.h"
#include "../GRPC/grpc_status_code.h"
#include "../GRPC/APIUtil.h"

using namespace std;
using namespace grpc;

static WFFacilities::WaitGroup wait_group(1);

APIUtil *apiUtil = nullptr;

int initialize(int argc, char *argv[]);

void register_service(GRPCServer &grpcServer);

void api(const GRPCReq *request, GRPCResp *response);

void check_task(const GRPCReq *request, GRPCResp *response, Json &json_data);

void show_task(const GRPCReq *request, GRPCResp *response, Json &json_data);

void sig_handler(int signo)
{
	wait_group.done();
}

int main(int argc, char *argv[])
{
	Util util;
	srand(time(NULL));
	while (true)
	{
		pid_t fpid = fork();
		// cout << "fpid:" << fpid << endl;
		if (fpid == 0)
		{
			int ret = initialize(argc, argv);
			exit(ret);
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
	apiUtil = new APIUtil();
	unsigned short port = 9000;
	string db_name = "";
	string port_str = "9000";
	bool loadCSR = 0; // DO NOT load CSR by default
	stringstream ss;
	if (argc < 2)
	{
		SLOG_DEBUG("Server will use the default port: " + port_str);
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
		port_str = Util::getArgValue(argc, argv, "p", "port", "9000");
		port = atoi(port_str.c_str());
		loadCSR = Util::string2int(Util::getArgValue(argc, argv, "c", "csr", "0"));
	}
	// check port.txt exist
	if (Util::file_exist("system.db/port.txt"))
	{
		cout << "Server port " + port_str + " is already in use." << endl;
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

	grpcServer.start(port);

	SLOG_DEBUG("grpc server port " + port_str);

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
		"/shutdown", [](const GRPCReq *request, GRPCResp *response)
		{ SLOG_DEBUG("shutdown GRPC server ok"); },
		methods);

	svr.ROUTE(
		"/", [](const GRPCReq *request, GRPCResp *response)
		{ api(request, response); },
		methods);
}

void api(const GRPCReq *request, GRPCResp *response)
{
	// check ip address
	auto *rpc_task = task_of(response);
	string ip_addr = rpc_task->peer_addr();
	string ipCheckResult = apiUtil->check_access_ip(ip_addr);
	if (ipCheckResult != "")
	{
		response->Error(StatusIPBlocked, ipCheckResult);
		return;
	}

	Json json_data;
	Json::AllocatorType &allocator = json_data.GetAllocator();
	if (request->contentType() == APPLICATION_URLENCODED)
	{
		std::map<std::string, std::string> params = request->queryList();
		std::map<std::string, std::string>::iterator iter = params.begin();
		while (iter != params.end())
		{
			json_data.AddMember(StringRef(iter->first.c_str()), StringRef(iter->second.c_str()), allocator);
			iter++;
		}
	}
	else if (request->contentType() == APPLICATION_JSON)
	{
		Json &json = request->json();
		json_data.CopyFrom(json, allocator, true);
	}

	string operation = json_data["operation"].GetString();
	if (operation == "check")
	{
		check_task(request, response, json_data);
		return;
	}

	if (json_data.HasMember("username") == false || json_data.HasMember("password") == false)
	{
		response->Error(StatusParamIsIllegal, "username or password is empty");
		return;
	}
	string username = json_data["username"].GetString();
	string password = json_data["password"].GetString();
	string encryption;
	string db_name;
	if (json_data.HasMember("encryption"))
	{
		encryption = json_data["encryption"].GetString();
	}
	if (json_data.HasMember("db_name"))
	{
		db_name = json_data["db_name"].GetString();
	}
	// check username and password
	string checkidentityresult = apiUtil->check_indentity(username, password, encryption);
	if (checkidentityresult.empty() == false)
	{
		response->Error(StatusAuthenticationFailed, checkidentityresult);
		return;
	}
	// check privilege
	if (apiUtil->check_privilege(username, operation, db_name) == 0)
	{
		string msg = "You have no " + operation + " privilege, operation failed";
		response->Error(StatusOperationConditionsAreNotSatisfied, msg);
		return;
	}
	// api operation
	if (operation == "show")
	{
		show_task(request, response, json_data);
	}
}

void check_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	string success = "the grpc server is running...";
	response->Success(success);
}

void show_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		string username = json_data["username"].GetString();

		vector<struct DatabaseInfo *> array;
		apiUtil->get_already_builds(username, array);

		Json json_data;
		json_data.IsObject();
		Document::AllocatorType &allocator = json_data.GetAllocator();
		size_t count = array.size();
		string line;
		stringstream str_stream;
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
		Document jsonArray;
		jsonArray.SetArray();
		line = str_stream.str();
		jsonArray.Parse(line.c_str());

		json_data.AddMember("StatusCode", 0, allocator);
		json_data.AddMember("StatusMsg", "Get the database list successfully!", allocator);
		json_data.AddMember("ResponseBody", jsonArray, allocator);

		// set response status and message
		response->Json(json_data);
	}
	catch (std::exception &e)
	{
		string msg = "Show failed:" + string(e.what());
		response->Error(StatusOperationFailed, msg);
	}
}