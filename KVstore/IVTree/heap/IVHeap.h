/*=============================================================================
# Filename: IVHeap.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:37
# Description: set and deal of IVNode*s in memory
=============================================================================*/

#ifndef _KVSTORE_IVTREE_HEAP_IVHEAP_H
#define _KVSTORE_IVTREE_HEAP_IVHEAP_H

#include "../../../Util/Util.h"
#include "../node/IVNode.h"

/* add, sub, modify: all can be done within O(logn) using adjust-function */
//QUERY: when modified, finding right position consumes O(n). How about keeping smallest?
//(add O(1), sub O(2n), modify O(n)
//TODO: to solve this probem, use another hash: (pointer, pos), to find the right position of
//given p in O(lgn) time

class IVHeap {
  private:
  IVNode** heap;   //dynamic array
  unsigned length; //valid elements num
  unsigned size;   //max-size of heap
  public:
  IVHeap();
  IVHeap(unsigned _size);
  IVNode* getTop() const; //return the top element
  unsigned getLen() const;
  unsigned getSize() const;
  bool isEmpty() const;
  bool insert(IVNode* _np);             //insert and adjust
  bool remove();                        //remove top and adjust
  bool modify(IVNode* _np, bool _flag); //searech modified element and adjust
  ~IVHeap();
  void print(std::string s); //DEBUG
};

#endif
