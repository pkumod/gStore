#pragma once
#include "ClusterDefined.h"
#include "../Util/Util.h"

namespace cluster
{
    class ClusterLog
    {
        uint32 term;
        uint32 index;
        uint32 status;
        uint32 nodeNum;

        public:
        ClusterLog();
        ~ClusterLog();

        void init();

        static uint32 getNodeNum();
        static void addLog(uint32 term, uint32 index, uint32 status, uint32 nodeNum);
        static void updateLog(uint32 term, uint32 index, uint32 status, uint32 nodeNum);
    };
}