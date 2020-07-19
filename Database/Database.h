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
#include "../VSTree/VSTree.h"
#include "../KVstore/KVstore.h"
#include "../StringIndex/StringIndex.h"
#include "../Parser/DBparser.h"
#include "../Parser/RDFParser.h"
#include "../Parser/SparqlParser.h"
#include "../Query/QueryCache.h"
#include "../Query/GeneralEvaluation.h"

class Database {
  public:
  //static const bool only_sub2idpre2id = true;
  //static const int internal = 100 * 1000;

  //void test();
  //void test_build_sig();
  //void test_join();
  //void printIDlist(int _i, int* _list, int _len, std::string _log);
  //void printPairList(int _i, int* _list, int _len, std::string _log);

  //when encode EntitySig, one way uses STRING-hash, the other one uses ID-hash
  //depending on this->encode_mode
  static const int STRING_MODE = 1;
  static const int ID_MODE = 2;
  Database();
  Database(std::string _name);
  void release(FILE* fp0);
  ~Database();

  bool save();
  bool load();
  bool unload();
  void clear();
  int query(const string _query, ResultSet& _result_set, FILE* _fp = stdout, bool update_flag = true, bool export_flag = false);

  //1. if subject of _triple doesn't exist,
  //then assign a new subid, and insert a new SigEntry
  //2. assign new tuple_id to tuple, if predicate or object doesn't exist before too;
  //3. if subject exist, update SigEntry, and update spo, ops... etc. if needed

  bool build(const string& _rdf_file);
  //interfaces to insert/delete from given rdf file
  bool insert(std::string _rdf_file, bool _is_restore = false);
  bool remove(std::string _rdf_file, bool _is_restore = false);

  bool backup();
  bool restore();

  //name of this DB
  string getName();
  //get infos
  TYPE_TRIPLE_NUM getTripleNum();
  TYPE_ENTITY_LITERAL_ID getEntityNum();
  TYPE_ENTITY_LITERAL_ID getLiteralNum();
  TYPE_ENTITY_LITERAL_ID getSubNum();
  TYPE_PREDICATE_ID getPreNum();

  //root Path of this DB + sixTuplesFile
  string getSixTuplesFile();

  //root Path of this DB + signatureBFile
  string getSignatureBFile();

  //root Path of this DB + DBInfoFile
  string getDBInfoFile();

  //id tuples file
  string getIDTuplesFile();

  VSTree* getVSTree();
  KVstore* getKVstore();
  StringIndex* getStringIndex();
  QueryCache* getQueryCache();
  TYPE_TRIPLE_NUM* getpre2num();
  TYPE_TRIPLE_NUM* getpre2sub();
  TYPE_TRIPLE_NUM* getpre2obj();
  TYPE_ENTITY_LITERAL_ID& getlimitID_literal();
  TYPE_ENTITY_LITERAL_ID& getlimitID_entity();
  TYPE_PREDICATE_ID& getlimitID_predicate();
  mutex& get_query_parse_lock();

  private:
  string name;
  string store_path;
  bool is_active;
  TYPE_TRIPLE_NUM triples_num;
  TYPE_ENTITY_LITERAL_ID entity_num;
  TYPE_ENTITY_LITERAL_ID sub_num;
  //BETTER: add object num
  TYPE_PREDICATE_ID pre_num;
  TYPE_ENTITY_LITERAL_ID literal_num;

  int encode_mode;
  bool if_loaded;

  //locks
  mutex query_parse_lock;
  //for read/write, we should use rwlock to improve parallism
  pthread_rwlock_t update_lock;
  //just for debug a block of code
  mutex debug_lock;
  // for getFinalResult
  mutex getFinalResult_lock;

  VSTree* vstree;
  KVstore* kvstore;
  StringIndex* stringindex;
  Join* join;

  //metadata of this database: sub_num, pre_num, obj_num, literal_num, etc.
  string db_info_file;

  //six tuples: <sub pre obj sid pid oid>
  string six_tuples_file;

  //B means binary
  string signature_binary_file;

  //id tuples file
  string id_tuples_file;
  string update_log;
  string update_log_since_backup;

  //pre2num mapping
  TYPE_TRIPLE_NUM* pre2num;
  //pre2subnum mapping
  TYPE_TRIPLE_NUM* pre2sub;
  //pre2objnum mapping
  TYPE_TRIPLE_NUM* pre2obj;
  //valid: check from minNumPID to maxNumPID
  TYPE_PREDICATE_ID maxNumPID, minNumPID;
  void setPreMap();

  //TODO: set the buffer capacity as dynamic according to the current memory usage
  //string buffer
  Buffer* entity_buffer;
  //unsigned offset; //maybe let id start from an offset
  unsigned entity_buffer_size;
  Buffer* literal_buffer;
  unsigned literal_buffer_size;

  QueryCache* query_cache;

  //Trie *trie;

  void setStringBuffer();
  void warmUp();
  //BETTER:add a predicate buffer for ?p query
  //However, I think this is not necessary because ?p is rare and the p2xx tree is small enough

  void query_stringIndex(int id);
  void check();
  //used for multiple threads
  void load_vstree(unsigned _vstree_size);
  void load_entity2id(int _mode);
  void load_id2entity(int _mode);
  void load_literal2id(int _mode);
  void load_id2literal(int _mode);
  void load_predicate2id(int _mode);
  void load_id2predicate(int _mode);
  void load_sub2values(int _mode);
  void load_obj2values(int _mode);
  void load_pre2values(int _mode);

  // functions used to build cache
  void load_cache();
  void get_important_preID();
  std::vector<TYPE_PREDICATE_ID> important_preID;
  void load_important_sub2values();
  void load_important_obj2values();
  void load_candidate_pre2values();
  void build_CacheOfPre2values();
  void build_CacheOfSub2values();
  void build_CacheOfObj2values();
  void get_important_subID();
  void get_important_objID();
  void get_candidate_preID();
  std::priority_queue<KEY_SIZE_VALUE> candidate_preID;
  std::priority_queue<KEY_SIZE_VALUE> important_subID;
  std::priority_queue<KEY_SIZE_VALUE> important_objID;

  //triple num per group for insert/delete
  //can not be too high, otherwise the heap will over
  static const int GROUP_SIZE = 1000;
  //manage the ID allocate and garbage
  static const TYPE_ENTITY_LITERAL_ID START_ID_NUM = 0;
  //static const int START_ID_NUM = 1000;
  /////////////////////////////////////////////////////////////////////////////////
  //NOTICE:error if >= LITERAL_FIRST_ID
  string free_id_file_entity;            //the first is limitID, then free id list
  TYPE_ENTITY_LITERAL_ID limitID_entity; //the current maxium ID num(maybe not used so much)
  BlockInfo* freelist_entity;            //free id list, reuse BlockInfo for Storage class
  TYPE_ENTITY_LITERAL_ID allocEntityID();
  void freeEntityID(TYPE_ENTITY_LITERAL_ID _id);
  /////////////////////////////////////////////////////////////////////////////////
  //NOTICE:error if >= 2*LITERAL_FIRST_ID
  string free_id_file_literal;
  TYPE_ENTITY_LITERAL_ID limitID_literal;
  BlockInfo* freelist_literal;
  TYPE_ENTITY_LITERAL_ID allocLiteralID();
  void freeLiteralID(TYPE_ENTITY_LITERAL_ID _id);
  /////////////////////////////////////////////////////////////////////////////////
  //NOTICE:error if >= 2*LITERAL_FIRST_ID
  string free_id_file_predicate;
  TYPE_PREDICATE_ID limitID_predicate;
  BlockInfo* freelist_predicate;
  TYPE_PREDICATE_ID allocPredicateID();
  void freePredicateID(TYPE_PREDICATE_ID _id);
  /////////////////////////////////////////////////////////////////////////////////
  void initIDinfo();  //initialize the members
  void resetIDinfo(); //reset the id info for build
  void readIDinfo();  //read and build the free list
  void writeIDinfo(); //write and empty the free list
  void saveIDinfo();  //write and empty the free list

  bool saveDBInfoFile();
  bool loadDBInfoFile();

  string getStorePath();

  //encode relative signature data of all Basic Graph Query, who union together into SPARQLquery
  void buildSparqlSignature(SPARQLquery& _sparql_q);

  //encode Triple into Subject EntityBitSet
  bool encodeTriple2SubEntityBitSet(EntityBitSet& _bitset, const Triple* _p_triple);
  //NOTICE: the encodeTriple with Triple* is invalid now(not enocde the linkage of neighbor-predicate)
  bool encodeTriple2SubEntityBitSet(EntityBitSet& _bitset, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id);
  //encode Triple into Object EntityBitSet
  bool encodeTriple2ObjEntityBitSet(EntityBitSet& _bitset, const Triple* _p_triple);
  bool encodeTriple2ObjEntityBitSet(EntityBitSet& _bitset, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _sub_id);

  bool calculateEntityBitSet(TYPE_ENTITY_LITERAL_ID _entity_id, EntityBitSet& _bitset);

  //check whether the relative 3-tuples exist
  //usually, through sp2olist
  bool exist_triple(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id);
  bool exist_triple(const TripleWithObjType& _triple);

  //* _rdf_file denotes the path of the RDF file, where stores the rdf data
  //* there are many step in this function, each one responds to an sub-function
  //* 1. map sub2id and pre2id
  //* 2. map literal2id and encode RDF data into signature,
  //*    storing in binary file: this->getSignatureBFile(), the order responds to subID
  //*    also, store six_tuples in file: this->getSixTuplesFile()
  //* 3. build: subID2objIDlist, <subIDpreID>2objIDlist subID2<preIDobjID>list
  //* 4. build: objID2subIDlist, <objIDpreID>2subIDlist objID2<preIDsubID>list
  //encodeRDF_new invoke new rdfParser to solve task 1 & 2 in one time scan.
  bool encodeRDF_new(const string _rdf_file);
  void readIDTuples(ID_TUPLE*& _p_id_tuples);
  void build_s2xx(ID_TUPLE*);
  void build_o2xx(ID_TUPLE*);
  void build_p2xx(ID_TUPLE*);

  //insert and delete, notice that modify is not needed here
  //we can read from file or use sparql syntax
  bool insertTriple(const TripleWithObjType& _triple, vector<unsigned>* _vertices = NULL, vector<unsigned>* _predicates = NULL);
  bool removeTriple(const TripleWithObjType& _triple, vector<unsigned>* _vertices = NULL, vector<unsigned>* _predicates = NULL);
  //NOTICE:one by one is too costly, sort and insert/delete at a time will be better
  unsigned insert(const TripleWithObjType* _triples, TYPE_TRIPLE_NUM _triple_num, bool _is_restore = false);
  //bool insert(const vector<TripleWithObjType>& _triples, vector<int>& _vertices, vector<int>& _predicates);
  unsigned remove(const TripleWithObjType* _triples, TYPE_TRIPLE_NUM _triple_num, bool _is_restore = false);

  bool sub2id_pre2id_obj2id_RDFintoSignature(const string _rdf_file);
  //bool literal2id_RDFintoSignature(const string _rdf_file, int** _p_id_tuples, TYPE_TRIPLE_NUM _id_tuples_max);

  bool objIDIsEntityID(TYPE_ENTITY_LITERAL_ID _id);

  //* join on the vector of CandidateList, available after retrieve from the VSTREE
  //* and store the resut in _result_set

  //bool join(vector<int*>& _result_list, int _var_id, int _pre_id, int _var_id2, const char _edge_type, int _var_num, bool shouldAddLiteral, IDList& _can_list);

  //bool select(vector<int*>& _result_list, int _var_id, int _pre_id, int _var_id2, const char _edge_type, int _var_num);

  //get the final string result_set from SPARQLquery
  bool getFinalResult(SPARQLquery& _sparql_q, ResultSet& _result_set);

  static int read_update_log(const string _path, multiset<string>& _i, multiset<string>& _r);
  bool restore_update(multiset<string>& _i, multiset<string>& _r);
  void clear_update_log();
};

#endif //_DATABASE_DATABASE_H
