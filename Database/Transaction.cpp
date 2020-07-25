#include "Transaction.h"

Transaction::Transaction(Database* database, string name, long int time, txn_id_t _TID)
{
	this->db = database;
	this->db_name = name;
	this->start_time = time;
	this->TID = _TID;
	this->state = TransactionState::WAITING;
	this->update_num = 0;
}