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
	array = NULL;
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
	cache_head = new IVEntry;
	cache_tail_id = -1;
}

IVArray::~IVArray()
{
	if (array != NULL)
	{
		delete [] array;
		array = NULL;
	}
	fclose(IVfile);
	delete BM;
	delete cache_head;
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
	cache_head = new IVEntry;
	cache_tail_id = -1;

	unsigned SETKEYNUM = 1 << 10;

	if (mode == "build")
	{
		CurCacheSize = 0;

		// temp is the smallest number >= _key_num and mod SET_KEY_INC = 0
		unsigned temp = ((_key_num + (1 << 10) - 1) >> 10) << 10;
		CurEntryNum = max(temp, SETKEYNUM);
		CurEntryNumChange = true;

		BM = new IVBlockManager(filename, mode, CurEntryNum);
		array = new IVEntry [CurEntryNum];

		IVfile = fopen(IVfile_name.c_str(), "w+b");
	
		if (BM == NULL || array == NULL || IVfile == NULL)
		{
			cout << "Initialize IVArray ERROR" << endl;
		}
	}
	else // open mode
	{
		CurCacheSize = 0;
		
		IVfile = fopen(IVfile_name.c_str(), "r+b");
		if (IVfile == NULL)
		{
			cout << "Error in open " << IVfile_name << endl;
			perror("fopen");
			exit(0);
		}
		
		int fd  = fileno(IVfile);

		pread(fd, &CurEntryNum, 1 * sizeof(unsigned), 0);

		BM = new IVBlockManager(filename, mode, CurEntryNum);
		if (BM == NULL)
		{
			cout << _filename << ": Fail to initialize IVBlockManager" << endl;
			exit(0);
		}

		array = new IVEntry [CurEntryNum];
		if (array == NULL)
		{
			cout << _filename << ": Fail to malloc enough space in main memory for array." << endl;
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

			array[i].setStore(_store);
			array[i].setDirtyFlag(false);
		
			if (_store > 0)
			{
				array[i].setUsedFlag(true);
			}	
		}
		//TODO PreLoad
//		PreLoad();

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
		if (array[i].isDirty())
		{
			char *str = NULL;
			unsigned long len = 0;
			unsigned int _store;
			// probably value has been written but store has not	
			if (array[i].isUsed() && array[i].getBstr(str, len, false))
			{
				//TODO Recycle free block
				_store = BM->WriteValue(str, len);
				array[i].setStore(_store);
			}

			_store = array[i].getStore();
//			if (i == 839)
//				cout << filename << " key " << i << " stored in block " << _store << endl;

			off_t offset = (off_t)(i + 1) * sizeof(unsigned);
			pwrite(fd, &_store, 1 * sizeof(unsigned), offset);

			array[i].setDirtyFlag(false);

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

	int nextID = array[targetID].getNext();
	cache_head->setNext(nextID);
	if (nextID != -1)
	{
		array[nextID].setPrev(-1);
	}
	else // p is tail
	{
		cache_tail_id = -1;
	}

	char *str = NULL;
	unsigned long len = 0;
	array[targetID].getBstr(str, len, false);
	CurCacheSize -= len;
	if (array[targetID].isDirty())
	{
		//TODO recycle free blocks
		unsigned store = BM->WriteValue(str, len);
		if (store == 0)
			return false;
		array[targetID].setStore(store);
	//	array[targetID].setDirtyFlag(false);
	}
	array[targetID].release();
	array[targetID].setCacheFlag(false);

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
			cout << filename << ": swapout error" << endl;
			exit(0);
		}
	}


	if (cache_tail_id == -1)
		cache_head->setNext(_key);
	else
		array[cache_tail_id].setNext(_key);

	array[_key].setPrev(cache_tail_id);
	array[_key].setNext(-1);
	cache_tail_id = _key;

	CurCacheSize += _len;
	array[_key].setBstr(_str, _len);
	array[_key].setCacheFlag(true);

//	this->CacheLock.unlock();
	return true;
}

//Update last used time of array[_key]
bool
IVArray::UpdateTime(unsigned _key, bool HasLock)
{
	if (array[_key].isPined()) // the cache pined should not be swaped out
		return true;

	if (_key == (unsigned) cache_tail_id)// already most recent
		return true;

//	if (!HasLock)
//		this->CacheLock.lock();
//	cout << "UpdateTime: " << _key << endl;
	int prevID = array[_key].getPrev();
	int nextID = array[_key].getNext();

	if (prevID == -1)
		cache_head->setNext(nextID);
	else
		array[prevID].setNext(nextID);

	array[nextID].setPrev(prevID); // since array[_key] is not tail, nextp will not be NULL

	array[_key].setPrev(cache_tail_id);
	array[_key].setNext(-1);
	array[cache_tail_id].setNext(_key);
	cache_tail_id = _key;

//	if (!HasLock)
//		this->CacheLock.unlock();
	return true;
}

bool
IVArray::search(unsigned _key, char *&_str, unsigned long & _len)
{
	//printf("%s search %d: ", filename.c_str(), _key);
	if (_key >= CurEntryNum ||!array[_key].isUsed())
	{
		_str = NULL;
		_len = 0;
		return false;
	}
	// try to read in main memory
	this->CacheLock.lock();
	if (array[_key].inCache())
	{
		UpdateTime(_key);
		this->CacheLock.unlock();
		bool ret = array[_key].getBstr(_str, _len);
		return ret;
	}
	// read in disk
	unsigned store = array[_key].getStore();
	if (!BM->ReadValue(store, _str, _len))
	{
		this->CacheLock.unlock();
		return false;
	}
	if(!VList::isLongList(_len))
	{
//		if (array[_key].Lock.try_lock())
//		{
//			if (array[_key].inCache())
//				return true;
			AddInCache(_key, _str, _len);
			char *debug = new char [_len];
			memcpy(debug, _str, _len);
			_str = debug;
	//		array[_key].Lock.unlock();
	
//		}
	}
	this->CacheLock.unlock();
	return true;
}

bool
IVArray::insert(unsigned _key, char *_str, unsigned long _len)
{
	this->CacheLock.lock();
	if (_key < CurEntryNum && array[_key].isUsed())
	{
		this->CacheLock.unlock();
		return false;
	}
	
	if (_key >= IVArray::MAX_KEY_NUM)
	{
		cout << _key << ' ' << MAX_KEY_NUM << endl;
		cout << "IVArray insert error: Key is bigger than MAX_KEY_NUM" << endl;
		this->CacheLock.unlock();
		return false;
	}

	//if (CurKeyNum >= CurEntryNum) // need to realloc
	if (_key >= CurEntryNum)
	{
		CurEntryNumChange = true;
		// temp is the smallest number >= _key and mod SET_KEY_INC = 0
		// unsigned temp = ((_key + (1 << 10) - 1) >> 10) << 10;
		unsigned OldEntryNum = CurEntryNum;
//		CurEntryNum = max(CurEntryNum + IVArray::SET_KEY_INC, temp);
		CurEntryNum = IVMIN(OldEntryNum << 1, static_cast<unsigned>(IVMAXKEYNUM));
		while (_key > CurEntryNum)
		{
			CurEntryNum = IVMIN(CurEntryNum << 1, static_cast<unsigned>(IVMAXKEYNUM));
		}

		IVEntry* newp = new IVEntry[CurEntryNum];
		if (newp == NULL)
		{
			cout << "IVArray insert error: main memory full" << endl;
			CurEntryNum = OldEntryNum;
			delete[] newp;
			this->CacheLock.unlock();
			return false;
		}

		for(unsigned i = 0; i < OldEntryNum; i++)
			newp[i].Copy(array[i]);

		delete [] array;
		array = newp;

	}

	// TODO maybe sometimes not to write in disk, but stored in main memory
	if (VList::isLongList(_len))
	{
		unsigned store = BM->WriteValue(_str, _len);
		if (store == 0)
		{
			return false;
		}
		array[_key].setStore(store);
	}
	else
	{
		AddInCache(_key, _str, _len);
	}
	//AddInCache(_key, _str, _len);
	
	array[_key].setUsedFlag(true);
	array[_key].setDirtyFlag(true);
	this->CacheLock.unlock();
	return true;
}

bool
IVArray::remove(unsigned _key)
{
	if (!array[_key].isUsed())
	{
		return false;
	}

	this->CacheLock.lock();
	unsigned store = array[_key].getStore();
	BM->FreeBlocks(store);

	array[_key].setUsedFlag(false);
	array[_key].setDirtyFlag(true);
	array[_key].setStore(0);

	if (array[_key].inCache())
	{
		RemoveFromLRUQueue(_key);
		if(array[_key].isPined())
			array[_key].setCachePinFlag(false);

		char *str = NULL;
		unsigned long len = 0;
		array[_key].getBstr(str, len, false);
		CurCacheSize += len;
		array[_key].setCacheFlag(false);
	}

	if (array[_key].isPined())
		array[_key].setCachePinFlag(false);

	array[_key].release();
	this->CacheLock.unlock();
	return true;

}

bool
IVArray::modify(unsigned _key, char *_str, unsigned long _len)
{
	//cout << "this IVArray::modify " << endl;
	if (!array[_key].isUsed())
	{
		
		return false;
	}
	array[_key].setDirtyFlag(true);
	this->CacheLock.lock();
	if (array[_key].inCache())
	{
		RemoveFromLRUQueue(_key);
		
		if(array[_key].isPined())
			array[_key].setCachePinFlag(false);

		char* str = NULL;
		unsigned long len = 0;
		array[_key].getBstr(str, len, false);

		array[_key].release();
		CurCacheSize -= len;
		AddInCache(_key, _str, _len);
	}
	else
	{
		//cout << "free disk and set" << endl;
		unsigned store = array[_key].getStore();
		BM->FreeBlocks(store);
		AddInCache(_key, _str, _len);
		
	}
	this->CacheLock.unlock();
	return true;
	
}

//Pin an entry in cache and never swap out
void
IVArray::PinCache(unsigned _key)
{
	//printf("%s search %d: ", filename.c_str(), _key);
	if (_key >= CurEntryNum ||!array[_key].isUsed())
	{
		return;
	}
	// try to read in main memory
	if (array[_key].inCache())
	{
		RemoveFromLRUQueue(_key);

		array[_key].setCachePinFlag(true);
	
		return;
	}
	// read in disk
	unsigned store = array[_key].getStore();
	char *_str = NULL;
	unsigned long _len = 0;
	if (!BM->ReadValue(store, _str, _len))
	{
		return;
	}

	array[_key].setBstr(_str, _len);
	array[_key].setCacheFlag(true);
	array[_key].setCachePinFlag(true);

	return;
}

void
IVArray::RemoveFromLRUQueue(unsigned _key)
{
	if (!array[_key].inCache() || array[_key].isPined())
		return;

	//this->CacheLock.lock();
	int prevID = array[_key].getPrev();
	int nextID = array[_key].getNext();

	if (prevID == -1)
		cache_head->setNext(nextID);
	else
		array[prevID].setNext(nextID);

	//cout << "next ID: " << nextID << endl;
	if (nextID != -1)
		array[nextID].setPrev(prevID); // since array[_key] is not tail, nextp will not be NULL
	else
		cache_tail_id = prevID;

	array[_key].setCacheFlag(false);
	array[_key].setPrev(-1);
	array[_key].setNext(-1);
	/*UpdateTime(_key, true);
	unsigned PrevID = array[_key].getPrev();
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
		//cout << "_key >= CurEntryNum ||!array[_key].isUsed()......................................................" << endl;
		ArrayUnlock();
		return false;
	}
	// try to read in main memory
	bool ret = array[_key].ReadVersion(AddSet, DelSet, txn, latched, is_firstread);
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
	if(!array[_key].isUsed())
	{
		//cerr << "empty entry!" << endl;
		_str = NULL;
		_len = 0;
		ArrayUnlock();
		this->CacheLock.unlock();
		return true;
	}
	
	
	if (array[_key].inCache())
	{
		UpdateTime(_key);
		this->CacheLock.unlock();
		bool ret = array[_key].getBstr(_str, _len);
		//cout << ret << endl;
		//_str maybe nullptr
		//cout << "get base str success......................................................" << endl;
		ArrayUnlock();
		if(ret == false && latched == true) assert(false);
		return ret;
	}
	
	// read in disk
	unsigned store = array[_key].getStore();
	if (!BM->ReadValue(store, _str, _len))
	{
		this->CacheLock.unlock();
		ArrayUnlock();
		//cout << "base str is null......................................................" << endl;
		return true;
	}
	if(!VList::isLongList(_len) )
	{
//		if (array[_key].Lock.try_lock())
//		{
//			if (array[_key].inCache())
//				return true;
			AddInCache(_key, _str, _len);
			char *debug = new char [_len];
			memcpy(debug, _str, _len);
			_str = debug;
	//		array[_key].Lock.unlock();
	
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
		
		bool ret = array[_key].WriteVersion(addset, delta, txn);
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
	//array[_key].setDirtyFlag(true);
	int ret = array[_key].WriteVersion(delta, delset, txn);
	if(ret != 1) {
		cerr << "write version failed!" << endl;
		txn->SetState(TransactionState::ABORTED);
		ArrayUnlock();
		return false;
	}
	//cout << "array[_key].inCache()" << array[_key].inCache() << endl;
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
			cerr << _key << ' ' << MAX_KEY_NUM << endl;
			cerr << "IVArray insert error: Key is bigger than MAX_KEY_NUM" << endl;
			ArrayUnlock();
			return 0;
		}
		ArrayUnlock();
 		ArrayExclusiveLock(); 
		if(_key >= CurEntryNum) //recheck
		{
			CurEntryNumChange = true;
			//assuming one expand is enough
			unsigned OldEntryNum = CurEntryNum;
			CurEntryNum = IVMIN(OldEntryNum << 1, static_cast<unsigned>(IVMAXKEYNUM));
			while (_key > CurEntryNum)
			{
				CurEntryNum = IVMIN(CurEntryNum << 1, static_cast<unsigned>(IVMAXKEYNUM));
			}

			IVEntry* newp = new IVEntry[CurEntryNum];
			if (newp == NULL)
			{
				cerr << "IVArray insert error: main memory full" << endl;
				CurEntryNum = OldEntryNum;
				delete[] newp;
				ArrayUnlock();
				return 0;
			}

			for(unsigned i = 0; i < OldEntryNum; i++)
				newp[i].Copy(array[i]);

			delete [] array;
			array = newp;
		}
		ArrayUnlock();
		//cerr << "Array lock downgrade !" << endl;
		ArraySharedLock();
	}
	//assert(_key < CurEntryNum);
	
	int ret = array[_key].GetExclusiveLatch(txn, has_read);
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
	bool ret = array[_key].UnLatch(txn, type);
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
	//bool delete_ret = array[_key].deleteUnCommittedVersion(txn);
	bool delete_ret = array[_key].InvalidExlusiveLatch(txn, has_read);
	//cerr << "delete_retdelete_ret TID:" << txn->GetTID() << " " << _key << "    " << delete_ret << endl;
	//bool unlock_ret = array[_key].releaseExlusiveLock(txn);
	ArrayUnlock();
	return delete_ret;
}

//gc

bool
IVArray::CleanDirtyKey(unsigned _key)
{
	ArraySharedLock();
	if (_key >= CurEntryNum || !array[_key].isVersioned())
	{
		ArrayUnlock();
		return false;
	}
	array[_key].CleanAllVersion();
	ArrayUnlock();
	return true;

}