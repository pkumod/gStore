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
	
	//MVCC
	atomic<bool> is_versioned;
	atomic<TYPE_TXN_ID> txnId;
	
	//not used
	atomic<TYPE_READ_CNT> readCnt;
	
	//Version List head point always point to the newest.
	list<shared_ptr<Version>> vList; 
	//protect Version list. TODO: use free-locked list for better concurrency
	Latch rwLatch;
	
	int TryEntryLock(shared_ptr<Transaction> txn);
	bool UnEntryLock(shared_ptr<Transaction> txn);
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
	shared_ptr<Version> searchVersion(shared_ptr<Transaction> txn);
	shared_ptr<Version> getLatestVersion(shared_ptr<Transaction> txn);
	
	//get exclusive lock before update
	int getExclusiveLatch(shared_ptr<Transaction> txn, bool has_read); 
	bool releaseExlusiveLatch(shared_ptr<Transaction> txn, bool has_read); //different from unlock
	bool invalidExlusiveLatch(shared_ptr<Transaction> txn, bool has_read); //undo

	bool unLatch(shared_ptr<Transaction> txn, LatchType latch_type); //commit

	//int tryExclusiveLock(shared_ptr<Transaction> txn);
	//bool releaseExlusiveLock(shared_ptr<Transaction> txn); 
	//int getExclusiveLock(shared_ptr<Transaction> txn);

	bool readVersion(VDataSet &AddSet, VDataSet &DelSet, shared_ptr<Transaction> txn, bool repeated_read = false); //read
	int  writeVersion(VDataSet &AddSet, VDataSet &DelSet, shared_ptr<Transaction> txn);
	bool addVersion(VDataSet &AddArray, VDataSet &DelArray, shared_ptr<Transaction> txn, bool has_read = false); //write
	
	void cleanAllVersion();
	~IVEntry();
};
