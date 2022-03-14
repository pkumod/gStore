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
	stringstream ss;
	if (argc < 2)
	{
		ss << "Server will use the default port: ";
		ss << port;
		Util::formatPrint(ss.str());
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
			cout << "\t-p,--port[option],\t\t the listen port. Default value is 9000." << endl;
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
		string port_str = Util::getArgValue(argc, argv, "p", "port", "9000");
		port = atoi(port_str.c_str());
	}
	if (Util::checkPort(port, "grpc") == false)
	{
		ss.clear();
		ss.str("");
		ss << "Server port ";
		ss << port;
		ss << "  has been used.";
		Util::formatPrint(ss.str(), "ERROR");
		return 0;
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
	ss << "Server port ";
	ss << port;
	Util::formatPrint(ss.str());
	wait_group.wait();
	server.stop();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	while (true)
	{
		pid_t fpid = 0;//fork();
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