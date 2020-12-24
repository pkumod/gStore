/*=============================================================================
# Filename: Tree.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:45
# Description: achieve functions in Tree.h
=============================================================================*/

#include "Tree.h"

using namespace std;

//tree's operations should be atom(if read nodes)
//sum the request and send to Storage at last
//ensure that all nodes operated are in memory
int request = 0;

Tree::Tree()
{
  height = 0;
  mode = "";
  root = NULL;
  leaves_head = NULL;
  leaves_tail = NULL;
  TSM = NULL;
  storepath = "";
  filename = "";
  transfer_size[0] = transfer_size[1] = transfer_size[2] = 0;
  this->stream = NULL;
}

Tree::Tree(const string& _storepath, const string& _filename, const char* _mode)
{
  storepath = _storepath;
  filename = _filename;
  this->height = 0;
  this->mode = string(_mode);
  string filepath = this->getFilePath();
  TSM = new Storage(filepath, this->mode, &this->height);
  if (this->mode == "open")
    this->TSM->preRead(this->root, this->leaves_head, this->leaves_tail);
  else
    this->root = NULL;
  this->transfer[0].setStr((char*)malloc(Util::TRANSFER_SIZE));
  this->transfer[1].setStr((char*)malloc(Util::TRANSFER_SIZE));
  this->transfer[2].setStr((char*)malloc(Util::TRANSFER_SIZE));
  this->transfer_size[0] = this->transfer_size[1] = this->transfer_size[2] = Util::TRANSFER_SIZE; //initialied to 1M
  this->stream = NULL;
}

string
Tree::getFilePath()
{
  return storepath + "/" + filename;
}

void //WARN: not check _str and _len
    Tree::CopyToTransfer(const char* _str, unsigned _len, unsigned _index)
{
  if (_index > 2)
    return;
  /*
	if(_str == NULL || _len == 0)
	{
		printf("error in CopyToTransfer: empty string\n");
		return;
	}
	*/
  //unsigned length = _bstr->getLen();
  unsigned length = _len;
  if (length + 1 > this->transfer_size[_index]) {
    transfer[_index].release();
    transfer[_index].setStr((char*)malloc(length + 1));
    this->transfer_size[_index] = length + 1; //one more byte: convenient to add \0
  }
  memcpy(this->transfer[_index].getStr(), _str, length);
  this->transfer[_index].getStr()[length] = '\0'; //set for string() in KVstore
  this->transfer[_index].setLen(length);
}

unsigned
Tree::getHeight() const
{
  return this->height;
}

void
Tree::setHeight(unsigned _h)
{
  this->height = _h;
}

Node*
Tree::getRoot() const
{
  return this->root;
}

void
Tree::prepare(Node* _np) const
{
  bool flag = _np->inMem();
  if (!flag)
    this->TSM->readNode(_np, &request); //readNode deal with request
}

bool
Tree::search(const char* _str1, unsigned _len1, char*& _str2, int& _len2)
{
  const Bstr* value = NULL;
  if (_str1 == NULL || _len1 == 0) {
    printf("error in Tree-search: empty string\n");
    return false;
  }
  this->CopyToTransfer(_str1, _len1, 1);
  bool ret = this->search(&transfer[1], value);
  if (ret) {
    _str2 = value->getStr();
    _len2 = value->getLen();
  }
  return ret;
}

bool
Tree::search(const Bstr* _key, const Bstr*& _value)
{
  request = 0;
  Bstr bstr = *_key; //not to modify its memory
  int store;
  Node* ret = this->find(_key, &store, false);
  if (ret == NULL || store == -1 || bstr != *(ret->getKey(store))) //tree is empty or not found
  {
    bstr.clear();
    return false;
  }
  const Bstr* val = ret->getValue(store);
  this->CopyToTransfer(val->getStr(), val->getLen(), 0); //not sum to request
  _value = &transfer[0];
  this->TSM->request(request);
  bstr.clear();
  return true;
}

bool
Tree::insert(const char* _str1, unsigned _len1, const char* _str2, unsigned _len2)
{
  if (_str1 == NULL || _len1 == 0) {
    printf("error in Tree-insert: empty string\n");
    return false;
  }
  this->CopyToTransfer(_str1, _len1, 1);
  this->CopyToTransfer(_str2, _len2, 2); //not check value
  bool ret = this->insert(&transfer[1], &transfer[2]);
  return ret;
}

bool
Tree::insert(const Bstr* _key, const Bstr* _value)
{
  request = 0;
  Node* ret;
  if (this->root == NULL) //tree is empty
  {
    leaves_tail = leaves_head = root = new LeafNode;
    request += Node::LEAF_SIZE;
    this->height = 1;
    root->setHeight(1); //add to heap later
  }
  //this->prepare(this->root); //root must be in-mem
  if (root->getNum() == Node::MAX_KEY_NUM) {
    Node* father = new IntlNode;
    request += Node::INTL_SIZE;
    father->addChild(root, 0);
    ret = root->split(father, 0);
    if (ret->isLeaf() && ret->getNext() == NULL)
      this->leaves_tail = ret;
    if (ret->isLeaf())
      request += Node::LEAF_SIZE;
    else
      request += Node::INTL_SIZE;
    this->height++; //height rises only when root splits
    //WARN: height area in Node: 4 bit!
    father->setHeight(this->height); //add to heap later
    this->TSM->updateHeap(ret, ret->getRank(), false);
    this->root = father;
  }
  Node* p = this->root;
  Node* q;
  int i, j;
  Bstr bstr = *_key;
  while (!p->isLeaf()) {
    //j = p->getNum();
    //for(i = 0; i < j; ++i)
    //if(bstr < *(p->getKey(i)))
    //break;
    //NOTICE: using binary search is better here
    i = p->searchKey_less(bstr);

    q = p->getChild(i);
    this->prepare(q);
    if (q->getNum() == Node::MAX_KEY_NUM) {
      ret = q->split(p, i);
      if (ret->isLeaf() && ret->getNext() == NULL)
        this->leaves_tail = ret;
      if (ret->isLeaf())
        request += Node::LEAF_SIZE;
      else
        request += Node::INTL_SIZE;
      //BETTER: in loop may update multiple times
      this->TSM->updateHeap(ret, ret->getRank(), false);
      this->TSM->updateHeap(q, q->getRank(), true);
      this->TSM->updateHeap(p, p->getRank(), true);
      if (bstr < *(p->getKey(i)))
        p = q;
      else
        p = ret;
    } else {
      p->setDirty();
      this->TSM->updateHeap(p, p->getRank(), true);
      p = q;
    }
  }
  //j = p->getNum();
  //for(i = 0; i < j; ++i)
  //if(bstr < *(p->getKey(i)))
  //break;
  i = p->searchKey_less(bstr);

  //insert existing key is ok, but not inserted in
  //however, the tree-shape may change due to possible split in former code
  bool ifexist = false;
  if (i > 0 && bstr == *(p->getKey(i - 1)))
    ifexist = true;
  else {
    p->addKey(_key, i, true);
    p->addValue(_value, i, true);
    p->addNum();
    request += (_key->getLen() + _value->getLen());
    p->setDirty();
    this->TSM->updateHeap(p, p->getRank(), true);
    //_key->clear();
    //_value->clear();
  }
  this->TSM->request(request);
  bstr.clear();    //NOTICE: must be cleared!
  return !ifexist; //QUERY(which case:return false)
}

bool
Tree::modify(const char* _str1, unsigned _len1, const char* _str2, unsigned _len2)
{
  if (_str1 == NULL || _len1 == 0) {
    printf("error in Tree-modify: empty string\n");
    return false;
  }
  this->CopyToTransfer(_str1, _len1, 1);
  this->CopyToTransfer(_str2, _len2, 2); //not check value
  bool ret = this->modify(&transfer[1], &transfer[2]);
  return ret;
}

bool
Tree::modify(const Bstr* _key, const Bstr* _value)
{
  request = 0;
  Bstr bstr = *_key;
  int store;
  Node* ret = this->find(_key, &store, true);
  if (ret == NULL || store == -1 || bstr != *(ret->getKey(store))) //tree is empty or not found
  {
    bstr.clear();
    return false;
  }
  unsigned len = ret->getValue(store)->getLen();
  ret->setValue(_value, store, true);
  request += (_value->getLen() - len);
  //_value->clear();
  ret->setDirty();
  this->TSM->request(request);
  bstr.clear();
  return true;
}

//this function is useful for search and modify, and range-query
Node* //return the first key's position that >= *_key
    Tree::find(const Bstr* _key, int* _store, bool ifmodify) const
{ //to assign value for this->bstr, function shouldn't be const!
  if (this->root == NULL)
    return NULL; //Tree Is Empty
  Node* p = root;
  int i, j;
  Bstr bstr = *_key; //local Bstr: multiple delete
  while (!p->isLeaf()) {
    if (ifmodify)
      p->setDirty();
    //j = p->getNum();
    //for(i = 0; i < j; ++i)				//BETTER(Binary-Search)
    //if(bstr < *(p->getKey(i)))
    //break;
    i = p->searchKey_less(bstr);

    p = p->getChild(i);
    this->prepare(p);
  }

  j = p->getNum();
  //for(i = 0; i < j; ++i)
  //if(bstr <= *(p->getKey(i)))
  //break;
  i = p->searchKey_lessEqual(bstr);

  if (i == j)
    *_store = -1; //Not Found
  else
    *_store = i;
  bstr.clear();
  return p;
}

/*
Node*
Tree::find(unsigned _len, const char* _str, int* store) const
{
}
*/

bool
Tree::remove(const char* _str, unsigned _len)
{
  if (_str == NULL || _len == 0) {
    printf("error in Tree-remove: empty string\n");
    return false;
  }
  this->CopyToTransfer(_str, _len, 1);
  bool ret = this->remove(&transfer[1]);
  return ret;
}

bool //BETTER: if not found, the road are also dirty! find first?
    Tree::remove(const Bstr* _key)
{
  request = 0;
  Node* ret;
  if (this->root == NULL) //tree is empty
    return false;
  Node* p = this->root;
  Node* q;
  int i, j;
  Bstr bstr = *_key;
  while (!p->isLeaf()) {
    j = p->getNum();
    //for(i = 0; i < j; ++i)
    //if(bstr < *(p->getKey(i)))
    //break;
    i = p->searchKey_less(bstr);

    q = p->getChild(i);
    this->prepare(q);
    if (q->getNum() < Node::MIN_CHILD_NUM) //==MIN_KEY_NUM
    {
      if (i > 0)
        this->prepare(p->getChild(i - 1));
      if (i < j)
        this->prepare(p->getChild(i + 1));
      ret = q->coalesce(p, i);
      if (ret != NULL)
        this->TSM->updateHeap(ret, 0, true); //non-sense node
      this->TSM->updateHeap(q, q->getRank(), true);
      if (q->isLeaf()) {
        if (q->getPrev() == NULL)
          this->leaves_head = q;
        if (q->getNext() == NULL)
          this->leaves_tail = q;
      }
      if (p->getNum() == 0) //root shrinks
      {
        //this->leaves_head = q;
        this->root = q;
        this->TSM->updateHeap(p, 0, true); //instead of delete p
        this->height--;
      }
    } else
      p->setDirty();
    this->TSM->updateHeap(p, p->getRank(), true);
    p = q;
  }
  bool flag = false;
  //j = p->getNum();		//LeafNode(maybe root)
  //for(i = 0; i < j; ++i)
  //	if(bstr == *(p->getKey(i)))
  //	{
  //		request -= p->getKey(i)->getLen();
  //		request -= p->getValue(i)->getLen();
  //		p->subKey(i, true);		//to release
  //		p->subValue(i, true);	//to release
  //		p->subNum();
  //		if(p->getNum() == 0)	//root leaf 0 key
  //		{
  //			this->root = NULL;
  //			this->leaves_head = NULL;
  //			this->leaves_tail = NULL;
  //			this->height = 0;
  //			this->TSM->updateHeap(p, 0, true);	//instead of delete p
  //		}
  //		p->setDirty();
  //		flag = true;
  //		break;
  //	}
  i = p->searchKey_equal(bstr);
  request -= p->getKey(i)->getLen();
  request -= p->getValue(i)->getLen();
  p->subKey(i, true);   //to release
  p->subValue(i, true); //to release
  p->subNum();
  if (p->getNum() == 0) //root leaf 0 key
  {
    this->root = NULL;
    this->leaves_head = NULL;
    this->leaves_tail = NULL;
    this->height = 0;
    this->TSM->updateHeap(p, 0, true); //instead of delete p
  }
  p->setDirty();
  flag = true;

  this->TSM->request(request);
  bstr.clear();
  return flag; //i == j, not found
}

const Bstr*
Tree::getRangeValue()
{
  if (this->stream == NULL) {
    fprintf(stderr, "Tree::getRangeValue(): no results now!\n");
    return NULL;
  }
  if (this->stream->isEnd()) {
    fprintf(stderr, "Tree::getRangeValue(): read till end now!\n");
    return NULL;
  }
  //NOTICE:this is one record, and donot free the memory!
  //NOTICE:Bstr[] but only one element, used as Bstr*
  return this->stream->read();
}

void
Tree::resetStream()
{
  if (this->stream == NULL) {
    fprintf(stderr, "no results now!\n");
    return;
  }
  this->stream->setEnd();
}

bool //special case: not exist, one-edge-case
    Tree::range_query(const Bstr* _key1, const Bstr* _key2)
{ //the range is: *_key1 <= x < *_key2
  //if(_key1 == NULL && _key2 == NULL)
  //return false;
  //ok to search one-edge, requiring only one be NULL
  //find and write value
  int store1, store2;
  Node* p1, *p2;
  if (_key1 != NULL) {
    request = 0;
    p1 = this->find(_key1, &store1, false);
    if (p1 == NULL || store1 == -1)
      return false; //no element
    this->TSM->request(request);
  } else {
    p1 = this->leaves_head;
    store1 = 0;
  }
  if (_key2 != NULL) { //QUERY: another strategy is to getnext and compare every time to tell end
    request = 0;
    p2 = this->find(_key2, &store2, false);
    if (p2 == NULL)
      return false;
    else if (store2 == -1)
      store2 = p2->getNum();
    else if (store2 == 0) {
      p2 = p2->getPrev();
      if (p2 == NULL)
        return false; //no element
      store2 = p2->getNum();
    }
    this->TSM->request(request);
  } else {
    p2 = this->leaves_tail;
    store2 = p2->getNum();
  }

  Node* p = p1;
  unsigned i, l, r;
  //get the num of answers first, not need to prepare the node
  unsigned ansNum = 0;
  while (true) {
    //request = 0;
    //this->prepare(p);
    if (p == p1)
      l = store1;
    else
      l = 0;
    if (p == p2)
      r = store2;
    else
      r = p->getNum();
    ansNum += (r - l);
    //this->TSM->request(request);
    if (p != p2)
      p = p->getNext();
    else
      break;
  }

  if (this->stream != NULL) {
    delete this->stream;
    this->stream = NULL;
  }
  vector<int> keys;
  vector<bool> desc;
  this->stream = new Stream(keys, desc, ansNum, 1, false);

  p = p1;
  while (1) {
    request = 0;
    this->prepare(p);
    if (p == p1)
      l = store1;
    else
      l = 0;
    if (p == p2)
      r = store2;
    else
      r = p->getNum();
    for (i = l; i < r; ++i) {
      //NOTICE:Bstr* in an array, used as Bstr[]
      this->stream->write(p->getValue(i));
    }
    this->TSM->request(request);
    if (p != p2)
      p = p->getNext();
    else
      break;
  }
  this->stream->setEnd();
  return true;
}

bool
Tree::save() //save the whole tree to disk
{
#ifdef DEBUG_KVSTORE
  printf("now to save tree!\n");
#endif
  if (TSM->writeTree(this->root))
    return true;
  else
    return false;
}

void
Tree::release(Node* _np) const
{
  if (_np == NULL)
    return;
  if (_np->isLeaf()) {
    delete _np;
    return;
  }
  int cnt = _np->getNum();
  for (; cnt >= 0; --cnt)
    release(_np->getChild(cnt));
  delete _np;
}

Tree::~Tree()
{
  delete this->stream; //maybe NULL
  delete TSM;
#ifdef DEBUG_KVSTORE
  printf("already empty the buffer, now to delete all nodes in tree!\n");
#endif
  //recursively delete each Node
  release(root);
}

void
Tree::print(string s)
{
#ifdef DEBUG_KVSTORE
  fputs(Util::showtime().c_str(), Util::debug_kvstore);
  fputs("Class Tree\n", Util::debug_kvstore);
  fputs("Message: ", Util::debug_kvstore);
  fputs(s.c_str(), Util::debug_kvstore);
  fputs("\n", Util::debug_kvstore);
  fprintf(Util::debug_kvstore, "Height: %d\n", this->height);
  if (s == "tree" || s == "TREE") {
    if (this->root == NULL) {
      fputs("Null Tree\n", Util::debug_kvstore);
      return;
    }
    Node** ns = new Node* [this->height];
    int* ni = new int[this->height];
    Node* np;
    int i, pos = 0;
    ns[pos] = this->root;
    ni[pos] = this->root->getNum();
    pos++;
    while (pos > 0) {
      np = ns[pos - 1];
      i = ni[pos - 1];
      this->prepare(np);
      if (np->isLeaf() || i < 0) //LeafNode or ready IntlNode
      {                          //child-num ranges: 0~num
        if (s == "tree")
          np->print("node");
        else
          np->print("NODE"); //print full node-information
        pos--;
        continue;
      } else {
        ns[pos] = np->getChild(i);
        ni[pos - 1]--;
        ni[pos] = ns[pos]->getNum();
        pos++;
      }
    }
    delete[] ns;
    delete[] ni;
  } else if (s == "LEAVES" || s == "leaves") {
    Node* np;
    for (np = this->leaves_head; np != NULL; np = np->getNext()) {
      this->prepare(np);
      if (s == "leaves")
        np->print("node");
      else
        np->print("NODE");
    }
  } else if (s == "check tree") {
    //check the tree, if satisfy B+ definition
    //TODO
  } else
    ;
#endif
}
