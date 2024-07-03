/*=============================================================================
# Filename: SINode.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:39
# Description: achieve functions in SINode.h
=============================================================================*/

#include "SINode.h"

using namespace std;

/**
 * new a Bstr Array
 */
void
SINode::AllocKeys()
{
	keys = new Bstr[MAX_KEY_NUM];
}

/**
 * default constructor for SINode
 * Specially In Memory is set to false
 */
SINode::SINode()
{
  store = node_flag_ = 0;
  node_flag_ |= NF_IM;
  heapId = -1;
  AllocKeys();
}

SINode::SINode(bool isVirtual)
{
  store = node_flag_ = 0;
  heapId = -1;
  keys = nullptr;
  if (!isVirtual)
  {
    node_flag_ |= NF_IM;
    AllocKeys();
  }
}

bool
SINode::isLeaf() const
{
	return this->node_flag_ & NF_IL;
}

bool
SINode::isDirty() const
{
	return this->node_flag_ & NF_ID;
}

void
SINode::setDirty()
{
	this->node_flag_ |= NF_ID;
}

void
SINode::delDirty()
{
	this->node_flag_ &= ~NF_ID;
}

bool
SINode::inMem() const
{
	return this->node_flag_ & NF_IM;
}

void
SINode::SetInMem()
{
	this->node_flag_ |= NF_IM;
}

void
SINode::SetMemFlagFalse()
{
	this->node_flag_ &= ~NF_IM;
}

/**
 * Get the rank of SINode. Rank has information of height, children num, dirty, etc.
 * @return rank
 */
unsigned
SINode::getRank() const
{
	return this->node_flag_ & NF_RK;
}

/**
 * Set Rank info.
 * @warning this operation will write height, children num, dirty information.
 * You should make sure the first 8 bit remains the same.
 * @param _rank the new rank
 */
void
SINode::setRank(unsigned _rank)
{
	this->node_flag_ &= ~NF_RK;
	this->node_flag_ |= _rank;
}

unsigned
SINode::getHeight() const
{
	return (this->node_flag_ & NF_HT) >> 20;
}

void
SINode::setHeight(unsigned _h)
{
	this->node_flag_ &= ~NF_HT;
	this->node_flag_ |= (_h << 20);
}

unsigned
SINode::GetKeyNum() const
{
	return (this->node_flag_ & NF_KN) >> 12;
}

bool
SINode::SetKeyNum(int _num)
{
	if (_num < 0 || (unsigned)_num > MAX_KEY_NUM)
	{
		print(string("error in SetKeyNum: Invalid num ") + Util::int2string(_num));
		return false;
	}
	this->node_flag_ &= ~NF_KN;
	this->node_flag_ |= (_num << 12);
	return true;
}

bool
SINode::AddKeyNum()
{
	if (this->GetKeyNum() + 1 > MAX_KEY_NUM)
	{
		print("error in AddKeyNum: Invalid!");
		return false;
	}
	this->node_flag_ += (1 << 12);
	return true;
}

bool
SINode::SubKeyNum()
{
	if (this->GetKeyNum() < 1)
	{
		print("error in SubKeyNum: Invalid!");
		return false;
	}
	this->node_flag_ -= (1 << 12);
	return true;
}

unsigned
SINode::GetStore() const
{
	return this->store;
}

void
SINode::setStore(unsigned _store)
{
	this->store = _store;
}

unsigned
SINode::getFlag() const
{
	return node_flag_;
}

void
SINode::setFlag(unsigned _flag)
{
	this->node_flag_ = _flag;
}

const Bstr*
SINode::getKey(int _index) const
{
	int num = this->GetKeyNum();
	if (_index < 0 || _index >= num)
	{
		//print(string("error in getKey: Invalid index ") + Util::int2string(_index));    
		printf("error in getKey: Invalid index\n");
		return NULL;
	}
	else
		return this->keys + _index;
}

bool
SINode::SetKey(const Bstr* _key, int _index, bool ifcopy)
{
	int num = this->GetKeyNum();
	if (_index < 0 || _index >= num)
	{
		print(string("error in SetKey: Invalid index ") + Util::int2string(_index));
		return false;
	}
	if (ifcopy)
		keys[_index].copy(_key, true);
	else
		keys[_index].assignCopy(*_key);
	return true;
}

bool
SINode::addKey(const Bstr* _key, int _index, bool ifcopy)
{
	if (_key == nullptr)
	{
		SLOG_ERROR("error SINode::addKey");
		return false;
	}
	int num = this->GetKeyNum();
	if (_index < 0 || _index > num)
	{
		SLOG_ERROR(string("error in addKey: Invalid index ") << Util::int2string(_index));
		return false;
	}
	if (num == MAX_KEY_NUM)
	{
		SLOG_ERROR("error addKey MAX_KEY_NUM, should split:" << num);
	}
	int i;
	//NOTICE: if num == MAX_KEY_NUM, will visit keys[MAX_KEY_NUM], not legal!!!
	//however. tree operations ensure that: when node is full, not add but split first!
	for (i = num - 1; i >= _index; --i)
		keys[i + 1] = keys[i];
	if (ifcopy)
		keys[_index].copy(_key);
	else
		keys[_index].assignCopy(*_key);

	return true;
}

bool 
SINode::addKey(char* _str, unsigned _len, int _index, bool ifcopy)
{
	int num = this->GetKeyNum();
	if (_index < 0 || _index > num)
	{
		print(string("error in addKey: Invalid index ") + Util::int2string(_index));
		return false;
	}
	if (num == MAX_KEY_NUM)
	{
		SLOG_ERROR("error addKey MAX_KEY_NUM, should split:" << num);
	}

	int i;
	//NOTICE: if num == MAX_KEY_NUM, will visit keys[MAX_KEY_NUM], not legal!!!
	//however. tree operations ensure that: when node is full, not add but split first!
	for (i = num - 1; i >= _index; --i)
		keys[i + 1].assignCopy(keys[i]);

	keys[_index].setStr(_str);
	keys[_index].setLen(_len);

	return true;
}

bool
SINode::subKey(int _index, bool ifdel)
{
	if (keys == nullptr)
	{
		SLOG_ERROR("error sub keys is null :" << _index);
		return false;
	}
	int num = this->GetKeyNum();
	if (_index < 0 || _index >= num)
	{
		SLOG_ERROR("error sub keys _index >= num :" << _index << "num:" << num);
		return false;
	}
	int i;
	if (ifdel)
	{
		keys[_index].release();
	}
	for (i = _index; i < num - 1; ++i)
		keys[i] = keys[i + 1];

	return true;
}

/**
 * use binary search to find string in keys.
 * @param _bstr string
 * @return the first position where key > _bstr
 */
int
SINode::searchKey_less(const Bstr& _bstr) const
{
	int num = this->GetKeyNum();
	int low = 0, high = num - 1, mid = -1;
	while (low <= high)
	{
		mid = (low + high) / 2;
		if (this->keys[mid] > _bstr)
		{
			if (low == mid)
				break;
			high = mid;
		}
		else
		{
			low = mid + 1;
		}
	}

	return low;
}

int
SINode::searchKey_equal(const Bstr& _bstr) const
{
	int num = this->GetKeyNum();
	//for(i = 0; i < num; ++i)
	//	if(bstr == *(p->getKey(i)))
	//	{

	int ret = this->searchKey_less(_bstr);
	if (ret > 0 && this->keys[ret - 1] == _bstr)
		return ret - 1;
	else
		return num;
}

int
SINode::searchKey_lessEqual(const Bstr& _bstr) const
{
	//int num = this->GetKeyNum();
	//for(i = 0; i < num; ++i)
	//if(bstr <= *(p->getKey(i)))
	//break;

	int ret = this->searchKey_less(_bstr);
	if (ret > 0 && this->keys[ret - 1] == _bstr)
		return ret - 1;
	else
		return ret;
}

/**
 * use binary search to find string in keys.
 * @param _str string pointer
 * @param _len string length
 * @return i, the first position where key[i] > string,
 * 0 if all keys > string
 */
int
SINode::searchKey_less(const char* _str, unsigned _len) const
{
  int num = this->GetKeyNum();

  int low = 0, high = num - 1, mid = -1;
  while (low <= high)
  {
    mid = (low + high) / 2;
    if (Util::compare(this->keys[mid].getStr(), this->keys[mid].getLen(), _str, _len) > 0)
    {
      if (low == mid)
        break;
      high = mid;
    }
    else
      low = mid + 1;
  }
  // case : 0 1 2 -> find(1.5）
  // low = 0, high = 2 mid = 1
  // low = 2, high = 2 mid = 2 break
  // return 2

  // case :  0 1 2 ->find(-1)
  // low = 0, high = 2 mid = 1
  // low = 0, high = 1 mid = 0 out
  // return 0

  // case 0 1 2 ->find(0)
  // low = 0, high = 2 mid = 1
  // low = 0, high = 1 mid = 0
  // low = 1, high = 1 mid = 1 break
  // return 1

  // case 0 1 2 ->find(3) or ->find(2)
  // low = 0, high = 2 mid = 1
  // low = 2, high = 2 mid = 2
  // low = 3, high = 2 break
  // return 3
  return low;
}

int 
SINode::searchKey_equal(const char* _str, unsigned _len) const
{
	int num = this->GetKeyNum();

	int ret = this->searchKey_less(_str, _len);
	//if (ret > 0 && this->keys[ret - 1] == _bstr)
	if (ret > 0 && Util::compare(this->keys[ret-1].getStr(), this->keys[ret-1].getLen(), _str, _len) == 0)
		return ret - 1;
	else
		return num;
}

int 
SINode::searchKey_lessEqual(const char* _str, unsigned _len) const
{
	int ret = this->searchKey_less(_str, _len);
	//if (ret > 0 && this->keys[ret - 1] == _bstr)
	if (ret > 0 && Util::compare(this->keys[ret-1].getStr(), this->keys[ret-1].getLen(), _str, _len) == 0)
		return ret - 1;
	else
		return ret;
}

