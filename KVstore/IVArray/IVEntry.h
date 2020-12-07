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
	atomic<TYPE_TXN_ID> txnID;
	
	//LCV = latest committed version
	atomic<TYPE_READ_CNT> readLCVcnt;
	
	//Version List head point always point to the newest.
	list<shared_ptr<Version>> vList; 
	//protect Version list. TODO: use free-locked list for better concurrency
	Latch rwLatch;


	bool get_shared_latch();
	bool get_exclusive_latch(TYPE_TXN_ID TID, bool is_SI);
	//SI only
	bool upgrade_latch(TYPE_TXN_ID TID);
	bool downgrade_latch(TYPE_TXN_ID TID); //abort
	//unlatch(commit)
	bool shared_unlatch();
	bool exclusive_unlatch(TYPE_TXN_ID TID, bool is_SI);
	//get status
	bool is_exclusive_latched();
	bool is_shared_latched();
	bool is_latched();
	TYPE_READ_CNT get_readers();
	bool is_owned_exclusive_latched(TYPE_TXN_ID TID);
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
	shared_ptr<Version> searchVersion(TYPE_TXN_ID TID);
	shared_ptr<Version> getLatestVersion(TYPE_TXN_ID TID);
	
	//get exclusive lock before update
	int getExclusiveLatch(shared_ptr<Transaction> txn, bool has_read); 
	bool releaseExlusiveLatch(shared_ptr<Transaction> txn, bool has_read); //different from unlock
	bool invalidExlusiveLatch(shared_ptr<Transaction> txn, bool has_read); //undo

	bool unLatch(shared_ptr<Transaction> txn, LatchType latch_type); //commit

	bool readVersion(VDataSet &AddSet, VDataSet &DelSet, shared_ptr<Transaction> txn, bool repeated_read = false); //read
	int  writeVersion(VDataSet &AddSet, VDataSet &DelSet, shared_ptr<Transaction> txn);
	
	void cleanAllVersion();
	~IVEntry();
};
