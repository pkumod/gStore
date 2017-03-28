/*=============================================================================
# Filename: VList.cpp
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2017-03-27 15:47
# Description: 
=============================================================================*/

#include "VList.h"

using namespace std;

VList::VList()
{							//not use ../logs/, notice the location of program
	cur_block_num = SET_BLOCK_NUM;
	filepath = "";
	freelist = NULL;
	treefp = NULL;
	minheap = NULL;
	max_buffer_size = Util::MAX_BUFFER_SIZE;
	heap_size = max_buffer_size / IVNode::INTL_SIZE;
	freemem = max_buffer_size;
}

VList::VList(string& _filepath, unsigned long long _buffer_size)
{
	cur_block_num = SET_BLOCK_NUM;		//initialize
	this->filepath = _filepath;
	if (_mode == string("build"))
		treefp = fopen(_filepath.c_str(), "w+b");
	else if (_mode == string("open"))
		treefp = fopen(_filepath.c_str(), "r+b");
	else
	{
		print(string("error in IVStorage: Invalid mode ") + _mode);
		return;
	}
	if (treefp == NULL)
	{
		print(string("error in IVStorage: Open error ") + _filepath);
		return;
	}
	this->treeheight = _height;		//originally set to 0
	this->max_buffer_size = _buffer_size;
	this->heap_size = this->max_buffer_size / IVNode::INTL_SIZE;
	this->freemem = this->max_buffer_size;
	this->freelist = new BlockInfo;	//null-head
	unsigned i, j, k;	//j = (SuperNum-1)*BLOCK_SIZE
	BlockInfo* bp;
	if (_mode == "build")
	{	//write basic information
		i = 0;
		fwrite(&i, sizeof(unsigned), 1, this->treefp);	//height
		fwrite(&i, sizeof(unsigned), 1, this->treefp); //rootnum
		fwrite(&cur_block_num, sizeof(unsigned), 1, this->treefp);	//current block num
		fseek(this->treefp, BLOCK_SIZE, SEEK_SET);
		bp = this->freelist;
		j = cur_block_num / 8;
		for (i = 0; i < j; ++i)
		{
			fputc(0, this->treefp);
			for (k = 0; k < 8; ++k)
			{
				bp->next = new BlockInfo(i * 8 + k + 1, NULL);
				bp = bp->next;
			}
		}
	}
	else	//_mode == "open"
	{
		//read basic information
		int rootnum;
		char c;
		fread(this->treeheight, sizeof(unsigned), 1, this->treefp);
		fread(&rootnum, sizeof(unsigned), 1, this->treefp);
		fread(&cur_block_num, sizeof(unsigned), 1, this->treefp);
		fseek(this->treefp, BLOCK_SIZE, SEEK_SET);
		bp = this->freelist;
		j = cur_block_num / 8;
		for (i = 0; i < j; ++i)
		{
			c = fgetc(treefp);
			for (k = 0; k < 8; ++k)
			{
				if ((c & (1 << k)) == 0)
				{
					bp->next = new BlockInfo(i * 8 + 7 - k + 1, NULL);
					bp = bp->next;
				}
			}
		}
		fseek(treefp, Address(rootnum), SEEK_SET);
		//treefp is now ahead of root-block
	}
	this->minheap = new IVHeap(this->heap_size);
}

long		//8-byte in 64-bit machine
IVStorage::Address(unsigned _blocknum) const  //BETTER: inline function
{
	if (_blocknum == 0)
		return 0;
	else if (_blocknum > cur_block_num)
	{
		//print(string("error in Address: Invalid blocknum ") + Util::int2string(_blocknum));
		return -1;		//address should be non-negative
	}
	//NOTICE: here should explictly use long
	return (long)(this->SuperNum + _blocknum - 1) * (long)BLOCK_SIZE;
}

unsigned
IVStorage::Blocknum(long address) const
{
	return (address / BLOCK_SIZE) + 1 - this->SuperNum;
}

unsigned
IVStorage::AllocBlock()
{
	BlockInfo* p = this->freelist->next;
	if (p == NULL)
	{
		for (unsigned i = 0; i < SET_BLOCK_INC; ++i)
		{
			cur_block_num++;	//BETTER: check if > MAX_BLOCK_NUM
			this->FreeBlock(cur_block_num);
		}
		p = this->freelist->next;
	}
	unsigned t = p->num;
	this->freelist->next = p->next;
	delete p;
	return t;
}

void
IVStorage::FreeBlock(unsigned _blocknum)
{			//QUERY: head-sub and tail-add will be better?
	BlockInfo* bp = new BlockInfo(_blocknum, this->freelist->next);
	this->freelist->next = bp;
}

//NOTICE: all reads are aligned to 4 bytes(including a string)
//a string may acrossseveral blocks

void
IVStorage::ReadAlign(unsigned* _next)
{
	if (ftell(treefp) % BLOCK_SIZE == 0)
	{
		fseek(treefp, Address(*_next), SEEK_SET);
		fread(_next, sizeof(unsigned), 1, treefp);
	}
}

void
IVStorage::WriteAlign(unsigned* _curnum, bool& _SpecialBlock)
{
	if (ftell(treefp) % BLOCK_SIZE == 0)
	{
		unsigned blocknum = this->AllocBlock();
		fseek(treefp, Address(*_curnum), SEEK_SET);
		if (_SpecialBlock)
		{
			fseek(treefp, 4, SEEK_CUR);
			_SpecialBlock = false;
		}
		fwrite(&blocknum, sizeof(unsigned), 1, treefp);
		fseek(treefp, Address(blocknum) + 4, SEEK_SET);
		*_curnum = blocknum;
	}
}

bool
IVStorage::readBstr(Bstr* _bp, unsigned* _next)
{
	//long address;
	unsigned len, i, j;
	fread(&len, sizeof(unsigned), 1, this->treefp);
	this->ReadAlign(_next);
	//this->request(len);
	char* s = (char*)malloc(len);
	_bp->setLen(len);
	for (i = 0; i + 4 < len; i += 4)
	{
		fread(s + i, sizeof(char), 4, treefp);
		this->ReadAlign(_next);
	}
	while (i < len)
	{
		fread(s + i, sizeof(char), 1, treefp);	//BETTER
		i++;
	}
	j = len % 4;
	if (j > 0)
		j = 4 - j;
	fseek(treefp, j, SEEK_CUR);
	this->ReadAlign(_next);
	_bp->setStr(s);
	return true;
}

bool
IVStorage::writeBstr(const Bstr* _bp, unsigned* _curnum, bool& _SpecialBlock)
{
	unsigned i, j, len = _bp->getLen();
	fwrite(&len, sizeof(unsigned), 1, treefp);
	this->WriteAlign(_curnum, _SpecialBlock);
	char* s = _bp->getStr();
	for (i = 0; i + 4 < len; i += 4)
	{
		fwrite(s + i, sizeof(char), 4, treefp);
		this->WriteAlign(_curnum, _SpecialBlock);
	}
	while (i < len)
	{
		fwrite(s + i, sizeof(char), 1, treefp);
		i++;
	}
	j = len % 4;
	if (j > 0)
		j = 4 - j;
	fseek(treefp, j, SEEK_CUR);
	this->WriteAlign(_curnum, _SpecialBlock);
	return true;
}

VList::~VList()
{
	//release heap and freelist...
#ifdef DEBUG_KVSTORE
	printf("now to release the kvstore!\n");
#endif
	BlockInfo* bp = this->freelist;
	BlockInfo* next;
	while (bp != NULL)
	{
		next = bp->next;
		delete bp;
		bp = next;
	}
#ifdef DEBUG_KVSTORE
	printf("already empty the freelist!\n");
#endif
	delete this->minheap;
#ifdef DEBUG_KVSTORE
	printf("already empty the buffer heap!\n");
#endif
	fclose(this->treefp);
	//#ifdef DEBUG_KVSTORE
	//NOTICE:there is more than one tree
	//fclose(Util::debug_kvstore);	//NULL is ok!
	//Util::debug_kvstore = NULL;
	//#endif
}

