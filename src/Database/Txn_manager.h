/*
written by zhangzhe
*/
#pragma once

#include "../Util/Transaction.h"
#include "../Util/Util.h"
#include "../Util/Latch.h"
#include "Database.h"
/*=================================================================
STATUS:
			Begin()            Commit()
	WAITING--------->RUNNING------------>COMMITTED
			 Rollback() |
						|
						v
					 ABORTED

================================================================*/
class Txn_manager {
private:
	Database* db;
	string db_name;
	map<txn_id_t, shared_ptr<Transaction> > txn_table;
	
	string log_path;
	string all_log_path;
	atomic<txn_id_t> cnt;
	TYPE_TS base_ts;
	ofstream out;
	ofstream out_all;
	
	//naive lock table 
	map<txn_id_t, vector<shared_ptr<Transaction> > > waiting_lists;
	
	//locks
	mutex log_lock;
	Latch checkpoint_lock;
	Latch table_lock;
	mutex DirtyKeys_lock;
	
	vector<IDSet> DirtyKeys;
	atomic<int> committed_num = {0};
	bool add_transaction(txn_id_t TID, shared_ptr<Transaction> txn);
	shared_ptr<Transaction> get_transaction(txn_id_t TID);

	inline txn_id_t ArrangeTID();
	inline txn_id_t ArrangeCommitID();
	
	inline TYPE_TS ArrangeTS();
	void writelog(string str);
	bool undo(string str, txn_id_t TID);
	bool redo(string str, txn_id_t TID);
	int Abort(txn_id_t TID);
	
	inline void lock_log() { log_lock.lock(); }
	inline void unlock_log() { log_lock.unlock(); }
	
	void add_dirty_keys(shared_ptr<Transaction> txn);
public:
	Txn_manager(Txn_manager const&) = delete;
	Txn_manager(Database* db, string db_name);
	~Txn_manager();
	
	//Basic 
	txn_id_t Begin(IsolationLevelType isolationlevel = IsolationLevelType::SERIALIZABLE);
	int Commit(txn_id_t TID);
	int Query(txn_id_t TID, string sparql, string & result);
	int Rollback(txn_id_t TID);
	
	//GC
	void Checkpoint();
	
	shared_ptr<Transaction> Get_Transaction(txn_id_t TID) { return txn_table[TID]; };
	txn_id_t find_latest_txn();
	
	inline Database* GetDatabase() { return this->db; }
	
	//exception 
	void abort_all_running();
	
	//DEBUG
	void print_txn_dataset(txn_id_t TID);
	
	//Recovery(not complete)
	void restore();
};
