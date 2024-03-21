#include "GraphLock.h"

bool GLatch::is_own_latch(ull TID)
{
    bool ret;
    latch.lock();
    ret = (_TID == TID);
    latch.unlock();
    return ret;
}

bool GLatch::tryexclusivelatch(ull TID, bool IS_SR)
{
    bool ret = true;
    latch.lock();
    if(_TID == TID){
        //owned lock do nothing;
        if(IS_SR) assert(_RCNT == 1);
        ret = true;
    }
    else{
        if(IS_SR){
            if(_TID != INVALID_TID || _RCNT != 0){
                ret = false;
            }
            else{
                _TID = TID; _RCNT = 1;
                ret = true;
            }
        }
        else{
            if(_TID != INVALID_TID) ret = false;
            else{
                _TID = TID; 
                ret = true;
            }
        }
    }
    latch.unlock();
    return ret;
}

bool GLatch::trysharedlatch(ull TID)
{
    bool ret = true;
    latch.lock();
    if(_TID == TID){
        //owned lock
        assert(_RCNT == 1);
        ret = true;
    } 
    else{
        if(_TID != INVALID_TID) ret = false;
        else{
            _RCNT++;
            ret = true;
        }
    }
    latch.unlock();
    return ret;
}

bool GLatch::unlatch(ull TID, bool type, bool IS_SR)
{
    bool ret = true;
    latch.lock();
    if(type == true) //X lock
    {
       if(TID != _TID){
           ret = false;
           cerr << "X latch released failed, wrong TID" << _TID << "   " << _RCNT << endl;
           //assert(TID == _TID);
       }
       else{
           if(IS_SR){
               if(_RCNT != 1){ // TID 1 -> INVALID_TID 0
                   ret = false;
                   cerr << "X latch released failed, wrong read count" << endl;
               }
               else{
                   _RCNT--;
                   _TID = INVALID_TID;
                   ret = true;
               }
           }
           else{
               _TID = INVALID_TID; // TID -> INVALID_TID
               ret = true;
           }
       }
    }
    else
    {
        if(!IS_SR){
            ret = false;
        }
        else{
            assert(_TID == INVALID_TID);
            if(_RCNT <= 0){ // INVALID_TID RCNT -> INVALID_TID RCNT-1
                cerr << "read count error!" << endl;
                ret = false;
            }
            else{
                _RCNT--;
                ret = true;
            }
        }
    }
    latch.unlock();
    return ret;
}

bool GLatch::tryupgradelatch(ull TID)
{
    bool ret = true;
    latch.lock();
    if(_TID != INVALID_TID ){ //X locked 
        ret = false;
    }
    else{
        if(_RCNT != 1){ // S locked
            ret = false;
        }
        else{
            _TID = TID;
            ret = true;
        }
    }
    latch.unlock();
    if(ret == false) assert(_TID == INVALID_TID);
    return ret;
}

bool GLatch::trydowngradelatch(ull TID)
{
    bool ret = true;
    latch.lock();
    if(_TID != TID){
        ret = false;
        cerr << "down grade latch failed wrong TID!" << endl;
    }
    else{
        if(_RCNT != 1){
            ret = false; 
            cerr << "down grade latch failed wrong read count!" << endl;
        }
        else{
            _TID = INVALID_TID;
            ret = true;
        }
    }
    latch.unlock();
    return ret;
}
