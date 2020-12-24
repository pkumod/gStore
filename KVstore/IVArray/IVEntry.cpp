/*=========================================================================
 * File name: IVEntry.cpp
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last Modified: 2018-01-30
 * Description:
 * Implementation of functions in IVEntry.h
 * =======================================================================*/

#include "IVEntry.h"

using namespace std;

IVEntry::IVEntry()
{
  store = 0;
  value = NULL;
  usedFlag = false;
  dirtyFlag = true;
  cacheFlag = false;
  CachePinFlag = false;
  prevID = nextID = -1;
}

void
IVEntry::setBstr(const Bstr* _value)
{
  if (value != NULL)
    delete value;
  value = new Bstr(*_value);
}

void
IVEntry::setBstr(char* _str, unsigned _len)
{
  if (value != NULL)
    delete value;
  value = new Bstr();
  //value->copy(_str, _len);
  value->setStr(_str);
  value->setLen(_len);
}

bool
IVEntry::getBstr(char*& _str, unsigned& _len, bool if_copy) const
{
  if (value == NULL) {
    _str = NULL;
    _len = 0;
    return false;
  }

  if (if_copy) {
    char* str = value->getStr();
    _len = value->getLen();
    _str = new char[_len];
    memcpy(_str, str, _len);
  } else {
    _str = value->getStr();
    _len = value->getLen();
  }
  return true;
}

void
IVEntry::setStore(unsigned _store)
{
  store = _store;
}

unsigned
IVEntry::getStore() const
{
  return store;
}

void
IVEntry::setUsedFlag(bool _flag)
{
  usedFlag = _flag;
}

bool
IVEntry::isUsed() const
{
  return usedFlag;
}

void
IVEntry::setDirtyFlag(bool _flag)
{
  dirtyFlag = _flag;
}

bool
IVEntry::isDirty() const
{
  return dirtyFlag;
}

void
IVEntry::setCacheFlag(bool _flag)
{
  cacheFlag = _flag;
}

bool
IVEntry::inCache() const
{
  return cacheFlag;
}

void
IVEntry::setCachePinFlag(bool _flag)
{
  CachePinFlag = _flag;
}

bool
IVEntry::isPined()
{
  return CachePinFlag;
}

void
IVEntry::release()
{
  if (value != NULL) {
    delete value;
  }
  value = NULL;
  nextID = prevID = -1;
}

void
IVEntry::Copy(const IVEntry& _entry)
{
  this->store = _entry.store;
  this->cacheFlag = _entry.cacheFlag;
  this->dirtyFlag = _entry.dirtyFlag;
  this->usedFlag = _entry.usedFlag;
  if (_entry.value != NULL) {
    this->value = new Bstr();
    value->copy(_entry.value);
  }
  this->prevID = _entry.prevID;
  this->nextID = _entry.nextID;
}

void
IVEntry::setPrev(int ID)
{
  prevID = ID;
}

int
IVEntry::getPrev() const
{
  return prevID;
}

void
IVEntry::setNext(int ID)
{
  nextID = ID;
}

int
IVEntry::getNext() const
{
  return nextID;
}

IVEntry::~IVEntry()
{
  this->release();
}
