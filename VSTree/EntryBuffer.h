/*
 *
 * EntryBuffer.h
 *
 *  Created on: 2014Äê7ÔÂ3ÈÕ
 *      Author: hanshuo
 */

#ifndef ENTRYBUFFER_H_
#define ENTRYBUFFER_H_

#include "../Util/Util.h"
#include "../Signature/SigEntry.h"

//class SigEntry;

//EntryBuffer is used to loading entries from hard disk when building VSTree.
class EntryBuffer {
  private:
  int capacity;
  int num;
  SigEntry* elems;

  public:
  static int DEFAULT_CAPACITY;

  EntryBuffer(int _capacity = 200000); // to be determine the default capacity.
  ~EntryBuffer();
  int getCapacity() const;
  int getNum() const;
  bool isEmpty() const;
  bool isFull() const;
  SigEntry* getElem(int _i);
  bool insert(const SigEntry& _entry);
  int fillElemsFromFile(FILE* _p_file); // fill this buffer with SigEntry from _p_file, until the buffer is full or meeting EOF.
  void clear();
};

#endif /* ENTRYBUFFER_H_ */
