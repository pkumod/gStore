#pragma once
#include <memory>
#include <functional>

typedef unsigned int      uint32;
typedef int               int32;
typedef long long         int64;
typedef unsigned long int uint64;
namespace cluster
{
    class ClusterDb;
    class ClusterDbNameLogInfo;
    class ClusterEntity;
    class ClusterEntityLeader;
    class ClusterEntityFollower;
    class TripleInfo;
    struct ClusterEvent;
    struct uint64StringPair;
    typedef std::shared_ptr<ClusterDbNameLogInfo> ClusterLogPtr;
    typedef std::shared_ptr<ClusterEntity> ClusterEntityPtr;
    typedef std::weak_ptr<ClusterEntity> ClusterEntityWeakPtr;
    typedef std::shared_ptr<ClusterEntityLeader> ClusterEntityLeaderPtr;
    typedef std::weak_ptr<ClusterEntityLeader> ClusterEntityLeaderWeaker;
    typedef std::shared_ptr<ClusterEntityFollower> ClusterEntityFollowerPtr;
    typedef std::weak_ptr<ClusterEntityFollower> ClusterEntityFollowerWeaker;
    typedef std::shared_ptr<ClusterDb> ClusterDbPtr;
    typedef std::shared_ptr<ClusterEvent> ClusterEventPtr;
}