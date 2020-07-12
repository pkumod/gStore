/*=============================================================================
# Filename: Heap.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:37
# Description: achieve functions in Heap.h
=============================================================================*/

#include "Heap.h"

using namespace std;

Heap::Heap()
{
  this->length = this->size = 0;
  this->heap = NULL;
}

Heap::Heap(unsigned _size)
{
  this->length = 0;
  this->size = _size;
  //this->heap = (Node**)malloc(this->size * sizeof(Node*));	//not use 4 or 8
  this->heap = new Node* [this->size];
  if (this->heap == NULL) {
    this->print("error in Heap: Allocation fail!");
    exit(1);
  }
  /*
	this->npmap = (Map*)malloc(this->size * sizeof(struct Map));
	if(this->npmap == NULL)
	{
		this->print("error in Heap: Allocation fail!");
		exit(1);
	}
	*/
}

Node*
Heap::getTop() const
{
  if (this->length > 0)
    return this->heap[0];
  else
    return NULL;
}

unsigned
Heap::getLen() const
{
  return this->length;
}

unsigned
Heap::getSize() const
{
  return this->size;
}

bool
Heap::isEmpty() const
{
  return this->length == 0;
}

bool
Heap::insert(Node* _np)
{
  if (this->length == this->size) //when full, reallocate
  {
    this->heap = (Node**)realloc(this->heap, 2 * this->size * sizeof(Node*));
    if (this->heap == NULL) {
      print("error in isert: Reallocation fail!");
      return false;
    }
    /*
		this->npmap = (struct Map*)realloc(this->npmap, 2 * this->size * sizeof(struct Map));
		if(this->npmap == NULL)
		{
			print("error in insert: Reallocation fail!");
			return false;
		}
		*/
    this->size = 2 * this->size;
  }
  unsigned i = this->length, j;
  while (i != 0) {
    j = (i - 1) / 2;
    if (_np->getRank() >= this->heap[j]->getRank())
      break;
    heap[i] = heap[j];
    //this->npmap[k].pos = i;			//adjust the position
    i = j;
  }
  this->heap[i] = _np;
  this->length++;
  return true;
}

bool
Heap::remove()
{
  if (this->length == 0) {
    print("error in remove: remove from empty heap!");
    return false;
  }
  //Node* tp = this->heap[0];
  this->length--;
  if (this->length == 0)
    return true;
  Node* xp = this->heap[this->length];
  unsigned i = 0, j = 1;
  while (j < this->length) {
    if (j < this->length - 1 && this->heap[j]->getRank() > this->heap[j + 1]->getRank())
      j++;
    if (xp->getRank() <= this->heap[j]->getRank())
      break;
    this->heap[i] = this->heap[j];
    i = j;
    j = 2 * i + 1;
  }
  this->heap[i] = xp;
  return true;
}

bool
Heap::modify(Node* _np, bool _flag) //control direction
{
  //search and adjust
  unsigned i, j;
  for (i = 0; i < this->length; ++i)
    if (this->heap[i] == _np)
      break;
  if (_flag == true) //move up
  {
    while (i != 0) {
      j = (i - 1) / 2;
      if (_np->getRank() < heap[j]->getRank()) {
        heap[i] = heap[j];
        heap[j] = _np;
        i = j;
      } else
        break;
    }
  } else //move down
  {
    j = 2 * i + 1;
    while (j < this->length) {
      if (j < this->length - 1 && heap[j]->getRank() > heap[j + 1]->getRank())
        j++;
      if (heap[j]->getRank() < _np->getRank()) {
        heap[i] = heap[j];
        heap[j] = _np;
        i = j;
      } else
        break;
    }
  }
  return true;
}

Heap::~Heap()
{
  delete[] this->heap;
  this->heap = NULL;
  this->length = this->size = 0;
}

void
Heap::print(string s)
{
#ifdef DEBUG_KVSTORE
#endif
}
