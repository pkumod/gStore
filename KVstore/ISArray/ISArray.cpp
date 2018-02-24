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
	array = NULL;
	ISfile = NULL;
	dir_path = "";
	ISfile_name = "";
	BM = NULL;
	CurKeyNum = 0;
	CurEntryNum = 0;
	CurCacheSize = 0;
	CurEntryNumChange = false;
	index_time_map.clear();
	time_index_map.clear();
	MAX_CACHE_SIZE = 0;
}

ISArray::~ISArray()
{
	if (array != NULL)
	{
		delete [] array;
		array = NULL;
	}
	fclose(ISfile);
	delete BM;
	index_time_map.clear();
	time_index_map.clear();
}

ISArray::ISArray(string _dir_path, string _filename, string mode, unsigned long long buffer_size, unsigned _key_num)
{
	dir_path = _dir_path;
	filename = _dir_path + "/" + _filename;
	ISfile_name = filename + "_ISfile";
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

		BM = new ISBlockManager(filename, mode, CurEntryNum);
		array = new ISEntry [CurEntryNum];

		ISfile = fopen(ISfile_name.c_str(), "w+b");
	
		if (BM == NULL || array == NULL || ISfile == NULL)
		{
			cout << "Initialize ISArray ERROR" << endl;
		}
	}
	else // open mode
	{
		CurCacheSize = 0;
		
		ISfile = fopen(ISfile_name.c_str(), "r+b");
		
		int fd  = fileno(ISfile);

		pread(fd, &CurEntryNum, 1 * sizeof(unsigned), 0);

		BM = new ISBlockManager(filename, mode, CurEntryNum);
		if (BM == NULL)
		{
			cout << _filename << ": Fail to initialize ISBlockManager" << endl;
			exit(0);
		}

		array = new ISEntry [CurEntryNum];
		if (array == NULL)
		{
			cout << _filename << ": Fail to malloc enough space in main memory for array." << endl;
			exit(0);
		}
		for(unsigned i = 0; i < CurEntryNum; i++)
		{
			unsigned _store;
			off_t offset = (i + 1) * sizeof(unsigned);
			pread(fd, &_store, 1 * sizeof(unsigned), offset);

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
}

bool
ISArray::PreLoad()
{
	if (array == NULL)
		return false;

	for(unsigned i = 0; i < CurEntryNum; i++)
	{
		if (!array[i].isUsed())
			continue;

		unsigned store = array[i].getStore();
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
ISArray::SwapOut()
{
	if (time_index_map.empty())
	{
		return false;
	}

	multimap <long, unsigned>::iterator it = time_index_map.begin();

	unsigned key = it->second;
	unsigned len = 0;
	char *str = NULL;
	array[key].getBstr(str, len);

	if (array[key].isDirty() && array[key].inCache())
	{
		//char *str = NULL;
		//array[key].getBstr(str, len);
		unsigned store = BM->WriteValue(str, len);
		array[key].setStore(store);
	}

	CurCacheSize -= len;
	delete [] str;

	array[key].release();
	array[key].setCacheFlag(false);

	index_time_map.erase(key);
	time_index_map.erase(it);

	return true;
}

// Add an entry into main memory
bool
ISArray::AddInCache(unsigned _key, char *_str, unsigned _len)
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
ISArray::UpdateTime(unsigned _key)
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
ISArray::search(unsigned _key, char *&_str, unsigned &_len)
{
	//printf("%s search %d: \n", filename.c_str(), _key);
	if (_key >= CurEntryNum ||!array[_key].isUsed())
	{
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
//	printf(" need to read disk ");
	// read in disk
	unsigned store = array[_key].getStore();
//	cout << "store: " << store << endl;
//	printf("stored in block %d, ", store);
	if (!BM->ReadValue(store, _str, _len))
	{
		return false;
	}

	AddInCache(_key, _str, _len);

	return true;
}

bool
ISArray::insert(unsigned _key, char *_str, unsigned _len)
{
	if (_key < CurEntryNum && array[_key].isUsed())
	{
		return false;
	}
	
	if (_key >= ISArray::MAX_KEY_NUM)
	{
		cout << _key << ' ' << MAX_KEY_NUM << endl;
		cout << "ISArray insert error: Key is bigger than MAX_KEY_NUM" << endl;
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
		CurEntryNum = max(CurEntryNum + ISArray::SET_KEY_INC, temp);
		
		ISEntry* newp = new ISEntry[CurEntryNum];
		if (newp == NULL)
		{
			cout << "ISArray insert error: main memory full" << endl;
			return false;
		}

		for(int i = 0; i < OldEntryNum; i++)
			newp[i].Copy(array[i]);

		delete [] array;
		array = newp;

	}

	AddInCache(_key, _str, _len);
	array[_key].setUsedFlag(true);
	array[_key].setDirtyFlag(true);
	return true;
}

bool
ISArray::remove(unsigned _key)
{
	if (_key >= CurEntryNum || !array[_key].isUsed())
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
ISArray::modify(unsigned _key, char *_str, unsigned _len)
{
	if (_key >= CurEntryNum ||!array[_key].isUsed())
	{
		return false;
	}

	array[_key].setDirtyFlag(true);
	if (array[_key].inCache())
	{
		char* str = NULL;
		unsigned len = 0;
		array[_key].getBstr(str, len);

		CurCacheSize -= len;
		array[_key].release();
		array[_key].setCacheFlag(false);
		unsigned store = BM->WriteValue(_str, _len);
		array[_key].setStore(store);
		
	}
	else
	{
		unsigned store = array[_key].getStore();
		BM->FreeBlocks(store);
		AddInCache(_key, _str, _len);
		
	}

	return true;
	
}


