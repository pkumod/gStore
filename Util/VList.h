/*=============================================================================
# Filename: VList.h
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2017-03-27 15:40
# Description: 
=============================================================================*/

#ifndef _UTIL_VLIST_H
#define _UTIL_VLIST_H

#include "Util.h"
#include "Bstr.h"
#include "ClassForVlistCache.h"

//NOTICE: not keep long list in memory, read each time
//but when can you free the long list(kvstore should release it after parsing)
//
//CONSIDER: if to keep long list in memory, should adjust the bstr in memory:
//unsigned: 0  char*: an object (if in memory, if modified, length, content, block num)
//when reading a long list in a node, generate the object first, and the object will tell you whether
//the list is in mmeory or not

//BETTER: use two kind of blocks in two files, like 1M and 1G (split the block num into two parts)

//STRUCT: a long list    real-address is the block ID in file2(only for long value lists, a list across several 1M blocks)
//tree-value Bstr: unsigned=the real address   char*=NULL
//in disk:
//file1 is tree file, the long list is represented as: 0 real-address
//NOTICE: long list is not kept in mmeory for cache, it is read/update each time on need!

//TODO: use fread/fwrite here instead of fgetc/fputc
//including  other trees

typedef char* CACHE_VALUE;
typedef std::map<unsigned, CACHE_VALUE> CACHE_TYPE;
typedef CACHE_TYPE::iterator CACHE_ITERATOR;

class VList {
  public:
  //NOTICE:the border is 10^6, but the block is larger, 1M
  //this is not choosed intuitively, we make sure that using vlist is better: transferring time>random seek time(x/40M>0.006)
  //Also notice that if no modification on data, read a node is almost sequentially in normal IVTree
  //In VList, case is the same and using VList may bring another seek cost!(it is not easy to setup cache for IVTree due to data struct)
  static const unsigned LENGTH_BORDER = 1000000;
  //static const unsigned LENGTH_BORDER = 100;
  static const unsigned BLOCK_SIZE = 1 << 20;    //fixed size of disk-block
  static const unsigned MAX_BLOCK_NUM = 1 << 23; //max block-num
  //below two constants: must can be exactly divided by 8
  static const unsigned SET_BLOCK_NUM = 1 << 3;        //initial blocks num
  static const unsigned SET_BLOCK_INC = SET_BLOCK_NUM; //base of blocks-num inc
  static const unsigned SuperNum = MAX_BLOCK_NUM / (8 * BLOCK_SIZE) + 1;

  private:
  //NOTICE: we need to set a buffer for vlist
  static const unsigned CACHE_LIMIT = 1 << 26; //not cache too long list
  //TODO+BETTER: get this paramemter by MemoryManager
  static const unsigned CACHE_CAPACITY = UINT_MAX;
  //BETTER+TODO: swap the buffer in and out according to access frequence
  //Here we simply add and give a warnning if cache overflow, but not swap/lock
  //TODO: swap if full, check if one is locked(being used by some query)
  std::map<unsigned, char*> vlist_cache;
  unsigned vlist_cache_left; //size of cache left
  //QUERY: maybe use array isntead of map will bAe better - char*[NULL]
  //NOTICE: check if the cache consumes too much memory.
  //In addition, for different trees, maybe different size of caches should be used, i.e. p2values can have longer list!

  unsigned long long max_buffer_size;
  unsigned cur_block_num;
  std::string filepath;
  BlockInfo* freelist;
  //very long value list are stored in a separate file(with large block)
  //
  //NOTICE: according to the summary result, 90% value lists are just below 100 bytes
  //<10%: 5000000~100M bytes
  FILE* valfp;
  // cache for vlist.
  Longlist_inMem longlist[2000];
  //NOTICE: freemem's type is long long here, due to large memory in server.
  //However, needmem in handler() and request() is ok to be int/unsigned.
  //Because the bstr' size is controlled, so is the node.
  unsigned long long freemem; //free memory to use, non-negative
  //unsigned long long time;			//QUERY(achieving an old-swap startegy?)
  long Address(unsigned _blocknum) const;
  unsigned Blocknum(long address) const;
  unsigned AllocBlock();
  void FreeBlock(unsigned _blocknum);
  void ReadAlign(unsigned* _next);
  void WriteAlign(unsigned* _next);
  bool readBstr(char*& _bp, unsigned& _len, unsigned* _next);
  bool writeBstr(const char* _str, unsigned _len, unsigned* _curnum);

  public:
  VList();
  VList(std::string& _filepath, std::string& _mode, unsigned long long _buffer_size); //create a fixed-size file or open an existence
  bool readValue(unsigned _block_num, char*& _str, unsigned& _len, unsigned _key);
  unsigned writeValue(char* _str, unsigned _len);
  bool removeValue(unsigned _block_num, unsigned _key);
  ~VList();

  void release_cache();
  static bool isLongList(unsigned _len);
  static bool listNeedDelete(unsigned _len);
  void AddIntoCache(unsigned _key, char*& _str, unsigned _len);
};

#endif
