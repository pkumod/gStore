/*=======================================================================
 * File name: IVBlockManager.cpp
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last Modified: 2018-02-08
 * Description:
 * Implementation of class IVBlockManager
 * ======================================================================*/

#include "IVBlockManager.h"

using namespace std;

IVBlockManager::IVBlockManager()
{
  cur_block_num = 0;

  index_len_map.clear();
  len_index_map.clear();

  FreeBlockList = NULL;
  ValueFile = NULL;

  BlockToWrite = NULL;
}

IVBlockManager::IVBlockManager(string& _filename, string& _mode, unsigned _keynum)
{
  //dir_path = _dir_path;
  FreeBlockList_path = _filename + "_FreeBlockList";
  ValueFile_path = _filename + "_ValueFile";

  if (_mode == "build") {
    FreeBlockList = fopen(FreeBlockList_path.c_str(), "w+b");
    ValueFile = fopen(ValueFile_path.c_str(), "w+b");
  } else if (_mode == "open") {
    FreeBlockList = fopen(FreeBlockList_path.c_str(), "r+b");
    ValueFile = fopen(ValueFile_path.c_str(), "r+b");
  } else {
    cout << "Error in BLockManager: Invalid Mode" << endl;
    return;
  }

  if (FreeBlockList == NULL || ValueFile == NULL) {
    cout << "Error in IVBlockManager: Open error" << endl;
    return;
  }

  index_len_map.clear();
  len_index_map.clear();
  BlockToWrite = NULL;

  if (_mode == "build") {
    cur_block_num = 0;
    unsigned SETBLOCKNUM = 1 << 8;
    unsigned requestnum = (unsigned)max(SETBLOCKNUM, _keynum);
    AllocBlock(requestnum);
    //index_len_map.insert(make_pair(1, requestnum));
    //len_index_map.insert(make_pair(requestnum, 1));
  } else // open mode
  {
    // read cur_block_num
    int fd = fileno(FreeBlockList);
    pread(fd, &cur_block_num, 1 * sizeof(unsigned), 0);
    // read FreeBlockList and establish 2 map
    unsigned tmp_index = 0, tmp_len;
    off_t offset = 1;

    pread(fd, &tmp_index, 1 * sizeof(unsigned), offset * sizeof(unsigned));
    offset++;

    while (tmp_index > 0) {
      //cout << _filename << ": free block " << tmp_index << " offset " << offset << endl;
      pread(fd, &tmp_len, 1 * sizeof(unsigned), offset * sizeof(unsigned));
      offset++;

      index_len_map.insert(make_pair(tmp_index, tmp_len));
      len_index_map.insert(make_pair(tmp_len, tmp_index));

      pread(fd, &tmp_index, 1 * sizeof(unsigned), offset * sizeof(unsigned));
      offset++;
    }
  }
}

void
IVBlockManager::SaveFreeBlockList()
{
  //	cout << "Save Free blocks " << FreeBlockList_path << endl;
  map<unsigned, unsigned>::iterator it;
  off_t offset = 0;
  int fd = fileno(FreeBlockList);

  pwrite(fd, &cur_block_num, 1 * sizeof(unsigned), offset * sizeof(unsigned));
  offset++;

  for (it = index_len_map.begin(); it != index_len_map.end(); it++) {
    //		cout << "block " << it->first << " length " << it->second << " offset " << offset << endl;
    pwrite(fd, &(it->first), 1 * sizeof(unsigned), offset * sizeof(unsigned));
    offset++;

    pwrite(fd, &(it->second), 1 * sizeof(unsigned), offset * sizeof(unsigned));
    offset++;
    //fwrite(&(it->first), sizeof(unsigned), 1, FreeBlockList);
    //fwrite(&(it->second), sizeof(unsigned), 1, FreeBlockList);
  }

  unsigned tmp = 0;
  pwrite(fd, &tmp, 1 * sizeof(unsigned), offset * sizeof(unsigned));
  //fwrite(&tmp, sizeof(unsigned), 1, FreeBlockList);
}

bool
IVBlockManager::ReadValue(unsigned _blk_index, char*& _str, unsigned& _len)
{
  if (_blk_index <= 0) {
    cout << "ReadValue Error: Block index <= 0." << endl;
    _str = NULL;
    _len = 0;
    return false;
  }

  unsigned next_blk = _blk_index; // index of next block
  unsigned Len_left;              // how many bits left to read
  int fd = fileno(ValueFile);

  //fseek(ValueFile, (off_t)BLOCK_SIZE * (_blk_index - 1) + sizeof(unsigned), SEEK_SET);
  off_t offset = (off_t)BLOCK_SIZE * (_blk_index - 1) + sizeof(unsigned);
  pread(fd, &Len_left, 1 * sizeof(unsigned), offset);
  //	fread(&Len_left, sizeof(unsigned), 1, ValueFile);

  _str = new char[Len_left];
  _len = Len_left;
  //cout << "Read Value: _len = " << _len << endl;
  //char *pstr = _str; // pointer to where data in next reading is stored

  if (_str == NULL) {
    cout << "IVBlockManager Error: fail when new" << endl;
    return false;
  }

  do {
    unsigned Bits2read = BLOCK_DATA_SIZE < Len_left ? BLOCK_DATA_SIZE : Len_left;

    offset = (off_t)BLOCK_SIZE * (next_blk - 1);
    pread(fd, &next_blk, 1 * sizeof(unsigned), offset);
    //		fseek(ValueFile, (off_t)BLOCK_SIZE * (next_blk - 1), SEEK_SET);
    //		fread(&next_blk, sizeof(unsigned), 1, ValueFile);
    offset += sizeof(unsigned) + sizeof(unsigned);
    pread(fd, _str + (_len - Len_left), Bits2read * sizeof(char), offset);
    ///		pread(fd, pstr, Bits2read * sizeof(char), offset);
    //		fseek(ValueFile, sizeof(unsigned), SEEK_CUR);
    //		fread(_str + (_len - Len_left), sizeof(char), Bits2read, ValueFile);

    //		pstr += Bits2read;
    Len_left -= Bits2read;

  } while (next_blk > 0 && Len_left > 0);

  //cout << "Read Value Check: _len = " << _len << endl;
  return true;
}

// Alloc free blocks, here len means number of blocks
bool
IVBlockManager::AllocBlock(unsigned len)
{
  if (len == 0)
    return true;

  unsigned SETBLOCKINC = 1 << 8;
  //TODO merge if block ahead is free
  unsigned AllocLen = (unsigned)max((unsigned)len, SETBLOCKINC);
  unsigned index = cur_block_num + 1;

  index_len_map[index] = AllocLen;
  len_index_map.insert(make_pair(AllocLen, index));

  cur_block_num += AllocLen;

  return true;
}

// Get Free Blocks which can fit _len bits and records them in BlockToWrite
bool
IVBlockManager::getWhereToWrite(unsigned _len)
{
  // try to find suitable free blocks in current free block lists
  // AllocNum is number of blocks which can fit in _len bits
  unsigned AllocNum = (unsigned)((_len + BLOCK_DATA_SIZE - 1) / BLOCK_DATA_SIZE);

  // map <unsigned, unsigned>::iterator it = len_index_map.upper_bound(AllocNum - 1);
  set<pair<unsigned, unsigned> >::iterator it = len_index_map.lower_bound(make_pair(AllocNum, (unsigned)0));
  if (it != len_index_map.end()) {
    // prepare BLockToWrite
    if (BlockToWrite != NULL) {
      // delete BlockToWrite;
      BlockInfo* p = BlockToWrite;
      BlockInfo* nextp = p->next;
      while (p != NULL) {
        nextp = p->next;
        delete p;
        p = nextp;
      }
      BlockToWrite = NULL;
    }

    unsigned BaseIndex = it->second;

    for (unsigned i = AllocNum; i > 0; i--) {
      BlockInfo* p = new BlockInfo(BaseIndex + i - 1, BlockToWrite);
      BlockToWrite = p;
    }

    // modify 2 maps
    unsigned newlen = it->first - AllocNum;
    if (newlen > 0) {
      unsigned newindex = BaseIndex + AllocNum;
      index_len_map[newindex] = newlen;
      len_index_map.insert(make_pair(newlen, newindex));
    }
    len_index_map.erase(it);
    index_len_map.erase(BaseIndex);

    return true;
  }
  // if can't. alloc new free blocks
  else {
    //TODO maybe use fragment?
    AllocBlock(AllocNum);
    return getWhereToWrite(_len);
  }
}

unsigned
IVBlockManager::WriteValue(const char* _str, const unsigned _len)
{
  if (!getWhereToWrite(_len)) {
    return false;
  }

  // write _str
  int fd = fileno(ValueFile);
  BlockInfo* p = BlockToWrite;
  char* pstr = (char*)_str; // pointer to buffer of where to write next
  unsigned len_left = _len; // how many bytes left to write

  while (p != NULL) {
    BlockInfo* nextp = p->next;
    unsigned Bits2Write = BLOCK_DATA_SIZE < len_left ? BLOCK_DATA_SIZE : len_left;
    off_t offset = (off_t)(BLOCK_SIZE) * (p->num - 1);
    unsigned NextIndex = 0;
    if (nextp != NULL) {
      NextIndex = nextp->num;
    }
    // write down next block's index
    pwrite(fd, &NextIndex, 1 * sizeof(unsigned), offset);
    offset += sizeof(unsigned);
    // write down how many bits left
    pwrite(fd, &len_left, 1 * sizeof(unsigned), offset);
    offset += sizeof(unsigned);
    // write down value
    pwrite(fd, pstr, Bits2Write * sizeof(char), offset);

    len_left -= Bits2Write;
    pstr += Bits2Write;
    p = nextp;
  }

  if (len_left > 0) {
    cout << "Get Where To Write error: space is not enough" << endl;
    return 0;
  }

  return BlockToWrite->num;
}

// the key is deleted, so free series of blocks, maybe useless if using SSD
bool
IVBlockManager::FreeBlocks(const unsigned index)
{
  if (index == 0)
    return true;

  unsigned _index = index;
  unsigned next_index;
  int fd = fileno(ValueFile);
  unsigned curlen = 1;
  unsigned cur_index = index;

  while (_index > 0) {
    off_t offset = (off_t)(_index - 1) * BLOCK_SIZE;

    pread(fd, &next_index, 1 * sizeof(unsigned), offset);

    if (next_index - index == 1) // continuous
    {
      curlen++;
    } else // not continuous
    {
      // merge if block before or after it is free
      map<unsigned, unsigned>::iterator it;

      it = index_len_map.lower_bound(cur_index);
      if (it != index_len_map.begin()) {
        --it;
        if (it->first + it->second == cur_index) // block before is free
        {
          cur_index = it->first;
          curlen += it->second;
          index_len_map.erase(it);
          len_index_map.erase(make_pair(it->second, it->first));
        }
      }

      it = index_len_map.upper_bound(cur_index);
      if (it != index_len_map.end()) {
        if (curlen + cur_index == it->first) // block after is free
        {
          curlen += it->second;
          auto tmp_key = it->second, tmp_val = it->first;
          index_len_map.erase(it);
          //len_index_map.erase(make_pair(it->second,it->first));
          len_index_map.erase(make_pair(tmp_key, tmp_val));
        }
      }

      index_len_map.insert(make_pair(cur_index, curlen));
      len_index_map.insert(make_pair(curlen, cur_index));

      curlen = 0;
      cur_index = next_index;
    }
    _index = next_index;
  }

  return true;
}

IVBlockManager::~IVBlockManager()
{
  //	if (BlockToWrite != NULL)
  //	{
  //		delete BlockToWrite;
  //		BlockToWrite = NULL;
  //	}
  BlockInfo* p = BlockToWrite;
  while (p != NULL) {
    BlockInfo* np = p->next;
    delete p;
    p = np;
  }
  BlockToWrite = NULL;
  fclose(FreeBlockList);
  fclose(ValueFile);
}
