/*=============================================================================
# Filename: SINode.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:39
# Description: achieve functions in SINode.h
=============================================================================*/

#include "SINode.h"

using namespace std;

void
SINode::AllocKeys()
{
  keys = new Bstr[MAX_KEY_NUM];
}

/*
void
SINode::FreeKeys()
{
delete[] keys;
}
*/

SINode::SINode()
{
  store = flag = 0;
  flag |= NF_IM;
  heapId = -1;
  AllocKeys();
}

SINode::SINode(bool isVirtual)
{
  store = flag = 0;
  heapId = -1;
  if (!isVirtual) {
    flag |= NF_IM;
    AllocKeys();
  }
}

/*
SINode::Node(Storage* TSM)
{
AllocKeys();
TSM->readSINode(this, Storage::OVER);
}
*/
bool
SINode::isLeaf() const
{
  return this->flag & NF_IL;
}

bool
SINode::isDirty() const
{
  return this->flag & NF_ID;
}

void
SINode::setDirty()
{
  this->flag |= NF_ID;
}

void
SINode::delDirty()
{
  this->flag &= ~NF_ID;
}

bool
SINode::inMem() const
{
  return this->flag & NF_IM;
}

void
SINode::setMem()
{
  this->flag |= NF_IM;
}

void
SINode::delMem()
{
  this->flag &= ~NF_IM;
}

/*
bool
SINode::isVirtual() const
{
return this->flag & NF_IV;
}

void
SINode::setVirtual()
{
this->flag |= NF_IV;
}

void
SINode::delVirtual()
{
this->flag &= ~NF_IV;
}
*/

unsigned
SINode::getRank() const
{
  return this->flag & NF_RK;
}

void
SINode::setRank(unsigned _rank)
{
  this->flag &= ~NF_RK;
  this->flag |= _rank;
}

unsigned
SINode::getHeight() const
{
  return (this->flag & NF_HT) >> 20;
}

void
SINode::setHeight(unsigned _h)
{
  this->flag &= ~NF_HT;
  this->flag |= (_h << 20);
}

unsigned
SINode::getNum() const
{
  return (this->flag & NF_KN) >> 12;
}

bool
SINode::setNum(int _num)
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
SINode::addNum()
{
  if (this->getNum() + 1 > MAX_KEY_NUM) {
    print("error in addNum: Invalid!");
    return false;
  }
  this->flag += (1 << 12);
  return true;
}

bool
SINode::subNum()
{
  if (this->getNum() < 1) {
    print("error in subNum: Invalid!");
    return false;
  }
  this->flag -= (1 << 12);
  return true;
}

unsigned
SINode::getStore() const
{
  return this->store;
}

void
SINode::setStore(unsigned _store)
{
  this->store = _store;
}

unsigned
SINode::getFlag() const
{
  return flag;
}

void
SINode::setFlag(unsigned _flag)
{
  this->flag = _flag;
}

const Bstr*
SINode::getKey(int _index) const
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
SINode::setKey(const Bstr* _key, int _index, bool ifcopy)
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
SINode::addKey(const Bstr* _key, int _index, bool ifcopy)
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
SINode::addKey(char* _str, unsigned _len, int _index, bool ifcopy)
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

  keys[_index].setStr(_str);
  keys[_index].setLen(_len);

  return true;
}

bool
SINode::subKey(int _index, bool ifdel)
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
SINode::searchKey_less(const Bstr& _bstr) const
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
SINode::searchKey_equal(const Bstr& _bstr) const
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
SINode::searchKey_lessEqual(const Bstr& _bstr) const
{
  //int num = this->getNum();
  //for(i = 0; i < num; ++i)
  //if(bstr <= *(p->getKey(i)))
  //break;

  int ret = this->searchKey_less(_bstr);
  if (ret > 0 && this->keys[ret - 1] == _bstr)
    return ret - 1;
  else
    return ret;
}

int
SINode::searchKey_less(const char* _str, unsigned _len) const
{
  int num = this->getNum();

  int low = 0, high = num - 1, mid = -1;
  while (low <= high) {
    mid = (low + high) / 2;
    //if (this->keys[mid] > _bstr)
    if (Util::compare(this->keys[mid].getStr(), this->keys[mid].getLen(), _str, _len) > 0) {
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
SINode::searchKey_equal(const char* _str, unsigned _len) const
{
  int num = this->getNum();
  //for(i = 0; i < num; ++i)
  //	if(bstr == *(p->getKey(i)))
  //	{

  int ret = this->searchKey_less(_str, _len);
  //if (ret > 0 && this->keys[ret - 1] == _bstr)
  if (ret > 0 && Util::compare(this->keys[ret - 1].getStr(), this->keys[ret - 1].getLen(), _str, _len) == 0)
    return ret - 1;
  else
    return num;
}

int
SINode::searchKey_lessEqual(const char* _str, unsigned _len) const
{
  int ret = this->searchKey_less(_str, _len);
  //if (ret > 0 && this->keys[ret - 1] == _bstr)
  if (ret > 0 && Util::compare(this->keys[ret - 1].getStr(), this->keys[ret - 1].getLen(), _str, _len) == 0)
    return ret - 1;
  else
    return ret;
}
