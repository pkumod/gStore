/*=============================================================================
# Filename: Node.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:39
# Description: achieve functions in Node.h
=============================================================================*/

#include "Node.h"

using namespace std;

void
Node::AllocKeys()
{
  keys = new Bstr[MAX_KEY_NUM];
}

/*
void
Node::FreeKeys()
{
	delete[] keys;
}
*/

Node::Node()
{
  store = flag = 0;
  flag |= NF_IM;
  AllocKeys();
}

Node::Node(bool isVirtual)
{
  store = flag = 0;
  if (!isVirtual) {
    flag |= NF_IM;
    AllocKeys();
  }
}

/*
Node::Node(Storage* TSM)
{
	AllocKeys();
	TSM->readNode(this, Storage::OVER);	
}
*/
bool
Node::isLeaf() const
{
  return this->flag & NF_IL;
}

bool
Node::isDirty() const
{
  return this->flag & NF_ID;
}

void
Node::setDirty()
{
  this->flag |= NF_ID;
}

void
Node::delDirty()
{
  this->flag &= ~NF_ID;
}

bool
Node::inMem() const
{
  return this->flag & NF_IM;
}

void
Node::setMem()
{
  this->flag |= NF_IM;
}

void
Node::delMem()
{
  this->flag &= ~NF_IM;
}

/*
bool
Node::isVirtual() const
{
	return this->flag & NF_IV;
}

void
Node::setVirtual()
{
	this->flag |= NF_IV;
}

void
Node::delVirtual()
{
	this->flag &= ~NF_IV;
}
*/

unsigned
Node::getRank() const
{
  return this->flag & NF_RK;
}

void
Node::setRank(unsigned _rank)
{
  this->flag &= ~NF_RK;
  this->flag |= _rank;
}

unsigned
Node::getHeight() const
{
  return (this->flag & NF_HT) >> 20;
}

void
Node::setHeight(unsigned _h)
{
  this->flag &= ~NF_HT;
  this->flag |= (_h << 20);
}

unsigned
Node::getNum() const
{
  return (this->flag & NF_KN) >> 12;
}

bool
Node::setNum(int _num)
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
Node::addNum()
{
  if (this->getNum() + 1 > MAX_KEY_NUM) {
    print("error in addNum: Invalid!");
    return false;
  }
  this->flag += (1 << 12);
  return true;
}

bool
Node::subNum()
{
  if (this->getNum() < 1) {
    print("error in subNum: Invalid!");
    return false;
  }
  this->flag -= (1 << 12);
  return true;
}

unsigned
Node::getStore() const
{
  return this->store;
}

void
Node::setStore(unsigned _store)
{
  this->store = _store;
}

unsigned
Node::getFlag() const
{
  return flag;
}

void
Node::setFlag(unsigned _flag)
{
  this->flag = _flag;
}

const Bstr*
Node::getKey(int _index) const
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    //print(string("error in getKey: Invalid index ") + Util::int2string(_index));
    printf("error in getKey: Invalid index\n");
    return NULL;
  } else
    return this->keys + _index;
}

bool
Node::setKey(const Bstr* _key, int _index, bool ifcopy)
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    print(string("error in setKey: Invalid index ") + Util::int2string(_index));
    return false;
  }
  if (ifcopy)
    keys[_index].copy(_key);
  else
    keys[_index] = *_key;
  return true;
}

bool
Node::addKey(const Bstr* _key, int _index, bool ifcopy)
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
  if (ifcopy)
    keys[_index].copy(_key);
  else
    keys[_index] = *_key;
  return true;
}

bool
Node::subKey(int _index, bool ifdel)
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    print(string("error in subKey: Invalid index ") + Util::int2string(_index));
    return false;
  }
  int i;
  if (ifdel)
    keys[_index].release();
  for (i = _index; i < num - 1; ++i)
    keys[i] = keys[i + 1];
  return true;
}

int
Node::searchKey_less(const Bstr& _bstr) const
{
  int num = this->getNum();
  //for(i = 0; i < num; ++i)
  //if(bstr < *(p->getKey(i)))
  //break;

  int low = 0, high = num - 1, mid = -1;
  while (low <= high) {
    mid = (low + high) / 2;
    if (this->keys[mid] > _bstr) {
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
Node::searchKey_equal(const Bstr& _bstr) const
{
  int num = this->getNum();
  //for(i = 0; i < num; ++i)
  //	if(bstr == *(p->getKey(i)))
  //	{

  int ret = this->searchKey_less(_bstr);
  if (ret > 0 && this->keys[ret - 1] == _bstr)
    return ret - 1;
  else
    return num;
}

int
Node::searchKey_lessEqual(const Bstr& _bstr) const
{
  int num = this->getNum();
  //for(i = 0; i < num; ++i)
  //if(bstr <= *(p->getKey(i)))
  //break;

  int ret = this->searchKey_less(_bstr);
  if (ret > 0 && this->keys[ret - 1] == _bstr)
    return ret - 1;
  else
    return ret;
}
