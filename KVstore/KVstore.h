/*
 * KVstore.h
 *
 *  Created on: 2014-5-8
 *      Author: liyouhuan
 */

#ifndef KVSTORE_H_
#define KVSTORE_H_
#include<iostream>
#include<string.h>
#include<sys/stat.h>
#include<dirent.h>
#include"Btree.h"
using namespace std;
class KVstore{
public:
	static const bool debug_mode = false;
	static const bool test = false;
	static const int READ_WRITE_MODE = 1;
	static const int CREATE_MODE = 2;

	/* include IN-neighbor & OUT-neighbor */
	int getEntityDegree(int _entity_id);
	int getEntityInDegree(int _entity_id);
	int getEntityOutDegree(int _entity_id);

	/* there are two situation when we need to update tuples list: s2o o2s sp2o op2s s2po o2ps
	 * 1. insert triple
	 * 2. remove triple
	 */
	void updateTupleslist_insert(int _sub_id, int _pre_id, int _obj_id);
	void updateTupleslist_remove(int _sub_id, int _pre_id, int _obj_id);
private:

	/* insert <_x_id, _y_id> into _xylist(keep _xylist(<x,y>) in ascending order) */
	bool insert_xy(int*& _xylist, int& _list_len,int _x_id, int _y_id);

	/* insert _x_id into _xlist(keep _xlist in ascending order) */
	bool insert_x(int*& _xlist, int& _list_len, int _x_id);

	/* remove _x_id from _xlist */
	bool remove_x(int*& _xlist, int& _list_len, int _x_id);

	/* remove <_x_id, _y_id> from _xylist */
	bool remove_xy(int*& _xylist, int& _list_len,int _x_id, int _y_id);

public:

	/* for entity2id */
	bool open_entity2id(const int _mode);
	int getIDByEntity(const string _entity);
	bool setIDByEntity(const string _entity, int _id);


	/* for id2entity */
	bool open_id2entity(const int _mode);
	string getEntityByID(int _id);
	bool setEntityByID(int _id, string _entity);

	/* for predicate2id */
	bool open_predicate2id(const int _mode);
	int getIDByPredicate(const string _predicate);
	bool setIDByPredicate(const string _predicate, int _id);

	/* for id2predicate */
	bool open_id2predicate(const int _mode);
	string getPredicateByID(int _id);
	bool setPredicateByID(const int _id, string _predicate);

	/* for id2literal */
	bool open_id2literal(const int _mode);
	string getLiteralByID(int _id);
	bool setLiteralByID(const int _id, string _literal);


	/* for literal2id */
	bool open_literal2id(const int _mode);
	int getIDByLiteral(string _literal);
	bool setIDByLiteral(const string _literal, int _id);

	/* for subID 2 objIDlist */
	bool open_subid2objidlist(const int _mode);
	bool getobjIDlistBysubID(int _subid, int*& _objidlist, int& _list_len);
	bool setobjIDlistBysubID(int _subid, const int* _objidlist, int _list_len);

	/* for objID 2 subIDlist */
	bool open_objid2subidlist(const int _mode);
	bool getsubIDlistByobjID(int _objid, int*& _subidlist, int& _list_len);
	bool setsubIDlistByobjID(int _objid, const int* _subidlist, int _list_len);

	/* for subID&preID 2 objIDlist */
	bool open_subIDpreID2objIDlist(const int _mode);
	bool getobjIDlistBysubIDpreID(int _subid, int _preid, int*& _objidlist, int& _list_len);
	bool setobjIDlistBysubIDpreID(int _subid, int _preid, const int* _objidlist, int _list_len);

	/* for objID&preID 2 subIDlist */
	bool open_objIDpreID2subIDlist(const int _mode);
	bool getsubIDlistByobjIDpreID(int _objid, int _preid, int*& _subidlist, int& _list_len);
	bool setsubIDlistByobjIDpreID(int _objid, int _preid, const int* _subidlist, int _list_len);

	/* for subID 2  preID&objIDlist */
	bool open_subID2preIDobjIDlist(const int _mode);
	bool getpreIDobjIDlistBysubID(int _subid, int*& _preid_objidlist, int& _list_len);
	bool setpreIDobjIDlistBysubID(int _subid, const int* _preid_objidlist, int _list_len);

	/* for objID 2 preID&subIDlist */
	bool open_objID2preIDsubIDlist(const int _mode);
	bool getpreIDsubIDlistByobjID(int _objid, int*& _preid_subidlist, int& _list_len);
	bool setpreIDsubIDlistByobjID(int _objid, const int* _preid_subidlist, int _list_len);

	/*
	 * _store_path denotes where to store the data
	 */
	KVstore(string _store_path = ".");
	~KVstore();
	void flush();
	void release();
	void open();

private:

	string store_path;
	/*
	 *
	 * map entity to its id, and id to the entity
	 * s_entity2id is relative store file name
	 */
	Btree* entity2id;
	Btree* id2entity;
	static string s_entity2id;
	static string s_id2entity;

	Btree* predicate2id;
	Btree* id2predicate;
	static string s_predicate2id;
	static string s_id2predicate;

	Btree* literal2id;
	Btree* id2literal;
	static string s_literal2id;
	static string s_id2literal;


	Btree* subID2objIDlist;
	Btree* objID2subIDlist;
	static string s_sID2oIDlist;
	static string s_oID2sIDlist;

	/* lack exist in update tuple */
	Btree* subIDpreID2objIDlist;
	Btree* objIDpreID2subIDlist;
	static string s_sIDpID2oIDlist;
	static string s_oIDpID2sIDlist;

	Btree* subID2preIDobjIDlist;
	Btree* objID2preIDsubIDlist;
	static string s_sID2pIDoIDlist;
	static string s_oID2pIDsIDlist;


	void flush(Btree* _p_btree);
	void release(Btree* _p_btree);
	bool setValueByKey(Btree* _p_btree, const char* _key, int _klen, const char* _val, int _vlen);
	bool getValueByKey(Btree* _p_btree, const char* _key, int _klen, char*& _val, int& _vlen);
	int getIDByStr(Btree* _p_btree, const char* _key, int _klen);
	bool removeKey(Btree* _p_btree, const char* _key, int _klen);

	/* Open a btree according the mode */
	/* CREATE_MODE: 		build a new btree and delete if exist 	*/
	/* READ_WRITE_MODE: 	open a btree, btree must exist  		*/
	bool open(Btree* & _p_btree, const string _tree_name, const int _mode);
};


#endif /* KVSTORE_H_ */
