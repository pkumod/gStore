/*=========================================================================
 * File name: ISEntry.cpp
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last Modified: 2018-01-30
 * Description:
 * Implementation of functions in ISEntry.h
 * =======================================================================*/

#include "ISEntry.h"

using namespace std;

ISEntry::ISEntry()
{
  store = 0;
  value = NULL;
  usedFlag = false;
  dirtyFlag = true;
  cacheFlag = false;
  prevID = nextID = -1;
}

void
ISEntry::setBstr(const Bstr* _value)
{
  if (value != NULL)
    delete value;
  value = new Bstr(*_value);
}

void
ISEntry::setBstr(const char* _str, unsigned _len)
{
  if (value != NULL)
    delete value;
  value = new Bstr();
  //value->copy(_str, _len);
  value->setStr((char*)_str);
  value->setLen(_len);
}

bool
ISEntry::getBstr(char*& _str, unsigned& _len, bool if_copy) const
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
ISEntry::setStore(unsigned _store)
{
  store = _store;
}

unsigned
ISEntry::getStore() const
{
  return store;
}

void
ISEntry::setUsedFlag(bool _flag)
{
  usedFlag = _flag;
}

bool
ISEntry::isUsed() const
{
  return usedFlag;
}

void
ISEntry::setDirtyFlag(bool _flag)
{
  dirtyFlag = _flag;
}

bool
ISEntry::isDirty() const
{
  return dirtyFlag;
}

void
ISEntry::setCacheFlag(bool _flag)
{
  cacheFlag = _flag;
}

bool
ISEntry::inCache() const
{
  return cacheFlag;
}

void
ISEntry::release()
{
  if (value != NULL) {
    delete value;
  }
  value = NULL;
  prevID = nextID = -1;
}

void
ISEntry::Copy(const ISEntry& _entry)
{
  this->store = _entry.store;
  this->cacheFlag = _entry.cacheFlag;
  this->dirtyFlag = _entry.dirtyFlag;
  this->usedFlag = _entry.usedFlag;
  this->prevID = _entry.prevID;
  this->nextID = _entry.nextID;
  if (_entry.value != NULL) {
    this->value = new Bstr();
    value->copy(_entry.value);
  }
}

void
ISEntry::setPrev(int ID)
{
  prevID = ID;
}

int
ISEntry::getPrev() const
{
  return prevID;
}

void
ISEntry::setNext(int ID)
{
  nextID = ID;
}

int
ISEntry::getNext() const
{
  return nextID;
}

ISEntry::~ISEntry()
{
  if (value != NULL) {
    delete value;
  }
  value = NULL;
}
