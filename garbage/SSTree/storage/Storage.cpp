/*=============================================================================
# Filename: Storage.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:43
# Description: achieve functions in Storage.h
=============================================================================*/

#include "Storage.h"

using namespace std;

Storage::Storage()
{ //not use ../logs/, notice the location of program
  cur_block_num = SET_BLOCK_NUM;
  filepath = "";
  freelist = NULL;
  treefp = NULL;
  minheap = NULL;
  freemem = MAX_BUFFER_SIZE;
}

Storage::Storage(string& _filepath, string& _mode, unsigned* _height)
{
  cur_block_num = SET_BLOCK_NUM; //initialize
  this->filepath = _filepath;
  if (_mode == string("build"))
    treefp = fopen(_filepath.c_str(), "w+b");
  else if (_mode == string("open"))
    treefp = fopen(_filepath.c_str(), "r+b");
  else {
    print(string("error in Storage: Invalid mode ") + _mode);
    return;
  }
  if (treefp == NULL) {
    print(string("error in Storage: Open error ") + _filepath);
    return;
  }
  this->treeheight = _height; //originally set to 0
  this->freemem = MAX_BUFFER_SIZE;
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
    int rootnum;
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
  this->minheap = new Heap(HEAP_SIZE);
}

bool
Storage::preRead(Node*& _root, Node*& _leaves_head, Node*& _leaves_tail) //pre-read and build whole tree
{                                                                        //set root(in memory) and leaves_head
  //TODO: false when exceed memory
  _leaves_tail = _leaves_head = _root = NULL;
  if (ftell(this->treefp) == 0) //root is null
  {
    return true;
  }
  unsigned next, store, j, pos = 0;
  unsigned h = *this->treeheight;
  Node* p;
  //read root node
  this->createNode(p);
  _root = p;
  fread(&next, sizeof(unsigned), 1, treefp);
  //use stack to achieve
  long address[h];   //current address
  unsigned used[h];  //used child num
  unsigned total[h]; //total child num
  unsigned block[h]; //next block num
  Node* nodes[h];
  address[pos] = ftell(treefp);
  used[pos] = 0;
  total[pos] = p->getNum() + 1;
  block[pos] = next;
  nodes[pos] = p;
  pos++;
  Node* prev = NULL;
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

long                                           //8-byte in 64-bit machine
    Storage::Address(unsigned _blocknum) const //BETTER: inline function
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
Storage::Blocknum(long address) const
{
  return (address / BLOCK_SIZE) + 1 - this->SuperNum;
}

unsigned
Storage::AllocBlock()
{
  BlockInfo* p = this->freelist->next;
  if (p == NULL) {
    for (unsigned i = 0; i < SET_BLOCK_INC; ++i) {
      this->cur_block_num++;
      this->FreeBlock(this->cur_block_num);
    }
    if (this->cur_block_num >= Storage::MAX_BLOCK_NUM) {
      cerr << "Error in ISStorage::AllocBlock() - cur_block_num overflow" << endl;
    }
    p = this->freelist->next;
  }
  unsigned t = p->num;
  this->freelist->next = p->next;
  delete p;
  return t;
}

void
Storage::FreeBlock(unsigned _blocknum)
{ //QUERY: head-sub and tail-add will be better?
  BlockInfo* bp = new BlockInfo(_blocknum, this->freelist->next);
  this->freelist->next = bp;
  //this->cur_block_num--;	//BETTER: check if reduce to 0
}

void
Storage::ReadAlign(unsigned* _next)
{
  if (ftell(treefp) % BLOCK_SIZE == 0) {
    fseek(treefp, Address(*_next), SEEK_SET);
    fread(_next, sizeof(unsigned), 1, treefp);
  }
}

void
Storage::WriteAlign(unsigned* _curnum, bool& _SpecialBlock)
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
Storage::readNode(Node* _np, long long* _request)
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
    *_request += Node::LEAF_SIZE;
  else
    *_request += Node::INTL_SIZE;
  _np->Normal();
  if (!flag)
    fseek(treefp, 4 * (num + 1), SEEK_CUR);
  for (i = 0; i < num; ++i) {
    this->readBstr(&bstr, &next);
    *_request += bstr.getLen();
    _np->setKey(&bstr, i);
  }
  if (flag) {
    for (i = 0; i < num; ++i) {
      this->readBstr(&bstr, &next);
      *_request += bstr.getLen();
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
Storage::createNode(Node*& _np) //cretae virtual nodes, not in-mem
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
  if ((t & Node::NF_IL) > 0) //WARN: according to setting
    flag = true;             //LeafNode
  if (flag) {
    //this->request(sizeof(LeafNode));
    _np = new LeafNode(true);
  } else {
    //this->request(sizeof(IntlNode));
    _np = new IntlNode(true);
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

bool
Storage::writeNode(Node* _np)
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
  for (i = 0; i < num; ++i)
    this->writeBstr(_np->getKey(i), &blocknum, SpecialBlock);
  if (flag) {
    for (i = 0; i < num; ++i)
      this->writeBstr(_np->getValue(i), &blocknum, SpecialBlock);
  }
  fseek(treefp, Address(blocknum), SEEK_SET);
  if (SpecialBlock)
    fseek(treefp, 4, SEEK_CUR);
  t = 0;
  fwrite(&t, sizeof(unsigned), 1, treefp); //the end-block
  //_np->setFlag(_np->getFlag() & ~Node::NF_ID);
  _np->delDirty();
  return true;
}

bool
Storage::readBstr(Bstr* _bp, unsigned* _next)
{
  //long address;
  unsigned len, i, j;
  fread(&len, sizeof(unsigned), 1, this->treefp);
  this->ReadAlign(_next);
  //this->request(len);
  char* s = (char*)malloc(len);
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
Storage::writeBstr(const Bstr* _bp, unsigned* _curnum, bool& _SpecialBlock)
{
  unsigned i, j, len = _bp->getLen();
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
Storage::writeTree(Node* _root) //write the whole tree back and close treefp
{
  fseek(this->treefp, 0, SEEK_SET);
  fwrite(this->treeheight, sizeof(unsigned), 1, treefp);
  //delete all nonsense-node in heap, otherwise will waste storage permanently
  Node* p;
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
    Node* p = _root;
    unsigned h = *this->treeheight, pos = 0;
    Node* ns[h];
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
  //fclose(this->treefp);
  return true;
}

void
Storage::updateHeap(Node* _np, unsigned _rank, bool _inheap) const
{
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
}

void
Storage::request(long long _needmem) //aligned to byte
{                                    //NOTICE: <0 means release
  if (_needmem > 0 && this->freemem < (unsigned long long)_needmem)
    if (!this->handler(_needmem - freemem)) //disaster in buffer memory
    {
      print(string("error in request: out of buffer-mem, now to exit"));
      exit(1);
    }
  this->freemem -= _needmem;
}

bool
Storage::handler(unsigned long long _needmem) //>0
{
  Node* p;
  unsigned long long size;
  //if(_needmem < SET_BUFFER_SIZE)		//to recover to SET_BUFFER_SIZE buffer
  //	_needmem = SET_BUFFER_SIZE;
  while (1) {
    p = this->minheap->getTop();
    if (p == NULL)
      return false; //can't satisfy or can't recover to SET_BUFFER_SIZE
    this->minheap->remove();
    size = p->getSize();
    this->freemem += size;
    this->writeNode(p);
    if (p->getNum() > 0)
      p->Virtual();
    else
      delete p; //non-sense node
    if (_needmem > size)
      _needmem -= size;
    else
      break;
  }
  return true;
}

Storage::~Storage()
{
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
#ifdef DEBUG_KVSTORE
  printf("already empty the buffer heap!\n");
#endif
  fclose(this->treefp);
  //#ifdef DEBUG_KVSTORE
  //	//NOTICE:there is more than one tree
  //	fclose(Util::debug_kvstore);	//NULL is ok!
  //	Util::debug_kvstore = NULL;
  //#endif
}

void
Storage::print(string s)
{
#ifdef DEBUG_KVSTORE
  fputs(Util::showtime().c_str(), Util::debug_kvstore);
  fputs("Class Storage\n", Util::debug_kvstore);
  fputs("Message: ", Util::debug_kvstore);
  fputs(s.c_str(), Util::debug_kvstore);
  fputs("\n", Util::debug_kvstore);
#endif
}
