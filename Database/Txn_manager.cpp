#include "Txn_manager.h"


Txn_manager::Txn_manager(Database *db, string db_name)
{
	this->db = db;
	this->db_name = db_name;
	this->log_path = "./" + db_name + ".db/" + "update.log";
	this->all_log_path = "./" + db_name + ".db/" + "update_since_backup.log";
	out.open(this->log_path.c_str(), ios::out | ios::app);
	out_all.open(this->all_log_path.c_str(), ios::out | ios::app);
	cnt.store(1);
	txn_table.clear();
	for(int i = 0; i < 3; i++)
	{
		IDSet s;
		this->DirtyKeys.push_back(s);
	}
}

Txn_manager::~Txn_manager()
{
	abort_all_running();
	Checkpoint();
	cout << "Checkpoint done" << endl;
	txn_table.clear();
	out.close();
	out_all.close();
}

void Txn_manager::writelog(string str)
{
	this->lock_log();
	out << str << endl;
	out_all << str << endl;
	this->unlock_log();
}

bool Txn_manager::add_transaction(txn_id_t TID, shared_ptr<Transaction> txn)
{
	//assert(txn_table.find(TID) == txn_table.end());
	table_lock.lockExclusive();
	txn_table.insert(pair<txn_id_t,shared_ptr<Transaction> >(TID, txn));
	table_lock.unlock();
	return true;
}

shared_ptr<Transaction> Txn_manager::get_transaction(txn_id_t TID)
{
	table_lock.lockShared();
	if(txn_table.find(TID) == txn_table.end()) {
		table_lock.unlock();
		cerr << "wrong TID" << endl;
		return nullptr;
	}
	auto p =  txn_table[TID];
	table_lock.unlock();
	return p;
}

//TODO: undo failed
bool Txn_manager::undo(string str, txn_id_t TID)
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
	shared_ptr<Transaction> txn = get_transaction(TID);
	if(db != nullptr)
		this->db->query(undo_sparql, rs, output);
	else
	{
		cout << "error! database has been flushed or removed!" << endl;
		return false;
	}
	return true;
}

bool Txn_manager::redo(string str, txn_id_t TID)
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
	shared_ptr<Transaction> txn = get_transaction(TID); 
	if(db != nullptr)
		this->db->query(redo_sparql, rs, output);
	else
	{
		cout << "error! database has been flushed or removed" << endl;
		return false;
	}
	return true;
}


/*
inline txn_id_t Txn_manager::ArrangeTID()
{
	srand(time(NULL));
	cnt++;
	return stod(Util::get_timestamp()) * 10000 + (rand() + cnt) % 10000 ;
}
*/

inline txn_id_t Txn_manager::ArrangeTID()
{
	txn_id_t TID = cnt.fetch_add(1);
	if(TID == INVALID_ID)
	{
		cerr << "TID wrapped! " << endl;
	}
	return TID;
}

inline txn_id_t Txn_manager::ArrangeCommitID()
{
	return cnt.load();
}

txn_id_t Txn_manager::Begin(IsolationLevelType isolationlevel)
{
	checkpoint_lock.lockShared();
	txn_id_t TID = this->ArrangeTID();
	if(TID == INVALID_ID)
	{
		cout << "TID wrapped, please run garbage clean!" << endl;
		checkpoint_lock.unlock();
		return TID;
	}
	shared_ptr<Transaction> txn = make_shared<Transaction>(this->db_name, Util::get_cur_time(), TID, isolationlevel);
	txn->SetCommitID(TID);
	add_transaction(TID, txn);
	string log_str = "Begin " + Util::int2string(TID);
	//writelog(log_str);
	txn->SetState(TransactionState::RUNNING);
	return TID;
}

int Txn_manager::Commit(txn_id_t TID)
{
	string log_str = "Commit " + Util::int2string(TID);
	shared_ptr<Transaction> txn = get_transaction(TID);
	if (txn == nullptr) {
		cerr << "wrong transaction id!" << endl;
		checkpoint_lock.unlock();
		return -1;
	}
	else if (txn->GetState() != TransactionState::RUNNING) {
		cerr << "transaction not in running state! commit failed" << " " << (int)txn->GetState() << endl;
		checkpoint_lock.unlock();
		return 1;
	}
	txn_id_t CID = this->ArrangeCommitID();
	txn->SetCommitID(CID);
	if(db != nullptr)
		db->TransactionCommit(txn);
	else
	{
		cout << "error! database has been flushed or removed" << endl;
		checkpoint_lock.unlock();
		return -1;
	}
	//writelog(log_str);
	txn->SetState(TransactionState::COMMITTED);
	txn->SetEndTime(Util::get_cur_time());
	add_dirty_keys(txn);
	checkpoint_lock.unlock();
	committed_num++;
	int cycle = 50000;
	if(committed_num.compare_exchange_strong(cycle, 0)){
		Checkpoint();
		cerr << "checkpoint done!" << endl;
	}
	return 0;
}

int Txn_manager::Abort(txn_id_t TID)
{
	string log_str = "Abort " + Util::int2string(TID);
	shared_ptr<Transaction> txn = get_transaction(TID);
	if (txn == nullptr) {
		cerr << "wrong transaction id!" << endl;
		return -1;
	}
	if(db != nullptr)
		db->TransactionRollback(txn);
	else
	{
		cout << "error! database has been flushed or removed" << endl;
		return -1;
	}
	//writelog(log_str);
	txn->SetState(TransactionState::ABORTED);
	txn->SetEndTime(Util::get_cur_time());
	checkpoint_lock.unlock();
	//add_dirty_keys(txn);
	return 0;
}

int Txn_manager::Rollback(txn_id_t TID)
{
	shared_ptr<Transaction> txn = get_transaction(TID);
	if (txn == nullptr) {
		cerr << "wrong transaction id!" << endl;
		return -1;
	}
	else if (txn->GetState() != TransactionState::RUNNING) {
		cerr << "transaction not in running state! rollback failed" << endl;
		return 1;
	}
	return Abort(TID);
}

int Txn_manager::Query(txn_id_t TID, string sparql, string& results)
{
	shared_ptr<Transaction> txn = get_transaction(TID);
	if(txn == nullptr)
	{
		cerr << "wrong transaction ID!" << endl;
		return -1;
	}
	if (txn->GetState() != TransactionState::RUNNING) {
		cerr << "transaction not in running state! Query failed" <<  " " << (int)txn->GetState() <<  endl;;
		return -99;
	}
	int ret_val;
	ResultSet rs;
	FILE* output = stdout;
	if(db != nullptr)
		ret_val = this->db->query(sparql, rs, output , true, false, txn);
	else{
		cout << "error! database has been flushed or removed" << endl;
		return -10;
	}
	if(txn->GetState() == TransactionState::ABORTED)
	{
		cout << "Transaction Abort due to Query failed. TID:" << TID << endl;
		Abort(TID);
		return -20;
	}
	if (ret_val < -1)   //non-update query
	{
		results = rs.to_JSON();
		return ret_val;
	}
	else
	{
		get_transaction(TID)->update_num += ret_val;
		return ret_val;
	}
}

void Txn_manager::Checkpoint()
{
	checkpoint_lock.lockExclusive();
	vector<unsigned> sub_ids , obj_ids, obj_literal_ids, pre_ids;
	sub_ids.insert(sub_ids.begin(), DirtyKeys[0].begin(), DirtyKeys[0].end());
	pre_ids.insert(pre_ids.begin(), DirtyKeys[1].begin(), DirtyKeys[1].end());
	for(auto key: DirtyKeys[2])
	{
		if(Util::is_entity_ele(key)){
			obj_ids.push_back(key);
		}
		else{
			obj_literal_ids.push_back(key);
		}
	}
	//cout << "sub_ids.size(): " << sub_ids.size() << endl;
	//cout << "pre_ids.size(): " << pre_ids.size() << endl;
	//cout << "obj_ids.size(): " << obj_ids.size() << endl;
	//cout << "obj_literal_ids.size(): " << obj_literal_ids.size() << endl;
 	if(db != nullptr)
		db->VersionClean(sub_ids, obj_ids, obj_literal_ids, pre_ids);
	checkpoint_lock.unlock();
}


//TODO:this function is not complete
void Txn_manager::restore()
{
	string line;
	ifstream in;
	in.open(this->log_path.c_str(), ios::in);
	txn_id_t  TID = -1; //NOT COMPLETE
	while (getline(in, line)) 
	{
		if (line[0] == 'B')
		{
			vector<string> redo_set;
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
				redo(redo_set[i], TID);
			}
		}
	}
}

txn_id_t Txn_manager::find_latest_txn()
{
	auto it = txn_table.begin();
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
	auto it = txn_table.begin();
	for (; it != txn_table.end(); it++)
	{
		if (it->second->GetState() == TransactionState::RUNNING)
		{
			Abort(it->first);
		}
	}
}

void Txn_manager::print_txn_dataset(txn_id_t TID)
{
	auto txn = Get_Transaction(TID);
	txn->print_all();
}

void Txn_manager::add_dirty_keys(shared_ptr<Transaction> txn)
{
	//lock_guard<mutex> lck(DirtyKeys_lock);
	DirtyKeys_lock.lock();
	const vector<IDSet> & sets = txn->Get_WriteSet();
	for(int i = 0; i < 3; i++)
	{
		DirtyKeys[i].insert(sets[i].begin(), sets[i].end());
	}
	DirtyKeys_lock.unlock();
}