/*
a test and test-and-set lock(ttas)
replace mutex in multhreading environment to improve performance
Author:Erik Rigtorp
reference:https://rigtorp.se/spinlock/
*/

#ifndef _UTIL_SPINLOCK_H
#define _UTIL_SPINLOCK_H

#include <pthread.h>

struct spinlock {
  pthread_spinlock_t _lock;
  spinlock(){
    pthread_spin_init(&_lock, PTHREAD_PROCESS_PRIVATE);
  }
  void lock() {
    pthread_spin_lock(&_lock);
  }

  bool try_lock(){
    return pthread_spin_trylock(&_lock);
  }
  void unlock(){
    pthread_spin_unlock(&_lock);
  }

  ~spinlock(){
    pthread_spin_destroy(&_lock);
  }
};

#endif