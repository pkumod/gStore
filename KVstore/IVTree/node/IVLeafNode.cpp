/*=============================================================================
# Filename: IVLeafNode.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:40
# Description: ahieve functions in IVLeafNode.h
=============================================================================*/

#include "IVLeafNode.h"

using namespace std;

void
IVLeafNode::AllocValues()
{
  values = new Bstr[MAX_KEY_NUM];
}

/*
void
IVLeafNode::FreeValues()
{
delete[] values;
}
*/

IVLeafNode::IVLeafNode()
{
  flag |= NF_IL; //leaf flag
  prev = next = NULL;
  AllocValues();
}

IVLeafNode::IVLeafNode(bool isVirtual)
{
  flag |= NF_IL;
  prev = next = NULL;
  if (!isVirtual)
    AllocValues();
}

/*
IVLeafNode::LeafNode(Storage* TSM)
{
AllocValues();
TSM->readNode(this, Storage::OVER);
}
*/

void
IVLeafNode::Virtual()
{
  //this->FreeKeys();
  //this->FreeValues();
  this->release();
  this->delMem();
}

void
IVLeafNode::Normal()
{
  this->AllocKeys();
  this->AllocValues();
  this->setMem();
}

IVNode*
IVLeafNode::getPrev() const
{
  return prev;
}

IVNode*
IVLeafNode::getNext() const
{
  return next;
}

const Bstr*
IVLeafNode::getValue(int _index) const
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    //print(string("error in getValue: Invalid index ") + Util::int2string(_index));
    return NULL;
  } else
    return this->values + _index;
}

bool
IVLeafNode::setValue(const Bstr* _value, int _index, bool _ifcopy)
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    print(string("error in setValue: Invalid index ") + Util::int2string(_index));
    return false;
  }

  this->values[_index].release(); //NOTICE: only used in modify

  if (_ifcopy) {
    this->values[_index].copy(_value);
  } else {
    this->values[_index] = *_value;
  }

  return true;
}

bool
IVLeafNode::getValue(VList* _vlist, int _index, char*& _str, unsigned& _len) const
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    //print(string("error in getValue: Invalid index ") + Util::int2string(_index));
    return NULL;
  }

  //read long list
  if (this->values[_index].isBstrLongList()) {
#ifdef DEBUG_VLIST
    cout << "this is a vlist in get()" << endl;
#endif
    unsigned block_num = this->values[_index].getLen();
    _vlist->readValue(block_num, _str, _len, this->keys[_index]);
  } else {
    _str = this->values[_index].getStr();
    _len = this->values[_index].getLen();
  }

  return true;
}

bool
IVLeafNode::setValue(VList* _vlist, int _index, char* _str, unsigned _len, bool ifcopy)
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    print(string("error in setValue: Invalid index ") + Util::int2string(_index));
    return false;
  }

  if (this->values[_index].isBstrLongList()) {
#ifdef DEBUG_VLIST
    cout << "this is a vlist in set()" << endl;
#endif
    unsigned block_num = this->values[_index].getLen();
    _vlist->removeValue(block_num, this->keys[_index]);
  } else {
    this->values[_index].release(); //NOTICE: only used in modify
  }

  //DEBUG: we do not need to copy here
  //we just need to ensure that the pointer's memory is not released

  //if (ifcopy)
  //{
  //this->values[_index].copy(_value);
  //}
  //else
  //{
  //this->values[_index] = *_value;
  if (VList::isLongList(_len)) {
    unsigned block_num = _vlist->writeValue(_str, _len);
    this->values[_index].setStr(NULL);
    this->values[_index].setLen(block_num);
    //NOTICE: we need to free the long list value
    delete[] _str;
  } else {
    this->values[_index].setStr(_str);
    this->values[_index].setLen(_len);
  }
  //}
  return true;
}

bool
IVLeafNode::addValue(VList* _vlist, int _index, char* _str, unsigned _len, bool ifcopy)
{
  int num = this->getNum();
  if (_index < 0 || _index > num) {
    print(string("error in addValue: Invalid index ") + Util::int2string(_index));
    return false;
  }

  for (int i = num - 1; i >= _index; --i)
    this->values[i + 1] = this->values[i];

  //if (ifcopy)
  //this->values[_index].copy(_value);
  //else
  //this->values[_index] = *_value;

  if (VList::isLongList(_len)) {
#ifdef DEBUG_VLIST
    cout << "this is a vlist in add()" << endl;
#endif
    unsigned block_num = _vlist->writeValue(_str, _len);
    this->values[_index].setStr(NULL);
    this->values[_index].setLen(block_num);
    //NOTICE: we need to free the long list value
    delete[] _str;
#ifdef DEBUG_VLIST
//cout<<"to check vlist: "<<this->values[_index].getLen()<<endl;
#endif
  } else {
    this->values[_index].setStr(_str);
    this->values[_index].setLen(_len);
  }
  //this->values[_index].setStr(_str);
  //this->values[_index].setLen(_len);

  return true;
}

bool
IVLeafNode::subValue(VList* _vlist, int _index, bool ifdel)
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    print(string("error in subValue: Invalid index ") + Util::int2string(_index));
    return false;
  }

  if (this->values[_index].isBstrLongList()) {
    unsigned block_num = this->values[_index].getLen();
    _vlist->removeValue(block_num, this->keys[_index]);
  } else {
    if (ifdel) {
      values[_index].release();
    }
  }

  for (int i = _index; i < num - 1; ++i)
    this->values[i] = this->values[i + 1];

  return true;
}

bool
IVLeafNode::addValue(const Bstr* _value, int _index, bool ifcopy)
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
IVLeafNode::subValue(int _index, bool ifdel)
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
IVLeafNode::setPrev(IVNode* _prev)
{
  this->prev = _prev;
}

void
IVLeafNode::setNext(IVNode* _next)
{
  this->next = _next;
}

unsigned
IVLeafNode::getSize() const
{
  unsigned sum = LEAF_SIZE, num = this->getNum(), i;
  for (i = 0; i < num; ++i) {
    sum += values[i].getLen();
  }
  return sum;
}

IVNode*
IVLeafNode::split(IVNode* _father, int _index)
{
  int num = this->getNum();
  IVNode* p = new IVLeafNode;      //right child
  p->setHeight(this->getHeight()); //NOTICE: assign height for new node
  p->setNext(this->next);
  this->setNext(p);
  p->setPrev(this);
  int i, k;
  for (i = MIN_KEY_NUM, k = 0; i < num; ++i, ++k) {
    p->addKey(this->keys[i], k);
    p->addValue(this->values + i, k);
    p->addNum();
  }
  unsigned tp = this->keys[MIN_KEY_NUM];
  this->setNum(MIN_KEY_NUM);
  _father->addKey(tp, _index);
  _father->addChild(p, _index + 1); //DEBUG(check the index)
  _father->addNum();
  _father->setDirty();
  p->setDirty();
  this->setDirty();
  return p;
}

IVNode*
IVLeafNode::coalesce(IVNode* _father, int _index)
{                                  //add a key or coalesce a neighbor to this
  int i, j = _father->getNum(), k; //BETTER: unsigned?
  IVNode* p = NULL;
  int ccase = 0;
  //const Bstr* bstr;
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
    IVNode* tp = _father->getChild(_index - 1);
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

  int tmp = 0;
  switch (ccase) {
  case 1: //union right to this
    for (i = 0; i < k; ++i) {
      this->addKey(p->getKey(i), this->getNum());
      this->addValue(p->getValue(i), this->getNum());
      this->addNum();
    }
    _father->subKey(_index);
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
    _father->setKey(p->getKey(1), _index);
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
    _father->subKey(_index - 1);
    _father->subChild(_index - 1);
    _father->subNum();
    this->prev = p->getPrev();
    if (this->prev != NULL) //else: leaves-list
      this->prev->setNext(this);
    p->setNum(0);
    //delete p;
    break;
  case 4: //move one from left
    tmp = p->getKey(k - 1);
    p->subKey(k - 1);
    this->addKey(tmp, 0);
    _father->setKey(tmp, _index - 1);
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
IVLeafNode::release()
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
    values[i].clear();
  }
  delete[] keys;
  delete[] values;
}

IVLeafNode::~IVLeafNode()
{
  release();
}

void
IVLeafNode::print(string s)
{
#ifdef DEBUG_KVSTORE
  unsigned num = this->getNum();
  fputs(Util::showtime().c_str(), Util::debug_kvstore);
  fputs("Class IVLeafNode\n", Util::debug_kvstore);
  fputs("Message: ", Util::debug_kvstore);
  fputs(s.c_str(), Util::debug_kvstore);
  fputs("\n", Util::debug_kvstore);
  unsigned i;
  if (s == "NODE") {
    fprintf(Util::debug_kvstore, "store: %u\tnum: %u\tflag: %u\n", this->store, num, this->flag);
    fprintf(Util::debug_kvstore, "prev: %p\tnext: %p\n", this->prev, this->next);
    for (i = 0; i < num; ++i) {
      //this->keys[i].print("BSTR");
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
