#include <signal.h>
#include "../GRPC/grpc.srpc.h"
#include "workflow/WFFacilities.h"
#include "srpc/rpc_module.h"
#include "srpc/rpc_span_policies.h"
#include "srpc/rpc_types.h"
using namespace srpc;
static WFFacilities::WaitGroup wait_group(1);
class ExampleServiceImpl : public Example::Service
{
public:
	void Echo(EchoRequest* request, EchoResponse* response, RPCContext* ctx) override
	{
		response->set_message("Hi, " + request->name());

		printf("get_req:\n%s\nset_resp:\n%s\n",
			request->DebugString().c_str(),
			response->DebugString().c_str());
	}
};
static void sig_handler(int signo)
{
	wait_group.done();
}
int main()
{
	//SRPCServer server_srpc;
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	SRPCHttpServer server_srpc_http;
	//BRPCServer server_brpc;
	//ThriftServer server_thrift;

	ExampleServiceImpl impl_pb;
	//AnotherThriftServiceImpl impl_thrift;

	/*server_srpc.add_service(&impl_pb);*/
	//server_srpc.add_service(&impl_thrift);
	server_srpc_http.add_service(&impl_pb);
	//server_srpc_http.add_service(&impl_thrift);
	//server_brpc.add_service(&impl_pb);
	//server_thrift.add_service(&impl_thrift);

	//server_srpc.start(1412);
	int result=server_srpc_http.start(8811);
	/*server_brpc.start(2020);
	server_thrift.start(9090);*/
	if (result == 0)
	{
		wait_group.wait();
		server_srpc_http.stop();
	}
	else
		perror("server start");

	/*pause();
	server_thrift.stop();
	server_brpc.stop();
	server_srpc_http.stop();
	server_srpc.stop();*/

	return 0;
}