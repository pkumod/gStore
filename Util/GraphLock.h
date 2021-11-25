#ifndef _UTIL_GRAPHLOCK_H
#define _UTIL_GRAPHLOCK_H

#include <atomic>
#include <utility>
#include "SpinLock.h"
#include <assert.h>
#include <iostream>
#include <mutex>
using namespace std;
typedef unsigned long long ull;

const ull INVALID_TID = (ull)-1;
inline __uint128_t pack(ull TID, ull rcnt)
{
    __uint128_t res = TID;
    res <<= 64;
    res += rcnt;
    return res;
}

inline pair<ull, ull> unpack(__uint128_t& lock)
{
    return {lock >> 64, lock};
}

struct GLatch{
    spinlock latch;
    ull _TID;
    ull _RCNT;

    explicit GLatch() 
    {
        _TID = INVALID_TID;
        _RCNT = 0;
    }

    GLatch(ull TID, ull rcnt){
        _TID = TID;
        _RCNT = rcnt;
    }

    GLatch(const GLatch &g){
        _TID = g._TID;
        _RCNT = g._RCNT;
    }

    GLatch &operator=(const GLatch &rhs)
    {
        this->_TID = rhs._TID;
        this->_RCNT = rhs._RCNT;
        return *this;
    }

    bool is_own_latch(ull TID);
    
    bool tryexclusivelatch(ull TID, bool IS_SR);
    bool trysharedlatch(ull TID);
    bool unlatch(ull TID, bool type, bool IS_SR);
    bool tryupgradelatch(ull TID);
    bool trydowngradelatch(ull TID);


    //status
    bool is_exclusive_latched(bool IS_SR)
    {
        latch.lock();
        bool ret = (_TID != INVALID_TID); 
        latch.unlock();
        return ret;
    }
    bool is_shared_latched(bool IS_SR)
    {
        latch.lock();
        bool ret = (_RCNT != 0);
        latch.unlock();
        return ret;
    }
    bool is_latched(bool IS_SR)
    {
        latch.lock();
        bool ret = (_TID != INVALID_TID) || (_RCNT != 0);
        latch.unlock();
        return ret;
    }
    ull get_readers(){
        latch.lock();
        int ret = _RCNT;
        latch.unlock();
        return ret;
    };
    ull get_TID(){
        latch.lock();
        int ret = _TID;
        latch.unlock();
        return ret;
    }
    //check before shared latch 

};

#endif