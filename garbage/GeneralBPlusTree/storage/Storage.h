/*=============================================================================
# Filename: Storage.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:43
# Description: swap between memory and disk, achieving system-like method
=============================================================================*/

#ifndef _KVSTORE_STORAGE_STORAGE_H
#define _KVSTORE_STORAGE_STORAGE_H

#include "../node/IntlNode.h"
#include "../node/LeafNode.h"
#include "../heap/Heap.h"
#include "file.h"

//It controls read, write, swap
class Storage {
  public:
  static const unsigned BLOCK_SIZE = 1 << 16; //fixed size of disk-block
  //there are 18 B+Tree indexes and one vstree index, so set 3G buffer size
  //static const unsigned long long MAX_BUFFER_SIZE = 0xC0000000;		//max buffer size
  //static const unsigned long long MAX_BUFFER_SIZE = 0x1ffffffff;		//max buffer size
  static const unsigned long long MAX_BUFFER_SIZE = 0xffffffff; //max buffer size
  //static const unsigned SET_BUFFER_SIZE = 1 << 30;		//set buffer size
  static const unsigned HEAP_SIZE = MAX_BUFFER_SIZE / Node::INTL_SIZE;
  static const unsigned MAX_BLOCK_NUM = 1 << 24; //max block-num
  //below two constants: must can be exactly divided by 8
  static const unsigned SET_BLOCK_NUM = 1 << 8;        //initial blocks num
  static const unsigned SET_BLOCK_INC = SET_BLOCK_NUM; //base of blocks-num inc
  static const unsigned SuperNum = MAX_BLOCK_NUM / (8 * BLOCK_SIZE) + 1;
  //static const unsigned TRANSFER_CAPACITY = BLOCK_SIZE;
  //enum ReadType { OVER = 0, EXPAND, NORMAL };
  private:
  unsigned cur_block_num;
  std::string filepath;
  unsigned* treeheight;
  BlockInfo* freelist;
  FILE* treefp;  //file: tree nodes
  Heap* minheap; //heap of Nodes's pointer, sorted in NF_RK
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
  Storage();
  Storage(std::string& _filepath, std::string& _mode, unsigned* _height); //create a fixed-size file or open an existence
  bool preRead(Node*& _root, Node*& _leaves_head, Node*& _leaves_tail);   //read and build all nodes, only root in memory
  bool readNode(Node* _np, int* _request);                                //read, if virtual
  bool createNode(Node*& _np);                                            //use fp to create a new node
  //NOTICE(if children and child not exist, build children's Nodes)
  bool writeNode(Node* _np);
  bool readBstr(Bstr* _bp, unsigned* _next);
  bool writeBstr(const Bstr* _bp, unsigned* _curnum, bool& _SpecialBlock);
  bool writeTree(Node* _np);
  void updateHeap(Node* _np, unsigned _rank, bool _inheap) const;
  void request(int _needmem);      //deal with memory request
  bool handler(unsigned _needmem); //swap some nodes out
  //bool update();				//update InMem Node's rank, with clock
  ~Storage();
  void print(std::string s); //DEBUG
};

#endif
