/*=============================================================================
# Filename: IVStorage.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:43
# Description: achieve functions in IVStorage.h
=============================================================================*/

#include "IVStorage.h"

using namespace std;

IVStorage::IVStorage()
{ //not use ../logs/, notice the location of program
  cur_block_num = SET_BLOCK_NUM;
  filepath = "";
  freelist = NULL;
  treefp = NULL;
  max_buffer_size = Util::MAX_BUFFER_SIZE;
  heap_size = max_buffer_size / IVNode::INTL_SIZE;
  freemem = max_buffer_size;
  minheap = NULL;
  this->value_list = NULL;
}

IVStorage::IVStorage(string& _filepath, string& _mode, unsigned* _height, unsigned long long _buffer_size, VList* _vlist)
{
  updateHeapTime = 0;
  cur_block_num = SET_BLOCK_NUM; //initialize
  this->filepath = _filepath;
  if (_mode == string("build"))
    treefp = fopen(_filepath.c_str(), "w+b");
  else if (_mode == string("open"))
    treefp = fopen(_filepath.c_str(), "r+b");
  else {
    print(string("error in IVStorage: Invalid mode ") + _mode);
    return;
  }
  if (treefp == NULL) {
    print(string("error in IVStorage: Open error ") + _filepath);
    return;
  }
  this->treeheight = _height; //originally set to 0
  this->max_buffer_size = _buffer_size;
  this->heap_size = this->max_buffer_size / IVNode::INTL_SIZE;
  this->freemem = this->max_buffer_size;
  this->freelist = new BlockInfo; //null-head
  unsigned i, j, k;               //j = (SuperNum-1)*BLOCK_SIZE
  BlockInfo* bp;
  if (_mode == "build") { //write basic information
    i = 0;
    fwrite(&i, sizeof(unsigned), 1, this->treefp);             //height
    fwrite(&i, sizeof(unsigned), 1, this->treefp);             //rootnum
    fwrite(&cur_block_num, sizeof(unsigned), 1, this->treefp); //current block num
    fseek(this->treefp, BLOCK_SIZE, SEEK_SET);
    bp = this->freelist;
    j = cur_block_num / 8;
    for (i = 0; i < j; ++i) {
      fputc(0, this->treefp);
      for (k = 0; k < 8; ++k) {
        bp->next = new BlockInfo(i * 8 + k + 1, NULL);
        bp = bp->next;
      }
    }
  } else //_mode == "open"
  {
    //read basic information
    unsigned rootnum;
    char c;
    fread(this->treeheight, sizeof(unsigned), 1, this->treefp);
    fread(&rootnum, sizeof(unsigned), 1, this->treefp);
    fread(&cur_block_num, sizeof(unsigned), 1, this->treefp);
    fseek(this->treefp, BLOCK_SIZE, SEEK_SET);
    bp = this->freelist;
    j = cur_block_num / 8;
    for (i = 0; i < j; ++i) {
      c = fgetc(treefp);
      for (k = 0; k < 8; ++k) {
        if ((c & (1 << k)) == 0) {
          bp->next = new BlockInfo(i * 8 + 7 - k + 1, NULL);
          bp = bp->next;
        }
      }
    }
    fseek(treefp, Address(rootnum), SEEK_SET);
    //treefp is now ahead of root-block
  }

  this->minheap = new IVHeap(this->heap_size);
  this->value_list = _vlist;
}

//preRead only put root, leaves_head and leaves_tail in memmory
bool
IVStorage::preRead(IVNode*& _root, IVNode*& _leaves_head, IVNode*& _leaves_tail) //pre-read and build whole tree
{                                                                                //set root(in memory) and leaves_head
  //TODO: false when exceed memory
  _leaves_tail = _leaves_head = _root = NULL;
  if (ftell(this->treefp) == 0) //root is null
  {
    return true;
  }
  unsigned next, store, j, pos = 0;
  unsigned h = *this->treeheight;
  IVNode* p;
  //read root node
  this->createNode(p);
  _root = p;
  fread(&next, sizeof(unsigned), 1, treefp);
  //use stack to achieve
  long address[h];   //current address
  unsigned used[h];  //used child num
  unsigned total[h]; //total child num
  unsigned block[h]; //next block num
  IVNode* nodes[h];
  address[pos] = ftell(treefp);
  used[pos] = 0;
  total[pos] = p->getNum() + 1;
  block[pos] = next;
  nodes[pos] = p;
  pos++;
  IVNode* prev = NULL;
  while (pos > 0) {
    j = pos - 1;
    if (nodes[j]->isLeaf() || used[j] == total[j]) //LeafNode or ready IntlNode
    {
      if (nodes[j]->isLeaf()) {
        if (prev != NULL) {
          prev->setNext(nodes[j]);
          nodes[j]->setPrev(prev);
        }
        prev = nodes[j];
      }
      pos--;
      continue;
    }
    fseek(this->treefp, address[j], SEEK_SET);
    fread(&store, sizeof(unsigned), 1, treefp);
    this->ReadAlign(block + j);
    address[j] = ftell(treefp);
    fseek(treefp, Address(store), SEEK_SET);
    this->createNode(p);
    nodes[j]->setChild(p, used[j]);
    used[j]++;
    fread(&next, sizeof(unsigned), 1, treefp);
    address[pos] = ftell(treefp);
    used[pos] = 0;
    total[pos] = p->getNum() + 1;
    block[pos] = next;
    nodes[pos] = p;
    pos++;
  }
  //set leaves and read root, which is always keeped in-mem
  p = _root;
  while (!p->isLeaf()) {
    p = p->getChild(0);
  }
  _leaves_head = p;
  p = _root;
  while (!p->isLeaf()) {
    p = p->getChild(p->getNum());
  }
  _leaves_tail = p;
  long long memory = 0;
  this->readNode(_root, &memory);
  this->request(memory);
  return true;
}

bool
IVStorage::preLoad(IVNode*& _root)
{
  if (_root == NULL) {
    return false;
  }
  // preload nodes of tree till storage is half full
  bool mem_full = false;
  std::queue<IVNode*> node_q;
  IVNode* p = _root;
  while (!p->isLeaf()) {
    IVNode* tmp;
    unsigned KN = p->getNum();
    for (unsigned i = 0; i <= KN; ++i) {
      tmp = p->getChild(i);
      long long memory = 0;
      this->readNode(tmp, &memory);
      this->request(memory);
      if (this->freemem < this->max_buffer_size / 2) {
        mem_full = true;
        break;
      }
      node_q.push(tmp);
    }
    if (mem_full)
      break;
    p = node_q.front();
    node_q.pop();
  }
  while (!node_q.empty()) {
    node_q.pop();
  }
  return true;
}

bool
IVStorage::fullLoad(IVNode*& _root)
{
  if (_root == NULL) {
    return false;
  }
  std::queue<IVNode*> node_q;
  IVNode* p = _root;
  while (!p->isLeaf()) {
    IVNode* tmp;
    unsigned KN = p->getNum();
    for (unsigned i = 0; i <= KN; ++i) {
      tmp = p->getChild(i);
      long long memory = 0;
      this->readNode(tmp, &memory);
      //this->request(memory);
      node_q.push(tmp);
    }
    p = node_q.front();
    node_q.pop();
  }
  while (!node_q.empty()) {
    node_q.pop();
  }
  return true;
}

long                                             //8-byte in 64-bit machine
    IVStorage::Address(unsigned _blocknum) const //BETTER: inline function
{
  if (_blocknum == 0)
    return 0;
  else if (_blocknum > cur_block_num) {
    //print(string("error in Address: Invalid blocknum ") + Util::int2string(_blocknum));
    return -1; //address should be non-negative
  }
  //NOTICE: here should explictly use long
  return (long)(this->SuperNum + _blocknum - 1) * (long)BLOCK_SIZE;
}

unsigned
IVStorage::Blocknum(long address) const
{
  return (address / BLOCK_SIZE) + 1 - this->SuperNum;
}

unsigned
IVStorage::AllocBlock()
{
  BlockInfo* p = this->freelist->next;
  if (p == NULL) {
    for (unsigned i = 0; i < SET_BLOCK_INC; ++i) {
      cur_block_num++; //BETTER: check if > MAX_BLOCK_NUM
      this->FreeBlock(cur_block_num);
    }
    p = this->freelist->next;
  }
  unsigned t = p->num;
  this->freelist->next = p->next;
  delete p;
  return t;
}

void
IVStorage::FreeBlock(unsigned _blocknum)
{ //QUERY: head-sub and tail-add will be better?
  BlockInfo* bp = new BlockInfo(_blocknum, this->freelist->next);
  this->freelist->next = bp;
}

//NOTICE: all reads are aligned to 4 bytes(including a string)
//a string may acrossseveral blocks

void
IVStorage::ReadAlign(unsigned* _next)
{
  if (ftell(treefp) % BLOCK_SIZE == 0) {
    fseek(treefp, Address(*_next), SEEK_SET);
    fread(_next, sizeof(unsigned), 1, treefp);
  }
}

void
IVStorage::WriteAlign(unsigned* _curnum, bool& _SpecialBlock)
{
  if (ftell(treefp) % BLOCK_SIZE == 0) {
    unsigned blocknum = this->AllocBlock();
    fseek(treefp, Address(*_curnum), SEEK_SET);
    if (_SpecialBlock) {
      fseek(treefp, 4, SEEK_CUR);
      _SpecialBlock = false;
    }
    fwrite(&blocknum, sizeof(unsigned), 1, treefp);
    fseek(treefp, Address(blocknum) + 4, SEEK_SET);
    *_curnum = blocknum;
  }
}

bool
IVStorage::readNode(IVNode* _np, long long* _request)
{
  if (_np == NULL || _np->inMem())
    return false; //can't read or needn't

  fseek(treefp, Address(_np->getStore()), SEEK_SET);
  bool flag = _np->isLeaf();
  unsigned next;
  unsigned i, num = _np->getNum();
  Bstr bstr;
  fseek(treefp, 4, SEEK_CUR);
  fread(&next, sizeof(unsigned), 1, treefp);

  //read data, use readBstr...
  //fread(treefp, "%u", &num);
  //_np->setNum(num);
  if (flag)
    *_request += IVNode::LEAF_SIZE;
  else
    *_request += IVNode::INTL_SIZE;
  _np->Normal();
  if (!flag)
    fseek(treefp, 4 * (num + 1), SEEK_CUR);

  //to read all keys
  //int tmp = -1;
  unsigned tmp = INVALID;
  for (i = 0; i < num; ++i) {
    fread(&tmp, sizeof(int), 1, treefp);
    this->ReadAlign(&next);
    _np->setKey(tmp, i);
  }

  if (flag) {
    //to read all values
    for (i = 0; i < num; ++i) {
      this->readBstr(&bstr, &next);
      //if not long list value
      if (bstr.getStr() != NULL) {
        *_request += bstr.getLen();
      }
      _np->setValue(&bstr, i);
    }
  }
  //_np->setFlag((_np->getFlag() & ~Node::NF_IV & ~Node::NF_ID) | Node::NF_IM);
  //_np->delVirtual();
  _np->delDirty();
  //_np->setMem();
  this->updateHeap(_np, _np->getRank(), false);
  bstr.clear();
  return true;
}

bool
IVStorage::createNode(IVNode*& _np) //cretae virtual nodes, not in-mem
{
  /*
	if(ftell(this->treefp)== 0)	//null root
	{
	_np = NULL;
	return false;
	}
	*/
  unsigned t;        //QUERY: maybe next-flag... will be better-storage?
  bool flag = false; //IntlNode
  fread(&t, sizeof(unsigned), 1, treefp);
  if ((t & IVNode::NF_IL) > 0) //WARN: according to setting
    flag = true;               //LeafNode
  if (flag) {
    //this->request(sizeof(LeafNode));
    _np = new IVLeafNode(true);
  } else {
    //this->request(sizeof(IntlNode));
    _np = new IVIntlNode(true);
  }
  //fseek(treefp, -4, SEEK_CUR);
  //_np->setFlag(_np->getFlag() | (t & Node::NF_RK));
  //_np->setRank(t);
  _np->setFlag(t);
  _np->delDirty();
  _np->delMem();
  _np->setStore(Blocknum(ftell(treefp) - 4));
  return true;
}

//BETTER: Does SpecialBlock really needed? why can't we place next before flag??
//
//NOTICE: root num begins from 1, if root num is 0, then it is invalid, i.e. the tree is NULL
//(and ftell(root address) will be 0 either)

bool
IVStorage::writeNode(IVNode* _np)
{
  if (_np == NULL || !_np->inMem() || (_np->getRank() > 0 && !_np->isDirty()))
    return false; //not need to write back

  unsigned num = _np->getNum(), i;
  bool flag = _np->isLeaf(), SpecialBlock = true;
  /*
	if(!flag)
	{
	for(i = 0; i <= num; ++i)
	if(_np->getChild(i)->isDirty())
	return false;	//NOTICE: all childs must be clean!
	}
	*/
  //to release original blocks
  unsigned store = _np->getStore(), next;
  //if first store is 0, meaning a new node
  fseek(this->treefp, Address(store) + 4, SEEK_SET);
  fread(&next, sizeof(unsigned), 1, treefp);
  while (store != 0) {
    this->FreeBlock(store);
    store = next;
    fseek(treefp, Address(store), SEEK_SET);
    fread(&next, sizeof(unsigned), 1, treefp);
  }
  if (num == 0)
    return true; //node is empty!
  unsigned t;
  //write Node information
  unsigned blocknum = this->AllocBlock();
  _np->setStore(blocknum);
  long address = this->Address(blocknum);
  fseek(this->treefp, address, SEEK_SET);
  t = _np->getFlag();
  fwrite(&t, sizeof(unsigned), 1, treefp); //DEBUG
  fseek(treefp, 4, SEEK_CUR);
  if (!flag) {
    for (i = 0; i <= num; ++i) {
      t = _np->getChild(i)->getStore();
      fwrite(&t, sizeof(unsigned), 1, treefp); //DEBUG
      this->WriteAlign(&blocknum, SpecialBlock);
    }
  }

  //int tmp = 0;
  unsigned tmp = INVALID;
  //to write all keys
  for (i = 0; i < num; ++i) {
    tmp = _np->getKey(i);
    fwrite(&tmp, sizeof(int), 1, treefp);
    this->WriteAlign(&blocknum, SpecialBlock);
  }

  if (flag) {
    //to write all values
    for (i = 0; i < num; ++i) {
      this->writeBstr(_np->getValue(i), &blocknum, SpecialBlock);
    }
  }
  fseek(treefp, Address(blocknum), SEEK_SET);
  if (SpecialBlock)
    fseek(treefp, 4, SEEK_CUR);
  t = 0;
  fwrite(&t, sizeof(unsigned), 1, treefp); //the end-block
                                           //_np->setFlag(_np->getFlag() & ~Node::NF_ID);
  //NOTICE:we may store the dirty bit into the tree file, but that is ok
  //Each time we read the tree file to construct a node, we always set the drity bit to 0
  _np->delDirty();
  return true;
}

bool
IVStorage::readBstr(Bstr* _bp, unsigned* _next)
{
  //long address;
  unsigned len, i, j;
  fread(&len, sizeof(unsigned), 1, this->treefp);
  this->ReadAlign(_next);

  //NOTICE: if this is a long list as value
  if (len == 0) {
    unsigned addr = 0;
    fread(&addr, sizeof(unsigned), 1, this->treefp);
#ifdef DEBUG_VLIST
    cout << "read a vlist in IVStorage - addr: " << addr << endl;
#endif
    _bp->setLen(addr);
    _bp->setStr(NULL);
    this->ReadAlign(_next);
    return true;
  }

  //this->request(len);

  //NOTICE: we use new for all, consistent with Bstr and KVstore
  //char* s = (char*)malloc(len);
  char* s = new char[len];
  _bp->setLen(len);
  for (i = 0; i + 4 < len; i += 4) {
    fread(s + i, sizeof(char), 4, treefp);
    this->ReadAlign(_next);
  }
  while (i < len) {
    fread(s + i, sizeof(char), 1, treefp); //BETTER
    i++;
  }
  j = len % 4;
  if (j > 0)
    j = 4 - j;
  fseek(treefp, j, SEEK_CUR);
  this->ReadAlign(_next);
  _bp->setStr(s);

  return true;
}

bool
IVStorage::writeBstr(const Bstr* _bp, unsigned* _curnum, bool& _SpecialBlock)
{
  unsigned i, j, len = _bp->getLen();

  //NOTICE: to write long list value
  if (_bp->getStr() == NULL) {
    unsigned flag = 0;
    fwrite(&flag, sizeof(unsigned), 1, treefp);
    this->WriteAlign(_curnum, _SpecialBlock);
    //then this is the real block num
    fwrite(&len, sizeof(unsigned), 1, treefp);
#ifdef DEBUG_VLIST
    cout << "to write a vlist in IVStorage::writeBstr() - blocknum: " << len << endl;
#endif
    this->WriteAlign(_curnum, _SpecialBlock);
    return true;
  }

  fwrite(&len, sizeof(unsigned), 1, treefp);
  this->WriteAlign(_curnum, _SpecialBlock);

  char* s = _bp->getStr();
  for (i = 0; i + 4 < len; i += 4) {
    fwrite(s + i, sizeof(char), 4, treefp);
    this->WriteAlign(_curnum, _SpecialBlock);
  }
  while (i < len) {
    fwrite(s + i, sizeof(char), 1, treefp);
    i++;
  }

  j = len % 4;
  if (j > 0)
    j = 4 - j;
  fseek(treefp, j, SEEK_CUR);
  this->WriteAlign(_curnum, _SpecialBlock);

  return true;
}

bool
IVStorage::writeTree(IVNode* _root) //write the whole tree back and close treefp
{
  fseek(this->treefp, 0, SEEK_SET);
  fwrite(this->treeheight, sizeof(unsigned), 1, treefp);
  //delete all nonsense-node in heap, otherwise will waste storage permanently
  IVNode* p;
  while (1) { //all non-sense nodes will be in-head-area, due to minimal rank
    p = minheap->getTop();
    if (p == NULL) //heap is empty, only when root==NULL
      break;
    if (p->getRank() == 0) //indicate non-sense node
    {
      this->minheap->remove();
      this->writeNode(p);
      delete p;
    } else
      break;
  }

  unsigned i, j, t;
  //QUERY: another way to write all nodes back is to print out all nodes in heap
  //but this method will cause no node in heap any more, while operations may be
  //afetr tree-saving.	Which method is better?
  //write nodes recursively using stack, including root-num
  if (_root != NULL) {
    IVNode* p = _root;
    unsigned h = *this->treeheight, pos = 0;
    IVNode* ns[h];
    int ni[h];
    ns[pos] = p;
    ni[pos] = p->getNum();
    pos++;
    while (pos > 0) {
      j = pos - 1;
      p = ns[j];
      if (p->isLeaf() || ni[j] < 0) //leaf or all childs are ready
      {
        this->writeNode(p);
        pos--;
        continue;
      }
      ns[pos] = p->getChild(ni[j]);
      ni[pos] = ns[pos]->getNum();
      pos++;
      ni[j]--;
    }
    t = _root->getStore();
  } else
    t = 0;

  fseek(this->treefp, 4, SEEK_SET);
  fwrite(&t, sizeof(unsigned), 1, treefp);             //write the root num
  fwrite(&cur_block_num, sizeof(unsigned), 1, treefp); //write current blocks num
  fseek(treefp, BLOCK_SIZE, SEEK_SET);
  j = cur_block_num / 8; //(SuperNum-1)*BLOCK_SIZE;
                         //reset to 1 first
  for (i = 0; i < j; ++i) {
    fputc(0xff, treefp);
  }
  char c;
  BlockInfo* bp = this->freelist->next;
  while (bp != NULL) {
//if not-use then set 0, aligned to byte!
#ifdef DEBUG_KVSTORE
    if (bp->num > cur_block_num) {
      printf("blocks num exceed, cur_block_num: %u\n", cur_block_num);
      exit(1);
    }
#endif
    j = bp->num - 1;
    i = j / 8;
    j = 7 - j % 8;
    fseek(treefp, BLOCK_SIZE + i, SEEK_SET);
    c = fgetc(treefp);
    fseek(treefp, -1, SEEK_CUR);
    fputc(c & ~(1 << j), treefp);
    bp = bp->next;
  }

  Util::Csync(this->treefp);
  //fclose(this->treefp);

  return true;
}

void
IVStorage::updateHeap(IVNode* _np, unsigned _rank, bool _inheap) const
{
  //long t1 = Util::get_cur_time();
  if (_inheap) //already in heap, to modify
  {
    unsigned t = _np->getRank();
    _np->setRank(_rank);
    if (t < _rank)
      this->minheap->modify(_np, false);
    else if (t > _rank)
      this->minheap->modify(_np, true);
    else
      ;
  } else //not in heap, to add
  {
    _np->setRank(_rank);
    this->minheap->insert(_np);
  }
  //long t2 = Util::get_cur_time();
  //updateHeapTime += (t2 - t1);
}

bool
IVStorage::request(long long _needmem) //aligned to byte
{                                      //NOTICE: <0 means release
  //cout<<"freemem: "<<this->freemem<<" needmem: "<<_needmem<<endl;
  if (_needmem > 0 && this->freemem < (unsigned long long)_needmem)
    if (!this->handler(_needmem - freemem)) //disaster in buffer memory
    {
      print(string("error in request: out of buffer-mem, now to exit"));
      //exit(1);
      return false;
    }
  this->freemem -= _needmem;
  return true;
}

bool
IVStorage::handler(unsigned long long _needmem) //>0
{
  cout << "swap happen" << endl;
  IVNode* p;
  unsigned long long size;
  //if(_needmem < SET_BUFFER_SIZE)		//to recover to SET_BUFFER_SIZE buffer
  //	_needmem = SET_BUFFER_SIZE;
  //cout<<"IVStorage::handler() - now to loop to release nodes"<<endl;
  while (1) {
    p = this->minheap->getTop();
    //cout<<"get heap top"<<endl;
    if (p == NULL) {
      cout << "the heap top is null" << endl;
      return false; //can't satisfy or can't recover to SET_BUFFER_SIZE
    }

    this->minheap->remove();
    //cout<<"node removed in heap"<<endl;
    size = p->getSize();
    this->freemem += size;
    this->writeNode(p);
    //cout<<"node write back"<<endl;
    if (p->getNum() > 0)
      p->Virtual();
    else
      delete p; //non-sense node
                //cout<<"node memory released"<<endl;
    if (_needmem > size) {
      //cout<<"reduce the request"<<endl;
      _needmem -= size;
    } else {
      //cout<<"ok to break"<<endl;
      break;
    }
  }
  //cout<<"IVStorage::handler() -- finished"<<endl;
  return true;
}

IVStorage::~IVStorage()
{
//cout << "update heap time is " << updateHeapTime << "ms" << endl;
//release heap and freelist...
#ifdef DEBUG_KVSTORE
  printf("now to release the kvstore!\n");
#endif
  BlockInfo* bp = this->freelist;
  BlockInfo* next;
  while (bp != NULL) {
    next = bp->next;
    delete bp;
    bp = next;
  }
#ifdef DEBUG_KVSTORE
  printf("already empty the freelist!\n");
#endif
  delete this->minheap;
  this->minheap = NULL;
#ifdef DEBUG_KVSTORE
  printf("already empty the buffer heap!\n");
#endif
  fclose(this->treefp);
  //#ifdef DEBUG_KVSTORE
  //NOTICE:there is more than one tree
  //fclose(Util::debug_kvstore);	//NULL is ok!
  //Util::debug_kvstore = NULL;
  //#endif
}

void
IVStorage::print(string s)
{
#ifdef DEBUG_KVSTORE
  fputs(Util::showtime().c_str(), Util::debug_kvstore);
  fputs("Class IVStorage\n", Util::debug_kvstore);
  fputs("Message: ", Util::debug_kvstore);
  fputs(s.c_str(), Util::debug_kvstore);
  fputs("\n", Util::debug_kvstore);
#endif
}
