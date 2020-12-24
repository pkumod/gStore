/*=============================================================================
# Filename: file.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:42
# Description: disk file memlayout
=============================================================================*/

#ifndef _KVSTORE_STORAGE_FILE_H
#define _KVSTORE_STORAGE_FILE_H

#include "../../Util/Util.h"

/******** manage the disk-file as Blocks-list ********/
/* All blocks of given file are viewed as an array, extended dynamicly
 * The first block(0, super-block) includes the information 
 * about the whole file(for example, the root Node's Block), 
 * especially a bitset use[MAXBN] which is used to recognize which 
 * block is in use
 * In practical, the normal block numbers from 1 to MAXBN-1, 
 * so 0 can be used as division.
 * (normal block is made of header and data)
 * When file opened, program must read this bitset and create an 
 * freelist(several 10Ms memory),
 * and remember to write back the bitset when closing.
 * We store each Tree-Node as a unit, which may contain several 
 * blocks, not requiring continuous.
 * While the tree is not closed, better to keep root Node in 
 * memory all the time.
 */

/*
struct Header				
{	//this is the header information at the 
	//beginning of each block, then the data	
	//
	//If this the first block of a node, we must also store
	 * the necessary information about the node. For example,
	 * a bit indicates whether a leaf-node, deciding how it 
	 * should be read.	The first block's prev and the 
	//final block's next should be 0
	//blockaddr_t prev;
	blockaddr_t next;			//WARN(maybe larger type!)
	//unsigned short end;			//valid data:0~end
};

struct SuperBlock				//SuperNum blocks, numbered 0
{
	unsigned height;
	unsigned rootnum;		//use a whole block, may store other information 
								//for example, nodes's num
	char use[BNWD];				//exactly SuperNum-1 blocks	
};

//numbered from 1 to MAX_BLOCK_NUM
struct Node						//may use several blocks, not must continuously
{
	unsigned flag;              //only in first block, special-block
	unsigned next;				//each real data-block, 0 means the end
	information:
	unsigned num;
	unsigned childs[];			//only in IntlNodes
	Bstr keys[];
	Bstr values[];				//only in LeafNodes
};
*/

//When stored in disk, every Node* pointer should be changed to block-address
//(a bit indicates whether a leaf!),
//and char* should be changed to the real string.

class BlockInfo {
  public:
  unsigned num;
  BlockInfo* next;
  BlockInfo()
  {
    num = 0;
    next = NULL;
  }
  BlockInfo(unsigned _num, BlockInfo* _bp)
  {
    num = _num;
    next = _bp;
  }
};

#endif
