#ifndef _GRPC_SERVERTASK_H
#define _GRPC_SERVERTASK_H

#include "grpc_message.h"
#include "grpc_noncopyable.h"

namespace grpc
{
class GRPCServerTask :  public WFServerTask<GRPCReq, GRPCResp> , public Noncopyable
{
public:
    using ProcFunc = std::function<void(GRPCTask *)>;
    using ServerCallBack = std::function<void(GRPCTask *)>;

    using WFServerTask::Series;

    GRPCServerTask(CommService *service, ProcFunc &process);

    void add_callback(const ServerCallBack &cb)
    { 
        cb_list_.push_back(cb);
    }

    void add_callback(ServerCallBack &&cb)
    {
        cb_list_.emplace_back(std::move(cb));
    }

    static size_t get_resp_offset()
    {
        GRPCServerTask task(nullptr);
        return task.resp_offset();
    }

    std::string peer_addr() const;

    unsigned short peer_port() const;
protected:
    void handle(int state, int error) override;

    CommMessageOut *message_out() override;
private:

    void set_callback()
    {}

    size_t resp_offset() const
    {
        return (const char *) (&this->resp) - (const char *) this;
    }

    GRPCServerTask(std::function<void(GRPCTask *)> proc) : WFServerTask(nullptr, nullptr, proc)
    {}

private:
    bool req_is_alive_;
    bool req_has_keep_alive_header_;
    std::string req_keep_alive_;
    std::vector<ServerCallBack> cb_list_;
};

inline GRPCServerTask *task_of(const SubTask *task)
{
    auto *series = static_cast<GRPCServerTask::Series *>(series_of(task));
    return static_cast<GRPCServerTask *>(series->task);
}

inline GRPCServerTask *task_of(const GRPCResp *resp)
{
    size_t http_resp_offset = GRPCServerTask::get_resp_offset();
    return (GRPCServerTask *) ((char *) (resp) - http_resp_offset);
}

} // namespace grpc

#endif // _GRPC_SERVERTASK_H