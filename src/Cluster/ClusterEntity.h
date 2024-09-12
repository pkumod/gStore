#pragma once
#include "ClusterDefined.h"
#include "../Util/Util.h"
#include "../Api/TimerProvider.h"
#include "../Api/HttpUtil.h"
#include "ClusterLog.h"
#include <map>
#include <thread>

namespace cluster
{
    class ClusterEntity : public std::enable_shared_from_this<ClusterEntity>
    {
        public:
        ClusterEntity(){}
        virtual ~ClusterEntity(){}

        // virtual function in here
        public:
        virtual void init() = 0;
        virtual ClusterRoleType getCluterRoleType()const = 0;
        virtual void startHeardBeat(){}
        virtual uint32 startNotify(uint32 term, uint32 index){return 0;}
        virtual uint32 startSyncNum(uint32 term, uint32 index, const std::string& nt){return 0;}
    };
}