#include "Latch.h"

Latch::Latch()
// Constructor
{
	pthread_rwlock_init(&lock,0);
}
//---------------------------------------------------------------------------
Latch::~Latch()
// Destructor
{
	pthread_rwlock_destroy(&lock);
}
//---------------------------------------------------------------------------
void Latch::lockExclusive()
// Lock exclusive
{
	pthread_rwlock_wrlock(&lock);
}
//---------------------------------------------------------------------------
bool Latch::tryLockExclusive()
// Try to lock exclusive
{
	return pthread_rwlock_trywrlock(&lock)==0;
}
//---------------------------------------------------------------------------
void Latch::lockShared()
// Lock shared
{
	pthread_rwlock_rdlock(&lock);
}
//---------------------------------------------------------------------------
bool Latch::tryLockShared()
// Try to lock stared
{
	return pthread_rwlock_tryrdlock(&lock)==0;
}
//---------------------------------------------------------------------------
bool Latch::unlock()
// Release the lock
{
	return pthread_rwlock_unlock(&lock)==0;
}
