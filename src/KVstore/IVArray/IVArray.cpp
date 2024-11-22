/*=========================================================================
 * File name: IVArray.cpp
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last Modified: 2018-02-08
 * Description:
 * Implementation of class IVArray
 * =========================================================================*/

#include "IVArray.h"

IVArray::IVArray()
{
	IVfile = NULL;
	dir_path = "";
	IVfile_name = "";
	BM = NULL;
	CurEntryNum = 0;
	CurCacheSize = 0;
	CurEntryNumChange = false;
	//index_time_map.clear();
	//time_index_map.clear();
	MAX_CACHE_SIZE = 0;
	cache_head = std::make_shared<IVEntry>();
	cache_tail_id = -1;
}

IVArray::~IVArray()
{
	fclose(IVfile);
	delete BM;
	cache_head.reset();
	cache_head = nullptr;
	//index_time_map.clear();
	//time_index_map.clear();
}

IVArray::IVArray(string _dir_path, string _filename, string mode, unsigned long long buffer_size, unsigned _key_num)
{
//	cout << "Initialize " << _filename << "..." << endl;
	dir_path = _dir_path;
	filename = _dir_path + "/" + _filename;
	IVfile_name = filename + "_IVfile";
	CurEntryNumChange = false;
	//index_time_map.clear();
	//time_index_map.clear();
	MAX_CACHE_SIZE = buffer_size;
//	MAX_CACHE_SIZE = 10 * (1 << 30);
	cache_head = std::make_shared<IVEntry>();
	cache_tail_id = -1;

	unsigned SETKEYNUM = 1 << 10;

	if (mode == "build")
	{
		CurCacheSize = 0;

		// temp is the smallest number >= _key_num and mod SET_KEY_INC = 0
		array_.allocBlock(_key_num);
		CurEntryNum = array_.getEntryNum();
		CurEntryNumChange = true;

		BM = new IVBlockManager(filename, mode, CurEntryNum);

		IVfile = fopen(IVfile_name.c_str(), "w+b");
	
		if (BM == NULL || array_.empty() || IVfile == NULL)
		{
			SLOG_ERROR("Initialize IVArray ERROR");
		}
	}
	else // open mode
	{
		CurCacheSize = 0;
		
		IVfile = fopen(IVfile_name.c_str(), "r+b");
		if (IVfile == NULL)
		{
			SLOG_ERROR("Error in open ");
			perror("fopen");
			exit(0);
		}
		
		int fd  = fileno(IVfile);

		pread(fd, &CurEntryNum, 1 * sizeof(unsigned), 0);

		if (CurEntryNum > 0)
			array_.allocBlock(CurEntryNum-1);
		else
			array_.allocBlock(CurEntryNum);

		BM = new IVBlockManager(filename, mode, CurEntryNum);
		if (BM == NULL)
		{
			SLOG_ERROR(_filename << ": Fail to initialize IVBlockManager");
			exit(0);
		}

//		cout << _filename << " CurEntryNum = " << CurEntryNum << endl;
		for(unsigned i = 0; i < CurEntryNum; i++)
		{
			unsigned _store;
			off_t offset = (i + 1) * sizeof(unsigned);
			pread(fd, &_store, 1 * sizeof(unsigned), offset);

//			if (i % 1000000 == 0)
//			cout << _filename << ": Key " << i << " stored in block " << _store << endl;

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
//	cout << _filename << " Done." << endl;
}

bool
IVArray::save()
{
	// save ValueFile and IVfile
	int fd = fileno(IVfile);

	if (CurEntryNumChange)
		pwrite(fd, &CurEntryNum, 1 * sizeof(unsigned), 0);
	CurEntryNumChange = false;

	for(unsigned i = 0; i < CurEntryNum; i++)
	{
		if (array_[i]->isDirty())
		{
			char *str = NULL;
			unsigned long len = 0;
			unsigned int _store;
			// probably value has been written but store has not	
			if (array_[i]->isUsed() && array_[i]->getBstr(str, len, false))
			{
				//TODO Recycle free block
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
IVArray::SwapOut()
{
	int targetID;
	if ((targetID = cache_head->getNext()) == -1) // cache is empty
	{
		return false;
	}

	int nextID = array_[targetID]->getNext();
	cache_head->setNext(nextID);
	if (nextID != -1)
	{
		array_[nextID]->setPrev(-1);
	}
	else // p is tail
	{
		cache_tail_id = -1;
	}

	char *str = NULL;
	unsigned long len = 0;
	array_[targetID]->getBstr(str, len, false);
	CurCacheSize -= len;
	if (array_[targetID]->isDirty())
	{
		//TODO recycle free blocks
		unsigned store = BM->WriteValue(str, len);
		if (store == 0)
			return false;
		array_[targetID]->setStore(store);
	//	array_[targetID]->setDirtyFlag(false);
	}
	array_[targetID]->release();
	array_[targetID]->setCacheFlag(false);

	return true;
}

// Add an entry into main memory
bool
IVArray::AddInCache(unsigned _key, char *_str, unsigned long _len)
{
	if (_len > MAX_CACHE_SIZE)
	{
		return false;
	}

//	this->CacheLock.lock();
	// ensure there is enough room in main memory
	while (CurCacheSize + _len > MAX_CACHE_SIZE)
	{
		if (!SwapOut())
		{
			SLOG_ERROR(filename << ": swapout error");
			exit(0);
		}
	}


	if (cache_tail_id == -1)
		cache_head->setNext(_key);
	else
		array_[cache_tail_id]->setNext(_key);

	array_[_key]->setPrev(cache_tail_id);
	array_[_key]->setNext(-1);
	cache_tail_id = _key;

	CurCacheSize += _len;
	array_[_key]->setBstr(_str, _len);
	array_[_key]->setCacheFlag(true);

//	this->CacheLock.unlock();
	return true;
}

//Update last used time of array[_key]
bool
IVArray::UpdateTime(unsigned _key, bool HasLock)
{
	if (array_[_key]->isPined()) // the cache pined should not be swaped out
		return true;

	if (_key == (unsigned) cache_tail_id)// already most recent
		return true;

//	if (!HasLock)
//		this->CacheLock.lock();
//	cout << "UpdateTime: " << _key << endl;
	int prevID = array_[_key]->getPrev();
	int nextID = array_[_key]->getNext();

	if (prevID == -1)
		cache_head->setNext(nextID);
	else
		array_[prevID]->setNext(nextID);

	array_[nextID]->setPrev(prevID); // since array[_key] is not tail, nextp will not be NULL

	array_[_key]->setPrev(cache_tail_id);
	array_[_key]->setNext(-1);
	array_[cache_tail_id]->setNext(_key);
	cache_tail_id = _key;

//	if (!HasLock)
//		this->CacheLock.unlock();
	return true;
}

bool
IVArray::search(unsigned _key, char *&_str, unsigned long & _len)
{
	//printf("%s search %d: ", filename.c_str(), _key);
	if (_key >= CurEntryNum ||!array_[_key]->isUsed())
	{
		_str = NULL;
		_len = 0;
		return false;
	}
	// try to read in main memory
	this->CacheLock.lock();
	if (array_[_key]->inCache())
	{
		UpdateTime(_key);
		this->CacheLock.unlock();
		bool ret = array_[_key]->getBstr(_str, _len);
		return ret;
	}
	// read in disk
	unsigned store = array_[_key]->getStore();
	if (!BM->ReadValue(store, _str, _len))
	{
		this->CacheLock.unlock();
		return false;
	}
	if(!VList::isLongList(_len))
	{
//		if (array_[_key]->Lock.try_lock())
//		{
//			if (array_[_key]->inCache())
//				return true;
			AddInCache(_key, _str, _len);
			char *debug = new char [_len];
			memcpy(debug, _str, _len);
			_str = debug;
	//		array_[_key]->Lock.unlock();
	
//		}
	}
	this->CacheLock.unlock();
	return true;
}

bool
IVArray::insert(unsigned _key, char *_str, unsigned long _len)
{
	this->CacheLock.lock();
	if (_key < CurEntryNum && array_[_key]->isUsed())
	{
		this->CacheLock.unlock();
		return false;
	}
	
	if (_key >= IVArray::MAX_KEY_NUM)
	{
		SLOG_ERROR(_key << ' ' << MAX_KEY_NUM);
		SLOG_ERROR("IVArray insert error: Key is bigger than MAX_KEY_NUM");
		this->CacheLock.unlock();
		return false;
	}

	//if (CurKeyNum >= CurEntryNum) // need to realloc
	if (_key >= CurEntryNum)
	{
		CurEntryNumChange = true;
		array_.allocBlock(_key);
		CurEntryNum = array_.getEntryNum();

	}

	// TODO maybe sometimes not to write in disk, but stored in main memory
	if (VList::isLongList(_len))
	{
		array_[_key]->release();
		unsigned store = BM->WriteValue(_str, _len);
		if (store == 0)
		{
			SLOG_ERROR("error: IVArray insert write value failed");
			this->CacheLock.unlock();
			return false;
		}
		array_[_key]->setStore(store);
		delete[] _str;
	}
	else
	{
		AddInCache(_key, _str, _len);
	}
	//AddInCache(_key, _str, _len);
	
	array_[_key]->setUsedFlag(true);
	array_[_key]->setDirtyFlag(true);
	this->CacheLock.unlock();
	return true;
}

bool
IVArray::remove(unsigned _key)
{
	if (!array_[_key]->isUsed())
	{
		return false;
	}

	this->CacheLock.lock();
	unsigned store = array_[_key]->getStore();
	BM->FreeBlocks(store);

	array_[_key]->setUsedFlag(false);
	array_[_key]->setDirtyFlag(true);
	array_[_key]->setStore(0);

	if (array_[_key]->inCache())
	{
		RemoveFromLRUQueue(_key);
		if(array_[_key]->isPined())
			array_[_key]->setCachePinFlag(false);

		char *str = NULL;
		unsigned long len = array_[_key]->getBstrLen();
		CurCacheSize -= len;
		array_[_key]->setCacheFlag(false);
	}

	if (array_[_key]->isPined())
		array_[_key]->setCachePinFlag(false);

	array_[_key]->release();
	this->CacheLock.unlock();
	return true;

}

bool
IVArray::modify(unsigned _key, char *_str, unsigned long _len)
{
	//cout << "this IVArray::modify " << endl;
	if (!array_[_key]->isUsed())
	{
		
		return false;
	}
	array_[_key]->setDirtyFlag(true);
	this->CacheLock.lock();
	if (array_[_key]->inCache())
	{
		RemoveFromLRUQueue(_key);
		
		if(array_[_key]->isPined())
			array_[_key]->setCachePinFlag(false);

		char* str = NULL;
		unsigned long len = array_[_key]->getBstrLen();
		unsigned store = array_[_key]->getStore();
		BM->FreeBlocks(store);
		array_[_key]->release();
		CurCacheSize -= len;

		if (VList::isLongList(_len))
		{
			store = BM->WriteValue(_str, _len);
			if (store == 0)
			{
				SLOG_ERROR("error: IVArray modify in cache write value failed");
				this->CacheLock.unlock();
				return false;
			}
			array_[_key]->setStore(store);
			array_[_key]->setCacheFlag(false);
			delete[] _str;
		}
		else
		{
			array_[_key]->setStore(0);
			AddInCache(_key, _str, _len);
		}
	}
	else
	{
		//cout << "free disk and set" << endl;
		unsigned store = array_[_key]->getStore();
		BM->FreeBlocks(store);
		array_[_key]->release();
		if (VList::isLongList(_len))
		{
			store = BM->WriteValue(_str, _len);
			if (store == 0)
			{
				SLOG_ERROR("error: IVArray modify no cache write value failed");
				this->CacheLock.unlock();
				return false;
			}
			array_[_key]->setStore(store);
			array_[_key]->setCacheFlag(false);
			delete[] _str;
		}
		else
		{
			array_[_key]->setStore(0);
			AddInCache(_key, _str, _len);
		}
	}
	
	array_[_key]->setUsedFlag(true);
	array_[_key]->setDirtyFlag(true);
	this->CacheLock.unlock();
	return true;
	
}

//Pin an entry in cache and never swap out
void
IVArray::PinCache(unsigned _key)
{
	//printf("%s search %d: ", filename.c_str(), _key);
	if (_key >= CurEntryNum ||!array_[_key]->isUsed())
	{
		return;
	}
	// try to read in main memory
	if (array_[_key]->inCache())
	{
		RemoveFromLRUQueue(_key);

		array_[_key]->setCachePinFlag(true);
	
		return;
	}
	// read in disk
	unsigned store = array_[_key]->getStore();
	char *_str = NULL;
	unsigned long _len = 0;
	if (!BM->ReadValue(store, _str, _len))
	{
		return;
	}

	array_[_key]->setBstr(_str, _len);
	array_[_key]->setCacheFlag(true);
	array_[_key]->setCachePinFlag(true);

	return;
}

void
IVArray::RemoveFromLRUQueue(unsigned _key)
{
	if (!array_[_key]->inCache() || array_[_key]->isPined())
		return;

	//this->CacheLock.lock();
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
	/*UpdateTime(_key, true);
	unsigned PrevID = array_[_key]->getPrev();
	cache_tail_id = PrevID;
	if (PrevID == -1)
		cache_head->setNext(-1);
	else
		array[PrevID].setNext(-1);*/

	//this->CacheLock.unlock();
	return;
}


//MVCC

bool 
IVArray::search(unsigned _key, char *& _str, unsigned long & _len, VDataSet& AddSet, VDataSet& DelSet, shared_ptr<Transaction> txn, bool &latched, bool is_firstread )
{
	ArraySharedLock();
	//printf("%s search %d: ", filename.c_str(), _key);
	if (_key >= CurEntryNum)
	{
		_str = NULL;
		_len = 0;
		//cout << "_key >= CurEntryNum ||!array_[_key]->isUsed()......................................................" << endl;
		ArrayUnlock();
		return false;
	}
	// try to read in main memory
	bool ret = array_[_key]->ReadVersion(AddSet, DelSet, txn, latched, is_firstread);
	// bool is_empty = AddSet.size() == 0 && DelSet.size() == 0;
	
	if(ret == false) {
		//cerr << "read version failed, query abort" << endl;
		_str = NULL;
		_len = 0;
		txn->SetState(TransactionState::ABORTED);
		assert(latched == false);
		ArrayUnlock();
		return false;
	}
	this->CacheLock.lock();
	if(!array_[_key]->isUsed())
	{
		//cerr << "empty entry!" << endl;
		_str = NULL;
		_len = 0;
		ArrayUnlock();
		this->CacheLock.unlock();
		return true;
	}
	
	
	if (array_[_key]->inCache())
	{
		UpdateTime(_key);
		this->CacheLock.unlock();
		bool ret = array_[_key]->getBstr(_str, _len);
		//cout << ret << endl;
		//_str maybe nullptr
		//cout << "get base str success......................................................" << endl;
		ArrayUnlock();
		if(ret == false && latched == true) assert(false);
		return ret;
	}
	
	// read in disk
	unsigned store = array_[_key]->getStore();
	if (!BM->ReadValue(store, _str, _len))
	{
		this->CacheLock.unlock();
		ArrayUnlock();
		//cout << "base str is null......................................................" << endl;
		return true;
	}
	if(!VList::isLongList(_len) )
	{
//		if (array_[_key]->Lock.try_lock())
//		{
//			if (array_[_key]->inCache())
//				return true;
			AddInCache(_key, _str, _len);
			char *debug = new char [_len];
			memcpy(debug, _str, _len);
			_str = debug;
	//		array_[_key]->Lock.unlock();
	
//		}
	}
	this->CacheLock.unlock();
	ArrayUnlock();
	return true;
}

bool 
IVArray::remove(unsigned _key, VDataSet& delta, shared_ptr<Transaction> txn)
{
	ArraySharedLock();
	if (_key < CurEntryNum)
	{
		//check if first remove
		VDataSet addset;
		addset.clear();
		
		bool ret = array_[_key]->WriteVersion(addset, delta, txn);
		if(ret == false) txn->SetState(TransactionState::ABORTED);
		ArrayUnlock();
		return ret;
	}
	else
	{
		ArrayUnlock();
		return false;
	}
}

bool 
IVArray::insert(unsigned _key, VDataSet& delta, shared_ptr<Transaction> txn)
{
	ArraySharedLock();
	if(_key >= CurEntryNum) {
		ArrayUnlock();
		return false; //not happen
	}
	VDataSet delset;
	delset.clear();
	//check if first insert 
	//array_[_key]->setDirtyFlag(true);
	int ret = array_[_key]->WriteVersion(delta, delset, txn);
	if(ret != 1) {
		SLOG_ERROR("write version failed!");
		txn->SetState(TransactionState::ABORTED);
		ArrayUnlock();
		return false;
	}
	//cout << "array_[_key]->inCache()" << array_[_key]->inCache() << endl;
	ArrayUnlock();
	return true;
}


int 
IVArray::TryExclusiveLatch(unsigned _key, shared_ptr<Transaction> txn, bool has_read )
{
	ArraySharedLock();
	if(_key >= CurEntryNum) //expand
	{
		//cerr << "expanding..............." << endl;
		if (_key >= IVArray::MAX_KEY_NUM)
		{
			SLOG_ERROR(_key << ' ' << MAX_KEY_NUM);
			SLOG_ERROR("IVArray insert error: Key is bigger than MAX_KEY_NUM");
			ArrayUnlock();
			return 0;
		}
		ArrayUnlock();
 		ArrayExclusiveLock(); 
		if(_key >= CurEntryNum) //recheck
		{
			CurEntryNumChange = true;
			
			array_.allocBlock(_key);
			CurEntryNum = array_.getEntryNum();
		}
		ArrayUnlock();
		//cerr << "Array lock downgrade !" << endl;
		ArraySharedLock();
	}
	//assert(_key < CurEntryNum);
	
	int ret = array_[_key]->GetExclusiveLatch(txn, has_read);
	ArrayUnlock();
	return ret;
}

bool 
IVArray::ReleaseLatch(unsigned _key, shared_ptr<Transaction> txn, IVEntry::LatchType type)
{
	ArraySharedLock();
	if (_key >= CurEntryNum)
	{
		ArrayUnlock();
		return false;
	}
	bool ret = array_[_key]->UnLatch(txn, type);
	ArrayUnlock();
	return ret;
}

bool
IVArray::Rollback(unsigned _key, shared_ptr<Transaction> txn, bool has_read)
{
	ArraySharedLock();
	if (_key >= CurEntryNum)
	{
		ArrayUnlock();
		return false;
	}
	//bool delete_ret = array_[_key]->deleteUnCommittedVersion(txn);
	bool delete_ret = array_[_key]->InvalidExlusiveLatch(txn, has_read);
	//cerr << "delete_retdelete_ret TID:" << txn->GetTID() << " " << _key << "    " << delete_ret << endl;
	//bool unlock_ret = array_[_key]->releaseExlusiveLock(txn);
	ArrayUnlock();
	return delete_ret;
}

//gc

bool
IVArray::CleanDirtyKey(unsigned _key)
{
	ArraySharedLock();
	if (_key >= CurEntryNum || !array_[_key]->isVersioned())
	{
		ArrayUnlock();
		return false;
	}
	array_[_key]->CleanAllVersion();
	ArrayUnlock();
	return true;

}