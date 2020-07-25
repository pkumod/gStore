#include "Txn_manager.h"


Txn_manager::Txn_manager(Database *db, string db_name)
{
	this->db = db;
	this->db_name = db_name;
	this->cnt = 100;
	this->log_path = "./" + db_name + ".db/" + "update.log";
	this->all_log_path = "./" + db_name + ".db/" + "update_since_backup.log";
	cnt = 0;
	txn_table.clear();
}

Txn_manager::~Txn_manager()
{
	abort_all_running();
	txn_table.clear();
}

void Txn_manager::writelog(string str)
{
	ofstream out;
	ofstream out_all;
	out.open(this->log_path.c_str(), ios::out | ios::app);
	out_all.open(this->all_log_path.c_str(), ios::out | ios::app);
	out << str << endl;
	out_all << str << endl;
}

bool Txn_manager::add_transaction(txn_id_t TID, Transaction* txn)
{
	//assert(txn_table.find(TID) == txn_table.end());
	txn_table.insert(pair<txn_id_t, Transaction*>(TID, txn));
	return true;
}

//TODO: undo failed
bool Txn_manager::undo(string str)
{
	Util::get_timestamp(str);
	string undo_sparql;
	if (str[0] == 'I') {
		undo_sparql = "DELETE DATA{";
	}
	else if (str[0] == 'D') {
		undo_sparql = "INSERT DATA{";
	}
	else {
		cerr << "wrong undo sparql: " << str << endl;
		return false;
	}
	undo_sparql += str.substr(2, str.length());
	undo_sparql += '}';
	ResultSet rs;
	FILE* output = stdout;
	this->db->query(undo_sparql, rs, output);
	return true;
}

bool Txn_manager::redo(string str)
{
	Util::get_timestamp(str);
	string redo_sparql;
	if (str[0] == 'I') {
		redo_sparql = "INSERT DATA{";
	}
	else if (str[0] == 'D') {
		redo_sparql = "DELETE DATA{";
	}
	else {
		cerr << "wrong redo sparql: " << str << endl;
		return false;
	}
	redo_sparql += str.substr(2, str.length());
	redo_sparql += '}';
	ResultSet rs;
	FILE* output = stdout;
	this->db->query(redo_sparql, rs, output);
	return true;
}

void Txn_manager::rollback(txn_id_t TID)
{
	string line;
	ifstream in;
	in.open(this->log_path.c_str(), ios::in);
	bool scan_over = false;
	while (getline(in, line))
	{
		if (line[0] == 'B') 
		{
			stringstream ss(line);
			string begin;
			txn_id_t tid;
			ss >> begin >> tid;
			if (tid != TID) continue;
			while (getline(in, line))
			{
				if (line[0] == 'A') 
				{
					scan_over = true;
					break;
				}
				undo(line);
			}
		}
		if (scan_over == true) break;
	}
}

inline txn_id_t Txn_manager::ArrangeTID()
{
	srand(time(NULL));
	cnt++;
	return stod(Util::get_timestamp()) * 10000 + (rand() + cnt) % 10000 ;
}

txn_id_t Txn_manager::Begin()
{
	this->lock_db();
	this->lock_log();
	txn_id_t TID = this->ArrangeTID();
	Transaction* txn = new Transaction(this->db, this->db_name, Util::get_cur_time(), TID);
	add_transaction(TID, txn);
	//TODO: starvation prevention
	string log_str = "Begin " + Util::int2string(TID);
	if (txn_table.find(TID) == txn_table.end()) {
		cerr << "wrong transaction id!" << endl;
		return -1;
	}
	writelog(log_str);
	txn_table[TID]->SetState(TransactionState::RUNNING);
	return TID;
}

int Txn_manager::Commit(txn_id_t TID)
{
	string log_str = "Commit " + Util::int2string(TID);
	if (txn_table.find(TID) == txn_table.end()) {
		this->unlock_log();
		this->unlock_db();
		cerr << "wrong transaction id!" << endl;
		return -1;
	}
	else if (txn_table[TID]->GetState() != TransactionState::RUNNING) {
		this->unlock_log();
		this->unlock_db();
		cerr << "transaction not in running state! commit failed";
		return 1;
	}
	writelog(log_str);
	txn_table[TID]->SetState(TransactionState::COMMITTED);
	txn_table[TID]->SetEndTime(Util::get_cur_time());
	this->unlock_log();
	this->unlock_db();
	return 0;
}

int Txn_manager::Abort(txn_id_t TID)
{
	string log_str = "Abort " + Util::int2string(TID);
	if (txn_table.find(TID) == txn_table.end()) {
		this->unlock_log();
		this->unlock_db();
		cerr << "wrong transaction id!" << endl;
		return -1;
	}
	else if (txn_table[TID]->GetState() != TransactionState::RUNNING) {
		this->unlock_log();
		this->unlock_db();
		cerr << "transaction not in running state! Abort failed";
		return 1;
	}
	writelog(log_str);
	rollback(TID);
	txn_table[TID]->SetState(TransactionState::ABORTED);
	txn_table[TID]->SetEndTime(Util::get_cur_time());
	this->unlock_log();
	this->unlock_db();
	return 0;
}

//TODO: transaction query
int Txn_manager::Query(txn_id_t TID, string sparql, bool auto_rollback)
{
	if (txn_table.find(TID) == txn_table.end()) {
		cerr << "wrong transaction id!" << endl;
		return false;
	}
	int ret_val;
	ResultSet rs;
	FILE* output = NULL;
	ret_val = this->db->query(sparql, rs, output);
	if (ret_val < -1)   //non-update query
	{
		bool ret = (ret_val == -100);
		if (auto_rollback && !ret) {
			this->Abort(TID);
			cerr << "query failed, auto rollback triger" << endl;
			return false;
		}
		return ret;
	}
	txn_table[TID]->update_num += ret_val;
	return true;
}

Database* Txn_manager::Checkpoint()
{
	delete this->db;
	this->db = new Database(this->db_name);
	this->db->load();
	return this->db;
}


//TODO:this function is not complete
void Txn_manager::restore()
{
	string line;
	ifstream in;
	in.open(this->log_path.c_str(), ios::in);
	while (getline(in, line)) 
	{
		if (line[0] == 'B')
		{
			vector<string> redo_set;
			bool full_txn = false;
			while (getline(in, line))
			{
				if (line[0] == 'B' || line[0] == 'C')
				{
					break;
				}
				else if (line[0] == 'A')
				{
					if (redo_set.size() != 0)
						continue;
					else
						break; //no update txn
				}
				else {
					redo_set.push_back(line);
				}
			}
			for (int i = 0; i < redo_set.size(); i++)
			{
				redo(redo_set[i]);
			}
		}
	}
}

txn_id_t Txn_manager::find_latest_txn()
{
	map<txn_id_t, Transaction*>::iterator it = txn_table.begin();
	if (it == txn_table.end()) {
		return 0;
	}
	txn_id_t max = it->first;
	bool is_found = false;
	if (it->second->GetState() == TransactionState::RUNNING)
	{
		is_found = true;
	}
	for (; it != txn_table.end(); it++)
	{
		if (it->second->GetStartTime() > txn_table[max]->GetStartTime() && it->second->GetState() == TransactionState::RUNNING)
		{
			max = it->first;
			is_found = true;
		}
	}
	if (is_found)
		return max;
	else
		return 0;
}

void Txn_manager::abort_all_running()
{
	map<txn_id_t, Transaction*>::iterator it = txn_table.begin();
	for (; it != txn_table.end(); it++)
	{
		if (it->second->GetState() == TransactionState::RUNNING)
		{
			Abort(it->first);
		}
	}
}

