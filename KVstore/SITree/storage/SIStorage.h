/*=============================================================================
# Filename: SIStorage.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:43
# Description: swap between memory and disk, achieving system-like method
=============================================================================*/

#ifndef _KVSTORE_SITREE_STORAGE_SISTORAGE_H
#define _KVSTORE_SITREE_STORAGE_SISTORAGE_H

#include "../node/SIIntlNode.h"
#include "../node/SILeafNode.h"
#include "../heap/SIHeap.h"

//TODO: whether to use heap or not, is a big question
//For single-query application, it seems that LRU list like VSTree is a better choice(no much cost in the buffer itself)
//But in multiple-queries case, things maybe different
//BETTER:
//add a heap position in node, to speed up the node-pointer searching
//lower the update times of heap, if the size is 128M, then each update is 27 at most
//if not update in time, then the heap maybe not be a heap, then why do we use heap? why not a simple array?

//It controls read, write, swap
class SIStorage {
  public:
  static const unsigned BLOCK_SIZE = Util::STORAGE_BLOCK_SIZE; //fixed size of disk-block
  //there are 18 B+Tree indexes and one vstree index, so set 3G buffer size
  //static const unsigned long long MAX_BUFFER_SIZE = Util::MAX_BUFFER_SIZE;		//max buffer size
  //static const unsigned SET_BUFFER_SIZE = 1 << 30;		//set buffer size
  //static const unsigned HEAP_SIZE = MAX_BUFFER_SIZE / SINode::INTL_SIZE;
  static const unsigned MAX_BLOCK_NUM = 1 << 24;       //max block-num
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
  SIHeap* minheap; //heap of Nodes's pointer, sorted in NF_RK
  //NOTICE: freemem's type is long long here, due to large memory in server.
  //However, needmem in handler() and request() is ok to be int/unsigned.
  //Because the bstr' size is controlled, so is the node.
  unsigned long long freemem; //free memory to use, non-negative
  //unsigned long long time;			//QUERY(achieving an old-swap startegy?)
  long Address(unsigned _blocknum) const;
  unsigned Blocknum(long address) const;
  unsigned AllocBlock();
  void FreeBlock(unsigned _blocknum);
  void ReadAlign(unsigned* _next);
  void WriteAlign(unsigned* _next, bool& _SpecialBlock);

  public:
  SIStorage();
  SIStorage(std::string& _filepath, std::string& _mode, unsigned* _height, unsigned long long _buffer_size); //create a fixed-size file or open an existence
  bool preRead(SINode*& _root, SINode*& _leaves_head, SINode*& _leaves_tail);                                //read and build all nodes, only root in memory
  bool fullLoad(SINode*& _root);                                                                             //puts all nodes of IVTree in memory
  bool readNode(SINode* _np, long long* _request);                                                           //read, if virtual
  bool createNode(SINode*& _np);                                                                             //use fp to create a new node
                                                                                                             //NOTICE(if children and child not exist, build children's Nodes)
  bool writeNode(SINode* _np);
  bool readBstr(Bstr* _bp, unsigned* _next);
  bool writeBstr(const Bstr* _bp, unsigned* _curnum, bool& _SpecialBlock);
  bool writeTree(SINode* _np);
  void updateHeap(SINode* _np, unsigned _rank, bool _inheap) const;
  bool request(long long _needmem);          //deal with memory request
  bool handler(unsigned long long _needmem); //swap some nodes out
  //bool update();				//update InMem Node's rank, with clock
  ~SIStorage();
  void print(std::string s); //DEBUG
};

#endif
