/*=====================================================================
 * File name: ISBlockManager.h
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last Modified: 2018-01-30
 * Description:
 * To manage the block where values is stored
 * each block's size is 4K, the first unsigned records the index of next block,
 * the second unsigned record the length of string
 * so each block stores 4096- 2 * sizeof(unsigned) bits value
 * ======================================================================*/

#include "../../Util/Util.h"

using namespace std;

class ISBlockManager {
  private:
  // size of block, 4K, unit is byte
  static const unsigned BLOCK_SIZE = 1 << 7; //1 << 12;
  // size of data stored in block, unit is byte
  static const unsigned BLOCK_DATA_SIZE = BLOCK_SIZE - 2 * sizeof(unsigned);
  // minimum initial blocks
  static const unsigned SET_BLOCK_NUM = 1 << 8;
  // minimum blocks-num inc
  static const unsigned SET_BLOCK_INC = SET_BLOCK_NUM;

  private:
  // record Index-Len pair of a series of continuous blocks
  // here len means number of blocks
  map<unsigned, unsigned> index_len_map;
  // record Len-Index pair of a series of continuous blocks
  // here len means number of blocks
  //multimap <unsigned, unsigned> len_index_map;
  set<pair<unsigned, unsigned> > len_index_map;

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

  bool getWhereToWrite(unsigned _len);

  public:
  ISBlockManager();
  ISBlockManager(string& _filename, string& _mode, unsigned _keynum = 0);
  ~ISBlockManager();

  unsigned WriteValue(const char* _str, const unsigned _len);

  bool ReadValue(unsigned _blk_index, char*& _str, unsigned& _len);

  void SaveFreeBlockList();

  bool FreeBlocks(const unsigned index);
};
