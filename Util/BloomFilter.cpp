/*=============================================================================
# Filename: BloomFilter.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2016-03-04 17:49
# Description: implement functions in BloomFilter.h
=============================================================================*/

#include "BloomFilter.h"

BloomFilter::BloomFilter()
{
  //TODO
}

BloomFilter::BloomFilter(unsigned _num)
{
  //TODO:fix _num to mod 8 == 0
}

void
BloomFilter::init()
{
  this->filter = (char*)calloc(this->length / 8, sizeof(char));
  //TODO:assign hash functions for hfptr
  //assign the rate of false positive, and then compute the length and hfnum according to key num
}

BloomFilter::~BloomFilter()
{
  //TODO
}

//NOTICE:there are two ways to change int to string, one digit to one character or just change int* to char*
//The latter is more efficient because the former consumes space and time:O(32) >= O(lgn)
void
BloomFilter::addRecord(int _record)
{
  //TODO
}

void
BloomFilter::addRecord(const char* _record, unsigned _len)
{
  //TODO
}

bool
BloomFilter::checkRecord(int _record) const
{
  //TODO
  return false;
}

bool
BloomFilter::checkRecord(const char* _record, unsigned _len) const
{
  //TODO
  return false;
}

//if( GETBIT(vector, Util::HFLPHash(ch,strlen(ch))%MAX) )
//{
//flag++;
//}
//else
//{
//SETBIT(vector,Util::HFLPHash(ch,strlen(ch))%MAX );
//}
