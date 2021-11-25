#include "Transaction.h"

Transaction::Transaction(string name, TYPE_TS time, txn_id_t _TID, IsolationLevelType _isolation)
{
	this->db_name = name;
	this->start_time = time;
	this->TID = _TID;
	this->state = TransactionState::WAITING;
	this->update_num = 0;
	this->wait_lock_time = 0; // 1ns
	this->retry_times = 1; // up to 32ns
	if(unsigned(_isolation) > 0 && unsigned(_isolation) <= 3) //valid value
		this->isolation = _isolation;
	else this->isolation = IsolationLevelType::SNAPSHOT; //default;
	// if(_isolation == IsolationLevelType::READ_COMMITTED)
	// {
	// 	cerr << "isolation level: READ_COMMITTED" << endl;
	// }
	// else if(_isolation == IsolationLevelType::SNAPSHOT)
	// {
	// 	cerr << "isolation level: SNAPSHOT" << endl;
	// }
	// else if(_isolation == IsolationLevelType::SERIALIZABLE)
	// {
	// 	cerr << "isolation level: SERIALIZABLE" << endl;
	// }
	// else
	// {
	// 	cerr << "Wrong isolation level!     " << unsigned(_isolation) << endl;
	// }
	for(int i = 0; i < 3; i++)
	{
		IDSet s;
		this->ReadSet.push_back(s);
	}
	
	for(int i = 0; i < 3; i++)
	{
		IDSet s;
		this->WriteSet.push_back(s);
	}
}


void Transaction::ReadSetInsert(TYPE_ENTITY_LITERAL_ID _ID, Transaction::IDType _type )
{
	this->ReadSet[(unsigned)_type].insert(_ID);
}

void Transaction::ReadSetDelete(TYPE_ENTITY_LITERAL_ID _ID, Transaction::IDType _type )
{
	this->ReadSet[(unsigned)_type].erase(_ID);
}

bool Transaction::ReadSetFind(TYPE_ENTITY_LITERAL_ID _ID, Transaction::IDType _type )
{
	return this->ReadSet[(unsigned)_type].count(_ID) != 0; //true is read before!
}

void Transaction::WriteSetInsert(IDTriple _Triple)
{
	this->WriteSet[(unsigned)Transaction::IDType::SUBJECT].insert(_Triple.get_subject());
	this->WriteSet[(unsigned)Transaction::IDType::PREDICATE].insert(_Triple.get_predicate());
	this->WriteSet[(unsigned)Transaction::IDType::OBJECT].insert(_Triple.get_object());
}

void Transaction::WriteSetDelete(IDTriple _Triple)
{
	this->WriteSet[(unsigned)Transaction::IDType::SUBJECT].erase(_Triple.get_subject());
	this->WriteSet[(unsigned)Transaction::IDType::PREDICATE].erase(_Triple.get_predicate());
	this->WriteSet[(unsigned)Transaction::IDType::OBJECT].erase(_Triple.get_object());
}

bool Transaction::WriteSetFind(TYPE_ENTITY_LITERAL_ID _ID, Transaction::IDType _type )
{
	return this->WriteSet[(unsigned)_type].count(_ID) != 0; //true is not write before!
}

void Transaction::print_ReadSet()
{
	cout << "IDType: SUBJECT: " << ReadSet[(unsigned)IDType::SUBJECT].size() << endl << endl;
	for(auto& it: ReadSet[(unsigned)IDType::SUBJECT])
		cout << it << " ";
	cout << endl;
	cout << "-----------------------------------------------------------------" << endl << endl;
	cout << "IDType: PREDICATE: " << ReadSet[(unsigned)IDType::PREDICATE].size() << endl << endl;
	for(auto& it: ReadSet[(unsigned)IDType::PREDICATE])
		cout << it << " ";
	cout << endl;
	cout << "-----------------------------------------------------------------" << endl << endl;
	cout << "IDType: OBJECT: " << ReadSet[(unsigned)IDType::OBJECT].size() << endl << endl;
	for(auto& it: ReadSet[(unsigned)IDType::OBJECT])
		cout << it << " ";
	cout << endl;
	cout << "-----------------------------------------------------------------" << endl << endl;
}

void Transaction::print_WriteSet()
{
	cout << "IDType: SUBJECT: " << WriteSet[(unsigned)IDType::SUBJECT].size() << endl << endl;
	for(auto& it: WriteSet[(unsigned)IDType::SUBJECT])
		cout << it << " ";
	cout << endl;
	cout << "-----------------------------------------------------------------" << endl << endl;
	cout << "IDType: PREDICATE: " << WriteSet[(unsigned)IDType::PREDICATE].size() << endl << endl;
	for(auto& it: WriteSet[(unsigned)IDType::PREDICATE])
		cout << it << " ";
	cout << endl;
	cout << "-----------------------------------------------------------------" << endl << endl;
	cout << "IDType: OBJECT: " << WriteSet[(unsigned)IDType::OBJECT].size() << endl << endl;
	for(auto& it: WriteSet[(unsigned)IDType::OBJECT])
		cout << it << " ";
	cout << endl;
	cout << "-----------------------------------------------------------------" << endl << endl;
}


void Transaction::print_all()
{
	cout << "TID:................................................" << this->TID << endl;
	print_ReadSet();
	//print_AddSet();
	print_WriteSet();
}

