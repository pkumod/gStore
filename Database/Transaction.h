#pragma once

#include "../Util/Util.h"
#include "Database.h"

typedef unsigned long long txn_id_t;
enum class TransactionState{ WAITING, RUNNING, COMMITTED, ABORTED};


class Transaction {
private:
	txn_id_t TID;
	string db_name;
	Database* db;
	TransactionState state;
	long int start_time;
	long int end_time;
	
public:
	unsigned long long int update_num;
	Transaction(Transaction const&) = delete;
	Transaction(Database* db, string name, long int time, txn_id_t TID);
	~Transaction() {}
	
	inline TransactionState GetState() { return this->state;  }
	inline void SetState(TransactionState _state) { this->state = _state; }
	inline void SetEndTime(long long int time) { end_time = time; }
	inline long GetStartTime() { return start_time;  }
	inline long GetEndTime() { return end_time;  }
};