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
	txnId.store(INVALID_ID); //no locked
	shared_ptr<Version> p = make_shared<Version>(0, INVALID_TS);
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
	this->is_versioned.store(_entry.is_versioned);
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

shared_ptr<Version> 
IVEntry::getLatestVersion(shared_ptr<Transaction> txn)
{
	auto head_ver = *vList.begin();
	if(head_ver->is_exclusive_latched() && head_ver->get_txn_id() != txn->GetTID()) // uncommitted but not owned head version
	{
		auto begin_it = vList.begin();
		return *(++begin_it); //next version
	}
	else return head_ver;
}

//read proper version following the version list
shared_ptr<Version>
IVEntry::searchVersion(shared_ptr<Transaction> txn)
{
	auto head_version = *vList.begin();
	//cout << "vList.size():             " << vList.size() << endl;
	for(auto it: vList)
	{
		//txn_ts in [begin_ts, end_ts)
		if(txn->GetTID() >= it->get_begin_ts() && txn->GetTID() < it->get_end_ts())
		{
			//cout << "txn->GetTID()                 " << txn->GetTID() << endl;
			//not owned uncommitted version then skip
			if(it->is_exclusive_latched() && !it->is_owned_exclusive_latch(txn->GetTID()))
			{
				assert(head_version == it);//uncommitted version must be head_version
				continue;
			}
			if(it->is_owned_exclusive_latch(txn->GetTID()))
			{
				//cout << "get owned version: " <<  txn->GetTID() << endl;
			}
			return it;
		}
	}
	return nullptr;
}

bool 
IVEntry::readVersion(VDataSet &AddSet, VDataSet &DelSet, shared_ptr<Transaction> txn, bool first_read)
{
	rwLatch.lockShared();
	shared_ptr<Version> cur_version;
	//cout << "this is readVersion" << endl;
	if(txn->GetIsolationLevelType() == IsolationLevelType::READ_COMMITTED)
	{
		cur_version = getLatestVersion(txn); //get latest committed version or owned uncommitted version
		if(cur_version == nullptr)
		{
			rwLatch.unlock();
			return false;
		}
	}
	else if (txn->GetIsolationLevelType() == IsolationLevelType::SNAPSHOT)
	{
		cur_version = searchVersion(txn); //get version according to timestamp
		if(cur_version == nullptr)
		{
			rwLatch.unlock();
			return false;
		}
	}
	else if (txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE)
	{
		//cout << "this is readVersion !!!!!!!!!!!!" << endl;
		cur_version = searchVersion(txn); //get version according to timestamp
		if(cur_version == nullptr) // no proper version
		{
			cout << "no proper version ! " << endl;
			rwLatch.unlock();
			return false;
		}
		if(cur_version->is_exclusive_latched() && !cur_version->is_owned_exclusive_latch(txn->GetTID()))
		{
			//owned version
		}
		else if(first_read) //first read
		{
			//cout << "get_shared_latch!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
			cur_version->get_shared_latch(txn->GetTID());
		}
	}
	else //not defined
	{
		rwLatch.unlock();
		return false;
	}
	cur_version->get_add_set(AddSet);
	//for(auto it: AddSet)
	//	cout << it.first << "   " << it.second << endl;
	cur_version->get_del_set(DelSet);
	rwLatch.unlock();
	return true;
}

//new to old
//create a new version from the latest version
//write transaction can see the uncommitted version, if not owned then abort
bool 
IVEntry::addVersion(VDataSet &AddSet, VDataSet &DelSet, shared_ptr<Transaction> txn, bool has_read)
{
	//get exclusive lock before!!!
	//cout << "this is addVersion..........................." << endl;
	rwLatch.lockExclusive(); //read thread will see inconsistent version?
	bool is_SI = (txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE);
	shared_ptr<Version> cur_version;
	shared_ptr<Version> new_version;
	shared_ptr<Version> head_version = *vList.begin();
	if (txn->GetIsolationLevelType() == IsolationLevelType::READ_COMMITTED)
	{
		
		if(head_version->is_owned_exclusive_latch(txn->GetTID())) //write in owned version
			new_version = head_version;
		else //create a new version
		{
			new_version = make_shared<Version>();
			*new_version = *head_version;
		}
	}
	else if (txn->GetIsolationLevelType() == IsolationLevelType::SNAPSHOT)
	{
		cur_version = searchVersion(txn);
		if(cur_version != head_version)
		{
			cout << "not head version" << endl;
			rwLatch.unlock();
			return false; //first updater wins, write-write conflict;
		}
		if(head_version->is_owned_exclusive_latch(txn->GetTID())) //write in owned version
			new_version = head_version;
		else if(!head_version->is_exclusive_latched()) //not exclusive latched
		{
			new_version = make_shared<Version>();
			*new_version = *head_version;
		}
		else
		{
			cerr << "exclusive latched by other transaction" << endl;
			rwLatch.unlock();
			return false;
		}
	}
	else if (txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE)
	{
		cur_version = searchVersion(txn);
		if(cur_version != head_version)
		{
			cerr << "first updater wins, write-write conflict!" << endl;
			rwLatch.unlock();
			return false; //first updater wins, write-write conflict;
		}

		if(head_version->is_owned_exclusive_latch(txn->GetTID())){//write in owned version
			new_version = head_version;
		} 
		else if(!head_version->is_exclusive_latched()) //not exclusive locked
		{
			//some updates will get shared latched before
			//can not upgrade latch due to more than one shared latch
			if(has_read) 
			{
				if(!head_version->upgrade_latch(txn->GetTID())){
					cerr << "upgrade lock failed due to other reader" << endl;
					rwLatch.unlock();
					return false;
				}	
			}
			else
			{
				//no read before, try to get exclusive lock
				if(head_version->get_exclusive_latch(txn->GetTID(), true ) == false)
				{
					
					cerr << "shared latched by other transaction" << endl;
					rwLatch.unlock();
					return false;
				}
				cerr << head_version->get_read_cnt() << "     " << head_version->get_txn_id() << endl;
			}
			new_version = make_shared<Version>();
			*new_version = *head_version;
		}
		else
		{
			//exclusive locked, not happen
			cout << "exclusive latched by other transaction" << endl;
			rwLatch.unlock();
			return false;
		}
		
	}
	else //not defined
	{
		cout << "IsolationLevelType not defined" << endl;
		rwLatch.unlock();
		return false;
	}

	if(new_version == head_version)//owned version
	{
		for(auto& ele: AddSet)
			head_version->add(ele);
		for(auto& ele: DelSet)
			head_version->remove(ele);
		setVersionFlag();
		rwLatch.unlock();
		return true;
	}

	

	//set new version info
	new_version->set_begin_ts(txn->GetTID());
	new_version->set_end_ts(INVALID_TS);
	new_version->get_exclusive_latch(txn->GetTID(), is_SI); //exclusive latched
	
	//unlatch the
	if(txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE)
		cur_version->exclusive_unlatch(txn->GetTID(), true);
		
	for(auto& ele: AddSet)
		new_version->add(ele);
	for(auto& ele: DelSet)
		new_version->remove(ele);
	//head point to new version
	vList.push_front(new_version);
	setVersionFlag();
	rwLatch.unlock();
	return true;
}

//txn aborts then delete its uncommitted version. Deleted version is always the latest version.
bool 
IVEntry::invalidExlusiveLatch(shared_ptr<Transaction> txn, bool has_read)
{
	//check the txn id
	rwLatch.lockExclusive();
	auto cur_version = searchVersion(txn);
	auto begin_it = vList.begin();
	auto head = *begin_it;
	if(cur_version != head) {
		cerr << "...........................not head_version !!" << endl;
		rwLatch.unlock();
		return false;
	}
	if(txn->GetTID() != head->get_txn_id()){
		cerr << "...........................not owned version!" << endl;
		rwLatch.unlock();
		return false; // not match
	} 
	
	vList.erase(vList.begin());
	if(has_read){
		begin_it = vList.begin();
		head = *begin_it;
		head->get_shared_latch(txn->GetTID());
	}
	

	if(vList.size() == 0) clearVersionFlag();
	rwLatch.unlock();
	return true;
}

bool
IVEntry::unLatch(shared_ptr<Transaction> txn, LatchType latch_type)
{
	bool is_SI = (txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE);
	if(is_SI && latch_type == LatchType::SHARED)
	{
		rwLatch.lockShared();
		auto cur_version = searchVersion(txn);
		if(cur_version == nullptr)
		{
			rwLatch.unlock();
			return false;
		}
		if(cur_version->get_txn_id() != txn->GetTID()){
			//only shared locked
			cur_version->get_txn_id();
			auto begin_it = vList.begin();
			auto head_ver = *begin_it;
			if(head_ver == cur_version) cerr << "head version shared_unlatch unlatch" << endl;
			txn->GetTID();
			cur_version->shared_unlatch();
		}
		rwLatch.unlock();
		return true;
	}
	else if(latch_type == LatchType::EXCLUSIVE)
	{
		rwLatch.lockExclusive();
		
		//check the head version 
		auto begin_it = vList.begin();
		auto head_ver = *begin_it;
		auto cur_version = searchVersion(txn);
		
		assert(head_ver == cur_version);//recheck 
		
		auto next = ++begin_it;
		if(head_ver->is_exclusive_latched() == false || head_ver->get_txn_id() != txn->GetTID())
		{
			if(head_ver->is_exclusive_latched() == false) cerr << "head version is not exclusive latched " << endl;
			else if(head_ver->get_txn_id() != txn->GetTID()) cerr << "head version is not owned !" <<  endl; 
			rwLatch.unlock();
			return false;
		}
		
		//update the version info
		head_ver->set_begin_ts(txn->GetCommitID());
		(*next)->set_end_ts(txn->GetCommitID());
		//unlatch
		bool ret = head_ver->exclusive_unlatch(txn->GetTID(), is_SI);
		rwLatch.unlock();
		if(ret == false){
			cerr << "head version unLatch failed" << endl;
		}
		return ret;
	}
	else
	{
		cerr << "error latch type !" << endl;
		rwLatch.unlock();
		return false;
	}
}

/*
int
IVEntry::TryEntryLock(shared_ptr<Transaction> txn)
{
	if(txnId.load() == txn->GetTID()) {
		//cout << "get owned Exclusive lock ................................" << endl;
		return -1; //owned, not first locked
	}
	TYPE_TXN_ID expected_ID = INVALID_ID;
	if(txnId.compare_exchange_strong(expected_ID, txn->GetTID()) == true) return 1;
	else return 0;
}

bool
IVEntry::UnEntryLock(shared_ptr<Transaction> txn)
{
	TYPE_TXN_ID expected_ID = txn->GetTID();
	
	return txnId.compare_exchange_strong(expected_ID,INVALID_ID);
}

int
IVEntry::getExclusiveLock(shared_ptr<Transaction> txn)
{
	return TryEntryLock(txn);
}

//not update version info here
bool
IVEntry::releaseExlusiveLock(shared_ptr<Transaction> txn)
{
	return UnEntryLock(txn);
}
*/

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
	vList.clear();
	shared_ptr<Version> p = make_shared<Version>(0, INVALID_TS);
	vList.push_back(p);//dummy version [0, INF)
	clearVersionFlag();
}

int 
IVEntry::getExclusiveLatch(shared_ptr<Transaction> txn, bool has_read)
{
	shared_ptr<Version> cur_version;
	shared_ptr<Version> new_version;
	shared_ptr<Version> head_version = *vList.begin();
	rwLatch.lockExclusive(); //read thread will see inconsistent version?
	if(txn->GetIsolationLevelType() == IsolationLevelType::READ_COMMITTED)
	{
		//check the head version
		if(head_version->is_exclusive_latched() == true)
		{
			if(head_version->is_owned_exclusive_latch(txn->GetTID()) == false){
				cerr << "head version has been exclusive latched now" << endl;
				rwLatch.unlock();
				return 0;
			}	
		}
		else
		{
			new_version = make_shared<Version>();
			*new_version = *head_version;
			new_version->set_begin_ts(txn->GetTID());
			new_version->get_exclusive_latch(txn->GetTID(), false);
			vList.push_front(new_version);
			setVersionFlag();
			rwLatch.unlock();
			return 1;
		}
	}
	else if(txn->GetIsolationLevelType() == IsolationLevelType::SNAPSHOT)
	{
		cur_version = searchVersion(txn);
		if(cur_version != head_version)
		{
			cerr << "updated by the newer transaction!";
			rwLatch.unlock();
			return 0;
		}
		if(head_version->is_exclusive_latched() == true)
		{
			if(head_version->is_owned_exclusive_latch(txn->GetTID()) == false){
				cerr << "head version has been exclusive latched now" << endl;
				rwLatch.unlock();
				return 0;
			}
			//owned do nothing
		}
		else
		{
			new_version = make_shared<Version>();
			*new_version = *head_version;
			new_version->set_begin_ts(txn->GetTID());
			new_version->get_exclusive_latch(txn->GetTID(), false);
			vList.push_front(new_version);
			setVersionFlag();
			rwLatch.unlock();
			return 1;
		}
		rwLatch.unlock();
		return 1;
	}
	else if(txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE)
	{
		cur_version = searchVersion(txn);
		if(cur_version != head_version)
		{
			cerr << "updated by the newer transaction!";
			rwLatch.unlock();
			return 0;
		}
		if(head_version->is_exclusive_latched() == true)
		{
			if(head_version->is_owned_exclusive_latch(txn->GetTID()) == false){
				cerr << "head version has been exclusive latched now" << endl;
				rwLatch.unlock();
				return 0;
			}
			//owned do nothing
		}
		else if(head_version->is_shared_latched() == true) //shared latched
		{
			//only has read(get shared latch) and upgrade success
			if(has_read && head_version->upgrade_latch(txn->GetTID()) == true)
			{
				new_version = make_shared<Version>();
				*new_version = *head_version;
				new_version->set_begin_ts(txn->GetTID());
				new_version->get_exclusive_latch(txn->GetTID(), true);
				head_version->exclusive_unlatch(txn->GetTID(), true);
				vList.push_front(new_version);
				setVersionFlag();
				rwLatch.unlock();
				return 1;
			}
			else
			{
				cerr << "shared locked or upgrade latch failed!" << endl;
				rwLatch.unlock();
				return 0;
			}
		}
		else //not locked
		{
			new_version = make_shared<Version>();
			*new_version = *head_version;
			new_version->set_begin_ts(txn->GetTID());
			new_version->get_exclusive_latch(txn->GetTID(), true);
			vList.push_front(new_version);
			setVersionFlag();
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
	auto cur_version = searchVersion(txn);
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