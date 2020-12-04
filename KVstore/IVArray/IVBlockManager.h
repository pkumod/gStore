/*=====================================================================
 * File name: IVBlockManager.h
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last Modified: 2018-01-30
 * Description:
 * To manage the block where values is stored
 * each block's size is 4K, the first unsigned records the index of next block,
 * the second unsigned record the length of string
 * so each block stores 4096- 2 * sizeof(unsigned) bits value
 * ======================================================================
 * Modified by: Yuqi Zhou 
 * Date: 2019-08-02
 * Description: 
 * Change the structure of each block, each block's size is 4K, the first
 * unsigned records the index of next block, the next two unsigned(
 * (unsigned long) records the length of string
 * so each block stores 4096- sizeof(unsigned) - sizeof(unsigned long) bits value
 * ======================================================================*/

#include "../../Util/Util.h"

using namespace std;

class IVBlockManager
{
private:
	// size of block, 4K, unit is byte
	static const unsigned BLOCK_SIZE = 1 << 8;//1 << 12;
	// size of data stored in block, unit is byte
	static const unsigned BLOCK_DATA_SIZE = BLOCK_SIZE -  sizeof(unsigned int)- sizeof(unsigned long);
	// minimum initial blocks
	static const unsigned SET_BLOCK_NUM = 1 << 8;
	// minimum blocks-num inc
	static const unsigned SET_BLOCK_INC = SET_BLOCK_NUM;

private:
	// record Index-Len pair of a series of continuous blocks
	// here len means number of blocks
	map <unsigned, unsigned> index_len_map;
	// record Len-Index pair of a series of continuous blocks
	// here len means number of blocks
	//multimap <unsigned, unsigned> len_index_map;
	set <pair<unsigned, unsigned> > len_index_map;

	//string dir_path;
	string FreeBlockList_path;
	string ValueFile_path;
	// File record which block is free
	FILE* FreeBlockList;
	// File record value lists
	FILE* ValueFile;

	BlockInfo* BlockToWrite;

	unsigned cur_block_num;

	bool AllocBlock(unsigned len);
	
	bool getWhereToWrite(unsigned long _len);
	
	mutex indexlock;
public:
	IVBlockManager();
	IVBlockManager(string& _filename, string &_mode, unsigned _keynum = 0);
	~IVBlockManager();

	unsigned WriteValue(const char *_str, const unsigned long _len);

	bool ReadValue(unsigned _blk_index, char *&_str, unsigned long &_len);

	void SaveFreeBlockList();

	bool FreeBlocks(const unsigned index);
};
