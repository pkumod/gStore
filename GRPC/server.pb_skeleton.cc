
#include "grpc.srpc.h"
#include "workflow/WFFacilities.h"

using namespace srpc;

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

class ExampleServiceImpl : public Example::Service
{
public:

	void Echo(EchoRequest *request, EchoResponse *response, srpc::RPCContext *ctx) override
	{}
};

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	unsigned short port = 1412;
	SRPCServer server;

	ExampleServiceImpl example_impl;
	server.add_service(&example_impl);

	server.start(port);
	wait_group.wait();
	server.stop();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
