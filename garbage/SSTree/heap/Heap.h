/*=============================================================================
# Filename: Heap.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:37
# Description: set and deal of Node*s in memory
=============================================================================*/

#ifndef _KVSTORE_HEAP_HEAP_H
#define _KVSTORE_HEAP_HEAP_H

#include "../../../Util/Util.h"
#include "../node/Node.h"

/* add, sub, modify: all can be done within O(logn) using adjust-function */
//QUERY: when modified, finding right position consumes O(n). How about keeping smallest?
//(add O(1), sub O(2n), modify O(n)
//TODO: to solve this probem, use another hash: (pointer, pos), to find the right position of
//given p in O(lgn) time

class Heap {
  private:
  Node** heap;     //dynamic array
  unsigned length; //valid elements num
  unsigned size;   //max-size of heap
  public:
  Heap();
  Heap(unsigned _size);
  Node* getTop() const; //return the top element
  unsigned getLen() const;
  unsigned getSize() const;
  bool isEmpty() const;
  bool insert(Node* _np);             //insert and adjust
  bool remove();                      //remove top and adjust
  bool modify(Node* _np, bool _flag); //searech modified element and adjust
  ~Heap();
  void print(std::string s); //DEBUG
};

#endif
