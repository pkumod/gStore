/*=============================================================================
# Filename: Database.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-23 14:20
# Description: originally written by liyouhuan, modified by zengli and chenjiaqi
=============================================================================*/

#ifndef _DATABASE_DATABASE_H
#define _DATABASE_DATABASE_H 

#include "../Util/Util.h"
#include "../Util/Triple.h"
#include "Join.h"
#include "../Query/IDList.h"
#include "../Query/ResultSet.h"
#include "../Query/SPARQLquery.h"
#include "../Query/BasicQuery.h"
#include "../Signature/SigEntry.h"
#include "../KVstore/KVstore.h"
#include "../VSTree/VSTree.h"
#include "../Parser/DBparser.h"
#include "../Parser/RDFParser.h"
#include "../Parser/SparqlParser.h"
#include "../Query/GeneralEvaluation.h"

class Database
{
public:
	static const bool only_sub2idpre2id = true;
	static const int internal = 100*1000;
	void test();
	void test_build_sig();
	void test_join();
	void printIDlist(int _i, int* _list, int _len, std::string _log);
	void printPairList(int _i, int* _list, int _len, std::string _log);

	 //when encode EntitySig, one way uses STRING-hash, the other one uses ID-hash
	 //depending on this->encode_mode  
	static const int STRING_MODE = 1;
	static const int ID_MODE = 2;
	Database();
	Database(std::string _name);
	void release(FILE* fp0);
	~Database();

	bool load();
	bool unload();
	bool query(const string _query, ResultSet& _result_set, FILE* _fp = stdout);
	bool query(const string _query, ResultSet& _result_set, string& intermediate_res, int myRank, FILE* _fp = stdout);

	 //1. if subject of _triple doesn't exist,
		//then assign a new subid, and insert a new SigEntry
	 //2. assign new tuple_id to tuple, if predicate or object doesn't exist before too;
	 //3. if subject exist, update SigEntry, and update spo, ops... etc. if needed

    bool insert(const string& _insert_rdf_file);
    bool remove(const string& _rdf_file);
    bool build(const string& _rdf_file);
	//interfaces to insert/delete from given rdf file
	bool insert(std::string _rdf_file, vector<int>& _vertices, vector<int>& _predicates);
	bool remove(std::string _rdf_file, vector<int>& _vertices, vector<int>& _predicates);

	/* name of this DB*/
	string getName();
	/* root Path of this DB + sixTuplesFile */
	string getSixTuplesFile();

	/* root Path of this DB + signatureBFile */
	string getSignatureBFile();

    /* root Path of this DB + DBInfoFile */
    string getDBInfoFile();

	bool loadInternalVertices(const string _in_file);

private:
	string name;
	bool is_active;
	int triples_num;
	int entity_num;
	int sub_num;
	int pre_num;
	int literal_num;

	int encode_mode;

	VSTree* vstree;
	KVstore* kvstore;
	Join* join;

	 //metadata of this database: sub_num, pre_num, obj_num, literal_num, etc. 
    string db_info_file;

	 //six tuples: <sub pre obj sid pid oid> 
	string six_tuples_file;
	 //B means binary 
	string signature_binary_file;

	/* internal vertices string */
	string internal_tag_str;
	
	//triple num per group for insert/delete
	//can not be too high, otherwise the heap will over
	static const int GROUP_SIZE = 1000;
	//manage the ID allocate and garbage
	static const int START_ID_NUM = 1000;
	/////////////////////////////////////////////////////////////////////////////////
	//NOTICE:error if >= LITERAL_FIRST_ID
	string free_id_file_entity; //the first is limitID, then free id list
	int limitID_entity; //the current maxium ID num(maybe not used so much)
	BlockInfo* freelist_entity; //free id list, reuse BlockInfo for Storage class
	int allocEntityID();
	void freeEntityID(int _id);
	/////////////////////////////////////////////////////////////////////////////////
	//NOTICE:error if >= 2*LITERAL_FIRST_ID
	string free_id_file_literal;
	int limitID_literal; 
	BlockInfo* freelist_literal; 
	int allocLiteralID();
	void freeLiteralID(int _id);
	/////////////////////////////////////////////////////////////////////////////////
	//NOTICE:error if >= 2*LITERAL_FIRST_ID
	string free_id_file_predicate;
	int limitID_predicate; 
	BlockInfo* freelist_predicate; 
	int allocPredicateID();
	void freePredicateID(int _id);
	/////////////////////////////////////////////////////////////////////////////////
	void initIDinfo();  //initialize the members
	void resetIDinfo(); //reset the id info for build
	void readIDinfo();  //read and build the free list
	void writeIDinfo(); //write and empty the free list


    bool saveDBInfoFile();
    bool loadDBInfoFile();

	string getStorePath();

	 //encode relative signature data of all Basic Graph Query, who union together into SPARQLquery 
	void buildSparqlSignature(SPARQLquery & _sparql_q);

	 //encode Triple into Subject EntityBitSet 
    bool encodeTriple2SubEntityBitSet(EntityBitSet& _bitset, const Triple* _p_triple);
	 //encode Triple into Object EntityBitSet 
    bool encodeTriple2ObjEntityBitSet(EntityBitSet& _bitset, const Triple* _p_triple);

	bool calculateEntityBitSet(int _entity_id, EntityBitSet & _bitset);

	 //check whether the relative 3-tuples exist
	 //usually, through sp2olist 
	bool exist_triple(int _sub_id, int _pre_id, int _obj_id);

	 //* _rdf_file denotes the path of the RDF file, where stores the rdf data
	 //* there are many step in this function, each one responds to an sub-function
	 //* 1. map sub2id and pre2id
	 //* 2. map literal2id and encode RDF data into signature,
	 //*    storing in binary file: this->getSignatureBFile(), the order responds to subID
	 //*    also, store six_tuples in file: this->getSixTuplesFile()
	 //* 3. build: subID2objIDlist, <subIDpreID>2objIDlist subID2<preIDobjID>list
	 //* 4. build: objID2subIDlist, <objIDpreID>2subIDlist objID2<preIDsubID>list
	//encodeRDF_new invoke new rdfParser to solve task 1 & 2 in one time scan.
	bool encodeRDF(const string _rdf_file);
	bool encodeRDF_new(const string _rdf_file);

	//insert and delete, notice that modify is not needed here
	//we can read from file or use sparql syntax
    int insertTriple(const TripleWithObjType& _triple);
    bool removeTriple(const TripleWithObjType& _triple);
	//NOTICE:one by one is too costly, sort and insert/delete at a time will be better
	bool insert(const TripleWithObjType* _triples, int _triple_num, vector<int>& _vertices, vector<int>& _predicates);
	//bool insert(const vector<TripleWithObjType>& _triples, vector<int>& _vertices, vector<int>& _predicates);
	bool remove(const TripleWithObjType* _triples, int _triple_num, vector<int>& _vertices, vector<int>& _predicates);
	//bool remove(const vector<TripleWithObjType>& _triples, vector<int>& _vertices, vector<int>& _predicates);

	bool sub2id_pre2id_obj2id_RDFintoSignature(const string _rdf_file, int**& _p_id_tuples, int & _id_tuples_max);
	bool literal2id_RDFintoSignature(const string _rdf_file, int** _p_id_tuples, int _id_tuples_max);
	
	bool s2o_s2po_sp2o(int** _p_id_tuples, int _id_tuples_max);
	bool o2s_o2ps_op2s(int** _p_id_tuples, int _id_tuples_max);
	//NOTICE: below is the new one
	//bool s2p_s2po_sp2o(int** _p_id_tuples, int _id_tuples_max);
	bool s2p_s2o_s2po_sp2o(int** _p_id_tuples, int _id_tuples_max);
	//bool s2p_s2o_s2po_sp2o_sp2n(int** _p_id_tuples, int _id_tuples_max);
	bool o2p_o2s_o2ps_op2s(int** _p_id_tuples, int _id_tuples_max);
	//bool o2p_o2s_o2ps_op2s_op2n(int** _p_id_tuples, int _id_tuples_max);
	bool p2s_p2o_p2so(int** _p_id_tuples, int _id_tuples_max);
	//bool p2s_p2o_p2so_p2n(int** _p_id_tuples, int _id_tuples_max);
	bool so2p_s2o(int** _p_id_tuples, int _id_tuples_max);

	static int _spo_cmp(const void* _a, const void* _b);
	static int _ops_cmp(const void* _a, const void* _b);
	static int _pso_cmp(const void* _a, const void* _b);
	static int _sop_cmp(const void* _a, const void* _b);
	bool objIDIsEntityID(int _id);

	 //* join on the vector of CandidateList, available after retrieve from the VSTREE
	 //* and store the resut in _result_set

	//bool join(vector<int*>& _result_list, int _var_id, int _pre_id, int _var_id2, const char _edge_type, int _var_num, bool shouldAddLiteral, IDList& _can_list);

	//bool select(vector<int*>& _result_list, int _var_id, int _pre_id, int _var_id2, const char _edge_type, int _var_num);

	 //get the final string result_set from SPARQLquery 
	bool getFinalResult(SPARQLquery& _sparql_q, ResultSet& _result_set);
};

#endif //_DATABASE_DATABASE_H

