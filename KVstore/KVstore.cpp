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

  if (trie != NULL) {
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

  if (trie == NULL)
    this->trie = new Trie;

  trie->SetStorePath(dictionary_store_path);
  trie->loadStringPrefix();
}

bool
KVstore::load_trie(int _mode)
{
  if (_mode == KVstore::CREATE_MODE) {
    trie = new Trie;
  } else if (_mode == KVstore::READ_WRITE_MODE) {
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

string
KVstore::getStringByID(TYPE_ENTITY_LITERAL_ID _id)
{
  if (Util::is_entity_ele(_id)) {
    return this->getEntityByID(_id);
  } else {
    return this->getLiteralByID(_id);
  }
}

TYPE_ENTITY_LITERAL_ID
KVstore::getIDByString(string _str)
{
  cout << "KVSTORE::GETIDBYSTRING" << endl;
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

  if (Util::isEntity(_str)) {
    //	string tmp_str = kv_trie->Compress(_str);
    //	cout << "GetIDBYString: " << tmp_str << endl;
    return this->getIDByEntity(_str);
  } else {
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
  unsigned _len = 0;
  bool _get = this->getValueByKey(this->objID2values, _entity_id, (char*&)_tmp, _len);

  int ret = 0;
  if (_get) {
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
  unsigned _len = 0;
  bool _get = this->getValueByKey(this->subID2values, _entity_id, (char*&)_tmp, _len);

  int ret = 0;
  if (_get) {
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
  unsigned _len = 0;
  bool _get = this->getValueByKey(this->objID2values, _literal_id, (char*&)_tmp, _len);

  int ret = 0;
  if (_get) {
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
  unsigned _len = 0;
  bool _get = this->getValueByKey(this->preID2values, _predicate_id, (char*&)_tmp, _len);

  int ret = 0;
  if (_get) {
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
  unsigned _len = 0;
  bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);

  int ret = 0;
  if (_get) {
    unsigned _result = KVstore::binarySearch(_preid, _tmp + 3, _tmp[1], 2);
    //if (_result != -1)
    if (_result != INVALID) {
      int _offset = _tmp[4 + 2 * _result];
      int _offset_next;
      if (_result == _tmp[1] - 1) {
        _offset_next = 3 + 2 * _tmp[1] + _tmp[0];
      } else {
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
  unsigned _len = 0;
  bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len);

  int ret = 0;
  if (_get) {
    unsigned _result = KVstore::binarySearch(_preid, _tmp + 2, _tmp[1], 2);
    //if (_result != -1)
    if (_result != INVALID) {
      int _offset = _tmp[3 + 2 * _result];
      int _offset_next;
      if (_result == _tmp[1] - 1) {
        _offset_next = 2 + 2 * _tmp[1] + _tmp[0];
      } else {
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
KVstore::updateTupleslist_insert(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id)
{
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

bool
KVstore::updateTupleslist_remove(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id)
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
  return this->updateRemove_s2values(_sub_id, _pre_id, _obj_id)
         && this->updateRemove_o2values(_sub_id, _pre_id, _obj_id)
         && this->updateRemove_p2values(_sub_id, _pre_id, _obj_id);
}

bool
KVstore::updateInsert_s2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id)
{
  unsigned* _tmp = NULL;
  unsigned _len = 0;
  bool _get = this->getValueByKey(this->subID2values, _sub_id, (char*&)_tmp, _len);
  bool _is_entity = Util::is_entity_ele(_obj_id);

  //subID doesn't exist
  if (!_get) {
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
  else {
    unsigned* _values;
    unsigned _values_len;
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
      } else {
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
      } else {
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
  delete[] _tmp;
  return true;
}

bool
KVstore::updateRemove_s2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id)
{
  unsigned* _tmp = NULL;
  unsigned _len = 0;
  bool _get = this->getValueByKey(this->subID2values, _sub_id, (char*&)_tmp, _len);
  bool _is_entity = Util::is_entity_ele(_obj_id);

  if (!_get) {
    return false;
  }

  //subID doesn't exist after removal
  if (_tmp[0] == 1) {
    this->removeKey(this->subID2values, _sub_id);
  }
  //subID still exists after removal
  else {
    unsigned* _values;
    unsigned _values_len;
    unsigned _position = KVstore::binarySearch(_pre_id, _tmp + 3, _tmp[1], 2);
    unsigned _oidlen_sp;
    //DEBUG: if _tmp[1] -1 < 0??
    if (_position == _tmp[1] - 1) {
      _oidlen_sp = 3 + 2 * _tmp[1] + _tmp[0] - _tmp[4 + 2 * _position];
    } else {
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
  delete[] _tmp;
  return true;
}

//TODO: TO BE IMPROVED
bool
KVstore::updateInsert_s2values(TYPE_ENTITY_LITERAL_ID _subid, const std::vector<unsigned>& _pidoidlist)
{
  vector<unsigned>::const_iterator iter = _pidoidlist.begin();
  while (iter < _pidoidlist.end()) {
    TYPE_PREDICATE_ID _preid = *iter;
    iter++;
    TYPE_ENTITY_LITERAL_ID _objid = *iter;
    iter++;
    this->updateInsert_s2values(_subid, _preid, _objid);
  }
  return true;
}

//TODO: TO BE IMPROVED
bool
KVstore::updateRemove_s2values(TYPE_ENTITY_LITERAL_ID _subid, const std::vector<unsigned>& _pidoidlist)
{
  vector<unsigned>::const_iterator iter = _pidoidlist.begin();
  while (iter < _pidoidlist.end()) {
    TYPE_PREDICATE_ID _preid = *iter;
    iter++;
    TYPE_ENTITY_LITERAL_ID _objid = *iter;
    iter++;
    this->updateRemove_s2values(_subid, _preid, _objid);
  }
  return true;
}

bool
KVstore::updateInsert_o2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id)
{
  unsigned* _tmp = NULL;
  unsigned _len = 0;
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
    unsigned _values_len;
    unsigned _position = KVstore::binarySearch(_pre_id, _tmp + 2, _tmp[1], 2);

    //preID doesn't exist
    if (_position == INVALID) {
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
      } else {
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
      } else {
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
  delete[] _tmp;
  return true;
}

bool
KVstore::updateRemove_o2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id)
{
  unsigned* _tmp = NULL;
  unsigned _len = 0;
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
    unsigned _values_len;
    unsigned _position = KVstore::binarySearch(_pre_id, _tmp + 2, _tmp[1], 2);
    unsigned _sidlen_op;
    if (_position == _tmp[1] - 1) {
      _sidlen_op = 2 + 2 * _tmp[1] + _tmp[0] - _tmp[3 + 2 * _position];
    } else {
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
  delete[] _tmp;
  return true;
}

//TODO: TO BE IMPROVED
bool
KVstore::updateInsert_o2values(TYPE_ENTITY_LITERAL_ID _objid, const std::vector<unsigned>& _pidsidlist)
{
  vector<unsigned>::const_iterator iter = _pidsidlist.begin();
  while (iter < _pidsidlist.end()) {
    TYPE_PREDICATE_ID _preid = *iter;
    iter++;
    TYPE_ENTITY_LITERAL_ID _subid = *iter;
    iter++;
    this->updateInsert_o2values(_subid, _preid, _objid);
  }
  return true;
}

bool
KVstore::updateRemove_o2values(TYPE_ENTITY_LITERAL_ID _objid, const std::vector<unsigned>& _pidsidlist)
{
  vector<unsigned>::const_iterator iter = _pidsidlist.begin();
  while (iter < _pidsidlist.end()) {
    TYPE_PREDICATE_ID _preid = *iter;
    iter++;
    TYPE_ENTITY_LITERAL_ID _subid = *iter;
    iter++;
    this->updateRemove_o2values(_subid, _preid, _objid);
  }
  return true;
}

bool
KVstore::updateInsert_p2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id)
{
  unsigned* _tmp = NULL;
  unsigned _len = 0;
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
    unsigned _values_len = _len / sizeof(unsigned) + 2;
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
  delete[] _tmp;
  return true;
}

bool
KVstore::updateRemove_p2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id)
{
  unsigned* _tmp = NULL;
  unsigned _len = 0;
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
    unsigned _values_len = _len / sizeof(unsigned) - 2;
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
  delete[] _tmp;
  return true;
}

bool
KVstore::updateInsert_p2values(TYPE_PREDICATE_ID _preid, const std::vector<unsigned>& _sidoidlist)
{
  vector<unsigned>::const_iterator iter = _sidoidlist.begin();
  while (iter < _sidoidlist.end()) {
    TYPE_ENTITY_LITERAL_ID _subid = *iter;
    iter++;
    TYPE_ENTITY_LITERAL_ID _objid = *iter;
    iter++;
    this->updateInsert_p2values(_subid, _preid, _objid);
  }
  return true;
}

bool
KVstore::updateRemove_p2values(TYPE_PREDICATE_ID _preid, const std::vector<unsigned>& _sidoidlist)
{
  vector<unsigned>::const_iterator iter = _sidoidlist.begin();
  while (iter < _sidoidlist.end()) {
    TYPE_ENTITY_LITERAL_ID _subid = *iter;
    iter++;
    TYPE_ENTITY_LITERAL_ID _objid = *iter;
    iter++;
    this->updateRemove_p2values(_subid, _preid, _objid);
  }
  return true;
}

//for entity2id
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_entity2id(int _mode)
{
  unsigned long long buffer_size;
  if (_mode == KVstore::CREATE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_entity2id_build;
  } else if (_mode == KVstore::READ_WRITE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_entity2id_query;
  } else {
    cerr << "Invalid open mode in open_entity2id, mode = " << _mode << endl;
    return false;
  }

  return this->open(this->entity2id, KVstore::s_entity2id, _mode, buffer_size);
}

bool
KVstore::close_entity2id()
{
  if (this->entity2id == NULL) {
    return true;
  }

  this->entity2id->save();
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
  string _entity = trie->Compress(_entity0);
  //return this->entity2id->remove(_entity.c_str(), _entity.length());
  return this->removeKey(this->entity2id, _entity.c_str(), _entity.length());
}

TYPE_ENTITY_LITERAL_ID
KVstore::getIDByEntity(string _entity0) const
{
  //this->load_trie();
  string _entity = trie->Compress(_entity0);
  return this->getIDByStr(this->entity2id, _entity.c_str(), _entity.length());
}

bool
KVstore::setIDByEntity(string _entity0, TYPE_ENTITY_LITERAL_ID _id)
{
  //return this->addValueByKey(this->entity2id, _entity.c_str(), _entity.length(), _id);
  //int len = _entity.length() + 1;

  //this->load_trie();
  string _entity = trie->Compress(_entity0);
  int len = _entity.length();
  char* str = new char[len];

  memcpy(str, _entity.c_str(), len);
  return this->addValueByKey(this->entity2id, str, len, _id);
}

//for id2entity
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_id2entity(int _mode)
{
  unsigned long long buffer_size;
  if (_mode == KVstore::CREATE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2entity_build;
  } else if (_mode == KVstore::READ_WRITE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2entity_query;
  } else {
    cerr << "Invalid open mode in open_id2entity, mode = " << _mode << endl;
    return false;
  }

  return this->open(this->id2entity, KVstore::s_id2entity, _mode, buffer_size);
}

bool
KVstore::close_id2entity()
{

  if (this->id2entity == NULL) {
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
KVstore::getEntityByID(TYPE_ENTITY_LITERAL_ID _id, bool needUnCompress) const
{
  char* _tmp = NULL;
  unsigned _len = 0;

  bool _get = this->getValueByKey(this->id2entity, _id, _tmp, _len);
  if (!_get) {
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
  if (needUnCompress) {
    trie->Uncompress(_tmp, _len, _ret);
  } else
    _ret.assign(_tmp, _len);
  delete[] _tmp;
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
  string _entity = this->trie->Compress(_entity0);

  int len = _entity.length();
  char* str = new char[len];

  memcpy(str, _entity.c_str(), len);

  return this->addValueByKey(this->id2entity, _id, str, len);
}

//for predicate2id
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_predicate2id(int _mode)
{
  unsigned long long buffer_size;
  if (_mode == KVstore::CREATE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_predicate2id_build;
  } else if (_mode == KVstore::READ_WRITE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_predicate2id_query;
  } else {
    cerr << "Invalid open mode in open_predicate2id, mode = " << _mode << endl;
    return false;
  }

  return this->open(this->predicate2id, KVstore::s_predicate2id, _mode, buffer_size);
}

bool
KVstore::close_predicate2id()
{
  if (this->predicate2id == NULL) {
    return true;
  }

  this->predicate2id->save();
  delete this->predicate2id;
  this->predicate2id = NULL;

  return true;
}

bool
KVstore::subIDByPredicate(string _predicate0)
{
  //this->load_trie();
  string _predicate = trie->Compress(_predicate0);
  //return this->predicate2id->remove(_predicate.c_str(), _predicate.length());
  return this->removeKey(this->predicate2id, _predicate.c_str(), _predicate.length());
}

TYPE_PREDICATE_ID
KVstore::getIDByPredicate(string _predicate0) const
{
  //this->load_trie();
  string _predicate = trie->Compress(_predicate0);
  return this->getIDByStr(this->predicate2id, _predicate.c_str(), _predicate.length());
}

bool
KVstore::setIDByPredicate(string _predicate0, TYPE_PREDICATE_ID _id)
{
  //return this->addValueByKey(this->predicate2id, _predicate.c_str(), _predicate.length(), _id);
  //int len = _predicate.length() + 1;
  //this->load_trie();
  string _predicate = trie->Compress(_predicate0);
  char* str = new char[_predicate.length()];
  memcpy(str, _predicate.c_str(), _predicate.length());
  return this->addValueByKey(this->predicate2id, str, _predicate.length(), _id);
}

//for id2predicate
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_id2predicate(int _mode)
{
  unsigned long long buffer_size;
  if (_mode == KVstore::CREATE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2predicate_build;
  } else if (_mode == KVstore::READ_WRITE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2predicate_query;
  } else {
    cerr << "Invalid open mode in open_id2predicate, mode = " << _mode << endl;
    return false;
  }

  return this->open(this->id2predicate, KVstore::s_id2predicate, _mode, buffer_size);
}

bool
KVstore::close_id2predicate()
{
  if (this->id2predicate == NULL) {
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
KVstore::getPredicateByID(TYPE_PREDICATE_ID _id, bool needUnCompress) const
{
  char* _tmp = NULL;
  unsigned _len = 0;

  bool _get = this->getValueByKey(this->id2predicate, _id, _tmp, _len);
  if (!_get) {
    return "";
  }

  string _ret;
  if (needUnCompress) {
    trie->Uncompress(_tmp, _len, _ret);
  } else
    _ret.assign(_tmp, _len);
  delete[] _tmp;
  return _ret;
}

bool
KVstore::setPredicateByID(TYPE_PREDICATE_ID _id, string _predicate0)
{
  //return this->addValueByKey(this->id2predicate, _id, _predicate.c_str(), _predicate.length());
  //int len = _predicate.length() + 1;
  string _predicate = trie->Compress(_predicate0);
  int len = _predicate.length();
  char* str = new char[len];
  memcpy(str, _predicate.c_str(), len);

  return this->addValueByKey(this->id2predicate, _id, str, len);
}

//for literal2id
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_literal2id(int _mode)
{
  unsigned long long buffer_size;
  if (_mode == KVstore::CREATE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_literal2id_build;
  } else if (_mode == KVstore::READ_WRITE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_literal2id_query;
  } else {
    cerr << "Invalid open mode in open_literal2id, mode = " << _mode << endl;
    return false;
  }

  return this->open(this->literal2id, KVstore::s_literal2id, _mode, buffer_size);
}

bool
KVstore::close_literal2id()
{
  if (this->literal2id == NULL) {
    return true;
  }

  this->literal2id->save();
  delete this->literal2id;
  this->literal2id = NULL;

  return true;
}

bool
KVstore::subIDByLiteral(string _literal0)
{
  //this->load_trie();
  string _literal = trie->Compress(_literal0);
  //return this->literal2id->remove(_literal.c_str(), _literal.length());
  return this->removeKey(this->literal2id, _literal.c_str(), _literal.length());
}

TYPE_ENTITY_LITERAL_ID
KVstore::getIDByLiteral(string _literal0) const
{
  //this->load_trie();
  string _literal = trie->Compress(_literal0);
  return this->getIDByStr(this->literal2id, _literal.c_str(), _literal.length());
  //TYPE_ENTITY_LITERAL_ID id = this->getIDByStr(this->literal2id, _literal.c_str(), _literal.length());
  //if(id != INVALID)
  //{
  //id += Util::LITERAL_FIRST_ID;
  //}
}

bool
KVstore::setIDByLiteral(string _literal0, TYPE_ENTITY_LITERAL_ID _id)
{
  //return this->addValueByKey(this->literal2id, _literal.c_str(), _literal.length(), _id);
  //int len = _literal.length() + 1;
  //this->load_trie();

  string _literal = trie->Compress(_literal0);
  int len = _literal.length();
  char* str = new char[len];
  memcpy(str, _literal.c_str(), len);

  return this->addValueByKey(this->literal2id, str, len, _id);
}

//for id2literal
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_id2literal(int _mode)
{
  unsigned long long buffer_size;
  if (_mode == KVstore::CREATE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2literal_build;
  } else if (_mode == KVstore::READ_WRITE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_id2literal_query;
  } else {
    cerr << "Invalid open mode in open_id2literal, mode = " << _mode << endl;
    return false;
  }

  return this->open(this->id2literal, KVstore::s_id2literal, _mode, buffer_size);
}

bool
KVstore::close_id2literal()
{
  if (this->id2literal == NULL) {
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
KVstore::getLiteralByID(TYPE_ENTITY_LITERAL_ID _id, bool needUnCompress) const
{
  char* _tmp = NULL;
  unsigned _len = 0;

  bool _get = this->getValueByKey(this->id2literal, _id, _tmp, _len);
  if (!_get) {
    //NOTICE:here assumes that all literals cannot be empty: ""
    return "";
  }
  string _ret;
  if (needUnCompress) {
    trie->Uncompress(_tmp, _len, _ret);
  } else
    _ret.assign(_tmp, _len);
  delete[] _tmp;
  return _ret;
}

bool
KVstore::setLiteralByID(TYPE_ENTITY_LITERAL_ID _id, string _literal0)
{
  //return this->addValueByKey(this->id2literal, _id, _literal.c_str(), _literal.length());
  //int len = _literal.length() + 1;
  string _literal = trie->Compress(_literal0);
  int len = _literal.length();
  char* str = new char[len];
  memcpy(str, _literal.c_str(), len);

  return this->addValueByKey(this->id2literal, _id, str, len);
}

bool
KVstore::open_subID2values(int _mode, TYPE_ENTITY_LITERAL_ID _entity_num)
{
  unsigned long long buffer_size;
  if (_mode == KVstore::CREATE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_sID2values_build;
  } else if (_mode == KVstore::READ_WRITE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_sID2values_query;
  } else {
    cerr << "Invalid open mode in open_subID2values, mode = " << _mode << endl;
    return false;
  }

  return this->open(this->subID2values, KVstore::s_sID2values, _mode, buffer_size, _entity_num);
}

bool
KVstore::close_subID2values()
{
  if (this->subID2values == NULL) {
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
  for (TYPE_TRIPLE_NUM i = 0; i < _triples_num; i++) {
    if (i + 1 == _triples_num || _p_id_tuples[i].subid != _p_id_tuples[i + 1].subid
        || _p_id_tuples[i].preid != _p_id_tuples[i + 1].preid || _p_id_tuples[i].objid != _p_id_tuples[i + 1].objid) {
      if (_sub_change) {
        _pidoffsetlist_s.clear();
        _oidlist_s.clear();
        _entity_num = 0;
      }

      TYPE_ENTITY_LITERAL_ID _sub_id = _p_id_tuples[i].subid;
      TYPE_PREDICATE_ID _pre_id = _p_id_tuples[i].preid;
      TYPE_ENTITY_LITERAL_ID _obj_id = _p_id_tuples[i].objid;

      if (_sub_pre_change) {
        _pidoffsetlist_s.push_back(_pre_id);
        _pidoffsetlist_s.push_back(_oidlist_s.size());
      }

      _oidlist_s.push_back(_obj_id);
      if (Util::is_entity_ele(_obj_id)) {
        _entity_num++;
      }

      _sub_change = (i + 1 == _triples_num) || (_p_id_tuples[i].subid != _p_id_tuples[i + 1].subid);
      _pre_change = (i + 1 == _triples_num) || (_p_id_tuples[i].preid != _p_id_tuples[i + 1].preid);
      _sub_pre_change = _sub_change || _pre_change;

      if (_sub_change) {
        for (unsigned j = 1; j < _pidoffsetlist_s.size(); j += 2) {
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
        for (j = 3, k = 0; k < _pidoffsetlist_s.size(); j++, k++) {
          _entrylist_s[j] = _pidoffsetlist_s[k];
        }
        //unsorted oidlist
        for (k = 0; k < _oidlist_s.size(); j++, k++) {
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
KVstore::getpreIDlistBysubID(TYPE_ENTITY_LITERAL_ID _subid, unsigned*& _preidlist, unsigned& _list_len, bool _no_duplicate) const
{
  //cout << "In getpreIDlistBysubID " << _subid << endl;
  if (!Util::is_entity_ele(_subid)) {
    _preidlist = NULL;
    _list_len = 0;
    return false;
  }
  unsigned* _tmp = NULL;
  unsigned _len = 0;
  bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);

  if (!_get) {
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
  //	}

  return true;
}

bool
KVstore::getobjIDlistBysubID(TYPE_ENTITY_LITERAL_ID _subid, unsigned*& _objidlist, unsigned& _list_len, bool _no_duplicate) const
{
  //cout << "In getobjIDlistBysubID " << _subid << endl;
  if (!Util::is_entity_ele(_subid)) {
    _objidlist = NULL;
    _list_len = 0;
    return false;
  }
  unsigned* _tmp = NULL;
  unsigned _len = 0;
  bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);
  if (!_get) {
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
  //	if(VList::listNeedDelete(_len))
  //	{
  delete[] _tmp;
  //_tmp = NULL;
  //	}

  return true;
}

bool
KVstore::getobjIDlistBysubIDpreID(TYPE_ENTITY_LITERAL_ID _subid, TYPE_PREDICATE_ID _preid, unsigned*& _objidlist, unsigned& _list_len, bool _no_duplicate) const
{
  //cout << "In getobjIDlistBysubIDpreID " << _subid << ' ' << _preid << endl;
  if (!Util::is_entity_ele(_subid)) {
    _objidlist = NULL;
    _list_len = 0;
    return false;
  }
  unsigned* _tmp = NULL;

  unsigned _len = 0;
  bool _get = this->getValueByKey(this->subID2values, _subid, (char*&)_tmp, _len);
  if (!_get) {
    _objidlist = NULL;
    _list_len = 0;
    return false;
  }
  //cout<<"check "<<_subid<<" "<<_preid<<endl;
  unsigned _result = KVstore::binarySearch(_preid, _tmp + 3, _tmp[1], 2);
  //if (_result == -1)
  if (_result == INVALID) {
    _objidlist = NULL;
    _list_len = 0;
    return false;
  }
  unsigned _offset = _tmp[4 + 2 * _result];
  unsigned _offset_next;
  if (_result == _tmp[1] - 1) {
    _offset_next = 3 + 2 * _tmp[1] + _tmp[0];
  } else {
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

bool
KVstore::getpreIDobjIDlistBysubID(TYPE_ENTITY_LITERAL_ID _subid, unsigned*& _preid_objidlist, unsigned& _list_len, bool _no_duplicate) const
{
  //cout << "In getpreIDobjIDlistBysubID " << _subid << endl;
  if (!Util::is_entity_ele(_subid)) {
    _preid_objidlist = NULL;
    _list_len = 0;
    return false;
  }
  unsigned* _tmp = NULL;

  unsigned _len = 0;
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
    } else {
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

bool
KVstore::open_objID2values(int _mode, TYPE_ENTITY_LITERAL_ID _entity_num, TYPE_ENTITY_LITERAL_ID _literal_num)
{
  unsigned long long buffer_size;
  if (_mode == KVstore::CREATE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_oID2values_build;
  } else if (_mode == KVstore::READ_WRITE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_oID2values_query;
  } else {
    cerr << "Invalid open mode in open_objID2values, mode = " << _mode << endl;
    return false;
  }

  return this->open(this->objID2values, KVstore::s_oID2values, _mode, buffer_size, _entity_num) && this->open(this->objID2values_literal, KVstore::s_oID2values + "_literal", _mode, buffer_size, _literal_num);
}

bool
KVstore::close_objID2values()
{
  if (this->objID2values == NULL && this->objID2values_literal == NULL) {
    return true;
  }

  if (this->objID2values != NULL) {
    this->objID2values->save();
    delete this->objID2values;
    this->objID2values = NULL;
  }
  if (this->objID2values_literal != NULL) {
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
  for (TYPE_TRIPLE_NUM i = 0; i < _triples_num; i++) {
    if (i + 1 == _triples_num || _p_id_tuples[i].subid != _p_id_tuples[i + 1].subid
        || _p_id_tuples[i].preid != _p_id_tuples[i + 1].preid || _p_id_tuples[i].objid != _p_id_tuples[i + 1].objid) {
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

      _obj_change = (i + 1 == _triples_num) || (_p_id_tuples[i].objid != _p_id_tuples[i + 1].objid);
      _pre_change = (i + 1 == _triples_num) || (_p_id_tuples[i].preid != _p_id_tuples[i + 1].preid);
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
KVstore::getpreIDlistByobjID(TYPE_ENTITY_LITERAL_ID _objid, unsigned*& _preidlist, unsigned& _list_len, bool _no_duplicate) const
{
  //cout << "In getpreIDlistByobjID " << _objid << endl;
  unsigned* _tmp = NULL;
  unsigned _len = 0;
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

bool
KVstore::getsubIDlistByobjID(TYPE_ENTITY_LITERAL_ID _objid, unsigned*& _subidlist, unsigned& _list_len, bool _no_duplicate) const
{
  //cout << "In getsubIDlistByobjID " << _objid << endl;
  unsigned* _tmp = NULL;
  unsigned _len = 0;
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

bool
KVstore::getsubIDlistByobjIDpreID(TYPE_ENTITY_LITERAL_ID _objid, TYPE_PREDICATE_ID _preid, unsigned*& _subidlist, unsigned& _list_len, bool _no_duplicate) const
{
  //cout << "In getsubIDlistByobjIDpreID " << _objid << ' ' << _preid << endl;
  unsigned* _tmp = NULL;
  unsigned _len = 0;
  bool _get = this->getValueByKey(this->objID2values, _objid, (char*&)_tmp, _len);
  if (!_get) {
    _subidlist = NULL;
    _list_len = 0;
    return false;
  }
  unsigned _result = KVstore::binarySearch(_preid, _tmp + 2, _tmp[1], 2);
  //if (_result == -1)
  if (_result == INVALID) {
    _subidlist = NULL;
    _list_len = 0;
    return false;
  }
  unsigned _offset = _tmp[3 + 2 * _result];
  unsigned _offset_next;

  if (_result == _tmp[1] - 1) {
    _offset_next = 2 + 2 * _tmp[1] + _tmp[0];
  } else {
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

bool
KVstore::getpreIDsubIDlistByobjID(TYPE_ENTITY_LITERAL_ID _objid, unsigned*& _preid_subidlist, unsigned& _list_len, bool _no_duplicate) const
{
  //cout << "In getpreIDsubIDlistByobjID " << _objid << endl;
  unsigned* _tmp = NULL;
  unsigned _len = 0;
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
    } else {
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

bool
KVstore::open_preID2values(int _mode, TYPE_PREDICATE_ID _pre_num)
{
  unsigned long long buffer_size;
  if (_mode == KVstore::CREATE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_pID2values_build;
  } else if (_mode == KVstore::READ_WRITE_MODE) {
    buffer_size = Util::MAX_BUFFER_SIZE * buffer_pID2values_query;
  } else {
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
  for (TYPE_TRIPLE_NUM i = 0; i < _triples_num; i++) {
    if (i + 1 == _triples_num || _p_id_tuples[i].subid != _p_id_tuples[i + 1].subid
        || _p_id_tuples[i].preid != _p_id_tuples[i + 1].preid || _p_id_tuples[i].objid != _p_id_tuples[i + 1].objid) {
      if (_pre_change) {
        _sidlist_p.clear();
        _oidlist_p.clear();
      }

      TYPE_ENTITY_LITERAL_ID _sub_id = _p_id_tuples[i].subid;
      TYPE_PREDICATE_ID _pre_id = _p_id_tuples[i].preid;
      TYPE_ENTITY_LITERAL_ID _obj_id = _p_id_tuples[i].objid;

      _sidlist_p.push_back(_sub_id);
      _oidlist_p.push_back(_obj_id);

      _pre_change = (i + 1 == _triples_num) || (_p_id_tuples[i].preid != _p_id_tuples[i + 1].preid);

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
KVstore::getsubIDlistBypreID(TYPE_PREDICATE_ID _preid, unsigned*& _subidlist, unsigned& _list_len, bool _no_duplicate) const
{
  //cout << "In getsubIDlistBypreID " << _preid << endl;
  unsigned* _tmp = NULL;
  unsigned _len = 0;
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

bool
KVstore::getobjIDlistBypreID(TYPE_PREDICATE_ID _preid, unsigned*& _objidlist, unsigned& _list_len, bool _no_duplicate) const
{
  //cout << "In getobjIDlistBypreID " << _preid << endl;
  unsigned* _tmp = NULL;
  unsigned _len = 0;
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

bool
KVstore::getsubIDobjIDlistBypreID(TYPE_PREDICATE_ID _preid, unsigned*& _subid_objidlist, unsigned& _list_len, bool _no_duplicate) const
{
#ifdef DEBUG_KVSTORE
  cout << "In getsubIDobjIDlistBypreID " << _preid << endl;
#endif
  unsigned* _tmp = NULL;
  unsigned _len = 0;
  bool _get = this->getValueByKey(this->preID2values, _preid, (char*&)_tmp, _len);
#ifdef DEBUG_KVSTORE
  cout << "the length of list: " << _len << endl;
#endif
  if (!_get) {
    _subid_objidlist = NULL;
    _list_len = 0;
    return false;
  }
  _list_len = _tmp[0] * 2;
  _subid_objidlist = new unsigned[_list_len];
  for (unsigned i = 0; i < _tmp[0]; i++) {
    _subid_objidlist[2 * i] = _tmp[1 + i];
    _subid_objidlist[2 * i + 1] = _tmp[1 + _tmp[0] + i];
#ifdef DEBUG_KVSTORE
    cout << _subid_objidlist[2 * i] << " " << _subid_objidlist[2 * i + 1] << endl;
#endif
  }

//if this is a long list, then we should remove itself after copying
//otherwise, we should not free the list memory
//	if(VList::listNeedDelete(_len))
//	{
#ifdef DEBUG_KVSTORE
  cout << "this is a vlist" << endl;
#endif
  delete[] _tmp;
  //_tmp = NULL;
  //	}

  return true;
}

bool
KVstore::getpreIDlistBysubIDobjID(TYPE_ENTITY_LITERAL_ID _subid, TYPE_ENTITY_LITERAL_ID _objid, unsigned*& _preidlist, unsigned& _list_len, bool _no_duplicate) const
{
  //cout << "In getpreIDlistBysubIDobjID " << _subid << ' ' << _objid << endl;
  unsigned* list1 = NULL, *list2 = NULL;
  unsigned len1 = 0, len2 = 0;
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
  unsigned _len = 0;
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
    } else {
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
    if (list[i] != INVALID) {
      _preidlist[j] = list[i];
      i++;
      j++;
    } else {
      i++;
    }
  }

  return true;
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
  } else if (_mode == KVstore::READ_WRITE_MODE) {
    smode = "open";
  } else {
    cerr << "Invalid open mode of: " << _tree_name << " mode = " << _mode << endl;
    return false;
  }
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
  } else if (_mode == KVstore::READ_WRITE_MODE) {
    smode = "open";
  } else {
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
  if (_array != NULL) {
    return false;
  }
  string smode;
  if (_mode == KVstore::CREATE_MODE) {
    smode = "build";
  } else if (_mode == KVstore::READ_WRITE_MODE) {
    smode = "open";
  } else {
    cerr << "Invalid open mode of: " << _name << " mode = " << _mode << endl;
    return false;
  }
  _array = new IVArray(this->store_path, _name, smode, _buffer_size, _key_num);

  return true;
}

void
KVstore::flush(SITree* _p_btree)
{
  if (_p_btree != NULL) {
    _p_btree->save();
  }
}

/*void 
KVstore::flush(ISTree* _p_btree) 
{
	if (_p_btree != NULL) 
	{
		_p_btree->save();
	}
}*/

void
KVstore::flush(ISArray* _array)
{
  if (_array != NULL) {
    _array->save();
  }
}

/*void 
KVstore::flush(IVTree* _p_btree) 
{
	if (_p_btree != NULL) 
	{
		_p_btree->save();
	}
}*/

void
KVstore::flush(IVArray* _array)
{
  if (_array != NULL) {
    _array->save();
  }
}

bool
KVstore::addValueByKey(SITree* _p_btree, char* _key, unsigned _klen, unsigned _val)
{
  return _p_btree->insert(_key, _klen, _val);
}

/*bool 
KVstore::addValueByKey(ISTree* _p_btree, unsigned _key, char* _val, unsigned _vlen) 
{
	return _p_btree->insert(_key, _val, _vlen);
}*/

bool
KVstore::addValueByKey(ISArray* _array, unsigned _key, char* _val, unsigned _vlen)
{
  if (_array == this->id2literal) {
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
KVstore::addValueByKey(IVArray* _array, unsigned _key, char* _val, unsigned _vlen)
{
  if (Util::is_literal_ele(_key) && _array == objID2values) {
    unsigned key = _key - Util::LITERAL_FIRST_ID;
    return objID2values_literal->insert(key, _val, _vlen);
  }
  return _array->insert(_key, _val, _vlen);
}

bool
KVstore::setValueByKey(SITree* _p_btree, char* _key, unsigned _klen, unsigned _val)
{
  return _p_btree->modify(_key, _klen, _val);
}

/*bool 
KVstore::setValueByKey(ISTree* _p_btree, unsigned _key, char* _val, unsigned _vlen) 
{
	return _p_btree->modify(_key, _val, _vlen);
}*/

bool
KVstore::setValueByKey(ISArray* _array, unsigned _key, char* _val, unsigned _vlen)
{
  if (_array == this->id2literal) {
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
KVstore::setValueByKey(IVArray* _array, unsigned _key, char* _val, unsigned _vlen)
{
  if (Util::is_literal_ele(_key) && _array == objID2values) {
    unsigned key = _key - Util::LITERAL_FIRST_ID;
    return objID2values_literal->modify(key, _val, _vlen);
  }
  return _array->modify(_key, _val, _vlen);
}

bool
KVstore::getValueByKey(SITree* _p_btree, const char* _key, unsigned _klen, unsigned* _val) const
{
  return _p_btree->search(_key, _klen, _val);
}

/*bool 
KVstore::getValueByKey(ISTree* _p_btree, unsigned _key, char*& _val, unsigned& _vlen) const 
{
	return _p_btree->search(_key, _val, _vlen);
}*/

bool
KVstore::getValueByKey(ISArray* _array, unsigned _key, char*& _val, unsigned& _vlen) const
{
  if (_array == this->id2literal) {
    unsigned key = _key - Util::LITERAL_FIRST_ID;
    return _array->search(key, _val, _vlen);
  }

  return _array->search(_key, _val, _vlen);
}

/*bool 
KVstore::getValueByKey(IVTree* _p_btree, unsigned _key, char*& _val, unsigned& _vlen) const 
{
	return _p_btree->search(_key, _val, _vlen);
}*/

bool
KVstore::getValueByKey(IVArray* _array, unsigned _key, char*& _val, unsigned& _vlen) const
{
  if (Util::is_literal_ele(_key) && _array == objID2values) {
    unsigned key = _key - Util::LITERAL_FIRST_ID;
    return objID2values_literal->search(key, _val, _vlen);
  }
  return _array->search(_key, _val, _vlen);
}

TYPE_ENTITY_LITERAL_ID
KVstore::getIDByStr(SITree* _p_btree, const char* _key, unsigned _klen) const
{
  unsigned val = 0;
  bool ret = _p_btree->search(_key, _klen, &val);
  if (!ret) {
    //return -1;
    return INVALID;
  }

  return val;
}

bool
KVstore::removeKey(SITree* _p_btree, const char* _key, unsigned _klen)
{
  return _p_btree->remove(_key, _klen);
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
  if (_array == this->id2literal) {
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
  if (Util::is_literal_ele(_key) && _array == objID2values) {
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
    } else if (_list1[i] > _list2[j]) {
      j++;
    } else {
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
    } else {
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
  if (Util::is_literal_ele(_entity_literal_id)) {
    TYPE_ENTITY_LITERAL_ID _literal_id = _entity_literal_id
                                         - Util::LITERAL_FIRST_ID;

    objID2values_literal->PinCache(_literal_id);
  }
  this->objID2values->PinCache(_entity_literal_id);
}

unsigned
KVstore::getSubListSize(TYPE_ENTITY_LITERAL_ID _sub_id)
{
  unsigned* _tmp = NULL;
  unsigned _ret;
  this->getValueByKey(this->subID2values, _sub_id, (char*&)_tmp, _ret);
  delete[] _tmp;
  return _ret;
}

unsigned
KVstore::getObjListSize(TYPE_ENTITY_LITERAL_ID _obj_id)
{
  unsigned* _tmp = NULL;
  unsigned _ret;
  this->getValueByKey(this->objID2values, _obj_id, (char*&)_tmp, _ret);
  delete[] _tmp;
  return _ret;
}

unsigned
KVstore::getPreListSize(TYPE_PREDICATE_ID _pre_id)
{
  unsigned* _tmp = NULL;
  unsigned _ret;
  this->getValueByKey(this->preID2values, _pre_id, (char*&)_tmp, _ret);
  delete[] _tmp;
  return _ret;
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
