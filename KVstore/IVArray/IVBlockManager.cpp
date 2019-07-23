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

IVBlockManager::IVBlockManager(string& _filename, string& _mode, unsigned _keynum )
{
	//dir_path = _dir_path;
	FreeBlockList_path = _filename + "_FreeBlockList";
	ValueFile_path = _filename + "_ValueFile";

	if (_mode == "build")
	{
		FreeBlockList = fopen(FreeBlockList_path.c_str(), "w+b");
		ValueFile = fopen(ValueFile_path.c_str(), "w+b");
	}
	else if (_mode == "open")
	{
		FreeBlockList = fopen(FreeBlockList_path.c_str(), "r+b");
		ValueFile = fopen(ValueFile_path.c_str(), "r+b");
	}
	else
	{
		cout << "Error in BLockManager: Invalid Mode" << endl;
		return;
	}

	if (FreeBlockList == NULL || ValueFile == NULL)
	{
		cout << "Error in IVBlockManager: Open error" << endl;
		return;
	}

	index_len_map.clear();
	len_index_map.clear();
	BlockToWrite = NULL;

	if (_mode == "build")
	{
		cur_block_num = 0;
		unsigned SETBLOCKNUM = 1 << 8;
		unsigned requestnum = (unsigned) max(SETBLOCKNUM, _keynum);
		AllocBlock(requestnum);
		//index_len_map.insert(make_pair(1, requestnum));
		//len_index_map.insert(make_pair(requestnum, 1));
	}
	else // open mode
	{
		// read cur_block_num
		int fd = fileno(FreeBlockList);
		pread(fd, &cur_block_num, 1 * sizeof(unsigned), 0);
		// read FreeBlockList and establish 2 map
		unsigned tmp_index = 0, tmp_len;
		off_t offset = 1;

		pread(fd, &tmp_index, 1 * sizeof(unsigned), offset * sizeof(unsigned));
		offset++;

		while (tmp_index > 0)
		{
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
	map <unsigned, unsigned>::iterator it;
	off_t offset = 0;
	int fd = fileno(FreeBlockList);

	pwrite(fd, &cur_block_num, 1 * sizeof(unsigned), offset * sizeof(unsigned));
	offset++;

	for(it = index_len_map.begin(); it != index_len_map.end(); it++)
	{
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
IVBlockManager::ReadValue(unsigned _blk_index, char *&_str, unsigned &_len)
{
	if (_blk_index <= 0)
	{
		cout << "ReadValue Error: Block index <= 0." << endl; 
		_str = NULL;
		_len = 0;
		return false;
	}

	unsigned next_blk = _blk_index; // index of next block
	unsigned pre_left; // how many bits left to read
	unsigned obj_left;
	int fd = fileno(ValueFile);

	// read first x list
	off_t offset = (off_t)(BLOCK_SIZE) * (_blk_index - 1);
	offset += 1 * sizeof(unsigned);
	pread(fd, &pre_left, 1 * sizeof(unsigned), offset);
	pread(fd, &obj_left, 1 * sizeof(unsigned), offset + 1 * sizeof(unsigned));
	obj_left = obj_left * 4 + 24;
	_str = new char[pre_left + obj_left];
	unsigned len1 = pre_left;
	
	while(pre_left > 0 && next_blk > 0)
	{
		unsigned Bits2read = BLOCK_DATA_SIZE < pre_left ? BLOCK_DATA_SIZE : pre_left;
		offset = (off_t)(BLOCK_SIZE) * (next_blk - 1);
		pread(fd, &next_blk, 1 * sizeof(unsigned), offset);
		offset += sizeof(unsigned) + sizeof(unsigned);
		pread(fd, _str + (len1 - pre_left), Bits2read * sizeof(char), offset);
		pre_left -= Bits2read;
	}

	// read second x list
	//offset = (off_t)(BLOCK_SIZE) * (next_blk - 1);
	//offset += 1 * sizeof(unsigned);
	//pread(fd, &obj_left, 1 * sizeof(unsigned), offset);
	//char *_str2 = new char[obj_left];
	unsigned len2 = obj_left;

	while(obj_left > 0 && next_blk > 0)
	{
		unsigned Bits2read = BLOCK_DATA_SIZE < obj_left ? BLOCK_DATA_SIZE : obj_left;
		offset = (off_t)(BLOCK_SIZE) * (next_blk - 1);
		pread(fd, &next_blk, 1 * sizeof(unsigned), offset);
		offset += sizeof(unsigned) + sizeof(unsigned);
		pread(fd, _str + (len2 - obj_left) + len1, Bits2read * sizeof(char), offset);
		obj_left -= Bits2read;
	}

	//char buff[1024];
	//char filepath[1024];
	//memset(buff, 0, 1024);
	//memset(filepath, 0, 1024);
	//sprintf(buff, "/proc/%d/fd/%d", getpid(), fd);
	//readlink(buff, filepath, 1024);


	//_str = new char[len1 + len2];
	_len = len1 + len2;
	//memcpy(_str, _str1, len1);
	//memcpy(_str + len1, _str2, len2);
	
	if (_str == NULL)
	{
		cout << "IVBlockManager Error: fail when new" << endl;
		return false;
	}
	
	//delete[] _str1;
	//delete[] _str2;
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
	unsigned AllocLen = (unsigned) max((unsigned)len, SETBLOCKINC);
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
	unsigned AllocNum = (unsigned) ((_len + BLOCK_DATA_SIZE - 1) / BLOCK_DATA_SIZE);

	// map <unsigned, unsigned>::iterator it = len_index_map.upper_bound(AllocNum - 1);
	set <pair<unsigned, unsigned> >::iterator it = len_index_map.lower_bound(make_pair(AllocNum, (unsigned)0));
	if (it != len_index_map.end())
	{
		// prepare BLockToWrite
		if (BlockToWrite != NULL)
		{
			// delete BlockToWrite;
			BlockInfo *p=BlockToWrite;
			BlockInfo *nextp=p->next;
			while(p!=NULL)
			{
				nextp=p->next;
				delete p;
				p=nextp;
			}
			BlockToWrite = NULL;
		}

		unsigned BaseIndex = it->second;

		for(unsigned i = AllocNum; i > 0; i--)
		{
			BlockInfo *p = new BlockInfo(BaseIndex + i - 1, BlockToWrite);
			BlockToWrite = p;
		}

		// modify 2 maps
		unsigned newlen = it->first - AllocNum;
		if (newlen > 0)
		{
			unsigned newindex = BaseIndex + AllocNum;
			index_len_map[newindex] = newlen;
			len_index_map.insert(make_pair(newlen, newindex));
		}
		len_index_map.erase(it);
		index_len_map.erase(BaseIndex);
		
		return true;
	}
	// if can't. alloc new free blocks
	else
	{
		//TODO maybe use fragment?
		AllocBlock(AllocNum);
		return getWhereToWrite(_len);
	}
}

unsigned
IVBlockManager::WriteValue(const char *_str, const unsigned _len)
{
	unsigned _pre_Size = *((unsigned*)_str + _len / 4 - 6);
	unsigned _pre_Pos = *((unsigned*)_str + _len / 4 - 5);
	unsigned _obj_Size = *((unsigned*)_str + _len / 4 - 4);
	unsigned _obj_Pos = *((unsigned*)_str + _len / 4 - 3);
	unsigned _add_type = *((unsigned*)_str + _len / 4 - 1);

        int fd = fileno(ValueFile);

        unsigned blockToSkip = _pre_Pos / BLOCK_DATA_SIZE;
        unsigned pre_left = _len - _obj_Size * 4 - 24;	

        char *_str1 = new char[pre_left];
	char *_bak1 = _str1;
        memcpy(_str1, _str, pre_left);
        unsigned obj_left = _obj_Size * 4 + 24;
        char *_str2 = new char[obj_left];
	char *_bak2 = _str2;
        memcpy(_str2, _str + pre_left, obj_left);

        char buff[1024];        
	char filepath[1024];
        memset(buff, 0, 1024);
        memset(filepath, 0, 1024);
        sprintf(buff, "/proc/%d/fd/%d", getpid(), fd);
        readlink(buff, filepath, 1024);

//cout << "writevalue:" << _add_type << endl;
//cout << "writevalue" << filepath << "-" << _add_type << "-" << pre_left << "-" << obj_left << endl;
	if (_add_type == 0)
	{
		unsigned add_block = pre_left / BLOCK_DATA_SIZE + obj_left / BLOCK_DATA_SIZE + (pre_left % BLOCK_DATA_SIZE != 0) + (obj_left % BLOCK_DATA_SIZE != 0);
		if (!getWhereToWrite(add_block * BLOCK_DATA_SIZE))
		{
			return false;
		}
		int fd = fileno(ValueFile);
		
		BlockInfo *p = BlockToWrite;
		*((unsigned*)_str + _len / 4 - 2) = p->num;
		*((unsigned*)_str2 + obj_left / 4 - 2) = p->num;
		//cout << "write - add" << p->num << endl;
		while (p != NULL && pre_left > 0)
		{
			BlockInfo *nextp = p->next;
			unsigned Bits2Write = BLOCK_DATA_SIZE < pre_left ? BLOCK_DATA_SIZE:pre_left;
			off_t offset = (off_t)(BLOCK_SIZE) * (p->num - 1);
			unsigned NextIndex = 0;
			if (nextp != NULL)
			{
				NextIndex = nextp->num;
			}
			pwrite(fd, &NextIndex, 1 * sizeof(unsigned), offset);
			offset += sizeof(unsigned);	
			pwrite(fd, &pre_left, 1 * sizeof(unsigned), offset);
			offset += sizeof(unsigned);
			pwrite(fd, _str1, Bits2Write * sizeof(char), offset);
			pre_left -= Bits2Write;
			_str1 += Bits2Write;
			p = nextp;
		}
                while (p != NULL && obj_left > 0)
                {
                        BlockInfo *nextp = p->next;
                        unsigned Bits2Write = BLOCK_DATA_SIZE < obj_left ? BLOCK_DATA_SIZE:obj_left;
                        off_t offset = (off_t)(BLOCK_SIZE) * (p->num - 1);
                        unsigned NextIndex = 0;
                        if (nextp != NULL)
                        {
                                NextIndex = nextp->num;
                        }
                        pwrite(fd, &NextIndex, 1 * sizeof(unsigned), offset);
                        offset += sizeof(unsigned);
                        pwrite(fd, &obj_left, 1 * sizeof(unsigned), offset);
                        offset += sizeof(unsigned);
                        pwrite(fd, _str2, Bits2Write * sizeof(char), offset);
                        obj_left -= Bits2Write;
                        _str2 += Bits2Write;
                        p = nextp;
                }

		if (pre_left > 0 || obj_left > 0)
		{
			cout << "Get Where To Write error: space is not enough" << endl;
			return 0;
		}
		
		delete[] _bak1;
		delete[] _bak2;
		
		return BlockToWrite->num;
	}

	_add_type = _add_type & 0x7fffffff;
	int free_num = int((pre_left + _add_type * 4) / BLOCK_DATA_SIZE) - int(pre_left / BLOCK_DATA_SIZE);

	unsigned _blk_Index = *((unsigned*)_str + _len / 4 - 2);
	unsigned next_blk = _blk_Index;
	off_t offset = (off_t)(BLOCK_SIZE) * (next_blk - 1);
	if (_pre_Pos >= 0)
	{
		offset = (off_t)BLOCK_SIZE * (next_blk - 1);
                pwrite(fd, _str1, 12 * sizeof(char), offset + 2 * sizeof(unsigned));

        	while (blockToSkip > 0 && next_blk > 0)
        	{
                	offset = (off_t)BLOCK_SIZE * (next_blk - 1);
                	pread(fd, &next_blk, 1 * sizeof(unsigned), offset);
                	pwrite(fd, &pre_left, 1 * sizeof(unsigned), offset + 1 * sizeof(unsigned));
	         	pre_left -= BLOCK_DATA_SIZE;
                	_str1 += BLOCK_DATA_SIZE;
                	blockToSkip--;
        	}
	
	        while (pre_left > 0 && next_blk > 0)
        	{
                	unsigned Bits2Write = BLOCK_DATA_SIZE < pre_left ? BLOCK_DATA_SIZE:pre_left;
               		offset = (off_t)BLOCK_SIZE * (next_blk - 1);
                	pread(fd, &next_blk, 1 * sizeof(unsigned), offset);
			pwrite(fd, &pre_left, 1 * sizeof(unsigned), offset + 1 * sizeof(unsigned));
                	pwrite(fd, _str1, Bits2Write * sizeof(char), offset + 2 * sizeof(unsigned));
                	pre_left -= Bits2Write;
                	_str1 += Bits2Write;
       		}	

        	if (pre_left > 0)
        	{
                	if (!getWhereToWrite(pre_left))
                	{
                        	return false;
                	}
                	BlockInfo *p = BlockToWrite;
                	pwrite(fd, &(p->num), 1 * sizeof(unsigned), offset + 1 * sizeof(unsigned));
                	unsigned NextIndex = next_blk;
                	offset = (off_t)(BLOCK_SIZE) * (p->num - 1);
                	pwrite(fd, &NextIndex, 1 * sizeof(unsigned), offset);
                	pwrite(fd, &pre_left, 1 * sizeof(unsigned), offset + 1 * sizeof(unsigned));
                	pwrite(fd, _str1, pre_left, offset + 2 * sizeof(unsigned));
        	}
	}

	// cout << "free:" << free_num << endl;
	if (free_num > 0)
	{
		unsigned tmp_blk, tmp_off;
		for (int i = 0; i < free_num; ++i)
		{
			tmp_blk = next_blk;
			tmp_off = (off_t)BLOCK_SIZE * (tmp_blk - 1);
			pread(fd, &tmp_blk, 1 * sizeof(unsigned), tmp_off);
		}
		pwrite(fd, 0, 1 * sizeof(unsigned), tmp_off);
		FreeBlocks(next_blk);
		pwrite(fd, &tmp_blk, 1 * sizeof(unsigned), offset);
	}


        offset = (off_t)(BLOCK_SIZE) * (next_blk - 1);
        blockToSkip = _obj_Pos / BLOCK_DATA_SIZE;

        while (blockToSkip > 0 && next_blk > 0)
        {
                offset = (off_t)BLOCK_SIZE * (next_blk - 1);
                pread(fd, &next_blk, 1 * sizeof(unsigned), offset);
                pwrite(fd, &obj_left, 1 * sizeof(unsigned), offset + 1 * sizeof(unsigned));
                obj_left -= BLOCK_DATA_SIZE;
                _str2 += BLOCK_DATA_SIZE;
                blockToSkip--;
        }

        while (obj_left > 0 && next_blk > 0)
        {
                unsigned Bits2Write = BLOCK_DATA_SIZE < obj_left ? BLOCK_DATA_SIZE:obj_left;
                offset = (off_t)BLOCK_SIZE * (next_blk - 1);
                pread(fd, &next_blk, 1 * sizeof(unsigned), offset);
                pwrite(fd, &obj_left, 1 * sizeof(unsigned), offset + 1 * sizeof(unsigned));
                pwrite(fd, _str2, Bits2Write * sizeof(char), offset + 2 * sizeof(unsigned));
                obj_left -= Bits2Write;
                _str2 += Bits2Write;
        }

        if (obj_left > 0)
        {
                if (!getWhereToWrite(obj_left))
                {
                        return false;
                }
                BlockInfo *p = BlockToWrite;
                pwrite(fd, &(p->num), 1 * sizeof(unsigned), offset + 1 * sizeof(unsigned));
		unsigned NextIndex = 0;
                offset = (off_t)(BLOCK_SIZE) * (p->num - 1);
                pwrite(fd, &NextIndex, 1 * sizeof(unsigned), offset);
                pwrite(fd, &obj_left, 1 * sizeof(unsigned), offset + 1 * sizeof(unsigned));
                pwrite(fd, _str2, obj_left, offset + 2 * sizeof(unsigned));
	}

	if (next_blk > 0)
	{
		pwrite(fd, 0, 1 * sizeof(unsigned), offset);
		FreeBlocks(next_blk);
	}

	delete[] _bak1;
	delete[] _bak2;

	return _blk_Index;
}

// the key is deleted, so free series of blocks, maybe useless if using SSD
bool 
IVBlockManager::FreeBlocks(const unsigned index)
{
	if (index == 0) return true;

	unsigned _index = index;
	unsigned next_index;
	int fd = fileno(ValueFile);
	unsigned curlen = 1;
	unsigned cur_index = index;

	while (_index > 0)
	{
		off_t offset = (off_t)(_index - 1) * BLOCK_SIZE;

		pread(fd, &next_index, 1 * sizeof(unsigned), offset);

		if (next_index - index == 1) // continuous
		{
			curlen++;
		}
		else // not continuous
		{
			// merge if block before or after it is free
			map <unsigned, unsigned>::iterator it;

			it = index_len_map.lower_bound(cur_index);
			if(it != index_len_map.begin())
			{
				--it;
				if (it->first + it->second == cur_index) // block before is free
				{
					cur_index = it->first;
					curlen += it->second;
					index_len_map.erase(it);
					len_index_map.erase(make_pair(it->second,it->first));
				}
			}

			it = index_len_map.upper_bound(cur_index);
			if(it != index_len_map.end())
			{
				if (curlen + cur_index == it->first) // block after is free
				{
					curlen += it->second;
					auto tmp_key = it->second, tmp_val = it->first;
					index_len_map.erase(it);
					//len_index_map.erase(make_pair(it->second,it->first));
					len_index_map.erase(make_pair(tmp_key,tmp_val));
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
	while (p != NULL)
	{
		BlockInfo *np = p->next;
		delete p;
		p = np;
	}
	BlockToWrite = NULL;
	fclose(FreeBlockList);
	fclose(ValueFile);
}


