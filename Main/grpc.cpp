#include "GRPC/grpc.srpc.h"
#include "srpc/rpc_module.h"
#include "srpc/rpc_span_policies.h"
#include "srpc/rpc_types.h"
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

int main()
{
	SRPCServer server_srpc;
	SRPCHttpServer server_srpc_http;
	BRPCServer server_brpc;
	ThriftServer server_thrift;

	ExampleServiceImpl impl_pb;
	AnotherThriftServiceImpl impl_thrift;

	server_srpc.add_service(&impl_pb);
	server_srpc.add_service(&impl_thrift);
	server_srpc_http.add_service(&impl_pb);
	server_srpc_http.add_service(&impl_thrift);
	server_brpc.add_service(&impl_pb);
	server_thrift.add_service(&impl_thrift);

	server_srpc.start(1412);
	server_srpc_http.start(8811);
	server_brpc.start(2020);
	server_thrift.start(9090);
	pause();
	server_thrift.stop();
	server_brpc.stop();
	server_srpc_http.stop();
	server_srpc.stop();

	return 0;
}