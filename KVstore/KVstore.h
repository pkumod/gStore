/*=============================================================================
# Filename:		KVstore.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified:	2015-10-23 14:23
# Description: 
=============================================================================*/

#ifndef _KVSTORE_KVSTORE_H
#define _KVSTORE_KVSTORE_H

#include "../Util/Util.h"
#include "Tree.h"

//TODO:add debug instruction, control if using the so2p index, which is really costly
//BETTER:keep o2s o2p o2ps, but use two tree to achieve, i.e. literal2xx and entity2xx

class KVstore
{
public:
	static const int READ_WRITE_MODE = 1;
	static const int CREATE_MODE = 2;

	 //include IN-neighbor & OUT-neighbor 
	int getEntityDegree(int _entity_id);
	int getEntityInDegree(int _entity_id);
	int getEntityOutDegree(int _entity_id);
	//============================================
	int getLiteralDegree(int _literal_id);
	int getPredicateDegree(int _predicate_id);
	//============================================
	int getSubjectPredicateDegree(int _subid, int _preid);
	int getObjectPredicateDegree(int _objid, int _preid);

	 //there are two situation when we need to update tuples list: s2o o2s sp2o op2s s2po o2ps
	 //1. insert triple
	 //2. remove triple
	int updateTupleslist_insert(int _sub_id, int _pre_id, int _obj_id);
	void updateTupleslist_remove(int _sub_id, int _pre_id, int _obj_id);

	bool updateInsert_sp2o(int _subid, int _preid, const std::vector<int>& _oidlist);
	bool updateRemove_sp2o(int _subid, int _preid, const std::vector<int>& _oidlist);

	bool updateInsert_s2po(int _subid, const std::vector<int>& _pidoidlist);
	bool updateRemove_s2po(int _subid, const std::vector<int>& _pidoidlist);

	bool updateInsert_s2p(int _subid, const std::vector<int>& _pidlist);
	bool updateRemove_s2p(int _subid, const std::vector<int>& _pidlist);

	bool updateInsert_s2o(int _subid, const std::vector<int>& _oidlist);
	bool updateRemove_s2o(int _subid, const std::vector<int>& _oidlist);

	bool updateInsert_op2s(int _objid, int _preid, const std::vector<int>& _sidlist);
	bool updateRemove_op2s(int _objid, int _preid, const std::vector<int>& _sidlist);

	bool updateInsert_o2ps(int _objid, const std::vector<int>& _pidsidlist);
	bool updateRemove_o2ps(int _objid, const std::vector<int>& _pidsidlist);

	bool updateInsert_o2p(int _objid, const std::vector<int>& _pidlist);
	bool updateRemove_o2p(int _objid, const std::vector<int>& _pidlist);

	bool updateInsert_o2s(int _objid, const std::vector<int>& _sidlist);
	bool updateRemove_o2s(int _objid, const std::vector<int>& _sidlist);

	bool updateInsert_p2so(int _preid, const std::vector<int>& _sidoidlist);
	bool updateRemove_p2so(int _preid, const std::vector<int>& _sidoidlist);

	bool updateInsert_p2s(int _preid, const std::vector<int>& _sidlist);
	bool updateRemove_p2s(int _preid, const std::vector<int>& _sidlist);

	bool updateInsert_p2o(int _preid, const std::vector<int>& _oidlist);
	bool updateRemove_p2o(int _preid, const std::vector<int>& _oidlist);

private:

	 //insert <_x_id, _y_id> into _xylist(keep _xylist(<x,y>) in ascending order) 
	bool insert_xy(int*& _xylist, int& _list_len,int _x_id, int _y_id);
	 //insert _x_id into _xlist(keep _xlist in ascending order) 
	bool insert_x(int*& _xlist, int& _list_len, int _x_id);
	 //remove _x_id from _xlist 
	bool remove_x(int*& _xlist, int& _list_len, int _x_id);
	 //remove <_x_id, _y_id> from _xylist 
	bool remove_xy(int*& _xylist, int& _list_len,int _x_id, int _y_id);
	//===================================================================================
	bool insertList_x(int*& _xlist, int& _list_len, const std::vector<int>& _list2);
	bool insertList_xy(int*& _xylist, int& _list_len, const std::vector<int>& list2);
	bool removeList_x(int*& _xlist, int& _list_len, const std::vector<int>& _list2);
	bool removeList_xy(int*& _xylist, int& _list_len, const std::vector<int>& _list2);

public:
	 //for entity2id 
	bool open_entity2id(int _mode);
	bool close_entity2id();
	bool subIDByEntity(std::string _entity);
	int getIDByEntity(std::string _entity);
	bool setIDByEntity(std::string _entity, int _id);

	 //for id2entity 
	bool open_id2entity(int _mode);
	bool close_id2entity();
	bool subEntityByID(int _id);
	std::string getEntityByID(int _id);
	bool setEntityByID(int _id, std::string _entity);

	 //for predicate2id 
	bool open_predicate2id(int _mode);
	bool close_predicate2id();
	bool subIDByPredicate(std::string _predicate);
	int getIDByPredicate(std::string _predicate);
	bool setIDByPredicate(std::string _predicate, int _id);

	 //for id2predicate 
	bool open_id2predicate(int _mode);
	bool close_id2predicate();
	bool subPredicateByID(int _id);
	std::string getPredicateByID(int _id);
	bool setPredicateByID(int _id, std::string _predicate);

	 //for id2literal 
	bool open_id2literal(int _mode);
	bool close_id2literal();
	bool subLiteralByID(int _id);
	std::string getLiteralByID(int _id);
	bool setLiteralByID(int _id, std::string _literal);

	 //for literal2id 
	bool open_literal2id(int _mode);
	bool close_literal2id();
	bool subIDByLiteral(std::string _literal);
	int getIDByLiteral(std::string _literal);
	bool setIDByLiteral(std::string _literal, int _id);

	 //for subID 2 objIDlist 
	bool open_subID2objIDlist(int _mode);
	bool close_subID2objIDlist();
	bool getobjIDlistBysubID(int _subid, int*& _objidlist, int& _list_len);
	bool addobjIDlistBysubID(int _subid, const int* _objidlist, int _list_len);
	bool setobjIDlistBysubID(int _subid, const int* _objidlist, int _list_len);

	 //for objID 2 subIDlist 
	bool open_objID2subIDlist(int _mode);
	bool close_objID2subIDlist();
	bool getsubIDlistByobjID(int _objid, int*& _subidlist, int& _list_len);
	bool addsubIDlistByobjID(int _objid, const int* _subidlist, int _list_len);
	bool setsubIDlistByobjID(int _objid, const int* _subidlist, int _list_len);

	 //for subID&preID 2 objIDlist 
	bool open_subIDpreID2objIDlist(int _mode);
	bool close_subIDpreID2objIDlist();
	bool getobjIDlistBysubIDpreID(int _subid, int _preid, int*& _objidlist, int& _list_len);
	bool addobjIDlistBysubIDpreID(int _subid, int _preid, const int* _objidlist, int _list_len);
	bool setobjIDlistBysubIDpreID(int _subid, int _preid, const int* _objidlist, int _list_len);

	 //for objID&preID 2 subIDlist 
	bool open_objIDpreID2subIDlist(int _mode);
	bool close_objIDpreID2subIDlist();
	bool getsubIDlistByobjIDpreID(int _objid, int _preid, int*& _subidlist, int& _list_len);
	bool addsubIDlistByobjIDpreID(int _objid, int _preid, const int* _subidlist, int _list_len);
	bool setsubIDlistByobjIDpreID(int _objid, int _preid, const int* _subidlist, int _list_len);

	 //for subID 2  preID&objIDlist 
	bool open_subID2preIDobjIDlist(int _mode);
	bool close_subID2preIDobjIDlist();
	bool getpreIDobjIDlistBysubID(int _subid, int*& _preid_objidlist, int& _list_len);
	bool addpreIDobjIDlistBysubID(int _subid, const int* _preid_objidlist, int _list_len);
	bool setpreIDobjIDlistBysubID(int _subid, const int* _preid_objidlist, int _list_len);

	 //for objID 2 preID&subIDlist 
	bool open_objID2preIDsubIDlist(int _mode);
	bool close_objID2preIDsubIDlist();
	bool getpreIDsubIDlistByobjID(int _objid, int*& _preid_subidlist, int& _list_len);
	bool addpreIDsubIDlistByobjID(int _objid, const int* _preid_subidlist, int _list_len);
	bool setpreIDsubIDlistByobjID(int _objid, const int* _preid_subidlist, int _list_len);

	//for subID 2 preIDlist
	bool open_subID2preIDlist(int _mode);
	bool close_subID2preIDlist();
	bool getpreIDlistBysubID(int _subid, int*& _preidlist, int& _list_len);
	bool addpreIDlistBysubID(int _subid, const int* _preidlist, int _list_len);
	bool setpreIDlistBysubID(int _subid, const int* _preidlist, int _list_len);

	//for preID 2 subIDlist
	bool open_preID2subIDlist(int _mode);
	bool close_preID2subIDlist();
	bool getsubIDlistBypreID(int _preid, int*& _subidlist, int& _list_len);
	bool addsubIDlistBypreID(int _preid, const int* _subidlist, int _list_len);
	bool setsubIDlistBypreID(int _preid, const int* _subidlist, int _list_len);

	//for objID 2 preIDlist
	bool open_objID2preIDlist(int _mode);
	bool close_objID2preIDlist();
	bool getpreIDlistByobjID(int _objid, int*& _preidlist, int& _list_len);
	bool addpreIDlistByobjID(int _objid, const int* _preidlist, int _list_len);
	bool setpreIDlistByobjID(int _objid, const int* _preidlist, int _list_len);

	//for preID 2 objIDlist
	bool open_preID2objIDlist(int _mode);
	bool close_preID2objIDlist();
	bool getobjIDlistBypreID(int _preid, int*& _objidlist, int& _list_len);
	bool addobjIDlistBypreID(int _preid, const int* _objidlist, int _list_len);
	bool setobjIDlistBypreID(int _preid, const int* _objidlist, int _list_len);

	//for subID&objID 2 preIDlist
	bool open_subIDobjID2preIDlist(int _mode);
	//bool close_subIDobjID2preIDlist();
	bool getpreIDlistBysubIDobjID(int _subID, int _objID, int*& _preidlist, int& _list_len);
	bool addpreIDlistBysubIDobjID(int _subID, int _objID, const int* _preidlist, int _list_len);
	bool setpreIDlistBysubIDobjID(int _subID, int _objID, const int* _preidlist, int _list_len);

	//for preID 2 subID&objIDlist
	bool open_preID2subIDobjIDlist(int _mode);
	bool close_preID2subIDobjIDlist();
	bool getsubIDobjIDlistBypreID(int _preid, int*& _subid_objidlist, int& _list_len);
	bool addsubIDobjIDlistBypreID(int _preid, const int* _subid_objidlist, int _list_len);
	bool setsubIDobjIDlistBypreID(int _preid, const int* _subid_objidlist, int _list_len);

	//QUERY:is the below 3 indexes needed?
	//In fact, p2so can compute the num of triples if dividing so_len by 2
	//However, sometimes this can be very large and costly
	//For example, the predicate is <rdf:type>
	//NOTICE: not change to set/add for below 3 indexes
	//not add close function, neither

	 //for predicate 2 triple num
	bool open_preID2num(int _mode);
	int getNumBypreID(int _preid);
	bool setNumBypreID(int _preid, int _tripleNum);

	//for subject&predicate 2 triple num
	bool open_subIDpreID2num(int _mode);
	int getNumBysubIDpreID(int _subID, int _preID);
	bool setNumBysubIDpreID(int _subID, int _preID, int _tripleNum);

	//for object&predicate 2 triple num
	bool open_objIDpreID2num(int _mode);
	int getNumByobjIDpreID(int _objid, int _preid);
	bool setNumByobjIDpreID(int _objid, int _preid, int _tripleNum);

	KVstore(std::string _store_path = ".");
	~KVstore();
	void flush();
	void release();
	void open();

private:

	std::string store_path;
	//map entity to its id, and id to the entity
	//s_entity2id is relative store file name
	SITree* entity2id;
	ISTree* id2entity;
	static std::string s_entity2id;
	static std::string s_id2entity;

	SITree* predicate2id;
	ISTree* id2predicate;
	static std::string s_predicate2id;
	static std::string s_id2predicate;

	SITree* literal2id;
	ISTree* id2literal;
	static std::string s_literal2id;
	static std::string s_id2literal;

	ISTree* subID2objIDlist;
	ISTree* objID2subIDlist;
	static std::string s_sID2oIDlist;
	static std::string s_oID2sIDlist;

	Tree* subIDpreID2objIDlist;
	Tree* objIDpreID2subIDlist;
	static std::string s_sIDpID2oIDlist;
	static std::string s_oIDpID2sIDlist;

	ISTree* subID2preIDobjIDlist;
	ISTree* objID2preIDsubIDlist;
	static std::string s_sID2pIDoIDlist;
	static std::string s_oID2pIDsIDlist;

	ISTree* subID2preIDlist;
	ISTree* preID2subIDlist;
	static std::string s_sID2pIDlist;
	static std::string s_pID2sIDlist;

	ISTree* objID2preIDlist;
	ISTree* preID2objIDlist;
	static std::string s_oID2pIDlist;
	static std::string s_pID2oIDlist;

	Tree* subIDobjID2preIDlist;
	ISTree* preID2subIDobjIDlist;
	static std::string s_sIDoID2pIDlist;
	static std::string s_pID2sIDoIDlist;

	//BETTER:this can use IITree if ok
	SITree* preID2num;
	SITree* subIDpreID2num;
	SITree* objIDpreID2num;
	static std::string s_pID2num;
	static std::string s_sIDpID2num;
	static std::string s_oIDpID2num;

	void flush(Tree* _p_btree);
	void flush(SITree* _p_btree);
	void flush(ISTree* _p_btree);

	bool addValueByKey(Tree* _p_btree, const char* _key, int _klen, const char* _val, int _vlen);
	bool addValueByKey(SITree* _p_btree, const char* _key, int _klen, int _val);
	bool addValueByKey(ISTree* _p_btree, int _key, const char* _val, int _vlen);

	bool setValueByKey(Tree* _p_btree, const char* _key, int _klen, const char* _val, int _vlen);
	bool setValueByKey(SITree* _p_btree, const char* _key, int _klen, int _val);
	bool setValueByKey(ISTree* _p_btree, int _key, const char* _val, int _vlen);

	bool getValueByKey(Tree* _p_btree, const char* _key, int _klen, char*& _val, int& _vlen);
	bool getValueByKey(SITree* _p_btree, const char* _key, int _klen, int* _val);
	bool getValueByKey(ISTree* _p_btree, int _key, char*& _val, int& _vlen);

	int getIDByStr(SITree* _p_btree, const char* _key, int _klen);

	bool removeKey(Tree* _p_btree, const char* _key, int _klen);
	bool removeKey(SITree* _p_btree, const char* _key, int _klen);
	bool removeKey(ISTree* _p_btree, int _key);

	//Open a btree according the mode 
	//CREATE_MODE: 		build a new btree and delete if exist 	
	//READ_WRITE_MODE: 	open a btree, btree must exist  		
	bool open(Tree*& _p_btree, std::string _tree_name, int _mode);
	bool open(SITree* & _p_btree, std::string _tree_name, int _mode);
	bool open(ISTree* & _p_btree, std::string _tree_name, int _mode);
};

#endif //_KVSTORE_KVSTORE_H

