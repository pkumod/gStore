#include <stdio.h>
#include "workflow/WFFacilities.h"
#include "../GRPC/grpcImpl.h"

using namespace std;
using namespace srpc;

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

int initialize(int argc, char *argv[])
{
	unsigned short port = 9000;
	string port_str = "9000";
	stringstream ss;
	if (argc < 2)
	{
		Util::formatPrint("Server will use the default port: " + port_str);
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
			Util::formatPrint("Invalid arguments! Input \"bin/grpc -h\" for help.");
			return 0;
		}
	}
	else
	{
		port_str = Util::getArgValue(argc, argv, "p", "port", "9000");
		port = atoi(port_str.c_str());
	}
	// check port.txt exist
	if (Util::file_exist("system.db/port.txt"))
	{
		Util::formatPrint("Server port " + port_str + " is already in use.", "ERROR");
		return -1;
	}

	// APIUtil grpcUtil;
	struct RPCServerParams params = srpc::RPC_SERVER_PARAMS_DEFAULT;
	// set max connections default is 2000
	params.max_connections = 10000;
	SRPCHttpServer server(&params);
	GrpcImpl grpc_service_impl(argc, argv);
	// register service
	server.add_service(&grpc_service_impl);

	server.start(port);
	ss.clear();
	ss.str("");
	ss << "grpc server port ";
	ss << port;
	Util::formatPrint(ss.str());
	//handle the Ctrl+C signal
	signal(SIGINT, sig_handler);

	wait_group.wait();
	server.stop();
	google::protobuf::ShutdownProtobufLibrary();
	Util::formatPrint("grpc server stoped.");
	return 0;
}

int main(int argc, char *argv[])
{
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
				Util::formatPrint("Stopped abnormally, restarting server...", "WARN");
			}
		}
		else
		{
			Util::formatPrint("Failed to start server: deamon fork failure.", "ERROR");
			return -1;
		}
	}

	return 0;
}