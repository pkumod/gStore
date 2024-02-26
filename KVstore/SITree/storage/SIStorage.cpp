/*=============================================================================
# Filename: SIStorage.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:43
# Description: achieve functions in SIStorage.h
=============================================================================*/

#include "SIStorage.h"

using namespace std;

SIStorage::SIStorage()
{							//not use ../logs/, notice the location of program
  cur_block_num = SET_BLOCK_NUM;
  filepath = "";
  freelist = NULL;
  tree_fp_ = NULL;
  min_heap_ = NULL;
  max_buffer_size = Util::MAX_BUFFER_SIZE;
  heap_size = max_buffer_size / SINode::INTL_SIZE;
  freemem = max_buffer_size;
}

SIStorage::SIStorage(string& _filepath, string& _mode, unsigned* _height, unsigned long long _buffer_size)
{
  cur_block_num = SET_BLOCK_NUM;		//initialize
  this->filepath = _filepath;
  if (_mode == string("build"))
    tree_fp_ = fopen(_filepath.c_str(), "w+b");
  else if (_mode == string("open"))
    tree_fp_ = fopen(_filepath.c_str(), "r+b");
  else
  {
    print(string("error in SIStorage: Invalid mode ") + _mode);
    return;
  }
  if (tree_fp_ == NULL)
  {
    print(string("error in SIStorage: Open error ") + _filepath);
    return;
  }
  this->tree_height_ = _height;		//originally set to 0
  this->max_buffer_size = _buffer_size;

  this->heap_size = this->max_buffer_size / SINode::INTL_SIZE;
  this->freemem = this->max_buffer_size;
  this->freelist = new BlockInfo;	//null-head
  unsigned i, j, k;	//j = (SuperNum-1)*BLOCK_SIZE
  BlockInfo* bp;
  if (_mode == "build")
  {	//write basic information
    i = 0;
    fwrite(&i, sizeof(unsigned), 1, this->tree_fp_);	//height
    fwrite(&i, sizeof(unsigned), 1, this->tree_fp_); //rootnum
    fwrite(&cur_block_num, sizeof(unsigned), 1, this->tree_fp_);	//current block num
    fseek(this->tree_fp_, BLOCK_SIZE, SEEK_SET);
    bp = this->freelist;
    j = cur_block_num / 8;
    for (i = 0; i < j; ++i)
    {
      fputc(0, this->tree_fp_);
      for (k = 0; k < 8; ++k)
      {
        bp->next = new BlockInfo(i * 8 + k + 1, NULL);
        bp = bp->next;
      }
    }
  }
  else	//_mode == "open"
  {
    //read basic information
    unsigned int rootnum;
    char c;
    fread(this->tree_height_, sizeof(unsigned), 1, this->tree_fp_);
    fread(&rootnum, sizeof(unsigned), 1, this->tree_fp_);
    fread(&cur_block_num, sizeof(unsigned), 1, this->tree_fp_);
    fseek(this->tree_fp_, BLOCK_SIZE, SEEK_SET);
    bp = this->freelist;
    j = cur_block_num / 8;
    for (i = 0; i < j; ++i)
    {
      c = fgetc(tree_fp_);
      for (k = 0; k < 8; ++k)
      {
        if ((c & (1 << k)) == 0)
        {
          bp->next = new BlockInfo(i * 8 + 7 - k + 1, NULL);
          bp = bp->next;
        }
      }
    }
    fseek(tree_fp_, Address(rootnum), SEEK_SET);
    //tree_fp_ is now ahead of root-block
  }
  this->min_heap_ = new SIHeap(this->heap_size);
}

/**
 * pre-read and build whole tree
 * @param _root reference of tree root
 * @param _leaves_head reference of leaves head
 * @param _leaves_tail reference of leaves tail
 * @return whether PreRead success
 */
bool
SIStorage::PreRead(SINode*& _root, SINode*& _leaves_head, SINode*& _leaves_tail)
{
  //set root(in memory) and leaves_head_
  _leaves_tail = _leaves_head = _root = NULL;
  if (ftell(this->tree_fp_) == 0)	//root is null
    return true;

  unsigned next, store, pos = 0;
  unsigned h = *this->tree_height_;
  SINode* p;

  //read root node, and read 4 bytes from this->tree_fp_
  this->CreateNode(p);
  _root = p;
  fread(&next, sizeof(unsigned), 1, tree_fp_);

  // use stack to achieve
  // this is a stack-style structure. i.e
  // address[j] means the address of current node in the j-th layer
  long address[h];	//current address
  unsigned used[h];	//used child num
  unsigned total[h];	//total child num
  unsigned block[h];	//next block num

  SINode* nodes[h];
  address[pos] = ftell(tree_fp_);
  used[pos] = 0;
  total[pos] = p->GetKeyNum() + 1;
  block[pos] = next;
  nodes[pos] = p;
  pos++;
  SINode* prev = NULL;

  // read tree from the disk, and build a linked list among the leaf nodes
  // because we have already read block 0 ,so pos starts from 1
  while (pos > 0)
  {
    unsigned int j = pos - 1;// nodes[j] is nodes[pos]'s parent
    if (nodes[j]->isLeaf() || used[j] == total[j])	//LeafNode or ready IntlNode
    {
      if (nodes[j]->isLeaf())
      {
        // build a linked list among the leaf nodes
        if (prev != nullptr)
        {
          prev->SetNext(nodes[j]);
          nodes[j]->setPrev(prev);
        }
        prev = nodes[j];
      }
      // walk back upper one layer
      pos--;
      continue;
    }

    // change the j-th layer information
    // move forward one node
    fseek(this->tree_fp_, address[j], SEEK_SET);
    fread(&store, sizeof(unsigned), 1, tree_fp_);
    this->ReadAlign(block + j);
    address[j] = ftell(tree_fp_);
    fseek(tree_fp_, Address(store), SEEK_SET);

    // Read the New node from disk
    this->CreateNode(p);
    nodes[j]->setChild(p, used[j]);
    used[j]++;
    fread(&next, sizeof(unsigned), 1, tree_fp_);
    address[pos] = ftell(tree_fp_);
    used[pos] = 0;
    total[pos] = p->GetKeyNum() + 1;
    block[pos] = next;
    nodes[pos] = p;
    // walk deeper one layer
    pos++;
  }

  //set leaves and read root, which is always kept in-mem
  p = _root;
  while (!p->isLeaf())
  {
    p = p->GetChild(0);
  }
  // the most left node
  _leaves_head = p;
  p = _root;
  while (!p->isLeaf())
  {
    p = p->GetChild(p->GetKeyNum());
  }
  // the most right node
  _leaves_tail = p;
  long long memory = 0;
  this->ReadNode(_root, &memory);
  this->request(memory);
  return true;
}

/**
 * load the entire tree
 * @param _root root node
 */
bool
SIStorage::fullLoad(SINode*& _root)
{
  if(_root == NULL)
  {
    return false;
  }
  std::queue <SINode*> node_q;
  SINode* p = _root;
  while (!p->isLeaf())
  {
    SINode* tmp;
    unsigned KN = p->GetKeyNum();
    for(unsigned i = 0; i <= KN; ++i)
    {
      tmp = p->GetChild(i);
      long long memory = 0;
      this->ReadNode(tmp, &memory);
      //this->request(memory);
      node_q.push(tmp);
    }
    p = node_q.front();
    node_q.pop();
  }
  while (!node_q.empty())
  {
    node_q.pop();
  }
  return true;
}

/**
 * @param block_num the block id
 * @return the true offset in the file of this block
 */
long
SIStorage::Address(unsigned block_num) const  //BETTER: inline function
{
  if (block_num == 0)
    return 0;
  else if (block_num > cur_block_num)
  {
    return -1;		//address should be non-negative
  }
  // NOTICE: here should explicitly use long
  //   (SuperNum + block_num - 1) * BLOCK_SIZE
  // = { MAX_BLOCK_NUM / (8 * BLOCK_SIZE) + 1 + block_num - 1 ) * BLOCK_SIZE
  // = { MAX_BLOCK_NUM / (8 * BLOCK_SIZE) + block_num ) * BLOCK_SIZE
  // = MAX_BLOCK_NUM / 8 +  block_num * BLOCK_SIZE
  return (long)(this->SuperNum + block_num - 1) * (long)BLOCK_SIZE;
}

/**
 * @param address the file offset
 * @return the block id corresponding to this offset
 */
unsigned
SIStorage::Blocknum(long address) const
{
  return (address / BLOCK_SIZE) + 1 - this->SuperNum;
}

/**
 * return a free block id in freelist.
 * if free list is empty, alloc SET_BLOCK_INC blocks and
 * add them into freelist before returning first block
 * @return the new block id
 */
unsigned
SIStorage::AllocBlock()
{
  BlockInfo* p = this->freelist->next;
  if (p == NULL)
  {
    for (unsigned i = 0; i < SET_BLOCK_INC; ++i)
    {
      cur_block_num++;	//BETTER: check if > MAX_BLOCK_NUM
      this->FreeBlock(cur_block_num);
    }
    p = this->freelist->next;
  }
  unsigned t = p->num;
  this->freelist->next = p->next;
  delete p;

  return t;
}

/**
 * alloc a BlockInfo and insert it to freelist
 * @param block_num the block id of the new BlockInfo
 */
void
SIStorage::FreeBlock(unsigned block_num)
{
  BlockInfo* bp = new BlockInfo(block_num, this->freelist->next);
  this->freelist->next = bp;
}


/**
 * if tree_fp points to the head of a page now, read
 * from tree_fp with the true offset calculated from
 * *next and update *next to the next page
 * Such way saves the frequency to access disk
 * @param _next the next block id if we met a new block
 */
void
SIStorage::ReadAlign(unsigned* _next)
{
  if (ftell(tree_fp_) % BLOCK_SIZE == 0)
  {
    fseek(tree_fp_, Address(*_next), SEEK_SET);
    fread(_next, sizeof(unsigned), 1, tree_fp_);
  }
}

/**
 * Write a new block. And write the next block id in the first 4 byte( if _SpecialBlock,
 * 4-7 bytes instead)
 * @param _curnum the new block id
 * @param _SpecialBlock means if this block is the first block of one SINode
 * storage. if this block is a special block, leave the first 4 bytes empty.
 * _SpecialBlock value will be changed if new block read
 */
void
SIStorage::WriteAlign(unsigned* _curnum, bool& _SpecialBlock)
{
  if (ftell(tree_fp_) % BLOCK_SIZE == 0)
  {
    unsigned blocknum = this->AllocBlock();
    fseek(tree_fp_, Address(*_curnum), SEEK_SET);
    if (_SpecialBlock)
    {
      fseek(tree_fp_, 4, SEEK_CUR);
      _SpecialBlock = false;
    }
    fwrite(&blocknum, sizeof(unsigned), 1, tree_fp_);
    fseek(tree_fp_, Address(blocknum) + 4, SEEK_SET);
    *_curnum = blocknum;
  }
}

/**
 * Read the SINode into memory , read its keys (and values)
 * from disk and change the SITree memory request size
 * @param _np the SINode
 * @param _request the memory SITree has used
 * @return whether the operation has successfully operated
 */
bool
SIStorage::ReadNode(SINode* _np, long long* _request)
{
  if (_np == NULL || _np->inMem())
  {
    std::cout << "error ReadNode: can't read or needn't" << std::endl;
    return false;
  }
  fseek(tree_fp_, Address(_np->GetStore()), SEEK_SET);

  bool np_is_leaf = _np->isLeaf();
  unsigned next;
  unsigned i, num = _np->GetKeyNum();
  Bstr bstr;
  fseek(tree_fp_, 4, SEEK_CUR);
  fread(&next, sizeof(unsigned), 1, tree_fp_);
  if (np_is_leaf)
    *_request += SINode::LEAF_SIZE;
  else
    *_request += SINode::INTL_SIZE;
  _np->Normal();
  if (!np_is_leaf)
    fseek(tree_fp_, 4 * (num + 1), SEEK_CUR);

  //to read all keys
  for (i = 0; i < num; ++i)
  {
    bstr.clear();
    this->ReadBstr(&bstr, &next);
    *_request += bstr.getLen();
    _np->SetKey(&bstr, i);
  }

  //to read all values
  if (np_is_leaf)
  {
    auto tmp = INVALID;
    for (i = 0; i < num; ++i)
    {
      fread(&tmp, sizeof(unsigned), 1, tree_fp_);
      this->ReadAlign(&next);
      _np->SetValue(tmp, i);
    }
  }
  _np->delDirty();
  this->updateHeap(_np, _np->getRank(), false);
  bstr.clear();

  return true;
}

/**
 * create virtual nodes, not in-mem. It will read 4 bytes from tree fp
 * @param _np the reference of created node
 * @return if success
 */
bool
SIStorage::CreateNode(SINode*& _np)
{
  unsigned t;		//QUERY: maybe next-node_flag_... will be better-storage?
  bool flag = false;			//IntlNode
  fread(&t, sizeof(unsigned), 1, tree_fp_);
  if ((t & SINode::NF_IL) > 0)	//WARN: according to setting
    flag = true;			//LeafNode
  if (flag)
  {
    _np = new SILeafNode(true);
  }
  else
  {
    _np = new SIIntlNode(true);
  }
  _np->setFlag(t);
  _np->delDirty();
  _np->SetMemFlagFalse();
  _np->setStore(Blocknum(ftell(tree_fp_) - 4));

  return true;
}

/**
 * Write a new node into disk. It will free the node's disk storage
 * first and re-alloc block after
 * @param _np the written node
 */
bool
SIStorage::WriteNode(SINode* _np)
{
  if (_np == NULL || !_np->inMem() || (_np->getRank() > 0 && !_np->isDirty()))
    return false;	//not need to write back

  unsigned num = _np->GetKeyNum(), i;
  bool flag = _np->isLeaf(), SpecialBlock = true;
  //to release original blocks
  unsigned store = _np->GetStore(), next;
  //if first store is 0, meaning a new node

  // the first block
  // 0-3 bytes empty
  // 4-7 bytes block1's id
  fseek(this->tree_fp_, Address(store) + 4, SEEK_SET);
  fread(&next, sizeof(unsigned), 1, tree_fp_);

  // free its origin blocks for further rusage
  // for block[i] i> 0
  // 0-3 bytes next_block_id
  // if  next_block_id==0, this is the last

  while (store != 0)
  {
    this->FreeBlock(store);
    store = next;
    fseek(tree_fp_, Address(store), SEEK_SET);
    fread(&next, sizeof(unsigned), 1, tree_fp_);
  }
  if (num == 0)
    return true;		//node is empty!
  unsigned t;
  //write Node information
  unsigned blocknum = this->AllocBlock();
  _np->setStore(blocknum);
  long address = this->Address(blocknum);
  fseek(this->tree_fp_, address, SEEK_SET);
  t = _np->getFlag();
  fwrite(&t, sizeof(unsigned), 1, tree_fp_); //DEBUG
  fseek(tree_fp_, 4, SEEK_CUR);
  if (!flag)
  {
    for (i = 0; i <= num; ++i)
    {
      t = _np->GetChild(i)->GetStore();
      fwrite(&t, sizeof(unsigned), 1, tree_fp_); //DEBUG
      this->WriteAlign(&blocknum, SpecialBlock);
    }
  }

  //to write all keys
  for (i = 0; i < num; ++i)
    this->writeBstr(_np->getKey(i), &blocknum, SpecialBlock);

  if (flag)
  {
    unsigned tmp = INVALID;
    //to write all values
    for (i = 0; i < num; ++i)
    {
      tmp = _np->GetValue(i);
      fwrite(&tmp, sizeof(unsigned), 1, tree_fp_);
      this->WriteAlign(&blocknum, SpecialBlock);
    }
  }
  fseek(tree_fp_, Address(blocknum), SEEK_SET);
  if (SpecialBlock)
    fseek(tree_fp_, 4, SEEK_CUR);
  t = 0;
  fwrite(&t, sizeof(unsigned), 1, tree_fp_);		//the end-block
  _np->delDirty();

  return true;
}

/**
 * Read str from address _next.
 * the pages makes a linked list starting from address next.
 * @param _bp the target Bstr
 * @param _next the pointer to address
 */
bool
SIStorage::ReadBstr(Bstr* _bp, unsigned* _next)
{
  //long address;
  unsigned len, i, j;
  fread(&len, sizeof(unsigned), 1, this->tree_fp_);
  this->ReadAlign(_next);
  char* s = new char[len];
  _bp->setLen(len);
  for (i = 0; i + 4 < len; i += 4)
  {
    fread(s + i, sizeof(char), 4, tree_fp_);
    this->ReadAlign(_next);
  }
  while (i < len)
  {
    fread(s + i, sizeof(char), 1, tree_fp_);	//BETTER
    i++;
  }
  j = len % 4;
  if (j > 0)
    j = 4 - j;
  fseek(tree_fp_, j, SEEK_CUR);
  this->ReadAlign(_next);
  _bp->setStr(s);

  return true;
}

/**
 * Write BStr to Disk while maintains block id chain. It will not delete _bp
 * @param _bp bstr
 * @param _curnum block id
 * @param _SpecialBlock if block is the first block of this node
 */
bool
SIStorage::writeBstr(const Bstr* _bp, unsigned* _curnum, bool& _SpecialBlock)
{
  unsigned i, j, len = _bp->getLen();
  fwrite(&len, sizeof(unsigned), 1, tree_fp_);
  this->WriteAlign(_curnum, _SpecialBlock);
  char* s = _bp->getStr();
  for (i = 0; i + 4 < len; i += 4)
  {
    fwrite(s + i, sizeof(char), 4, tree_fp_);
    this->WriteAlign(_curnum, _SpecialBlock);
  }
  while (i < len)
  {
    fwrite(s + i, sizeof(char), 1, tree_fp_);
    i++;
  }
  j = len % 4;
  if (j > 0)
    j = 4 - j;
  fseek(tree_fp_, j, SEEK_CUR);
  this->WriteAlign(_curnum, _SpecialBlock);

  return true;
}

/**
 * write the whole tree back and close tree_fp_. First meta data, then
 * free blocks bitmap and then the node content.
 * @param _np the root pointer
 */
bool
SIStorage::WriteTree(SINode* _np)	//
{
  fseek(this->tree_fp_, 0, SEEK_SET);
  fwrite(this->tree_height_, sizeof(unsigned), 1, tree_fp_);

  //delete all nonsense-node in heap, otherwise will waste storage permanently
  SINode* p;
  while (1)
  {
    //all non-sense nodes will be in-head-area, due to minimal rank
    p = min_heap_->getTop();
    if (p == NULL)	//heap is empty, only when root==NULL
      break;
    if (p->getRank() == 0)	//indicate non-sense node
    {
      this->min_heap_->RemoveTop();
      this->WriteNode(p);
      delete p;
    }
    else
      break;
  }

  unsigned i, j, t;
  //QUERY: another way to write all nodes back is to print out all nodes in heap
  //but this method will cause no node in heap any more, while operations may be
  //after tree-saving.	Which method is better?
  //The codes below write nodes recursively using stack, including root-num
  if (_np != NULL)
  {
    SINode* p = _np;
    unsigned h = *this->tree_height_, pos = 0;
    SINode* ns[h];
    int ni[h];
    ns[pos] = p;
    ni[pos] = p->GetKeyNum();
    pos++;
    while (pos > 0)
    {
      j = pos - 1;
      p = ns[j];
      if (p->isLeaf() || ni[j] < 0)	//leaf or all childs are ready
      {
        this->WriteNode(p);
        pos--;
        continue;
      }
      ns[pos] = p->GetChild(ni[j]);
      ni[pos] = ns[pos]->GetKeyNum();
      pos++;
      ni[j]--;
    }
    t = _np->GetStore();
  }
  else
    t = 0;

  fseek(this->tree_fp_, 4, SEEK_SET);
  fwrite(&t, sizeof(unsigned), 1, tree_fp_);	//write the root num
  fwrite(&cur_block_num, sizeof(unsigned), 1, tree_fp_);//write current blocks num
  fseek(tree_fp_, BLOCK_SIZE, SEEK_SET);
  j = cur_block_num / 8;	//(SuperNum-1)*BLOCK_SIZE;
  //reset to 1 first
  for (i = 0; i < j; ++i)
  {
    fputc(0xff, tree_fp_);
  }
  char c;
  BlockInfo* bp = this->freelist->next;
  while (bp != NULL)
  {
    //if not-use then set 0, aligned to byte!
#ifdef DEBUG_KVSTORE
    if (bp->num > cur_block_num)
		{
			printf("blocks num exceed, cur_block_num: %u\n", cur_block_num);
			exit(1);
		}
#endif
    j = bp->num - 1;
    i = j / 8;
    j = 7 - j % 8;
    fseek(tree_fp_, BLOCK_SIZE + i, SEEK_SET);
    c = fgetc(tree_fp_);
    fseek(tree_fp_, -1, SEEK_CUR);
    fputc(c & ~(1 << j), tree_fp_);
    bp = bp->next;
  }

  Util::Csync(this->tree_fp_);
  //fclose(this->tree_fp_);

  return true;
}

/**
 * Insert a new node to min heap, or changed the rank of a node
 * @param _np the SINode
 * @param _rank its new rank
 * @param _inheap if the node has already been in heap
 */
void
SIStorage::updateHeap(SINode* _np, unsigned _rank, bool _inheap) const
{
  if (_inheap)	//already in heap, to modify
  {
    unsigned t = _np->getRank();
    _np->setRank(_rank);
    if (t < _rank)
      this->min_heap_->modify(_np, false);
    else if (t > _rank)
      this->min_heap_->modify(_np, true);
    else;
  }
  else		//not in heap, to add
  {
    _np->setRank(_rank);
    this->min_heap_->Insert(_np);
  }
}

/**
 * alloc memory to satisfy SITree memory request
 * @param needed_mem the memory request
 */
bool
SIStorage::request(long long needed_mem)	//aligned to byte
{	//NOTICE: <0 means release
  if (needed_mem > 0 && this->freemem < (unsigned long long)needed_mem)
    if (!this->handler(needed_mem - freemem))	//disaster in buffer memory
    {
      print(string("error in request: out of buffer-mem, now to exit"));
      //exit(1);
      return false;;
    }
  this->freemem -= needed_mem;

  return true;
}

/**
 * Delete the top node in min heap until free memory reaches the needed size
 * @param needed_mem the need memory size
 * @return if we have enough free memory after the operation
 */
bool
SIStorage::handler(unsigned long long needed_mem)	//>0
{
  SINode* p = nullptr;
  unsigned long long size;
  while (1)
  {
    p = this->min_heap_->getTop();
    if (p == NULL)
      return false;	//can't satisfy or can't recover to SET_BUFFER_SIZE
    this->min_heap_->RemoveTop();
    size = p->GetSize();
    this->freemem += size;
    this->WriteNode(p);
    if (p->GetKeyNum() > 0)
      p->Virtual();
    else
      delete p;	//non-sense node
    if (needed_mem > size)
      needed_mem -= size;
    else
      break;
  }

  return true;
}

SIStorage::~SIStorage()
{
  //release heap and freelist...
#ifdef DEBUG_KVSTORE
  printf("now to release the kvstore!\n");
#endif
  BlockInfo* bp = this->freelist;
  BlockInfo* next;
  while (bp != NULL)
  {
    next = bp->next;
    delete bp;
    bp = next;
  }
#ifdef DEBUG_KVSTORE
  printf("already empty the freelist!\n");
#endif
  delete this->min_heap_;
  min_heap_ = NULL;
#ifdef DEBUG_KVSTORE
  printf("already empty the buffer heap!\n");
#endif
  fclose(this->tree_fp_);
}

void
SIStorage::print(string s)
{
#ifdef DEBUG_KVSTORE
  fputs(Util::showtime().c_str(), Util::debug_kvstore);
	fputs("Class SIStorage\n", Util::debug_kvstore);
	fputs("Message: ", Util::debug_kvstore);
	fputs(s.c_str(), Util::debug_kvstore);
	fputs("\n", Util::debug_kvstore);
#endif
}

