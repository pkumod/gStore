/*=========================================================================
 * File name: ISArray.cpp
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last Modified: 2018-02-08
 * Description:
 * Implementation of class ISArray
 * =========================================================================*/

#include "ISArray.h"

ISArray::ISArray()
{
	ISfile = NULL;
	dir_path = "";
	ISfile_name = "";
	// BM = NULL;
	CurEntryNum = 0;
	CurCacheSize = 0;
	CurEntryNumChange = false;
	MAX_CACHE_SIZE = 0;
	cache_head = std::make_shared<ISEntry>();
	cache_tail_id = -1;
}

ISArray::~ISArray()
{
	fclose(ISfile);
	BM.reset();
	BM = nullptr;
	cache_head.reset();
	cache_head = nullptr;
}

ISArray::ISArray(string _dir_path, string _filename, string mode, unsigned long long buffer_size, unsigned _key_num)
{
	dir_path = _dir_path;
	filename = _dir_path + "/" + _filename;
	ISfile_name = filename + "_ISfile";
	CurEntryNumChange = false;
	MAX_CACHE_SIZE = buffer_size;
	cache_head = std::make_shared<ISEntry>();
	cache_tail_id = -1;

	unsigned SETKEYNUM = 1 << 10;

	if (mode == "build")
	{
		CurCacheSize = 0;

		array_.allocBlock(_key_num);
		CurEntryNum = array_.getEntryNum();
		CurEntryNumChange = true;

		BM = std::make_shared<ISBlockManager>(filename, mode, CurEntryNum);

		ISfile = fopen(ISfile_name.c_str(), "w+b");
	
		if (BM == NULL || array_.empty() || ISfile == NULL)
		{
			SLOG_ERROR("Initialize ISArray ERROR");
		}
	}
	else // open mode
	{
		CurCacheSize = 0;
		
		ISfile = fopen(ISfile_name.c_str(), "r+b");
		
		int fd  = fileno(ISfile);

		pread(fd, &CurEntryNum, 1 * sizeof(unsigned), 0);
		
		if (CurEntryNum > 0)
			array_.allocBlock(CurEntryNum-1);
		else
			array_.allocBlock(CurEntryNum);
		
		BM = std::make_shared<ISBlockManager>(filename, mode, CurEntryNum);
		if (BM == NULL)
		{
			SLOG_ERROR(_filename << ": Fail to initialize ISBlockManager");
			exit(0);
		}

		for(unsigned i = 0; i < CurEntryNum; i++)
		{
			unsigned _store;
			off_t offset = (i + 1) * sizeof(unsigned);
			pread(fd, &_store, 1 * sizeof(unsigned), offset);

			array_[i]->setStore(_store);
			array_[i]->setDirtyFlag(false);
		
			if (_store > 0)
			{
				array_[i]->setUsedFlag(true);
			}	
		}
		//TODO PreLoad
//		PreLoad();

		CurEntryNum = array_.getEntryNum();
	}
}

bool
ISArray::PreLoad()
{
	if (array_.empty())
		return false;

	for(unsigned i = 0; i < CurEntryNum; i++)
	{
		if (!array_[i]->isUsed())
			continue;

		unsigned store = array_[i]->getStore();
		char *str = NULL;
		unsigned len = 0;

		if (!BM->ReadValue(store, str, len))
			return false;
		if (CurCacheSize + len > (MAX_CACHE_SIZE >> 1))
			break;
		
		AddInCache(i, str, len);
		
		delete [] str;
	}

	return true;
}

bool
ISArray::save()
{
	// save ValueFile and ISfile
	int fd = fileno(ISfile);

	if (CurEntryNumChange)
		pwrite(fd, &CurEntryNum, 1 * sizeof(unsigned), 0);
	CurEntryNumChange = false;

	for(unsigned i = 0; i < CurEntryNum; i++)
	{
		if (array_[i]->isDirty())
		{
			char *str = NULL;
			unsigned len = 0;
			unsigned _store;
			// probably value has been written but store has not	
			if (array_[i]->isUsed() && array_[i]->getBstr(str, len, false))
			{
				_store = BM->WriteValue(str, len);
				array_[i]->setStore(_store);
			}

			_store = array_[i]->getStore();
//			if (i == 839)
//				cout << filename << " key " << i << " stored in block " << _store << endl;

			off_t offset = (off_t)(i + 1) * sizeof(unsigned);
			pwrite(fd, &_store, 1 * sizeof(unsigned), offset);

			array_[i]->setDirtyFlag(false);

		}

	}

	BM->SaveFreeBlockList();

	return true;
}

// Swap the least used entry out of main memory
bool
ISArray::SwapOut()
{
	int targetID;
	if ((targetID = cache_head->getNext()) == -1)
		return false;

	int nextID = array_[targetID]->getNext();
	//cout << "nextID = " << nextID << endl;
	cache_head->setNext(nextID);
	if (nextID != -1)
	{
		array_[nextID]->setPrev(-1);
	}
	else
	{
		cache_tail_id = -1;
	}

	char *str = NULL;
	unsigned len = 0;
	array_[targetID]->getBstr(str, len, false);
	CurCacheSize -= len;
	if (array_[targetID]->isDirty())
	{
		//TODO recycle free blocks
		unsigned store = BM->WriteValue(str, len);
		if (store == 0)
			return false;
		array_[targetID]->setStore(store);
		//array[targetID].setDirtyFlag(false);
	}
	array_[targetID]->release();
	array_[targetID]->setCacheFlag(false);

	return true;
}

// Add an entry into main memory
bool
ISArray::AddInCache(unsigned _key, char *_str, unsigned _len)
{
	if (_len > MAX_CACHE_SIZE)
	{
		//TODO should write to disk
		return false;
	}
	// ensure there is enough room in main memory
	while (CurCacheSize + _len > MAX_CACHE_SIZE)
	{
		if (!SwapOut())
		{
			SLOG_ERROR("Error in SwapOut: CurCacheSize is " << CurCacheSize << " , MaxSize is " << MAX_CACHE_SIZE << " , need size " << _len);
			// false means cache is empty
			exit(0);
			//	CurCacheSize = 0;
		//	break;
		}
	}

	CurCacheSize += _len;
	array_[_key]->setBstr(_str, _len);
	array_[_key]->setCacheFlag(true);

	if (cache_tail_id == -1)
		cache_head->setNext(_key);
	else
		array_[cache_tail_id]->setNext(_key);

	array_[_key]->setPrev(cache_tail_id);
	array_[_key]->setNext(-1);
	cache_tail_id = _key;

	return true;
}

//Update last used time of array[_key]
bool
ISArray::UpdateTime(unsigned _key)
{
	if (_key == (unsigned) cache_tail_id)
		return true;

	int prevID = array_[_key]->getPrev();
	int nextID = array_[_key]->getNext();
	if (prevID == -1)
		cache_head->setNext(nextID);
	else
		array_[prevID]->setNext(nextID);
	array_[nextID]->setPrev(prevID);

	array_[_key]->setPrev(cache_tail_id);
	array_[_key]->setNext(-1);
	array_[cache_tail_id]->setNext(_key);
	cache_tail_id = _key;

	return true;
}

bool
ISArray::search(unsigned _key, char *&_str, unsigned &_len)
{
	this->AccessLock.lock();
	//	printf("%s search %d: \n", filename.c_str(), _key);
	if (_key >= CurEntryNum || !array_[_key]->isUsed())
	{
		_str = NULL;
		_len = 0;
		this->AccessLock.unlock();
		return false;
	}
	// try to read in main memory
	if (array_[_key]->inCache())
	{
		UpdateTime(_key);
		this->AccessLock.unlock();
		return array_[_key]->getBstr(_str, _len);
	}
//	printf(" need to read disk ");
	// read in disk
	unsigned store = array_[_key]->getStore();
//	cout << "store: " << store << endl;
//	printf("stored in block %d, ", store);
	if (!BM->ReadValue(store, _str, _len))
	{
		this->AccessLock.unlock();
		return false;
	}

	AddInCache(_key, _str, _len);
	char *debug = new char [_len];
	memcpy(debug, _str, _len);
	_str = debug;

	//	printf("str = %s, len = %d\n", _str, _len);
	this->AccessLock.unlock();
	return true;
}

bool
ISArray::insert(unsigned _key, char *_str, unsigned _len)
{
	this->AccessLock.lock();
	if (_key < CurEntryNum && array_[_key]->isUsed())
	{
		SLOG_ERROR("_key is exist:please not add repeadted");
		this->AccessLock.unlock();
		return false;
	}
	
	if (_key >= ISArray::MAX_KEY_NUM)
	{
		SLOG_ERROR(_key << ' ' << MAX_KEY_NUM);
		SLOG_ERROR("ISArray insert error: Key is bigger than MAX_KEY_NUM");
		this->AccessLock.unlock();
		return false;
	}

	// bool Alloc = false;
	//if (CurKeyNum >= CurEntryNum) // need to realloc
	if (_key >= CurEntryNum)
	{
		CurEntryNumChange = true;
		array_.allocBlock(_key);
		CurEntryNum = array_.getEntryNum();
	}

	AddInCache(_key, _str, _len);
	array_[_key]->setUsedFlag(true);
	array_[_key]->setDirtyFlag(true);
	this->AccessLock.unlock();
	return true;
}

bool
ISArray::remove(unsigned _key)
{
	this->AccessLock.lock();
	if (_key >= CurEntryNum || !array_[_key]->isUsed())
	{
		this->AccessLock.unlock();
		return false;
	}


	unsigned store = array_[_key]->getStore();
	BM->FreeBlocks(store);

	array_[_key]->setUsedFlag(false);
	array_[_key]->setDirtyFlag(true);
	array_[_key]->setStore(0);

	if (array_[_key]->inCache())
	{
		RemoveFromLRUQueue(_key);

		char *str = NULL;
		unsigned len = 0;
		array_[_key]->getBstr(str, len, false);
		CurCacheSize -= len;
		array_[_key]->setCacheFlag(false);
	}

	array_[_key]->release();

	this->AccessLock.unlock();
	return true;

}

bool
ISArray::modify(unsigned _key, char *_str, unsigned _len)
{
	this->AccessLock.lock();
	if (_key >= CurEntryNum || !array_[_key]->isUsed())
	{
		this->AccessLock.unlock();
		return false;
	}

	array_[_key]->setDirtyFlag(true);
	if (array_[_key]->inCache())
	{
		RemoveFromLRUQueue(_key);

		char* str = NULL;
		unsigned len = 0;
		array_[_key]->getBstr(str, len, false);

		CurCacheSize -= len;
		array_[_key]->release();
		array_[_key]->setCacheFlag(false);
		//unsigned store = BM->WriteValue(_str, _len);
		//array_[_key]->setStore(store);
		
	}

	unsigned store = array_[_key]->getStore();
	BM->FreeBlocks(store);
	AddInCache(_key, _str, _len);

	this->AccessLock.unlock();
	return true;
	
}

void
ISArray::RemoveFromLRUQueue(unsigned _key)
{
	if (!array_[_key]->inCache())
		return;
	int prevID = array_[_key]->getPrev();
	int nextID = array_[_key]->getNext();

	if (prevID == -1)
		cache_head->setNext(nextID);
	else
		array_[prevID]->setNext(nextID);

	//cout << "next ID: " << nextID << endl;
	if (nextID != -1)
		array_[nextID]->setPrev(prevID); // since array[_key] is not tail, nextp will not be NULL
	else
		cache_tail_id = prevID;

	array_[_key]->setCacheFlag(false);
	array_[_key]->setPrev(-1);
	array_[_key]->setNext(-1);

/*	UpdateTime(_key);
	unsigned PrevID = array_[_key]->getPrev();
	cache_tail_id = PrevID;
	if (PrevID == -1)
		cache_head->setNext(-1);
	else
		array[PrevID].setNext(-1);
*/
	return;
}
