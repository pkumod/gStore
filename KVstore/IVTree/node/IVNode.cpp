/*=============================================================================
# Filename: IVNode.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:39
# Description: achieve functions in IVNode.h
=============================================================================*/

#include "IVNode.h"

using namespace std;

void
IVNode::AllocKeys()
{
  keys = new unsigned[MAX_KEY_NUM];
}

/*
void
IVNode::FreeKeys()
{
delete[] keys;
}
*/

IVNode::IVNode()
{
  store = flag = 0;
  flag |= NF_IM;
  AllocKeys();
}

IVNode::IVNode(bool isVirtual)
{
  store = flag = 0;
  if (!isVirtual) {
    flag |= NF_IM;
    AllocKeys();
  }
}

/*
IVNode::Node(Storage* TSM)
{
AllocKeys();
TSM->readIVNode(this, Storage::OVER);
}
*/
bool
IVNode::isLeaf() const
{
  return this->flag & NF_IL;
}

bool
IVNode::isDirty() const
{
  return this->flag & NF_ID;
}

void
IVNode::setDirty()
{
  this->flag |= NF_ID;
}

void
IVNode::delDirty()
{
  this->flag &= ~NF_ID;
}

bool
IVNode::inMem() const
{
  return this->flag & NF_IM;
}

void
IVNode::setMem()
{
  this->flag |= NF_IM;
}

void
IVNode::delMem()
{
  this->flag &= ~NF_IM;
}

/*
bool
IVNode::isVirtual() const
{
return this->flag & NF_IV;
}

void
IVNode::setVirtual()
{
this->flag |= NF_IV;
}

void
IVNode::delVirtual()
{
this->flag &= ~NF_IV;
}
*/

unsigned
IVNode::getRank() const
{
  return this->flag & NF_RK;
}

void
IVNode::setRank(unsigned _rank)
{
  this->flag &= ~NF_RK;
  this->flag |= _rank;
}

unsigned
IVNode::getHeight() const
{
  return (this->flag & NF_HT) >> 20;
}

void
IVNode::setHeight(unsigned _h)
{
  this->flag &= ~NF_HT;
  this->flag |= (_h << 20);
}

unsigned
IVNode::getNum() const
{
  return (this->flag & NF_KN) >> 12;
}

bool
IVNode::setNum(int _num)
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
IVNode::addNum()
{
  if (this->getNum() + 1 > MAX_KEY_NUM) {
    print("error in addNum: Invalid!");
    return false;
  }
  this->flag += (1 << 12);
  return true;
}

bool
IVNode::subNum()
{
  if (this->getNum() < 1) {
    print("error in subNum: Invalid!");
    return false;
  }
  this->flag -= (1 << 12);
  return true;
}

unsigned
IVNode::getStore() const
{
  return this->store;
}

void
IVNode::setStore(unsigned _store)
{
  this->store = _store;
}

unsigned
IVNode::getFlag() const
{
  return flag;
}

void
IVNode::setFlag(unsigned _flag)
{
  this->flag = _flag;
}

unsigned
IVNode::getKey(int _index) const
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    //print(string("error in getKey: Invalid index ") + Util::int2string(_index));
    printf("error in getKey: Invalid index\n");
    return -1;
  } else
    return this->keys[_index];
}

bool
IVNode::setKey(unsigned _key, int _index)
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
IVNode::addKey(unsigned _key, int _index)
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
IVNode::subKey(int _index)
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
IVNode::searchKey_less(unsigned _key) const
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
IVNode::searchKey_equal(unsigned _key) const
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
IVNode::searchKey_lessEqual(unsigned _key) const
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
