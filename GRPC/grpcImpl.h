/*
 * @Author: wangjian
 * @Date: 2022-11-28 09:55:49
 * @LastEditTime: 2022-04-18 15:22:37
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /gStore/GRPC/grpcImpl.h
 */
#include <stdio.h>
#include <regex>
#include "grpc.srpc.h"
#include "APIUtil.h"

using namespace srpc;
using namespace std;
using namespace rapidjson;

class GrpcImpl : public grpc::Service
{
public:
    GrpcImpl();
    GrpcImpl(int argc, char *argv[]);
    ~GrpcImpl();
    void api(CommonRequest *request, CommonResponse *response, srpc::RPCContext *ctx) override;
    void shutdown(CommonRequest *request, CommonResponse *response, srpc::RPCContext *ctx) override;
private:
    APIUtil *apiUtil;
    // for db
    void build_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void load_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void unload_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void monitor_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void drop_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void show_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void backup_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void restore_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void query_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void export_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void login_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void check_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void begin_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void tquery_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void commit_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void rollback_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void checkpoint_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void test_connect_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void core_version_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void batch_insert_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void batch_remove_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void shutdown_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    // for db user
    void user_manage_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void user_show_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void user_privilege_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void user_password_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    // for db log
    void txn_log_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void query_log_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void access_log_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    // for ip manage
    void ip_manage_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void parse_query_result(CommonRequest *&request, CommonResponse *&response,  ResultSet& rs);
    // for personalized function
    void fun_query_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void fun_cudb_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void fun_review_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
    void fun_copy_from_fun_info(struct PFNInfo * target, FunInfo * source);
    void fun_copy_from_pfn_info(FunInfo * target, struct PFNInfo * source);
    // for unknown operation
    void default_task(CommonRequest *&request, CommonResponse *&response, srpc::RPCContext *&ctx);
};

