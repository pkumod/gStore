/*=======================================================================
 * File name: IVEntry.h
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last Modified: 2018-01-30
 * Description: head file of IVEntry, the most basic unit of IVArray
 * =====================================================================*/

#include "../../Util/Util.h"
#include "../../Util/VList.h"
#include "../../Util/Version.h"
#include "../../Util/Transaction.h"
#include "../../Util/GraphLock.h"

using namespace std;

class IVEntry
{
private:
	bool usedFlag;   // mark if the entry is used
	bool dirtyFlag;
	bool cacheFlag;
	bool CachePinFlag;
	unsigned store;  //index of block where value is stored
	// pointer to id for LRU list
	int prevID;
	int nextID;
	
	Bstr* value;
	
	GLatch glatch;
	//MVCC
	atomic<bool> is_versioned;
	
	//Version List end point always point to the newest.
	vector<shared_ptr<Version>> vList; 
	//protect Version list. TODO: use free-locked list for better concurrency
	Latch rwLatch;
public:
	IVEntry();
	enum class LatchType { SHARED, EXCLUSIVE};
	void setBstr(const Bstr* _value);
	bool getBstr(char *& _str, unsigned long& _len, bool if_copy = true) const;
	void setBstr(char *_str, unsigned long _len);
	
	void setStore(unsigned _store);
	unsigned getStore() const;

	void setUsedFlag(bool _flag);
	bool isUsed() const;

	void setDirtyFlag(bool _flag);
	bool isDirty() const;
	
	void setCacheFlag(bool _flag);
	bool inCache() const;

	void setCachePinFlag(bool _flag);
	bool isPined();

	void release();

	void Copy(const IVEntry& _entry);

	void setPrev(int ID);
	int getPrev() const;
	void setNext(int ID);
	int getNext() const;
	
	
	
	//MVCC
	void setVersionFlag();
	void clearVersionFlag();
	bool isVersioned() { return is_versioned.load();}
	
	bool checkVersion(TYPE_TXN_ID TID, bool IS_RC);
	int checkheadVersion(TYPE_TXN_ID TID);
	void getProperVersion(TYPE_TXN_ID TID, VDataSet &addarray, VDataSet &delarray);
	void getLatestVersion(TYPE_TXN_ID TID, VDataSet &addarray, VDataSet &delarray);
	
	void version_merge(VDataArray &addarray, VDataArray &delarray, VDataSet &AddSet, VDataSet &DelSet);
	//get exclusive lock before update
	int GetExclusiveLatch(shared_ptr<Transaction> txn, bool has_read); 
	bool InvalidExlusiveLatch(shared_ptr<Transaction> txn, bool has_read); 

	bool UnLatch(shared_ptr<Transaction> txn, LatchType latch_type); //commit

	bool ReadVersion(VDataSet &AddSet, VDataSet &DelSet, shared_ptr<Transaction> txn, bool &latched, bool first_read = false ); //read
	int  WriteVersion(VDataSet &AddSet, VDataSet &DelSet, shared_ptr<Transaction> txn);
	
	void CleanAllVersion();
	~IVEntry();
};
