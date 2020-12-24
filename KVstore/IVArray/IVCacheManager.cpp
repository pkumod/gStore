/*========================================================================
 * File name: IVCacheManager.cpp
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last Modified: 2018-01-30
 * Description
 * Implementation of functions in IVCacheManager.h
 * =======================================================================*/

#include "IVCacheManager.h"

using namespace std;

IVCacheManager::IVCacheManager(IVEntry* p)
{
  cur_cache_size = 0;
  index_time_map.clear();
  time_index_map.clear();
  array = p;
}

bool
IVCacheManager::CacheAdd(const unsigned& _index, const char* _str, const unsigned& _len)
{
  if (_len + cur_cache_size > MAX_CACHE_SIZE)
    return false;

  array[index].setBstr(_str, _len);
  cur_cache_size += _len;
}

bool
IVCacheManager::CacheSub(const unsigned& _index, const unsigned& _len)
{
}
