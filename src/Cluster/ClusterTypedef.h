#pragma once
#include <memory>
#include <functional>
#include "ClusterDefined.h"

typedef unsigned int uint32;
typedef int          int32;
typedef long long    int64;
namespace cluster
{
    class ClusterLog;
    class ClusterEntity;
    class ClusterEntity;
    class ClusterEntityLeader;
    class ClusterEntityFollow;
    typedef std::shared_ptr<ClusterLog> ClusterLogPtr;
    typedef std::shared_ptr<ClusterEntity> ClusterEntityPtr;
    typedef std::shared_ptr<ClusterEntityLeader> ClusterEntityLeaderPtr;
    typedef std::weak_ptr<ClusterEntityLeader> ClusterEntityLeaderWeaker;
    typedef std::shared_ptr<ClusterEntityFollow> ClusterEntityFollowPtr;
    typedef std::weak_ptr<ClusterEntityFollow> ClusterEntityFollowWeaker;
}