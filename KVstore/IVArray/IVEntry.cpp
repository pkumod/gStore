/*=========================================================================
 * File name: IVEntry.cpp
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last Modified: 2018-01-30
 * Description:
 * Implementation of functions in IVEntry.h
 * =======================================================================*/

#include "IVEntry.h"

using namespace std;

IVEntry::IVEntry()
{
	store = 0;
	value = NULL;
	usedFlag = false;
	dirtyFlag = true;
	cacheFlag = false;
	CachePinFlag = false;
	prevID = nextID = -1;
	txnID.store(INVALID_ID); //no locked
	readLCVcnt.store(0);
	shared_ptr<Version> p = make_shared<Version>(0, INVALID_ID);
	vList.push_back(p); //dummy version [0, INF)
	clearVersionFlag();
}

void
IVEntry::setBstr(const Bstr* _value)
{
	if (value != NULL)
		delete value;
	value = new Bstr(*_value);
}

void
IVEntry::setBstr(char *_str, unsigned long _len)
{
	if (value != NULL)
		delete value;
	value = new Bstr();
	//value->copy(_str, _len);
	value->setStr(_str);
	value->setLen(_len);
}

bool
IVEntry::getBstr(char *& _str, unsigned long &_len, bool if_copy) const
{
	if (value == NULL)
	{
		_str = NULL;
		_len = 0;
		return false;
	}

	if (if_copy)
	{
		char *str = value->getStr();
		_len = value->getLen();
		_str = new char [_len];
		memcpy(_str, str, _len);
	}
	else
	{
		_str = value->getStr();
		_len = value->getLen();
	}
	return true;
}

void
IVEntry::setStore(unsigned _store)
{
	store = _store;
}

unsigned
IVEntry::getStore() const
{
	return store;
}

void
IVEntry::setUsedFlag(bool _flag)
{
	usedFlag = _flag;
}

bool
IVEntry::isUsed() const
{
	return usedFlag;
}

void
IVEntry::setDirtyFlag(bool _flag)
{
	dirtyFlag = _flag;
}

bool
IVEntry::isDirty() const
{
	return dirtyFlag;
}

void
IVEntry::setCacheFlag(bool _flag)
{
	cacheFlag = _flag;
}

bool
IVEntry::inCache() const
{
	return cacheFlag;
}

void
IVEntry::setCachePinFlag(bool _flag)
{
	CachePinFlag = _flag;
}

bool
IVEntry::isPined()
{
	return CachePinFlag;
}

void
IVEntry::release()
{
	if (value != NULL)
	{
		delete value;
	}
	value = NULL;
	nextID = prevID = -1;
	//if(is_versioned.load())
	//{
		//cout << "error: versions has not been merged!" << endl;
	//}
}

void
IVEntry::Copy(const IVEntry& _entry)
{
	this->store = _entry.store;
	this->cacheFlag = _entry.cacheFlag;
	this->dirtyFlag = _entry.dirtyFlag;
	this->usedFlag = _entry.usedFlag;
	if (_entry.value != NULL)
	{
		this->value = new Bstr();
		value->copy(_entry.value);
	}
	this->prevID = _entry.prevID;
	this->nextID = _entry.nextID;
	this->vList = move(_entry.vList);
	this->is_versioned.store(_entry.is_versioned.load());
	this->txnID.store(_entry.txnID.load());
	this->readLCVcnt.store(_entry.readLCVcnt.load());
}

void
IVEntry::setPrev(int ID)
{
	prevID = ID;
}

int
IVEntry::getPrev() const
{
	return prevID;
}

void
IVEntry::setNext(int ID)
{
	nextID = ID;
}

int
IVEntry::getNext() const
{
	return nextID;
}

IVEntry::~IVEntry()
{
	this->release();
}

//MVCC

//basic latch function
bool 
IVEntry::get_shared_latch(TYPE_TXN_ID TID)
{
	if(txnID.load() == TID) return true;
	if(txnID.load() != INVALID_ID) return false;
	readLCVcnt++;
	return true;
}

bool 
IVEntry::get_exclusive_latch(TYPE_TXN_ID TID, bool is_SI)
{
	TYPE_TXN_ID expected = INVALID_ID;
	if(txnID.compare_exchange_strong(expected, TID))
	{
		if(is_SI){
			unsigned expected_cnt = 0;
			if(readLCVcnt.compare_exchange_strong(expected_cnt, 1) == false)
			{
				cerr << "read cnt is not equal to zero!" << endl;
				txnID.store(INVALID_ID);
				return false;
			}
		}
		return true;
	}
	else
		return false;
}

bool
IVEntry::upgrade_latch(TYPE_TXN_ID TID)
{
	if(readLCVcnt.load() != 1) {
		cerr << "read cnt is not equal to one!" << endl;
		return false;
	}
	TYPE_TXN_ID expected = INVALID_ID;
	return txnID.compare_exchange_strong(expected, TID);
}

bool
IVEntry::downgrade_latch(TYPE_TXN_ID TID)
{
	if(readLCVcnt.load() != 1) {
		cerr << "read cnt is not equal to one!" << endl;
		return false;
	}
	return txnID.compare_exchange_strong(TID, INVALID_ID);
	
}

bool
IVEntry::shared_unlatch()
{
	if(readLCVcnt.load() == 0) return false;
	readLCVcnt--;
	return true;
}

bool
IVEntry::exclusive_unlatch(TYPE_TXN_ID TID, bool is_SI)
{
	if(txnID.compare_exchange_strong(TID, INVALID_ID)) {
		if(is_SI)
		{
			unsigned expected_cnt = 1;
			if(readLCVcnt.compare_exchange_strong(expected_cnt, 0) == false){
				cerr << "exclusive latch error, read count not equal to one!" << endl;
				return false;
			}
		}
		return true;
	}
	else{
		cerr << "exclusive unlatch error due to wrong TID!" << endl; 
		return false;
	}
}

bool 
IVEntry::is_exclusive_latched()
{
	return txnID.load() != INVALID_ID;
}
bool 
IVEntry::is_shared_latched()
{
	return readLCVcnt.load() != 0;
}

bool 
IVEntry::is_latched()
{
	return is_exclusive_latched() || is_shared_latched();
}

TYPE_READ_CNT 
IVEntry::get_readers()
{
	return readLCVcnt.load();
}

bool 
IVEntry::is_owned_exclusive_latched(TYPE_TXN_ID TID)
{
	return txnID.load() == TID;
}

shared_ptr<Version> 
IVEntry::getLatestVersion(TYPE_TXN_ID TID, VDataArray &addarray, VDataArray &delarray)
{
	auto start_iter = vList.begin();
	//cerr << "vList.size():  " << vList.size() << endl;
	if(is_exclusive_latched() && txnID.load() != TID) // uncommitted but not owned head version
	{
		start_iter++;
	}
	for(auto it = start_iter; it != vList.end(); it++)
	{
		auto addvec = (*it)->get_add_set();
		auto delvec = (*it)->get_del_set();
		addarray.insert(addarray.end(), addvec.begin(), addvec.end());
		delarray.insert(delarray.end(), delvec.begin(), delvec.end());
	}
	return *start_iter;
}

//read proper version following the version list
shared_ptr<Version>
IVEntry::searchVersion(TYPE_TXN_ID TID)
{
	auto start_iter = vList.begin();
	//cerr << "vList.size():  " << vList.size() << endl;
	if(is_exclusive_latched() && txnID.load() != TID) // uncommitted but not owned head version
	{
		start_iter++;
	}

	for(; start_iter != vList.end(); start_iter++)
	{
		if(TID >= (*start_iter)->get_begin_ts() && TID < (*start_iter)->get_end_ts())
		{
			return *start_iter;
		}
	}
	if(start_iter == vList.end()) return nullptr;
}

shared_ptr<Version>
IVEntry::getProperVersion(TYPE_TXN_ID TID, VDataArray &addarray, VDataArray &delarray, bool IS_SI)
{
	auto start_iter = vList.begin();
	if(!IS_SI && is_exclusive_latched() && txnID.load() != TID) // uncommitted but not owned head version
	{
		start_iter++;
	}

	for(; start_iter != vList.end(); start_iter++)
	{
		if(TID >= (*start_iter)->get_begin_ts() && TID < (*start_iter)->get_end_ts())
		{
			break;
		}
	}
	if(start_iter == vList.end()) return nullptr;
	auto ret = *start_iter;
	for(auto it = start_iter; it != vList.end(); it++)
	{
		auto addvec = (*it)->get_add_set();
		auto delvec = (*it)->get_del_set();
		addarray.insert(addarray.end(), addvec.begin(), addvec.end());
		delarray.insert(delarray.end(), delvec.begin(), delvec.end());
	}
	return ret;
}

void 
IVEntry::version_merge(VDataArray &addarray, VDataArray &delarray, VDataSet &AddSet, VDataSet &DelSet)
{
	sort(addarray.begin(), addarray.end());
	sort(delarray.begin(), delarray.end());
	//cerr << "addarray.size():  " << addarray.size() << "delarray.size():  " << delarray.size() << endl;
	VDataArray interarray, _addarray, _delarray;
	set_intersection(addarray.begin(), addarray.end(), delarray.begin(), delarray.end(), inserter(interarray, interarray.end()));
	set_difference(addarray.begin(), addarray.end(), interarray.begin(), interarray.end(), inserter(_addarray, _addarray.end()));
	set_difference(delarray.begin(), delarray.end(), interarray.begin(), interarray.end(), inserter(_delarray, _delarray.end()));
	AddSet.insert(_addarray.begin(), _addarray.end());
	DelSet.insert(_delarray.begin(), _delarray.end());
	//no duplicate check
	assert(AddSet.size() == _addarray.size());
	assert(DelSet.size() == _delarray.size());
}

bool 
IVEntry::readVersion(VDataSet &AddSet, VDataSet &DelSet, shared_ptr<Transaction> txn, bool first_read)
{
	rwLatch.lockShared();
	shared_ptr<Version> cur_version;
	//cout << "this is readVersion" << endl;
	//if(vList.size() == 1) return false;
	VDataArray addarry, delarray;
	if(txn->GetIsolationLevelType() == IsolationLevelType::READ_COMMITTED)
	{
		cur_version = getLatestVersion(txn->GetTID(), addarry, delarray); //get latest committed version or owned uncommitted version
		if(cur_version == *vList.end()) return false;
	}
	else if (txn->GetIsolationLevelType() == IsolationLevelType::SNAPSHOT)
	{
		auto start_version = getProperVersion(txn->GetTID(), addarry, delarray, false); //get version according to timestamp
		if(start_version == nullptr)
		{
			rwLatch.unlock();
			return false;
		}
	}
	else if (txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE)
	{
		//cerr << "this is readVersion !!!!!!!!!!!!" << endl;
		//cerr << "first_read................................" << first_read << endl;
		auto head_version = *vList.begin();
		auto cur_version = getProperVersion(txn->GetTID(), addarry, delarray, true); //get version according to timestamp
		if(cur_version == nullptr) // no proper version
		{
			cerr << "no proper version found!" << endl;
			rwLatch.unlock();
			return false;
		}
		
		if(cur_version == head_version)
		{
			//first read and not owned version
			if(is_exclusive_latched() && !is_owned_exclusive_latched(txn->GetTID()))
			{
				cerr << "read uncommitted version which not owned!(NEVER happen)" << endl;
				rwLatch.unlock();
				return false;
			}
			else if(is_exclusive_latched() && is_owned_exclusive_latched(txn->GetTID()))
			{
				//owned version then do nothing
			}
			else if(first_read)
			{
				//not exclusive latched and first read
				bool ret = get_shared_latch(txn->GetTID());
				if(ret == false)
				{
					cerr << "shared latch failed!(NEVER happen)" << endl;
					rwLatch.unlock();
					return false;
				}
			}
		}
	}
	else //not defined
	{
		rwLatch.unlock();
		return false;
	}
	version_merge(addarry, delarray, AddSet, DelSet);
	rwLatch.unlock();
	return true;
}


//txn aborts then delete its uncommitted version. Deleted version is always the latest version.
bool 
IVEntry::invalidExlusiveLatch(shared_ptr<Transaction> txn, bool has_read)
{
	//check the txn id
	rwLatch.lockExclusive();
	auto cur_version = searchVersion(txn->GetTID());
	auto begin_it = vList.begin();
	auto head_version = *begin_it;
	if(cur_version != head_version) {
		cerr << "...........................not head_version !!" << endl;
		rwLatch.unlock();
		return false;
	}
	if(txn->GetTID() != txnID.load()){
		cerr << "...........................not owned version!" << endl;
		rwLatch.unlock();
		return false; // not match
	} 

	vList.pop_front();
	if(vList.size() == 1) clearVersionFlag(); //dummy version left
	
	bool ret;
	bool is_SI = txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE;
	if(is_SI && has_read)
	{
		//SI only
		ret = downgrade_latch(txn->GetTID());
	}
	else
	{
		
		ret = exclusive_unlatch(txn->GetTID(), is_SI);
		//cerr << "................exclusive_unlatch. TID:" << txn->GetTID() <<  "  ret: " << ret << endl;
	}

	rwLatch.unlock();
	return ret;
}

bool
IVEntry::unLatch(shared_ptr<Transaction> txn, LatchType latch_type)
{
	bool is_SI = (txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE);
	if(is_SI && latch_type == LatchType::SHARED)
	{
		rwLatch.lockShared();
		auto cur_version = searchVersion(txn->GetTID());
		auto head = vList.begin();
		auto head_version = *head;
		if(cur_version == nullptr)
		{
			cerr << "no proper version found !" << endl;
			rwLatch.unlock();
			return false;
		}	
		if(head_version == cur_version){
			if(txnID.load() != INVALID_ID){
				//NOTICE: owned version should call exclusive unlatch
				cerr << "head version is exclusive latched!" << endl;
				return false;
			}
			bool ret = shared_unlatch();
			rwLatch.unlock();
			return ret;
		}
		else
		{
			rwLatch.unlock();
			return true;
		}
		
	}
	else if(latch_type == LatchType::EXCLUSIVE)
	{
		rwLatch.lockExclusive();
		
		//check the head version 
		auto begin_it = vList.begin();
		auto head_ver = *begin_it;
		auto next = ++begin_it;
		auto cur_version = searchVersion(txn->GetTID());

		assert(head_ver == cur_version);//recheck 

		if(is_exclusive_latched() == false)
		{
			cerr << "head version is not exclusive latched " << endl;
			rwLatch.unlock();
			return false;
		}
		if(txnID.load() != txn->GetTID())
		{
			cerr << "head version is not owned !" <<  endl; 
			rwLatch.unlock();
			return false;
		}
		//update the version info
		head_ver->set_begin_ts(txn->GetCommitID());
		(*next)->set_end_ts(txn->GetCommitID());
		//unlatch
		bool ret = exclusive_unlatch(txn->GetTID(), is_SI);
		
		if(ret == false){
			cerr << "head version unLatch failed" << endl;
		}
		if(this->usedFlag == false){
			this->usedFlag = true;
		}
		setVersionFlag();
		rwLatch.unlock();
		return ret;
	}
	else
	{
		cerr << "error latch type !" << endl;
		rwLatch.unlock();
		return false;
	}
}


void 
IVEntry::setVersionFlag()
{
	bool expected = false;
	is_versioned.compare_exchange_strong(expected, true);
}

void 
IVEntry::clearVersionFlag()
{
	bool expected = true;
	is_versioned.compare_exchange_strong(expected, false);
}

void
IVEntry::cleanAllVersion()
{
	rwLatch.lockExclusive();
	vList.clear();
	shared_ptr<Version> p = make_shared<Version>(0, INVALID_TS);
	vList.push_back(p);//dummy version [0, INF)
	clearVersionFlag();
	rwLatch.unlock();
}

int 
IVEntry::getExclusiveLatch(shared_ptr<Transaction> txn, bool has_read)
{
	rwLatch.lockExclusive(); //read thread will see inconsistent version?
	shared_ptr<Version> cur_version;
	shared_ptr<Version> new_version;
	shared_ptr<Version> head_version = *vList.begin();
	if(txn->GetIsolationLevelType() == IsolationLevelType::READ_COMMITTED)
	{
		//check the head version
		if(is_exclusive_latched() == true)
		{
			if(is_owned_exclusive_latched(txn->GetTID()) == false){
				// cerr << "head version has been exclusive latched now" << txnID.load() << endl;
				rwLatch.unlock();
				return 0;
			}
			else
			{
				rwLatch.unlock();
				return 1;
			}
		}
		else
		{
			new_version = make_shared<Version>();
			new_version->set_begin_ts(txn->GetTID());
			new_version->set_end_ts(INVALID_TS);
			get_exclusive_latch(txn->GetTID(), false);
			vList.push_front(new_version);
			rwLatch.unlock();
			return 1;
		}
	}
	else if(txn->GetIsolationLevelType() == IsolationLevelType::SNAPSHOT)
	{
		
		if(is_exclusive_latched() == true)
		{
			if(is_owned_exclusive_latched(txn->GetTID()) == false){
				// cerr << "head version has been exclusive latched now" << endl;
				rwLatch.unlock();
				return 0;
			}
			//owned do nothing
			else
			{
				rwLatch.unlock();
				return 1;
			}
		}
		else
		{
			cur_version = searchVersion(txn->GetTID());
			if(cur_version != head_version)
			{
				//cerr << "updated by the newer transaction! Not locked!" << endl;
				rwLatch.unlock();
				return 0;
			}
			get_exclusive_latch(txn->GetTID(), false);
			new_version = make_shared<Version>();
			new_version->set_end_ts(INVALID_TS);
			new_version->set_begin_ts(txn->GetTID());
			vList.push_front(new_version);
			rwLatch.unlock();
			return 1;
		}
	}
	else if(txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE)
	{
		//cerr << "this is IsolationLevelType::SERIALIZABLE getExclusivelocks" << endl;
		
		if(is_exclusive_latched() == true)
		{
			if(is_owned_exclusive_latched(txn->GetTID()) == false){
				// cerr << "head version has been exclusive latched now" << endl;
				rwLatch.unlock();
				return 0;
			}
			//owned do nothing
			else
			{
				rwLatch.unlock();
				return 1;
			}
		}
		else if(is_shared_latched() == true) //shared latched
		{
			//only has read(get shared latch) and upgrade success
			cur_version = searchVersion(txn->GetTID());
			if(cur_version != head_version)
			{
				cerr << "updated by the newer transaction! shared locked!" << endl;
				rwLatch.unlock();
				return 0;
			}
			if(has_read && upgrade_latch(txn->GetTID()) == true)
			{
				new_version = make_shared<Version>();
				new_version->set_end_ts(INVALID_TS);
				new_version->set_begin_ts(txn->GetTID());
				vList.push_front(new_version);
				rwLatch.unlock();
				return 1;
			}
			else
			{
				cerr << "shared locked or upgrade latch failed!   " << has_read <<  endl;
				rwLatch.unlock();
				return 0;
			}
		}
		else //not locked
		{
			cur_version = searchVersion(txn->GetTID());
			if(cur_version != head_version)
			{
				//cerr << "updated by the newer transaction! Not locked!" << endl;;
				rwLatch.unlock();
				return 0;
			}
			get_exclusive_latch(txn->GetTID(), true);
			new_version = make_shared<Version>();
			new_version->set_end_ts(INVALID_TS);
			new_version->set_begin_ts(txn->GetTID());	
			vList.push_front(new_version);
			rwLatch.unlock();
			return 1;
		}
	}
	else
	{
		cerr << "undefined IsolationLevelType!" << endl;
		rwLatch.unlock();
		return 0;
	}
}

int
IVEntry::writeVersion(VDataSet &AddSet, VDataSet &DelSet, shared_ptr<Transaction> txn)
{
	//other transaction can not access, use shared lock 
	rwLatch.lockShared();
	auto cur_version = searchVersion(txn->GetTID());
	shared_ptr<Version> head_version = *vList.begin();

	//recheck
	assert(cur_version == head_version);
	assert(cur_version->get_begin_ts() == txn->GetTID()); //owned

	for(auto it: AddSet)
		cur_version->add(it);
	for(auto it: DelSet)
		cur_version->remove(it);
	rwLatch.unlock();
	return 1;
}

//abort only
bool 
IVEntry::releaseExlusiveLatch(shared_ptr<Transaction> txn, bool has_read)
{
	return this->invalidExlusiveLatch(txn, has_read);
}