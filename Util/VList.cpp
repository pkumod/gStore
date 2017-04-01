/*=============================================================================
# Filename: VList.cpp
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2017-03-27 15:47
# Description: 
=============================================================================*/

#include "VList.h"

using namespace std;

bool 
VList::isLongList(unsigned _len)
{
	return _len > VList::LENGTH_BORDER;
}

VList::VList()
{							//not use ../logs/, notice the location of program
	cur_block_num = SET_BLOCK_NUM;
	filepath = "";
	freelist = NULL;
	max_buffer_size = Util::MAX_BUFFER_SIZE;
	freemem = max_buffer_size;
}

VList::VList(string& _filepath, string& _mode, unsigned long long _buffer_size)
{
	cur_block_num = SET_BLOCK_NUM;		//initialize
	this->filepath = _filepath;

	if (_mode == string("build"))
		valfp = fopen(_filepath.c_str(), "w+b");
	else if (_mode == string("open"))
		valfp = fopen(_filepath.c_str(), "r+b");
	else
	{
		cout<<string("error in VList: Invalid mode ") + _mode<<endl;
		return;
	}
	if (valfp == NULL)
	{
		cout<<string("error in VList: Open error ") + _filepath<<endl;
		return;
	}

	this->max_buffer_size = _buffer_size;
	this->freemem = this->max_buffer_size;
	this->freelist = new BlockInfo;	//null-head

	//TODO: read/write by char is too slow, how about read all and deal , then clear?
	//
	//BETTER: hwo about assign IDs in a dynamic way?
	//limitID freelist
	//QUETY: can free id list consume very large memory??

	unsigned i, j, k;	//j = (SuperNum-1)*BLOCK_SIZE
	BlockInfo* bp;
	if (_mode == "build")
	{	//write basic information
		i = 0;
		fwrite(&cur_block_num, sizeof(unsigned), 1, this->valfp);	//current block num
		//NOTICE: use a 1M block for a unsigned?? not ok!
		fseek(this->valfp, BLOCK_SIZE, SEEK_SET);
		bp = this->freelist;
		j = cur_block_num / 8;
		for (i = 0; i < j; ++i)
		{
			fputc(0, this->valfp);
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
		char c;
		fread(&cur_block_num, sizeof(unsigned), 1, this->valfp);
		fseek(this->valfp, BLOCK_SIZE, SEEK_SET);
		bp = this->freelist;
		j = cur_block_num / 8;
		for (i = 0; i < j; ++i)
		{
			c = fgetc(valfp);
			for (k = 0; k < 8; ++k)
			{
				if ((c & (1 << k)) == 0)
				{
					bp->next = new BlockInfo(i * 8 + 7 - k + 1, NULL);
					bp = bp->next;
				}
			}
		}
	}

	//NOTICE: we do not need to alloc the blocks for free block id list, AllocBlock is only for later blocks
}

long		//8-byte in 64-bit machine
VList::Address(unsigned _blocknum) const  //BETTER: inline function
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
VList::Blocknum(long address) const
{
	return (address / BLOCK_SIZE) + 1 - this->SuperNum;
}

unsigned
VList::AllocBlock()
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
VList::FreeBlock(unsigned _blocknum)
{			//QUERY: head-sub and tail-add will be better?
	BlockInfo* bp = new BlockInfo(_blocknum, this->freelist->next);
	this->freelist->next = bp;
}

//NOTICE: all reads are aligned to 4 bytes(including a string)
//a string may acrossseveral blocks
//
//NOTICE: not use buffer, read/write on need, update at once, so no need to write back at last
//NOTICE: the next is placed at the begin of a block


void
VList::ReadAlign(unsigned* _next)
{
	if (ftell(valfp) % BLOCK_SIZE == 0)
	{
		fseek(valfp, Address(*_next), SEEK_SET);
		fread(_next, sizeof(unsigned), 1, valfp);
	}
}

void
VList::WriteAlign(unsigned* _curnum)
{
	if (ftell(valfp) % BLOCK_SIZE == 0)
	{
		unsigned blocknum = this->AllocBlock();
		fseek(valfp, Address(*_curnum), SEEK_SET);
		fwrite(&blocknum, sizeof(unsigned), 1, valfp);
		fseek(valfp, Address(blocknum) + 4, SEEK_SET);
		*_curnum = blocknum;
	}
}

bool 
VList::readValue(unsigned _block_num, char*& _str, unsigned& _len)
{
#ifdef DEBUG_VLIST
	cout<<"to get value of block num: "<<_block_num<<endl;
#endif
	fseek(valfp, Address(_block_num), SEEK_SET);
	unsigned next;
	fread(&next, sizeof(unsigned), 1, valfp);
	this->readBstr(_str, _len, &next);

	return true;
}

unsigned 
VList::writeValue(const char* _str, unsigned _len)
{
	unsigned blocknum = this->AllocBlock();
	unsigned curnum = blocknum;

	//NOTICE: here we must skip the next position first
	fseek(valfp, Address(curnum) + 4, SEEK_SET);
	this->writeBstr(_str, _len, &curnum);

#ifdef DEBUG_VLIST
	cout<<"to write value - block num: "<<blocknum<<endl;
#endif
	return blocknum;
}

bool 
VList::removeValue(unsigned _block_num)
{
	unsigned store = _block_num, next;
	fseek(this->valfp, Address(store), SEEK_SET);
	fread(&next, sizeof(unsigned), 1, valfp);

	while (store != 0)
	{
		this->FreeBlock(store);
		store = next;
		fseek(valfp, Address(store), SEEK_SET);
		fread(&next, sizeof(unsigned), 1, valfp);
	}

	return true;
}

bool
VList::readBstr(char*& _str, unsigned& _len, unsigned* _next)
{
	//long address;
	unsigned len, i, j;
	fread(&len, sizeof(unsigned), 1, this->valfp);
#ifdef DEBUG_VLIST
	cout<<"the length of value: "<<len<<endl;
#endif
	this->ReadAlign(_next);

	//char* s = (char*)malloc(len);
	char* s = new char[len];
	_len = len;

	for (i = 0; i + 4 < len; i += 4)
	{
		fread(s + i, sizeof(char), 4, valfp);
		this->ReadAlign(_next);
	}
	while (i < len)
	{
		fread(s + i, sizeof(char), 1, valfp);	//BETTER
		i++;
	}

	j = len % 4;
	if (j > 0)
		j = 4 - j;
	fseek(valfp, j, SEEK_CUR);

	//NOTICE+DEBUG: I think no need to align here, later no data to read 
	//(if need to read, then fseek again to find a new value)
	//this->ReadAlign(_next);

	_str = s;
	return true;
}

bool
VList::writeBstr(const char* _str, unsigned _len, unsigned* _curnum)
{
	unsigned i, j, len = _len;
	fwrite(&len, sizeof(unsigned), 1, valfp);
	this->WriteAlign(_curnum);
	//cout<<"to write bstr, length: "<<len<<endl;

	//BETTER: compute this need how many blocks first, then write a block a time

	const char* s = _str;
	for (i = 0; i + 4 < len; i += 4)
	{
		fwrite(s + i, sizeof(char), 4, valfp);
		this->WriteAlign(_curnum);
	}
	while (i < len)
	{
		fwrite(s + i, sizeof(char), 1, valfp);
		i++;
	}

	j = len % 4;
	if (j > 0)
		j = 4 - j;
	fseek(valfp, j, SEEK_CUR);

	//NOTICE+DEBUG: I think no need to align here, later no data to write 
	//(if need to write, then fseek again to write a new value)
	//this->WriteAlign(_curnum);
	fseek(valfp, Address(*_curnum), SEEK_SET);
	unsigned t = 0;
	fwrite(&t, sizeof(unsigned), 1, valfp);

	return true;
}

VList::~VList()
{
	//write the info back
	fseek(this->valfp, 0, SEEK_SET);
	fwrite(&cur_block_num, sizeof(unsigned), 1, valfp);//write current blocks num
	fseek(valfp, BLOCK_SIZE, SEEK_SET);
	int i, j = cur_block_num / 8;	//(SuperNum-1)*BLOCK_SIZE;
	for (i = 0; i < j; ++i)
	{
		//reset to 1 first
		fputc(0xff, valfp);
	}
	char c;
	BlockInfo* bp = this->freelist->next;
	while (bp != NULL)
	{
		//if not-use then set 0, aligned to byte!
#ifdef DEBUG_KVSTORE
		if (bp->num > cur_block_num)
		{
			printf("blocks num exceed, cur_block_num: %u\n", cur_block_num);
			exit(1);
		}
#endif
		j = bp->num - 1;
		i = j / 8;
		j = 7 - j % 8;
		fseek(valfp, BLOCK_SIZE + i, SEEK_SET);
		c = fgetc(valfp);
		fseek(valfp, -1, SEEK_CUR);
		fputc(c & ~(1 << j), valfp);
		bp = bp->next;
	}

	bp = this->freelist;
	BlockInfo* next;
	while (bp != NULL)
	{
		next = bp->next;
		delete bp;
		bp = next;
	}
	fclose(this->valfp);
}

