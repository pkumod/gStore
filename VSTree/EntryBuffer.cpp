/*
 * EntryBuffer.cpp
 *
 *  Created on: 2014-7-3
 *      Author: hanshuo
 */

#include "EntryBuffer.h"

using namespace std;

int EntryBuffer::DEFAULT_CAPACITY = 2 * 1000 * 1000;

EntryBuffer::EntryBuffer(int _capacity)
{
  this->capacity = _capacity;
  this->num = 0;
  this->elems = new SigEntry[this->capacity];
}

EntryBuffer::~EntryBuffer()
{
  delete[] this->elems;
}

int EntryBuffer::getCapacity() const
{
  return this->capacity;
}

int EntryBuffer::getNum() const
{
  return this->num;
}

bool EntryBuffer::isEmpty() const
{
  return (this->num == 0);
}

bool EntryBuffer::isFull() const
{
  return (this->num == this->capacity);
}

SigEntry* EntryBuffer::getElem(int _i)
{
  return &this->elems[_i];
}

bool EntryBuffer::insert(const SigEntry& _entry)
{
  if (this->isFull()) {
    cerr << "error, entry buffer is full, can not insert now in EntryBuffer::insert." << endl;
    return false;
  }

  this->elems[this->num++] = _entry;

  return true;
}

/* clear the entry buffer and fill it from the entry file until the buffer is full or the file ends. */
int EntryBuffer::fillElemsFromFile(FILE* _p_file)
{
  this->clear();
  SigEntry entry;

  // bug here! if we read an entry, we must put it into buffer. Otherwise this entry will be missed.
  //    fread((char *)&entry, sizeof(SigEntry), 1, _p_file);
  //    while (!this->isFull() &&  !feof(_p_file))
  //    {
  //        this->insert(entry);
  //        fread((char *)&entry, sizeof(SigEntry), 1, _p_file);
  //    }

  while (!this->isFull()) {
    bool is_reach_EOF = feof(_p_file);
    bool is_entry_read = (fread((char*)&entry, sizeof(SigEntry), 1, _p_file) == 1);
    if (is_reach_EOF || !is_entry_read) {
      break;
    }
    this->insert(entry);
  }

  return this->num;
}

void EntryBuffer::clear()
{
  this->num = 0;
}
