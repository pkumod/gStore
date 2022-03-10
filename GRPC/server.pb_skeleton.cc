/*
 * @Author: your name
 * @Date: 2022-03-10 20:12:56
 * @LastEditTime: 2022-03-10 20:12:57
 * @LastEditors: your name
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /gstore/GRPC/server.pb_skeleton.cc
 */

#include "grpc.srpc.h"
#include "workflow/WFFacilities.h"

using namespace srpc;

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

class grpcServiceImpl : public grpc::Service
{
public:

	void api(CommonRequest *request, CommonResponse *response, srpc::RPCContext *ctx) override
	{}
};

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	unsigned short port = 1412;
	SRPCServer server;

	grpcServiceImpl grpc_impl;
	server.add_service(&grpc_impl);

	server.start(port);
	wait_group.wait();
	server.stop();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
