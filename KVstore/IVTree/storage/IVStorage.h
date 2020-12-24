/*=============================================================================
# Filename: IVStorage.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:43
# Description: swap between memory and disk, achieving system-like method
=============================================================================*/

#ifndef _KVSTORE_IVTREE_STORAGE_IVSTORAGE_H
#define _KVSTORE_IVTREE_STORAGE_IVSTORAGE_H

#include "../../../Util/VList.h"
#include "../node/IVIntlNode.h"
#include "../node/IVLeafNode.h"
#include "../heap/IVHeap.h"

//It controls read, write, swap
class IVStorage {
  public:
  static const unsigned BLOCK_SIZE = Util::STORAGE_BLOCK_SIZE; //fixed size of disk-block
  //there are 18 B+Tree indexes and one vstree index, so set 3G buffer size
  //static const unsigned long long MAX_BUFFER_SIZE = Util::MAX_BUFFER_SIZE;		//max buffer size
  //static const unsigned SET_BUFFER_SIZE = 1 << 30;		//set buffer size
  //static const unsigned HEAP_SIZE = MAX_BUFFER_SIZE / IVNode::INTL_SIZE;

  //DEBUG: maybe need to set larger, now the file size is 64G at most
  static const unsigned MAX_BLOCK_NUM = 1 << 24; //max block-num
  //below two constants: must can be exactly divided by 8
  static const unsigned SET_BLOCK_NUM = 1 << 8;        //initial blocks num
  static const unsigned SET_BLOCK_INC = SET_BLOCK_NUM; //base of blocks-num inc
  static const unsigned SuperNum = MAX_BLOCK_NUM / (8 * BLOCK_SIZE) + 1;
  //static const unsigned TRANSFER_CAPACITY = BLOCK_SIZE;
  //enum ReadType { OVER = 0, EXPAND, NORMAL };

  private:
  unsigned long updateHeapTime;
  unsigned long long max_buffer_size;
  unsigned heap_size;
  unsigned cur_block_num;
  std::string filepath;
  unsigned* treeheight;
  BlockInfo* freelist;
  FILE* treefp;    //file: tree nodes
  IVHeap* minheap; //heap of Nodes's pointer, sorted in NF_RK

  //very long value list are stored in a separate file(with large block)
  //
  //NOTICE: according to the summary result, 90% value lists are just below 100 bytes
  //<10%: 5000000~100M bytes
  VList* value_list;

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
  IVStorage();
  IVStorage(std::string& _filepath, std::string& _mode, unsigned* _height, unsigned long long _buffer_size, VList* _vlist); //create a fixed-size file or open an existence
  bool preRead(IVNode*& _root, IVNode*& _leaves_head, IVNode*& _leaves_tail);                                               //read and build all nodes, only root in memory
  bool preLoad(IVNode*& _root);                                                                                             //puts all nodes of IVTree in memory until the memory is full
  bool fullLoad(IVNode*& _root);                                                                                            //puts all nodes of IVTree in memory, assuming the memory is enough
  bool readNode(IVNode* _np, long long* _request);                                                                          //read, if virtual
  bool createNode(IVNode*& _np);                                                                                            //use fp to create a new node
                                                                                                                            //NOTICE(if children and child not exist, build children's Nodes)
  bool writeNode(IVNode* _np);
  bool readBstr(Bstr* _bp, unsigned* _next);
  bool writeBstr(const Bstr* _bp, unsigned* _curnum, bool& _SpecialBlock);
  bool writeTree(IVNode* _np);
  void updateHeap(IVNode* _np, unsigned _rank, bool _inheap) const;
  bool request(long long _needmem);          //deal with memory request
  bool handler(unsigned long long _needmem); //swap some nodes out
  //bool update();				//update InMem Node's rank, with clock
  ~IVStorage();
  void print(std::string s); //DEBUG
};

#endif
