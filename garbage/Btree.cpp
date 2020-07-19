/*
 * Btree.cpp
 *
 *  Created on: 2014-4-18
 *      Author: liyouhuan
 */
#include "Btree.h"
Btree::Btree(const string& _store_path, const string& _file_name, const char* _mode)
{
  mode = string(_mode);
  storePath = _store_path;
  fileName = _file_name;
  btree = NULL;

  string _tmp_init = "1234567890";
  this->value_transfer = new bstr(_tmp_init.c_str(), _tmp_init.length());
  this->value_transfer_max_len = _tmp_init.length();

  string _btree_path = _store_path + "/" + fileName;
  bool _open_ok = false;
  if (strcmp(_mode, "r") == 0) { //read-only, btree must exist
    _open_ok = openRead(_btree_path);
  } else if (strcmp(_mode, "rw") == 0) { //read and write, btree must exist
    _open_ok = openReadWrite(_btree_path);
  } else if (strcmp(_mode, "w") == 0) { //build a new btree, delete if exist
    _open_ok = openWrite(_btree_path);
  } else {
    cout << "[" << _mode << "] is error mode when open: " << _btree_path << endl;
  }
  {
    if (_open_ok) {
      btree->log("new Btree\n");
    } else {
      btree->log(("open failed: " + _file_name).c_str());
    }
  }
}
Btree::~Btree()
{
  delete this->value_transfer;
  this->value_transfer_max_len = 0;
  delete btree;
  btree = NULL;
}
bool Btree::insert(const bstr& _key, const bstr& _val)
{
  mleafdata _mdata;
  _mdata.set_sKey_sValue(_key.str, _key.len, _val.str, _val.len);
  return btree->Insert(_mdata);
}
bool Btree::insert(const char* _key, int _klen, const char* _val, int _vlen)
{
  mleafdata _mdata;
  _mdata.mData.sKey = (char*)_key;
  _mdata.mData.mLenKey = _klen;
  _mdata.pVal = new mValue();
  _mdata.pVal->Term[0] = (char*)_val;
  _mdata.pVal->lenTerm[0] = _vlen;

  bool _insert = btree->Insert(_mdata);

  _mdata.mData.sKey = NULL;
  _mdata.mData.mLenKey = 0;
  _mdata.pVal->Term[0] = NULL;
  _mdata.pVal->lenTerm[0] = 0;
  delete _mdata.pVal;
  _mdata.pVal = NULL;

  return _insert;
}
bool Btree::search(const char* _key, int _klen, char*& _val, int& _vlen) const
{
  KeyType _k;
  _k.sKey = (char*)_key;
  _k.mLenKey = _klen;

  mleafdata _mdata;
  /* pVal of _mdata is got by assign_val_ptr,
	 * so in this function, we should not release pVal */
  bool _search = btree->Search(_k, _mdata);
  if (!_search) {
    _val = NULL;
    _vlen = 0;
    _k.sKey = NULL;
    _k.mLenKey = 0;
    _mdata.pVal = NULL;
    return false;
  }
  _vlen = _mdata.pVal->lenTerm[0];
  _val = new char[_vlen + 1];
  memcpy(_val, _mdata.pVal->Term[0], _vlen);
  _val[_vlen] = '\0';
  /* pVal is got by assign_val_ptr,
	 * so here should not release pVal */
  _mdata.pVal = NULL;
  _k.sKey = NULL;
  _k.mLenKey = 0;

  return true;
}

bool Btree::search(const char* _key, int _klen)
{

  KeyType _k;
  _k.sKey = (char*)_key;
  _k.mLenKey = _klen;

  mleafdata _mdata;
  /* pVal of _mdata is got by assign_val_ptr,
	 * so in this function, we should not release pVal */
  bool _search = btree->Search(_k, _mdata);
  if (!_search) {
    _k.sKey = NULL;
    _k.mLenKey = 0;
    return false;
  }

  if (this->value_transfer_max_len < _mdata.pVal->lenTerm[0]) {
    delete[](this->value_transfer);
    this->value_transfer = new bstr(_mdata.pVal->Term[0], _mdata.pVal->lenTerm[0]);
    this->value_transfer_max_len = _mdata.pVal->lenTerm[0];
  } else {
    (this->value_transfer)->len = _mdata.pVal->lenTerm[0];
    memcpy((this->value_transfer)->str, _mdata.pVal->Term[0], _mdata.pVal->lenTerm[0]);
    ((this->value_transfer)->str)[(this->value_transfer)->len] = '\0';
  }

  /* pVal is got by assign_val_ptr,
	 * so here should not release pVal */
  _mdata.pVal = NULL;
  _k.sKey = NULL;
  _k.mLenKey = 0;

  return true;
}

const bstr* Btree::getValueTransfer()
{
  return this->value_transfer;
}
bool Btree::remove(const bstr& _kstr)
{
  KeyType _key;
  _key.set_sKey(_kstr.cstr(), _kstr.len);
  return btree->Delete(_key);
}
bool Btree::remove(const char* _key, int _klen)
{
  KeyType _btree_key;
  _btree_key.set_sKey(_key, _klen);
  return btree->Delete(_btree_key);
}
bool Btree::flush()
{
  if (this->btree == NULL) {
    return false;
  }
  (this->btree)->StoreTree();
  return true;
}
bool Btree::release()
{
  this->flush();
  if (this->btree != NULL) {
    delete btree;
  }

  btree = new BPlusTree((this->getBtreeFilePath()).c_str(), "rw");
  return true;
}
bool Btree::close()
{
  btree->log("before flush\n");
  this->flush();
  btree->log("after flush\n");
  delete btree;
  btree = NULL;
  return true;
}
/*
 * private
 */
bool Btree::deleteifExistPath(const string& _path)
{
  return true;
}
bool Btree::openRead(const string& _btree_path)
{
  {
    if (btree != NULL) {
      cout << "err in openRead btree [" + _btree_path + "]: btree is not NULL" << endl;
      exit(0);
    }
  }
  this->btree = new BPlusTree((_btree_path).c_str(), "open");
  return true;
}
bool Btree::openWrite(const string& _btree_path)
{
  this->btree = new BPlusTree((_btree_path).c_str(), "build");
  return true;
}
bool Btree::openReadWrite(const string& _btree_path)
{
  {
    if (btree != NULL) {
      cout << "err in openReadWrite btree [" + _btree_path + "]: btree is not NULL" << endl;
      exit(0);
    }
  }
  this->btree = new BPlusTree((_btree_path).c_str(), "open");
  return true;
}
