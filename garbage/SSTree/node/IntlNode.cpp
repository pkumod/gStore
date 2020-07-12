/*=============================================================================
# Filename: IntlNode.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:40
# Description: achieve functions in IntlNode.h
=============================================================================*/

#include "IntlNode.h"

using namespace std;

/*
void
IntlNode::AllocChilds()
{
	childs = (Node**)malloc(sizeof(Node*) * MAX_CHILD_NUM);
}
*/

IntlNode::IntlNode()
{
  memset(childs, 0, sizeof(Node*) * MAX_CHILD_NUM);
  //this->AllocChilds();
}

IntlNode::IntlNode(bool isVirtual) //call father-class's constructor automaticlly
{
  memset(childs, 0, sizeof(Node*) * MAX_CHILD_NUM);
  //this->AllocChilds();
}

/*
IntlNode::IntlNode(Storage* TSM)	//QUERY
{
	TSM->readNode(this, Storage::OVER);
}
*/

void
IntlNode::Virtual()
{
  //this->FreeKeys();
  this->release();
  this->delMem();
}

void
IntlNode::Normal()
{
  this->AllocKeys();
  this->setMem();
}

Node*
IntlNode::getChild(int _index) const
{
  int num = this->getNum();
  if (_index < 0 || _index > num) //num keys, num+1 childs
  {
    //print(string("error in getChild: Invalid index ") + Util::int2string(_index));
    return NULL;
  } else
    return childs[_index];
}

bool
IntlNode::setChild(Node* _child, int _index)
{
  int num = this->getNum();
  if (_index < 0 || _index > num) {
    print(string("error in setChild: Invalid index ") + Util::int2string(_index));
    return false;
  }
  this->childs[_index] = _child;
  return true;
}

bool
IntlNode::addChild(Node* _child, int _index)
{
  int num = this->getNum();
  if (_index < 0 || _index > num + 1) {
    print(string("error in addChild: Invalid index ") + Util::int2string(_index));
    return false;
  }
  int i;
  for (i = num; i >= _index; --i) //DEBUG: right bounder!!!
    childs[i + 1] = childs[i];
  childs[_index] = _child;
  return true;
}

bool
IntlNode::subChild(int _index)
{
  int num = this->getNum();
  if (_index < 0 || _index > num) {
    print(string("error in subchild: Invalid index ") + Util::int2string(_index));
    return false;
  }
  int i;
  for (i = _index; i < num; ++i) //DEBUG: right bounder!!!
    childs[i] = childs[i + 1];
  return true;
}

unsigned
IntlNode::getSize() const
{
  unsigned sum = INTL_SIZE, num = this->getNum(), i;
  for (i = 0; i < num; ++i)
    sum += keys[i].getLen();
  return sum;
}

Node*
IntlNode::split(Node* _father, int _index)
{
  int num = this->getNum();
  Node* p = new IntlNode; //right child
  p->setHeight(this->getHeight());
  int i, k;
  for (i = MIN_CHILD_NUM, k = 0; i < num; ++i, ++k) {
    p->addKey(this->keys + i, k);
    p->addChild(this->childs[i], k);
    p->addNum();
  }
  p->addChild(this->childs[i], k);
  const Bstr* tp = this->keys + MIN_KEY_NUM;
  this->setNum(MIN_KEY_NUM);
  _father->addKey(tp, _index);
  _father->addChild(p, _index + 1); //DEBUG(check the index)
  _father->addNum();
  _father->setDirty();
  p->setDirty();
  this->setDirty();
  return p;
}

Node*
IntlNode::coalesce(Node* _father, int _index)
{
  //int num = this->getNum();
  int i, j = _father->getNum(), k; //BETTER: unsigned?
  Node* p;
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
    this->addKey(_father->getKey(_index), this->getNum());
    this->addNum();
    for (i = 0; i < k; ++i) {
      this->addKey(p->getKey(i), this->getNum());
      this->addChild(p->getChild(i), this->getNum());
      this->addNum();
    }
    this->setChild(p->getChild(i), this->getNum());
    _father->subKey(_index);
    _father->subChild(_index + 1);
    _father->subNum();
    p->setNum(0);
    //delete p;
    break;
  case 2: //move one form right
    this->addKey(_father->getKey(_index), this->getNum());
    _father->setKey(p->getKey(0), _index);
    p->subKey(0);
    this->addChild(p->getChild(0), this->getNum() + 1);
    p->subChild(0);
    this->addNum();
    p->subNum();
    break;
  case 3: //union left to this
    this->addKey(_father->getKey(_index - 1), 0);
    this->addNum();
    for (i = k; i > 0; --i) {
      int t = i - 1;
      this->addKey(p->getKey(t), 0);
      this->addChild(p->getChild(i), 0);
      this->addNum();
    }
    this->addChild(p->getChild(0), 0);
    _father->subKey(_index - 1);
    _father->subChild(_index - 1);
    _father->subNum();
    p->setNum(0);
    //delete p;
    break;
  case 4: //move one from left
    bstr = p->getKey(k - 1);
    p->subKey(k - 1);
    this->addKey(_father->getKey(_index - 1), 0);
    _father->setKey(bstr, _index - 1);
    this->addChild(p->getChild(k), 0);
    p->subChild(k);
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
IntlNode::release()
{
  if (!this->inMem())
    return;
  unsigned num = this->getNum();
  //delete[] keys;  //this will release all!!!
  for (unsigned i = num; i < MAX_KEY_NUM; ++i)
    keys[i].clear();
  delete[] keys;
}

IntlNode::~IntlNode()
{
  release();
  //free(childs);
}

void
IntlNode::print(string s)
{
#ifdef DEBUG_KVSTORE
  int num = this->getNum();
  fputs(Util::showtime().c_str(), Util::debug_kvstore);
  fputs("Class IntlNode\n", Util::debug_kvstore);
  fputs("Message: ", Util::debug_kvstore);
  fputs(s.c_str(), Util::debug_kvstore);
  fputs("\n", Util::debug_kvstore);
  if (s == "node" || s == "NODE") {
    fprintf(Util::debug_kvstore, "store: %u\tnum: %u\tflag: %u\n", this->store, num, this->flag);
    int i;
    for (i = 0; i < num; ++i) {
      if (s == "node")
        this->keys[i].print("bstr");
      else
        this->keys[i].print("BSTR");
    }
  } else if (s == "check node") {
    //TODO(check node, if satisfy B+ definition)
  } else
    ;
#endif
}
