#ifndef _GRPC_NONCOPYABLE_H
#define _GRPC_NONCOPYABLE_H

namespace grpc
{

class Noncopyable
{
public:
    Noncopyable(const Noncopyable&) = delete;
    void operator=(const Noncopyable&) = delete;

protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
};

} // namespace grpc

#endif // _GRPC_NONCOPYABLE_H