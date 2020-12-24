/*=============================================================================
# Filename: IDList.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-23 15:03
# Description: originally written by liyouhuan, modified by zengli 
=============================================================================*/

#include "../Util/Util.h"

#ifndef _QUERY_IDLIST_H
#define _QUERY_IDLIST_H

class IDList {
  public:
  IDList();
  unsigned getID(unsigned _i) const;
  bool addID(unsigned _id);

  //check whether _id exists in this IDList.
  bool isExistID(unsigned _id) const;
  unsigned size() const;
  bool empty() const;
  const std::vector<unsigned>* getList() const;
  unsigned& operator[](const unsigned& _i);
  std::string to_str();
  int sort();
  void clear();
  void copy(const std::vector<unsigned>& _new_idlist);
  void copy(const IDList* _new_idlist);

  // intersect/union _id_list to this IDList, note that the two list must be ordered before using these two functions.
  unsigned intersectList(const unsigned* _id_list, unsigned _list_len);
  unsigned intersectList(const IDList&);
  unsigned unionList(const unsigned* _id_list, unsigned _list_len, bool only_literal = false);
  unsigned unionList(const IDList&, bool only_literal = false);
  unsigned bsearch_uporder(unsigned _key);
  static IDList* intersect(const IDList&, const unsigned*, unsigned);
  std::vector<unsigned>::iterator eraseAt(std::vector<unsigned>::iterator it);
  std::vector<unsigned>::iterator begin()
  {
    return id_list.begin();
  }
  std::vector<unsigned>::iterator end()
  {
    return id_list.end();
  }

  private:
  std::vector<unsigned> id_list;
  bool erase(unsigned i);
};

#endif //_QUERY_IDLIST_H
