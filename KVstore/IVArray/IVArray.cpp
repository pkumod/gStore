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
	CurKeyNum = 0;
	CurEntryNum = 0;
	CurCacheSize = 0;
	CurEntryNumChange = false;
	index_time_map.clear();
	time_index_map.clear();
	MAX_CACHE_SIZE = 0;
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
	index_time_map.clear();
	time_index_map.clear();
}

IVArray::IVArray(string _dir_path, string _filename, string mode, unsigned long long buffer_size, unsigned _key_num)
{
	cout << "Initialize " << _filename << "...";
	dir_path = _dir_path;
	filename = _dir_path + "/" + _filename;
	IVfile_name = filename + "_IVfile";
	CurEntryNumChange = false;
	index_time_map.clear();
	time_index_map.clear();
	MAX_CACHE_SIZE = buffer_size;

	unsigned SETKEYNUM = 1 << 10;

	if (mode == "build")
	{
		CurCacheSize = 0;

		// temp is the smallest number >= _key_num and mod SET_KEY_INC = 0
		unsigned temp = ((_key_num + (1 << 10) - 1) >> 10) << 10;
		CurKeyNum = 0;
		CurEntryNum = max(temp, SETKEYNUM);
		CurEntryNumChange = true;

		BM = new BlockManager(filename, mode, CurEntryNum);
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
		
		int fd  = fileno(IVfile);

		pread(fd, &CurEntryNum, 1 * sizeof(unsigned), 0);

		BM = new BlockManager(filename, mode, CurEntryNum);
		if (BM == NULL)
		{
			cout << _filename << ": Fail to initialize BlockManager" << endl;
			exit(0);
		}

		array = new IVEntry [CurEntryNum];
		if (array == NULL)
		{
			cout << _filename << ": Fail to malloc enough space in main memory for array." << endl;
			exit(0);
		}
		for(unsigned i = 0; i < CurEntryNum; i++)
		{
			unsigned _store;
			pread(fd, &_store, 1 * sizeof(unsigned), sizeof(unsigned) + i * sizeof(unsigned));

			array[i].setStore(_store);
			array[i].setDirtyFlag(false);
		
			if (_store > 0)
			{
				array[i].setUsedFlag(true);
			}	
		}

	}
	cout << "Done." << endl;
}

bool
IVArray::save()
{
	// save ValueFile and IVfile
	int fd = fileno(IVfile);

	if (CurEntryNumChange)
		pwrite(fd, &CurEntryNum, 1 * sizeof(unsigned), 0);

	for(unsigned i = 0; i < CurEntryNum; i++)
	{
		if (array[i].isDirty())
		{
			char *str = NULL;
			unsigned len = 0;
			unsigned _store;
			// probably value has been written but store has not	
			if (array[i].isUsed() && array[i].getBstr(str, len))
			{
				_store = BM->WriteValue(str, len);
				array[i].setStore(_store);
			}

			_store = array[i].getStore();

			pwrite(fd, &_store, 1 * sizeof(unsigned), i * sizeof(unsigned) + sizeof(unsigned));

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
	if (time_index_map.empty())
	{
		return false;
	}

	multimap <long, unsigned>::iterator it = time_index_map.begin();

	unsigned key = it->second;
	if (array[key].isDirty() && array[key].inCache())
	{
		char *str = NULL;
		unsigned len = 0;
		array[key].getBstr(str, len);
		unsigned store = BM->WriteValue(str, len);
		array[key].setStore(store);
	}
	array[key].release();
	array[key].setCacheFlag(false);

	index_time_map.erase(key);
	time_index_map.erase(it);

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
	// ensure there is enough room in main memory
	while (CurCacheSize + _len > MAX_CACHE_SIZE)
		SwapOut();

	CurCacheSize += _len;
	array[_key].setBstr(_str, _len);
	array[_key].setCacheFlag(true);

	//modify maps
	long time = Util::get_cur_time();
	index_time_map[_key] = time;
	time_index_map.insert(make_pair(time, _key));

	return true;
}

//Update last used time of array[_key]
bool
IVArray::UpdateTime(unsigned _key)
{
	map <unsigned, long>::iterator it;
	if ((it = index_time_map.find(_key)) == index_time_map.end())
	{
		return false;
	}

	unsigned oldtime = it->second;
	long time = Util::get_cur_time();
	it->second = time;

	pair < multimap<long, unsigned>::iterator, multimap<long, unsigned>::iterator > ret;
	ret = time_index_map.equal_range(oldtime);

	multimap <long, unsigned>::iterator p;
	for(p = ret.first; p != ret.second; p++)
	{
		if (p->second == _key)
			break;
	}

	if (p == ret.second)
	{
		return false;
	}
	time_index_map.erase(p);
	time_index_map.insert(make_pair(time, _key));

	return true;
}

bool
IVArray::search(unsigned _key, char *&_str, unsigned &_len)
{
	if (_key >= CurEntryNum ||!array[_key].isUsed())
	{
		cout << "IVArray " << filename << "  Search Error: Key " << _key << " is not available." << endl;
		_str = NULL;
		_len = 0;
		return false;
	}
	// try to read in main memory
	if (array[_key].inCache())
	{
		UpdateTime(_key);
		return array[_key].getBstr(_str, _len);
	}
	// read in disk
	unsigned store = array[_key].getStore();
//	cout << "store: " << store << endl;
	if (!BM->ReadValue(store, _str, _len))
	{
		return false;
	}
	// try to add the entry into cache
/*	if (VList::isLongList(_len) && _len + CurCacheSize <= IVArray::MAX_CACHE_SIZE)
	{
		array[_key].setBstr(_str, _len);
		array[_key].setCacheFlag(true);

		CurCacheSize += _len;
	}*/
	if (!VList::isLongList(_len))
		AddInCache(_key, _str, _len);

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
		cout << _key << ' ' << MAX_KEY_NUM << endl;
		cout << "IVArray insert error: Key is bigger than MAX_KEY_NUM" << endl;
		return false;
	}

	CurKeyNum++;
	//if (CurKeyNum >= CurEntryNum) // need to realloc
	if (_key >= CurEntryNum)
	{
		CurEntryNumChange = true;
		// temp is the smallest number >= _key and mod SET_KEY_INC = 0
		unsigned temp = ((_key + (1 << 10) - 1) >> 10) << 10;
		unsigned OldEntryNum = CurEntryNum;
		CurEntryNum = max(CurEntryNum + IVArray::SET_KEY_INC, temp);
		
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
			return false;
		}
		array[_key].setStore(store);
	}
	else
	{
		AddInCache(_key, _str, _len);
	}
	
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

	CurKeyNum--;

	unsigned store = array[_key].getStore();
	BM->FreeBlocks(store);

	array[_key].setUsedFlag(false);
	array[_key].setDirtyFlag(true);
	array[_key].setStore(0);

	if (array[_key].inCache())
	{
		char *str = NULL;
		unsigned len = 0;
		array[_key].getBstr(str, len);
		CurCacheSize += len;
		array[_key].setCacheFlag(false);
	}

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
		char* str = NULL;
		unsigned len = 0;
		array[_key].getBstr(str, len);

		if (!VList::isLongList(_len))
		{
			CurCacheSize -= len;
			CurCacheSize += _len;
			array[_key].setBstr(_str, _len);
		}
		else
		{
			CurCacheSize -= len;
			array[_key].release();
			array[_key].setCacheFlag(false);
			unsigned store = BM->WriteValue(_str, _len);
			array[_key].setStore(store);
		}
	}
	else
	{
		unsigned store = array[_key].getStore();
		BM->FreeBlocks(store);
		if (VList::isLongList(_len))
		{
			unsigned store = BM->WriteValue(_str, _len);
			array[_key].setStore(store);
		}
		else
		{
			AddInCache(_key, _str, _len);
		}
	}

	return true;
	
}


