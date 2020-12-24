/*=============================================================================
# Filename: LeafNode.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:40
# Description: ahieve functions in LeafNode.h
=============================================================================*/

#include "LeafNode.h"

using namespace std;

void
LeafNode::AllocValues()
{
  values = new Bstr[MAX_KEY_NUM];
}

/*
void
LeafNode::FreeValues()
{
	delete[] values;
}
*/

LeafNode::LeafNode()
{
  flag |= NF_IL; //leaf flag
  prev = next = NULL;
  AllocValues();
}

LeafNode::LeafNode(bool isVirtual)
{
  flag |= NF_IL;
  prev = next = NULL;
  if (!isVirtual)
    AllocValues();
}

/*
LeafNode::LeafNode(Storage* TSM)
{
	AllocValues();
	TSM->readNode(this, Storage::OVER);	
}
*/

void
LeafNode::Virtual()
{
  //this->FreeKeys();
  //this->FreeValues();
  this->release();
  this->delMem();
}

void
LeafNode::Normal()
{
  this->AllocKeys();
  this->AllocValues();
  this->setMem();
}

Node*
LeafNode::getPrev() const
{
  return prev;
}

Node*
LeafNode::getNext() const
{
  return next;
}

const Bstr*
LeafNode::getValue(int _index) const
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    //print(string("error in getValue: Invalid index ") + Util::int2string(_index));
    return NULL;
  } else
    return this->values + _index;
}

bool
LeafNode::setValue(const Bstr* _value, int _index, bool ifcopy)
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    print(string("error in setValue: Invalid index ") + Util::int2string(_index));
    return false;
  }
  this->values[_index].release(); //NOTICE: only used in modify
  if (ifcopy)
    this->values[_index].copy(_value);
  else
    this->values[_index] = *_value;
  return true;
}

bool
LeafNode::addValue(const Bstr* _value, int _index, bool ifcopy)
{
  int num = this->getNum();
  if (_index < 0 || _index > num) {
    print(string("error in addValue: Invalid index ") + Util::int2string(_index));
    return false;
  }
  int i;
  for (i = num - 1; i >= _index; --i)
    this->values[i + 1] = this->values[i];
  if (ifcopy)
    this->values[_index].copy(_value);
  else
    this->values[_index] = *_value;
  return true;
}

bool
LeafNode::subValue(int _index, bool ifdel)
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    print(string("error in subValue: Invalid index ") + Util::int2string(_index));
    return false;
  }
  int i;
  if (ifdel)
    values[_index].release();
  for (i = _index; i < num - 1; ++i)
    this->values[i] = this->values[i + 1];
  return true;
}

void
LeafNode::setPrev(Node* _prev)
{
  this->prev = _prev;
}

void
LeafNode::setNext(Node* _next)
{
  this->next = _next;
}

unsigned
LeafNode::getSize() const
{
  unsigned sum = LEAF_SIZE, num = this->getNum(), i;
  for (i = 0; i < num; ++i) {
    sum += keys[i].getLen();
    sum += values[i].getLen();
  }
  return sum;
}

Node*
LeafNode::split(Node* _father, int _index)
{
  int num = this->getNum();
  Node* p = new LeafNode;          //right child
  p->setHeight(this->getHeight()); //NOTICE: assign height for new node
  p->setNext(this->next);
  this->setNext(p);
  p->setPrev(this);
  int i, k;
  for (i = MIN_KEY_NUM, k = 0; i < num; ++i, ++k) {
    p->addKey(this->keys + i, k);
    p->addValue(this->values + i, k);
    p->addNum();
  }
  const Bstr* tp = this->keys + MIN_KEY_NUM;
  this->setNum(MIN_KEY_NUM);
  _father->addKey(tp, _index, true);
  _father->addChild(p, _index + 1); //DEBUG(check the index)
  _father->addNum();
  _father->setDirty();
  p->setDirty();
  this->setDirty();
  return p;
}

Node*
LeafNode::coalesce(Node* _father, int _index)
{                                  //add a key or coalesce a neighbor to this
  int i, j = _father->getNum(), k; //BETTER: unsigned?
  Node* p = NULL;
  int ccase = 0;
  const Bstr* bstr;
  if (_index < j) //the right neighbor
  {
    p = _father->getChild(_index + 1);
    k = p->getNum();
    if ((unsigned)k > MIN_KEY_NUM)
      ccase = 2;
    else //==MIN_KEY_NUM
      ccase = 1;
  }
  if (_index > 0) //the left neighbor
  {
    Node* tp = _father->getChild(_index - 1);
    unsigned tk = tp->getNum();
    if (ccase < 2) {
      if (ccase == 0)
        ccase = 3;
      if (tk > MIN_KEY_NUM)
        ccase = 4;
    }
    if (ccase > 2) {
      p = tp;
      k = tk;
    }
  }
  switch (ccase) {
  case 1: //union right to this
    for (i = 0; i < k; ++i) {
      this->addKey(p->getKey(i), this->getNum());
      this->addValue(p->getValue(i), this->getNum());
      this->addNum();
    }
    _father->subKey(_index, true);
    _father->subChild(_index + 1);
    _father->subNum();
    this->next = p->getNext();
    if (this->next != NULL)
      this->next->setPrev(this);
    p->setNum(0); //NOTICE: adjust num before delete!
    //delete p;
    break;
  case 2: //move one from right
    this->addKey(p->getKey(0), this->getNum());
    _father->setKey(p->getKey(1), _index, true);
    p->subKey(0);
    this->addValue(p->getValue(0), this->getNum());
    p->subValue(0);
    this->addNum();
    p->subNum();
    break;
  case 3: //union left to this
    //BETTER: move all keys/etc one time
    for (i = k; i > 0; --i) {
      int t = i - 1;
      this->addKey(p->getKey(t), 0);
      this->addValue(p->getValue(t), 0);
      this->addNum();
    }
    _father->subKey(_index - 1, true);
    _father->subChild(_index - 1);
    _father->subNum();
    this->prev = p->getPrev();
    if (this->prev != NULL) //else: leaves-list
      this->prev->setNext(this);
    p->setNum(0);
    //delete p;
    break;
  case 4: //move one from left
    bstr = p->getKey(k - 1);
    p->subKey(k - 1);
    this->addKey(bstr, 0);
    _father->setKey(bstr, _index - 1, true);
    this->addValue(p->getValue(k - 1), 0);
    p->subValue(k - 1);
    this->addNum();
    p->subNum();
    break;
  default:
    print("error in coalesce: Invalid case!");
    //printf("error in coalesce: Invalid case!");
  }
  _father->setDirty();
  p->setDirty();
  this->setDirty();
  if (ccase == 1 || ccase == 3)
    return p;
  else
    return NULL;
}

void
LeafNode::release()
{
  if (!this->inMem())
    return;
  unsigned num = this->getNum();
  /*
	for(int i = 0; i < num; ++i)
	{
		keys[i].release();
		values[i].release();
	}
	*/
  for (unsigned i = num; i < MAX_KEY_NUM; ++i) {
    keys[i].clear();
    values[i].clear();
  }
  delete[] keys;
  delete[] values;
}

LeafNode::~LeafNode()
{
  release();
}

void
LeafNode::print(string s)
{
#ifdef DEBUG_KVSTORE
  unsigned num = this->getNum();
  fputs(Util::showtime().c_str(), Util::debug_kvstore);
  fputs("Class LeafNode\n", Util::debug_kvstore);
  fputs("Message: ", Util::debug_kvstore);
  fputs(s.c_str(), Util::debug_kvstore);
  fputs("\n", Util::debug_kvstore);
  unsigned i;
  if (s == "NODE") {
    fprintf(Util::debug_kvstore, "store: %u\tnum: %u\tflag: %u\n", this->store, num, this->flag);
    fprintf(Util::debug_kvstore, "prev: %p\tnext: %p\n", this->prev, this->next);
    for (i = 0; i < num; ++i) {
      this->keys[i].print("BSTR");
      this->values[i].print("BSTR");
    }
  } else if (s == "node") {
    fprintf(Util::debug_kvstore, "store: %u\tnum: %u\tflag: %u\n", this->store, num, this->flag);
    fprintf(Util::debug_kvstore, "prev: %p\tnext: %p\n", this->prev, this->next);
  } else if (s == "check node") {
    //check the node, if satisfy B+ definition
    bool flag = true;
    if (num < MIN_KEY_NUM || num > MAX_KEY_NUM)
      flag = false;
    if (flag) {
      for (i = 1; i < num; ++i) {
        if (keys[i] > keys[i - 1])
          continue;
        else
          break;
      }
      if (i < num)
        flag = false;
    }
    this->print("node");
    if (flag)
      fprintf(Util::debug_kvstore, "This node is good\n");
    else
      fprintf(Util::debug_kvstore, "This node is bad\n");
  } else
    ;
#endif
}
