#ifndef _UTIL_VERSION_H
#define _UTIL_VERSION_H


#include "Util.h"

using namespace std;

typedef set<pair<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>> VDataSet;
typedef pair<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID> VData;
typedef vector<pair<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>> VDataArray;
class Version
{
private:
	VDataSet add_set;
	VDataSet del_set;
	atomic<TYPE_TXN_ID> begin_ts;
	atomic<TYPE_TXN_ID> end_ts;
	atomic<TYPE_TXN_ID> txn_id; //write lock
	atomic<TYPE_READ_CNT> read_cnt; //read lock
	//Version* next;
public:
	Version();
	Version(TYPE_TXN_ID _begin_ts, TYPE_TXN_ID _end_ts);
	Version(TYPE_TXN_ID _begin_ts, TYPE_TXN_ID _end_ts, Version* V);
	Version& operator=(const Version& V);
	
	~Version() { add_set.clear(); del_set.clear();};
	void add(VData value);
	void remove(VData value);
	
	//get and set
	inline TYPE_TXN_ID get_begin_ts() const {return this->begin_ts.load();};
	inline TYPE_TXN_ID get_end_ts() const{ return this->end_ts.load();};

	void get_add_set(VDataSet&) const;
	void get_del_set(VDataSet&) const;
	TYPE_TXN_ID get_txn_id() const {return this->txn_id.load();}
	TYPE_READ_CNT get_read_cnt() const { return this->read_cnt.load(); }
		   
		   
	inline void set_begin_ts(TYPE_TXN_ID _begin_ts){ this->begin_ts.store(_begin_ts);};
	inline void set_end_ts(TYPE_TXN_ID _end_ts){ this->end_ts.store(_end_ts);};

	inline void set_add_set(VDataSet _add_set){ this->add_set = _add_set; };
	inline void set_del_set(VDataSet _del_set){ this->del_set = _del_set;}; 
	void set_txn_id(TYPE_TXN_ID _TID) { this->txn_id.store(_TID); } 
	
	//lock
	bool is_exclusive_latched();
	bool is_owned_exclusive_latch(TYPE_TXN_ID _TID);
	bool is_shared_latched();
	bool is_latched(); 
	bool get_exclusive_latch(TYPE_TXN_ID _TID, bool is_SI = false);
	bool exclusive_unlatch(TYPE_TXN_ID _TID, bool is_SI = false);

	//SI only
	void shared_unlatch();
	bool get_shared_latch(TYPE_TXN_ID _TID);
	bool upgrade_latch(TYPE_TXN_ID _TID);
	bool downgrade_latch(TYPE_TXN_ID _TID);
	//write
};


#endif
