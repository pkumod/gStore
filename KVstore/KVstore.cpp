/*=============================================================================
# Filename: KVstore.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-23 14:25
# Description: Modified By Wang Libo
=============================================================================*/

#include "KVstore.h"

using namespace std;

//sets store_path as the root dir of this KVstore
//initial all Tree pointers as NULL
KVstore::KVstore(string _store_path) {
	this->store_path = _store_path;

	this->entity2id = NULL;
	this->id2entity = NULL;

	this->predicate2id = NULL;
	this->id2predicate = NULL;

	this->literal2id = NULL;
	this->id2literal = NULL;

	this->subID2values = NULL;
	this->preID2values = NULL;
	this->objID2values = NULL;
}

//Release all the memory used in this KVstore before destruction
KVstore::~KVstore() {
	this->flush();
	this->release();
}

//Flush all modified parts into the disk, which will not release any memory
//Does nothing to null tree pointers or parts that has not been modified 
void KVstore::flush() {
	this->flush(this->entity2id);
	this->flush(this->id2entity);

	this->flush(this->literal2id);
	this->flush(this->id2literal);

	this->flush(this->predicate2id);
	this->flush(this->id2predicate);

	this->flush(this->subID2values);
	this->flush(this->preID2values);
	this->flush(this->objID2values);
}

void KVstore::release() {
	delete this->entity2id;
	this->entity2id = NULL;
	delete this->id2entity;
	this->id2entity = NULL;

	delete this->literal2id;
	this->literal2id = NULL;
	delete this->id2literal;
	this->id2literal = NULL;

	delete this->predicate2id;
	this->predicate2id = NULL;
	delete this->id2predicate;
	this->id2predicate = NULL;

	delete this->subID2values;
	this->subID2values = NULL;
	delete this->preID2values;
	this->preID2values = NULL;
	delete this->objID2values;
	this->objID2values = NULL;
}

void KVstore::open() {
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
}

int KVstore::getEntityDegree(int _entity_id) const {
	return this->getEntityInDegree(_entity_id) + this->getEntityOutDegree(_entity_id);
}

int KVstore::getEntityInDegree(int _entity_id) const {
	//cout << "In getEntityInDegree " << _entity_id << endl;
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->objID2values, _entity_id, (char*&)_tmp, _len);
	if (!_get) {
		return 0;
	}
	return _tmp[0];
}

int KVstore::getEntityOutDegree(int _entity_id) const {
	//cout << "In getEntityOutDegree " << _entity_id << endl;
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->subID2values, _entity_id, (char*&)_tmp, _len);
	if (!_get) {
		return 0;
	}
	return _tmp[0];
}

int KVstore::getLiteralDegree(int _literal_id) const {
	//cout << "In getLiteralDegree " << _literal_id << endl;
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->objID2values, _literal_id, (char*&)_tmp, _len);
	if (!_get) {
		return 0;
	}
	return _tmp[0];
}

int KVstore::getPredicateDegree(int _predicate_id) const {
	//cout << "In getPredicate Degree " << _predicate_id << endl;
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->preID2values, _predicate_id, (char*&)_tmp, _len);
	if (!_get) {
		return 0;
	}
	return _tmp[0];
}

int KVstore::getSubjectPredicateDegree(int _subid, int _preid) const {
	//cout << "In getSubjectPredicateDegree " << _subid << ' ' << _preid << endl;
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);
	if (!_get) {
		return 0;
	}
	int _result = KVstore::binarySearch(_preid, _tmp + 3, _tmp[1], 2);
	if (_result == -1) {
		return 0;
	}
	int _offset = _tmp[4 + 2 * _result];
	int _offset_next;
	if (_result == _tmp[1] - 1) {
		_offset_next = 3 + 2 * _tmp[1] + _tmp[0];
	}
	else {
		_offset_next = _tmp[6 + 2 * _result];
	}
	return _offset_next - _offset;
}

int KVstore::getObjectPredicateDegree(int _objid, int _preid) const {
	//cout << "In getObjectPredicateDegree " << _objid << _preid << endl;
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len);
	if (!_get) {
		return 0;
	}
	int _result = KVstore::binarySearch(_preid, _tmp + 2, _tmp[1], 2);
	if (_result == -1) {
		return 0;
	}
	int _offset = _tmp[3 + 2 * _result];
	int _offset_next;
	if (_result == _tmp[1] - 1) {
		_offset_next = 2 + 2 * _tmp[1] + _tmp[0];
	}
	else {
		_offset_next = _tmp[5 + 2 * _result];
	}
	return _offset_next - _offset;
}

bool KVstore::updateTupleslist_insert(int _sub_id, int _pre_id, int _obj_id) {
	//int* _tmp = NULL;
	//int _len = 0;
	//bool _get;
	//cout << "Inserting:\t" << _sub_id << '\t' << _pre_id << '\t' << _obj_id << endl;
	//cout << "Before insertion:" << endl;
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
	//bool flag = this->updateInsert_s2values(_sub_id, _pre_id, _obj_id)
	//	&& this->updateInsert_o2values(_sub_id, _pre_id, _obj_id)
	//	&& this->updateInsert_p2values(_sub_id, _pre_id, _obj_id);
	//cout << "After insertion:" << endl;
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
	return this->updateInsert_s2values(_sub_id, _pre_id, _obj_id)
		&& this->updateInsert_o2values(_sub_id, _pre_id, _obj_id)
		&& this->updateInsert_p2values(_sub_id, _pre_id, _obj_id);
}

bool KVstore::updateTupleslist_remove(int _sub_id, int _pre_id, int _obj_id) {
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
	return this->updateRemove_s2values(_sub_id, _pre_id, _obj_id)
		&& this->updateRemove_o2values(_sub_id, _pre_id, _obj_id)
		&& this->updateRemove_p2values(_sub_id, _pre_id, _obj_id);
}

bool KVstore::updateInsert_s2values(int _sub_id, int _pre_id, int _obj_id) {
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->subID2values, _sub_id, (char*&)_tmp, _len);
	bool _is_entity = KVstore::isEntity(_obj_id);

	//subID doesn't exist
	if (!_get) {
		int _values[6];
		_values[0] = 1;
		_values[1] = 1;
		_values[2] = _is_entity ? 1 : 0;
		_values[3] = _pre_id;
		_values[4] = 5;
		_values[5] = _obj_id;
		this->addValueByKey(this->subID2values, _sub_id, (char*)_values, sizeof(int) * 6);
	}

	//subID exists
	else {
		int* _values;
		int _values_len;
		int _position = KVstore::binarySearch(_pre_id, _tmp + 3, _tmp[1], 2);

		//preID doesn't exist
		if (_position == -1) {
			_values_len = _len / sizeof(int) + 3;
			_values = new int[_values_len];
			_values[0] = _tmp[0] + 1;
			_values[1] = _tmp[1] + 1;
			_values[2] = _tmp[2] + (_is_entity ? 1 : 0);
			int i, j;
			for (i = 0, j = 3; i < _tmp[1] && _tmp[3 + 2 * i] < _pre_id; i++, j += 2) {
				_values[j] = _tmp[3 + 2 * i];
				_values[j + 1] = _tmp[4 + 2 * i] + 2;
			}
			_values[j] = _pre_id;
			int _offset_old;
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
		else {
			_values_len = _len / sizeof(int) + 1;
			_values = new int[_values_len];
			memcpy(_values, _tmp, sizeof(int) * _tmp[4 + 2 * _position]);
			_values[0]++;
			if (_is_entity) {
				_values[2]++;
			}
			for (int i = _position + 1; i < _tmp[1]; i++) {
				_values[4 + 2 * i]++;
			}
			int i, j;
			int right;
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

		this->setValueByKey(this->subID2values, _sub_id, (char*)_values, sizeof(int) * _values_len);
		delete[] _values;
	}

	return true;
}

bool KVstore::updateRemove_s2values(int _sub_id, int _pre_id, int _obj_id) {
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->subID2values, _sub_id, (char*&)_tmp, _len);
	bool _is_entity = KVstore::isEntity(_obj_id);

	if (!_get) {
		return false;
	}

	//subID doesn't exist after removal
	if (_tmp[0] == 1) {
		this->removeKey(this->subID2values, _sub_id);
	}

	//subID still exists after removal
	else {
		int* _values;
		int _values_len;
		int _position = KVstore::binarySearch(_pre_id, _tmp + 3, _tmp[1], 2);
		int _oidlen_sp;
		if (_position == _tmp[1] - 1) {
			_oidlen_sp = 3 + 2 * _tmp[1] + _tmp[0] - _tmp[4 + 2 * _position];
		}
		else {
			_oidlen_sp = _tmp[6 + 2 * _position] - _tmp[4 + 2 * _position];
		}

		//preID doesn't exist after removal
		if (_oidlen_sp == 1) {
			_values_len = _len / sizeof(int) - 3;
			_values = new int[_values_len];
			memcpy(_values, _tmp, sizeof(int) * (3 + 2 * _position));
			_values[0]--;
			_values[1]--;
			if (_is_entity) {
				_values[2]--;
			}
			for (int i = 0; i < _position; i++) {
				_values[4 + 2 * i] -= 2;
			}
			int i, j;
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
			_values_len = _len / sizeof(int) - 1;
			_values = new int[_values_len];
			memcpy(_values, _tmp, sizeof(int) * _tmp[4 + 2 * _position]);
			_values[0]--;
			if (_is_entity) {
				_values[2]--;
			}
			for (int i = _position + 1; i < _tmp[1]; i++) {
				_values[4 + 2 * i]--;
			}
			int i, j;
			for (i = _tmp[4 + 2 * _position], j = _tmp[4 + 2 * _position];
				i < 3 + 2 * _tmp[1] + _tmp[0] && _tmp[i] < _obj_id; i++, j++) {
				_values[j] = _tmp[i];
			}
			i++;
			for (; i < 3 + 2 * _tmp[1] + _tmp[0]; i++, j++) {
				_values[j] = _tmp[i];
			}
		}

		this->setValueByKey(this->subID2values, _sub_id, (char*)_values, sizeof(int) * _values_len);
		delete[] _values;
	}

	return true;
}

//TO BE IMPROVED
bool KVstore::updateInsert_s2values(int _subid, const std::vector<int>& _pidoidlist) {
	vector<int>::const_iterator iter = _pidoidlist.begin();
	while (iter < _pidoidlist.end()) {
		int _preid = *iter;
		iter++;
		int _objid = *iter;
		iter++;
		this->updateInsert_s2values(_subid, _preid, _objid);
	}
	return true;
}

//TO BE IMPROVED
bool KVstore::updateRemove_s2values(int _subid, const std::vector<int>& _pidoidlist) {
	vector<int>::const_iterator iter = _pidoidlist.begin();
	while (iter < _pidoidlist.end()) {
		int _preid = *iter;
		iter++;
		int _objid = *iter;
		iter++;
		this->updateRemove_s2values(_subid, _preid, _objid);
	}
	return true;
}

bool KVstore::updateInsert_o2values(int _sub_id, int _pre_id, int _obj_id) {
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->objID2values, _obj_id, (char*&)_tmp, _len);

	//objID doesn't exist
	if (!_get) {
		int _values[5];
		_values[0] = 1;
		_values[1] = 1;
		_values[2] = _pre_id;
		_values[3] = 4;
		_values[4] = _sub_id;
		this->addValueByKey(this->objID2values, _obj_id, (char*)_values, sizeof(int) * 5);
	}

	//objID exists
	else {
		int* _values;
		int _values_len;
		int _position = KVstore::binarySearch(_pre_id, _tmp + 2, _tmp[1], 2);

		//preID doesn't exist
		if (_position == -1) {
			_values_len = _len / sizeof(int) + 3;
			_values = new int[_values_len];
			_values[0] = _tmp[0] + 1;
			_values[1] = _tmp[1] + 1;
			int i, j;
			for (i = 0, j = 2; i < _tmp[1] && _tmp[2 + 2 * i] < _pre_id; i++, j += 2) {
				_values[j] = _tmp[2 + 2 * i];
				_values[j + 1] = _tmp[3 + 2 * i] + 2;
			}
			_values[j] = _pre_id;
			int _offset_old;
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
			_values_len = _len / sizeof(int) + 1;
			_values = new int[_values_len];
			memcpy(_values, _tmp, sizeof(int) * _tmp[3 + 2 * _position]);
			_values[0]++;
			for (int i = _position + 1; i < _tmp[1]; i++) {
				_values[3 + 2 * i]++;
			}
			int i, j;
			int right;
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

		this->setValueByKey(this->objID2values, _obj_id, (char*)_values, sizeof(int) * _values_len);
		delete[] _values;
	}

	return true;
}

bool KVstore::updateRemove_o2values(int _sub_id, int _pre_id, int _obj_id) {
	int* _tmp = NULL;
	int _len = 0;
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
		int* _values;
		int _values_len;
		int _position = KVstore::binarySearch(_pre_id, _tmp + 2, _tmp[1], 2);
		int _sidlen_op;
		if (_position == _tmp[1] - 1) {
			_sidlen_op = 2 + 2 * _tmp[1] + _tmp[0] - _tmp[3 + 2 * _position];
		}
		else {
			_sidlen_op = _tmp[5 + 2 * _position] - _tmp[3 + 2 * _position];
		}

		//preID doesn't exist after removal
		if (_sidlen_op == 1) {
			_values_len = _len / sizeof(int) - 3;
			_values = new int[_values_len];
			memcpy(_values, _tmp, sizeof(int) * (2 + 2 * _position));
			_values[0]--;
			_values[1]--;
			for (int i = 0; i < _position; i++) {
				_values[3 + 2 * i] -= 2;
			}
			int i, j;
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
			_values_len = _len / sizeof(int) - 1;
			_values = new int[_values_len];
			memcpy(_values, _tmp, sizeof(int) * _tmp[3 + 2 * _position]);
			_values[0]--;
			for (int i = _position + 1; i < _tmp[1]; i++) {
				_values[3 + 2 * i]--;
			}
			int i, j;
			for (i = _tmp[3 + 2 * _position], j = _tmp[3 + 2 * _position];
				i < 2 + 2 * _tmp[1] + _tmp[0] && _tmp[i] < _sub_id; i++, j++) {
				_values[j] = _tmp[i];
			}
			i++;
			for (; i < 2 + 2 * _tmp[1] + _tmp[0]; i++, j++) {
				_values[j] = _tmp[i];
			}
		}

		this->setValueByKey(this->objID2values, _obj_id, (char*)_values, sizeof(int) * _values_len);
		delete[] _values;
	}

	return true;
}

//TO BE IMPROVED
bool KVstore::updateInsert_o2values(int _objid, const std::vector<int>& _pidsidlist) {
	vector<int>::const_iterator iter = _pidsidlist.begin();
	while (iter < _pidsidlist.end()) {
		int _preid = *iter;
		iter++;
		int _subid = *iter;
		iter++;
		this->updateInsert_o2values(_subid, _preid, _objid);
	}
	return true;
}

//TO BE IMPROVED
bool KVstore::updateRemove_o2values(int _objid, const std::vector<int>& _pidsidlist) {
	vector<int>::const_iterator iter = _pidsidlist.begin();
	while (iter < _pidsidlist.end()) {
		int _preid = *iter;
		iter++;
		int _subid = *iter;
		iter++;
		this->updateRemove_o2values(_subid, _preid, _objid);
	}
	return true;
}

bool KVstore::updateInsert_p2values(int _sub_id, int _pre_id, int _obj_id) {
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->preID2values, _pre_id, (char*&)_tmp, _len);

	//preid doesn't exist
	if (!_get) {
		int _values[3];
		_values[0] = 1;
		_values[1] = _sub_id;
		_values[2] = _obj_id;
		this->addValueByKey(this->preID2values, _pre_id, (char*)_values, sizeof(int) * 3);
	}

	//preid exists
	else {
		int _values_len = _len / sizeof(int) + 2;
		int* _values = new int[_values_len];
		int i, j;
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
		this->setValueByKey(this->preID2values, _pre_id, (char*)_values, sizeof(int) * _values_len);
		delete[] _values;
	}

	return true;
}

bool KVstore::updateRemove_p2values(int _sub_id, int _pre_id, int _obj_id) {
	int* _tmp = NULL;
	int _len = 0;
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
		int _values_len = _len / sizeof(int) - 2;
		int* _values = new int[_values_len];
		int i, j;
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
		this->setValueByKey(this->preID2values, _pre_id, (char*)_values, sizeof(int) * _values_len);
		delete[] _values;
	}

	return true;
}

//TO BE IMPROVED
bool KVstore::updateInsert_p2values(int _preid, const std::vector<int>& _sidoidlist) {
	vector<int>::const_iterator iter = _sidoidlist.begin();
	while (iter < _sidoidlist.end()) {
		int _subid = *iter;
		iter++;
		int _objid = *iter;
		iter++;
		this->updateInsert_p2values(_subid, _preid, _objid);
	}
	return true;
}

//TO BE IMPROVED
bool KVstore::updateRemove_p2values(int _preid, const std::vector<int>& _sidoidlist) {
	vector<int>::const_iterator iter = _sidoidlist.begin();
	while (iter < _sidoidlist.end()) {
		int _subid = *iter;
		iter++;
		int _objid = *iter;
		iter++;
		this->updateRemove_p2values(_subid, _preid, _objid);
	}
	return true;
}

//for entity2id
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool KVstore::open_entity2id(int _mode) {
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_entity2id_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_entity2id_query;
	}
	else {
		cerr << "Invalid open mode in open_entity2id, mode = " << _mode << endl;
		return false;
	}
	return this->open(this->entity2id, KVstore::s_entity2id, _mode, buffer_size);
}

bool KVstore::close_entity2id() {
	if (this->entity2id == NULL) {
		return true;
	}
	this->entity2id->save();
	delete this->entity2id;
	this->entity2id = NULL;
	return true;
}

bool KVstore::subIDByEntity(string _entity) {
	return this->entity2id->remove(_entity.c_str(), _entity.length());
}

int KVstore::getIDByEntity(string _entity) const {
	return this->getIDByStr(this->entity2id, _entity.c_str(), _entity.length());
}

bool KVstore::setIDByEntity(string _entity, int _id) {
	return this->addValueByKey(this->entity2id, _entity.c_str(), _entity.length(), _id);
}

//for id2entity
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool KVstore::open_id2entity(int _mode) {
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2entity_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2entity_query;
	}
	else {
		cerr << "Invalid open mode in open_id2entity, mode = " << _mode << endl;
		return false;
	}
	return this->open(this->id2entity, KVstore::s_id2entity, _mode, buffer_size);
}

bool KVstore::close_id2entity() {
	if (this->id2entity == NULL) {
		return true;
	}
	this->id2entity->save();
	delete this->id2entity;
	this->id2entity = NULL;
	return true;
}

bool KVstore::subEntityByID(int _id) {
	return this->id2entity->remove(_id);
}

string KVstore::getEntityByID(int _id) const {
	char* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->id2entity, _id, _tmp, _len);
	if (!_get) {
		return "";
	}
	string _ret = string(_tmp);
	return _ret;
}

bool KVstore::setEntityByID(int _id, string _entity) {
	return this->addValueByKey(this->id2entity, _id, _entity.c_str(), _entity.length());
}

//for predicate2id
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool KVstore::open_predicate2id(int _mode) {
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_predicate2id_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_predicate2id_query;
	}
	else {
		cerr << "Invalid open mode in open_predicate2id, mode = " << _mode << endl;
		return false;
	}
	return this->open(this->predicate2id, KVstore::s_predicate2id, _mode, buffer_size);
}

bool KVstore::close_predicate2id() {
	if (this->predicate2id == NULL) {
		return true;
	}
	this->predicate2id->save();
	delete this->predicate2id;
	this->predicate2id = NULL;
	return true;
}

bool KVstore::subIDByPredicate(string _predicate) {
	return this->predicate2id->remove(_predicate.c_str(), _predicate.length());
}

int KVstore::getIDByPredicate(string _predicate) const {
	return this->getIDByStr(this->predicate2id, _predicate.c_str(), _predicate.length());
}

bool KVstore::setIDByPredicate(string _predicate, int _id) {
	return this->addValueByKey(this->predicate2id, _predicate.c_str(), _predicate.length(), _id);
}

//for id2predicate
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool KVstore::open_id2predicate(int _mode) {
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2predicate_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2predicate_query;
	}
	else {
		cerr << "Invalid open mode in open_id2predicate, mode = " << _mode << endl;
		return false;
	}
	return this->open(this->id2predicate, KVstore::s_id2predicate, _mode, buffer_size);
}

bool KVstore::close_id2predicate() {
	if (this->id2predicate == NULL) {
		return true;
	}
	this->id2predicate->save();
	delete this->id2predicate;
	this->id2predicate = NULL;
	return true;
}

bool KVstore::subPredicateByID(int _id) {
	return this->id2predicate->remove(_id);
}

string KVstore::getPredicateByID(int _id) const {
	char* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->id2predicate, _id, _tmp, _len);
	if (!_get) {
		return "";
	}
	string _ret = string(_tmp);
	return _ret;
}

bool KVstore::setPredicateByID(int _id, string _predicate) {
	return this->addValueByKey(this->id2predicate, _id, _predicate.c_str(), _predicate.length());
}

//for literal2id
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool KVstore::open_literal2id(int _mode) {
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_literal2id_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_literal2id_query;
	}
	else {
		cerr << "Invalid open mode in open_literal2id, mode = " << _mode << endl;
		return false;
	}
	return this->open(this->literal2id, KVstore::s_literal2id, _mode, buffer_size);
}

bool KVstore::close_literal2id() {
	if (this->literal2id == NULL) {
		return true;
	}
	this->literal2id->save();
	delete this->literal2id;
	this->literal2id = NULL;
	return true;
}

bool KVstore::subIDByLiteral(string _literal) {
	return this->literal2id->remove(_literal.c_str(), _literal.length());
}

int KVstore::getIDByLiteral(string _literal) const {
	return this->getIDByStr(this->literal2id, _literal.c_str(), _literal.length());
}

bool KVstore::setIDByLiteral(string _literal, int _id) {
	return this->addValueByKey(this->literal2id, _literal.c_str(), _literal.length(), _id);
}

//for id2literal
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool KVstore::open_id2literal(int _mode) {
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2literal_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2literal_query;
	}
	else {
		cerr << "Invalid open mode in open_id2literal, mode = " << _mode << endl;
		return false;
	}
	return this->open(this->id2literal, KVstore::s_id2literal, _mode, buffer_size);
}

bool KVstore::close_id2literal() {
	if (this->id2literal == NULL) {
		return true;
	}
	this->id2literal->save();
	delete this->id2literal;
	this->id2literal = NULL;
	return true;
}

bool KVstore::subLiteralByID(int _id) {
	return this->id2literal->remove(_id);
}

string KVstore::getLiteralByID(int _id) const {
	char* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->id2literal, _id, _tmp, _len);
	if (!_get) {
		//NOTICE:here assumes that all literals cannot be empty: ""
		return "";
	}
	string _ret = string(_tmp);
	return _ret;
}

bool KVstore::setLiteralByID(int _id, string _literal) {
	return this->addValueByKey(this->id2literal, _id, _literal.c_str(), _literal.length());
}

bool KVstore::open_subID2values(int _mode) {
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_sID2values_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_sID2values_query;
	}
	else {
		cerr << "Invalid open mode in open_subID2values, mode = " << _mode << endl;
		return false;
	}
	return this->open(this->subID2values, KVstore::s_sID2values, _mode, buffer_size);
}

bool KVstore::close_subID2values() {
	if (this->subID2values == NULL) {
		return true;
	}
	this->subID2values->save();
	delete this->subID2values;
	this->subID2values = NULL;
	return true;
}

bool KVstore::build_subID2values(int** _p_id_tuples, int _triples_num) {
	cout << "Begin building subID2values..." << endl;
	qsort(_p_id_tuples, _triples_num, sizeof(int*), KVstore::_spo_cmp);
	vector<int> _oidlist_s;
	vector<int> _pidoffsetlist_s;
	int _entity_num = 0;

	//true means the next sub is a different one from the current one
	bool _sub_change = true;
	//true means the next <sub,pre> is different from the current pair
	bool _sub_pre_change = true;
	//true means the next pre is different from the current one
	bool _pre_change = true;

	this->open_subID2values(KVstore::CREATE_MODE);

	for (int i = 0; i < _triples_num; i++) {
		if (i + 1 == _triples_num || _p_id_tuples[i][0] != _p_id_tuples[i + 1][0]
			|| _p_id_tuples[i][1] != _p_id_tuples[i + 1][1] || _p_id_tuples[i][2] != _p_id_tuples[i + 1][2]) {
			if (_sub_change) {
				_pidoffsetlist_s.clear();
				_oidlist_s.clear();
				_entity_num = 0;
			}

			int _sub_id = _p_id_tuples[i][0];
			int _pre_id = _p_id_tuples[i][1];
			int _obj_id = _p_id_tuples[i][2];

			if (_sub_pre_change) {
				_pidoffsetlist_s.push_back(_pre_id);
				_pidoffsetlist_s.push_back(_oidlist_s.size());
			}

			_oidlist_s.push_back(_obj_id);
			if (KVstore::isEntity(_obj_id)) {
				_entity_num++;
			}

			_sub_change = (i + 1 == _triples_num) || (_p_id_tuples[i][0] != _p_id_tuples[i + 1][0]);
			_pre_change = (i + 1 == _triples_num) || (_p_id_tuples[i][1] != _p_id_tuples[i + 1][1]);
			_sub_pre_change = _sub_change || _pre_change;

			if (_sub_change) {
				for (unsigned j = 1; j < _pidoffsetlist_s.size(); j += 2) {
					_pidoffsetlist_s[j] += 3 + _pidoffsetlist_s.size();
				}
				int* _entrylist_s = new int[3 + _pidoffsetlist_s.size() + _oidlist_s.size()];
				//triples number
				_entrylist_s[0] = _oidlist_s.size();
				//pre number
				_entrylist_s[1] = _pidoffsetlist_s.size() / 2;
				//entity number
				_entrylist_s[2] = _entity_num;
				unsigned j, k;
				//pidoffsetlist
				for (j = 3, k = 0; k < _pidoffsetlist_s.size(); j++, k++) {
					_entrylist_s[j] = _pidoffsetlist_s[k];
				}
				//unsorted oidlist
				for (k = 0; k < _oidlist_s.size(); j++, k++) {
					_entrylist_s[j] = _oidlist_s[k];
				}

				this->addValueByKey(this->subID2values, _sub_id, (char*)_entrylist_s, sizeof(int) * j);
				delete[] _entrylist_s;
			}
		}
	}

	this->close_subID2values();
	cout << "Finished building subID2values" << endl;
	return true;
}

bool KVstore::getpreIDlistBysubID(int _subid, int*& _preidlist, int& _list_len, bool _no_duplicate) const {
	//cout << "In getpreIDlistBysubID " << _subid << endl;
	if (!isEntity(_subid)) {
		_preidlist = NULL;
		_list_len = 0;
		return false;
	}
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);
	if (!_get) {
		_preidlist = NULL;
		_list_len = 0;
		return false;
	}
	_list_len = _tmp[1];
	_preidlist = new int[_list_len];
	for (int i = 0; i < _list_len; i++) {
		_preidlist[i] = _tmp[2 * i + 3];
	}
	return true;
}

bool KVstore::getobjIDlistBysubID(int _subid, int*& _objidlist, int& _list_len, bool _no_duplicate) const {
	//cout << "In getobjIDlistBysubID " << _subid << endl;
	if (!isEntity(_subid)) {
		_objidlist = NULL;
		_list_len = 0;
		return false;
	}
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);
	if (!_get) {
		_objidlist = NULL;
		_list_len = 0;
		return false;
	}
	_list_len = _tmp[0];
	_objidlist = new int[_list_len];
	memcpy(_objidlist, _tmp + 3 + 2 * _tmp[1], sizeof(int) * _list_len);
	Util::sort(_objidlist, _list_len);
	if (_no_duplicate) {
		_list_len = Util::removeDuplicate(_objidlist, _list_len);
	}
	return true;
}

bool KVstore::getobjIDlistBysubIDpreID(int _subid, int _preid, int*& _objidlist, int& _list_len, bool _no_duplicate) const {
	//cout << "In getobjIDlistBysubIDpreID " << _subid << ' ' << _preid << endl;
	if (!isEntity(_subid)) {
		_objidlist = NULL;
		_list_len = 0;
		return false;
	}
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);
	if (!_get) {
		_objidlist = NULL;
		_list_len = 0;
		return false;
	}
	int _result = KVstore::binarySearch(_preid, _tmp + 3, _tmp[1], 2);
	if (_result == -1) {
		_objidlist = NULL;
		_list_len = 0;
		return false;
	}
	int _offset = _tmp[4 + 2 * _result];
	int _offset_next;
	if (_result == _tmp[1] - 1) {
		_offset_next = 3 + 2 * _tmp[1] + _tmp[0];
	}
	else {
		_offset_next = _tmp[6 + 2 * _result];
	}
	_list_len = _offset_next - _offset;
	_objidlist = new int[_list_len];
	memcpy(_objidlist, _tmp + _offset, sizeof(int) * _list_len);
	return true;
}

bool KVstore::getpreIDobjIDlistBysubID(int _subid, int*& _preid_objidlist, int& _list_len, bool _no_duplicate) const {
	//cout << "In getpreIDobjIDlistBysubID " << _subid << endl;
	if (!isEntity(_subid)) {
		_preid_objidlist = NULL;
		_list_len = 0;
		return false;
	}
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);
	if (!_get) {
		_preid_objidlist = NULL;
		_list_len = 0;
		return false;
	}
	_list_len = 2 * _tmp[0];
	_preid_objidlist = new int[_list_len];
	int _offset_next;
	int j = 0;
	for (int i = 0; i < _tmp[1]; i++) {
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
	return true;
}

bool KVstore::open_objID2values(int _mode) {
	unsigned long long buffer_size;
	if (_mode == KVstore::CREATE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_oID2values_build;
	}
	else if (_mode == KVstore::READ_WRITE_MODE) {
		buffer_size = Util::MAX_BUFFER_SIZE * buffer_oID2values_query;
	}
	else {
		cerr << "Invalid open mode in open_objID2values, mode = " << _mode << endl;
		return false;
	}
	return this->open(this->objID2values, KVstore::s_oID2values, _mode, buffer_size);
}

bool KVstore::close_objID2values() {
	if (this->objID2values == NULL) {
		return true;
	}
	this->objID2values->save();
	delete this->objID2values;
	this->objID2values = NULL;
	return true;
}

bool KVstore::build_objID2values(int** _p_id_tuples, int _triples_num) {
	cout << "Begin building objID2values..." << endl;
	qsort(_p_id_tuples, _triples_num, sizeof(int*), KVstore::_ops_cmp);
	vector<int> _sidlist_o;
	vector<int> _pidoffsetlist_o;

	//true means the next obj is a different one from the current one
	bool _obj_change = true;
	//true means the next <obj, pre> is different from the current pair
	bool _obj_pre_change = true;
	//true means the next pre is different from the current one
	bool _pre_change = true;

	this->open_objID2values(KVstore::CREATE_MODE);

	for (int i = 0; i < _triples_num; i++) {
		if (i + 1 == _triples_num || _p_id_tuples[i][2] != _p_id_tuples[i + 1][2]
			|| _p_id_tuples[i][1] != _p_id_tuples[i + 1][1] || _p_id_tuples[i][0] != _p_id_tuples[i + 1][0]) {
			if (_obj_change) {
				_pidoffsetlist_o.clear();
				_sidlist_o.clear();
			}

			int _sub_id = _p_id_tuples[i][0];
			int _pre_id = _p_id_tuples[i][1];
			int _obj_id = _p_id_tuples[i][2];

			if (_obj_pre_change) {
				_pidoffsetlist_o.push_back(_pre_id);
				_pidoffsetlist_o.push_back(_sidlist_o.size());
			}

			_sidlist_o.push_back(_sub_id);

			_obj_change = (i + 1 == _triples_num) || (_p_id_tuples[i][2] != _p_id_tuples[i + 1][2]);
			_pre_change = (i + 1 == _triples_num) || (_p_id_tuples[i][1] != _p_id_tuples[i + 1][1]);
			_obj_pre_change = _obj_change || _pre_change;

			if (_obj_change) {
				for (unsigned j = 1; j < _pidoffsetlist_o.size(); j += 2) {
					_pidoffsetlist_o[j] += 2 + _pidoffsetlist_o.size();
				}
				int* _entrylist_o = new int[2 + _pidoffsetlist_o.size() + _sidlist_o.size()];
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
				this->addValueByKey(this->objID2values, _obj_id, (char*)_entrylist_o, sizeof(int) * j);
				delete[] _entrylist_o;
			}
		}
	}

	this->close_objID2values();
	cout << "Finished building objID2values" << endl;
	return true;
}

bool KVstore::getpreIDlistByobjID(int _objid, int*& _preidlist, int& _list_len, bool _no_duplicate) const {
	//cout << "In getpreIDlistByobjID " << _objid << endl;
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len);
	if (!_get) {
		_preidlist = NULL;
		_list_len = 0;
		return false;
	}
	_list_len = _tmp[1];
	_preidlist = new int[_list_len];
	for (int i = 0; i < _list_len; i++) {
		_preidlist[i] = _tmp[2 * i + 2];
	}
	return true;
}

bool KVstore::getsubIDlistByobjID(int _objid, int*& _subidlist, int& _list_len, bool _no_duplicate) const {
	//cout << "In getsubIDlistByobjID " << _objid << endl;
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len);
	if (!_get) {
		_subidlist = NULL;
		_list_len = 0;
		return false;
	}
	_list_len = _tmp[0];
	_subidlist = new int[_list_len];
	memcpy(_subidlist, _tmp + 2 + 2 * _tmp[1], sizeof(int) * _list_len);
	Util::sort(_subidlist, _list_len);
	if (_no_duplicate) {
		_list_len = Util::removeDuplicate(_subidlist, _list_len);
	}
	return true;
}

bool KVstore::getsubIDlistByobjIDpreID(int _objid, int _preid, int*& _subidlist, int& _list_len, bool _no_duplicate) const {
	//cout << "In getsubIDlistByobjIDpreID " << _objid << ' ' << _preid << endl;
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len);
	if (!_get) {
		_subidlist = NULL;
		_list_len = 0;
		return false;
	}
	int _result = KVstore::binarySearch(_preid, _tmp + 2, _tmp[1], 2);
	if (_result == -1) {
		_subidlist = NULL;
		_list_len = 0;
		return false;
	}
	int _offset = _tmp[3 + 2 * _result];
	int _offset_next;
	if (_result == _tmp[1] - 1) {
		_offset_next = 2 + 2 * _tmp[1] + _tmp[0];
	}
	else {
		_offset_next = _tmp[5 + 2 * _result];
	}
	_list_len = _offset_next - _offset;
	_subidlist = new int[_list_len];
	memcpy(_subidlist, _tmp + _offset, sizeof(int) * _list_len);
	return true;
}

bool KVstore::getpreIDsubIDlistByobjID(int _objid, int*& _preid_subidlist, int& _list_len, bool _no_duplicate) const {
	//cout << "In getpreIDsubIDlistByobjID " << _objid << endl;
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len);
	if (!_get) {
		_preid_subidlist = NULL;
		_list_len = 0;
		return false;
	}
	_list_len = 2 * _tmp[0];
	_preid_subidlist = new int[_list_len];
	int _offset_next;
	int j = 0;
	for (int i = 0; i < _tmp[1]; i++) {
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
	return true;
}

bool KVstore::open_preID2values(int _mode) {
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
	return this->open(this->preID2values, KVstore::s_pID2values, _mode, buffer_size);
}

bool KVstore::close_preID2values() {
	if (this->preID2values == NULL) {
		return true;
	}
	this->preID2values->save();
	delete this->preID2values;
	this->preID2values = NULL;
	return true;
}

bool KVstore::build_preID2values(int** _p_id_tuples, int _triples_num) {
	cout << "Begin building preID2values..." << endl;
	qsort(_p_id_tuples, _triples_num, sizeof(int*), KVstore::_pso_cmp);
	vector<int> _sidlist_p;
	vector<int> _oidlist_p;

	//true means the next pre is different from the current one
	bool _pre_change = true;

	this->open_preID2values(KVstore::CREATE_MODE);

	for (int i = 0; i < _triples_num; i++) {
		if (i + 1 == _triples_num || _p_id_tuples[i][0] != _p_id_tuples[i + 1][0]
			|| _p_id_tuples[i][1] != _p_id_tuples[i + 1][1] || _p_id_tuples[i][2] != _p_id_tuples[i + 1][2]) {
			if (_pre_change) {
				_sidlist_p.clear();
				_oidlist_p.clear();
			}

			int _sub_id = _p_id_tuples[i][0];
			int _pre_id = _p_id_tuples[i][1];
			int _obj_id = _p_id_tuples[i][2];

			_sidlist_p.push_back(_sub_id);
			_oidlist_p.push_back(_obj_id);

			_pre_change = (i + 1 == _triples_num) || (_p_id_tuples[i][1] != _p_id_tuples[i + 1][1]);

			if (_pre_change) {
				int* _entrylist_p = new int[1 + _sidlist_p.size() * 2];
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
				this->addValueByKey(this->preID2values, _pre_id, (char*)_entrylist_p, sizeof(int) * j);
				delete[] _entrylist_p;
			}
		}
	}

	this->close_preID2values();
	cout << "Finished building preID2values" << endl;
	return true;
}

bool KVstore::getsubIDlistBypreID(int _preid, int*& _subidlist, int& _list_len, bool _no_duplicate) const {
	//cout << "In getsubIDlistBypreID " << _preid << endl;
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->preID2values, _preid, (char*&)_tmp, _len);
	if (!_get) {
		_subidlist = NULL;
		_list_len = 0;
		return false;
	}
	_list_len = _tmp[0];
	_subidlist = new int[_list_len];
	memcpy(_subidlist, _tmp + 1, sizeof(int) * _list_len);
	if (_no_duplicate) {
		_list_len = Util::removeDuplicate(_subidlist, _list_len);
	}
	return true;
}

bool KVstore::getobjIDlistBypreID(int _preid, int*& _objidlist, int& _list_len, bool _no_duplicate) const {
	//cout << "In getobjIDlistBypreID " << _preid << endl;
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->preID2values, _preid, (char*&)_tmp, _len);
	if (!_get) {
		_objidlist = NULL;
		_list_len = 0;
		return false;
	}
	_list_len = _tmp[0];
	_objidlist = new int[_list_len];
	memcpy(_objidlist, _tmp + 1 + _tmp[0], sizeof(int) * _list_len);
	Util::sort(_objidlist, _list_len);
	if (_no_duplicate) {
		_list_len = Util::removeDuplicate(_objidlist, _list_len);
	}
	return true;
}

bool KVstore::getsubIDobjIDlistBypreID(int _preid, int*& _subid_objidlist, int& _list_len, bool _no_duplicate) const {
	//cout << "In getsubIDobjIDlistBypreID " << _preid << endl;
	int* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->preID2values, _preid, (char*&)_tmp, _len);
	if (!_get) {
		_subid_objidlist = NULL;
		_list_len = 0;
		return false;
	}
	_list_len = _tmp[0] * 2;
	_subid_objidlist = new int[_list_len];
	for (int i = 0; i < _tmp[0]; i++) {
		_subid_objidlist[2 * i] = _tmp[1 + i];
		_subid_objidlist[2 * i + 1] = _tmp[1 + _tmp[0] + i];
	}
	return true;
}

bool KVstore::getpreIDlistBysubIDobjID(int _subid, int _objid, int*& _preidlist, int& _list_len, bool _no_duplicate) const {
	//cout << "In getpreIDlistBysubIDobjID " << _subid << ' ' << _objid << endl;
	int *list1 = NULL, *list2 = NULL;
	int len1 = 0, len2 = 0;
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
	vector<int> list = KVstore::intersect(list1, list2, len1, len2);
	delete[] list1;
	delete[] list2;
	int len = list.size();
	if (len == 0) {
		_preidlist = NULL;
		_list_len = 0;
		return false;
	}
	int* _tmp = NULL;
	int _len = 0;
	this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);
	_list_len = len;
	int _result = 0;
	for (int i = 0; i < len; i++) {
		int _preid = list[i];
		for (; _result < _tmp[1]; _result++) {
			if (_tmp[3 + 2 * _result] == _preid) {
				break;
			}
		}
		if (_result == _tmp[1]) {
			for (int j = i; j < len; j++) {
				list[j] = -1;
				_list_len--;
			}
			break;
		}
		int _offset = _tmp[4 + 2 * _result];
		int _offset_next;
		if (_result == _tmp[1] - 1) {
			_offset_next = 3 + 2 * _tmp[1] + _tmp[0];
		}
		else {
			_offset_next = _tmp[6 + 2 * _result];
		}
		if (KVstore::binarySearch(_objid, _tmp + _offset, _offset_next - _offset) == -1) {
			list[i] = -1;
			_list_len--;
		}
	}
	if (_list_len == 0) {
		_preidlist = NULL;
		return false;
	}
	_preidlist = new int[_list_len];
	int i = 0, j = 0;
	while (i < len) {
		if (list[i] != -1) {
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

int KVstore::_spo_cmp(const void* _a, const void* _b) {
	int** _p_a = (int**)_a;
	int** _p_b = (int**)_b;

	int _sub_id_a = (*_p_a)[0];
	int _sub_id_b = (*_p_b)[0];
	if (_sub_id_a != _sub_id_b) {
		return _sub_id_a - _sub_id_b;
	}

	int _pre_id_a = (*_p_a)[1];
	int _pre_id_b = (*_p_b)[1];
	if (_pre_id_a != _pre_id_b) {
		return _pre_id_a - _pre_id_b;
	}

	int _obj_id_a = (*_p_a)[2];
	int _obj_id_b = (*_p_b)[2];
	if (_obj_id_a != _obj_id_b) {
		return _obj_id_a - _obj_id_b;
	}

	return 0;
}

int KVstore::_ops_cmp(const void* _a, const void* _b) {
	int** _p_a = (int**)_a;
	int** _p_b = (int**)_b;

	int _obj_id_a = (*_p_a)[2];
	int _obj_id_b = (*_p_b)[2];
	if (_obj_id_a != _obj_id_b) {
		return _obj_id_a - _obj_id_b;
	}

	int _pre_id_a = (*_p_a)[1];
	int _pre_id_b = (*_p_b)[1];
	if (_pre_id_a != _pre_id_b) {
		return _pre_id_a - _pre_id_b;
	}

	int _sub_id_a = (*_p_a)[0];
	int _sub_id_b = (*_p_b)[0];
	if (_sub_id_a != _sub_id_b) {
		return _sub_id_a - _sub_id_b;
	}

	return 0;
}

int KVstore::_pso_cmp(const void* _a, const void* _b) {
	int** _p_a = (int**)_a;
	int** _p_b = (int**)_b;

	int _pre_id_a = (*_p_a)[1];
	int _pre_id_b = (*_p_b)[1];
	if (_pre_id_a != _pre_id_b) {
		return _pre_id_a - _pre_id_b;
	}

	int _sub_id_a = (*_p_a)[0];
	int _sub_id_b = (*_p_b)[0];
	if (_sub_id_a != _sub_id_b) {
		return _sub_id_a - _sub_id_b;
	}

	int _obj_id_a = (*_p_a)[2];
	int _obj_id_b = (*_p_b)[2];
	if (_obj_id_a != _obj_id_b) {
		return _obj_id_a - _obj_id_b;
	}

	return 0;
}

bool KVstore::open(SITree*& _p_btree, string _tree_name, int _mode, unsigned long long _buffer_size) {
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
	_p_btree = new SITree(this->store_path, _tree_name, smode, _buffer_size);
	return true;
}

bool KVstore::open(ISTree*& _p_btree, string _tree_name, int _mode, unsigned long long _buffer_size) {
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
}

void KVstore::flush(SITree* _p_btree) {
	if (_p_btree != NULL) {
		_p_btree->save();
	}
}

void KVstore::flush(ISTree* _p_btree) {
	if (_p_btree != NULL) {
		_p_btree->save();
	}
}

bool KVstore::addValueByKey(SITree* _p_btree, const char* _key, int _klen, int _val) {
	return _p_btree->insert(_key, _klen, _val);
}

bool KVstore::addValueByKey(ISTree* _p_btree, int _key, const char* _val, int _vlen) {
	return _p_btree->insert(_key, _val, _vlen);
}

bool KVstore::setValueByKey(SITree* _p_btree, const char* _key, int _klen, int _val) {
	return _p_btree->modify(_key, _klen, _val);
}

bool KVstore::setValueByKey(ISTree* _p_btree, int _key, const char* _val, int _vlen) {
	return _p_btree->modify(_key, _val, _vlen);
}

bool KVstore::getValueByKey(SITree* _p_btree, const char* _key, int _klen, int* _val) const {
	return _p_btree->search(_key, _klen, _val);
}

bool KVstore::getValueByKey(ISTree* _p_btree, int _key, char*& _val, int& _vlen) const {
	return _p_btree->search(_key, _val, _vlen);
}

int KVstore::getIDByStr(SITree* _p_btree, const char* _key, int _klen) const {
	int val = 0;
	bool ret = _p_btree->search(_key, _klen, &val);
	if (!ret)
	{
		return -1;
	}
	return val;
}

bool KVstore::removeKey(SITree* _p_btree, const char* _key, int _klen) {
	return _p_btree->remove(_key, _klen);
}

bool KVstore::removeKey(ISTree* _p_btree, int _key) {
	return _p_btree->remove(_key);
}

vector<int> KVstore::intersect(const int* _list1, const int* _list2, int _len1, int _len2) {
	int i = 0, j = 0;
	vector<int> ret;
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

int KVstore::binarySearch(int _key, const int* _list, int _list_len, int _step) {
	int _left = 0;
	int _right = _list_len - 1;
	int _mid;
	while (_left <= _right) {
		_mid = (_right - _left) / 2 + _left;
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
	return -1;
}

bool KVstore::isEntity(int id) {
	return id < Util::LITERAL_FIRST_ID;
}

string KVstore::s_entity2id = "s_entity2id";
string KVstore::s_id2entity = "s_id2entity";
unsigned short KVstore::buffer_entity2id_build = 8;
unsigned short KVstore::buffer_id2entity_build = 8;
unsigned short KVstore::buffer_entity2id_query = 2;
unsigned short KVstore::buffer_id2entity_query = 1;

string KVstore::s_predicate2id = "s_predicate2id";
string KVstore::s_id2predicate = "s_id2predicate";
unsigned short KVstore::buffer_predicate2id_build = 8;
unsigned short KVstore::buffer_id2predicate_build = 8;
unsigned short KVstore::buffer_predicate2id_query = 1;
unsigned short KVstore::buffer_id2predicate_query = 1;

string KVstore::s_literal2id = "s_literal2id";
string KVstore::s_id2literal = "s_id2literal";
unsigned short KVstore::buffer_literal2id_build = 8;
unsigned short KVstore::buffer_id2literal_build = 8;
unsigned short KVstore::buffer_literal2id_query = 2;
unsigned short KVstore::buffer_id2literal_query = 1;

string KVstore::s_sID2values = "s_sID2values";
string KVstore::s_oID2values = "s_oID2values";
string KVstore::s_pID2values = "s_pID2values";
unsigned short KVstore::buffer_sID2values_build = 32;
unsigned short KVstore::buffer_oID2values_build = 32;
unsigned short KVstore::buffer_pID2values_build = 16;
unsigned short KVstore::buffer_sID2values_query = 16;
unsigned short KVstore::buffer_oID2values_query = 16;
unsigned short KVstore::buffer_pID2values_query = 8;
