/*=======================================================================
 * File name: IVEntry.h
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last Modified: 2018-01-30
 * Description: head file of IVEntry, the most basic unit of IVArray
 * =====================================================================*/

#include "../../Util/Util.h"
#include "../../Util/VList.h"

using namespace std;

class IVEntry {
  bool usedFlag; // mark if the entry is used
  bool dirtyFlag;
  bool cacheFlag;
  bool CachePinFlag;
  unsigned store; //index of block where value is stored
  // pointer to id for LRU list
  int prevID;
  int nextID;

  Bstr* value;

  public:
  IVEntry();

  void setBstr(const Bstr* _value);
  bool getBstr(char*& _str, unsigned& _len, bool if_copy = true) const;
  void setBstr(char* _str, unsigned _len);

  void setStore(unsigned _store);
  unsigned getStore() const;

  void setUsedFlag(bool _flag);
  bool isUsed() const;

  void setDirtyFlag(bool _flag);
  bool isDirty() const;

  void setCacheFlag(bool _flag);
  bool inCache() const;

  void setCachePinFlag(bool _flag);
  bool isPined();

  void release();

  void Copy(const IVEntry& _entry);

  void setPrev(int ID);
  int getPrev() const;
  void setNext(int ID);
  int getNext() const;

  ~IVEntry();
};
