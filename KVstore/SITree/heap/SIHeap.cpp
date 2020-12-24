/*=============================================================================
# Filename: SIHeap.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:37
# Description: achieve functions in SIHeap.h
=============================================================================*/

#include "SIHeap.h"

using namespace std;

SIHeap::SIHeap()
{
  this->length = this->size = 0;
  this->heap = NULL;
}

SIHeap::SIHeap(unsigned _size)
{
  this->length = 0;
  this->size = _size;
  this->heap = (SINode**)malloc(this->size * sizeof(SINode*)); //not use 4 or 8
  //this->heap = new SINode*[this->size];
  if (this->heap == NULL) {
    this->print("error in SIHeap: Allocation fail!");
    exit(1);
  }
  /*
	this->npmap = (Map*)malloc(this->size * sizeof(struct Map));
	if(this->npmap == NULL)
	{
	this->print("error in SIHeap: Allocation fail!");
	exit(1);
	}
	*/
}

SINode*
SIHeap::getTop() const
{
  if (this->length > 0)
    return this->heap[0];
  else
    return NULL;
}

unsigned
SIHeap::getLen() const
{
  return this->length;
}

unsigned
SIHeap::getSize() const
{
  return this->size;
}

bool
SIHeap::isEmpty() const
{
  return this->length == 0;
}

bool
SIHeap::insert(SINode* _np)
{
  if (this->length == this->size) //when full, reallocate
  {
    cout << "check: double the heap" << endl;
    this->heap = (SINode**)realloc(this->heap, 2 * this->size * sizeof(SINode*));
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
    heap[i]->heapId = i;
    //this->npmap[k].pos = i;			//adjust the position
    i = j;
  }
  this->heap[i] = _np;
  _np->heapId = i;
  this->length++;
  return true;
}

bool
SIHeap::remove()
{
  if (this->length == 0) {
    print("error in remove: remove from empty heap!");
    return false;
  }
  //Node* tp = this->heap[0];
  this->length--;
  this->heap[0]->heapId = -1;
  if (this->length == 0)
    return true;
  SINode* xp = this->heap[this->length];
  unsigned i = 0, j = 1;
  while (j < this->length) {
    if (j < this->length - 1 && this->heap[j]->getRank() > this->heap[j + 1]->getRank())
      j++;
    if (xp->getRank() <= this->heap[j]->getRank())
      break;
    this->heap[i] = this->heap[j];
    this->heap[i]->heapId = i;
    i = j;
    j = 2 * i + 1;
  }
  this->heap[i] = xp;
  this->heap[i]->heapId = i;
  return true;
}

bool
SIHeap::modify(SINode* _np, bool _flag) //control direction
{
  //search and adjust
  unsigned i, j;
  i = _np->heapId;
  // for (i = 0; i < this->length; ++i)
  // if (this->heap[i] == _np)
  // break;
  if (_flag == true) //move up
  {
    while (i != 0) {
      j = (i - 1) / 2;
      if (_np->getRank() < heap[j]->getRank()) {
        heap[i] = heap[j];
        heap[i]->heapId = i;
        heap[j] = _np;
        heap[j]->heapId = j;
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
        heap[i]->heapId = i;
        heap[j] = _np;
        heap[j]->heapId = j;
        i = j;
      } else
        break;
    }
  }
  return true;
}

SIHeap::~SIHeap()
{
  //delete[] this->heap;
  free(this->heap);
  this->heap = NULL;
  this->length = this->size = 0;
}

void
SIHeap::print(string s)
{
#ifdef DEBUG_KVSTORE
#endif
}
