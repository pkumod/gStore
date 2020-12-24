/*=============================================================================
# Filename: ResultFilter.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-05-03 15:36
# Description:
=============================================================================*/

#ifndef _QUERY_RESULTFILTER_H
#define _QUERY_RESULTFILTER_H

#include "BasicQuery.h"
#include "SPARQLquery.h"
#include "../Util/Util.h"

class ResultFilter {
  private:
  static const unsigned MAX_SIZE = 1048576;
  inline unsigned hash(unsigned x)
  {
    x = (x + 0x7ed55d16) + (x << 12);
    x = (x ^ 0xc761c23c) ^ (x >> 19);
    x = (x + 0x165667b1) + (x << 5);
    x = (x + 0xd3a2646c) ^ (x << 9);
    x = (x + 0xfd7046c5) + (x << 3);
    x = (x ^ 0xb55a4f09) ^ (x >> 16);
    return x & (MAX_SIZE - 1);
  }

  //var	valid	hash_table
  std::map<std::string, pair<int, std::vector<int> > > result_filter;

  public:
  void addVar(std::string var);
  void changeResultHashTable(SPARQLquery& query, int value);
  void candFilterWithResultHashTable(BasicQuery& basicquery);
};

#endif // _QUERY_RESULTFILTER_H
