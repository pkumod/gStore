/*=========================================================================
 * File name: IVCacheManager.h
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last Modified: 2018-01-30
 * Description:
 * To Manage the cache(in main memory) of Index-Value
 * =========================================================================*/

#include "../../Util/Util.h"
#include "IVEntry.h"

using namespace std;

class IVCacheManager {
  public:
  // Maximum size of cache
  static const unsigned MAX_CACHE_SIZE = 1 << 30; // 1G

  private:
  unsigned cur_cache_size;
  IVEntry* array;
  // Record the last used time of each value in cache
  map<unsigned, unsigned long> index_time_map;
  // Record time-index pair
  multimap<unsigned long, unsigned> time_index_map;

  public:
  IVCacheManager(IVEntry* p);
  bool CacheAdd(const unsigned& _index, const char* _str, const unsigned& _len);
  bool CacheSub(const unsigned& _index, const unsigned& _len);
};
