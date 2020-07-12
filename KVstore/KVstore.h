/*=============================================================================
# Filename:		KVstore.h
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified:	2018-02-09 14:23
# Description: Modified by Wang Libo
=============================================================================*/

#ifndef _KVSTORE_KVSTORE_H
#define _KVSTORE_KVSTORE_H

#include "../Util/Util.h"
#include "../Util/VList.h"
#include "Tree.h"
#include "../Trie/Trie.h"
#include "IVArray/IVArray.h"
#include "ISArray/ISArray.h"

//TODO: is it needed to keep a length in Bstr?? especially for IVTree?
//add a length: sizeof bstr from 8 to 16(4 -> 8 for alignment)
//add a \0 in tail: only add 1 char
//QUERY: but to count the length each time maybe very costly?
//No, because triple num is stored in char* now!!!! we do not need to save it again
//
//QUERY: but to implement vlist, we need a unsigned flag
//What is more, we need to store the string in disk, how can we store it if without the length?
//unsigned type stored as chars, maybe will have '\0'
//In memory, we do not know when the oidlist ends if without the original length (butthe triple num will answer this!)
//
//TODO: entity_border in s2values list is not needed!!! not waste memory here

//STRUCT:
//1. s2xx
//Triple Num   Pre Num   Entity Num   p1 offset1  p2 offset2  ...  pn offsetn (olist-p1) (olist-p2) ... (olist-pn)
//2. o2xx
//Triple Num   Pre Num   p1 offset1  p2 offset2 ... pn offsetn (slist-p1) (slist-p2) ... (slist-pn)
//3. p2xx
//Triple Num   (sid list)  (oid list) (not sorted, matched with sid one by one)

class KVstore {
  public:
  static const int READ_WRITE_MODE = 1; //Open a B tree, which must exist
  static const int CREATE_MODE = 2;     //Build a new B tree and delete existing ones (if any)

  KVstore(std::string _store_path = ".");
  ~KVstore();
  void flush();
  void release();
  void open();

  std::string getStringByID(TYPE_ENTITY_LITERAL_ID _id);
  TYPE_ENTITY_LITERAL_ID getIDByString(std::string _str);

  // functions to load Vlist's cache
  void AddIntoSubCache(TYPE_ENTITY_LITERAL_ID _entity_id);
  void AddIntoObjCache(TYPE_ENTITY_LITERAL_ID _entity_literal_id);
  void AddIntoPreCache(TYPE_PREDICATE_ID _pre_id);
  unsigned getPreListSize(TYPE_PREDICATE_ID _pre_id);
  unsigned getSubListSize(TYPE_ENTITY_LITERAL_ID _sub_id);
  unsigned getObjListSize(TYPE_ENTITY_LITERAL_ID _obj_id);

  //===============================================================================

  //including IN-neighbor & OUT-neighbor
  unsigned getEntityDegree(TYPE_ENTITY_LITERAL_ID _entity_id) const;
  unsigned getEntityInDegree(TYPE_ENTITY_LITERAL_ID _entity_id) const;
  unsigned getEntityOutDegree(TYPE_ENTITY_LITERAL_ID _entity_id) const;

  unsigned getLiteralDegree(TYPE_ENTITY_LITERAL_ID _literal_id) const;
  unsigned getPredicateDegree(TYPE_PREDICATE_ID _predicate_id) const;

  unsigned getSubjectPredicateDegree(TYPE_ENTITY_LITERAL_ID _subid, TYPE_PREDICATE_ID _preid) const;
  unsigned getObjectPredicateDegree(TYPE_ENTITY_LITERAL_ID _objid, TYPE_PREDICATE_ID _preid) const;

  //===============================================================================
  //Before calling these functions, we are sure that the triples doesn't exist.

  bool updateTupleslist_insert(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id);
  bool updateTupleslist_remove(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id);

  bool updateInsert_s2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id);
  bool updateRemove_s2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id);
  bool updateInsert_s2values(TYPE_ENTITY_LITERAL_ID _subid, const std::vector<unsigned>& _pidoidlist);
  bool updateRemove_s2values(TYPE_ENTITY_LITERAL_ID _subid, const std::vector<unsigned>& _pidoidlist);

  bool updateInsert_o2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id);
  bool updateRemove_o2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id);
  bool updateInsert_o2values(TYPE_ENTITY_LITERAL_ID _objid, const std::vector<unsigned>& _pidsidlist);
  bool updateRemove_o2values(TYPE_ENTITY_LITERAL_ID _objid, const std::vector<unsigned>& _pidsidlist);

  bool updateInsert_p2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id);
  bool updateRemove_p2values(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id);
  bool updateInsert_p2values(TYPE_PREDICATE_ID _preid, const std::vector<unsigned>& _sidoidlist);
  bool updateRemove_p2values(TYPE_PREDICATE_ID _preid, const std::vector<unsigned>& _sidoidlist);

  //===============================================================================

  //for entity2id
  bool open_entity2id(int _mode);
  bool close_entity2id();
  bool subIDByEntity(std::string _entity);
  TYPE_ENTITY_LITERAL_ID getIDByEntity(std::string _entity) const;
  bool setIDByEntity(std::string _entity, TYPE_ENTITY_LITERAL_ID _id);

  //for id2entity
  bool open_id2entity(int _mode);
  bool close_id2entity();
  bool subEntityByID(TYPE_ENTITY_LITERAL_ID _id);
  std::string getEntityByID(TYPE_ENTITY_LITERAL_ID _id, bool needUnCompress = true) const;
  bool setEntityByID(TYPE_ENTITY_LITERAL_ID _id, std::string _entity);

  //for predicate2id
  bool open_predicate2id(int _mode);
  bool close_predicate2id();
  bool subIDByPredicate(std::string _predicate);
  TYPE_PREDICATE_ID getIDByPredicate(std::string _predicate) const;
  bool setIDByPredicate(std::string _predicate, TYPE_PREDICATE_ID _id);

  //for id2predicate
  bool open_id2predicate(int _mode);
  bool close_id2predicate();
  bool subPredicateByID(TYPE_PREDICATE_ID _id);
  std::string getPredicateByID(TYPE_PREDICATE_ID _id, bool needUnCompress = true) const;
  bool setPredicateByID(TYPE_PREDICATE_ID _id, std::string _predicate);

  //for literal2id
  bool open_literal2id(int _mode);
  bool close_literal2id();
  bool subIDByLiteral(std::string _literal);
  TYPE_ENTITY_LITERAL_ID getIDByLiteral(std::string _literal) const;
  bool setIDByLiteral(std::string _literal, TYPE_ENTITY_LITERAL_ID _id);

  //for id2literal
  bool open_id2literal(int _mode);
  bool close_id2literal();
  bool subLiteralByID(TYPE_ENTITY_LITERAL_ID _id);
  std::string getLiteralByID(TYPE_ENTITY_LITERAL_ID _id, bool needUnCompress = true) const;
  bool setLiteralByID(TYPE_ENTITY_LITERAL_ID _id, std::string _literal);

  //===============================================================================

  //for subID2values
  bool open_subID2values(int _mode, TYPE_ENTITY_LITERAL_ID _entity_num = 0);
  bool close_subID2values();
  bool build_subID2values(ID_TUPLE* _p_id_tuples, TYPE_TRIPLE_NUM _triples_num, TYPE_ENTITY_LITERAL_ID total_entity_num);
  bool getpreIDlistBysubID(TYPE_ENTITY_LITERAL_ID _subid, unsigned*& _preidlist, unsigned& _list_len, bool _no_duplicate = false) const;
  bool getobjIDlistBysubID(TYPE_ENTITY_LITERAL_ID _subid, unsigned*& _objidlist, unsigned& _list_len, bool _no_duplicate = false) const;
  bool getobjIDlistBysubIDpreID(TYPE_ENTITY_LITERAL_ID _subid, TYPE_PREDICATE_ID _preid, unsigned*& _objidlist, unsigned& _list_len, bool _no_duplicate = false) const;
  bool getpreIDobjIDlistBysubID(TYPE_ENTITY_LITERAL_ID _subid, unsigned*& _preid_objidlist, unsigned& _list_len, bool _no_duplicate = false) const;

  //for objID2values
  bool open_objID2values(int _mode, TYPE_ENTITY_LITERAL_ID _entitynum = 0, TYPE_ENTITY_LITERAL_ID _literal_num = 0);
  bool close_objID2values();
  bool build_objID2values(ID_TUPLE* _p_id_tuples, TYPE_TRIPLE_NUM _triples_num, TYPE_ENTITY_LITERAL_ID total_entity_num, TYPE_ENTITY_LITERAL_ID total_literal_num);
  bool getpreIDlistByobjID(TYPE_ENTITY_LITERAL_ID _objid, unsigned*& _preidlist, unsigned& _list_len, bool _no_duplicate = false) const;
  bool getsubIDlistByobjID(TYPE_ENTITY_LITERAL_ID _objid, unsigned*& _subidlist, unsigned& _list_len, bool _no_duplicate = false) const;
  bool getsubIDlistByobjIDpreID(TYPE_ENTITY_LITERAL_ID _objid, TYPE_PREDICATE_ID _preid, unsigned*& _subidlist, unsigned& _list_len, bool _no_duplicate = false) const;
  bool getpreIDsubIDlistByobjID(TYPE_ENTITY_LITERAL_ID _objid, unsigned*& _preid_subidlist, unsigned& _list_len, bool _no_duplicate = false) const;

  //for preID2values
  bool open_preID2values(int _mode, TYPE_PREDICATE_ID _pre_num = 0);
  bool close_preID2values();
  bool build_preID2values(ID_TUPLE* _p_id_tuples, TYPE_TRIPLE_NUM _triples_num, TYPE_PREDICATE_ID total_pre_num);
  bool getsubIDlistBypreID(TYPE_PREDICATE_ID _preid, unsigned*& _subidlist, unsigned& _list_len, bool _no_duplicate = false) const;
  bool getobjIDlistBypreID(TYPE_PREDICATE_ID _preid, unsigned*& _objidlist, unsigned& _list_len, bool _no_duplicate = false) const;
  bool getsubIDobjIDlistBypreID(TYPE_PREDICATE_ID _preid, unsigned*& _subid_objidlist, unsigned& _list_len, bool _no_duplicate = false) const;

  //for so2p
  bool getpreIDlistBysubIDobjID(TYPE_ENTITY_LITERAL_ID _subID, TYPE_ENTITY_LITERAL_ID _objID, unsigned*& _preidlist, unsigned& _list_len, bool _no_duplicate = false) const;

  bool load_trie(int _mode);

  Trie* getTrie();

  private:
  std::string store_path;

  std::string dictionary_store_path;

  Trie* trie;

  SITree* entity2id;
  //ISTree* id2entity;
  ISArray* id2entity;
  static std::string s_entity2id;
  static std::string s_id2entity;
  static unsigned short buffer_entity2id_build;
  static unsigned short buffer_id2entity_build;
  static unsigned short buffer_entity2id_query;
  static unsigned short buffer_id2entity_query;

  SITree* predicate2id;
  //ISTree* id2predicate;
  ISArray* id2predicate;
  static std::string s_predicate2id;
  static std::string s_id2predicate;
  static unsigned short buffer_predicate2id_build;
  static unsigned short buffer_id2predicate_build;
  static unsigned short buffer_predicate2id_query;
  static unsigned short buffer_id2predicate_query;

  SITree* literal2id;
  //ISTree* id2literal;
  ISArray* id2literal;
  static std::string s_literal2id;
  static std::string s_id2literal;
  static unsigned short buffer_literal2id_build;
  static unsigned short buffer_id2literal_build;
  static unsigned short buffer_literal2id_query;
  static unsigned short buffer_id2literal_query;

  //	IVTree* subID2values;
  //	IVTree* objID2values;
  //	IVTree* preID2values;

  IVArray* subID2values;
  IVArray* objID2values;
  IVArray* objID2values_literal;
  IVArray* preID2values;
  static std::string s_sID2values;
  static std::string s_oID2values;
  static std::string s_pID2values;
  static unsigned short buffer_sID2values_build;
  static unsigned short buffer_oID2values_build;
  static unsigned short buffer_pID2values_build;
  static unsigned short buffer_sID2values_query;
  static unsigned short buffer_oID2values_query;
  static unsigned short buffer_pID2values_query;

  //===============================================================================

  bool open(SITree*& _p_btree, std::string _tree_name, int _mode, unsigned long long _buffer_size);
  //bool open(ISTree* & _p_btree, std::string _tree_name, int _mode, unsigned long long _buffer_size);
  bool open(ISArray*& _array, std::string _name, int _mode, unsigned long long _buffer_size, unsigned _key_num = 0);
  //bool open(IVTree* & _p_btree, std::string _tree_name, int _mode, unsigned long long _buffer_size);
  bool open(IVArray*& _array, std::string _name, int _mode, unsigned long long _buffer_size, unsigned _key_num = 0);

  void flush(SITree* _p_btree);
  //void flush(ISTree* _p_btree);
  void flush(ISArray* _array);
  //void flush(IVTree* _p_btree);
  void flush(IVArray* _array);

  bool addValueByKey(SITree* _p_btree, char* _key, unsigned _klen, unsigned _val);
  //bool addValueByKey(ISTree* _p_btree, unsigned _key, char* _val, unsigned _vlen);
  bool addValueByKey(ISArray* _array, unsigned _key, char* _val, unsigned _vlen);
  //bool addValueByKey(IVTree* _p_btree, unsigned _key, char* _val, unsigned _vlen);
  bool addValueByKey(IVArray* _array, unsigned _key, char* _val, unsigned _vlen);

  bool setValueByKey(SITree* _p_btree, char* _key, unsigned _klen, unsigned _val);
  //bool setValueByKey(ISTree* _p_btree, unsigned _key, char* _val, unsigned _vlen);
  bool setValueByKey(ISArray* _array, unsigned _key, char* _val, unsigned _vlen);
  //	bool setValueByKey(IVTree* _p_btree, unsigned _key, char* _val, unsigned _vlen);
  bool setValueByKey(IVArray* _array, unsigned _key, char* _val, unsigned _vlen);

  bool getValueByKey(SITree* _p_btree, const char* _key, unsigned _klen, unsigned* _val) const;
  //bool getValueByKey(ISTree* _p_btree, unsigned _key, char*& _val, unsigned& _vlen) const;
  bool getValueByKey(ISArray* _array, unsigned _key, char*& _val, unsigned& _vlen) const;
  //	bool getValueByKey(IVTree* _p_btree, unsigned _key, char*& _val, unsigned& _vlen) const;
  bool getValueByKey(IVArray* _array, unsigned _key, char*& _val, unsigned& _vlen) const;

  TYPE_ENTITY_LITERAL_ID getIDByStr(SITree* _p_btree, const char* _key, unsigned _klen) const;

  bool removeKey(SITree* _p_btree, const char* _key, unsigned _klen);
  //bool removeKey(ISTree* _p_btree, unsigned _key);
  bool removeKey(ISArray* _array, unsigned _key);
  //	bool removeKey(IVTree* _p_btree, unsigned _key);
  bool removeKey(IVArray* _array, unsigned _key);

  static std::vector<unsigned> intersect(const unsigned* _list1, const unsigned* _list2, unsigned _len1, unsigned _len2);
  static unsigned binarySearch(unsigned key, const unsigned* _list, unsigned _list_len, int step = 1);
  static bool isEntity(TYPE_ENTITY_LITERAL_ID id);
};

#endif //_KVSTORE_KVSTORE_H
