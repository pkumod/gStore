#include "IdUtil.h"

namespace gs
{
    IdUtil::IdUtil(){}
    
    IdUtil::~IdUtil(){}
	
	uint64_t IdUtil::last_timestamp_ = 0;
	uint64_t IdUtil::sequence_ = 0;

    uint64_t IdUtil::nextTimestamp(uint64_t last_timestamp)
    {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
		while (timestamp <= last_timestamp)
		{
			now = std::chrono::system_clock::now();
			timestamp =	std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
		}
		return timestamp;
    }

    uint64_t IdUtil::nextUID()
    {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
		uint64_t sequence = 0;
		if (timestamp == last_timestamp_)
		{
			sequence = (sequence_ + 1) & kSequenceMask;
			if (sequence == 0)
			{
				// If the ID generated in the current millisecond exceeds the maximum value, wait for the next millisecond
				timestamp = nextTimestamp(last_timestamp_);
			}
		}
		else
		{
			sequence = 0;
		}
		last_timestamp_ = timestamp;
		sequence_ = sequence;
		uint64_t id = ((timestamp - kEpoch) << kTimestampShift) | (worker_id_ << kWorkerIdShift) | sequence;
		return id;
    }

    void IdUtil::nextUID(std::string& uid)
    {
        uid = std::to_string(nextUID());
    }

    std::string IdUtil::getConvertTimeById(const std::string& id)
	{
		if (id.empty())
		{
			return id;
		}
		time_t time = std::stoul(id);
		time = ((time >> kTimestampShift) + kEpoch)/1000;
		struct tm *timeinfo = nullptr;
		char buffer[64];
		timeinfo = localtime(&time);
		strftime(buffer,sizeof(buffer),"%Y%m%d",timeinfo);
		return std::string(buffer);
	}

	/**
	 * Generate a random number between 1 and 2147483647
	 */
	int IdUtil::randNum()
	{
		// Use the time function to get a "seed” value for srand
		time_t seed = time(0);
		srand(seed);
		int result = 0;
		while (result == 0)
		{
			result = rand();
		}
		return result;
	}
}