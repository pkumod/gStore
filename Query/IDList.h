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

class IDList
{
public:
	IDList();
	//inline unsigned getID(unsigned _i) const;

	inline unsigned
  getID(unsigned _i) const
  {
    if (this->size() > _i)
    {
      return this->id_list[_i];
    }

    //return -1;
    return INVALID;
  }

	bool addID(unsigned _id);

	//check whether _id exists in this IDList.
	bool isExistID(unsigned _id) const;
	unsigned size() const;
	bool empty() const;
	const std::vector<unsigned>* getList()const;
	unsigned& operator[] (const unsigned & _i);
	std::string to_str();
	int sort();
	void clear();
	void copy(const std::vector<unsigned>& _new_idlist);
	void copy(const IDList* _new_idlist);
    void reserve(size_t size);
	// intersect/union _id_list to this IDList, note that the two list must be ordered before using these two functions.
	unsigned intersectList(const unsigned* _id_list, unsigned _list_len);
	unsigned intersectList(const IDList&);
	unsigned unionList(const unsigned* _id_list, unsigned _list_len, bool only_literal=false);
	unsigned unionList(const IDList&, bool only_literal=false);
	unsigned bsearch_uporder(unsigned _key);
	static IDList* intersect(const IDList&, const unsigned*, unsigned);
	std::vector<unsigned>::iterator eraseAt(std::vector<unsigned>::iterator  it);
	std::vector<unsigned>::iterator begin(){	return id_list.begin();}
	std::vector<unsigned>::iterator end(){	return id_list.end();}
private:
	std::vector<unsigned> id_list;
	bool erase(unsigned i);
};

using IDCachesSharePtr = std::shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID,std::shared_ptr<IDList>>>;

/**
 * stores child and its predicates
 * extending IDList, each element in IDListWithAppending is (main_key,[attached elements])
 * join operation will join  (1,[2 3]) and (1,[4 5]) to (1,[2 3 4 5])
 */
class IDListWithAppending
{
 public:
  std::shared_ptr<std::map<
      TYPE_ENTITY_LITERAL_ID, // main key
      std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> // attached elements
      >> contents_;

  IDListWithAppending():contents_(std::make_shared<std::remove_reference<decltype(*contents_)>::type>()){
  };
  IDListWithAppending(const IDList &id_list);
  IDListWithAppending(TYPE_ENTITY_LITERAL_ID* id_list, size_t records_num,
                      size_t one_record_len, size_t main_key_position);
  void Init(const TYPE_ENTITY_LITERAL_ID* id_list, size_t records_num,
                    size_t one_record_len, size_t main_key_position);
  void Intersect(const TYPE_ENTITY_LITERAL_ID* id_list, size_t record_num, size_t one_record_len, size_t main_key_position);

  size_t Size(){ return this->contents_->size();}
  bool Empty() {return this->contents_->empty();}
  bool Erase(TYPE_ENTITY_LITERAL_ID main_key){ return this->contents_->erase(main_key);}
  size_t MainKeyNum() {return this->contents_->size();}
};

#endif //_QUERY_IDLIST_H

