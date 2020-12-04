#include "Version.h"


Version::Version()
{
	this->begin_ts = INVALID_TS;
	this->end_ts = INVALID_TS;
	this->txn_id = INVALID_ID;
	this->read_cnt= 0;
}

Version::Version(TYPE_TXN_ID _begin_ts, TYPE_TXN_ID _end_ts)
{
	this->begin_ts = _begin_ts;
	this->end_ts = _end_ts;
	this->txn_id = INVALID_ID;
	this->read_cnt= 0;
}

Version& Version::operator=(const Version& V)
{
	this->begin_ts.store(V.begin_ts.load());
	this->end_ts.store(V.end_ts.load());
	this->txn_id.store(INVALID_ID);
	this->read_cnt.store(0);
	this->add_set = V.add_set;
	this->del_set = V.del_set;
	return *this;
}

Version::Version(TYPE_TXN_ID _begin_ts, TYPE_TXN_ID _end_ts, Version* V)
{
	this->begin_ts = _begin_ts;
	this->end_ts = _end_ts;
	this->txn_id.store(INVALID_ID);
	this->read_cnt.store(0);
	this->add_set = V->add_set;
	this->del_set = V->del_set;
}


void Version::add(VData value)
{
	auto it = this->del_set.find(value);
	if(it == this->del_set.end())
		this->add_set.insert(value);
	else
		this->del_set.erase(it);
}

void Version::remove(VData value)
{
	auto it = this->add_set.find(value);
	if(it == this->add_set.end())
		this->del_set.insert(value);
	else
		this->add_set.erase(it);
}

bool Version::is_exclusive_latched() 
{ 
	return txn_id.load() != INVALID_ID ;
}

bool Version::is_owned_exclusive_latch(TYPE_TXN_ID _TID) 
{ 
	return txn_id.load() == _TID; 
}

bool Version::is_shared_latched() 
{ 
	return (read_cnt.load() != 0 && txn_id.load() == INVALID_ID); 
}

bool Version::is_latched() 
{
	return txn_id.load() != INVALID_ID || read_cnt.load() != 0; 
}

void Version::shared_unlatch() 
{ 
	assert(read_cnt.load() != 0); 
	read_cnt--;
}

bool Version::exclusive_unlatch(TYPE_TXN_ID _TID, bool is_SI) 
{ 
	if(is_exclusive_latched() == false) return false;
	if(!txn_id.compare_exchange_strong(_TID, INVALID_ID)){
		cerr << "TID is wrong!" << endl;
		return false;
	}
	if(is_SI && read_cnt.load() == 0) 
	{
		cerr << "read_cnt is zero" << endl;
		return false;  
	}
	read_cnt--; 
	return true;
}

//SI only
bool Version::get_shared_latch(TYPE_TXN_ID _TID)
{
	if(txn_id.load() ==  INVALID_ID) //no exclusive locked
	{
		read_cnt++;
	}
	else if(txn_id.load() != _TID) //exclusive locked
	{
		cerr << "get shared latch failed due to exclusive locked" << endl;
		return false;
	}
	//owned exclusive locked
	return true;
}

bool Version::get_exclusive_latch(TYPE_TXN_ID _TID, bool is_SI)
{
	TYPE_TXN_ID expected_ID = INVALID_ID;
	TYPE_READ_CNT expected_CNT = 0;
	if(!txn_id.compare_exchange_strong(expected_ID, _TID)) return false;
	if(is_SI && !read_cnt.compare_exchange_strong(expected_CNT, 1))
	{
		txn_id.compare_exchange_strong(_TID, expected_ID);// undo
		return false;
	}
	return true;
} 

//SI only
bool Version::upgrade_latch(TYPE_TXN_ID _TID)
{
	if(read_cnt.load() != 1) 
	{
		return false;
	}
	TYPE_TXN_ID expected_ID = INVALID_ID;
	if(txn_id.compare_exchange_strong(expected_ID, _TID)) return true;
	else return false;
}

bool Version::downgrade_latch(TYPE_TXN_ID _TID)
{
	if(read_cnt.load() != 1) 
	{
		cerr << "read cnt is not one !" << endl;
		return false;
	}
	return txn_id.compare_exchange_strong(_TID, INVALID_ID);
}

void Version::get_add_set(VDataSet& _addset) const
{
	_addset = this->add_set;
}
void Version::get_del_set(VDataSet& _delset) const
{
	_delset = this->del_set;
}
	