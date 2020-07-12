/*=============================================================================
# Filename: ISLeafNode.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:40
# Description: ahieve functions in ISLeafNode.h
=============================================================================*/

#include "ISLeafNode.h"

using namespace std;

void
ISLeafNode::AllocValues()
{
  values = new Bstr[MAX_KEY_NUM];
}

/*
void
ISLeafNode::FreeValues()
{
delete[] values;
}
*/

ISLeafNode::ISLeafNode()
{
  flag |= NF_IL; //leaf flag
  prev = next = NULL;
  AllocValues();
}

ISLeafNode::ISLeafNode(bool isVirtual)
{
  flag |= NF_IL;
  prev = next = NULL;
  if (!isVirtual)
    AllocValues();
}

/*
ISLeafNode::LeafNode(Storage* TSM)
{
AllocValues();
TSM->readNode(this, Storage::OVER);
}
*/

void
ISLeafNode::Virtual()
{
  //this->FreeKeys();
  //this->FreeValues();
  this->release();
  this->delMem();
}

void
ISLeafNode::Normal()
{
  this->AllocKeys();
  this->AllocValues();
  this->setMem();
}

ISNode*
ISLeafNode::getPrev() const
{
  return prev;
}

ISNode*
ISLeafNode::getNext() const
{
  return next;
}

const Bstr*
ISLeafNode::getValue(int _index) const
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    //cout<<"null in getValue: "<<_index<<endl;
    //print(string("error in getValue: Invalid index ") + Util::int2string(_index));
    return NULL;
  } else
    return this->values + _index;
}

bool
ISLeafNode::setValue(const Bstr* _value, int _index, bool ifcopy)
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
ISLeafNode::addValue(const Bstr* _value, int _index, bool ifcopy)
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
ISLeafNode::setValue(char* _str, unsigned _len, int _index, bool ifcopy)
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    //print(string("error in setValue: Invalid index ") + Util::int2string(_index));
    return false;
  }
  this->values[_index].release(); //NOTICE: only used in modify

  this->values[_index].setStr(_str);
  this->values[_index].setLen(_len);

  return true;
}

bool
ISLeafNode::addValue(char* _str, unsigned _len, int _index, bool ifcopy)
{
  int num = this->getNum();
  //cout<<"addValue: "<<num<<" "<<_index<<endl;

  if (_index < 0 || _index > num) {
    //print(string("error in addValue: Invalid index ") + Util::int2string(_index));
    //cout<<"error in addValue: "<<_index<<" "<<num<<endl;
    return false;
  }
  int i;
  for (i = num - 1; i >= _index; --i)
    this->values[i + 1] = this->values[i];

  this->values[_index].setStr(_str);
  this->values[_index].setLen(_len);
  //cout<<"show: "<<this->values[_index].getLen()<<" "<<this->values[_index].getStr()[0]<<endl;

  return true;
}

bool
ISLeafNode::subValue(int _index, bool ifdel)
{
  int num = this->getNum();
  if (_index < 0 || _index >= num) {
    //print(string("error in subValue: Invalid index ") + Util::int2string(_index));
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
ISLeafNode::setPrev(ISNode* _prev)
{
  this->prev = _prev;
}

void
ISLeafNode::setNext(ISNode* _next)
{
  this->next = _next;
}

unsigned
ISLeafNode::getSize() const
{
  unsigned sum = LEAF_SIZE, num = this->getNum(), i;
  for (i = 0; i < num; ++i) {
    sum += values[i].getLen();
  }
  return sum;
}

ISNode*
ISLeafNode::split(ISNode* _father, int _index)
{
  int num = this->getNum();
  ISNode* p = new ISLeafNode;      //right child
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
  //cout<<"split: "<<tp<<" "<<this->values[MIN_KEY_NUM].getStr()<<endl;
  this->setNum(MIN_KEY_NUM);
  _father->addKey(tp, _index);
  _father->addChild(p, _index + 1); //DEBUG(check the index)
  _father->addNum();
  _father->setDirty();
  p->setDirty();
  this->setDirty();
  return p;
}

ISNode*
ISLeafNode::coalesce(ISNode* _father, int _index)
{                                  //add a key or coalesce a neighbor to this
  int i, j = _father->getNum(), k; //BETTER: unsigned?
  ISNode* p = NULL;
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
    ISNode* tp = _father->getChild(_index - 1);
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

  unsigned tmp = 0;
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
    //print("error in coalesce: Invalid case!");
    cout << "error in coalesce: Invalid case!" << endl;
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
ISLeafNode::release()
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

ISLeafNode::~ISLeafNode()
{
  release();
}

void
ISLeafNode::print(string s)
{
#ifdef DEBUG_KVSTORE
  unsigned num = this->getNum();
  fputs(Util::showtime().c_str(), Util::debug_kvstore);
  fputs("Class ISLeafNode\n", Util::debug_kvstore);
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
