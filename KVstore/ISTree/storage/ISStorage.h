/*=============================================================================
# Filename: ISStorage.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:43
# Description: swap between memory and disk, achieving system-like method
=============================================================================*/

#ifndef _KVSTORE_ISTREE_STORAGE_ISSTORAGE_H
#define _KVSTORE_ISTREE_STORAGE_ISSTORAGE_H

#include "../node/ISIntlNode.h"
#include "../node/ISLeafNode.h"
#include "../heap/ISHeap.h"

//It controls read, write, swap
class ISStorage {
  public:
  static const unsigned BLOCK_SIZE = Util::STORAGE_BLOCK_SIZE; //fixed size of disk-block
  //there are 18 B+Tree indexes and one vstree index, so set 3G buffer size
  //static const unsigned long long MAX_BUFFER_SIZE = Util::MAX_BUFFER_SIZE;		//max buffer size
  //static const unsigned SET_BUFFER_SIZE = 1 << 30;		//set buffer size
  //static const unsigned HEAP_SIZE = MAX_BUFFER_SIZE / ISNode::INTL_SIZE;
  static const unsigned MAX_BLOCK_NUM = 1 << 24; //max block-num
  //below two constants: must can be exactly divided by 8
  static const unsigned SET_BLOCK_NUM = 1 << 8;        //initial blocks num
  static const unsigned SET_BLOCK_INC = SET_BLOCK_NUM; //base of blocks-num inc
  static const unsigned SuperNum = MAX_BLOCK_NUM / (8 * BLOCK_SIZE) + 1;
  //static const unsigned TRANSFER_CAPACITY = BLOCK_SIZE;
  //enum ReadType { OVER = 0, EXPAND, NORMAL };
  private:
  unsigned long long max_buffer_size;
  unsigned heap_size;
  unsigned cur_block_num;
  std::string filepath;
  unsigned* treeheight;
  BlockInfo* freelist;
  FILE* treefp;    //file: tree nodes
  ISHeap* minheap; //heap of Nodes's pointer, sorted in NF_RK
  //NOTICE: freemem's type is long long here, due to large memory in server.
  //However, needmem in handler() and request() is ok to be int/unsigned.
  //Because the bstr' size is controlled, so is the node.
  unsigned long long freemem; //free memory to use, non-negative
  //unsigned long long time;			//QUERY(achieving an old-swap startegy?)

  //QUERY: should this be long long? (otherwise will be different in 32-bit and 64-bit machine)
  long Address(unsigned _blocknum) const;
  unsigned Blocknum(long address) const;
  unsigned AllocBlock();
  void FreeBlock(unsigned _blocknum);
  void ReadAlign(unsigned* _next);
  void WriteAlign(unsigned* _next, bool& _SpecialBlock);

  public:
  ISStorage();
  ISStorage(std::string& _filepath, std::string& _mode, unsigned* _height, unsigned long long _buffer_size); //create a fixed-size file or open an existence
  bool preRead(ISNode*& _root, ISNode*& _leaves_head, ISNode*& _leaves_tail);                                //read and build all nodes, only root in memory
  bool fullLoad(ISNode*& _root);                                                                             //puts all nodes of IVTree in memory, assuming the memory is enough
  bool readNode(ISNode* _np, long long* _request);                                                           //read, if virtual
  bool createNode(ISNode*& _np);                                                                             //use fp to create a new node
                                                                                                             //NOTICE(if children and child not exist, build children's Nodes)
  bool writeNode(ISNode* _np);
  bool readBstr(Bstr* _bp, unsigned* _next);
  bool writeBstr(const Bstr* _bp, unsigned* _curnum, bool& _SpecialBlock);
  bool writeTree(ISNode* _np);
  void updateHeap(ISNode* _np, unsigned _rank, bool _inheap) const;
  bool request(long long _needmem);          //deal with memory request
  bool handler(unsigned long long _needmem); //swap some nodes out
  //bool update();				//update InMem Node's rank, with clock
  ~ISStorage();
  void print(std::string s); //DEBUG
};

#endif
