#ifndef _UTIL_LATCH_H
#define _UTIL_LATCH_H

#include <pthread.h>
class Latch
{
private:
	/// The real latch
	pthread_rwlock_t lock;

	Latch(const Latch&);
	void operator=(const Latch&);

public:
	/// Constructor
	Latch();
	/// Destructor
	~Latch();

	/// Lock exclusive
	void lockExclusive();
	/// Try to lock exclusive
	bool tryLockExclusive();
	/// Lock shared
	void lockShared();
	/// Try to lock shared
	bool tryLockShared();
	/// Release the lock. Returns true if the lock seems to be free now (hint only)
	void unlock();
};

#endif
