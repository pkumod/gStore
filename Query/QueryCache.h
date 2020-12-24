/*=============================================================================
# Filename: QueryCache.h
# Author: Hao, Cui
# Mail: prospace@bupt.edu.cn
# Last Modified: 2017-04-06
# Description: 
=============================================================================*/

#ifndef _QUERY_QUERYCACHE_H
#define _QUERY_QUERYCACHE_H

#include "../Util/Util.h"
#include "QueryTree.h"
#include "TempResult.h"
#include "Varset.h"

class QueryCache {
  private:
  const int TRIPLE_NUM_LIMIT = 30;
  const int VAR_NUM_LIMIT = 8;
  const int MINIMAL_EVA_TIME_LIMIT = 100;
  const long long ITEM_MEMORY_LIMIT = 1000000LL;
  const long long TOTAL_MEMORY_LIMIT = 100000000LL;

  mutex query_cache_lock;

  int time_now;
  long long total_memory_used;

  typedef std::vector<QueryTree::GroupPattern::Pattern> Patterns;
  typedef std::pair<int, std::pair<Patterns, std::vector<std::string> > > LRUStruct;

  std::map<Patterns, std::map<std::vector<std::string>, std::pair<std::vector<unsigned>, int> > > cache;
  std::priority_queue<LRUStruct, std::vector<LRUStruct>, std::greater<LRUStruct> > lru;

  bool getMinimalRepresentation(const Patterns& triple_pattern, Patterns& minimal_repre, std::map<std::string, std::string>& minimal_mapping);

  public:
  QueryCache()
      : time_now(0)
      , total_memory_used(0)
  {
  }

  bool tryCaching(const Patterns& triple_pattern, const TempResult& temp_result, int eva_time);
  bool checkCached(const Patterns& triple_pattern, const Varset& varset, TempResult& temp_result);
  void clear();
};

#endif // _QUERY_QUERYCACHE_H
