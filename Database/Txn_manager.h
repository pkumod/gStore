#pragma once

#include "Transaction.h"
#include "../Util/Util.h"

class Txn_manager {
private:
	Database* db;
	string db_name;
	map<txn_id_t, Transaction*> txn_table;
	mutex log_lock;
	mutex db_lock;
	string log_path;
	string all_log_path;
	txn_id_t cnt;

	bool add_transaction(txn_id_t TID, Transaction* txn);
	void rollback(txn_id_t TID);
	inline txn_id_t ArrangeTID();
	void writelog(string str);
	bool undo(string str);
	bool redo(string str);

	inline void lock_log() { log_lock.lock(); }
	inline void unlock_log() { log_lock.unlock(); }
	inline void lock_db() { db_lock.lock(); }
	inline void unlock_db() { db_lock.unlock(); }
public:
	Txn_manager(Txn_manager const&) = delete;
	Txn_manager(Database* db, string db_name);
	~Txn_manager();
	txn_id_t Begin();
	int Commit(txn_id_t TID);
	int Abort(txn_id_t TID);
	int Query(txn_id_t TID, string sparql, bool auto_rollback = false);
	void restore();
	Database* Checkpoint();
	inline Database* GetDatabase() { return this->db; }
	Transaction* Get_Transaction(txn_id_t TID) { return txn_table[TID]; };
	txn_id_t find_latest_txn();
	void abort_all_running();
};