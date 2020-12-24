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

/* add, sub, modify: all can be done within O(logn) using adjust-function */
//QUERY: when modified, finding right position consumes O(n). How about keeping smallest?
//(add O(1), sub O(2n), modify O(n)
//TODO: to solve this probem, use another hash: (pointer, pos), to find the right position of
//given p in O(lgn) time

class SIHeap {
  private:
  SINode** heap;   //dynamic array
  unsigned length; //valid elements num
  unsigned size;   //max-size of heap
  public:
  SIHeap();
  SIHeap(unsigned _size);
  SINode* getTop() const; //return the top element
  unsigned getLen() const;
  unsigned getSize() const;
  bool isEmpty() const;
  bool insert(SINode* _np);             //insert and adjust
  bool remove();                        //remove top and adjust
  bool modify(SINode* _np, bool _flag); //searech modified element and adjust
  ~SIHeap();
  void print(std::string s); //DEBUG
};

#endif
