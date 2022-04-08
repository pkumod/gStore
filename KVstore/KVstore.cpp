/*=============================================================================
# Filename: KVstore.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2018-02-09 14:25
# Description: Modified By Wang Libo
=============================================================================*/

#include "KVstore.h"

using namespace std;

//sets store_path as the root dir of this KVstore
//initial all Tree pointers as NULL
KVstore::KVstore(string _store_path) 
{
	this->store_path = _store_path;

	this->dictionary_store_path = _store_path + "/StringPrefix.dc";
	this->trie = NULL;

	this->entity2id = NULL;
	this->id2entity = NULL;

	this->predicate2id = NULL;
	this->id2predicate = NULL;

	this->literal2id = NULL;
	this->id2literal = NULL;

	this->subID2values = NULL;
	this->preID2values = NULL;
	this->objID2values = NULL;
	this->objID2values_literal = NULL;
}

//Release all the memory used in this KVstore before destruction
KVstore::~KVstore() 
{
	this->flush();
	this->release();
}

//Flush all modified parts into the disk, which will not release any memory
//Does nothing to null tree pointers or parts that has not been modified 
void 
KVstore::flush() 
{
	this->flush(this->entity2id);
	this->flush(this->id2entity);

	this->flush(this->literal2id);
	this->flush(this->id2literal);

	this->flush(this->predicate2id);
	this->flush(this->id2predicate);

	this->flush(this->subID2values);
	this->flush(this->preID2values);
	this->flush(this->objID2values);
	this->flush(this->objID2values_literal);
}

void 
KVstore::release() 
{
	//cout << "delete entity2id" << endl;
	delete this->entity2id;
	this->entity2id = NULL;
	//cout << "delete id2eneity" << endl;
	delete this->id2entity;
	this->id2entity = NULL;
	//cout << "delete literal2id" << endl;
	delete this->literal2id;
	this->literal2id = NULL;
	//cout << "delete id2leteral" << endl;
	delete this->id2literal;
	this->id2literal = NULL;

	//cout << "delete p2id" << endl;
	delete this->predicate2id;
	this->predicate2id = NULL;
	//cout << "delte id2p" << endl;
	delete this->id2predicate;
	this->id2predicate = NULL;

	//cout << "delete s2v" << endl;
	delete this->subID2values;
	this->subID2values = NULL;
	//cout << "delete p2v" << endl;
	delete this->preID2values;
	this->preID2values = NULL;
	//cout << "delete o2v" << endl;
	delete this->objID2values;
	this->objID2values = NULL;

	delete this->objID2values_literal;
	this->objID2values_literal = NULL;

	if (trie != NULL)
	{
		trie->SetStorePath(dictionary_store_path);
		delete this->trie;
	}
	this->trie = NULL;
}

void 
KVstore::open() 
{
	cout << "open KVstore" << endl;

	this->open_entity2id(KVstore::READ_WRITE_MODE);
	this->open_id2entity(KVstore::READ_WRITE_MODE);

	this->open_literal2id(KVstore::READ_WRITE_MODE);
	this->open_id2literal(KVstore::READ_WRITE_MODE);

	this->open_predicate2id(KVstore::READ_WRITE_MODE);
	this->open_id2predicate(KVstore::READ_WRITE_MODE);

	this->open_subID2values(KVstore::READ_WRITE_MODE);
	this->open_objID2values(KVstore::READ_WRITE_MODE);
	this->open_preID2values(KVstore::READ_WRITE_MODE);

	if(trie==NULL)
		this->trie = new Trie;

	trie->SetStorePath(dictionary_store_path);
	trie->loadStringPrefix();
}

bool
KVstore::load_trie(int _mode)
{
	if (_mode == KVstore::CREATE_MODE)
	{
		trie = new Trie;
	}
	else if(_mode == KVstore::READ_WRITE_MODE)
	{
		trie = new Trie;
		trie->SetStorePath(dictionary_store_path);
		trie->loadStringPrefix();
	}

	if (trie != NULL)
		return true;
	else
		return false;
}

Trie*
KVstore::getTrie()
{
	return this->trie;
}

void
KVstore::set_if_single_thread(bool _single)
{
  this->entity2id->SetSingleThread(_single);
  this->predicate2id->SetSingleThread(_single);
  this->literal2id->SetSingleThread(_single);
}

string 
KVstore::getStringByID(TYPE_ENTITY_LITERAL_ID _id)
{
	if(Util::is_entity_ele(_id))
	{
		return this->getEntityByID(_id);
	}
	else
	{
		return this->getLiteralByID(_id);
	}
}

TYPE_ENTITY_LITERAL_ID
KVstore::getIDByString(string _str)
{
	// cout << "KVSTORE::GETIDBYSTRING" << endl;
	//load kv_trie
/*	if (kv_trie == NULL)
	{
		string dictionary_path = this->store_path.substr(0, store_path.length() - 9);
		dictionary_path = dictionary_path + "/dictionary.dc";

		this->kv_trie = new Trie;
		if (!kv_trie->LoadTrie(dictionary_path))
		{
			exit(0);
		}
	}*/

	if(Util::isEntity(_str))
	{
	//	string tmp_str = kv_trie->Compress(_str);
	//	cout << "GetIDBYString: " << tmp_str << endl;
		return this->getIDByEntity(_str);
	}
	else
	{
		return this->getIDByLiteral(_str);
	}
}

unsigned
KVstore::getEntityDegree(TYPE_ENTITY_LITERAL_ID _entity_id) const 
{
	return this->getEntityInDegree(_entity_id) + this->getEntityOutDegree(_entity_id);
}

unsigned
KVstore::getEntityInDegree(TYPE_ENTITY_LITERAL_ID _entity_id) const 
{
	//cout << "In getEntityInDegree " << _entity_id << endl;
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->objID2values, _entity_id, (char*&)_tmp, _len);

	int ret = 0;
	if (_get) 
	{
		ret = _tmp[0];
	}

	//if this is a long list, then we should remove itself after copying
	//otherwise, we should not free the list memory
	delete[] _tmp;
		//_tmp = NULL;

	return ret;
}

unsigned
KVstore::getEntityOutDegree(TYPE_ENTITY_LITERAL_ID _entity_id) const 
{
	//cout << "In getEntityOutDegree " << _entity_id << endl;
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->subID2values, _entity_id, (char*&)_tmp, _len);

	int ret = 0;
	if (_get) 
	{
		ret = _tmp[0];
	}

	//if this is a long list, then we should remove itself after copying
	//otherwise, we should not free the list memory
	delete[] _tmp;
		//_tmp = NULL;

	return ret;
}

unsigned
KVstore::getLiteralDegree(TYPE_ENTITY_LITERAL_ID _literal_id) const 
{
	//cout << "In getLiteralDegree " << _literal_id << endl;
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->objID2values, _literal_id, (char*&)_tmp, _len);

	int ret = 0;
	if (_get) 
	{
		ret = _tmp[0];
	}

	//if this is a long list, then we should remove itself after copying
	//otherwise, we should not free the list memory
	delete[] _tmp;
	return ret;
}

unsigned
KVstore::getPredicateDegree(TYPE_PREDICATE_ID _predicate_id) const 
{
	//cout << "In getPredicate Degree " << _predicate_id << endl;
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->preID2values, _predicate_id, (char*&)_tmp, _len);

	int ret = 0;
	if (_get) 
	{
		ret = _tmp[0];
	}

	//if this is a long list, then we should remove itself after copying
	//otherwise, we should not free the list memory
	delete[] _tmp;
		//_tmp = NULL;

	return ret;
}

unsigned 
KVstore::getSubjectPredicateDegree(TYPE_ENTITY_LITERAL_ID _subid, TYPE_PREDICATE_ID _preid) const 
{
	//cout << "In getSubjectPredicateDegree " << _subid << ' ' << _preid << endl;
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);

	int ret = 0;
	if(_get)
	{
		unsigned _result = KVstore::binarySearch(_preid, _tmp + 3, _tmp[1], 2);
		//if (_result != -1) 
		if (_result != INVALID) 
		{
			int _offset = _tmp[4 + 2 * _result];
			int _offset_next;
			if (_result == _tmp[1] - 1) 
			{
				_offset_next = 3 + 2 * _tmp[1] + _tmp[0];
			}
			else 
			{
				_offset_next = _tmp[6 + 2 * _result];
			}
			ret = _offset_next - _offset;
		}
	}

	//if this is a long list, then we should remove itself after copying
	//otherwise, we should not free the list memory
	delete[] _tmp;
		//_tmp = NULL;
	return ret;
}

unsigned 
KVstore::getObjectPredicateDegree(TYPE_ENTITY_LITERAL_ID _objid, TYPE_PREDICATE_ID _preid) const 
{
	//cout << "In getObjectPredicateDegree " << _objid << _preid << endl;
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len);

	int ret = 0;
	if (_get) 
	{
		unsigned _result = KVstore::binarySearch(_preid, _tmp + 2, _tmp[1], 2);
		//if (_result != -1) 
		if (_result != INVALID) 
		{
			int _offset = _tmp[3 + 2 * _result];
			int _offset_next;
			if (_result == _tmp[1] - 1) 
			{
				_offset_next = 2 + 2 * _tmp[1] + _tmp[0];
			}
			else 
			{
				_offset_next = _tmp[5 + 2 * _result];
			}
			ret = _offset_next - _offset;
		}
	}

	//if this is a long list, then we should remove itself after copying
	//otherwise, we should not free the list memory
	delete[] _tmp;
		//_tmp = NULL;
	return ret;
}

bool 
KVstore::updateTupleslist_insert(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id, shared_ptr<Transaction> txn) 
{
	if(txn == nullptr)
	{
	return this->updateInsert_s2values(_sub_id, _pre_id, _obj_id)
		&& this->updateInsert_o2values(_sub_id, _pre_id, _obj_id)
		&& this->updateInsert_p2values(_sub_id, _pre_id, _obj_id);
	}
	else
	{
		// cout << "transaction update" << endl;
		return this->updateInsert_s2values(_sub_id, _pre_id, _obj_id, txn)
		&& this->updateInsert_o2values(_sub_id, _pre_id, _obj_id, txn)
		&& this->updateInsert_p2values(_sub_id, _pre_id, _obj_id, txn);
	}
}

bool 
KVstore::updateTupleslist_remove(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id, shared_ptr<Transaction> txn) 
{
	//int* _tmp = NULL;
	//int _len = 0;
	//bool _get;
	//cout << "Removing:\t" << _sub_id << '\t' << _pre_id << '\t' << _obj_id << endl;
	//cout << "Before removal:" << endl;
	//_get = this->getValueByKey(this->subID2values, _sub_id, (char*&)_tmp, _len);
	//if (!_get) {
	//	cout << "SubID " << _sub_id << ": doesn't exist." << endl;
	//}
	//else {
	//	cout << "SubID " << _sub_id << ':';
	//	for (unsigned i = 0; i < _len / sizeof(int); i++) {
	//		cout << '\t' << _tmp[i];
	//	}
	//	cout << endl;
	//}
	//_get = this->getValueByKey(this->objID2values, _obj_id, (char*&)_tmp, _len);
	//if (!_get) {
	//	cout << "ObjID " << _obj_id << ": doesn't exist." << endl;
	//}
	//else {
	//	cout << "ObjID " << _obj_id << ':';
	//	for (unsigned i = 0; i < _len / sizeof(int); i++) {
	//		cout << '\t' << _tmp[i];
	//	}
	//	cout << endl;
	//}
	//_get = this->getValueByKey(this->preID2values, _pre_id, (char*&)_tmp, _len);
	//if (!_get) {
	//	cout << "PreID " << _pre_id << ": doesn't exist." << endl;
	//}
	//else {
	//	cout << "PreID " << _pre_id << ':';
	//	for (unsigned i = 0; i < _len / sizeof(int); i++) {
	//		cout << '\t' << _tmp[i];
	//	}
	//	cout << endl;
	//}
	//bool flag = this->updateRemove_s2values(_sub_id, _pre_id, _obj_id)
	//	&& this->updateRemove_o2values(_sub_id, _pre_id, _obj_id)
	//	&& this->updateRemove_p2values(_sub_id, _pre_id, _obj_id);
	//cout << "After removal:" << endl;
	//_get = this->getValueByKey(this->subID2values, _sub_id, (char*&)_tmp, _len);
	//if (!_get) {
	//	cout << "SubID " << _sub_id << ": doesn't exist." << endl;
	//}
	//else {
	//	cout << "SubID " << _sub_id << ':';
	//	for (unsigned i = 0; i < _len / sizeof(int); i++) {
	//		cout << '\t' << _tmp[i];
	//	}
	//	cout << endl;
	//}
	//_get = this->getValueByKey(this->objID2values, _obj_id, (char*&)_tmp, _len);
	//if (!_get) {
	//	cout << "ObjID " << _obj_id << ": doesn't exist." << endl;
	//}
	//else {
	//	cout << "ObjID " << _obj_id << ':';
	//	for (unsigned i = 0; i < _len / sizeof(int); i++) {
	//		cout << '\t' << _tmp[i];
	//	}
	//	cout << endl;
	//}
	//_get = this->getValueByKey(this->preID2values, _pre_id, (char*&)_tmp, _len);
	//if (!_get) {
	//	cout << "PreID " << _pre_id << ": doesn't exist." << endl;
	//}
	//else {
	//	cout << "PreID " << _pre_id << ':';
	//	for (unsigned i = 0; i < _len / sizeof(int); i++) {
	//		cout << '\t' << _tmp[i];
	//	}
	//	cout << endl;
	//}
	//return flag;
	if(txn == nullptr){
	return this->updateRemove_s2values(_sub_id, _pre_id, _obj_id)
		&& this->updateRemove_o2values(_sub_id, _pre_id, _obj_id)
		&& this->updateRemove_p2values(_sub_id, _pre_id, _obj_id);
	}
	else
	{
		return this->updateRemove_s2values(_sub_id, _pre_id, _obj_id, txn)
		&& this->updateRemove_o2values(_sub_id, _pre_id, _obj_id, txn)
		&& this->updateRemove_p2values(_sub_id, _pre_id, _obj_id, txn);
	}
}

bool 
KVstore::updateInsert_s2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id) 
{
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->subID2values, _sub_id, (char*&)_tmp, _len);
	bool _is_entity = Util::is_entity_ele(_obj_id);

	//subID doesn't exist
	if (!_get) 
	{
		//unsigned _values[6];
		unsigned* _values = new unsigned[6];
		_values[0] = 1;
		_values[1] = 1;
		_values[2] = _is_entity ? 1 : 0;
		_values[3] = _pre_id;
		_values[4] = 5;
		_values[5] = _obj_id;
		//NOTICE: not use array in stack here, otherwise it will be freed, and data in B+Tree, too
		this->addValueByKey(this->subID2values, _sub_id, (char*)_values, sizeof(int) * 6);
	}

	//subID exists
	else 
	{
		unsigned* _values;
		unsigned long _values_len;
		unsigned _position = KVstore::binarySearch(_pre_id, _tmp + 3, _tmp[1], 2);

		//preID doesn't exist
		if (_position == INVALID) 
		//if (_position == -1) 
		{
			_values_len = _len / sizeof(unsigned) + 3;
			_values = new unsigned[_values_len];
			_values[0] = _tmp[0] + 1;
			_values[1] = _tmp[1] + 1;
			_values[2] = _tmp[2] + (_is_entity ? 1 : 0);
			unsigned i, j;
			for (i = 0, j = 3; i < _tmp[1] && _tmp[3 + 2 * i] < _pre_id; i++, j += 2) {
				_values[j] = _tmp[3 + 2 * i];
				_values[j + 1] = _tmp[4 + 2 * i] + 2;
			}
			_values[j] = _pre_id;
			unsigned _offset_old;
			if (i == _tmp[1]) {
				_offset_old = 3 + 2 * _tmp[1] + _tmp[0];
			}
			else {
				_offset_old = _tmp[4 + 2 * i];
			}
			_values[j + 1] = _offset_old + 2;
			j += 2;
			for (; i < _tmp[1]; i++, j += 2) {
				_values[j] = _tmp[3 + 2 * i];
				_values[j + 1] = _tmp[4 + 2 * i] + 3;
			}
			for (i = 3 + 2 * _tmp[1]; i < 3 + 2 * _tmp[1] + _tmp[0]; i++, j++) {
				if (i == _offset_old) {
					_values[j] = _obj_id;
					j++;
				}
				_values[j] = _tmp[i];
			}
			if (i == _offset_old) {
				_values[j] = _obj_id;
			}
		}

		//preID exists
		else 
		{
			_values_len = _len / sizeof(unsigned) + 1;
			_values = new unsigned[_values_len];
			memcpy(_values, _tmp, sizeof(unsigned) * _tmp[4 + 2 * _position]);
			_values[0]++;
			if (_is_entity) {
				_values[2]++;
			}
			for (unsigned i = _position + 1; i < _tmp[1]; i++) {
				_values[4 + 2 * i]++;
			}
			unsigned i, j;
			unsigned right;
			if (_position == _tmp[1] - 1) {
				right = 3 + 2 * _tmp[1] + _tmp[0];
			}
			else {
				right = _tmp[6 + 2 * _position];
			}
			for (i = _tmp[4 + 2 * _position], j = _tmp[4 + 2 * _position]; i < right && _tmp[i] < _obj_id; i++, j++) {
				_values[j] = _tmp[i];
			}
			_values[j] = _obj_id;
			j++;
			for (; i < 3 + 2 * _tmp[1] + _tmp[0]; i++, j++) {
				_values[j] = _tmp[i];
			}
		}
		this->setValueByKey(this->subID2values, _sub_id, (char*)_values, sizeof(unsigned) * _values_len);
		//delete[] _values;
	}
	delete []_tmp;
	return true;
}

bool 
KVstore::updateRemove_s2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id) 
{
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->subID2values, _sub_id, (char*&)_tmp, _len);
	bool _is_entity = Util::is_entity_ele(_obj_id);

	if (!_get) 
	{
		return false;
	}

	//subID doesn't exist after removal
	if (_tmp[0] == 1) 
	{
		this->removeKey(this->subID2values, _sub_id);
	}
	//subID still exists after removal
	else 
	{
		unsigned* _values;
		unsigned long _values_len;
		unsigned _position = KVstore::binarySearch(_pre_id, _tmp + 3, _tmp[1], 2);
		unsigned _oidlen_sp;
		//DEBUG: if _tmp[1] -1 < 0??
		if (_position == _tmp[1] - 1) 
		{
			_oidlen_sp = 3 + 2 * _tmp[1] + _tmp[0] - _tmp[4 + 2 * _position];
		}
		else 
		{
			_oidlen_sp = _tmp[6 + 2 * _position] - _tmp[4 + 2 * _position];
		}

		//preID doesn't exist after removal
		if (_oidlen_sp == 1) {
			_values_len = _len / sizeof(unsigned) - 3;
			_values = new unsigned[_values_len];
			memcpy(_values, _tmp, sizeof(unsigned) * (3 + 2 * _position));
			_values[0]--;
			_values[1]--;
			if (_is_entity) {
				_values[2]--;
			}
			for (unsigned i = 0; i < _position; i++) {
				_values[4 + 2 * i] -= 2;
			}
			unsigned i, j;
			for (i = 5 + 2 * _position, j = 3 + 2 * _position; i < 3 + 2 * _tmp[1]; i += 2, j += 2) {
				_values[j] = _tmp[i];
				_values[j + 1] = _tmp[i + 1] - 3;
			}
			for (; i < 3 + 2 * _tmp[1] + _tmp[0]; i++, j++) {
				if (i == _tmp[4 + 2 * _position]) {
					j--;
					continue;
				}
				_values[j] = _tmp[i];
			}
		}

		//preID still exists after removal
		else {
			_values_len = _len / sizeof(unsigned) - 1;
			_values = new unsigned[_values_len];
			memcpy(_values, _tmp, sizeof(unsigned) * _tmp[4 + 2 * _position]);
			_values[0]--;
			if (_is_entity) {
				_values[2]--;
			}
			for (unsigned i = _position + 1; i < _tmp[1]; i++) {
				_values[4 + 2 * i]--;
			}
			unsigned i, j;
			for (i = _tmp[4 + 2 * _position], j = _tmp[4 + 2 * _position];
				i < 3 + 2 * _tmp[1] + _tmp[0] && _tmp[i] < _obj_id; i++, j++) {
				_values[j] = _tmp[i];
			}
			i++;
			for (; i < 3 + 2 * _tmp[1] + _tmp[0]; i++, j++) {
				_values[j] = _tmp[i];
			}
		}

		this->setValueByKey(this->subID2values, _sub_id, (char*)_values, sizeof(unsigned) * _values_len);
		//delete[] _values;
	}
	delete []_tmp;
	return true;
}

//TODO: TO BE IMPROVED
unsigned 
KVstore::updateInsert_s2values(TYPE_ENTITY_LITERAL_ID _subid, const std::vector<unsigned>& _pidoidlist) 
{
	if(_pidoidlist.size() == 0) return 0;
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);
	_len = _len/sizeof(unsigned);
	//cout << "_len：          " << _len << endl;
	unsigned* values = NULL;
	unsigned long values_len = 0;
	unsigned update_num = this->Insert_s2values(_pidoidlist, _tmp, _len, values, values_len);
	if(update_num == 0) return 0;
	if(_len == 0){
		this->addValueByKey(this->subID2values, _subid, (char*)values, sizeof(unsigned) * values_len);
		return update_num;
	}
	else
	{
		this->setValueByKey(this->subID2values, _subid, (char*)values, sizeof(unsigned) * values_len);
		return update_num;
	}
}

//TODO: TO BE IMPROVED
unsigned 
KVstore::updateRemove_s2values(TYPE_ENTITY_LITERAL_ID _subid, const std::vector<unsigned>& _pidoidlist) 
{
	if(_pidoidlist.size() == 0) return 0;
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);
	_len = _len/sizeof(unsigned);
	if(_len == 0) return 0;
	unsigned* values = NULL;
	unsigned long values_len = 0;
	unsigned update_num = this->Remove_s2values(_pidoidlist, _tmp, _len, values, values_len);
	if(update_num == 0) return 0;
	if(values_len != 0)
		this->setValueByKey(this->subID2values, _subid, (char*)values, sizeof(unsigned) * values_len);
	else{
		this->removeKey(this->subID2values, _subid);
	}

	return update_num;
}

unsigned 
KVstore::Insert_s2values(const vector<unsigned> &_pidoidlist, unsigned* _tmp,  unsigned long _len, unsigned*& values, unsigned long& values_len) const
{
	if(_pidoidlist.size() == 0)
	{
		values = _tmp;
		values_len = _len;
		return 0;
	}
	map<unsigned, vector<unsigned>> mp;

	unsigned update_num = 0;
    int n = 0;
    int entity_num = 0;
	for(int i = 0; i < _pidoidlist.size(); i += 2)
    {
        mp[_pidoidlist[i]].push_back(_pidoidlist[i+1]);
		pair<int, bool> res = {0, true};
        if(res.second == true) {
            n++;
            if(Util::is_entity_ele(_pidoidlist[i+1])) entity_num++;
        }
    }

	if (!_len) 
	{
		unsigned* _values = new unsigned[3 + 3*n];
		//cout << 2 + 3*n << endl;
		_values[0] = n;
		_values[1] = mp.size();
		_values[2] = entity_num;
		//cout << "mp.size()" << _values[1] << endl;
		int j = 3, len;
		for(auto &it: mp)
		{
			_values[j] = it.first;
			j += 2;
		}

		len = j;
		j = 4;
		for(auto &it: mp)
		{
			_values[j] = len;
			for(auto &sid: it.second)
			{
				_values[len] = sid;
				len++;
			}
			j += 2; //_values[j] = offset
		}

		//assert(_values[0] ==  _values[_values[1]*2+2] - _values[4] + 1);
		assert(len == _values[0] + 2*_values[1] + 3);
		values = new unsigned[len];
		memcpy(values, _values, sizeof(unsigned) * len);
		values_len = len;
		delete [] _values;
		update_num = values[0];
	}
	else
	{
		unsigned* _values;
		unsigned long _values_len;
		_values_len = _len + 3*n;
		_values = new unsigned[_values_len];
		memset(_values, 0, sizeof(unsigned)* _values_len);
		auto it = mp.begin();
		int old_p_offset = 3, p_offset = 3;
		int old_p_len = _tmp[1] * 2 + 3;
		//merge pid
		while(it != mp.end() && old_p_offset < old_p_len)
		{
			if(it->first > _tmp[old_p_offset])
			{
				_values[p_offset] = _tmp[old_p_offset];
				old_p_offset += 2;
			}
			else if(it-> first < _tmp[old_p_offset])
			{
				_values[p_offset] = it->first;
				it++;
			}
			else
			{
				_values[p_offset] = it->first;
				it++;
				old_p_offset += 2;
			}
			p_offset += 2;
		}

		while(it != mp.end()){
			_values[p_offset] = it->first;
			it++; p_offset += 2;
		}
		while(old_p_offset < old_p_len){
			_values[p_offset] = _tmp[old_p_offset];
			old_p_offset += 2; p_offset += 2;
		}

		_values[1] = (p_offset-1) / 2 - 1;
		
		//merge sid
		it = mp.begin();
		int o_offset = p_offset;
		int p_len = p_offset;
		int entity_num = 0;
		p_offset = old_p_offset = 3;
		_values[4] = o_offset;

		while(it != mp.end() && old_p_offset < old_p_len)
		{
			//merge sids
			int next_o_offset;
			if(old_p_offset + 3 > old_p_len){
				next_o_offset = _len; //to the last pid
			}
			else{
				next_o_offset = _tmp[old_p_offset+3];
			}
			if(_values[p_offset] == it->first && _values[p_offset] == _tmp[old_p_offset])
			{
				auto oids = it->second;
				auto o_it = oids.begin();
				int old_o_offset = _tmp[old_p_offset+1];
				while(o_it != oids.end() && old_o_offset < next_o_offset)
				{
					if(*o_it > _tmp[old_o_offset])
					{
						_values[o_offset] = _tmp[old_o_offset];
						old_o_offset++;
						o_offset++;
					}
					else if(*o_it < _tmp[old_o_offset])
					{
						_values[o_offset] = *o_it;
						o_it++;
						o_offset++;
					}
					else
					{
						_values[o_offset] = _tmp[old_o_offset];
						old_o_offset++;
						o_it++;
						o_offset++;
					}
					if(Util::is_entity_ele(_values[o_offset-1])) entity_num++;
				}
				while(o_it != oids.end())
				{
					_values[o_offset] = *o_it;
					o_it++;
					o_offset++;
					if(Util::is_entity_ele(_values[o_offset-1])) entity_num++;
				}
				while(old_o_offset < next_o_offset)
				{
					_values[o_offset] = _tmp[old_o_offset];
					old_o_offset++;
					o_offset++;
					if(Util::is_entity_ele(_values[o_offset-1])) entity_num++;
				}
				it++;
				old_p_offset += 2;
			}
			else if(_values[p_offset] == it->first && _values[p_offset] != _tmp[old_p_offset])
			{
				auto oids = it->second;
				for(auto oid: oids)
				{
					_values[o_offset] = oid;
					o_offset++;
					if(Util::is_entity_ele(_values[o_offset-1])) entity_num++;
				}
				it++;
			}
			else if(_values[p_offset] != it->first && _values[p_offset] == _tmp[old_p_offset])
			{
				int old_o_offset = _tmp[old_p_offset+1];
				while(old_o_offset < next_o_offset)
				{
					_values[o_offset] = _tmp[old_o_offset];
					old_o_offset++;
					o_offset++;
					if(Util::is_entity_ele(_values[o_offset-1])) entity_num++;
				}
				old_p_offset += 2;
			}
			else
			{
				cerr << "error!" << endl;
			}
			if(p_offset + 3 < p_len)
				_values[p_offset + 3] = o_offset;
			p_offset += 2;
		}

		while(it != mp.end())
		{
			assert(_values[p_offset] == it->first);
			auto oids = it->second;
			for(auto oid: oids)
			{
				_values[o_offset] = oid;
				o_offset++;
				if(Util::is_entity_ele(_values[o_offset-1])) entity_num++;
			}
			if(p_offset + 3 < p_len)
				_values[p_offset + 3] = o_offset;
			it++;
			p_offset += 2;
		}

		while(old_p_offset < old_p_len)
		{
			assert(_values[p_offset] == _tmp[old_p_offset]);
			int old_o_offset = _tmp[old_p_offset+1];
			int next_o_offset;
			if(old_p_offset + 3 > old_p_len){
				next_o_offset = _len; //to the last pid
			}
			else{
				next_o_offset = _tmp[old_p_offset+3];
			}
			while(old_o_offset < next_o_offset)
			{
				_values[o_offset] = _tmp[old_o_offset];
				old_o_offset++;
				o_offset++;
				if(Util::is_entity_ele(_values[o_offset-1])) entity_num++;
			}
			if(p_offset + 3 < p_len)
				_values[p_offset + 3] = o_offset;
			old_p_offset += 2;
			p_offset += 2;
		}

		int len = o_offset;

		_values[0] = len - 3 - _values[1]*2;
		_values[2] = entity_num;
		//assert(len == _values[0] + 2*_values[1] + 3);
		values = new unsigned[len];
		memcpy(values, _values, sizeof(unsigned) * len);
		values_len = len;
		delete [] _values;
		update_num = values[0] - _tmp[0];
	} 
	delete [] _tmp;
	return update_num;

}

/*
input:
    _pidoidlist: remove PO pairs
	_tmp: input data nums(unsigned nums but not bytes num!)
	_len: bytes nums
	_values: output data
	values_len: output data nums(unsigned nums but not bytes num!)
*/
unsigned 
KVstore::Remove_s2values(const vector<unsigned> &_pidoidlist, unsigned* _tmp,  unsigned long _len, unsigned*& values, unsigned long& values_len) const
{
	if(_pidoidlist.size() == 0){
		values = _tmp;
		values_len = 0;
		return 0;
	} 
	map<unsigned, set<unsigned>> mp;

	for(int i = 0; i < _pidoidlist.size(); i += 2)
    {
        auto res = mp[_pidoidlist[i]].insert(_pidoidlist[i+1]);
	}

	int update_num = 0;
	if (!_len) 
	{
		return 0;
	}

	unsigned* _values = new unsigned[_len];

    auto it = mp.begin();
    int old_p_len = _tmp[1] * 2 + 3;
    int p_len = old_p_len;
    int old_p_offset, p_offset;
    int old_o_offset, o_offset;
    old_p_offset = p_offset = 3;
    old_o_offset = o_offset = _tmp[1]*2 + 3;
    int prev_o_offset = o_offset;
    int pre_num = _tmp[1];
    _values[4] = o_offset;
    int entity_num = 0;
    while(it != mp.end() && old_p_offset < old_p_len)
    {
        int next_o_offset;
        if(old_p_offset + 3 > old_p_len){
            next_o_offset = _len; //to the last pid
        }
        else{
            next_o_offset = _tmp[old_p_offset+3];
        }

        if(_tmp[old_p_offset] == it->first)
        {
            auto oids = it->second;
            auto o_it = oids.begin();
            while(old_o_offset < next_o_offset && o_it != oids.end())
            {
                if(_tmp[old_o_offset] == *o_it) //delete
                {
					//cout << _tmp[old_o_offset] << endl;
                    old_o_offset++;
                    o_it++;
                }
                else if(_tmp[old_o_offset] > *o_it)
                {
                    o_it++;
                }
                else
                {
                    _values[o_offset] = _tmp[old_o_offset];
                    if(Util::is_entity_ele(_values[o_offset])) entity_num++;
                    o_offset++;
                    old_o_offset++;
                }
            }

            while(old_o_offset < next_o_offset)
            {
                _values[o_offset] = _tmp[old_o_offset];
                if(Util::is_entity_ele(_values[o_offset])) entity_num++;
                o_offset++;
                old_o_offset++;
            }

            while(o_it != oids.end())
            {
                o_it++;
                //do nothing
            }

            if(prev_o_offset != o_offset){ //no need delete pre
                _values[p_offset] = _tmp[old_p_offset];
                if(p_offset + 3 < p_len)
                    _values[p_offset + 3] = o_offset;
                p_offset += 2;
            }
            else
            {
                pre_num--;
            }
            old_p_offset += 2;
            it++;

            prev_o_offset = o_offset;
        }
        else if(_tmp[old_p_offset] > it->first)
        {
            it++;
        }
        else if(_tmp[old_p_offset] < it->first)
        {
            while(old_o_offset < next_o_offset)
            {
                _values[o_offset] = _tmp[old_o_offset];
                if(Util::is_entity_ele(_values[o_offset])) entity_num++;
                o_offset++;
                old_o_offset++;
            }
            //set predicate
            _values[p_offset] = _tmp[old_p_offset];
            if(p_offset + 3 < p_len)
                _values[p_offset + 3] = o_offset;
            p_offset += 2;
            old_p_offset += 2;
            //record prev offset
            prev_o_offset = o_offset;
        }
    }

    while(it != mp.end()){
        it++; //do nothing
    }

    while(old_p_offset < old_p_len)
    {
		cout << " old_p_offset < old_p_len " << endl;
        int next_o_offset;
        if(old_p_offset + 3 > old_p_len){
            next_o_offset = _len; //to the last pid
        }
        else{
            next_o_offset = _tmp[old_p_offset+3];
        }

        while(old_o_offset < next_o_offset)
        {
            _values[o_offset] = _tmp[old_o_offset];
            if(Util::is_entity_ele(_values[o_offset])) entity_num++;
            o_offset++;
            old_o_offset++;
        }
        if(p_offset + 3 < p_len)
            _values[p_offset + 3] = o_offset;
        _values[p_offset] = _tmp[old_p_offset];
        p_offset += 2;
        old_p_offset += 2;
    }
    _values[0] = o_offset - old_p_len;
    //cout << o_offset << " " << old_p_len << " " << _values[0] << endl;
    int deleted_pre_num = _tmp[1] - pre_num;
    _values[1] = pre_num;
    _values[2] = entity_num;
    int len = pre_num*2 + 3 + _values[0];
    values = new unsigned[len];
    values[0] = _values[0];
    values[1] = _values[1];
    values[2] = _values[2];
    for(int i = 3; i < 2*pre_num + 3; i += 2){
        values[i] = _values[i];
        values[i+1] = _values[i+1] - deleted_pre_num*2;
    }
    for(int i = 2*pre_num + 3, j = _tmp[1] * 2 + 3; i < len && j < _len; i++, j++)
        values[i] = _values[j];
	values_len = len;
	delete [] _values;
	update_num =  _tmp[0] - values[0];
	delete [] _tmp;
	if(values[0] == 0){
        delete [] values;
        values = nullptr;
        values_len = 0;
    }
	return update_num;
}
	
bool 
KVstore::updateInsert_o2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id) 
{
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->objID2values, _obj_id, (char*&)_tmp, _len);

	//objID doesn't exist
	if (!_get) {
		//unsigned _values[5];
		unsigned* _values = new unsigned[5];
		_values[0] = 1;
		_values[1] = 1;
		_values[2] = _pre_id;
		_values[3] = 4;
		_values[4] = _sub_id;
		this->addValueByKey(this->objID2values, _obj_id, (char*)_values, sizeof(unsigned) * 5);
	}

	//objID exists
	else {
		unsigned* _values;
		unsigned long _values_len;
		unsigned _position = KVstore::binarySearch(_pre_id, _tmp + 2, _tmp[1], 2);

		//preID doesn't exist
		if (_position == INVALID) 
		{
			_values_len = _len / sizeof(unsigned) + 3;
			_values = new unsigned[_values_len];
			_values[0] = _tmp[0] + 1;
			_values[1] = _tmp[1] + 1;
			unsigned i, j;
			for (i = 0, j = 2; i < _tmp[1] && _tmp[2 + 2 * i] < _pre_id; i++, j += 2) {
				_values[j] = _tmp[2 + 2 * i];
				_values[j + 1] = _tmp[3 + 2 * i] + 2;
			}
			_values[j] = _pre_id;
			unsigned _offset_old;
			if (i == _tmp[1]) {
				_offset_old = 2 + 2 * _tmp[1] + _tmp[0];
			}
			else {
				_offset_old = _tmp[3 + 2 * i];
			}
			_values[j + 1] = _offset_old + 2;
			j += 2;
			for (; i < _tmp[1]; i++, j += 2) {
				_values[j] = _tmp[2 + 2 * i];
				_values[j + 1] = _tmp[3 + 2 * i] + 3;
			}
			for (i = 2 + 2 * _tmp[1]; i < 2 + 2 * _tmp[1] + _tmp[0]; i++, j++) {
				if (i == _offset_old) {
					_values[j] = _sub_id;
					j++;
				}
				_values[j] = _tmp[i];
			}
			if (i == _offset_old) {
				_values[j] = _sub_id;
				j++;
			}
		}

		//preID exists
		else {
			_values_len = _len / sizeof(unsigned) + 1;
			_values = new unsigned[_values_len];
			memcpy(_values, _tmp, sizeof(unsigned) * _tmp[3 + 2 * _position]);
			_values[0]++;
			for (unsigned i = _position + 1; i < _tmp[1]; i++) {
				_values[3 + 2 * i]++;
			}
			unsigned i, j;
			unsigned right;
			if (_position == _tmp[1] - 1) {
				right = 2 + 2 * _tmp[1] + _tmp[0];
			}
			else {
				right = _tmp[5 + 2 * _position];
			}
			for (i = _tmp[3 + 2 * _position], j = _tmp[3 + 2 * _position]; i < right && _tmp[i] < _sub_id; i++, j++) {
				_values[j] = _tmp[i];
			}
			_values[j] = _sub_id;
			j++;
			for (; i < 2 + 2 * _tmp[1] + _tmp[0]; i++, j++) {
				_values[j] = _tmp[i];
			}
		}

		this->setValueByKey(this->objID2values, _obj_id, (char*)_values, sizeof(unsigned) * _values_len);
		//delete[] _values;
	}
	delete []_tmp;
	return true;
}

bool 
KVstore::updateRemove_o2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id) 
{
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->objID2values, _obj_id, (char*&)_tmp, _len);

	if (!_get) {
		return false;
	}

	//objID doesn't exist after removal
	if (_tmp[0] == 1) {
		this->removeKey(this->objID2values, _obj_id);
	}

	//objID still exists after removal
	else {
		unsigned* _values;
		unsigned long _values_len;
		unsigned _position = KVstore::binarySearch(_pre_id, _tmp + 2, _tmp[1], 2);
		unsigned _sidlen_op;
		if (_position == _tmp[1] - 1) {
			_sidlen_op = 2 + 2 * _tmp[1] + _tmp[0] - _tmp[3 + 2 * _position];
		}
		else {
			_sidlen_op = _tmp[5 + 2 * _position] - _tmp[3 + 2 * _position];
		}

		//preID doesn't exist after removal
		if (_sidlen_op == 1) {
			_values_len = _len / sizeof(unsigned) - 3;
			_values = new unsigned[_values_len];
			memcpy(_values, _tmp, sizeof(unsigned) * (2 + 2 * _position));
			_values[0]--;
			_values[1]--;
			for (unsigned i = 0; i < _position; i++) {
				_values[3 + 2 * i] -= 2;
			}
			unsigned i, j;
			for (i = 4 + 2 * _position, j = 2 + 2 * _position; i < 2 + 2 * _tmp[1]; i += 2, j += 2) {
				_values[j] = _tmp[i];
				_values[j + 1] = _tmp[i + 1] - 3;
			}
			for (; i < 2 + 2 * _tmp[1] + _tmp[0]; i++, j++) {
				if (i == _tmp[3 + 2 * _position]) {
					j--;
					continue;
				}
				_values[j] = _tmp[i];
			}
		}

		//preID still exists after removal
		else {
			_values_len = _len / sizeof(unsigned) - 1;
			_values = new unsigned[_values_len];
			memcpy(_values, _tmp, sizeof(unsigned) * _tmp[3 + 2 * _position]);
			_values[0]--;
			for (unsigned i = _position + 1; i < _tmp[1]; i++) {
				_values[3 + 2 * i]--;
			}
			unsigned i, j;
			for (i = _tmp[3 + 2 * _position], j = _tmp[3 + 2 * _position];
				i < 2 + 2 * _tmp[1] + _tmp[0] && _tmp[i] < _sub_id; i++, j++) {
				_values[j] = _tmp[i];
			}
			i++;
			for (; i < 2 + 2 * _tmp[1] + _tmp[0]; i++, j++) {
				_values[j] = _tmp[i];
			}
		}

		this->setValueByKey(this->objID2values, _obj_id, (char*)_values, sizeof(unsigned) * _values_len);
		//delete[] _values;
	}
	delete []_tmp;
	return true;
}

//TODO: TO BE IMPROVED
unsigned 
KVstore::updateInsert_o2values(TYPE_ENTITY_LITERAL_ID _objid, const std::vector<unsigned>& _pidsidlist) 
{
	if(_pidsidlist.size() == 0) return 0;
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len);
	_len = _len/sizeof(unsigned);

	unsigned* values = NULL;
	unsigned long values_len = 0;
	unsigned update_num = this->Insert_o2values(_pidsidlist, _tmp, _len, values, values_len);
	if(update_num == 0) return 0;
	if(_len == 0){
		this->addValueByKey(this->objID2values, _objid, (char*)values, sizeof(unsigned) * values_len);
		return update_num;
	}
	else
	{
		this->setValueByKey(this->objID2values, _objid, (char*)values, sizeof(unsigned) * values_len);
		return update_num;
	}
}

unsigned 
KVstore::updateRemove_o2values(TYPE_ENTITY_LITERAL_ID _objid, const std::vector<unsigned>& _pidsidlist) 
{
	if(_pidsidlist.size() == 0) return 0;
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	unsigned update_num = 0;
	bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len);
	_len = _len / sizeof(unsigned);
	if (!_len) {
		return 0;
	}

	unsigned* values = NULL;
	unsigned long values_len = 0;
	update_num = this->Remove_o2values(_pidsidlist, _tmp, _len, values, values_len);
	if(update_num == 0) return 0;
	if(values_len != 0)
		this->setValueByKey(this->objID2values, _objid, (char*)values, sizeof(unsigned) * values_len);
	else{
		this->removeKey(this->objID2values, _objid);
	}

	return update_num;
	
}


unsigned 
KVstore::Insert_o2values(const std::vector<unsigned>& _pidsidlist, unsigned* _tmp,  unsigned long _len, unsigned*& values, unsigned long& values_len) const
{
	if(_pidsidlist.size() == 0)
	{
		values = _tmp;
		values_len = _len;
		return 0;
	}
	map<unsigned, vector<unsigned>> mp;

    int n = 0;
	unsigned update_num = 0;
    for(int i = 0; i < _pidsidlist.size(); i += 2)
    {
		pair<int, bool> res{0, true};
        mp[_pidsidlist[i]].push_back(_pidsidlist[i+1]);
        if(res.second == true) n++;
    }

	//objID doesn't exist
	if (!_len) {
		unsigned* _values = new unsigned[2 + 3*n];
		//cout << 2 + 3*n << endl;
		_values[0] = n;
		_values[1] = mp.size();
		//cout << "mp.size()" << _values[1] << endl;
		int j = 2, len;
		for(auto &it: mp)
		{
			_values[j] = it.first;
			j += 2;
		}
		len = j;
		j = 3;
		for(auto &it: mp)
		{
			_values[j] = len;
			for(auto &sid: it.second)
			{
				_values[len] = sid;
				len++;
			}
			j += 2; //_values[j] = offset
		}
		assert(len == _values[0] + 2*_values[1] + 2);
		values = new unsigned[len];
		memcpy(values, _values, sizeof(unsigned) * len);
		values_len = len;
		delete [] _values;
		update_num = values[0];
	}
	else
	{
		unsigned* _values;
		unsigned long _values_len;
		_values_len = _len + 3*n;
		_values = new unsigned[_values_len];
		memset(_values, 0, sizeof(unsigned)* _values_len);
		auto it = mp.begin();
		int old_p_offset = 2, p_offset = 2;
		int old_p_len = _tmp[1] * 2 + 2;
		//merge pid
		while(it != mp.end() && old_p_offset < old_p_len)
		{
			if(it->first > _tmp[old_p_offset])
			{
				_values[p_offset] = _tmp[old_p_offset];
				old_p_offset += 2;
			}
			else if(it-> first < _tmp[old_p_offset])
			{
				_values[p_offset] = it->first;
				it++;
			}
			else
			{
				_values[p_offset] = it->first;
				it++;
				old_p_offset += 2;
			}
			p_offset += 2;
		}

		while(it != mp.end()){
			_values[p_offset] = it->first;
			it++; p_offset += 2;
		}
		while(old_p_offset < old_p_len){
			_values[p_offset] = _tmp[old_p_offset];
			old_p_offset += 2; p_offset += 2;
		}

		_values[1] = p_offset / 2 - 1;

		//merge sid
		it = mp.begin();
		int s_offset = p_offset;
		int p_len = p_offset;

		p_offset = old_p_offset = 2;
		_values[3] = s_offset;

		while(it != mp.end() && old_p_offset < old_p_len)
		{
			//merge sids
			int next_s_offset;
			if(old_p_offset + 3 > old_p_len){
				next_s_offset = _len; //to the last pid
			}
			else{
				next_s_offset = _tmp[old_p_offset+3];
			}
			if(_values[p_offset] == it->first && _values[p_offset] == _tmp[old_p_offset])
			{
				auto sids = it->second;
				auto s_it = sids.begin();
				int old_s_offset = _tmp[old_p_offset+1];
				while(s_it != sids.end() && old_s_offset < next_s_offset)
				{
					if(*s_it > _tmp[old_s_offset])
					{
						_values[s_offset] = _tmp[old_s_offset];
						old_s_offset++;
						s_offset++;
					}
					else if(*s_it < _tmp[old_s_offset])
					{
						_values[s_offset] = *s_it;
						s_it++;
						s_offset++;
					}
					else
					{
						_values[s_offset] = _tmp[old_s_offset];
						old_s_offset++;
						s_it++;
						s_offset++;
					}
				}
				while(s_it != sids.end())
				{
					_values[s_offset] = *s_it;
					s_it++;
					s_offset++;
				}
				while(old_s_offset < next_s_offset)
				{
					_values[s_offset] = _tmp[old_s_offset];
					old_s_offset++;
					s_offset++;
				}
				it++;
				old_p_offset += 2;
			}
			else if(_values[p_offset] == it->first && _values[p_offset] != _tmp[old_p_offset])
			{
				auto sids = it->second;
				for(auto sid: sids)
				{
					_values[s_offset] = sid;
					s_offset++;
				}
				it++;
			}
			else if(_values[p_offset] != it->first && _values[p_offset] == _tmp[old_p_offset])
			{
				int old_s_offset = _tmp[old_p_offset+1];
				while(old_s_offset < next_s_offset)
				{
					_values[s_offset] = _tmp[old_s_offset];
					old_s_offset++;
					s_offset++;
				}
				old_p_offset += 2;
			}
			else
			{
				cerr << "error!" << endl;
			}
			if(p_offset + 3 < p_len)
				_values[p_offset + 3] = s_offset;
			p_offset += 2;
		}

		while(it != mp.end())
		{
			assert(_values[p_offset] == it->first);
			auto sids = it->second;
			for(auto sid: sids)
			{
				_values[s_offset] = sid;
				s_offset++;
			}
			if(p_offset + 3 < p_len)
				_values[p_offset + 3] = s_offset;
			it++;
			p_offset += 2;
		}

		while(old_p_offset < old_p_len)
		{
			assert(_values[p_offset] == _tmp[old_p_offset]);
			int old_s_offset = _tmp[old_p_offset+1];
			int next_s_offset;
			if(old_p_offset + 3 > old_p_len){
				next_s_offset = _len; //to the last pid
			}
			else{
				next_s_offset = _tmp[old_p_offset+3];
			}
			while(old_s_offset < next_s_offset)
			{
				_values[s_offset] = _tmp[old_s_offset];
				old_s_offset++;
				s_offset++;
			}
			if(p_offset + 3 < p_len)
				_values[p_offset + 3] = s_offset;
			old_p_offset += 2;
			p_offset += 2;
		}

		int len = s_offset;

		_values[0] = len - 2 - 2*_values[1];
		//assert(len == _values[0] + 2*_values[1] + 2);
		values = new unsigned[len];
		memcpy(values, _values, sizeof(unsigned) * len);
		//cout << _tmp[0] << values[0] << endl;
		values_len = len;
		delete [] _values;
		update_num = values[0] - _tmp[0];
	}
	delete []_tmp;
	return update_num;
}

unsigned 
KVstore::Remove_o2values(const std::vector<unsigned>& _pidsidlist, unsigned* _tmp,  unsigned long _len, unsigned*& values, unsigned long& values_len) const
{
	unsigned update_num = 0;
	if (!_len) {
		values = _tmp;
		values_len = _len;
		return 0;
	}

	map<unsigned, set<unsigned>> mp;

    for(int i = 0; i < _pidsidlist.size(); i += 2)
    {
       auto res = mp[_pidsidlist[i]].insert(_pidsidlist[i+1]);
    }

    unsigned* _values = new unsigned[_len];

    auto it = mp.begin();
    int old_p_len = _tmp[1] * 2 + 2;
    int p_len = old_p_len;
    int old_p_offset, p_offset;
    int old_s_offset, s_offset;
    old_p_offset = p_offset = 2;
    old_s_offset = s_offset = _tmp[1]*2 + 2;
    int prev_s_offset = s_offset;
    int pre_num = _tmp[1];
    _values[3] = s_offset;
    while(it != mp.end() && old_p_offset < old_p_len)
    {
        int next_s_offset;
        if(old_p_offset + 3 > old_p_len){
            next_s_offset = _len; //to the last pid
        }
        else{
            next_s_offset = _tmp[old_p_offset+3];
        }

        if(_tmp[old_p_offset] == it->first)
        {
            auto sids = it->second;
            auto s_it = sids.begin();
            while(old_s_offset < next_s_offset && s_it != sids.end())
            {
                if(_tmp[old_s_offset] == *s_it) //delete
                {
                    old_s_offset++;
                    s_it++;
                }
                else if(_tmp[old_s_offset] > *s_it)
                {
                    s_it++;
                }
                else
                {
                    _values[s_offset] = _tmp[old_s_offset];
                    s_offset++;
                    old_s_offset++;
                }
            }

            while(old_s_offset < next_s_offset)
            {
                _values[s_offset] = _tmp[old_s_offset];
                s_offset++;
                old_s_offset++;
            }

            while(s_it != sids.end())
            {
                s_it++;
                //do nothing
            }

            if(prev_s_offset != s_offset){ //no need delete pre
                _values[p_offset] = _tmp[old_p_offset];
                if(p_offset + 3 < p_len)
                    _values[p_offset + 3] = s_offset;
                p_offset += 2;
            }
            else
            {
                pre_num--;
            }
            old_p_offset += 2;
            it++;

            prev_s_offset = s_offset;
        }
        else if(_tmp[old_p_offset] > it->first)
        {
            it++;
        }
        else if(_tmp[old_p_offset] < it->first)
        {
            while(old_s_offset < next_s_offset)
            {
                _values[s_offset] = _tmp[old_s_offset];
                s_offset++;
                old_s_offset++;
            }
            //set predicate
            _values[p_offset] = _tmp[old_p_offset];
            if(p_offset + 3 < p_len)
                _values[p_offset + 3] = s_offset;
            p_offset += 2;
            old_p_offset += 2;
            //record prev offset
            prev_s_offset = s_offset;
        }
    }

    while(it != mp.end()){
        it++; //do nothing
    }

    while(old_p_offset < old_p_len)
    {
        int next_s_offset;
        if(old_p_offset + 3 > old_p_len){
            next_s_offset = _len; //to the last pid
        }
        else{
            next_s_offset = _tmp[old_p_offset+3];
        }

        while(old_s_offset < next_s_offset)
        {
            _values[s_offset] = _tmp[old_s_offset];
            s_offset++;
            old_s_offset++;
        }
        if(p_offset + 3 < p_len)
            _values[p_offset + 3] = s_offset;
        _values[p_offset] = _tmp[old_p_offset];
        p_offset += 2;
        old_p_offset += 2;
    }
    _values[0] = s_offset - old_p_len;
    int deleted_pre_num = _tmp[1] - pre_num;
    _values[1] = pre_num;
    int len = pre_num*2 + 2 + _values[0];
    values = new unsigned[len];
    values[0] = _values[0];
    values[1] = _values[1];
    for(int i = 2; i < 2*pre_num + 2; i += 2){
        values[i] = _values[i];
        values[i+1] = _values[i+1] - deleted_pre_num*2;
    }
    for(int i = 2*pre_num + 2, j = _tmp[1] * 2 + 2; i < len && j < _len; i++, j++)
        values[i] = _values[j];
	delete [] _values;
	values_len = len;
	update_num = _tmp[0] - values[0];
	delete [] _tmp;
	if(values[0] == 0)
    {
        delete [] values;
        values = nullptr;
        values_len = 0;
    }
	return update_num;
}

bool 
KVstore::updateInsert_p2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id) 
{
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->preID2values, _pre_id, (char*&)_tmp, _len);
	
	//preid doesn't exist
	if (!_get) {
		//unsigned _values[3];
		unsigned* _values = new unsigned[3];
		_values[0] = 1;
		_values[1] = _sub_id;
		_values[2] = _obj_id;
		this->addValueByKey(this->preID2values, _pre_id, (char*)_values, sizeof(unsigned) * 3);
	}

	//preid exists
	else {
		unsigned long  _values_len = _len / sizeof(unsigned) + 2;
		unsigned* _values = new unsigned[_values_len];
		unsigned i, j;
		_values[0] = _tmp[0] + 1;
		for (i = 1, j = 1;
			i < 1 + _tmp[0] && (_tmp[i] < _sub_id || (_tmp[i] == _sub_id && _tmp[i + _tmp[0]] < _obj_id));
			i++, j++) {
			_values[j] = _tmp[i];
			_values[j + _tmp[0] + 1] = _tmp[i + _tmp[0]];
		}
		_values[j] = _sub_id;
		_values[j + _tmp[0] + 1] = _obj_id;
		j++;
		for (; i < 1 + _tmp[0]; i++, j++) {
			_values[j] = _tmp[i];
			_values[j + _tmp[0] + 1] = _tmp[i + _tmp[0]];
		}
		this->setValueByKey(this->preID2values, _pre_id, (char*)_values, sizeof(unsigned) * _values_len);
		//delete[] _values;
	}
	delete []_tmp;
	return true;
}

bool 
KVstore::updateRemove_p2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id) 
{
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->preID2values, _pre_id, (char*&)_tmp, _len);

	if (!_get) {
		return false;
	}

	//preid doesn't exist after removal
	if (_tmp[0] == 1) {
		this->removeKey(this->preID2values, _pre_id);
	}

	//preid still exists after removal
	else {
		unsigned long _values_len = _len / sizeof(unsigned) - 2;
		unsigned* _values = new unsigned[_values_len];
		unsigned i, j;
		_values[0] = _tmp[0] - 1;
		for (i = 1, j = 1;
			i < 1 + _tmp[0] && (_tmp[i] < _sub_id || (_tmp[i] == _sub_id && _tmp[i + _tmp[0]] < _obj_id));
			i++, j++) {
			_values[j] = _tmp[i];
			_values[j + _tmp[0] - 1] = _tmp[i + _tmp[0]];
		}
		i++;
		for (; i < 1 + _tmp[0]; i++, j++) {
			_values[j] = _tmp[i];
			_values[j + _tmp[0] - 1] = _tmp[i + _tmp[0]];
		}
		this->setValueByKey(this->preID2values, _pre_id, (char*)_values, sizeof(unsigned) * _values_len);
		//delete[] _values;
	}
	delete []_tmp;
	return true;
}

unsigned 
KVstore::updateInsert_p2values(TYPE_PREDICATE_ID _preid, const std::vector<unsigned>& _sidoidlist) 
{
	if(_sidoidlist.size() == 0) return 0;
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->preID2values, _preid, (char*&)_tmp, _len);
	_len = _len / sizeof(unsigned);
	//cout << "updateInsert_p2values........" << endl;
	unsigned* values = NULL;
	unsigned long values_len = 0;
	unsigned update_num = this->Insert_p2values(_sidoidlist, _tmp, _len, values, values_len);
	if(update_num == 0) return 0;
	if(_len == 0){
		//cout << "...............preID2values addValueByKey" << endl;
		this->addValueByKey(this->preID2values, _preid, (char*)values, sizeof(unsigned)*values_len);
	}
	else
	{
		this->setValueByKey(this->preID2values, _preid, (char*)values, sizeof(unsigned)*values_len);
	}
	return update_num;
}

unsigned 
KVstore::updateRemove_p2values(TYPE_PREDICATE_ID _preid, const std::vector<unsigned>& _sidoidlist) 
{
	if(_sidoidlist.size() == 0) return 0;
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	bool _get = this->getValueByKey(this->preID2values, _preid, (char*&)_tmp, _len);
	_len = _len / sizeof(unsigned);

	unsigned* values = NULL;
	unsigned long values_len = 0;
	unsigned update_num = this->Remove_p2values(_sidoidlist, _tmp, _len, values, values_len);
	if(update_num == 0) {
		return 0;
	}
	if(values_len != 0){
		this->setValueByKey(this->preID2values, _preid, (char*)values, sizeof(unsigned)*values_len);
	}
	else
	{
		this->removeKey(this->preID2values, _preid);
	}
	return update_num;
}

unsigned 
KVstore::Insert_p2values(const std::vector<unsigned>& _sidoidlist, unsigned* _tmp,  unsigned long _len, unsigned*& values, unsigned long& values_len) const
{
	if(_sidoidlist.size() == 0)
	{
		values = _tmp;
		values_len = _len;
		return 0;
	}
	map<unsigned, vector<unsigned>> mp;

    int n = 0;
	unsigned update_num = 0;
    for(int i = 0; i < _sidoidlist.size(); i += 2)
    {
       pair<int, bool> res = {0, true};
	   mp[_sidoidlist[i]].push_back(_sidoidlist[i+1]);
       if(res.second == true) n++;
    }

	//preid doesn't exist
	if (!_len) {
		values = new unsigned[1 + 2*n];
		values[0] = n;
		int i = 1;
		for(auto &it: mp)
		{
			int sid = it.first;
			auto oids = it.second;
			for(auto oid: oids)
			{
				values[i] = sid;
				values[i + n] = oid;
				i++;
			}
		}
		assert(i == n+1);
		values_len = 2*n + 1;
		update_num = n;
	}
	else
	{
		unsigned* _values;
		unsigned long _values_len;
		_values_len = _len + 2*n;
		_values = new unsigned[_values_len];
		memset(_values, -1, sizeof(unsigned)* _values_len);

		auto it = mp.begin();
		int old_len = _tmp[0];
		int tmp_len = (_values_len - 1) / 2;
		assert(tmp_len == n + old_len);
		int old_s_offset = 1, s_offset = 1;
		while(old_s_offset <= _tmp[0] && it != mp.end())
		{
			if(_tmp[old_s_offset] == it->first)
			{
				auto oids = it->second;
				auto o_it = oids.begin();
				while(o_it != oids.end() && old_s_offset <= _tmp[0] && _tmp[old_s_offset] == it->first)
				{
					if(*o_it < _tmp[old_s_offset + old_len])
					{
						_values[s_offset] = it->first;
						_values[s_offset + tmp_len] = *o_it;
						o_it++;
					}
					else if(*o_it > _tmp[old_s_offset + old_len])
					{
						_values[s_offset] = _tmp[old_s_offset];
						_values[s_offset + tmp_len] = _tmp[old_s_offset + old_len];
						old_s_offset++;
					}
					else
					{
						_values[s_offset] = _tmp[old_s_offset];
						_values[s_offset + tmp_len] = _tmp[old_s_offset + old_len];
						o_it++;
						old_s_offset++;
					}
					s_offset++;
				}

				while(o_it != oids.end())
				{
					_values[s_offset] = it->first;
					_values[s_offset + tmp_len] = *o_it;
					o_it++;
					s_offset++;
				}

				while(old_s_offset <= _tmp[0] && _tmp[old_s_offset] == it->first)
				{
					_values[s_offset] = _tmp[old_s_offset];
					_values[s_offset + tmp_len] = _tmp[old_s_offset + old_len];
					old_s_offset++;
					s_offset++;
				}
				it++;
			}
			else if(_tmp[old_s_offset] > it->first)
			{
				auto oids = it->second;
				auto o_it = oids.begin();
				while(o_it != oids.end())
				{
					_values[s_offset] = it->first;
					_values[s_offset + tmp_len] = *o_it;
					s_offset++; o_it++;
				}
				it++;
			}
			else
			{
				while(old_s_offset <= _tmp[0] && _tmp[old_s_offset] < it->first)
				{
					_values[s_offset] = _tmp[old_s_offset];
					_values[s_offset + tmp_len] = _tmp[old_s_offset + old_len];
					old_s_offset++;
					s_offset++;
				}
			}
		}

		while(old_s_offset <= _tmp[0])
		{
			_values[s_offset] = _tmp[old_s_offset];
			_values[s_offset + tmp_len] = _tmp[old_s_offset + old_len];
			old_s_offset++;
			s_offset++;
		}

		while(it != mp.end())
		{
			auto oids = it->second;
			auto o_it = oids.begin();
			while(o_it != oids.end())
			{
				_values[s_offset] = it->first;
				_values[s_offset + tmp_len] = *o_it;
				s_offset++; o_it++;
			}
			it++;
		}

		//compaction
		_values[0] = s_offset - 1;
		values = new unsigned[1 + 2*_values[0]];
		values[0] = _values[0];
		for(int i = 1; i < s_offset; i++)
		{
			values[i] = _values[i];
			values[i+values[0]] = _values[i+tmp_len];
		}
		values_len = 2*values[0] + 1;
		delete [] _values;
		update_num = values[0] - _tmp[0];
	}

	delete []_tmp;
	return update_num;
}

unsigned 
KVstore::Remove_p2values(const std::vector<unsigned>& _sidoidlist, unsigned* _tmp,  unsigned long _len, unsigned*& values, unsigned long& values_len) const
{
	if(_sidoidlist.size() == 0)
	{
		values = _tmp;
		values_len = _len;
		return 0;
	}
	map<unsigned, set<unsigned>> mp;

    int n = 0;
	unsigned update_num = 0;
    for(int i = 0; i < _sidoidlist.size(); i += 2)
    {
       auto res = mp[_sidoidlist[i]].insert(_sidoidlist[i+1]);
       if(res.second == true) n++;
    }
	
	if (!_len) {
		return 0;
	}

	 unsigned* _values;
    _values = new unsigned[_len];
    memset(_values, -1, sizeof(unsigned)* _len);

    auto it = mp.begin();
    int old_len = _tmp[0];
    int tmp_len = old_len;
    int old_s_offset = 1, s_offset = 1;
    while(old_s_offset <= _tmp[0] && it != mp.end())
    {
        if(_tmp[old_s_offset] == it->first)
        {
            auto oids = it->second;
            auto o_it = oids.begin();
            while(o_it != oids.end() && old_s_offset <= _tmp[0] && _tmp[old_s_offset] == it->first)
            {
                if(*o_it < _tmp[old_s_offset + old_len])
                {
                    o_it++;
                }
                else if(*o_it > _tmp[old_s_offset + old_len])
                {
                    _values[s_offset] = _tmp[old_s_offset];
                    _values[s_offset + tmp_len] = _tmp[old_s_offset + old_len];
                    old_s_offset++;
                    s_offset++;
                }
                else
                {
                    o_it++;
                    old_s_offset++;
                }
            }

            while(o_it != oids.end())
            {
                o_it++;
                //do nothing
            }

            while(old_s_offset <= _tmp[0] && _tmp[old_s_offset] == it->first)
            {
                _values[s_offset] = _tmp[old_s_offset];
                _values[s_offset + tmp_len] = _tmp[old_s_offset + old_len];
                old_s_offset++;
                s_offset++;
            }
            it++;
        }
        else if(_tmp[old_s_offset] > it->first)
        {
            it++; //do nothing
        }
        else
        {
            while(old_s_offset <= _tmp[0] && _tmp[old_s_offset] < it->first)
            {
                _values[s_offset] = _tmp[old_s_offset];
                _values[s_offset + tmp_len] = _tmp[old_s_offset + old_len];
                old_s_offset++;
                s_offset++;
            }
        }
    }

    while(old_s_offset <= _tmp[0])
    {
        _values[s_offset] = _tmp[old_s_offset];
        _values[s_offset + tmp_len] = _tmp[old_s_offset + old_len];
        old_s_offset++;
        s_offset++;
    }

    while(it != mp.end())
    {
        it++; //do nothing
    }

    //compaction
    _values[0] = s_offset - 1;
    values = new unsigned[1 + 2*_values[0]];
    values[0] = _values[0];
    for(int i = 1; i < s_offset; i++)
    {
        values[i] = _values[i];
        values[i+values[0]] = _values[i+tmp_len];
    }
    values_len = 2*values[0] + 1;
	delete [] _values;
	
	update_num = _tmp[0] - values[0];
	delete [] _tmp;
	if(values[0] == 0)
    {
        delete [] values;
        values = nullptr;
        values_len = 0;
    }
	return update_num;
}

//for entity2id
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool 
KVstore::open_entity2id(int _mode) 
{
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_entity2id_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_entity2id_query;
	}
	else 
	{
		cerr << "Invalid open mode in open_entity2id, mode = " << _mode << endl;
		return false;
	}

	return this->open(this->entity2id, KVstore::s_entity2id, _mode, buffer_size);
}

bool 
KVstore::close_entity2id() 
{
	if (this->entity2id == NULL) 
	{
		return true;
	}

  this->entity2id->Save();
	delete this->entity2id;
	this->entity2id = NULL;

	return true;
}

bool 
KVstore::subIDByEntity(string _entity0) 
{
	//NOTICE: no need to copy _entity to a char* buffer
	//_entity will not be released befor ethis function ends
	//so _entity.c_str() is a valid const char*
	//this->load_trie();
	//string _entity = trie->Compress(_entity0);
	//return this->entity2id->remove(_entity.c_str(), _entity.length());
	return this->removeKey(this->entity2id, _entity0.c_str(), _entity0.length());
}

TYPE_ENTITY_LITERAL_ID
KVstore::getIDByEntity(string _entity0) const  
{
	//this->load_trie();
	//string _entity = trie->Compress(_entity0);
	return this->getIDByStr(this->entity2id, _entity0.c_str(), _entity0.length());
}

/**
 * Set ID to entity
 * @warning You Must Look up _entity0 first!
 * Only if it doesn't have an ID can you insert it!
 * @param _entity0 the inserted entity
 * @param _id the given id
 * @return bool
 */
bool 
KVstore::setIDByEntity(string _entity0, TYPE_ENTITY_LITERAL_ID _id) 
{
	//return this->addValueByKey(this->entity2id, _entity.c_str(), _entity.length(), _id);
	//int len = _entity.length() + 1;

	//this->load_trie();
	//string _entity = trie->Compress(_entity0);
	int len = _entity0.length();
	char* str = new char[len];

	memcpy(str, _entity0.c_str(), len);
	return this->addValueByKey(this->entity2id, str, len, _id);
}

//for id2entity
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool 
KVstore::open_id2entity(int _mode) 
{
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2entity_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2entity_query;
	}
	else 
	{
		cerr << "Invalid open mode in open_id2entity, mode = " << _mode << endl;
		return false;
	}

	return this->open(this->id2entity, KVstore::s_id2entity, _mode, buffer_size);
}

bool 
KVstore::close_id2entity() 
{

	if (this->id2entity == NULL) 
	{
		return true;
	}

	this->id2entity->save();
	delete this->id2entity;
	this->id2entity = NULL;

	return true;
}

bool 
KVstore::subEntityByID(TYPE_ENTITY_LITERAL_ID _id) 
{
	//return this->id2entity->remove(_id);
	return this->removeKey(this->id2entity, _id);
}

string 
KVstore::getEntityByID(TYPE_ENTITY_LITERAL_ID _id,  bool needUnCompress ) const
{
	char* _tmp = NULL;
	unsigned _len = 0;

	bool _get = this->getValueByKey(this->id2entity, _id, _tmp, _len);
	if (!_get) 
	{
		return "";
	}

	//NOTICE: no need to add \0 at last if we indicate the length
//	cout << "in getEntityByID: ";
//	cout << _tmp << endl;
//	unsigned length = _len + 1;
//	char * debug = new char [length];
//	memcpy(debug, _tmp, _len);
//	debug[_len] = '\0';
//	cout << "in getEntityByID (add): ";
//	_tmp = debug;
	//cout << _tmp << endl;	
	string _ret;
	if (needUnCompress)
	{
		trie->Uncompress(_tmp, _len, _ret);
	}
	else
		_ret.assign(_tmp, _len);
	delete [] _tmp;
//	cout << "string is: " << _ret << endl;
//	string test1 = _ret;
//	test1.append('\0');
//	cout << "test1 is " << test1 << endl;
	//cout << "test2 is " << test2 << endl;
	return _ret;
}

bool 
KVstore::setEntityByID(TYPE_ENTITY_LITERAL_ID _id, string _entity0) 
{
	//return this->addValueByKey(this->id2entity, _id, _entity.c_str(), _entity.length());
	//int len = _entity.length() + 1;
	//string _entity = this->trie->Compress(_entity0);

	int len = _entity0.length();
	char* str = new char[len];

	memcpy(str, _entity0.c_str(), len);

	return this->addValueByKey(this->id2entity, _id, str, len);
}

//for predicate2id
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool 
KVstore::open_predicate2id(int _mode) 
{
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_predicate2id_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_predicate2id_query;
	}
	else 
	{
		cerr << "Invalid open mode in open_predicate2id, mode = " << _mode << endl;
		return false;
	}

	return this->open(this->predicate2id, KVstore::s_predicate2id, _mode, buffer_size);
}

bool 
KVstore::close_predicate2id() 
{
	if (this->predicate2id == NULL) 
	{
		return true;
	}

  this->predicate2id->Save();
	delete this->predicate2id;
	this->predicate2id = NULL;

	return true;
}

bool 
KVstore::subIDByPredicate(string _predicate0) 
{
	//this->load_trie();
	//string _predicate = trie->Compress(_predicate0);
	//return this->predicate2id->remove(_predicate.c_str(), _predicate.length());
	return this->removeKey(this->predicate2id, _predicate0.c_str(), _predicate0.length());
}

TYPE_PREDICATE_ID
KVstore::getIDByPredicate(string _predicate0) const 
{
	//this->load_trie();
	//string _predicate = trie->Compress(_predicate0);
	return this->getIDByStr(this->predicate2id, _predicate0.c_str(), _predicate0.length());
}

/**
 * Set ID to Predicate string
 * @warning You Must Look up _predicate0 first!
 * Only if it doesn't have an ID can you insert it!
 * @param _predicate0 the inserted Predicate
 * @param _id the given id
 * @return bool
 */
bool 
KVstore::setIDByPredicate(string _predicate0, TYPE_PREDICATE_ID _id) 
{
	//return this->addValueByKey(this->predicate2id, _predicate.c_str(), _predicate.length(), _id);
	//int len = _predicate.length() + 1;
	//this->load_trie();
	//string _predicate = trie->Compress(_predicate0);
	char* str = new char[_predicate0.length()];
	memcpy(str, _predicate0.c_str(), _predicate0.length());
	return this->addValueByKey(this->predicate2id, str, _predicate0.length(), _id);
}

//for id2predicate
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool 
KVstore::open_id2predicate(int _mode) 
{
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2predicate_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2predicate_query;
	}
	else 
	{
		cerr << "Invalid open mode in open_id2predicate, mode = " << _mode << endl;
		return false;
	}

	return this->open(this->id2predicate, KVstore::s_id2predicate, _mode, buffer_size);
}

bool 
KVstore::close_id2predicate() 
{
	if (this->id2predicate == NULL) 
	{
		return true;
	}

	this->id2predicate->save();
	delete this->id2predicate;
	this->id2predicate = NULL;

	return true;
}

bool 
KVstore::subPredicateByID(TYPE_PREDICATE_ID _id) 
{
	//return this->id2predicate->remove(_id);
	return this->removeKey(this->id2predicate, _id);
}

string 
KVstore::getPredicateByID(TYPE_PREDICATE_ID _id, bool needUnCompress ) const
{
	char* _tmp = NULL;
	unsigned _len = 0;

	bool _get = this->getValueByKey(this->id2predicate, _id, _tmp, _len);
	if (!_get)
	{
		return "";
	}

	string _ret;
	if (needUnCompress)
	{
		trie->Uncompress(_tmp, _len, _ret);
	}
	else
		_ret.assign(_tmp, _len);
	delete []_tmp;
	return _ret;
}

bool 
KVstore::setPredicateByID(TYPE_PREDICATE_ID _id, string _predicate0) 
{
	//return this->addValueByKey(this->id2predicate, _id, _predicate.c_str(), _predicate.length());
	//int len = _predicate.length() + 1;
	//string _predicate = trie->Compress(_predicate0);
	int len = _predicate0.length();
	char* str = new char[len];
	memcpy(str, _predicate0.c_str(), len);

	return this->addValueByKey(this->id2predicate, _id, str, len);
}

//for literal2id
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool 
KVstore::open_literal2id(int _mode) 
{
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_literal2id_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_literal2id_query;
	}
	else 
	{
		cerr << "Invalid open mode in open_literal2id, mode = " << _mode << endl;
		return false;
	}

	return this->open(this->literal2id, KVstore::s_literal2id, _mode, buffer_size);
}

bool 
KVstore::close_literal2id() 
{
	if (this->literal2id == NULL) 
	{
		return true;
	}

  this->literal2id->Save();
	delete this->literal2id;
	this->literal2id = NULL;

	return true;
}

bool 
KVstore::subIDByLiteral(string _literal0) 
{
	//this->load_trie();
	//string _literal = trie->Compress(_literal0);
	//return this->literal2id->remove(_literal.c_str(), _literal.length());
	return this->removeKey(this->literal2id, _literal0.c_str(), _literal0.length());
}

TYPE_ENTITY_LITERAL_ID
KVstore::getIDByLiteral(string _literal0) const 
{
	//this->load_trie();
	//string _literal = trie->Compress(_literal0);
	return this->getIDByStr(this->literal2id, _literal0.c_str(), _literal0.length());
	//TYPE_ENTITY_LITERAL_ID id = this->getIDByStr(this->literal2id, _literal.c_str(), _literal.length());
	//if(id != INVALID)
	//{
		//id += Util::LITERAL_FIRST_ID;
	//}
}

/**
 * Set ID to Literale string
 * @warning You Must Look up _literal0 first!
 * Only if it doesn't have an ID can you insert it!
 * @param _literal0 the inserted literal
 * @param _id the given id
 * @return bool
 */
bool 
KVstore::setIDByLiteral(string _literal0, TYPE_ENTITY_LITERAL_ID _id) 
{
	//return this->addValueByKey(this->literal2id, _literal.c_str(), _literal.length(), _id);
	//int len = _literal.length() + 1;
	//this->load_trie();

	//string _literal = trie->Compress(_literal0);	
	int len = _literal0.length();
	char* str = new char[len];
	memcpy(str, _literal0.c_str(), len);

	return this->addValueByKey(this->literal2id, str, len, _id);
}

//for id2literal
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool 
KVstore::open_id2literal(int _mode) 
{
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2literal_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2literal_query;
	}
	else 
	{
		cerr << "Invalid open mode in open_id2literal, mode = " << _mode << endl;
		return false;
	}

	return this->open(this->id2literal, KVstore::s_id2literal, _mode, buffer_size);
}

bool 
KVstore::close_id2literal() 
{
	if (this->id2literal == NULL) 
	{
		return true;
	}

	this->id2literal->save();
	delete this->id2literal;
	this->id2literal = NULL;

	return true;
}

bool 
KVstore::subLiteralByID(TYPE_ENTITY_LITERAL_ID _id) 
{
	//return this->id2literal->remove(_id - Util::LITERAL_FIRST_ID);
	return this->removeKey(this->id2literal, _id);
}

string 
KVstore::getLiteralByID(TYPE_ENTITY_LITERAL_ID _id, bool needUnCompress ) const
{
	char* _tmp = NULL;
	unsigned _len = 0;

	bool _get = this->getValueByKey(this->id2literal, _id, _tmp, _len);
	if (!_get) 
	{
		//NOTICE:here assumes that all literals cannot be empty: ""
		return "";
	}
	string _ret ;
	if (needUnCompress)
	{
		trie->Uncompress(_tmp, _len, _ret);
	}
	else
		_ret.assign(_tmp, _len);
	delete []_tmp;
	return _ret;
}

bool 
KVstore::setLiteralByID(TYPE_ENTITY_LITERAL_ID _id, string _literal0) 
{
	//return this->addValueByKey(this->id2literal, _id, _literal.c_str(), _literal.length());
	//int len = _literal.length() + 1;
	//string _literal = trie->Compress(_literal0);
	int len = _literal0.length();
	char* str = new char[len];
	memcpy(str, _literal0.c_str(), len);

	return this->addValueByKey(this->id2literal, _id, str, len);
}

bool 
KVstore::open_subID2values(int _mode, TYPE_ENTITY_LITERAL_ID _entity_num) 
{
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_sID2values_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_sID2values_query;
	}
	else 
	{
		cerr << "Invalid open mode in open_subID2values, mode = " << _mode << endl;
		return false;
	}

	return this->open(this->subID2values, KVstore::s_sID2values, _mode, buffer_size, _entity_num);
}

bool 
KVstore::close_subID2values() 
{
	if (this->subID2values == NULL) 
	{
		return true;
	}

	this->subID2values->save();
	delete this->subID2values;
	this->subID2values = NULL;
	cout << "Done closing subID2values" << endl;

	return true;
}

//STRUCT of s2xx: triple_number pre_num entity_border p1 offset1 p2 offset2 ... pn offsetn
//p1-list(in offset1) p2-list(in offset2) ... pn-list(in offsetn)
//(the final whole list is a unsorted olist)
bool 
KVstore::build_subID2values(ID_TUPLE* _p_id_tuples, TYPE_TRIPLE_NUM _triples_num, TYPE_ENTITY_LITERAL_ID total_entity_num) 
{
	cout << "Begin building subID2values..." << endl;
	//qsort(_p_id_tuples, _triples_num, sizeof(int*), Util::_spo_cmp);
	vector<unsigned> _oidlist_s;
	vector<unsigned> _pidoffsetlist_s;
	//NOTICE: this is used for entity-literal border, but not used now
	//it is only set for the whole olist in s2po, not for sp2o
	unsigned _entity_num = 0;

	//true means the next sub is a different one from the current one
	bool _sub_change = true;
	//true means the next <sub,pre> is different from the current pair
	bool _sub_pre_change = true;
	//true means the next pre is different from the current one
	bool _pre_change = true;

	this->open_subID2values(KVstore::CREATE_MODE, total_entity_num);

	//NOTICE: i*3 + j maybe break the unsigned limit
	//for (unsigned long i = 0; i < _triples_num; i++) 
	for (TYPE_TRIPLE_NUM i = 0; i < _triples_num; i++) 
	{
		if (i + 1 == _triples_num || _p_id_tuples[i].subid != _p_id_tuples[i+1].subid
			|| _p_id_tuples[i].preid != _p_id_tuples[i+1].preid || _p_id_tuples[i].objid != _p_id_tuples[i+1].objid) 
		{
			if (_sub_change) 
			{
				_pidoffsetlist_s.clear();
				_oidlist_s.clear();
				_entity_num = 0;
			}

			TYPE_ENTITY_LITERAL_ID _sub_id = _p_id_tuples[i].subid;
			TYPE_PREDICATE_ID _pre_id = _p_id_tuples[i].preid;
			TYPE_ENTITY_LITERAL_ID _obj_id = _p_id_tuples[i].objid;

			if (_sub_pre_change) 
			{
				_pidoffsetlist_s.push_back(_pre_id);
				_pidoffsetlist_s.push_back(_oidlist_s.size());
			}

			_oidlist_s.push_back(_obj_id);
			if (Util::is_entity_ele(_obj_id)) 
			{
				_entity_num++;
			}

			_sub_change = (i + 1 == _triples_num) || (_p_id_tuples[i].subid != _p_id_tuples[i+1].subid);
			_pre_change = (i + 1 == _triples_num) || (_p_id_tuples[i].preid != _p_id_tuples[i+1].preid);
			_sub_pre_change = _sub_change || _pre_change;

			if (_sub_change) 
			{
				for (unsigned j = 1; j < _pidoffsetlist_s.size(); j += 2) 
				{
					_pidoffsetlist_s[j] += 3 + _pidoffsetlist_s.size();
				}
				unsigned* _entrylist_s = new unsigned[3 + _pidoffsetlist_s.size() + _oidlist_s.size()];
				//triples number
				_entrylist_s[0] = _oidlist_s.size();
				//pre number
				_entrylist_s[1] = _pidoffsetlist_s.size() / 2;
				//entity number
				_entrylist_s[2] = _entity_num;
				unsigned j, k;
				//pidoffsetlist
				for (j = 3, k = 0; k < _pidoffsetlist_s.size(); j++, k++) 
				{
					_entrylist_s[j] = _pidoffsetlist_s[k];
				}
				//unsorted oidlist
				for (k = 0; k < _oidlist_s.size(); j++, k++) 
				{
					_entrylist_s[j] = _oidlist_s[k];
				}

				this->addValueByKey(this->subID2values, _sub_id, (char*)_entrylist_s, sizeof(unsigned) * j);
				//delete[] _entrylist_s;
			}
		}
	}

	this->close_subID2values();
	cout << "Finished building subID2values" << endl;

	return true;
}

bool 
KVstore::getpreIDlistBysubID(TYPE_ENTITY_LITERAL_ID _subid, unsigned*& _preidlist, unsigned& _list_len, bool _no_duplicate, shared_ptr<Transaction> txn) const 
{
	//cout << "In getpreIDlistBysubID " << _subid << endl;
	if (!Util::is_entity_ele(_subid)) {
		_preidlist = NULL;
		_list_len = 0;
		return false;
	}
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	if(txn == nullptr)
	{
		bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);

		if (!_get) 
		{
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}
	
		_list_len = _tmp[1];
		_preidlist = new unsigned[_list_len];
		for (unsigned i = 0; i < _list_len; i++) {
			_preidlist[i] = _tmp[2 * i + 3];
		}

		//if this is a long list, then we should remove itself after copying
		//otherwise, we should not free the list memory
	//	if(VList::listNeedDelete(_len))
	//	{
			delete[] _tmp;
			//_tmp = NULL;
//		}
		return true;
	}
	else
	{
		bool FirstRead = false, latched = false;
		bool IS_SI = txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE;
		//check if get shared-lock
		if(IS_SI)
			FirstRead = !txn->ReadSetFind(_subid, Transaction::IDType::SUBJECT) && !txn->WriteSetFind(_subid, Transaction::IDType::SUBJECT);
		
		VDataSet addset, delset;
		bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len, addset, delset, txn, latched, FirstRead);

		if(IS_SI && latched && FirstRead)
			txn->ReadSetInsert(_subid, Transaction::IDType::SUBJECT);
		
		if(txn->GetState() == TransactionState::ABORTED)
			txn->SetErrorType(TransactionErrorType::SUB_S);
		if (!_get) 
		{
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}

		vector<unsigned> add_pidoidlist, del_pidoidlist;
		for(auto po: addset)
		{
			add_pidoidlist.push_back(po.first);
			add_pidoidlist.push_back(po.second);
		}

		for(auto po: delset)
		{
			del_pidoidlist.push_back(po.first);
			del_pidoidlist.push_back(po.second);
		}

		unsigned * _values = nullptr;
		unsigned long _values_len;
		//cout << ".........._tmp:                                                 " <<  _tmp << endl;
		if(add_pidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_s2values(add_pidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cout << ".........._tmp:                                                 " <<  _tmp << endl;
		if(del_pidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_s2values(del_pidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}

		if(_len == 0) {
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}

		_list_len = _tmp[1];
		_preidlist = new unsigned[_list_len];
		for (unsigned i = 0; i < _list_len; i++) {
			_preidlist[i] = _tmp[2 * i + 3];
		}
		delete[] _tmp;
		return true;
	}
}

bool 
KVstore::getobjIDlistBysubID(TYPE_ENTITY_LITERAL_ID _subid, unsigned*& _objidlist, unsigned& _list_len, bool _no_duplicate, shared_ptr<Transaction> txn) const 
{
	//cout << "In getobjIDlistBysubID " << _subid << endl;
	if (!Util::is_entity_ele(_subid)) {
		_objidlist = NULL;
		_list_len = 0;
		return false;
	}
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	if(txn == nullptr)
	{
		bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);
		if (!_get) 
		{

			_objidlist = NULL;
			_list_len = 0;
			return false;
		}

		_list_len = _tmp[0];
		_objidlist = new unsigned[_list_len];
		memcpy(_objidlist, _tmp + 3 + 2 * _tmp[1], sizeof(unsigned) * _list_len);
		Util::sort(_objidlist, _list_len);
		if (_no_duplicate) {
			_list_len = Util::removeDuplicate(_objidlist, _list_len);
		}
	
		//if this is a long list, then we should remove itself after copying
		//otherwise, we should not free the list memory
	//		if(VList::listNeedDelete(_len))
	//		{
			delete[] _tmp;
			//_tmp = NULL;
	//		}
	
		return true;
	}
	else
	{
		bool FirstRead = false , latched = false;
		bool IS_SI = txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE;
		//check if get shared-lock
		if(IS_SI)
			FirstRead = !txn->ReadSetFind(_subid, Transaction::IDType::SUBJECT) && !txn->WriteSetFind(_subid, Transaction::IDType::SUBJECT);
		
		VDataSet addset, delset;
		bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len, addset, delset, txn, latched, FirstRead);

		if(txn->GetState() == TransactionState::ABORTED)
			txn->SetErrorType(TransactionErrorType::SUB_S);

		if (!_get) 
		{
			_objidlist = NULL;
			_list_len = 0;
			return false;
		}

		if(IS_SI && latched && FirstRead)
			txn->ReadSetInsert(_subid, Transaction::IDType::SUBJECT);

		vector<unsigned> add_pidoidlist, del_pidoidlist;
		for(auto po: addset)
		{
			add_pidoidlist.push_back(po.first);
			add_pidoidlist.push_back(po.second);
		}

		for(auto po: delset)
		{
			del_pidoidlist.push_back(po.first);
			del_pidoidlist.push_back(po.second);
		}

		unsigned * _values = nullptr;
		unsigned long _values_len;
		//cout << ".........._tmp:                                                 " <<  _tmp << endl;
		if(add_pidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_s2values(add_pidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cout << ".........._tmp:                                                 " <<  _tmp << endl;
		if(del_pidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_s2values(del_pidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cout << ".........._tmp:                                                 " <<  _tmp << endl;
		if(_len == 0) {
			_objidlist = NULL;
			_list_len = 0;
			return false;
		}
		_list_len = _tmp[0];
		_objidlist = new unsigned[_list_len];
		memcpy(_objidlist, _tmp + 3 + 2 * _tmp[1], sizeof(unsigned) * _list_len);
		Util::sort(_objidlist, _list_len);
		if (_no_duplicate) {
			_list_len = Util::removeDuplicate(_objidlist, _list_len);
		}
	
		delete[] _tmp;
		return true;
	}
}

bool 
KVstore::getobjIDlistBysubIDpreID(TYPE_ENTITY_LITERAL_ID _subid, TYPE_PREDICATE_ID _preid, unsigned*& _objidlist, unsigned& _list_len, bool _no_duplicate, shared_ptr<Transaction> txn) const 
{
//	cout << "In getobjIDlistBysubIDpreID " << _subid << ' ' << _preid << endl;
	if (!Util::is_entity_ele(_subid)) {
		_objidlist = NULL;
		_list_len = 0;
		return false;
	}
	unsigned* _tmp = NULL;

	unsigned long _len = 0;
	if(txn == nullptr)
	{
		bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);
		if (!_get) {
			_objidlist = NULL;
			_list_len = 0;
			return false;
		}
		//cout<<"check "<<_subid<<" "<<_preid<<endl;
		unsigned _result = KVstore::binarySearch(_preid, _tmp + 3, _tmp[1], 2);
		//if (_result == -1) 
		if (_result == INVALID) 
		{
			_objidlist = NULL;
			_list_len = 0;
			return false;
		}
		unsigned _offset = _tmp[4 + 2 * _result];
		unsigned _offset_next;
		if (_result == _tmp[1] - 1) {
			_offset_next = 3 + 2 * _tmp[1] + _tmp[0];
		}
		else {
			_offset_next = _tmp[6 + 2 * _result];
		}
		_list_len = _offset_next - _offset;
		_objidlist = new unsigned[_list_len];
		memcpy(_objidlist, _tmp + _offset, sizeof(unsigned) * _list_len);

		//if this is a long list, then we should remove itself after copying
		//otherwise, we should not free the list memory
	//	if(VList::listNeedDelete(_len))
	//	{
			delete[] _tmp;
			//_tmp = NULL;
	//	}

		return true;
	}
	else
	{
		bool FirstRead = false, latched = false;
		bool IS_SI = txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE;
		//check if get shared-lock
		if(IS_SI)
			FirstRead = !txn->ReadSetFind(_subid, Transaction::IDType::SUBJECT);
		
		VDataSet addset, delset;
		bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len, addset, delset, txn, latched, FirstRead);
		
		if(txn->GetState() == TransactionState::ABORTED)
			txn->SetErrorType(TransactionErrorType::SUB_S);

		if (!_get) 
		{
			_objidlist = NULL;
			_list_len = 0;
			return false;
		}

		if(IS_SI && latched && FirstRead)
			txn->ReadSetInsert(_subid, Transaction::IDType::SUBJECT);
		vector<unsigned> add_pidoidlist, del_pidoidlist;
		for(auto po: addset)
		{
			add_pidoidlist.push_back(po.first);
			add_pidoidlist.push_back(po.second);
		}

		for(auto po: delset)
		{
			del_pidoidlist.push_back(po.first);
			del_pidoidlist.push_back(po.second);
		}

		unsigned * _values = nullptr;
		unsigned long _values_len;
		//cerr << ".........._len:                                                 " <<  _len << endl;
		if(add_pidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_s2values(add_pidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cerr << ".........._len:                                                 " <<  _len << endl;
		if(del_pidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_s2values(del_pidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cerr << ".........._len:                                                 " <<  _len << endl;
		// for(int i = 0; i < _len / sizeof(unsigned); i++) cerr << _tmp[i] << " ";
		// cerr << endl;
		if(_len == 0) {
			_objidlist = NULL;
			_list_len = 0;
			return false;
		}
		unsigned _result = KVstore::binarySearch(_preid, _tmp + 3, _tmp[1], 2);
		//if (_result == -1) 
		if (_result == INVALID) 
		{
			_objidlist = NULL;
			_list_len = 0;
			//delete[] _tmp;
			return false;
		}
		unsigned _offset = _tmp[4 + 2 * _result];
		unsigned _offset_next;
		if (_result == _tmp[1] - 1) {
			_offset_next = 3 + 2 * _tmp[1] + _tmp[0];
		}
		else {
			_offset_next = _tmp[6 + 2 * _result];
		}
		_list_len = _offset_next - _offset;
		_objidlist = new unsigned[_list_len];
		memcpy(_objidlist, _tmp + _offset, sizeof(unsigned) * _list_len);

		delete[] _tmp;
		return true;
	}
}

/**
 * @param _subid
 * @param _preid_objidlist [p0 o0, p1 o1, p2 o2,...,pn on]
 * @param _list_len n
 * @param _no_duplicate reserve the repeated elements
 * @param txn
 * @return
 */
bool 
KVstore::getpreIDobjIDlistBysubID(TYPE_ENTITY_LITERAL_ID _subid, unsigned*& _preid_objidlist, unsigned& _list_len, bool _no_duplicate, shared_ptr<Transaction> txn) const 
{
	//cout << "In getpreIDobjIDlistBysubID " << _subid << endl;
	if (!Util::is_entity_ele(_subid)) 
	{
		_preid_objidlist = NULL;
		_list_len = 0;
		return false;
	}
	unsigned* _tmp = NULL;

	unsigned long _len = 0;
	if(txn == nullptr)
	{
		bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);
		if (!_get) {
			_preid_objidlist = NULL;
			_list_len = 0;
			return false;
		}

		_list_len = 2 * _tmp[0];
		_preid_objidlist = new unsigned[_list_len];
		unsigned _offset_next;
		unsigned j = 0;
		for (unsigned i = 0; i < _tmp[1]; i++) {
			if (i == _tmp[1] - 1) {
				_offset_next = 3 + 2 * _tmp[1] + _tmp[0];
			}
			else {
				_offset_next = _tmp[6 + 2 * i];
			}
			for (; 3 + 2 * _tmp[1] + j < _offset_next; j++) {
				_preid_objidlist[2 * j] = _tmp[3 + 2 * i];
				_preid_objidlist[2 * j + 1] = _tmp[3 + 2 * _tmp[1] + j];
			}
		}

		//if this is a long list, then we should remove itself after copying
		//otherwise, we should not free the list memory
	//	if(VList::listNeedDelete(_len))
	//	{
			delete[] _tmp;
			//_tmp = NULL;
	//	}

		return true;
	}
	else
	{
		bool FirstRead = false, latched = false;
		//check if get shared-lock
		bool IS_SI = txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE;
		if(IS_SI)
			FirstRead = !txn->ReadSetFind(_subid, Transaction::IDType::SUBJECT) && !txn->WriteSetFind(_subid, Transaction::IDType::SUBJECT);
		
		VDataSet addset, delset;

		bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len, addset, delset, txn, latched, FirstRead);
		
		if(txn->GetState() == TransactionState::ABORTED)
			txn->SetErrorType(TransactionErrorType::SUB_S);

		if (!_get) {
			_preid_objidlist = NULL;
			_list_len = 0;
			return false;
		}
		
		if(IS_SI && latched && FirstRead)
			txn->ReadSetInsert(_subid, Transaction::IDType::SUBJECT);

		vector<unsigned> add_pidoidlist, del_pidoidlist;
		for(auto po: addset)
		{
			add_pidoidlist.push_back(po.first);
			add_pidoidlist.push_back(po.second);
		}

		for(auto po: delset)
		{
			del_pidoidlist.push_back(po.first);
			del_pidoidlist.push_back(po.second);
		}

		unsigned * _values = nullptr;
		unsigned long _values_len;
		if(add_pidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_s2values(add_pidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cout << ".........._tmp:                                                 " <<  _tmp << endl;
		if(del_pidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_s2values(del_pidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cout << ".........._tmp:                                                 " <<  _tmp << endl;
		if(_len == 0) {
			_preid_objidlist = NULL;
			_list_len = 0;
			return false;
		}
		else
		{
			_list_len = 2 * _tmp[0];
			_preid_objidlist = new unsigned[_list_len];
			unsigned _offset_next;
			unsigned j = 0;
			for (unsigned i = 0; i < _tmp[1]; i++) {
				if (i == _tmp[1] - 1) {
					_offset_next = 3 + 2 * _tmp[1] + _tmp[0];
				}
				else {
					_offset_next = _tmp[6 + 2 * i];
				}
				for (; 3 + 2 * _tmp[1] + j < _offset_next; j++) {
					_preid_objidlist[2 * j] = _tmp[3 + 2 * i];
					_preid_objidlist[2 * j + 1] = _tmp[3 + 2 * _tmp[1] + j];
				}
			}
		}
		delete[] _tmp;
		return true;
	}
}

bool 
KVstore::open_objID2values(int _mode, TYPE_ENTITY_LITERAL_ID _entity_num, TYPE_ENTITY_LITERAL_ID _literal_num) 
{
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_oID2values_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) 
	{
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_oID2values_query;
	}
	else 
	{
		cerr << "Invalid open mode in open_objID2values, mode = " << _mode << endl;
		return false;
	}

	return this->open(this->objID2values, KVstore::s_oID2values, _mode, buffer_size, _entity_num) && 
		   this->open(this->objID2values_literal, KVstore::s_oID2values + "_literal", _mode, buffer_size, _literal_num);
}

bool 
KVstore::close_objID2values() 
{
	if (this->objID2values == NULL && this->objID2values_literal == NULL) 
	{
		return true;
	}

	if (this->objID2values != NULL)
	{
		this->objID2values->save();
		delete this->objID2values;
		this->objID2values = NULL;
	}
	if (this->objID2values_literal != NULL)
	{
		this->objID2values_literal->save();
		delete this->objID2values_literal;
		this->objID2values_literal = NULL;
	}

	return true;
}

//NOTICE: do not need entity border here, because no literal in o2pslist
//STRUCT of o2xx: triple_number pre_num p1 offset1 p2 offset2 ... pn offsetn
//p1-list(in offset1) p2-list(in offset2) ... pn-list(in offsetn)
//(the final whole list is a unsorted slist)
bool 
KVstore::build_objID2values(ID_TUPLE* _p_id_tuples, TYPE_TRIPLE_NUM _triples_num, TYPE_ENTITY_LITERAL_ID total_entity_num, TYPE_ENTITY_LITERAL_ID total_literal_num) 
{
	cout << "Begin building objID2values..." << endl;
	//qsort(_p_id_tuples, _triples_num, sizeof(int*), Util::_ops_cmp);
	vector<unsigned> _sidlist_o;
	vector<unsigned> _pidoffsetlist_o;

	//true means the next obj is a different one from the current one
	bool _obj_change = true;
	//true means the next <obj, pre> is different from the current pair
	bool _obj_pre_change = true;
	//true means the next pre is different from the current one
	bool _pre_change = true;

	this->open_objID2values(KVstore::CREATE_MODE, total_entity_num, total_literal_num);

	//for (unsigned long i = 0; i < _triples_num; i++) 
	for (TYPE_TRIPLE_NUM i = 0; i < _triples_num; i++) 
	{
		if (i + 1 == _triples_num || _p_id_tuples[i].subid != _p_id_tuples[i+1].subid
			|| _p_id_tuples[i].preid != _p_id_tuples[i+1].preid || _p_id_tuples[i].objid != _p_id_tuples[i+1].objid) {
			if (_obj_change) {
				_pidoffsetlist_o.clear();
				_sidlist_o.clear();
			}

			TYPE_ENTITY_LITERAL_ID _sub_id = _p_id_tuples[i].subid;
			TYPE_PREDICATE_ID _pre_id = _p_id_tuples[i].preid;
			TYPE_ENTITY_LITERAL_ID _obj_id = _p_id_tuples[i].objid;

			if (_obj_pre_change) {
				_pidoffsetlist_o.push_back(_pre_id);
				_pidoffsetlist_o.push_back(_sidlist_o.size());
			}

			_sidlist_o.push_back(_sub_id);

			_obj_change = (i + 1 == _triples_num) || (_p_id_tuples[i].objid != _p_id_tuples[i+1].objid);
			_pre_change = (i + 1 == _triples_num) || (_p_id_tuples[i].preid != _p_id_tuples[i+1].preid);
			_obj_pre_change = _obj_change || _pre_change;

			if (_obj_change) {
				for (unsigned j = 1; j < _pidoffsetlist_o.size(); j += 2) {
					_pidoffsetlist_o[j] += 2 + _pidoffsetlist_o.size();
				}
				unsigned* _entrylist_o = new unsigned[2 + _pidoffsetlist_o.size() + _sidlist_o.size()];
				//triples number
				_entrylist_o[0] = _sidlist_o.size();
				//pre number
				_entrylist_o[1] = _pidoffsetlist_o.size() / 2;
				unsigned j, k;
				//pidoffsetlist
				for (j = 2, k = 0; k < _pidoffsetlist_o.size(); j++, k++) {
					_entrylist_o[j] = _pidoffsetlist_o[k];
				}
				//unsorted sidlist
				for (k = 0; k < _sidlist_o.size(); j++, k++) {
					_entrylist_o[j] = _sidlist_o[k];
				}
				
				this->addValueByKey(this->objID2values, _obj_id, (char*)_entrylist_o, sizeof(unsigned) * j);
				
				//delete[] _entrylist_o;
			}
		}
	}
	this->close_objID2values();
	cout << "Finished building objID2values" << endl;
	return true;
}

bool 
KVstore::getpreIDlistByobjID(TYPE_ENTITY_LITERAL_ID _objid, unsigned*& _preidlist, unsigned& _list_len, bool _no_duplicate, shared_ptr<Transaction> txn) const 
{
	//cerr << "InIn getpreIDlistByobjID " << _objid << endl;
	if(_objid == INVALID_ENTITY_LITERAL_ID) {
		_preidlist = nullptr;
		_list_len = 0;
		return false;
	}
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	if(txn == nullptr)
	{
		//cerr << "In getpreIDlistByobjID no transaction! " << endl;
		bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len);
		if (!_get) {
			
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}
		_list_len = _tmp[1];
		_preidlist = new unsigned[_list_len];
		for (unsigned i = 0; i < _list_len; i++) {
			_preidlist[i] = _tmp[2 * i + 2];
		}

		//if this is a long list, then we should remove itself after copying
		//otherwise, we should not free the list memory
	//	if(VList::listNeedDelete(_len))
	//	{
			delete[] _tmp;
			//_tmp = NULL;
	//	}

		return true;
	}
	else
	{
		bool FirstRead = false , latched = false;
		//check if get shared-lock
		bool IS_SI = txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE;
		if(IS_SI)
			FirstRead = !txn->ReadSetFind(_objid, Transaction::IDType::OBJECT) && !txn->WriteSetFind(_objid, Transaction::IDType::OBJECT);
		
		VDataSet addset, delset;
		
		bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len, addset, delset, txn, latched, FirstRead);

		if(txn->GetState() == TransactionState::ABORTED)
			txn->SetErrorType(TransactionErrorType::OBJ_S);

		if (!_get) {
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}
		
		if(IS_SI && latched && FirstRead)
			txn->ReadSetInsert(_objid, Transaction::IDType::OBJECT);

		vector<unsigned> add_pidsidlist, del_pidsidlist;
		for(auto ps: addset){
			add_pidsidlist.push_back(ps.first);
			add_pidsidlist.push_back(ps.second);
		}

		for(auto ps: delset){
			del_pidsidlist.push_back(ps.first);
			del_pidsidlist.push_back(ps.second);
		}

		unsigned * _values = nullptr;
		unsigned long _values_len;
		if(add_pidsidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_o2values(add_pidsidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cerr << "_len:                                                 " <<  _len << endl;
		if(del_pidsidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_o2values(del_pidsidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}

		if(_len == 0) {
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}
		
		_list_len = _tmp[1];
		_preidlist = new unsigned[_list_len];
		for (unsigned i = 0; i < _list_len; i++) {
			_preidlist[i] = _tmp[2 * i + 2];
		}
		delete[] _tmp;

		return true;
	}
	
}

bool 
KVstore::getsubIDlistByobjID(TYPE_ENTITY_LITERAL_ID _objid, unsigned*& _subidlist, unsigned& _list_len, bool _no_duplicate, shared_ptr<Transaction> txn) const 
{
	//cerr << "InIn getsubIDlistByobjID " << _objid << endl;
	if(_objid == INVALID_ENTITY_LITERAL_ID) {
		_subidlist = nullptr;
		_list_len = 0;
		return false;
	}
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	if(txn == nullptr)
	{
		bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len);
		if (!_get) {
			_subidlist = NULL;
			_list_len = 0;
			return false;
		}

		_list_len = _tmp[0];
		_subidlist = new unsigned[_list_len];
		memcpy(_subidlist, _tmp + 2 + 2 * _tmp[1], sizeof(unsigned) * _list_len);
		Util::sort(_subidlist, _list_len);
		if (_no_duplicate) {
			_list_len = Util::removeDuplicate(_subidlist, _list_len);
		}

		//if this is a long list, then we should remove itself after copying
		//otherwise, we should not free the list memory
	//	if(VList::listNeedDelete(_len))
	//	{
			delete[] _tmp;
			//_tmp = NULL;
	//	}

		return true;
	}
	else
	{
		bool FirstRead = false, latched = false;
		bool IS_SI = txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE;
		//check if get shared-lock
		if(IS_SI)
			FirstRead = !txn->ReadSetFind(_objid, Transaction::IDType::OBJECT) && !txn->WriteSetFind(_objid, Transaction::IDType::OBJECT);
		
		VDataSet addset, delset;
		
		bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len, addset, delset, txn, latched,  FirstRead);
		
		if(txn->GetState() == TransactionState::ABORTED)
			txn->SetErrorType(TransactionErrorType::OBJ_S);

		if (!_get) {
			_subidlist = NULL;
			_list_len = 0;
			return false;
		}

		if(IS_SI && latched && FirstRead)
			txn->ReadSetInsert(_objid, Transaction::IDType::OBJECT);

		vector<unsigned> add_pidsidlist, del_pidsidlist;
		for(auto ps: addset){
			add_pidsidlist.push_back(ps.first);
			add_pidsidlist.push_back(ps.second);
		}

		for(auto ps: delset){
			del_pidsidlist.push_back(ps.first);
			del_pidsidlist.push_back(ps.second);
		}
		unsigned * _values = nullptr;
		unsigned long _values_len;
		if(add_pidsidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_o2values(add_pidsidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cerr << "_len:                                                 " <<  _len << endl;
		if(del_pidsidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_o2values(del_pidsidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}

		if(_len == 0) {
			_subidlist = NULL;
			_list_len = 0;
			return false;
		}
		
		_list_len = _tmp[0];
		_subidlist = new unsigned[_list_len];
		memcpy(_subidlist, _tmp + 2 + 2 * _tmp[1], sizeof(unsigned) * _list_len);
		Util::sort(_subidlist, _list_len);
		if (_no_duplicate) {
			_list_len = Util::removeDuplicate(_subidlist, _list_len);
		}

		delete[] _tmp;
		
		return true;
	}
}

bool 
KVstore::getsubIDlistByobjIDpreID(TYPE_ENTITY_LITERAL_ID _objid, TYPE_PREDICATE_ID _preid, unsigned*& _subidlist, unsigned& _list_len, bool _no_duplicate, shared_ptr<Transaction> txn) const 
{
	//cerr << "InIn getsubIDlistByobjIDpreID " << _objid << ' ' << _preid << endl;
	if(_objid == INVALID_ENTITY_LITERAL_ID) {
		_subidlist = nullptr;
		_list_len = 0;
		return false;
	}
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	if(txn == nullptr)
	{
		bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len);
		if (!_get) {
			_subidlist = NULL;
			_list_len = 0;
			return false;
		}
		unsigned _result = KVstore::binarySearch(_preid, _tmp + 2, _tmp[1], 2);
		//if (_result == -1) 
		if (_result == INVALID) 
		{
			_subidlist = NULL;
			_list_len = 0;
			delete[] _tmp;
			return false;
		}
		unsigned _offset = _tmp[3 + 2 * _result];
		unsigned _offset_next;

		if (_result == _tmp[1] - 1) {
			_offset_next = 2 + 2 * _tmp[1] + _tmp[0];
		}
		else {
			_offset_next = _tmp[5 + 2 * _result];
		}
		_list_len = _offset_next - _offset;
		_subidlist = new unsigned[_list_len];
		memcpy(_subidlist, _tmp + _offset, sizeof(unsigned) * _list_len);

		//if this is a long list, then we should remove itself after copying
		//otherwise, we should not free the list memory
		delete[] _tmp;
			//_tmp = NULL;
		return true;
	}
	else
	{
		bool FirstRead = false, latched = false;
		//check if get shared-lock
		bool IS_SI = txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE;
		if(IS_SI)
			FirstRead = !txn->ReadSetFind(_objid, Transaction::IDType::OBJECT);
		
		VDataSet addset, delset;
		
		bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len, addset, delset, txn, latched, FirstRead);

		if(txn->GetState() == TransactionState::ABORTED)
			txn->SetErrorType(TransactionErrorType::OBJ_S);

		if (!_get) {
			_subidlist = NULL;
			_list_len = 0;
			return false;
		}

		if(IS_SI && latched && FirstRead)
			txn->ReadSetInsert(_objid, Transaction::IDType::OBJECT);

		unsigned tmp_len = _len;
		// if(addset.size() != 0 || delset.size() != 0){
		// 	cerr << addset.size() << "   " << delset.size() << endl;
		// }
		vector<unsigned> add_pidsidlist, del_pidsidlist;
		for(auto ps: addset){
			add_pidsidlist.push_back(ps.first);
			add_pidsidlist.push_back(ps.second);
		}

		for(auto ps: delset){
			del_pidsidlist.push_back(ps.first);
			del_pidsidlist.push_back(ps.second);
		}
		unsigned * _values = nullptr;
		unsigned long _values_len;
		if(add_pidsidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_o2values(add_pidsidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cerr << "_len:                                                 " <<  _len << endl;
		if(del_pidsidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_o2values(del_pidsidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}

		if(_len == 0) {
			_subidlist = NULL;
			_list_len = 0;
			return false;
		}
		if(_tmp[1] == 0 || _tmp[1] == 32767){
			cerr << "InIn getsubIDlistByobjIDpreID " << _objid << ' ' << _preid << endl;
			cerr << " _tmp[0] :" << _tmp[0] << endl;
			cerr << " _tmp[1] :" << _tmp[1] << endl;
			cerr << "_len:" << _len << endl;
			cerr << "old_len:  " << tmp_len << endl;
			cerr << "add_pidsidlist.size():  " << add_pidsidlist.size() << " " << "del_pidsidlist.size(): " << del_pidsidlist.size() << endl;

			// for(int i = 0; i < _len / sizeof(unsigned); i++){
			// 	cerr << _tmp[i] << " ";
			// }
			// cerr << endl;
		}
			
		unsigned _result = KVstore::binarySearch(_preid, _tmp + 2, _tmp[1], 2);
		//if (_result == -1) 
		if (_result == INVALID) 
		{
			_subidlist = NULL;
			_list_len = 0;
			delete[] _tmp;
			return false;
		}
		unsigned _offset = _tmp[3 + 2 * _result];
		unsigned _offset_next;

		if (_result == _tmp[1] - 1) {
			_offset_next = 2 + 2 * _tmp[1] + _tmp[0];
		}
		else {
			_offset_next = _tmp[5 + 2 * _result];
		}
		_list_len = _offset_next - _offset;
		_subidlist = new unsigned[_list_len];
		memcpy(_subidlist, _tmp + _offset, sizeof(unsigned) * _list_len);
		delete[] _tmp;
		return true;
	}
}

bool 
KVstore::getpreIDsubIDlistByobjID(TYPE_ENTITY_LITERAL_ID _objid, unsigned*& _preid_subidlist, unsigned& _list_len, bool _no_duplicate, shared_ptr<Transaction> txn) const 
{
	//cerr << "InIn getpreIDsubIDlistByobjID " << _objid << endl;
	if(_objid == INVALID_ENTITY_LITERAL_ID) {
		_preid_subidlist = nullptr;
		_list_len = 0;
		return false;
	}
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	if(txn == nullptr)
	{
		bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len);
		if (!_get) {
			_preid_subidlist = NULL;
			_list_len = 0;
			return false;
		}

		_list_len = 2 * _tmp[0];
		_preid_subidlist = new unsigned[_list_len];
		unsigned _offset_next;
		unsigned j = 0;
		for (unsigned i = 0; i < _tmp[1]; i++) {
			if (i == _tmp[1] - 1) {
				_offset_next = 2 + 2 * _tmp[1] + _tmp[0];
			}
			else {
				_offset_next = _tmp[5 + 2 * i];
			}
			for (; 2 + 2 * _tmp[1] + j < _offset_next; j++) {
				_preid_subidlist[2 * j] = _tmp[2 + 2 * i];
				_preid_subidlist[2 * j + 1] = _tmp[2 + 2 * _tmp[1] + j];
			}
		}

		//if this is a long list, then we should remove itself after copying
		//otherwise, we should not free the list memory
	//	if(VList::listNeedDelete(_len))
	//	{
			delete[] _tmp;
			//_tmp = NULL;
	//	}

		return true;
	}
	else
	{
		bool FirstRead = false, latched = false;
		bool IS_SI = txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE;
		//check if get shared-lock
		if(IS_SI)
			FirstRead = !txn->ReadSetFind(_objid, Transaction::IDType::OBJECT) && !txn->WriteSetFind(_objid, Transaction::IDType::OBJECT);
		
		VDataSet addset, delset;
		
		bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len, addset, delset, txn, latched, FirstRead);
		
		if(txn->GetState() == TransactionState::ABORTED)
			txn->SetErrorType(TransactionErrorType::OBJ_S);

		if (!_get) {
			_preid_subidlist = NULL;
			_list_len = 0;
			return false;
		}

		if(IS_SI && latched && FirstRead)
			txn->ReadSetInsert(_objid, Transaction::IDType::OBJECT);

		vector<unsigned> add_pidsidlist, del_pidsidlist;
		for(auto ps: addset){
			add_pidsidlist.push_back(ps.first);
			add_pidsidlist.push_back(ps.second);
		}

		for(auto ps: delset){
			del_pidsidlist.push_back(ps.first);
			del_pidsidlist.push_back(ps.second);
		}
		unsigned * _values = nullptr;
		unsigned long _values_len;
		if(add_pidsidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_o2values(add_pidsidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cerr << "_len:                                                 " <<  _len << endl;
		if(del_pidsidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_o2values(del_pidsidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}

		if(_len == 0) {
			_preid_subidlist = NULL;
			_list_len = 0;
			return false;
		}
		_list_len = 2 * _tmp[0];
		_preid_subidlist = new unsigned[_list_len];
		unsigned _offset_next;
		unsigned j = 0;
		for (unsigned i = 0; i < _tmp[1]; i++) {
			if (i == _tmp[1] - 1) {
				_offset_next = 2 + 2 * _tmp[1] + _tmp[0];
			}
			else {
				_offset_next = _tmp[5 + 2 * i];
			}
			for (; 2 + 2 * _tmp[1] + j < _offset_next; j++) {
				_preid_subidlist[2 * j] = _tmp[2 + 2 * i];
				_preid_subidlist[2 * j + 1] = _tmp[2 + 2 * _tmp[1] + j];
			}
		}
		
		delete[] _tmp;
		return true;
	}
}
bool 
KVstore::open_preID2values(int _mode, TYPE_PREDICATE_ID _pre_num) 
{
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_pID2values_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_pID2values_query;
	}
	else {
		cerr << "Invalid open mode in open_preID2values, mode = " << _mode << endl;
		return false;
	}
	return this->open(this->preID2values, KVstore::s_pID2values, _mode, buffer_size, _pre_num);
}

bool 
KVstore::close_preID2values() 
{
	if (this->preID2values == NULL) {
		return true;
	}

	this->preID2values->save();
	delete this->preID2values;
	this->preID2values = NULL;

	return true;
}

bool 
KVstore::build_preID2values(ID_TUPLE* _p_id_tuples, TYPE_TRIPLE_NUM _triples_num, TYPE_PREDICATE_ID total_pre_num) 
//NOTICE: if we sort sidlist, then oidlist is not sorted; otherwise if we sort oidlist, then sidlist is not sorted
//STRUCT of p2xx: triple_number sidlist oidlist(not sorted, linked with sidlist one by one)
{
	cout << "Begin building preID2values..." << endl;
	//qsort(_p_id_tuples, _triples_num, sizeof(int*), Util::_pso_cmp);
	vector<unsigned> _sidlist_p;
	vector<unsigned> _oidlist_p;

	//true means the next pre is different from the current one
	bool _pre_change = true;

	this->open_preID2values(KVstore::CREATE_MODE, total_pre_num);

	//for (unsigned long i = 0; i < _triples_num; i++) 
	for (TYPE_TRIPLE_NUM i = 0; i < _triples_num; i++) 
	{
		if (i + 1 == _triples_num || _p_id_tuples[i].subid != _p_id_tuples[i+1].subid
			|| _p_id_tuples[i].preid != _p_id_tuples[i+1].preid || _p_id_tuples[i].objid != _p_id_tuples[i+1].objid) {
			if (_pre_change) {
				_sidlist_p.clear();
				_oidlist_p.clear();
			}

			TYPE_ENTITY_LITERAL_ID _sub_id = _p_id_tuples[i].subid;
			TYPE_PREDICATE_ID _pre_id = _p_id_tuples[i].preid;
			TYPE_ENTITY_LITERAL_ID _obj_id = _p_id_tuples[i].objid;

			_sidlist_p.push_back(_sub_id);
			_oidlist_p.push_back(_obj_id);

			_pre_change = (i + 1 == _triples_num) || (_p_id_tuples[i].preid != _p_id_tuples[i+1].preid);

			if (_pre_change) {
				unsigned* _entrylist_p = new unsigned[1 + _sidlist_p.size() * 2];
				//triples number
				_entrylist_p[0] = _sidlist_p.size();
				unsigned j, k;
				//sidlist
				for (j = 1, k = 0; k < _sidlist_p.size(); j++, k++) {
					_entrylist_p[j] = _sidlist_p[k];
				}
				//unsorted oidlist
				for (k = 0; k < _oidlist_p.size(); j++, k++) {
					_entrylist_p[j] = _oidlist_p[k];
				}
				this->addValueByKey(this->preID2values, _pre_id, (char*)_entrylist_p, sizeof(unsigned) * j);
				//delete[] _entrylist_p;
			}
		}
	}

	this->close_preID2values();
	cout << "Finished building preID2values" << endl;
	return true;
}

bool 
KVstore::getsubIDlistBypreID(TYPE_PREDICATE_ID _preid, unsigned*& _subidlist, unsigned& _list_len, bool _no_duplicate, shared_ptr<Transaction> txn) const 
{
	//cout << "In getsubIDlistBypreID " << _preid << endl;
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	if(txn == nullptr)
	{
		bool _get = this->getValueByKey(this->preID2values, _preid, (char*&)_tmp, _len);
		if (!_get) {
			_subidlist = NULL;
			_list_len = 0;
			return false;
		}

		_list_len = _tmp[0];
		_subidlist = new unsigned[_list_len];
		memcpy(_subidlist, _tmp + 1, sizeof(unsigned) * _list_len);
		if (_no_duplicate) {
			_list_len = Util::removeDuplicate(_subidlist, _list_len);
		}

		//if this is a long list, then we should remove itself after copying
		//otherwise, we should not free the list memory
	//	if(VList::listNeedDelete(_len))
	//	{
			delete[] _tmp;
			//_tmp = NULL;
	//	}

		return true;
	}
	else
	{
		bool FirstRead = false, latched = false;
		bool IS_SI = txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE;
		//check if get shared-lock
		if(IS_SI)
			FirstRead = !txn->ReadSetFind(_preid, Transaction::IDType::PREDICATE) && !txn->WriteSetFind(_preid, Transaction::IDType::PREDICATE);
		
		VDataSet addset, delset;
		
		bool _get = this->getValueByKey(this->preID2values, _preid, (char*&)_tmp, _len, addset, delset, txn, latched,  FirstRead);

		if(txn->GetState() == TransactionState::ABORTED)
			txn->SetErrorType(TransactionErrorType::PRE_S);

		if (!_get) {
			_subidlist = NULL;
			_list_len = 0;
			return false;
		}
		
		if(IS_SI && FirstRead && FirstRead)
			txn->ReadSetInsert(_preid, Transaction::IDType::PREDICATE);

		vector<unsigned> add_sidoidlist, del_sidoidlist;
		for(auto so: addset)
		{
			add_sidoidlist.push_back(so.first);
			add_sidoidlist.push_back(so.second);
		}

		for(auto so: delset)
		{
			del_sidoidlist.push_back(so.first);
			del_sidoidlist.push_back(so.second);
		}
		
		unsigned * _values = nullptr;
		unsigned long _values_len;
		//cout << "_len:                                                 " <<  _len << endl;
		//cerr << "addset.size():           " << addset.size() << "   delset.size()                      " << delset.size() << endl;
		if(add_sidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_p2values(add_sidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cout << "_len:                                                 " <<  _len << endl;
		if(del_sidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_p2values(del_sidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}

		if(_len == 0){
			_subidlist = NULL;
			_list_len = 0;
			return false;
		}

		_list_len = _tmp[0];
		_subidlist = new unsigned[_list_len];
		memcpy(_subidlist, _tmp + 1, sizeof(unsigned) * _list_len);
		if (_no_duplicate) {
			_list_len = Util::removeDuplicate(_subidlist, _list_len);
		}

		delete[] _tmp;

		return true;
	}
}

bool 
KVstore::getobjIDlistBypreID(TYPE_PREDICATE_ID _preid, unsigned*& _objidlist, unsigned& _list_len, bool _no_duplicate, shared_ptr<Transaction> txn) const {
	//cout << "In getobjIDlistBypreID " << _preid << endl;
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	if(txn == nullptr)
	{
		bool _get = this->getValueByKey(this->preID2values, _preid, (char*&)_tmp, _len);
		if (!_get) {
			_objidlist = NULL;
			_list_len = 0;
			return false;
		}

		_list_len = _tmp[0];
		_objidlist = new unsigned[_list_len];
		memcpy(_objidlist, _tmp + 1 + _tmp[0], sizeof(unsigned) * _list_len);
		Util::sort(_objidlist, _list_len);
		if (_no_duplicate) {
			_list_len = Util::removeDuplicate(_objidlist, _list_len);
		}

		//if this is a long list, then we should remove itself after copying
		//otherwise, we should not free the list memory
	//	if(VList::listNeedDelete(_len))
	//	{
			delete[] _tmp;
			//_tmp = NULL;
	//	}

		return true;
	}
	else
	{
		bool FirstRead = false, latched = false;
		bool IS_SI = txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE;
		//check if get shared-lock
		if(IS_SI)
			FirstRead = !txn->ReadSetFind(_preid, Transaction::IDType::PREDICATE) && !txn->WriteSetFind(_preid, Transaction::IDType::PREDICATE);
		
		VDataSet addset, delset;
		
		bool _get = this->getValueByKey(this->preID2values, _preid, (char*&)_tmp, _len, addset, delset, txn, latched, FirstRead);

		if(txn->GetState() == TransactionState::ABORTED)
			txn->SetErrorType(TransactionErrorType::PRE_S);

		//cout << "getobjIDlistBypreID:.............................." << _get << endl;
		if (!_get) {
			_objidlist = NULL;
			_list_len = 0;
			return false;
		}
		
		if(IS_SI && FirstRead && FirstRead)
			txn->ReadSetInsert(_preid, Transaction::IDType::PREDICATE);

		vector<unsigned> add_sidoidlist, del_sidoidlist;
		for(auto so: addset)
		{
			add_sidoidlist.push_back(so.first);
			add_sidoidlist.push_back(so.second);
		}

		for(auto so: delset)
		{
			del_sidoidlist.push_back(so.first);
			del_sidoidlist.push_back(so.second);
		}
		
		unsigned * _values = nullptr;
		unsigned long _values_len;
		//cout << "_len:                                                 " <<  _len << endl;
		//cerr << "addset.size():           " << addset.size() << "   delset.size()                      " << delset.size() << endl;
		if(add_sidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_p2values(add_sidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cout << "_len:                                                 " <<  _len << endl;
		if(del_sidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_p2values(del_sidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}

		if(_len == 0){
			_objidlist = NULL;
			_list_len = 0;
			return false;
		}

		_list_len = _tmp[0];
		_objidlist = new unsigned[_list_len];
		memcpy(_objidlist, _tmp + 1 + _tmp[0], sizeof(unsigned) * _list_len);
		Util::sort(_objidlist, _list_len);
		if (_no_duplicate) {
			_list_len = Util::removeDuplicate(_objidlist, _list_len);
		}
		
		delete[] _tmp;
		return true;
	}
}

bool 
KVstore::getsubIDobjIDlistBypreID(TYPE_PREDICATE_ID _preid, unsigned*& _subid_objidlist, unsigned& _list_len, bool _no_duplicate, shared_ptr<Transaction> txn) const 
{
#ifdef DEBUG_KVSTORE
	cout << "In getsubIDobjIDlistBypreID " << _preid << endl;
#endif
	unsigned* _tmp = NULL;
	unsigned long _len = 0;
	if(txn == nullptr)
	{
		bool _get = this->getValueByKey(this->preID2values, _preid, (char*&)_tmp, _len);
#ifdef DEBUG_KVSTORE
	cout<<"the length of list: "<<_len<<endl;
#endif
		if (!_get) 
		{
			_subid_objidlist = NULL;
			_list_len = 0;
			return false;
		}
		_list_len = _tmp[0] * 2;
		_subid_objidlist = new unsigned[_list_len];
		for (unsigned i = 0; i < _tmp[0]; i++) 
		{
			_subid_objidlist[2 * i] = _tmp[1 + i];
			_subid_objidlist[2 * i + 1] = _tmp[1 + _tmp[0] + i];
#ifdef DEBUG_KVSTORE
		cout<<_subid_objidlist[2*i]<<" "<<_subid_objidlist[2*i+1]<<endl;
#endif
		}

		//if this is a long list, then we should remove itself after copying
		//otherwise, we should not free the list memory
	//	if(VList::listNeedDelete(_len))
	//	{
#ifdef DEBUG_KVSTORE
		cout<<"this is a vlist"<<endl;
#endif
			delete[] _tmp;
			//_tmp = NULL;
	//	}

		return true;
	}
	else
	{
		bool FirstRead = false, latched = false;
		//check if get shared-lock
		bool IS_SI = txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE;
		if(IS_SI)
			FirstRead = !txn->ReadSetFind(_preid, Transaction::IDType::PREDICATE) && !txn->WriteSetFind(_preid, Transaction::IDType::PREDICATE);
		
		VDataSet addset, delset;
		
		bool _get = this->getValueByKey(this->preID2values, _preid, (char*&)_tmp, _len, addset, delset, txn, latched,  FirstRead);

		if(txn->GetState() == TransactionState::ABORTED)
			txn->SetErrorType(TransactionErrorType::PRE_S);

		if (!_get) {
			_subid_objidlist = NULL;
			_list_len = 0;
			return false;
		}
		
		if(IS_SI && latched && FirstRead)
			txn->ReadSetInsert(_preid, Transaction::IDType::PREDICATE);

		vector<unsigned> add_sidoidlist, del_sidoidlist;
		for(auto so: addset)
		{
			add_sidoidlist.push_back(so.first);
			add_sidoidlist.push_back(so.second);
		}

		for(auto so: delset)
		{
			del_sidoidlist.push_back(so.first);
			del_sidoidlist.push_back(so.second);
		}
		
		unsigned * _values = nullptr;
		unsigned long _values_len;
		//cout << "_len:                                                 " <<  _len << endl;
		//cerr << "addset.size():           " << addset.size() << "   delset.size()                      " << delset.size() << endl;
		if(add_sidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_p2values(add_sidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cout << "_len:                                                 " <<  _len << endl;
		if(del_sidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_p2values(del_sidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//bool ret = true;
		if(_len == 0) {
			_subid_objidlist = NULL;
			_list_len = 0;
			//ret = false;
			return false;
		}
		else
		{
			_list_len = 2 * _tmp[0];
			_subid_objidlist = new unsigned[_list_len];
			for (unsigned i = 0; i < _tmp[0]; i++) 
			{
				_subid_objidlist[2 * i] = _tmp[1 + i];
				_subid_objidlist[2 * i + 1] = _tmp[1 + _tmp[0] + i];
			};
		}
		
		delete[] _tmp;
		return true;
	}
}

bool 
KVstore::getpreIDlistBysubIDobjID(TYPE_ENTITY_LITERAL_ID _subid, TYPE_ENTITY_LITERAL_ID _objid, unsigned*& _preidlist, unsigned& _list_len, bool _no_duplicate, shared_ptr<Transaction> txn) const 
{
	//cout << "In getpreIDlistBysubIDobjID " << _subid << ' ' << _objid << endl;
	unsigned *list1 = NULL, *list2 = NULL;
	unsigned len1 = 0, len2 = 0;
	if(txn == nullptr)
	{
		if (!this->getpreIDlistBysubID(_subid, list1, len1, true)) {
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}

		if (!this->getpreIDlistByobjID(_objid, list2, len2, true)) {
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}
		vector<unsigned> list = KVstore::intersect(list1, list2, len1, len2);
		delete[] list1;
		delete[] list2;
		unsigned len = list.size();
		if (len == 0) {
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}
		unsigned* _tmp = NULL;
		unsigned long _len = 0;
		this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);
		_list_len = len;
		unsigned _result = 0;
		for (unsigned i = 0; i < len; i++) {
			TYPE_PREDICATE_ID _preid = list[i];
			for (; _result < _tmp[1]; _result++) {
				if (_tmp[3 + 2 * _result] == _preid) {
					break;
				}
			}
			if (_result == _tmp[1]) {
				for (unsigned j = i; j < len; j++) {
					//list[j] = -1;
					list[j] = INVALID;
					_list_len--;
				}
				break;
			}
			unsigned _offset = _tmp[4 + 2 * _result];
			unsigned _offset_next;
			if (_result == _tmp[1] - 1) {
				_offset_next = 3 + 2 * _tmp[1] + _tmp[0];
			}
			else {
				_offset_next = _tmp[6 + 2 * _result];
			}
			if (KVstore::binarySearch(_objid, _tmp + _offset, _offset_next - _offset) == INVALID) {
				list[i] = INVALID;
				_list_len--;
			}
		}

		//if this is a long list, then we should remove itself after copying
		//otherwise, we should not free the list memory
	//	if(VList::listNeedDelete(_len))
	//	{
			delete[] _tmp;
			//_tmp = NULL;
	//	}

		if (_list_len == 0) {
			_preidlist = NULL;
			return false;
		}
		_preidlist = new unsigned[_list_len];
		unsigned i = 0, j = 0;
		while (i < len) {
			if (list[i] != INVALID) 
			{
				_preidlist[j] = list[i];
				i++;
				j++;
			}
			else {
				i++;
			}
		}

		return true;
	}
	else
	{
		if (!this->getpreIDlistBysubID(_subid, list1, len1, true, txn)) {
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}

		if (!this->getpreIDlistByobjID(_objid, list2, len2, true, txn)) {
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}
		vector<unsigned> list = KVstore::intersect(list1, list2, len1, len2);
		delete[] list1;
		delete[] list2;
		unsigned len = list.size();
		if (len == 0) {
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}
		
		unsigned* _tmp = NULL;
		unsigned long _len = 0;
		VDataSet addset, delset;
		bool latched = false; 
		bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len, addset, delset, txn, latched,  false);		
		
		vector<unsigned> add_pidoidlist, del_pidoidlist;
		for(auto po: addset)
		{
			add_pidoidlist.push_back(po.first);
			add_pidoidlist.push_back(po.second);
		}

		for(auto po: delset)
		{
			del_pidoidlist.push_back(po.first);
			del_pidoidlist.push_back(po.second);
		}

		unsigned * _values = nullptr;
		unsigned long _values_len;
		if(add_pidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_s2values(add_pidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		if(del_pidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_s2values(del_pidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		if(_len == 0) {
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}
		
		_list_len = len;
		unsigned _result = 0;
		for (unsigned i = 0; i < len; i++) {
			TYPE_PREDICATE_ID _preid = list[i];
			for (; _result < _tmp[1]; _result++) {
				if (_tmp[3 + 2 * _result] == _preid) {
					break;
				}
			}
			if (_result == _tmp[1]) {
				for (unsigned j = i; j < len; j++) {
					//list[j] = -1;
					list[j] = INVALID;
					_list_len--;
				}
				break;
			}
			unsigned _offset = _tmp[4 + 2 * _result];
			unsigned _offset_next;
			if (_result == _tmp[1] - 1) {
				_offset_next = 3 + 2 * _tmp[1] + _tmp[0];
			}
			else {
				_offset_next = _tmp[6 + 2 * _result];
			}
			if (KVstore::binarySearch(_objid, _tmp + _offset, _offset_next - _offset) == INVALID) {
				list[i] = INVALID;
				_list_len--;
			}
		}

		//if this is a long list, then we should remove itself after copying
		//otherwise, we should not free the list memory
	//	if(VList::listNeedDelete(_len))
	//	{
		if(_tmp != nullptr)
			delete[] _tmp;
			//_tmp = NULL;
	//	}

		if (_list_len == 0) {
			_preidlist = NULL;
			return false;
		}
		_preidlist = new unsigned[_list_len];
		unsigned i = 0, j = 0;
		while (i < len) {
			if (list[i] != INVALID) 
			{
				_preidlist[j] = list[i];
				i++;
				j++;
			}
			else {
				i++;
			}
		}
		return true;
	}
		
}


bool 
KVstore::open(SITree*& _p_btree, string _tree_name, int _mode, unsigned long long _buffer_size) 
{
	if (_p_btree != NULL) {
		return false;
	}
	string smode;
	if (_mode == KVstore::CREATE_MODE) {
		smode = "build";
	}
	else if (_mode == KVstore::READ_WRITE_MODE) {
		smode = "open";
	}
	else {
		cerr << "Invalid open mode of: " << _tree_name << " mode = " << _mode << endl;
		return false;
	}
    cout<<"open the "<<_tree_name<<endl;
	_p_btree = new SITree(this->store_path, _tree_name, smode, _buffer_size);
	return true;
}

/*bool 
KVstore::open(ISTree*& _p_btree, string _tree_name, int _mode, unsigned long long _buffer_size) 
{
	if (_p_btree != NULL) {
		return false;
	}
	string smode;
	if (_mode == KVstore::CREATE_MODE) {
		smode = "build";
	}
	else if (_mode == KVstore::READ_WRITE_MODE) {
		smode = "open";
	}
	else {
		cerr << "Invalid open mode of: " << _tree_name << " mode = " << _mode << endl;
		return false;
	}
	_p_btree = new ISTree(this->store_path, _tree_name, smode, _buffer_size);
	return true;
}*/

bool 
KVstore::open(ISArray*& _array, string _name, int _mode, unsigned long long _buffer_size, unsigned _key_num) 
{
	if (_array != NULL) {
		return false;
	}
	string smode;
	if (_mode == KVstore::CREATE_MODE) {
		smode = "build";
	}
	else if (_mode == KVstore::READ_WRITE_MODE) {
		smode = "open";
	}
	else {
		cerr << "Invalid open mode of: " << _name << " mode = " << _mode << endl;
		return false;
	}
	_array = new ISArray(this->store_path, _name, smode, _buffer_size, _key_num);
	return true;
}

/*bool 
KVstore::open(IVTree*& _p_btree, string _tree_name, int _mode, unsigned long long _buffer_size) 
{
	if (_p_btree != NULL) {
		return false;
	}
	string smode;
	if (_mode == KVstore::CREATE_MODE) {
		smode = "build";
	}
	else if (_mode == KVstore::READ_WRITE_MODE) {
		smode = "open";
	}
	else {
		cerr << "Invalid open mode of: " << _tree_name << " mode = " << _mode << endl;
		return false;
	}
	_p_btree = new IVTree(this->store_path, _tree_name, smode, _buffer_size);

	return true;
}*/

bool
KVstore::open(IVArray*& _array, string _name, int _mode, unsigned long long _buffer_size, unsigned _key_num)
{
	if (_array != NULL)
	{
		return false;
	}
	string smode;
	if (_mode == KVstore::CREATE_MODE)
	{
		smode = "build";
	}
	else if (_mode == KVstore::READ_WRITE_MODE)
	{
		smode = "open";
	}
	else
	{
		cerr << "Invalid open mode of: " << _name << " mode = " << _mode << endl;
		return false;
	}
	_array = new IVArray(this->store_path, _name, smode, _buffer_size, _key_num);

	return true;
}

void 
KVstore::flush(SITree* _p_btree) 
{
	if (_p_btree != NULL) 
	{
      _p_btree->Save();
	}
}

/*void 
KVstore::flush(ISTree* _p_btree) 
{
	if (_p_btree != NULL) 
	{
		_p_btree->Save();
	}
}*/


void 
KVstore::flush(ISArray* _array)
{
	if (_array != NULL)
	{
		_array->save();
	}
}


/*void 
KVstore::flush(IVTree* _p_btree) 
{
	if (_p_btree != NULL) 
	{
		_p_btree->Save();
	}
}*/

void
KVstore::flush(IVArray* _array)
{
	if (_array != NULL)
	{
		_array->save();
	}
}

bool 
KVstore::addValueByKey(SITree* _p_btree, char* _key, unsigned _klen, unsigned _val) 
{
	return _p_btree->Insert(_key, _klen, _val);
}

/*bool 
KVstore::addValueByKey(ISTree* _p_btree, unsigned _key, char* _val, unsigned _vlen) 
{
	return _p_btree->insert(_key, _val, _vlen);
}*/

bool
KVstore::addValueByKey(ISArray* _array, unsigned _key, char* _val, unsigned _vlen)
{
	if (_array == this->id2literal)
	{
		unsigned key = _key - Util::LITERAL_FIRST_ID;
		return _array->insert(key, _val, _vlen);
	}
	return _array->insert(_key, _val, _vlen);
}

/*bool 
KVstore::addValueByKey(IVTree* _p_btree, unsigned _key, char* _val, unsigned _vlen) 
{
	return _p_btree->insert(_key, _val, _vlen);
}*/

bool
KVstore::addValueByKey(IVArray *_array, unsigned _key, char* _val, unsigned long _vlen)
{
	if (Util::is_literal_ele(_key) && _array == objID2values)
	{
		unsigned key = _key - Util::LITERAL_FIRST_ID;
		return objID2values_literal->insert(key, _val, _vlen);
	}
	return _array->insert(_key, _val, _vlen);
}

bool 
KVstore::setValueByKey(SITree* _p_btree, char* _key, unsigned _klen, unsigned _val) 
{
	return _p_btree->Modify(_key, _klen, _val);
}

/*bool 
KVstore::setValueByKey(ISTree* _p_btree, unsigned _key, char* _val, unsigned _vlen) 
{
	return _p_btree->modify(_key, _val, _vlen);
}*/

bool
KVstore::setValueByKey(ISArray* _array, unsigned _key, char* _val, unsigned _vlen)
{
	if (_array == this->id2literal)
	{
		unsigned key = _key - Util::LITERAL_FIRST_ID;
		return _array->modify(key, _val, _vlen);
	}
	return _array->modify(_key, _val, _vlen);
}

/*bool
KVstore::setValueByKey(IVTree* _p_btree, unsigned _key, char* _val, unsigned _vlen) 
{
	return _p_btree->modify(_key, _val, _vlen);
}*/

bool
KVstore::setValueByKey(IVArray* _array, unsigned _key, char* _val, unsigned long _vlen)
{
	if (Util::is_literal_ele(_key) && _array == objID2values)
	{
		unsigned key = _key - Util::LITERAL_FIRST_ID;
		return objID2values_literal->modify(key, _val, _vlen);
	}
	return _array->modify(_key, _val, _vlen);
}

bool 
KVstore::getValueByKey(SITree* _p_btree, const char* _key, unsigned _klen, unsigned* _val) const 
{
	return _p_btree->Search(_key, _klen, _val);
}

/*bool 
KVstore::getValueByKey(ISTree* _p_btree, unsigned _key, char*& _val, unsigned& _vlen) const 
{
	return _p_btree->Search(_key, _val, _vlen);
}*/

bool
KVstore::getValueByKey(ISArray* _array, unsigned _key, char*& _val, unsigned& _vlen) const
{
	if (_array == this->id2literal)
	{
		unsigned key = _key - Util::LITERAL_FIRST_ID;
		return _array->search(key, _val, _vlen);
	}

	return _array->search(_key, _val, _vlen);
}

/*bool 
KVstore::getValueByKey(IVTree* _p_btree, unsigned _key, char*& _val, unsigned& _vlen) const 
{
	return _p_btree->Search(_key, _val, _vlen);
}*/

bool
KVstore::getValueByKey(IVArray* _array, unsigned _key, char*& _val, unsigned long & _vlen) const
{
	if (Util::is_literal_ele(_key) && _array == objID2values)
	{
		unsigned key = _key - Util::LITERAL_FIRST_ID;
		return objID2values_literal->search(key, _val, _vlen);
	}
	return _array->search(_key, _val, _vlen);
}

TYPE_ENTITY_LITERAL_ID
KVstore::getIDByStr(SITree* _p_btree, const char* _key, unsigned _klen) const 
{
	unsigned val = 0;
	bool ret = _p_btree->Search(_key, _klen, &val);
	if (!ret)
	{
		//return -1;
		return INVALID;
	}

	return val;
}

bool 
KVstore::removeKey(SITree* _p_btree, const char* _key, unsigned _klen)
{
	return _p_btree->Remove(_key, _klen);
}

/*
bool 
KVstore::removeKey(ISTree* _p_btree, unsigned _key) 
{
	return _p_btree->remove(_key);
}*/

bool
KVstore::removeKey(ISArray* _array, unsigned _key)
{
	if (_array == this->id2literal)
	{
		unsigned key = _key - Util::LITERAL_FIRST_ID;
		return _array->remove(key);
	}
	return _array->remove(_key);
}

/*bool 
KVstore::removeKey(IVTree* _p_btree, unsigned _key) 
{
	return _p_btree->remove(_key);
}*/

bool
KVstore::removeKey(IVArray* _array, unsigned _key)
{
	if (Util::is_literal_ele(_key) && _array == objID2values)
	{
		unsigned key = _key - Util::LITERAL_FIRST_ID;
		return objID2values_literal->remove(key);
	}

	return _array->remove(_key);
}

vector<unsigned> 
KVstore::intersect(const unsigned* _list1, const unsigned* _list2, unsigned _len1, unsigned _len2) 
{
	unsigned i = 0, j = 0;
	vector<unsigned> ret;
	while (i < _len1 && j < _len2) {
		if (_list1[i] < _list2[j]) {
			i++;
		}
		else if (_list1[i] > _list2[j]) {
			j++;
		}
		else {
			ret.push_back(_list1[i]);
			i++;
			j++;
		}
	}
	return ret;
}

unsigned
KVstore::binarySearch(unsigned _key, const unsigned* _list, unsigned _list_len, int _step) 
{
	long long _left = 0;
	long long _right = _list_len - 1;
	long long _mid;
	while (_left <= _right) {
		_mid = (_right - _left) / 2 + _left;
		//cout<<"check "<<_step<<" "<<_mid<<" "<<_step*_mid<<" "<<_list_len<<endl;
		if (_key == _list[_step * _mid]) {
			return _mid;
		}
		if (_key < _list[_step * _mid]) {
			_right = _mid - 1;
		}
		else {
			_left = _mid + 1;
		}
	}

	//return -1;
	return INVALID;
}

bool 
KVstore::isEntity(TYPE_ENTITY_LITERAL_ID id) 
{
	return id < Util::LITERAL_FIRST_ID;
}

void
KVstore::AddIntoPreCache(TYPE_PREDICATE_ID _pre_id)
{
	this->preID2values->PinCache(_pre_id);
}
void 
KVstore::AddIntoSubCache(TYPE_ENTITY_LITERAL_ID _entity_id)
{
	this->subID2values->PinCache(_entity_id);
}

void 
KVstore::AddIntoObjCache(TYPE_ENTITY_LITERAL_ID _entity_literal_id)
{
	if (Util::is_literal_ele(_entity_literal_id))
	{
		TYPE_ENTITY_LITERAL_ID _literal_id = _entity_literal_id
						- Util::LITERAL_FIRST_ID;

		objID2values_literal->PinCache(_literal_id);
	}
	this->objID2values->PinCache(_entity_literal_id);
}

unsigned long
KVstore::getSubListSize(TYPE_ENTITY_LITERAL_ID _sub_id)
{
	unsigned* _tmp = NULL;
	unsigned long _ret;
	this->getValueByKey(this->subID2values, _sub_id, (char*&) _tmp, _ret);
	delete [] _tmp;
	return _ret;
}

unsigned long
KVstore::getObjListSize(TYPE_ENTITY_LITERAL_ID _obj_id)
{
	unsigned* _tmp = NULL;
	unsigned long _ret;
	this->getValueByKey(this->objID2values, _obj_id, (char*&) _tmp, _ret);
	delete [] _tmp;
	return _ret;
}

unsigned long
KVstore::getPreListSize(TYPE_PREDICATE_ID _pre_id)
{
	unsigned* _tmp = NULL;
	unsigned long _ret;
	this->getValueByKey(this->preID2values, _pre_id, (char*&) _tmp, _ret);
	delete [] _tmp;
	return _ret;
}

// check by sp2o, maybe there is other better method.
// same as Database::exist_triple
bool
KVstore::existThisTriple(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id) {
	unsigned* _objidlist = nullptr;
	unsigned _list_len = 0;
	this->getobjIDlistBysubIDpreID(_sub_id, _pre_id, _objidlist, _list_len, true);

	bool is_exist = false;
	if (Util::bsearch_int_uporder(_obj_id, _objidlist, _list_len) != INVALID) {
		is_exist = true;
	}

	delete[] _objidlist;
	return is_exist;
}

//TODO+BETTER: adjust the buffer size according to current memory usage(global memory manager)
//better to adjust these parameters according to memory usage and entity num
//need a memory manager first
string KVstore::s_entity2id = "s_entity2id";
string KVstore::s_id2entity = "s_id2entity";
unsigned short KVstore::buffer_entity2id_build = 8;
unsigned short KVstore::buffer_id2entity_build = 1;
unsigned short KVstore::buffer_entity2id_query = 2;
unsigned short KVstore::buffer_id2entity_query = 1;

string KVstore::s_predicate2id = "s_predicate2id";
string KVstore::s_id2predicate = "s_id2predicate";
unsigned short KVstore::buffer_predicate2id_build = 8;
unsigned short KVstore::buffer_id2predicate_build = 1;
unsigned short KVstore::buffer_predicate2id_query = 1;
unsigned short KVstore::buffer_id2predicate_query = 1;

string KVstore::s_literal2id = "s_literal2id";
string KVstore::s_id2literal = "s_id2literal";
unsigned short KVstore::buffer_literal2id_build = 8;
unsigned short KVstore::buffer_id2literal_build = 1;
unsigned short KVstore::buffer_literal2id_query = 2;
unsigned short KVstore::buffer_id2literal_query = 1;

string KVstore::s_sID2values = "s_sID2values";
string KVstore::s_oID2values = "s_oID2values";
string KVstore::s_pID2values = "s_pID2values";
unsigned short KVstore::buffer_sID2values_build = 16;
unsigned short KVstore::buffer_oID2values_build = 16;
unsigned short KVstore::buffer_pID2values_build = 8;
unsigned short KVstore::buffer_sID2values_query = 16;
unsigned short KVstore::buffer_oID2values_query = 16;
unsigned short KVstore::buffer_pID2values_query = 8;

//MVCC 

//MVCC

bool
KVstore::getValueByKey(IVArray* _array, unsigned _key, char*& _val, unsigned long & _vlen, VDataSet& AddSet, VDataSet& DelSet, shared_ptr<Transaction> txn, bool &latched, bool FirstRead) const
{
	//cout << "getValueByKey                  " << _key << FirstRead << endl;
	//cout << "this is transaction getValueByKey ..................." << endl;
	if (Util::is_literal_ele(_key) && _array == objID2values)
	{
		unsigned key = _key - Util::LITERAL_FIRST_ID;
		return objID2values_literal->search(key, _val, _vlen, AddSet, DelSet, txn, latched, FirstRead);
	}
	return _array->search(_key, _val, _vlen, AddSet, DelSet, txn, latched,  FirstRead);
}

bool 
KVstore::updateInsert_s2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id, shared_ptr<Transaction> txn)
{
	VDataSet addset;
	addset.insert(VData{_pre_id, _obj_id});
	return insert_values(this->subID2values, _sub_id, addset, txn);
}
bool 
KVstore::updateRemove_s2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id, shared_ptr<Transaction> txn)
{
	VDataSet delset;
	delset.insert(VData{_pre_id, _obj_id});
	return remove_values(this->subID2values, _sub_id, delset, txn);
}
bool 
KVstore::updateInsert_o2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id, shared_ptr<Transaction> txn)
{
	//cout << "updateInsert_o2values......................" << endl;
	VDataSet addset;
	addset.insert(VData{_pre_id, _sub_id});
	return insert_values(this->objID2values, _obj_id, addset, txn);
}
bool 
KVstore::updateRemove_o2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id, shared_ptr<Transaction> txn)
{
	VDataSet delset;
	delset.insert(VData{_pre_id, _sub_id});
	return remove_values(this->objID2values, _obj_id, delset, txn);
}
bool 
KVstore::updateInsert_p2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id, shared_ptr<Transaction> txn)
{
	//cout << "updateInsert_p2values......................" << endl;
	VDataSet addset;
	addset.insert(VData{_sub_id, _obj_id});
	return insert_values(this->preID2values, _pre_id, addset, txn);
}
bool 
KVstore::updateRemove_p2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id, shared_ptr<Transaction> txn)
{
	VDataSet delset;
	delset.insert(VData{_sub_id, _obj_id});
	return remove_values(this->preID2values, _pre_id, delset, txn);
}
	
bool
KVstore::insert_values(IVArray* _array, unsigned _key, VDataSet &addset, shared_ptr<Transaction> txn)
{
	if (Util::is_literal_ele(_key) && _array == objID2values)
	{
		unsigned key = _key - Util::LITERAL_FIRST_ID;
		return objID2values_literal->insert(key, addset, txn);
	}
	return _array->insert(_key, addset, txn);
}

bool 
KVstore::remove_values(IVArray* _array, unsigned _key, VDataSet &delset, shared_ptr<Transaction> txn)
{
	if (Util::is_literal_ele(_key) && _array == objID2values)
	{
		unsigned key = _key - Util::LITERAL_FIRST_ID;
		return objID2values_literal->remove(key, delset, txn);
	}
	else return _array->remove(_key, delset, txn);
}

bool 
KVstore::GetExclusiveLock(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id, shared_ptr<Transaction> txn)
{
	int base_timer = txn->get_wait_lock_time();
	int times = txn->get_retry_times();
	// for(int i = 0; i < times; i++)
	// {
		if(try_exclusive_lock(_sub_id, _pre_id, _obj_id, txn)) return true;
		//usleep(base_timer);
		//base_timer = base_timer*2;
	//}
	return false;
}


bool 
KVstore::GetExclusiveLocks(vector<TYPE_ENTITY_LITERAL_ID>& sids, vector<TYPE_ENTITY_LITERAL_ID>& oids, vector<TYPE_PREDICATE_ID>& pids, shared_ptr<Transaction> txn)
{
	int base_timer = txn->get_wait_lock_time();
	int times = txn->get_retry_times();
	// for(int i = 0; i < times; i++)
	// {
		if(try_exclusive_locks(sids, oids, pids, txn)) return true;
		//usleep(base_timer);
		//base_timer = base_timer*2;
	//}
	return false;
}

//get exclusive lock and create new version atomicly for write operation
bool 
KVstore::try_exclusive_lock(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id, shared_ptr<Transaction> txn)
{
	bool sub_has_read = txn->ReadSetFind(_sub_id, Transaction::IDType::SUBJECT);
	bool pre_has_read = txn->ReadSetFind(_pre_id, Transaction::IDType::PREDICATE);
	bool obj_has_read = txn->ReadSetFind(_obj_id, Transaction::IDType::OBJECT);
	bool sub_has_write = txn->WriteSetFind(_sub_id, Transaction::IDType::SUBJECT);
	bool pre_has_write = txn->WriteSetFind(_pre_id, Transaction::IDType::PREDICATE);
	bool obj_has_write = txn->WriteSetFind(_obj_id, Transaction::IDType::OBJECT);

	int sub_ret = 0, obj_ret = 0, pre_ret = 0;

	if(!pre_has_write){
		//cerr << "Try to get PREDICATE exclusive lock................." << endl;
	 	pre_ret = get_exclusive_latch(this->preID2values, _pre_id,txn, pre_has_read);
	 	if(pre_ret == 0)
		{
			//cerr << this_thread::get_id() << "pre id latch get failed!" << endl;
			txn->SetErrorType(TransactionErrorType::PRE_X);
	 		return false;
	 	}
	 }

	if(!sub_has_write){
		//cerr << "Try to get SUBJECT exclusive lock................." << endl;
		sub_ret = get_exclusive_latch(this->subID2values, _sub_id, txn, sub_has_read);
		if(sub_ret == 0){
			txn->SetErrorType(TransactionErrorType::SUB_X);
			bool invalid_pre = true;
	 		if(!pre_has_write && pre_ret == 1)
	 			invalid_pre = invalid_values(this->preID2values, _pre_id, txn, pre_has_read);
			assert(invalid_pre);
	 		return false;
		}
	}

	if(!obj_has_write){
		//cerr << "Try to get OBJECT exclusive lock................." << endl;
		obj_ret = get_exclusive_latch(this->objID2values, _obj_id, txn, obj_has_read);
		if(obj_ret == 0)
		{
			txn->SetErrorType(TransactionErrorType::OBJ_X);
			bool invalid_pre = true;
	 		if(!pre_has_write && pre_ret == 1){
				 invalid_pre = invalid_values(this->preID2values, _pre_id, txn, pre_has_read);
			}
			assert(invalid_pre);
			bool invalid_sub = true;
			if(!sub_has_write && sub_ret == 1) {
				invalid_sub = invalid_values(this->subID2values, _sub_id, txn, sub_has_read);
			}
			assert(invalid_sub);
			return false;
		}
	}

	//cout << "getExclusiveLocks success! TID:" << txn->GetTID() << endl;
	return true;
}


bool 
KVstore::try_exclusive_locks(vector<TYPE_ENTITY_LITERAL_ID>& sids, vector<TYPE_ENTITY_LITERAL_ID>& oids, vector<TYPE_PREDICATE_ID>& pids, shared_ptr<Transaction> txn)
{
	int s_num = sids.size(), p_num = pids.size(), o_num = oids.size();
	vector<bool> sub_has_write(s_num), pre_has_write(p_num), obj_has_write(o_num);
	vector<bool> sub_has_read(s_num), pre_has_read(p_num), obj_has_read(o_num);
	for(int i = 0; i < s_num; i++){
		sub_has_read[i] = txn->ReadSetFind(sids[i], Transaction::IDType::SUBJECT);
		sub_has_write[i] = txn->WriteSetFind(sids[i], Transaction::IDType::SUBJECT);
	}

	for(int i = 0; i < p_num; i++){
		pre_has_read[i] = txn->ReadSetFind(pids[i], Transaction::IDType::PREDICATE);
		pre_has_write[i] = txn->WriteSetFind(pids[i], Transaction::IDType::PREDICATE);
	}

	for(int i = 0; i < o_num; i++){
		obj_has_read[i] = txn->ReadSetFind(oids[i], Transaction::IDType::OBJECT);
		obj_has_write[i] = txn->WriteSetFind(oids[i],Transaction::IDType::OBJECT);
	}
	int sub_ret, obj_ret, pre_ret;
	bool sub_invalid = true, obj_invalid = true, pre_invalid = true;
	int k = 0;
	for(; k < s_num; k++)
	{
		if(!sub_has_write[k]){
			sub_ret = get_exclusive_latch(this->subID2values, sids[k], txn, sub_has_read[k]);
		}
		if(sub_ret == 0){
			for(int i = 0; i < k; i++)
			{
				bool invalid = true;
				if(!sub_has_write[i]){
					invalid = invalid_values(this->subID2values, sids[i], txn, sub_has_read[i]);
				}
				if(invalid == false){
					sub_invalid = false;
				}
			}
			return false;
		}
	}

	k = 0;
	for(; k < o_num; k++)
	{
		if(!obj_has_write[k]){
			obj_ret = get_exclusive_latch(this->objID2values, oids[k], txn, obj_has_read[k]);
		}
		if(obj_ret == 0)
		{
			for(int i = 0; i < k; i++)
			{
				bool invalid = true;
				if(!obj_has_write[i]){
					invalid = invalid_values(this->objID2values, oids[i], txn, obj_has_read[i]);
				}
				if(invalid == false){
					obj_invalid = false;
				}
			}

			for(int i = 0; i < s_num; i++)
			{
				bool invalid = true;
				if(!sub_has_write[i]){
					invalid = invalid_values(this->subID2values, sids[i], txn, sub_has_read[i]);
				}
				if(invalid == false){
					sub_invalid = false;
				}
			}
			return false;
		}
	}

	k = 0;
	 for(; k < p_num; k++)
	 {
	 	if(!pre_has_write[k]){
	 		pre_ret = get_exclusive_latch(this->preID2values, pids[k], txn, pre_has_read[k]);
	 	}
	 	if(pre_ret == 0)
	 	{
	 		for(int i = 0; i < k; i++)
	 		{
	 			bool invalid = true;
	 			if(!pre_has_write[i]){
	 				invalid = invalid_values(this->preID2values, pids[i], txn, pre_has_read[i]);
	 			}
	 			if(invalid == false){
	 				obj_invalid = false;
	 			}
	 		}

	 		for(int i = 0; i < s_num; i++)
	 		{
	 			bool invalid = true;
	 			if(!sub_has_write[i]){
	 				invalid = invalid_values(this->subID2values, sids[i], txn, sub_has_read[i]);
	 			}
	 			if(invalid == false){
	 				sub_invalid = false;
	 			}
	 		}

	 		for(int i = 0; i < o_num; i++)
			{
	 			bool invalid = true;
	 			if(!obj_has_write[i]){
	 				invalid = invalid_values(this->objID2values, oids[i], txn, obj_has_read[i]);
				}
				if(invalid == false){
	 				obj_invalid = false;
	 			}
	 		}

	 	}
	}

	return true;
}

int 
KVstore::get_exclusive_latch(IVArray* _array, unsigned _key, shared_ptr<Transaction> txn, bool has_read) const
{
	if (Util::is_literal_ele(_key) && _array == objID2values)
	{
		unsigned key = _key - Util::LITERAL_FIRST_ID;
		return objID2values_literal->TryExclusiveLatch(key, txn, has_read);
	}
	return _array->TryExclusiveLatch(_key, txn, has_read);
}

//abort
bool
KVstore::ReleaseExclusiveLock(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id, shared_ptr<Transaction> txn)
{
	bool ret = true;
	//we can not unlatch exsited exclusive latch
	bool sub_has_read = txn->ReadSetFind(_sub_id, Transaction::IDType::SUBJECT);
	bool pre_has_read = txn->ReadSetFind(_pre_id, Transaction::IDType::PREDICATE);
	bool obj_has_read = txn->ReadSetFind(_obj_id, Transaction::IDType::OBJECT);
	if(txn != nullptr && txn->WriteSetFind(_sub_id, Transaction::IDType::SUBJECT) == false){
		if(invalid_values(this->subID2values, _sub_id, txn, sub_has_read) == false) {
			ret = false;
		}
	}
	if(txn != nullptr && txn->WriteSetFind(_obj_id, Transaction::IDType::OBJECT) == false){		
		if(invalid_values(this->objID2values, _obj_id, txn, obj_has_read) == false){
			ret = false;
		}
	}
	if(txn != nullptr && txn->WriteSetFind(_pre_id, Transaction::IDType::PREDICATE) == false){ 
		if(invalid_values(this->preID2values, _pre_id, txn,pre_has_read) == false) {
			ret = false;
		}
	}
	return ret;
}

bool 
KVstore::invalid_values(IVArray* _array, unsigned _key, shared_ptr<Transaction> txn, bool has_read)
{
	if (Util::is_literal_ele(_key) && _array == objID2values)
	{
		unsigned key = _key - Util::LITERAL_FIRST_ID;
		return objID2values_literal->Rollback(key, txn, has_read);
	}
	return _array->Rollback(_key, txn, has_read);
}


bool 
KVstore::release_exclusive_latch(IVArray* _array, unsigned _key, shared_ptr<Transaction> txn) const
{
	if (Util::is_literal_ele(_key) && _array == objID2values)
	{
		unsigned key = _key - Util::LITERAL_FIRST_ID;
		return objID2values_literal->ReleaseLatch(key, txn, IVEntry::LatchType::EXCLUSIVE);
	}
	return _array->ReleaseLatch(_key, txn, IVEntry::LatchType::EXCLUSIVE);
}

bool 
KVstore::release_shared_latch(IVArray* _array, unsigned _key, shared_ptr<Transaction> txn) const
{
	if (Util::is_literal_ele(_key) && _array == objID2values)
	{
		unsigned key = _key - Util::LITERAL_FIRST_ID;
		return objID2values_literal->ReleaseLatch(key, txn, IVEntry::LatchType::SHARED);
	}
	return _array->ReleaseLatch(_key, txn, IVEntry::LatchType::SHARED);
}

bool 
KVstore::ReleaseAllLocks(shared_ptr<Transaction> txn) const
{
	auto& WriteSet = txn->Get_WriteSet();
	auto& subWset = WriteSet[(unsigned)Transaction::IDType::SUBJECT];
	auto& preWset = WriteSet[(unsigned)Transaction::IDType::PREDICATE];
	auto& objWset = WriteSet[(unsigned)Transaction::IDType::OBJECT];
	
	int ret1 = true;
	//shared Latch
	if(txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE)
	{
		auto& ReadSet = txn->Get_ReadSet();
		auto& subRset = ReadSet[(unsigned)Transaction::IDType::SUBJECT];
		auto& preRset = ReadSet[(unsigned)Transaction::IDType::PREDICATE];
		auto& objRset = ReadSet[(unsigned)Transaction::IDType::OBJECT];
		//check the released key
		for(auto &it: preRset)
		{
			if(preWset.find(it) == preWset.end())
			{
				if(this->release_shared_latch(this->preID2values, it, txn) == false){
					ret1 = false;//release shared latch
				}
			}
		}

		for(auto &it: subRset)
		{
			if(subWset.find(it) == subWset.end())//not exclusive latched
			{	
				if(this->release_shared_latch(this->subID2values, it, txn) == false) {
					ret1 = false;//release shared latch
				}
			}
		}
		
		for(auto &it: objRset)
		{
			if(objWset.find(it) == objWset.end())
			{
				if(this->release_shared_latch(this->objID2values, it, txn) == false){
					ret1 = false;//release shared latch
				}

			}
		}
	}
	
	//get all key with exclusive latched
	//exclusive Latch
	
	bool ret2 = true;
	//release exclusive latch
	for(auto &it: preWset)
		if(this->release_exclusive_latch(this->preID2values, it, txn) == false){
				ret2 = false;
		}
	for(auto &it: subWset)
		if(this->release_exclusive_latch(this->subID2values, it, txn) == false) {
				ret2 = false;
		}
	for(auto &it: objWset)
		if(this->release_exclusive_latch(this->objID2values, it, txn) == false){
				ret2 = false;
		}
	assert(ret1 && ret2);
	return ret1 && ret2;
	
}

bool
KVstore::TransactionInvalid(shared_ptr<Transaction> txn)
{
	auto& WriteSet = txn->Get_WriteSet();
	auto& subWset = WriteSet[(unsigned)Transaction::IDType::SUBJECT];
	auto& preWset = WriteSet[(unsigned)Transaction::IDType::PREDICATE];
	auto& objWset = WriteSet[(unsigned)Transaction::IDType::OBJECT];
	
	int ret1 = true;
	if(txn->GetIsolationLevelType() == IsolationLevelType::SERIALIZABLE)
	{
		auto& ReadSet = txn->Get_ReadSet();
		auto& subRset = ReadSet[(unsigned)Transaction::IDType::SUBJECT];
		auto& preRset = ReadSet[(unsigned)Transaction::IDType::PREDICATE];
		auto& objRset = ReadSet[(unsigned)Transaction::IDType::OBJECT];
		//check the released key
		for(auto &it: preRset)
		{
			if(preWset.find(it) == preWset.end())
			{
				if(this->release_shared_latch(this->preID2values, it, txn) == false) ret1 = false;
			}
		}
		for(auto &it: subRset)
		{
			if(subWset.find(it) == subWset.end())//shared lock only
			{			
				if(this->release_shared_latch(this->subID2values, it, txn) == false) {
					ret1 = false;//release shared latch
				}
			}
		}
		for(auto &it: objRset)
		{
			if(objWset.find(it) == objWset.end())
			{
				if(this->release_shared_latch(this->objID2values, it, txn) == false){
					ret1 = false;
				} 
			}
		}
	}
	
	//get all key with exclusive latched
	//exclusive Latch
	
	bool ret2 = true;
	//invalid all updates and release latches
	for(auto &it: preWset)
	 	if(this->invalid_values(this->preID2values, it, txn, false) == false) ret2 = false;
	for(auto &it: subWset){
		if(this->invalid_values(this->subID2values, it, txn, false) == false) ret2 = false;
	}
	for(auto &it: objWset){
		if(this->invalid_values(this->objID2values, it, txn, false) == false) ret2 = false;	
	}
	
	if(ret1 == false)
	{
		cerr << "shared unlock failed !" << endl;
	}
	if(ret2 == false)
	{
		cerr << "exclusive unlock failed !" << endl;
	}
	assert(ret1 && ret2);
	return ret1 && ret2;
}

void print_merge_tmp(unsigned* _tmp, unsigned long _len)
{
	for(int i = 0; i < (_len / sizeof(unsigned)); i++)
		cout << _tmp[i] << "  ";
	cout << endl;
}

void 
KVstore::IVArrayVacuum(vector<unsigned>& sub_ids , vector<unsigned>& obj_ids, vector<unsigned>& obj_literal_ids, vector<unsigned>& pre_ids) 
{
	//no transaction is running in this database!
	//we can use four thread to clean three(four) index!
	string name("clean");
	shared_ptr<Transaction> clean_txn = make_shared<Transaction>(name, (INVALID_TS - 1), INVALID_TID, IsolationLevelType::READ_COMMITTED);
	//vector<unsigned> sub_ids, obj_ids, obj_literal_ids, pre_ids;
	s2values_vacuum(sub_ids, clean_txn);
	o2values_vacuum(obj_ids, clean_txn);
	o2values_literal_vacuum(obj_literal_ids, clean_txn);
	p2values_vacuum(pre_ids, clean_txn);
	
	//join here	
	//dictionary_Vacuum();
}

void 
KVstore::s2values_vacuum(vector<unsigned>& sub_ids, shared_ptr<Transaction> txn) 
{
	//cerr << "KVstore::s2values_Vacuum................................." << endl;
	//getDirtyKeys(this->subID2values, sub_ids);
	bool FirstRead = false;
	unsigned* _tmp = NULL;
	unsigned long _len = 0, base_len;
	bool base_empty = false;
	//cout << "sub_ids.size():" << sub_ids.size() << endl;
	for(auto _subid:  sub_ids)
	{
		VDataSet addset, delset;
		bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len, addset, delset, txn, FirstRead);
		
		base_empty = (_tmp == nullptr);
		vector<unsigned> add_pidoidlist, del_pidoidlist;
		for(auto po: addset)
		{
			add_pidoidlist.push_back(po.first);
			add_pidoidlist.push_back(po.second);
		}

		for(auto po: delset)
		{
			del_pidoidlist.push_back(po.first);
			del_pidoidlist.push_back(po.second);
		}
		//cout << add_pidoidlist.size() << "     " << del_pidoidlist.size() << endl;
		//if(!_get) cout << " this->getValueByKey failed ......." << endl;
		unsigned * _values = nullptr;
		unsigned long _values_len = 0;
		// cout << "_len:                                                 " <<  _len << endl;
		// for(int i = 0; i < _len / sizeof(unsigned); i++) cout << _tmp[i] << " ";
		// cout << endl;
		//cerr << "addset.size():           " << addset.size() << "    delset.size()                      " << delset.size() << endl;
		if(add_pidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_s2values(add_pidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cerr << ".........._len:                                                 " <<  _len << endl;
		if(del_pidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_s2values(del_pidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cerr << ".........._len:                                                 " <<  _len << endl;

		// for(int i = 0; i < _len / sizeof(unsigned); i++) cout << _tmp[i] << " ";
		// cout << endl;
		this->clean_dirty_key(this->subID2values, _subid);

		if(_len == 0 ) {
			if(base_empty == false){
				cerr << "delete keys" << endl;
				this->removeKey(this->subID2values, _subid);
			}
			continue;
		}
		
		if(base_empty == true){
			//cout << "addValueByKey! in s2values" << endl;
			this->addValueByKey(this->subID2values, _subid, (char*)_tmp, _len);
		}
		else
			this->setValueByKey(this->subID2values, _subid, (char*)_tmp, _len);
		//reset
		//delete [] _tmp;
		//print_merge_tmp(_tmp, _len);
	}
}

void 
KVstore::o2values_vacuum(vector<unsigned>& obj_ids, shared_ptr<Transaction> txn) 
{
	//cerr << "KVstore::o2values_Vacuum..............................................." << endl;
	//getDirtyKeys(this->objID2values, obj_ids);
	bool FirstRead = false;
	unsigned* _tmp = NULL;
	unsigned long _len = 0, base_len;
	bool base_empty = false;
	//cerr << "obj_ids.size()" << obj_ids.size() << endl;
	for(auto _obj_id:  obj_ids)
	{
		VDataSet addset, delset;
		bool _get = this->getValueByKey(this->objID2values, _obj_id, (char*&)_tmp, _len, addset, delset, txn, FirstRead);
		base_empty = (_tmp == nullptr);
		vector<unsigned> add_pidsidlist, del_pidsidlist;
		for(auto ps: addset){
			add_pidsidlist.push_back(ps.first);
			add_pidsidlist.push_back(ps.second);
		}

		for(auto ps: delset){
			del_pidsidlist.push_back(ps.first);
			del_pidsidlist.push_back(ps.second);
		}
		unsigned * _values = nullptr;
		unsigned long _values_len = 0;
		//cout << "_len:                                                 " <<  _len << endl;
		if(add_pidsidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_o2values(add_pidsidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cerr << "_len:                                                 " <<  _len << endl;
		if(del_pidsidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_o2values(del_pidsidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cout << "_len:                                                 " <<  _len << endl;
		// for(int i = 0; i < _len; i++) cout << _tmp[i] << " ";
		// cout << endl;
		this->clean_dirty_key(this->objID2values, _obj_id);

		if(_len == 0) {
			if(base_empty == false){
				cerr << "delete keys" << endl;
				this->removeKey(this->objID2values, _obj_id);
			}
			continue;
		}
		
		if(base_empty == true){
			cout << "addValueByKey in objID2values" << endl;
			this->addValueByKey(this->objID2values, _obj_id, (char*)_tmp, _len);
		} 
		else this->setValueByKey(this->objID2values, _obj_id, (char*)_tmp, _len);
		//print_merge_tmp(_tmp, _len);
		//reset
		//delete [] _tmp;
	}
}

void 
KVstore::o2values_literal_vacuum(vector<unsigned>& obj_literal_ids, shared_ptr<Transaction> txn) 
{
	//cerr << "KVstore::o2values_literal_Vacuum.............................................." << endl;
	//getDirtyKeys(this->objID2values_literal, obj_literal_ids);
	bool FirstRead = false;
	unsigned* _tmp = NULL;
	unsigned long _len = 0, base_len;
	bool base_empty = false;
	//cerr << "o2values_literal.size()" << obj_literal_ids.size() << endl;
	for(auto _obj_id:  obj_literal_ids)
	{
		VDataSet addset, delset;
		bool _get = this->getValueByKey(this->objID2values, _obj_id, (char*&)_tmp, _len, addset, delset, txn, FirstRead);
		base_empty = (_tmp == nullptr);
		vector<unsigned> add_pidsidlist, del_pidsidlist;
		for(auto ps: addset){
			add_pidsidlist.push_back(ps.first);
			add_pidsidlist.push_back(ps.second);
		}

		for(auto ps: delset){
			del_pidsidlist.push_back(ps.first);
			del_pidsidlist.push_back(ps.second);
		}

		unsigned * _values = nullptr;
		unsigned long _values_len = 0;
		//cout << "_len:                                                 " <<  _len << endl;
		//for(int i = 0; i < _len / sizeof(unsigned); i++) cout << _tmp[i] << " ";
		//cout << endl;
		//cerr << "addset.size():           " << addset.size() << "    delset.size()                      " << delset.size() << endl;
		if(add_pidsidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_o2values(add_pidsidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cout << "_len:                                                 " <<  _len << endl;
		if(del_pidsidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_o2values(del_pidsidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cout << "_len:                                                 " <<  _len << endl;
		//for(int i = 0; i < _len / sizeof(unsigned); i++) cout << _tmp[i] << " ";
		//cout << endl;
		this->clean_dirty_key(this->objID2values_literal, _obj_id - Util::LITERAL_FIRST_ID);
		if(_len == 0) {
			if(base_empty == false)
			{
				cerr << "delete keys" << endl;
				// assert(false);
				this->removeKey(this->objID2values, _obj_id);
			}
			continue;
		}
		
		if(base_empty == true){
			this->addValueByKey(this->objID2values, _obj_id, (char*)_tmp, _len);
			
		} 
		else this->setValueByKey(this->objID2values, _obj_id, (char*)_tmp, _len);
		//print_merge_tmp(_tmp, _len);
		//reset
		//delete [] _tmp;
		_tmp = nullptr;
		_len = 0;
	}
}

void 
KVstore::p2values_vacuum(vector<unsigned>& pre_ids, shared_ptr<Transaction> txn) 
{
	//cerr << "KVstore::p2values_Vacuum....................................................." << endl;
	//getDirtyKeys(this->preID2values, pre_ids);
	bool FirstRead = false;
	unsigned* _tmp = NULL;
	unsigned long _len = 0, base_len;
	bool base_empty = false;
	//cerr << "pre_ids.size():" << pre_ids.size() << endl;
	for(auto _pre_id:  pre_ids)
	{
		VDataSet addset, delset;
		bool _get = this->getValueByKey(this->preID2values, _pre_id, (char*&)_tmp, _len, addset, delset, txn, FirstRead);
		base_empty = (_tmp == nullptr) ;
		vector<unsigned> add_sidoidlist, del_sidoidlist;
		for(auto so: addset)
		{
			add_sidoidlist.push_back(so.first);
			add_sidoidlist.push_back(so.second);
		}

		for(auto so: delset)
		{
			del_sidoidlist.push_back(so.first);
			del_sidoidlist.push_back(so.second);
		}
		
		unsigned * _values = nullptr;
		unsigned long _values_len;
		//cout << "_len:                                                 " <<  _len << endl;
		//cerr << "addset.size():           " << addset.size() << "   delset.size()                      " << delset.size() << endl;
		if(add_sidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Insert_p2values(add_sidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cout << "_len:                                                 " <<  _len << endl;
		if(del_sidoidlist.size() != 0){
			_len = _len / sizeof(unsigned);
			this->Remove_p2values(del_sidoidlist, _tmp, _len, _values, _values_len);
			_tmp = _values;
			_len = _values_len * sizeof(unsigned);
		}
		//cout << "_len:                                                 " <<  _len << endl;
		this->clean_dirty_key(this->preID2values, _pre_id);

		if(_len == 0) {
			if(base_empty == false){
				cerr << "delete keys" << endl;
				this->removeKey(this->preID2values, _pre_id);
			}
			continue;
		}
		
		if(base_empty == true)
			this->addValueByKey(this->preID2values, _pre_id, (char*)_tmp, _len);
		else
			this->setValueByKey(this->preID2values, _pre_id, (char*)_tmp, _len);
		//print_merge_tmp(_tmp, _len);
		//cout << "ret:                           " << ret << endl;
		//reset
		//delete [] _tmp;
		_tmp = nullptr;
		_len = 0;
	}
}


bool 
KVstore::clean_dirty_key(IVArray* _array, unsigned _key)
{
	return _array->CleanDirtyKey(_key);
}
