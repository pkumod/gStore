#pragma once
#include "GlobalTypedef.h"

namespace gutil
{
    class IdUtil
    {
    private:
        static const uint64_t kEpoch = 1710172800000; // 2024-03-12 00:00:00.000
        static const uint64_t kSequenceBits = 12;
        static const uint64_t kWorkerIdBits = 10;
        static const uint64_t kMaxWorkerId = (1 << kWorkerIdBits) - 1;
        static const uint64_t kSequenceMask = (1 << kSequenceBits) - 1;
        static const uint64_t kTimestampShift = kSequenceBits + kWorkerIdBits;
        static const uint64_t kWorkerIdShift = kSequenceBits;
        static const uint16_t worker_id_ = 1;
        static uint64_t last_timestamp_;
        static uint64_t sequence_;
        static uint64_t nextTimestamp(uint64_t last_timestamp);
    public:
        IdUtil();
        ~IdUtil();
        static uint64_t nextUID();
        static void nextUID(std::string& uid);
        static std::string getConvertTimeById(const std::string& id);
        static int randNum();
    };
}