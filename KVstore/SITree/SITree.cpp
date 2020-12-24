/*=============================================================================
# Filename: SITree.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:45
# Description: achieve functions in SITree.h
=============================================================================*/

#include "SITree.h"

using namespace std;

SITree::SITree()
{
  height = 0;
  mode = "";
  root = NULL;
  leaves_head = NULL;
  leaves_tail = NULL;
  TSM = NULL;
  storepath = "";
  filename = "";
  //transfer_size[0] = transfer_size[1] = transfer_size[2] = 0;
  this->request = 0;
}

SITree::SITree(string _storepath, string _filename, string _mode, unsigned long long _buffer_size)
{
  storepath = _storepath;
  filename = _filename;
  this->height = 0;
  this->mode = string(_mode);
  string filepath = this->getFilePath();
  TSM = new SIStorage(filepath, this->mode, &this->height, _buffer_size);
  if (this->mode == "open") {
    this->TSM->preRead(this->root, this->leaves_head, this->leaves_tail);
    //this->TSM->fullLoad(this->root);
  } else
    this->root = NULL;
  //this->transfer[0].setStr((char*)malloc(Util::TRANSFER_SIZE));
  //this->transfer[1].setStr((char*)malloc(Util::TRANSFER_SIZE));
  //this->transfer[2].setStr((char*)malloc(Util::TRANSFER_SIZE));
  //this->transfer_size[0] = this->transfer_size[1] = this->transfer_size[2] = Util::TRANSFER_SIZE;		//initialied to 1M
  this->request = 0;
}

string
SITree::getFilePath()
{
  return storepath + "/" + filename;
}

//void			//WARN: not check _str and _len
//SITree::CopyToTransfer(const char* _str, unsigned _len, unsigned _index)
//{
//if (_index > 2)
//return;
//[>
//if(_str == NULL || _len == 0)
//{
//printf("error in CopyToTransfer: empty string\n");
//return;
//}
//*/
////unsigned length = _bstr->getLen();
//unsigned length = _len;
//if (length + 1 > this->transfer_size[_index])
//{
//transfer[_index].release();
//transfer[_index].setStr((char*)malloc(length + 1));
//this->transfer_size[_index] = length + 1;	//one more byte: convenient to add \0
//}
//memcpy(this->transfer[_index].getStr(), _str, length);
//this->transfer[_index].getStr()[length] = '\0';	//set for string() in KVstore
//this->transfer[_index].setLen(length);
//}

unsigned
SITree::getHeight() const
{
  return this->height;
}

void
SITree::setHeight(unsigned _h)
{

  this->height = _h;
}

SINode*
SITree::getRoot() const
{
  return this->root;
}

void
SITree::prepare(SINode* _np)
{
  //this->request = 0;
  bool flag = _np->inMem();
  if (!flag)
    this->TSM->readNode(_np, &request); //readNode deal with request
}

bool
SITree::search(const char* _str, unsigned _len, unsigned* _val)
{
  this->AccessLock.lock();
  if (_str == NULL || _len == 0) {
    printf("error in SITree-search: empty string\n");
    //*_val = -1;
    this->AccessLock.unlock();
    return false;
  }
  //this->CopyToTransfer(_str, _len, 1);

  request = 0;
  //Bstr bstr = this->transfer[1];	//not to modify its memory
  //Bstr bstr(_str, _len, true);
  int store;
  SINode* ret = this->find(_str, _len, &store, false);
  if (ret == NULL || store == -1) //tree is empty or not found
  {
    //bstr.clear();
    this->AccessLock.unlock();
    return false;
  }
  const Bstr* tmp = ret->getKey(store);
  if (Util::compare(_str, _len, tmp->getStr(), tmp->getLen()) != 0) //tree is empty or not found
  {
    this->AccessLock.unlock();
    return false;
  }
  *_val = ret->getValue(store);
  this->TSM->request(request);

  //bstr.clear();
  this->AccessLock.unlock();
  return true;
}

bool
SITree::insert(char* _str, unsigned _len, unsigned _val)
{
  this->AccessLock.lock();
  if (_str == NULL || _len == 0) {
    printf("error in SITree-insert: empty string\n");
    this->AccessLock.unlock();
    return false;
  }
  //this->CopyToTransfer(_str, _len, 1);

  this->request = 0;
  SINode* ret;
  if (this->root == NULL) //tree is empty
  {
    leaves_tail = leaves_head = root = new SILeafNode;
    request += SINode::LEAF_SIZE;
    this->height = 1;
    root->setHeight(1); //add to heap later
  }

  //this->prepare(this->root); //root must be in-mem
  if (root->getNum() == SINode::MAX_KEY_NUM) {
    SINode* father = new SIIntlNode;
    request += SINode::INTL_SIZE;
    father->addChild(root, 0);
    ret = root->split(father, 0);
    if (ret->isLeaf() && ret->getNext() == NULL)
      this->leaves_tail = ret;
    if (ret->isLeaf())
      request += SINode::LEAF_SIZE;
    else
      request += SINode::INTL_SIZE;
    this->height++; //height rises only when root splits
    //WARN: height area in SINode: 4 bit!
    father->setHeight(this->height); //add to heap later
    this->TSM->updateHeap(ret, ret->getRank(), false);
    this->root = father;
  }

  SINode* p = this->root;
  SINode* q;
  int i;
  //const Bstr* _key = &transfer[1];
  //Bstr bstr = *_key;
  while (!p->isLeaf()) {
    //j = p->getNum();
    //for(i = 0; i < j; ++i)
    //if(bstr < *(p->getKey(i)))
    //break;
    //NOTICE: using binary search is better here
    i = p->searchKey_less(_str, _len);

    q = p->getChild(i);
    this->prepare(q);
    if (q->getNum() == SINode::MAX_KEY_NUM) {
      ret = q->split(p, i);
      if (ret->isLeaf() && ret->getNext() == NULL)
        this->leaves_tail = ret;
      if (ret->isLeaf())
        request += SINode::LEAF_SIZE;
      else
        request += SINode::INTL_SIZE;
      //BETTER: in loop may update multiple times
      this->TSM->updateHeap(ret, ret->getRank(), false);
      this->TSM->updateHeap(q, q->getRank(), true);
      this->TSM->updateHeap(p, p->getRank(), true);
      //if (bstr < *(p->getKey(i)))
      const Bstr* tmp = p->getKey(i);
      int cmp_res = Util::compare(_str, _len, tmp->getStr(), tmp->getLen());
      if (cmp_res < 0)
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
  i = p->searchKey_less(_str, _len);

  //insert existing key is ok, but not inserted in
  //however, the tree-shape may change due to possible split in former code
  bool ifexist = false;
  //if (i > 0 && bstr == *(p->getKey(i - 1)))
  if (i > 0) {
    const Bstr* tmp = p->getKey(i - 1);
    int cmp_res = Util::compare(_str, _len, tmp->getStr(), tmp->getLen());
    if (cmp_res == 0) {
      ifexist = true;
    }
  }

  if (!ifexist) {
    p->addKey(_str, _len, i, true);
    p->addValue(_val, i);
    p->addNum();
    request += _len;
    p->setDirty();
    this->TSM->updateHeap(p, p->getRank(), true);
  }

  this->TSM->request(request);
  //bstr.clear();		//NOTICE: must be cleared!
  this->AccessLock.unlock();
  return !ifexist; //QUERY(which case:return false)
}

bool
SITree::modify(const char* _str, unsigned _len, unsigned _val)
{
  this->AccessLock.lock();
  if (_str == NULL || _len == 0) {
    printf("error in SITree-modify: empty string\n");
    this->AccessLock.unlock();
    return false;
  }
  //this->CopyToTransfer(_str, _len, 1);

  this->request = 0;
  //const Bstr* _key = &transfer[1];
  //Bstr bstr = *_key;
  int store;
  SINode* ret = this->find(_str, _len, &store, true);
  if (ret == NULL || store == -1) //tree is empty or not found
  {
    //bstr.clear();
    this->AccessLock.unlock();
    return false;
  }
  const Bstr* tmp = ret->getKey(store);
  if (Util::compare(_str, _len, tmp->getStr(), tmp->getLen()) != 0) //tree is empty or not found
  {
    this->AccessLock.unlock();
    return false;
  }

  ret->setValue(_val, store);
  ret->setDirty();
  this->TSM->request(request);
  //bstr.clear();
  this->AccessLock.unlock();
  return true;
}

//this function is useful for search and modify, and range-query
SINode* //return the first key's position that >= *_key
    SITree::find(const char* _str, unsigned _len, int* _store, bool ifmodify)
{ //to assign value for this->bstr, function shouldn't be const!
  if (this->root == NULL)
    return NULL; //SITree Is Empty

  SINode* p = root;
  int i, j;
  //Bstr bstr = *_key;					//local Bstr: multiple delete
  while (!p->isLeaf()) {
    if (ifmodify)
      p->setDirty();
    //j = p->getNum();
    //for(i = 0; i < j; ++i)				//BETTER(Binary-Search)
    //if(bstr < *(p->getKey(i)))
    //break;
    i = p->searchKey_less(_str, _len);

    p = p->getChild(i);
    this->prepare(p);
  }

  j = p->getNum();
  //for(i = 0; i < j; ++i)
  //if(bstr <= *(p->getKey(i)))
  //break;
  i = p->searchKey_lessEqual(_str, _len);

  if (i == j)
    *_store = -1; //Not Found
  else
    *_store = i;

  //bstr.clear();

  return p;
}

/*
SINode*
SITree::find(unsigned _len, const char* _str, int* store) const
{
}
*/

bool
SITree::remove(const char* _str, unsigned _len)
{
  this->AccessLock.lock();
  if (_str == NULL || _len == 0) {
    printf("error in SITree-remove: empty string\n");
    this->AccessLock.unlock();
    return false;
  }
  //this->CopyToTransfer(_str, _len, 1);

  request = 0;
  //const Bstr* _key = &transfer[1];
  SINode* ret;
  if (this->root == NULL) //tree is empty
  {
    this->AccessLock.unlock();
    return false;
  }
  SINode* p = this->root;
  SINode* q;
  int i, j;
  //Bstr bstr = *_key;
  while (!p->isLeaf()) {
    j = p->getNum();
    //for(i = 0; i < j; ++i)
    //if(bstr < *(p->getKey(i)))
    //break;
    i = p->searchKey_less(_str, _len);

    q = p->getChild(i);
    this->prepare(q);
    if (q->getNum() < SINode::MIN_CHILD_NUM) //==MIN_KEY_NUM
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
  i = p->searchKey_equal(_str, _len);
  //WARN+NOTICE:here must check, because the key to remove maybe not exist
  if (i != (int)p->getNum()) {
    request -= p->getKey(i)->getLen();
    p->subKey(i, true); //to release
    p->subValue(i);     //to release
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
  }

  this->TSM->request(request);
  //bstr.clear();
  this->AccessLock.unlock();
  return flag; //i == j, not found
}

bool
SITree::save() //save the whole tree to disk
{
  this->AccessLock.lock();
#ifdef DEBUG_KVSTORE
  printf("now to save tree!\n");
#endif
  if (TSM->writeTree(this->root)) {
    this->AccessLock.unlock();
    return true;
  } else {
    this->AccessLock.unlock();
    return false;
  }
}

void
SITree::release(SINode* _np) const
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

SITree::~SITree()
{
  //cout << "SITree" << endl;
  //cout << "delete TSM" << endl;
  delete TSM;
  TSM = NULL;
#ifdef DEBUG_KVSTORE
//printf("already empty the buffer, now to delete all nodes in tree!\n");
#endif
  //cout << "release" << endl;
  //recursively delete each SINode
  release(root);
  //cout << "~SITree done" << endl;
}

void
SITree::print(string s)
{
#ifdef DEBUG_KVSTORE
  fputs(Util::showtime().c_str(), Util::debug_kvstore);
  fputs("Class SITree\n", Util::debug_kvstore);
  fputs("Message: ", Util::debug_kvstore);
  fputs(s.c_str(), Util::debug_kvstore);
  fputs("\n", Util::debug_kvstore);
  fprintf(Util::debug_kvstore, "Height: %d\n", this->height);
  if (s == "tree" || s == "TREE") {
    if (this->root == NULL) {
      fputs("Null SITree\n", Util::debug_kvstore);
      return;
    }
    SINode** ns = new SINode* [this->height];
    int* ni = new int[this->height];
    SINode* np;
    int i, pos = 0;
    ns[pos] = this->root;
    ni[pos] = this->root->getNum();
    pos++;
    while (pos > 0) {
      np = ns[pos - 1];
      i = ni[pos - 1];
      this->prepare(np);
      if (np->isLeaf() || i < 0) //LeafSINode or ready IntlNode
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
    SINode* np;
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
