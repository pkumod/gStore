/*=============================================================================
# Filename: SIHeap.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:37
# Description: set and deal of SINode*s in memory
=============================================================================*/

#ifndef _KVSTORE_SITREE_HEAP_SIHEAP_H
#define _KVSTORE_SITREE_HEAP_SIHEAP_H

#include "../../../Util/Util.h"
#include "../node/SINode.h"

/**
 * add, sub, modify: all can be done within O(log n) using adjust-function
 * using an array to implement a heap.
 * Structure: heap[i] is the parent of heap[2i+1] and heap[2i+2], heap[0] is the top
 * heap[i]'s rank <= (heap[2i+1] and heap[2i+2])'s rank
 * traverse this heap, we can get SINode* by ranks descending
 */
class SIHeap
{
private:
  //dynamic array
  SINode** heap;
  //valid elements num
  unsigned length;
  //max-size of heap
  unsigned size;
 public:
  SIHeap();
  SIHeap(unsigned _size);
  SINode* getTop() const;			//return the top element
  unsigned getLen() const;
  unsigned getSize() const;
  bool isEmpty() const;
  bool Insert(SINode* _np);	//insert and adjust
  bool RemoveTop();			//remove top and adjust
  bool modify(SINode* _np, bool _flag);			//searech modified element and adjust
  ~SIHeap();
  void print(std::string s);			//DEBUG
};

#endif

