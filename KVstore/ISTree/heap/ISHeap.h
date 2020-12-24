/*=============================================================================
# Filename: ISHeap.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:37
# Description: set and deal of ISNode*s in memory
=============================================================================*/

#ifndef _KVSTORE_ISTREE_HEAP_ISHEAP_H
#define _KVSTORE_ISTREE_HEAP_ISHEAP_H

#include "../../../Util/Util.h"
#include "../node/ISNode.h"

/* add, sub, modify: all can be done within O(logn) using adjust-function */
//QUERY: when modified, finding right position consumes O(n). How about keeping smallest?
//(add O(1), sub O(2n), modify O(n)
//TODO: to solve this probem, use another hash: (pointer, pos), to find the right position of
//given p in O(lgn) time

class ISHeap {
  private:
  ISNode** heap;   //dynamic array
  unsigned length; //valid elements num
  unsigned size;   //max-size of heap
  public:
  ISHeap();
  ISHeap(unsigned _size);
  ISNode* getTop() const; //return the top element
  unsigned getLen() const;
  unsigned getSize() const;
  bool isEmpty() const;
  bool insert(ISNode* _np);             //insert and adjust
  bool remove();                        //remove top and adjust
  bool modify(ISNode* _np, bool _flag); //searech modified element and adjust
  ~ISHeap();
  void print(std::string s); //DEBUG
};

#endif
