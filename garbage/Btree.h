/*
 * Btree.h
 *
 *  Created on: 2014-4-18
 *      Author: liyouhuan
 */

#ifndef BTREE_H_
#define BTREE_H_
#include "CBtreeH.h"
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;
class bstr {
  public:
  char* str;
  int len;
  bstr(const char* _s, int _len)
  {
    str = new char[_len + 1];
    memcpy(str, _s, _len * sizeof(char));
    str[_len] = '\0';
    len = _len;
  }
  bstr(FILE*& _fp)
  {
    fread((char*)&len, sizeof(int), 1, _fp);
    str = new char[len + 1];
    fread(str, sizeof(char), len, _fp);
    str[len] = '\0';
  }
  void b_write(ofstream& _of)
  {
    _of.write((char*)&len, sizeof(int));
    _of.write(str, len * sizeof(char));
  }
  void b_write(FILE* _fp)
  {
    fwrite((char*)&len, sizeof(int), 1, _fp);
    fwrite(str, sizeof(char), len, _fp);
  }
  const char* cstr() const
  {
    return str;
  }
  ~bstr()
  {
    delete[] str;
  }
};

class Btree {
  public:
  //mode = r w rw
  Btree(const string& _store_path, const string& _file_name, const char* _mode);
  ~Btree();
  bool insert(const bstr& _key, const bstr& _val);
  bool insert(const char* _key, int _klen, const char* _val, int _vlen);
  bool search(const char* _key, int _klen, char*& _val, int& _vlen) const;
  bool search(const char* _key, int _klen);
  const bstr* getValueTransfer();
  bool remove(const bstr& _key);
  bool remove(const char* _key, int _klen);
  bool close();
  bool flush();
  bool release();

  private:
  string getBtreeFilePath()
  {
    return storePath + "/" + fileName;
  }
  bool deleteifExistPath(const string& _path);
  bool openRead(const string& _btree_path);
  bool openWrite(const string& _btree_path);
  bool openReadWrite(const string& _btree_path);

  private:
  BPlusTree* btree;
  string storePath;
  string fileName;
  string mode;
  bstr* value_transfer;
  int value_transfer_max_len;
};

#endif /* BTREE_H_ */
