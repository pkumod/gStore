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
  this->heap = (SINode**)malloc(this->size * sizeof(SINode*));	//not use 4 or 8
  if (this->heap == NULL)
  {
    this->print("error in SIHeap: Allocation fail!");
    exit(1);
  }
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

/**
 * insert a node into SIHeap. Like the implement of std::vector,
 * if heap size exceed the capacity, double the size.
 * @param _np the inserted node
 */
bool
SIHeap::Insert(SINode* _np)
{
  if (this->length == this->size)	//when full, reallocate
  {
    SLOG_CORE("check: double the heap");
    this->heap = (SINode**)realloc(this->heap, 2 * this->size * sizeof(SINode*));
    if (this->heap == NULL)
    {
      print("error in isert: Reallocation fail!");
      return false;
    }
    this->size = 2 * this->size;
  }
  // the inserted position
  unsigned int i = this->length;

  // the father of inserted position
  unsigned j;

  // find a position where parent's rank <= np's rank
  while (i != 0)
  {
    j = (i - 1) / 2;
    if (_np->getRank() >= this->heap[j]->getRank())
      break;
    heap[i] = heap[j];
    heap[i]->heapId = i;
    i = j;
  }
  this->heap[i] = _np;
  _np->heapId = i;
  this->length++;
  return true;
}

/**
 * RemoveTop the top element, and adjust the most right leaf node's position.
 * From top to bottom, adjust nodes' position, and place the most right leaf node
 * after find a position where its rank >= the most right leaf node.
 * @return if remove from an empty tree, return false
 */
bool
SIHeap::RemoveTop()
{
  if (this->length == 0)
  {
    print("error in remove: remove from empty heap!");
    return false;
  }
  this->length--;
  this->heap[0]->heapId = -1;
  if (this->length == 0)
    return true;
  SINode* xp = this->heap[this->length];

  // the most right leaf node's NEW position
  unsigned int i = 0;
  // the left child of i
  unsigned int j = 1;

  while (j < this->length)
  {
    // if right child has lower rank, choose the right child
    if (j < this->length - 1 && this->heap[j]->getRank() > this->heap[j + 1]->getRank())
      j++;
    // if xp cannot go deeper, stop
    if (xp->getRank() <= this->heap[j]->getRank())
      break;
    // move j to its parent's position
    this->heap[i] = this->heap[j];
    this->heap[i]->heapId = i;
    // going down
    i = j;
    j = 2 * i + 1;
  }
  this->heap[i] = xp;
  this->heap[i]->heapId = i;
  return true;
}

/**
 * adjust _np position
 * @param _np the node to adjust
 * @param move_up move _np up
 */
bool
SIHeap::modify(SINode* _np, bool _flag)
{
  //Search and adjust
  if (_np->heapId < 0)
  {
    return false;
  }
  unsigned i, j;
  i = _np->heapId;
  if (_flag)	//move up
  {
    while (i != 0)
    {
      j = (i - 1) / 2;
      if (_np->getRank() < heap[j]->getRank())
      {
        heap[i] = heap[j];
        heap[i]->heapId = i;
        heap[j] = _np;
        heap[j]->heapId = j;
        i = j;
      }
      else
        break;
    }
  }
  else				//move down
  {
    j = 2 * i + 1;
    while (j < this->length)
    {
      if (j < this->length - 1 && heap[j]->getRank() > heap[j + 1]->getRank())
        j++;
      if (heap[j]->getRank() < _np->getRank())
      {
        heap[i] = heap[j];
        heap[i]->heapId = i;
        heap[j] = _np;
        heap[j]->heapId = j;
        i = j;
      }
      else
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

