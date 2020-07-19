/*=============================================================================
# Filename: ISHeap.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:37
# Description: achieve functions in ISHeap.h
=============================================================================*/

#include "ISHeap.h"

using namespace std;

ISHeap::ISHeap()
{
  this->length = this->size = 0;
  this->heap = NULL;
}

ISHeap::ISHeap(unsigned _size)
{
  this->length = 0;
  this->size = _size;
  this->heap = (ISNode**)malloc(this->size * sizeof(ISNode*)); //not use 4 or 8
  //this->heap = new ISNode*[this->size];
  if (this->heap == NULL) {
    this->print("error in ISHeap: Allocation fail!");
    exit(1);
  }
  /*
	this->npmap = (Map*)malloc(this->size * sizeof(struct Map));
	if(this->npmap == NULL)
	{
	this->print("error in ISHeap: Allocation fail!");
	exit(1);
	}
	*/
}

ISNode*
ISHeap::getTop() const
{
  if (this->length > 0)
    return this->heap[0];
  else
    return NULL;
}

unsigned
ISHeap::getLen() const
{
  return this->length;
}

unsigned
ISHeap::getSize() const
{
  return this->size;
}

bool
ISHeap::isEmpty() const
{
  return this->length == 0;
}

bool
ISHeap::insert(ISNode* _np)
{
  if (this->length == this->size) //when full, reallocate
  {
    cout << "check: double the heap" << endl;
    //WARN: realloc should be combined with malloc instead of new
    //http://bbs.csdn.net/topics/320148799
    //For new, use placement new to enlarge space: http://blog.csdn.net/vangoals/article/details/4252833
    this->heap = (ISNode**)realloc(this->heap, 2 * this->size * sizeof(ISNode*));
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
ISHeap::remove()
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
  ISNode* xp = this->heap[this->length];
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
ISHeap::modify(ISNode* _np, bool _flag) //control direction
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

ISHeap::~ISHeap()
{
  //delete[] this->heap;
  free(this->heap);
  this->heap = NULL;
  this->length = this->size = 0;
}

void
ISHeap::print(string s)
{
#ifdef DEBUG_KVSTORE
#endif
}
