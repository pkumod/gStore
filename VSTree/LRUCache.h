/*=============================================================================
# Filename: LRUCache.h
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2016-04-11 14:05
# Description: written by hanshuo
=============================================================================*/

#ifndef _VSTREE_LRUCACHE_H
#define _VSTREE_LRUCACHE_H

#include "../Util/Util.h"

class VNode;

//NOTICE: we should implement the LRU, not simply FIFO
//not only consider updates, but also visits
//TODO:this may cause the cost of mutiple-thread-sync very high

// before using the cache, you must loadCache or createCache.
class LRUCache {
  public:
  static int DEFAULT_CAPACITY;

  LRUCache(int _capacity = -1);
  ~LRUCache();
  //load cache's elements from an exist data file.
  bool loadCache(std::string _filePath = "./tree_file");
  //create a new empty data file, the original one will be overwrite.
  bool createCache(std::string _filePath = "./tree_file");
  //get the value(node's pointer) by key(node's file line).
  VNode* get(int _key);
  //set the key(node's file line) and value(node's pointer). if the key exists now, the value of this key will be overwritten.
  bool set(int _key, VNode* _value);
  //delete a node from LRUcache and file
  bool del(int _key);
  //update the _key's mapping _value. if the key do not exist, this operation will fail and return false.
  bool update(int _key, VNode* _value);
  //write out all the elements to hard disk.
  bool flush();
  int getCapacity();
  int getRestAmount();
  void showAmount();
  bool isFull();

  private:
  int capacity;
  int size;
  int* next;
  int* prev;
  //NOTICE:pos is the index of keys and values, while file_line(key) is the ID of VNode,
  //as well as the file part number
  //(pos is needed due to swap-in-out and insert/delete)
  int* keys;
  VNode** values;
  //NOTICE: the key is node file line, i.e. vstree node ID
  std::map<int, int> key2pos; // mapping from key to pos.
  std::string dataFilePath;
  static const int DEFAULT_NUM = 2;
  static const int START_INDEX = 0;
  static const int END_INDEX = 1;
  static const int NULL_INDEX = -1;
  static const int EOF_FLAG = -1;
  //put the new visited one to the tail
  void refresh(int _pos);
  //free the memory of the _pos element in cache.
  void freeElem(int _pos);
  //set the memory of the _pos element in cache
  void setElem(int _pos, int _key, VNode* _value);
  //move pos2 ele to pos1, and pos1 ele should be freed
  void fillElem(int _pos1, int _pos2);
  bool freeDisk(int _pos);
  //just write the values[_pos] to the hard disk, the VNode in memory will not be free.
  bool writeOut(int _pos, int _fileLine = -1);
  //read the value from hard disk, and put it to the values[_pos].
  //before use it, you must make sure that the _pos element in cache is free(unoccupied).
  bool readIn(int _pos, int _fileLine);

//NOTICE: cost of rw lock is higher than mutex
//By default, read lock is recursive, write lock not, we can set mutex as recursive
//You can acquire read lock first and then upgrade to write lock, finally unlock twice
//R/W lock only fits for many-read and rare-write cases
//
//lock the whole buffer if get/set/swap element
#ifdef THREAD_VSTREE_ON
  pthread_rwlock_t cache_lock;
//TODO:if find no unlocked one to swap out, then need to wait by cond
//pthread_cond_t cache_cond;
#endif
};

#endif //_VSTREE_LRUCACHE_H
