/*=============================================================================
# Filename:		KVstore.h
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified:	2015-10-23 14:23
# Description: Modified by Wang Libo
=============================================================================*/

#ifndef _KVSTORE_KVSTORE_H
#define _KVSTORE_KVSTORE_H

#include "../Util/Util.h"
#include "../Util/VList.h"
#include "Tree.h"

//TODO: is it needed to keep a length in Bstr?? especially for IVTree?
//add a length: sizeof bstr from 8 to 16(4 -> 8 for alignment)
//add a \0 in tail: only add 1 char
//QUERY: but to count the length each time maybe very costly?
//No, because triple num is stored in char* now!!!! we do not need to save it again
//TODO: entity_border in s2values list is not needed!!! not waste memory here

class KVstore
{
public:
	static const int READ_WRITE_MODE = 1;	//Open a B tree, which must exist
	static const int CREATE_MODE = 2;		//Build a new B tree and delete existing ones (if any)

	KVstore(std::string _store_path = ".");
	~KVstore();
	void flush();
	void release();
	void open();

	//===============================================================================

	//including IN-neighbor & OUT-neighbor 
	int getEntityDegree(int _entity_id) const;
	int getEntityInDegree(int _entity_id) const;
	int getEntityOutDegree(int _entity_id) const;

	int getLiteralDegree(int _literal_id) const;
	int getPredicateDegree(int _predicate_id) const;

	int getSubjectPredicateDegree(int _subid, int _preid) const;
	int getObjectPredicateDegree(int _objid, int _preid) const;

	//===============================================================================
	//Before calling these functions, we are sure that the triples doesn't exist.

	bool updateTupleslist_insert(int _sub_id, int _pre_id, int _obj_id);
	bool updateTupleslist_remove(int _sub_id, int _pre_id, int _obj_id);

	bool updateInsert_s2values(int _sub_id, int _pre_id, int _obj_id);
	bool updateRemove_s2values(int _sub_id, int _pre_id, int _obj_id);
	bool updateInsert_s2values(int _subid, const std::vector<int>& _pidoidlist);
	bool updateRemove_s2values(int _subid, const std::vector<int>& _pidoidlist);

	bool updateInsert_o2values(int _sub_id, int _pre_id, int _obj_id);
	bool updateRemove_o2values(int _sub_id, int _pre_id, int _obj_id);
	bool updateInsert_o2values(int _objid, const std::vector<int>& _pidsidlist);
	bool updateRemove_o2values(int _objid, const std::vector<int>& _pidsidlist);

	bool updateInsert_p2values(int _sub_id, int _pre_id, int _obj_id);
	bool updateRemove_p2values(int _sub_id, int _pre_id, int _obj_id);
	bool updateInsert_p2values(int _preid, const std::vector<int>& _sidoidlist);
	bool updateRemove_p2values(int _preid, const std::vector<int>& _sidoidlist);

	//===============================================================================

	//for entity2id 
	bool open_entity2id(int _mode);
	bool close_entity2id();
	bool subIDByEntity(std::string _entity);
	int getIDByEntity(std::string _entity) const;
	bool setIDByEntity(std::string _entity, int _id);

	//for id2entity 
	bool open_id2entity(int _mode);
	bool close_id2entity();
	bool subEntityByID(int _id);
	std::string getEntityByID(int _id) const;
	bool setEntityByID(int _id, std::string _entity);

	//for predicate2id 
	bool open_predicate2id(int _mode);
	bool close_predicate2id();
	bool subIDByPredicate(std::string _predicate);
	int getIDByPredicate(std::string _predicate) const;
	bool setIDByPredicate(std::string _predicate, int _id);

	//for id2predicate 
	bool open_id2predicate(int _mode);
	bool close_id2predicate();
	bool subPredicateByID(int _id);
	std::string getPredicateByID(int _id) const;
	bool setPredicateByID(int _id, std::string _predicate);

	//for literal2id 
	bool open_literal2id(int _mode);
	bool close_literal2id();
	bool subIDByLiteral(std::string _literal);
	int getIDByLiteral(std::string _literal) const;
	bool setIDByLiteral(std::string _literal, int _id);

	//for id2literal 
	bool open_id2literal(int _mode);
	bool close_id2literal();
	bool subLiteralByID(int _id);
	std::string getLiteralByID(int _id) const;
	bool setLiteralByID(int _id, std::string _literal);

	//===============================================================================

	//for subID2values
	bool open_subID2values(int _mode);
	bool close_subID2values();
	bool build_subID2values(int** _p_id_tuples, int _triples_num);
	bool getpreIDlistBysubID(int _subid, int*& _preidlist, int& _list_len, bool _no_duplicate = false) const;
	bool getobjIDlistBysubID(int _subid, int*& _objidlist, int& _list_len, bool _no_duplicate = false) const;
	bool getobjIDlistBysubIDpreID(int _subid, int _preid, int*& _objidlist, int& _list_len, bool _no_duplicate = false) const;
	bool getpreIDobjIDlistBysubID(int _subid, int*& _preid_objidlist, int& _list_len, bool _no_duplicate = false) const;

	//for objID2values
	bool open_objID2values(int _mode);
	bool close_objID2values();
	bool build_objID2values(int** _p_id_tuples, int _triples_num);
	bool getpreIDlistByobjID(int _objid, int*& _preidlist, int& _list_len, bool _no_duplicate = false) const;
	bool getsubIDlistByobjID(int _objid, int*& _subidlist, int& _list_len, bool _no_duplicate = false) const;
	bool getsubIDlistByobjIDpreID(int _objid, int _preid, int*& _subidlist, int& _list_len, bool _no_duplicate = false) const;
	bool getpreIDsubIDlistByobjID(int _objid, int*& _preid_subidlist, int& _list_len, bool _no_duplicate = false) const;

	//for preID2values
	bool open_preID2values(int _mode);
	bool close_preID2values();
	bool build_preID2values(int** _p_id_tuples, int _triples_num);
	bool getsubIDlistBypreID(int _preid, int*& _subidlist, int& _list_len, bool _no_duplicate = false) const;
	bool getobjIDlistBypreID(int _preid, int*& _objidlist, int& _list_len, bool _no_duplicate = false) const;
	bool getsubIDobjIDlistBypreID(int _preid, int*& _subid_objidlist, int& _list_len, bool _no_duplicate = false) const;

	//for so2p
	bool getpreIDlistBysubIDobjID(int _subID, int _objID, int*& _preidlist, int& _list_len, bool _no_duplicate = false) const;


private:
	std::string store_path;

	SITree* entity2id;
	ISTree* id2entity;
	static std::string s_entity2id;
	static std::string s_id2entity;
	static unsigned short buffer_entity2id_build;
	static unsigned short buffer_id2entity_build;
	static unsigned short buffer_entity2id_query;
	static unsigned short buffer_id2entity_query;

	SITree* predicate2id;
	ISTree* id2predicate;
	static std::string s_predicate2id;
	static std::string s_id2predicate;
	static unsigned short buffer_predicate2id_build;
	static unsigned short buffer_id2predicate_build;
	static unsigned short buffer_predicate2id_query;
	static unsigned short buffer_id2predicate_query;

	SITree* literal2id;
	ISTree* id2literal;
	static std::string s_literal2id;
	static std::string s_id2literal;
	static unsigned short buffer_literal2id_build;
	static unsigned short buffer_id2literal_build;
	static unsigned short buffer_literal2id_query;
	static unsigned short buffer_id2literal_query;

	IVTree* subID2values;
	IVTree* objID2values;
	IVTree* preID2values;
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

	bool open(SITree* & _p_btree, std::string _tree_name, int _mode, unsigned long long _buffer_size);
	bool open(ISTree* & _p_btree, std::string _tree_name, int _mode, unsigned long long _buffer_size);
	bool open(IVTree* & _p_btree, std::string _tree_name, int _mode, unsigned long long _buffer_size);

	void flush(SITree* _p_btree);
	void flush(ISTree* _p_btree);
	void flush(IVTree* _p_btree);

	bool addValueByKey(SITree* _p_btree, char* _key, int _klen, int _val);
	bool addValueByKey(ISTree* _p_btree, int _key, char* _val, int _vlen);
	bool addValueByKey(IVTree* _p_btree, int _key, char* _val, int _vlen);

	bool setValueByKey(SITree* _p_btree, char* _key, int _klen, int _val);
	bool setValueByKey(ISTree* _p_btree, int _key, char* _val, int _vlen);
	bool setValueByKey(IVTree* _p_btree, int _key, char* _val, int _vlen);

	bool getValueByKey(SITree* _p_btree, const char* _key, int _klen, int* _val) const;
	bool getValueByKey(ISTree* _p_btree, int _key, char*& _val, int& _vlen) const;
	bool getValueByKey(IVTree* _p_btree, int _key, char*& _val, int& _vlen) const;

	int getIDByStr(SITree* _p_btree, const char* _key, int _klen) const;

	bool removeKey(SITree* _p_btree, const char* _key, int _klen);
	bool removeKey(ISTree* _p_btree, int _key);
	bool removeKey(IVTree* _p_btree, int _key);

	static std::vector<int> intersect(const int* _list1, const int* _list2, int _len1, int _len2);
	static int binarySearch(int key, const int* _list, int _list_len, int step = 1);
	static bool isEntity(int id);
};

#endif //_KVSTORE_KVSTORE_H
