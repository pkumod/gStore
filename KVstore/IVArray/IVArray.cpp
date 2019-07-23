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
//	MAX_CACHE_SIZE = 10 * (1 << 18);
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
//cout << _filename << ": Key " << i << " stored in block " << _store << endl;
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
			unsigned len = 0;
			unsigned _store;
			// probably value has been written but store has not	
			if (array[i].isUsed() && array[i].getBstr(str, len, false))
			{
				//TODO Recycle free block
				_store = BM->WriteValue(str, len);

				//cout << ": Key " << i << " stored in block " << _store << endl;
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
		cout << "cache is empty" << endl;
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
	unsigned len = 0;
	array[targetID].getBstr(str, len, false);
	CurCacheSize -= len;
	if (array[targetID].isDirty())
	{
		//TODO recycle free blocks
		unsigned store = BM->WriteValue(str, len);
	//	cout << "SwapOut:" << len << endl;
		if (store == 0)
		{
			cout<< "writeValue return wrong store"<<endl;
			return false;
		}
		array[targetID].setStore(store);
	//	array[targetID].setDirtyFlag(false);
	}
	array[targetID].release();
	array[targetID].setCacheFlag(false);

	return true;
}

// Add an entry into main memory
bool
IVArray::AddInCache(unsigned _key, char *_str, unsigned _len)
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
IVArray::search(unsigned _key, char *&_str, unsigned &_len)
{
	this->CacheLock.lock();
	//printf("%s search %d: ", filename.c_str(), _key);
	if (_key >= CurEntryNum ||!array[_key].isUsed())
	{
		_str = NULL;
		_len = 0;
		this->CacheLock.unlock();
		return false;
	}
	// try to read in main memory
	
	
	//char *tmpStr=NULL;
	
	if (array[_key].inCache())
	{
		UpdateTime(_key);
		bool ret = array[_key].getBstr(_str, _len);
		this->CacheLock.unlock();

		//_str=new char[_len-24];
		//memcpy(_str,tmpStr,_len-24);
		_len -= 24;
		//delete[] tmpStr;	
	
		return ret;
	}
	// read in disk
	unsigned store = array[_key].getStore();
	//cout << "search:" << store << endl;
	if (!BM->ReadValue(store, _str, _len))
	{
		this->CacheLock.unlock();
		return false;
	}
	//cout << "search:" << _key << "---" << _len << endl;	
	if(!VList::isLongList(_len))
	{
//		if (array[_key].Lock.try_lock())
//		{
//			if (array[_key].inCache())
//				return true;
			AddInCache(_key, _str, _len);
		        char *debug = new char[_len];
		        memcpy(debug, _str, _len);
			//_len -= 24;
        		_str = debug;
	//		array[_key].Lock.unlock();
	
//		}
	}
        //char *debug = new char[_len-24];
        //memcpy(debug, tmpStr, _len-24);
        //_str = debug;
        _len -= 24;
	//delete[] tmpStr;
	
	this->CacheLock.unlock();
	return true;
}

bool
IVArray::search(unsigned _key, char *&_str, unsigned &_len, unsigned &_index,int &_in_cache)
{
	this->CacheLock.lock();
	if (_key >= CurEntryNum ||!array[_key].isUsed())
	{
		_str = NULL;
		_len = 0;
		this->CacheLock.unlock();
		return false;
	}

	//char *tmpStr=NULL;
	
	if (array[_key].inCache())
	{
		
		_in_cache=1;
		
		UpdateTime(_key);
		bool ret = array[_key].getBstr(_str, _len);
		_index = *((unsigned*)_str + _len / 4 - 2);
		this->CacheLock.unlock();
		
		//_str=new char[_len-24];
		//memcpy(_str,tmpStr,_len-24);
		_len -= 24;
		//delete[] tmpStr;
		return ret;
	}

	_in_cache=0;	
	
	unsigned store = array[_key].getStore();

        //cout << "new search,store:" << store<<",_key:" <<_key<< endl;
	if (!BM->ReadValue(store, _str, _len))
	{
		this->CacheLock.unlock();
		return false;
	}
	//cout << "new search:" << _key << "---" << _len << endl;
	if(!VList::isLongList(_len))
	{
		AddInCache(_key, _str, _len);
        	char *debug = new char[_len];
        	memcpy(debug, _str, _len);
        	_str = debug;
		//_len -= 24;
		//_index = store;
	}
        //char *debug = new char[_len-24];
        //memcpy(debug, tmpStr, _len-24);
        //_str = debug;
        _len -= 24;
        _index = store;
	//delete[] tmpStr;

	this->CacheLock.unlock();
	return true;
}

bool
IVArray::insert(unsigned _key, char *_str, unsigned _len)
{
	if (_key < CurEntryNum && array[_key].isUsed())
	{
		return false;
	}
	
	if (_key >= IVArray::MAX_KEY_NUM)
	{
		cout << "IVArray insert error: Key is bigger than MAX_KEY_NUM" << endl;
		return false;
	}

	//if (CurKeyNum >= CurEntryNum) // need to realloc
	if (_key >= CurEntryNum)
	{
		CurEntryNumChange = true;
		// temp is the smallest number >= _key and mod SET_KEY_INC = 0
		unsigned temp = ((_key + (1 << 10) - 1) >> 10) << 10;
		unsigned OldEntryNum = CurEntryNum;
//		CurEntryNum = max(CurEntryNum + IVArray::SET_KEY_INC, temp);
		CurEntryNum = IVMIN(OldEntryNum << 1, IVMAXKEYNUM);

		IVEntry* newp = new IVEntry[CurEntryNum];
		if (newp == NULL)
		{
			cout << "IVArray insert error: main memory full" << endl;
			return false;
		}

		for(int i = 0; i < OldEntryNum; i++)
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
//cout<<"writevalue return wrong store"<<endl;
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

	return true;
}

bool
IVArray::remove(unsigned _key)
{
	if (!array[_key].isUsed())
	{
		return false;
	}


	unsigned store = array[_key].getStore();
	BM->FreeBlocks(store);

	array[_key].setUsedFlag(false);
	array[_key].setDirtyFlag(true);

//cout<<"the store of key:"<<_key<<"is set to 0"<<endl;

	array[_key].setStore(0);

	if (array[_key].inCache())
	{
		RemoveFromLRUQueue(_key);
		if(array[_key].isPined())
			array[_key].setCachePinFlag(false);

		char *str = NULL;
		unsigned len = 0;
		array[_key].getBstr(str, len, false);
		CurCacheSize -= len;
		array[_key].setCacheFlag(false);
	}

	if (array[_key].isPined())
		array[_key].setCachePinFlag(false);

	array[_key].release();

	return true;

}

bool
IVArray::modify(unsigned _key, char *_str, unsigned _len)
{
	if (!array[_key].isUsed())
	{
		return false;
	}

	array[_key].setDirtyFlag(true);

	if (array[_key].inCache())
	{
		RemoveFromLRUQueue(_key);
		if(array[_key].isPined())
			array[_key].setCachePinFlag(false);

		char* str = NULL;
		unsigned len = 0;
		array[_key].getBstr(str, len, false);

		array[_key].release();
		CurCacheSize -= len;

		AddInCache(_key, _str, _len);
	}
	else
	{
		unsigned store = array[_key].getStore();
		//BM->FreeBlocks(store);
		*((unsigned*)_str + _len / 4 - 2) = store;
		AddInCache(_key, _str, _len);
	}

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
	unsigned _len = 0;
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
