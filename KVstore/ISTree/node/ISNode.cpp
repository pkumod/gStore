/*=============================================================================
# Filename: ISNode.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:39
# Description: achieve functions in ISNode.h
=============================================================================*/

#include "ISNode.h"

using namespace std;

void
ISNode::AllocKeys()
{
  keys = new int[MAX_KEY_NUM];
}

/*
void
ISNode::FreeKeys()
{
delete[] keys;
}
*/

ISNode::ISNode()
{
  store = flag = 0;
  flag |= NF_IM;
  heapId = -1;
  AllocKeys();
}

ISNode::ISNode(bool isVirtual)
{
  store = flag = 0;
  heapId = -1;
  if (!isVirtual) {
    flag |= NF_IM;
    AllocKeys();
  }
}

/*
ISNode::Node(Storage* TSM)
{
AllocKeys();
TSM->readISNode(this, Storage::OVER);
}
*/
bool
ISNode::isLeaf() const
{
  return this->flag & NF_IL;
}

bool
ISNode::isDirty() const
{
  return this->flag & NF_ID;
}

void
ISNode::setDirty()
{
  this->flag |= NF_ID;
}

void
ISNode::delDirty()
{
  this->flag &= ~NF_ID;
}

bool
ISNode::inMem() const
{
  return this->flag & NF_IM;
}

void
ISNode::setMem()
{
  this->flag |= NF_IM;
}

void
ISNode::delMem()
{
  this->flag &= ~NF_IM;
}

/*
bool
ISNode::isVirtual() const
{
return this->flag & NF_IV;
}

void
ISNode::setVirtual()
{
this->flag |= NF_IV;
}

void
ISNode::delVirtual()
{
this->flag &= ~NF_IV;
}
*/

unsigned
ISNode::getRank() const
{
  return this->flag & NF_RK;
}

void
ISNode::setRank(unsigned _rank)
{
  this->flag &= ~NF_RK;
  this->flag |= _rank;
}

unsigned
ISNode::getHeight() const
{
  return (this->flag & NF_HT) >> 20;
}

void
ISNode::setHeight(unsigned _h)
{
  this->flag &= ~NF_HT;
  this->flag |= (_h << 20);
}

unsigned
ISNode::getNum() const
{
  return (this->flag & NF_KN) >> 12;
}

bool
ISNode::setNum(int _num)
{
  if (_num < 0 || (unsigned)_num > MAX_KEY_NUM) {
    print(string("error in setNum: Invalid num ") + Util::int2string(_num));
    return false;
  }
  this->flag &= ~NF_KN;
  this->flag |= (_num << 12);
  return true;
}

bool
ISNode::addNum()
{
  if (this->getNum() + 1 > MAX_KEY_NUM) {
    print("error in addNum: Invalid!");
    return false;
  }
  this->flag += (1 << 12);
  return true;
}

bool
ISNode::subNum()
{
  if (this->getNum() < 1) {
    print("error in subNum: Invalid!");
    return false;
  }
  this->flag -= (1 << 12);
  return true;
}

unsigned
ISNode::getStore() const
{
  return this->store;
}

void
ISNode::setStore(unsigned _store)
{
  this->store = _store;
}

unsigned
ISNode::getFlag() const
{
  return flag;
}

void
ISNode::setFlag(unsigned _flag)
{
  this->flag = _flag;
}

unsigned
ISNode::getKey(int _index) const
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    //print(string("error in getKey: Invalid index ") + Util::int2string(_index));
    printf("error in getKey: Invalid index\n");
    //return -1;
    return INVALID;
  } else
    return this->keys[_index];
}

bool
ISNode::setKey(unsigned _key, int _index)
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    print(string("error in setKey: Invalid index ") + Util::int2string(_index));
    return false;
  }
  keys[_index] = _key;
  return true;
}

bool
ISNode::addKey(unsigned _key, int _index)
{
  int num = this->getNum();
  if (_index < 0 || _index > num) {
    print(string("error in addKey: Invalid index ") + Util::int2string(_index));
    return false;
  }
  int i;
  //NOTICE: if num == MAX_KEY_NUM, will visit keys[MAX_KEY_NUM], not legal!!!
  //however. tree operations ensure that: when node is full, not add but split first!
  for (i = num - 1; i >= _index; --i)
    keys[i + 1] = keys[i];
  keys[_index] = _key;
  return true;
}

bool
ISNode::subKey(int _index)
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    print(string("error in subKey: Invalid index ") + Util::int2string(_index));
    return false;
  }
  int i;
  for (i = _index; i < num - 1; ++i)
    keys[i] = keys[i + 1];
  return true;
}

int
ISNode::searchKey_less(unsigned _key) const
{
  int num = this->getNum();
  //for(i = 0; i < num; ++i)
  //if(bstr < *(p->getKey(i)))
  //break;

  int low = 0, high = num - 1, mid = -1;
  while (low <= high) {
    mid = (low + high) / 2;
    if (this->keys[mid] > _key) {
      if (low == mid)
        break;
      high = mid;
    } else {
      low = mid + 1;
    }
  }
  return low;
}

int
ISNode::searchKey_equal(unsigned _key) const
{
  int num = this->getNum();
  //for(i = 0; i < num; ++i)
  //	if(bstr == *(p->getKey(i)))
  //	{

  int ret = this->searchKey_less(_key);
  if (ret > 0 && this->keys[ret - 1] == _key)
    return ret - 1;
  else
    return num;
}

int
ISNode::searchKey_lessEqual(unsigned _key) const
{
  //int num = this->getNum();
  //for(i = 0; i < num; ++i)
  //if(bstr <= *(p->getKey(i)))
  //break;

  int ret = this->searchKey_less(_key);
  if (ret > 0 && this->keys[ret - 1] == _key)
    return ret - 1;
  else
    return ret;
}
