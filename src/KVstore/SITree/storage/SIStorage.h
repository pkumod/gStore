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

/**
 * The Controller of read, write, swap operation
 *
 * The SITree File are composed of 3 part:
 * [mete data][free block bit map][block content]
 *
 * [mete data]
 * 0-4 byte:  (unsigned int) tree height
 * 4-7 byte:  (unsigned int) the root SINode's block id
 * 8-11 byte: (unsigned int) cur_block_num
 *
 * [free block bit map]
 * Attention : scalar is bits
 * 0 ~ MAX_BLOCK_NUM bits: bit i represent whether block i has been used
 *                         0: free 1:used
 * the max size of the SITree File ~ 68,719,476,736 Byte ~ 64 GB
 *
 * [block content]
 *  A node may occupy many blocks. Suppose Node i has n blocks, then
 *  each block has Util::STORAGE_BLOCK_SIZE Bytes
 *  [block 0]
 *      0-3 bytes store node flag info
 *      4-7 bytes store block id of [block 1]
 *      8 - (Util::STORAGE_BLOCK_SIZE-1) bytes stores the actual content
 *  [block 1]
 *      0-3 bytes store block id of [block 2]
 *      4-(Util::STORAGE_BLOCK_SIZE-1) bytes stores the actual content
 *
 *      ......
 *
 *  [block n-1]
 *      0-3 bytes store block id of [block n]
 *      4-(Util::STORAGE_BLOCK_SIZE-1) bytes stores the actual content
 *  [block n]
 *      0-3 bytes is zero
 *      4-(Util::STORAGE_BLOCK_SIZE-1) bytes stores the actual content
 *
 *  actual content refers to keys and values. keys will first be stored
 *  and later values.
 */
class SIStorage
{
public:
  //fixed size of disk-block
  static const unsigned BLOCK_SIZE = Util::STORAGE_BLOCK_SIZE;
  //max block-num ,must can be exactly divided by 8
  static const unsigned MAX_BLOCK_NUM = 1 << 24;
  //below two constants: must can be exactly divided by 8
  //initial blocks num
  static const unsigned SET_BLOCK_NUM = 1 << 8;
  //base of blocks-num inc
  static const unsigned SET_BLOCK_INC = SET_BLOCK_NUM;
  // how many pages the bitmap and metadata may occupy, also the start block
  // of the actual content
  static const unsigned SuperNum = MAX_BLOCK_NUM / (8 * BLOCK_SIZE) + 1;
 private:
	unsigned long long max_buffer_size;
	unsigned heap_size;
	unsigned cur_block_num;
	std::string filepath;
	unsigned* tree_height_;
	BlockInfo* freelist;
	FILE* tree_fp_;						//file: tree nodes

	SIHeap* min_heap_;					//heap of Nodes's pointer, sorted in NF_RK

	//NOTICE: freemem's type is long long here, due to large memory in server.
	//However, needmem in handler() and request() is ok to be int/unsigned.
	//Because the bstr' size is controlled, so is the node.
	unsigned long long freemem;  		//free memory to use, non-negative
	//unsigned long long time;			//QUERY(achieving an old-swap startegy?)
	long Address(unsigned block_num) const;
	unsigned Blocknum(long address) const;
	unsigned AllocBlock();
	void FreeBlock(unsigned block_num);
	void ReadAlign(unsigned* _next);
	void WriteAlign(unsigned* _next, bool& _SpecialBlock);

public:
	SIStorage();
	SIStorage(std::string& _filepath, std::string& _mode, unsigned* _height, unsigned long long _buffer_size);//create a fixed-size file or open an existence
	bool PreRead(SINode*& _root, SINode*& _leaves_head, SINode*& _leaves_tail);		//read and build all nodes, only root in memory
	bool fullLoad(SINode*& _root); //puts all nodes of IVTree in memory
	bool ReadNode(SINode* _np, long long* _request);	//read, if virtual
	bool CreateNode(SINode*& _np);		//use fp to create a new node
										//NOTICE(if children and child not exist, build children's Nodes)
	bool WriteNode(SINode* _np);
	bool ReadBstr(Bstr* _bp, unsigned* _next);
	bool writeBstr(const Bstr* _bp, unsigned* _curnum, bool& _SpecialBlock);
	bool WriteTree(SINode* _np);
	void updateHeap(SINode* _np, unsigned _rank, bool _inheap) const;
	bool request(long long needed_mem);			//deal with memory request
	bool handler(unsigned long long needed_mem);	//swap some nodes out
	//bool update();				//update InMem Node's rank, with clock
	~SIStorage();
	void print(std::string s);				//DEBUG
};

#endif

