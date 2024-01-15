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
	this->glatch = _entry.glatch;
	this->is_versioned.store(_entry.is_versioned.load());
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

//Read Committed
void
IVEntry::getLatestVersion(TYPE_TXN_ID TID, VDataSet &addset, VDataSet &delset)
{
	rwLatch.lockShared();
	unsigned n = vList.size();
	for(unsigned i = 0; i < n - 1; i++)
	{
		vList[i]->get_version(addset, delset);
		cout << "begin_ts" << vList[i]->get_begin_ts() << "  end_ts" << vList[i]->get_end_ts() << endl;
	}
	if((vList[n-1]->get_begin_ts() == INVALID_TS && vList[n-1]->get_end_ts() == TID) || (vList[n-1]->get_end_ts() == INVALID_TS)) //private version or committed version
	{
		vList[n-1]->get_version(addset, delset);
		cout << "begin_ts" << vList[n-1]->get_begin_ts() << "  end_ts" << vList[n-1]->get_end_ts() << endl;
	}
	rwLatch.unlock();
}

//check the head version when create new version
bool
IVEntry::checkVersion(TYPE_TXN_ID TID, bool IS_RC)
{
	int n = vList.size();
	auto latest = vList[n-1];
	if(latest->get_begin_ts() == INVALID_TS ){
		if(latest->get_end_ts() == TID) return true; //owned lock
		else return false; // locked
	} 
	if(!IS_RC && TID < latest->get_begin_ts()) return false; //old write
	return true;
}
//check the head version when read version
int 
IVEntry::checkheadVersion(TYPE_TXN_ID TID)
{
	int n = vList.size();
	int ret = 0;
	if((vList[n-1]->get_begin_ts() == INVALID_TS && TID > vList[n-1]->get_end_ts())) ret = -1; //try reading not owned uncommitted version, abort
	if(vList[n-1]->get_begin_ts() <= TID) ret = 1; //try aquiring the lock(maybe failed because new verison is not added in list yet)
	return ret;
}

//SI and SR
void
IVEntry::getProperVersion(TYPE_TXN_ID TID, VDataSet &addset, VDataSet &delset)
{
	int n = vList.size();
	for(int k = 0; k < n; k++){
		if(TID < vList[k]->get_begin_ts()){
			break;
		}
		vList[k]->get_version(addset, delset);
	}
	if(vList[n-1]->get_begin_ts() == INVALID_TS && TID == vList[n-1]->get_end_ts()){
		vList[n-1]->get_version(addset, delset); //read private version
	}
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
IVEntry::ReadVersion(VDataSet &AddSet, VDataSet &DelSet, shared_ptr<Transaction> txn, bool &latched, bool first_read)
{
	if(txn->GetIsolationLevelType() == IsolationLevelType::READ_COMMITTED)
	{
		getLatestVersion(txn->GetTID(), AddSet, DelSet); //get latest committed version or owned uncommitted version
	}
	else if (txn->GetIsolationLevelType() == IsolationLevelType::SNAPSHOT)
	{
		rwLatch.lockShared();
		getProperVersion(txn->GetTID(), AddSet, DelSet); //get version according to timestamp
		rwLatch.unlock();
	}
	else if (txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE)
	{
		rwLatch.lockShared();
		int ret = checkheadVersion(txn->GetTID());
		if(ret == -1){
			rwLatch.unlock();
			assert(latched == false);
			return false;
		}
		else if(ret == 1 && first_read){
			latched = glatch.trysharedlatch(txn->GetTID());
			if(!latched){
				rwLatch.unlock();
				assert(latched == false);
				return false;
			}
		}
		getProperVersion(txn->GetTID(), AddSet, DelSet);
		rwLatch.unlock();
	}
	else //not defined
	{
		assert(false);
	}
	return true;
}


//txn aborts then delete its uncommitted version. Deleted version is always the latest version.
bool 
IVEntry::InvalidExlusiveLatch(shared_ptr<Transaction> txn, bool has_read)
{
	bool IS_SR = txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE;
	auto TID = txn->GetTID();
	rwLatch.lockExclusive();
	if(IS_SR)
	{
		//SR only
		if(has_read)
		{
			if(glatch.trydowngradelatch(TID) == false) {
				assert(false);
				rwLatch.unlock();
				return false;
			}
		}
		else
		{
			if(glatch.unlatch(TID, true, IS_SR) == false) {
				cerr << "exclusive unlatch failed!" << endl;
				rwLatch.unlock();
				return false;
			}
		}
	}
	//assert(vList.back()->get_end_ts() == TID);// recheck
	vList.pop_back();
	rwLatch.unlock();
	return true;
}

bool
IVEntry::UnLatch(shared_ptr<Transaction> txn, LatchType latch_type)
{
	bool IS_SR = (txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE);
	auto TID = txn->GetTID();
	if(IS_SR && latch_type == LatchType::SHARED)
	{
		bool ret = glatch.unlatch(TID, false, IS_SR);
		return ret;
	}
	else if(latch_type == LatchType::EXCLUSIVE)
	{
		auto cid = txn->GetCommitID();
		
		rwLatch.lockExclusive();
		int k = vList.size();
		if(IS_SR){
			if(glatch.unlatch(TID, true, IS_SR) == false){
				//assert(glatch.get_TID() == TID);
				rwLatch.unlock();
				return false;
			}
		}
		//update the version info(unlock)
		//assert(vList[k-1]->get_end_ts() == TID);
		cout << "cid..................." << cid << endl;
		vList[k-1]->set_begin_ts(cid);
		vList[k-1]->set_end_ts(INVALID_TS);
		vList[k-2]->set_end_ts(cid);
		rwLatch.unlock();
		return true;
	}
	else
	{
		cerr << "error latch type !" << endl;
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
IVEntry::CleanAllVersion()
{
	rwLatch.lockExclusive();
	vList.clear();
	shared_ptr<Version> p = make_shared<Version>(0, INVALID_TS);
	vList.push_back(p);//dummy version [0, INF)
	clearVersionFlag();
	rwLatch.unlock();
}

/*
NOTICE:
we don't need any lock in RC and SI here but only head version check
SR need exclusive lock to restrict the read operations!
*/
int 
IVEntry::GetExclusiveLatch(shared_ptr<Transaction> txn, bool has_read)
{
	auto TID = txn->GetTID();
	shared_ptr<Version> new_version = make_shared<Version>(INVALID_TS, TID); //[-1, TID]
	rwLatch.lockExclusive();
	if(txn->GetIsolationLevelType() == IsolationLevelType::READ_COMMITTED)
	{
		//check the timestamp(lock)
		if(!checkVersion(TID, true)){
			rwLatch.unlock();
			return 0;
		}
	}
	else if(txn->GetIsolationLevelType() == IsolationLevelType::SNAPSHOT)
	{
		//check the timestamp(lock)
		if(!checkVersion(TID, false)){
			rwLatch.unlock();
			return 0;
		}
	}
	else if(txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE)
	{
		if(!checkVersion(TID, false)){
			rwLatch.unlock();
			return 0;
		}
		if(has_read){
			if(glatch.tryupgradelatch(TID) == false){
				rwLatch.unlock();
				return 0;
			}
		}
		else{
			//cerr << "get exclusive latch here" << endl;
			if(glatch.tryexclusivelatch(TID, true) == false){
				rwLatch.unlock();
				return 0;
			}
		}
	}
	else
	{
		cerr << "undefined IsolationLevelType!" << endl;
		rwLatch.unlock();
		return 0;
	}
	vList.push_back(new_version);
	//assert(glatch.get_TID() == TID);
	setVersionFlag();
	rwLatch.unlock();
	return 1;
}

int
IVEntry::WriteVersion(VDataSet &AddSet, VDataSet &DelSet, shared_ptr<Transaction> txn)
{
	//private version no need any lock here
	rwLatch.lockShared();
	//assert(glatch.get_TID() == txn->GetTID());
	//assert(vList.size() != 0);
	for(auto it: AddSet)
		vList.back()->add(it);
	for(auto it: DelSet)
		vList.back()->remove(it);
	rwLatch.unlock();
	return 1;
}
