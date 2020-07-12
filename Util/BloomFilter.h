/*=============================================================================
# Filename: BloomFilter.h
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-20 13:18
# Description: http://blog.csdn.net/hguisu/article/details/7866173
TODO:this strategy can be used in Join and KVstore-search/modify/remove, or the uppest level!
really better? appropiate if key num small but search too many!
=============================================================================*/

#ifndef _UTIL_BLOOMFILTER_H
#define _UTIL_BLOOMFILTER_H

#include "Util.h"

#define SETBIT(ch, n) ch[n / 8] |= 1 << (7 - n % 8)
#define GETBIT(ch, n) (ch[n / 8] & (1 << (7 - n % 8))) >> (7 - n % 8)

class BloomFilter {
  public:
  BloomFilter();
  BloomFilter(unsigned _num); //num of all keys
  void addRecord(int _record);
  //NOTICE:we hope a Bstr-like struct here, for the length maybe very large
  void addRecord(const char* _record, unsigned _len);
  bool checkRecord(int _record) const;
  bool checkRecord(const char* _record, unsigned _len) const;
  ~BloomFilter();

  private:
  unsigned length;     //length of total bits, mod 8 == 0
  char* filter;        //the bit space
  unsigned hfnum;      //num of hash functions
  double rate;         //false positive
  HashFunction* hfptr; //hash functions pointer array

  void init();
};

#endif //_UTIL_BLOOMFILTER_H
