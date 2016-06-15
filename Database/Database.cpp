/*=============================================================================
# Filename: Database.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-23 14:22
# Description: originally written by liyouhuan, modified by zengli and chenjiaqi
=============================================================================*/

#include "Database.h"

using namespace std;

Database::Database()
{
    this->name = "";
    string store_path = ".";
    this->signature_binary_file = "signature.binary";
    this->six_tuples_file = "six_tuples";
    this->db_info_file = "db_info_file.dat";

    string kv_store_path = store_path + "/kv_store";
    this->kvstore = new KVstore(kv_store_path);

    string vstree_store_path = store_path + "/vs_store";
    this->vstree = new VSTree(vstree_store_path);

    this->encode_mode = Database::STRING_MODE;
    this->is_active = false;
    this->sub_num = 0;
    this->pre_num = 0;
    this->literal_num = 0;
    this->entity_num = 0;
    this->triples_num = 0;

	this->join = NULL;
}

Database::Database(string _name)
{
    this->name = _name;
    string store_path = this->name;

    this->signature_binary_file = "signature.binary";
    this->six_tuples_file = "six_tuples";
    this->db_info_file = "db_info_file.dat";

    string kv_store_path = store_path + "/kv_store";
    this->kvstore = new KVstore(kv_store_path);

    string vstree_store_path = store_path + "/vs_store";
    this->vstree = new VSTree(vstree_store_path);

    this->encode_mode = Database::STRING_MODE;
    this->is_active = false;
    this->sub_num = 0;
    this->pre_num = 0;
    this->literal_num = 0;
    this->entity_num = 0;
    this->triples_num = 0;

	this->join = NULL;
}

void
Database::release(FILE* fp0)
{
    fprintf(fp0, "begin to delete DB!\n");
    fflush(fp0);
    delete this->vstree;
    fprintf(fp0, "ok to delete vstree!\n");
    fflush(fp0);
    delete this->kvstore;
    fprintf(fp0, "ok to delete kvstore!\n");
    fflush(fp0);
    //fclose(Util::debug_database);
    //Util::debug_database = NULL;	//debug: when multiple databases
    fprintf(fp0, "ok to delete DB!\n");
    fflush(fp0);
}

Database::~Database()
{
	this->unload();
    //fclose(Util::debug_database);
    //Util::debug_database = NULL;	//debug: when multiple databases
}

bool
Database::load()
{
	//DEBUG:what if loaded several times?to check if loaded?
    bool flag = (this->vstree)->loadTree();
    if (!flag)
    {
        cerr << "load tree error. @Database::load()" << endl;
        return false;
    }

    flag = this->loadDBInfoFile();
    if (!flag)
    {
        cerr << "load database info error. @Database::load()" << endl;
        return false;
    }

    (this->kvstore)->open();
    cout << "finish load" << endl;

    return true;
}

bool
Database::unload()
{
    delete this->vstree;
	this->vstree = NULL;
    delete this->kvstore;
	this->kvstore = NULL;

    return true;
}

string
Database::getName()
{
    return this->name;
}

bool
Database::query(const string _query, ResultSet& _result_set, FILE* _fp)
{
	//int sid = kvstore->getIDByEntity("<http://example/bob>");
	//int oid = kvstore->getIDByLiteral("\"Bob\"");
	//cout << "sid: " << sid << "\toid: " << oid << endl;
	//int* list = NULL;
	//int len = 0;
	//kvstore->getpreIDlistBysubIDobjID(sid, oid, list, len);
	//kvstore->getpreIDlistBysubID(sid, list, len);
	//for(int i = 0; i < len; ++i)
	//{
		  //cout << "predicate "  << list[i] << endl;
		  //cout << kvstore->getPredicateByID(list[i]) << endl;
	//}
	//cout << "the end" << endl;

	//string ret = Util::getExactPath(_query.c_str());
    long tv_begin = Util::get_cur_time();

	GeneralEvaluation general_evaluation(this->vstree, this->kvstore, _result_set);
	general_evaluation.doQuery(_query);

	//if (!general_evaluation.parseQuery(_query))
		//return false;

    //DBparser _parser;
    //SPARQLquery _sparql_q(_query);
    //_parser.sparqlParser(_query, _sparql_q);

    //long tv_parse = Util::get_cur_time();
    //cout << "after Parsing, used " << (tv_parse - tv_begin) << endl;
    //cout << "after Parsing..." << endl << _sparql_q.triple_str() << endl;

	//general_evaluation.getSPARQLQuery().encodeQuery(this->kvstore, general_evaluation.getSPARQLQueryVarset());

    //cout << "sparqlSTR:\t" << _sparql_q.to_str() << endl;
	//cout << "sparqlSTR:\t" << general_evaluation.getSPARQLQuery().to_str() << endl;

    //long tv_encode = Util::get_cur_time();
    //cout << "after Encode, used " << (tv_encode - tv_parse) << "ms." << endl;

    //_result_set.select_var_num = _sparql_q.getQueryVarNum();

    //(this->vstree)->retrieve(_sparql_q);
	//(this->vstree)->retrieve(general_evaluation.getSPARQLQuery());

    //long tv_retrieve = Util::get_cur_time();
    //cout << "after Retrieve, used " << (tv_retrieve - tv_encode) << "ms." << endl;

	//this->join = new Join(kvstore);
	//this->join->join_sparql(general_evaluation.getSPARQLQuery());
	//delete this->join;

    //long tv_join = Util::get_cur_time();
    //cout << "after Join, used " << (tv_join - tv_retrieve) << "ms." << endl;
	//this->handle(general_evaluation.getSPARQLQuery());

	//general_evaluation.generateEvaluationPlan(general_evaluation.getQueryTree().getPatternGroup());
	//general_evaluation.doEvaluationPlan();
	//general_evaluation.getFinalResult(_result_set);
    //this->getFinalResult(_sparql_q, _result_set);

	//long tv_final = Util::get_cur_time();
	//cout << "after finalResult, used " << (tv_final - tv_join) << "ms." << endl;

    //cout << "Total time used: " << (tv_final - tv_begin) << "ms." << endl;

    //testing...
    cout << "final result is : " << endl;
#ifndef STREAM_ON
    cout << _result_set.to_str() << endl;
#else
    _result_set.output(_fp);
    //cout<<endl;		//empty the buffer;print an empty line
	fprintf(_fp, "\n");    
	fflush(_fp);       //to empty the output buffer in C (fflush(stdin) not work in GCC)
#endif

#ifdef DEBUG_PRECISE
	fprintf(stderr, "the query function exits!\n");
#endif
    return true;
}

//NOTICE: not used now!
bool
Database::insert(const string& _insert_rdf_file)
{
    bool flag = this->load();

    if (!flag)
    {
        return false;
    }
    cout << "finish loading" << endl;

    long tv_load = Util::get_cur_time();

    ifstream _fin(_insert_rdf_file.c_str());
    if(!_fin) {
        cerr << "fail to open : " << _insert_rdf_file << ".@insert_test" << endl;
        exit(0);
    }

    TripleWithObjType* triple_array = new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP];
    //parse a file
    RDFParser _parser(_fin);

    int insert_triple_num = 0;
    long long sum_avg_len = 0;
#ifdef DEBUG
    Util::logging("==> while(true)");
#endif
    while(true)
    {
        int parse_triple_num = 0;
        _parser.parseFile(triple_array, parse_triple_num);
#ifdef DEBUG
		stringstream _ss;
		_ss << "finish rdfparser" << insert_triple_num << endl;
		Util::logging(_ss.str());
		cout << _ss.str() << endl;
#endif

        if(parse_triple_num == 0)
        {
            break;
        }


        /* Process the Triple one by one */
        for(int i = 0; i < parse_triple_num; i ++)
        {
            //debug
//            {
//                stringstream _ss;
//                _ss << "insert triple: " << triple_array[i].toString() << " insert_triple_num=" << insert_triple_num << endl;
//                Util::logging(_ss.str());
//            }
            sum_avg_len += this->insertTriple(triple_array[i]);
            insert_triple_num ++;

            //debug
//            {
//                if (insert_triple_num % 100 == 0)
//                {
//                    sum_avg_len /= 100;
//                    cout <<"average update len per 100 triple: " << sum_avg_len <<endl;
//                    sum_avg_len = 0;
//                }
//            }
        }
    }

    long tv_insert = Util::get_cur_time();
    cout << "after insert, used " << (tv_insert - tv_load) << "ms." << endl;

    flag = this->vstree->saveTree();
    if (!flag)
    {
        return false;
    }

    flag = this->saveDBInfoFile();
    if (!flag)
    {
        return false;
    }

    cout << "insert rdf triples done." << endl;

    return true;
}

//TODO and DEBUG
bool
Database::remove(const string& _rdf_file)
{
    bool flag = this->saveDBInfoFile();
    if (!flag)
    {
        return false;
    }
    return true;
}

//TODO:modify using delete and insert

bool
Database::build(const string& _rdf_file)
{
	string ret = Util::getExactPath(_rdf_file.c_str());
    long tv_build_begin = Util::get_cur_time();

    string store_path = this->name;
    Util::create_dir(store_path);

    string kv_store_path = store_path + "/kv_store";
    Util::create_dir(kv_store_path);

    string vstree_store_path = store_path + "/vs_store";
    Util::create_dir(vstree_store_path);

    cout << "begin encode RDF from : " << ret << " ..." << endl;

	//BETTER+TODO:now require that dataset size < memory
	//to support really larger datasets, divide and insert into B+ tree and VStree
	//(read the value, add list and set; update the signature, remove and reinsert)
	//the query process is nearly the same
	//QUERY:build each index in one thread to speed up, but the sorting case?
	//one sorting order for several indexes

    // to be switched to new encodeRDF method.
//    this->encodeRDF(ret);
    this->encodeRDF_new(ret);
    cout << "finish encode." << endl;
	//this->kvstore->flush();
	delete this->kvstore;
	this->kvstore = NULL;
	sync();
	cout <<"sync kvstore"<<endl;
	//this->kvstore->release();
	//cout<<"release kvstore"<<endl;

    //(this->kvstore)->open();
    string _entry_file = this->getSignatureBFile();

    cout << "begin build VS-Tree on " << ret << "..." << endl;
    (this->vstree)->buildTree(_entry_file);

    long tv_build_end = Util::get_cur_time();
    cout << "after build, used " << (tv_build_end - tv_build_begin) << "ms." << endl;
    cout << "finish build VS-Tree." << endl;

	//this->vstree->saveTree();
	delete this->vstree;
	this->vstree = NULL;
	sync();
	cout<<"sync vstree"<<endl;

	string cmd = "rm -rf " + _entry_file;
	system(cmd.c_str());
	cout<<"signature file removed"<<endl;

    return true;
}

 //root Path of this DB + sixTuplesFile 
string
Database::getSixTuplesFile()
{
    return this->getStorePath() + "/" + this->six_tuples_file;
}

/* root Path of this DB + signatureBFile */
string
Database::getSignatureBFile()
{
    return this->getStorePath() + "/" + this->signature_binary_file;
}

/* root Path of this DB + DBInfoFile */
string
Database::getDBInfoFile()
{
    return this->getStorePath() + "/" + this->db_info_file;
}

bool
Database::saveDBInfoFile()
{
    FILE* filePtr = fopen(this->getDBInfoFile().c_str(), "wb");

    if (filePtr == NULL)
    {
        cerr << "error, can not create db info file. @Database::saveDBInfoFile"  << endl;
        return false;
    }

    fseek(filePtr, 0, SEEK_SET);

    fwrite(&this->triples_num, sizeof(int), 1, filePtr);
    fwrite(&this->entity_num, sizeof(int), 1, filePtr);
    fwrite(&this->sub_num, sizeof(int), 1, filePtr);
    fwrite(&this->pre_num, sizeof(int), 1, filePtr);
    fwrite(&this->literal_num, sizeof(int), 1, filePtr);
    fwrite(&this->encode_mode, sizeof(int), 1, filePtr);
    fclose(filePtr);

	Util::triple_num = this->triples_num;
	Util::pre_num = this->pre_num;
	Util::entity_num = this->entity_num;
	Util::literal_num = this->literal_num;

    return true;
}

bool
Database::loadDBInfoFile()
{
    FILE* filePtr = fopen(this->getDBInfoFile().c_str(), "rb");

    if (filePtr == NULL)
    {
        cerr << "error, can not open db info file. @Database::loadDBInfoFile"  << endl;
        return false;
    }

    fseek(filePtr, 0, SEEK_SET);

    fread(&this->triples_num, sizeof(int), 1, filePtr);
    fread(&this->entity_num, sizeof(int), 1, filePtr);
    fread(&this->sub_num, sizeof(int), 1, filePtr);
    fread(&this->pre_num, sizeof(int), 1, filePtr);
    fread(&this->literal_num, sizeof(int), 1, filePtr);
    fread(&this->encode_mode, sizeof(int), 1, filePtr);
    fclose(filePtr);

	Util::triple_num = this->triples_num;
	Util::pre_num = this->pre_num;
	Util::entity_num = this->entity_num;
	Util::literal_num = this->literal_num;

    return true;
}

string
Database::getStorePath()
{
    return this->name;
}


//encode relative signature data of the query graph 
void
Database::buildSparqlSignature(SPARQLquery & _sparql_q)
{
    vector<BasicQuery*>& _query_union = _sparql_q.getBasicQueryVec();
    for(unsigned int i_bq = 0; i_bq < _query_union.size(); i_bq ++)
    {
        BasicQuery* _basic_q = _query_union[i_bq];
        _basic_q->encodeBasicQuery(this->kvstore, _sparql_q.getQueryVar());
    }
}



bool
Database::calculateEntityBitSet(int _sub_id, EntityBitSet & _bitset)
{
    int* _polist = NULL;
    int _list_len = 0;
    (this->kvstore)->getpreIDobjIDlistBysubID(_sub_id, _polist, _list_len);
    Triple _triple;
    _triple.subject = (this->kvstore)->getEntityByID(_sub_id);
    for(int i = 0; i < _list_len; i += 2)
    {
        int _pre_id = _polist[i];
        int _obj_id = _polist[i+1];
        _triple.object = (this->kvstore)->getEntityByID(_obj_id);
        if(_triple.object == "")
        {
            _triple.object = (this->kvstore)->getLiteralByID(_obj_id);
        }
        _triple.predicate = (this->kvstore)->getPredicateByID(_pre_id);
        this->encodeTriple2SubEntityBitSet(_bitset, &_triple);
    }
    return true;
}

//encode Triple into subject SigEntry 
bool
Database::encodeTriple2SubEntityBitSet(EntityBitSet& _bitset, const Triple* _p_triple)
{
    int _pre_id = -1;
    {
        _pre_id = (this->kvstore)->getIDByPredicate(_p_triple->predicate);
		 //BETTER: checking whether _pre_id is -1 or not will be more reliable 
    }

    Signature::encodePredicate2Entity(_pre_id, _bitset, Util::EDGE_OUT);
    if(this->encode_mode == Database::ID_MODE)
    {
		//TODO
    }
    else if(this->encode_mode == Database::STRING_MODE)
    {
        Signature::encodeStr2Entity( (_p_triple->object).c_str(), _bitset);
    }

    return true;
}

//encode Triple into object SigEntry 
bool
Database::encodeTriple2ObjEntityBitSet(EntityBitSet& _bitset, const Triple* _p_triple)
{
    int _pre_id = -1;
    {
        _pre_id = (this->kvstore)->getIDByPredicate(_p_triple->predicate);
		//BETTER: checking whether _pre_id is -1 or not will be more reliable 
    }

    Signature::encodePredicate2Entity(_pre_id, _bitset, Util::EDGE_IN);
    if(this->encode_mode == Database::ID_MODE)
    {
		//TODO
    }
    else if(this->encode_mode == Database::STRING_MODE)
    {
        Signature::encodeStr2Entity( (_p_triple->subject ).c_str(), _bitset);
    }

    return true;
}

//check whether the relative 3-tuples exist usually, through sp2olist 
bool
Database::exist_triple(int _sub_id, int _pre_id, int _obj_id)
{
    int* _objidlist = NULL;
    int _list_len = 0;
    (this->kvstore)->getobjIDlistBysubIDpreID(_sub_id, _pre_id, _objidlist, _list_len);

    bool is_exist = false;
//	for(int i = 0; i < _list_len; i ++)
//	{
//		if(_objidlist[i] == _obj_id)
//		{
//		    is_exist = true;
//			break;
//		}
//	}
    if (Util::bsearch_int_uporder(_obj_id, _objidlist, _list_len) != -1)
    {
        is_exist = true;
    }
    delete[] _objidlist;

    return is_exist;
}

//NOTICE: all constants are transfered to ids in memory
//this maybe not ok when size is too large!
bool
Database::encodeRDF_new(const string _rdf_file)
{
#ifdef DEBUG
	//cerr<< "now to log!!!" << endl;
    Util::logging("In encodeRDF_new");
	//cerr<< "end log!!!" << endl;
#endif
    int ** _p_id_tuples = NULL;
    int _id_tuples_max = 0;

	//map sub2id, pre2id, entity/literal in obj2id, store in kvstore, encode RDF data into signature 
    this->sub2id_pre2id_obj2id_RDFintoSignature(_rdf_file, _p_id_tuples, _id_tuples_max);

    /* map subid 2 objid_list  &
     * subIDpreID 2 objid_list &
     * subID 2 <preIDobjID>_list */
	//this->s2o_s2po_sp2o(_p_id_tuples, _id_tuples_max);

    /* map objid 2 subid_list  &
     * objIDpreID 2 subid_list &
     * objID 2 <preIDsubID>_list */
	//this->o2s_o2ps_op2s(_p_id_tuples, _id_tuples_max);

	//this->s2p_s2po_sp2o(_p_id_tuples, _id_tuples_max);
	//NOTICE:we had better compute the corresponding triple num here
	this->s2p_s2o_s2po_sp2o(_p_id_tuples, _id_tuples_max);
	//this->s2p_s2o_s2po_sp2o_sp2n(_p_id_tuples, _id_tuples_max);
	this->o2p_o2s_o2ps_op2s(_p_id_tuples, _id_tuples_max);
	//this->o2p_o2s_o2ps_op2s_op2n(_p_id_tuples, _id_tuples_max);
	this->p2s_p2o_p2so(_p_id_tuples, _id_tuples_max);
	//this->p2s_p2o_p2so_p2n(_p_id_tuples, _id_tuples_max);
	//
	//WARN:this is too costly because s-o key num is too large
	//100G+ for DBpedia2014
	//this->so2p_s2o(_p_id_tuples, _id_tuples_max);

	//WARN:we must free the memory for id_tuples array
	for(int i = 0; i < _id_tuples_max; ++i)
	{
		delete[] _p_id_tuples[i];
	}
	delete[] _p_id_tuples;

    bool flag = this->saveDBInfoFile();
    if (!flag)
    {
        return false;
    }

    Util::logging("finish encodeRDF_new");

    return true;
}

bool
Database::sub2id_pre2id_obj2id_RDFintoSignature(const string _rdf_file, int**& _p_id_tuples, int & _id_tuples_max)
{
    int _id_tuples_size;
    {   /* initial */
        _id_tuples_max = 10*1000*1000;
        _p_id_tuples = new int*[_id_tuples_max];
        _id_tuples_size = 0;
        this->sub_num = 0;
        this->pre_num = 0;
        this->entity_num = 0;
        this->literal_num = 0;
        this->triples_num = 0;
        (this->kvstore)->open_entity2id(KVstore::CREATE_MODE);
        (this->kvstore)->open_id2entity(KVstore::CREATE_MODE);
        (this->kvstore)->open_predicate2id(KVstore::CREATE_MODE);
        (this->kvstore)->open_id2predicate(KVstore::CREATE_MODE);
        (this->kvstore)->open_literal2id(KVstore::CREATE_MODE);
        (this->kvstore)->open_id2literal(KVstore::CREATE_MODE);
    }

    Util::logging("finish initial sub2id_pre2id_obj2id");


    ifstream _fin(_rdf_file.c_str());
    if(!_fin) {
        cerr << "sub2id&pre2id&obj2id: Fail to open : " << _rdf_file << endl;
        exit(0);
    }

    string _six_tuples_file = this->getSixTuplesFile();
    ofstream _six_tuples_fout(_six_tuples_file.c_str());
    if(! _six_tuples_fout) 
	{
        cerr << "sub2id&pre2id&obj2id: Fail to open: " << _six_tuples_file << endl;
        exit(0);
    }

    TripleWithObjType* triple_array = new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP];

    /*	don't know the number of entity
    *	pre allocate entitybitset_max EntityBitSet for storing signature, double the space until the _entity_bitset is used up.
    */
    int entitybitset_max = 10000;
    EntityBitSet** _entity_bitset = new EntityBitSet* [entitybitset_max];
    for(int i = 0; i < entitybitset_max; i ++)
    {
        _entity_bitset[i] = new EntityBitSet();
        _entity_bitset[i] -> reset();
    }
    EntityBitSet _tmp_bitset;

    //parse a file
    RDFParser _parser(_fin);

    Util::logging("==> while(true)");

    while(true)
    {
        int parse_triple_num = 0;

        _parser.parseFile(triple_array, parse_triple_num);

        {
            stringstream _ss;
            _ss << "finish rdfparser" << this->triples_num << endl;
            Util::logging(_ss.str());
            cout << _ss.str() << endl;
        }
        if(parse_triple_num == 0) {
            break;
        }

        /* Process the Triple one by one */
        for(int i = 0; i < parse_triple_num; i ++)
        {
            this->triples_num ++;

            /* if the _id_tuples exceeds, double the space */
            if(_id_tuples_size == _id_tuples_max) {
                int _new_tuples_len = _id_tuples_max * 2;
                int** _new_id_tuples = new int*[_new_tuples_len];
                memcpy(_new_id_tuples, _p_id_tuples, sizeof(int*) * _id_tuples_max);
                delete[] _p_id_tuples;
                _p_id_tuples = _new_id_tuples;
                _id_tuples_max = _new_tuples_len;
            }

            // For subject
            // (all subject is entity, some object is entity, the other is literal)
            string _sub = triple_array[i].getSubject();
            int _sub_id = (this->kvstore)->getIDByEntity(_sub);
            if(_sub_id == -1) 
			{
                _sub_id = this->entity_num;
                (this->kvstore)->setIDByEntity(_sub, _sub_id);
                (this->kvstore)->setEntityByID(_sub_id, _sub);
                this->entity_num ++;
            }
            //  For predicate
            string _pre = triple_array[i].getPredicate();
            int _pre_id = (this->kvstore)->getIDByPredicate(_pre);
            if(_pre_id == -1) 
			{
                _pre_id = this->pre_num;
                (this->kvstore)->setIDByPredicate(_pre, _pre_id);
                (this->kvstore)->setPredicateByID(_pre_id, _pre);
                this->pre_num ++;
            }

            //  For object
            string _obj = triple_array[i].getObject();
            int _obj_id = -1;
            // obj is entity
            if (triple_array[i].isObjEntity())
            {
                _obj_id = (this->kvstore)->getIDByEntity(_obj);
                if(_obj_id == -1)
                {
                    _obj_id = this->entity_num;
                    (this->kvstore)->setIDByEntity(_obj, _obj_id);
                    (this->kvstore)->setEntityByID(_obj_id, _obj);
                    this->entity_num ++;
                }
            }
            //obj is literal
            if (triple_array[i].isObjLiteral())
            {
                _obj_id = (this->kvstore)->getIDByLiteral(_obj);
                if(_obj_id == -1)
                {
                    _obj_id = Util::LITERAL_FIRST_ID + (this->literal_num);
                    (this->kvstore)->setIDByLiteral(_obj, _obj_id);
                    (this->kvstore)->setLiteralByID(_obj_id, _obj);
                    this->literal_num ++;
//#ifdef DEBUG
					//if(_obj == "\"Bob\"")
					//{
						//cout << "this is id for Bob: " << _obj_id << endl;
					//}
					//cout<<"literal should be bob: " << kvstore->getLiteralByID(_obj_id)<<endl;
					//cout<<"id for bob: "<<kvstore->getIDByLiteral("\"Bob\"")<<endl;
//#endif
                }
            }

            //  For id_tuples
            _p_id_tuples[_id_tuples_size] = new int[3];
            _p_id_tuples[_id_tuples_size][0] = _sub_id;
            _p_id_tuples[_id_tuples_size][1] = _pre_id;
            _p_id_tuples[_id_tuples_size][2] = _obj_id;
            _id_tuples_size ++;

#ifdef DEBUG_PRECISE
            //  save six tuples
            {
                _six_tuples_fout << _sub_id << '\t'
                                 << _pre_id << '\t'
                                 << _obj_id << '\t'
                                 << _sub	<< '\t'
                                 << _pre << '\t'
                                 << _obj	<< endl;
            }
#endif
            //_entity_bitset is used up, double the space
            if (this->entity_num >= entitybitset_max)
            {
                EntityBitSet** _new_entity_bitset = new EntityBitSet* [entitybitset_max * 2];
                memcpy(_new_entity_bitset, _entity_bitset, sizeof(EntityBitSet*) * entitybitset_max);
                delete[] _entity_bitset;
                _entity_bitset = _new_entity_bitset;

                for(int i = entitybitset_max; i < entitybitset_max * 2; i ++)
                {
                    _entity_bitset[i] = new EntityBitSet();
                    _entity_bitset[i] -> reset();
                }

                entitybitset_max *= 2;
            }

            {
                _tmp_bitset.reset();
                Signature::encodePredicate2Entity(_pre_id, _tmp_bitset, Util::EDGE_OUT);
                Signature::encodeStr2Entity(_obj.c_str(), _tmp_bitset);
                *_entity_bitset[_sub_id] |= _tmp_bitset;
            }

            if(triple_array[i].isObjEntity())
            {
                _tmp_bitset.reset();
                Signature::encodePredicate2Entity(_pre_id, _tmp_bitset, Util::EDGE_IN);
                Signature::encodeStr2Entity(_sub.c_str(), _tmp_bitset);
                *_entity_bitset[_obj_id] |= _tmp_bitset;
            }
        }
    }

    Util::logging("==> end while(true)");

    delete[] triple_array;
    _fin.close();
    _six_tuples_fout.close();


    {   /* save all entity_signature into binary file */
        string _sig_binary_file = this->getSignatureBFile();
        FILE* _sig_fp = fopen(_sig_binary_file.c_str(), "wb");
        if(_sig_fp == NULL) {
            cerr << "Failed to open : " << _sig_binary_file << endl;
        }

        EntityBitSet _all_bitset;
        for(int i = 0; i < this->entity_num; i ++)
        {
            SigEntry* _sig = new SigEntry(EntitySig(*_entity_bitset[i]), i);

            fwrite(_sig, sizeof(SigEntry), 1, _sig_fp);
            _all_bitset |= *_entity_bitset[i];
            delete _sig;
        }
        fclose(_sig_fp);

        for (int i = 0; i < entitybitset_max; i++)
        {
            delete _entity_bitset[i];
        }
        delete[] _entity_bitset;
    }

    {
        stringstream _ss;
        _ss << "finish sub2id pre2id obj2id" << endl;
        _ss << "tripleNum is " << this->triples_num << endl;
        _ss << "entityNum is " << this->entity_num << endl;
        _ss << "preNum is " << this->pre_num << endl;
        _ss << "literalNum is " << this->literal_num << endl;
        Util::logging(_ss.str());
        cout << _ss.str() << endl;
    }

    return true;
}


/*
 *	only after we determine the entityID(subid),
 *	we can determine the literalID(objid)
 */
bool
Database::sub2id_pre2id(const string _rdf_file, int**& _p_id_tuples, int & _id_tuples_max)
{
    int _id_tuples_size;;
    {   /* initial */
        _id_tuples_max = 10*1000*1000;
        _p_id_tuples = new int*[_id_tuples_max];
        _id_tuples_size = 0;
        this->sub_num = 0;
        this->pre_num = 0;
        this->triples_num = 0;
        (this->kvstore)->open_entity2id(KVstore::CREATE_MODE);
        (this->kvstore)->open_id2entity(KVstore::CREATE_MODE);
        (this->kvstore)->open_predicate2id(KVstore::CREATE_MODE);
        (this->kvstore)->open_id2predicate(KVstore::CREATE_MODE);
    }

    Util::logging("finish initial sub2id_pre2id");
    {   /* map sub2id and pre2id */
        ifstream _fin(_rdf_file.c_str());
        if(!_fin) {
            cerr << "sub2id&pre2id: Fail to open : " << _rdf_file << endl;
            exit(0);
        }

        Triple* triple_array = new Triple[DBparser::TRIPLE_NUM_PER_GROUP];

        DBparser _parser;
        /* In while(true): For sub2id and pre2id.
         * parsed all RDF triples one group by one group
         * when parsed out an group RDF triples
         * for each triple
         * assign subject with subid, and predicate with preid
         * when get all sub2id,
         * we can assign object with objid in next while(true)
         * so that we can differentiate subject and object by their id
         *  */
        Util::logging("==> while(true)");
        while(true)
        {
            int parse_triple_num = 0;
            _parser.rdfParser(_fin, triple_array, parse_triple_num);
            {
                stringstream _ss;
                _ss << "finish rdfparser" << this->triples_num << endl;
                Util::logging(_ss.str());
                cout << _ss.str() << endl;
            }
            if(parse_triple_num == 0) {
                break;
            }

            /* Process the Triple one by one */
            for(int i = 0; i < parse_triple_num; i ++)
            {
                this->triples_num ++;

                /* if the _id_tuples exceeds, double the space */
                if(_id_tuples_size == _id_tuples_max) {
                    int _new_tuples_len = _id_tuples_max * 2;
                    int** _new_id_tuples = new int*[_new_tuples_len];
                    memcpy(_new_id_tuples, _p_id_tuples, sizeof(int*) * _id_tuples_max);
                    delete[] _p_id_tuples;
                    _p_id_tuples = _new_id_tuples;
                    _id_tuples_max = _new_tuples_len;
                }

                /*
                 * For subject
                 * (all subject is entity, some object is entity, the other is literal)
                 * */
                string _sub = triple_array[i].subject;
                int _sub_id = (this->kvstore)->getIDByEntity(_sub);
                if(_sub_id == -1) {
                    _sub_id = this->sub_num;
                    (this->kvstore)->setIDByEntity(_sub, _sub_id);
                    (this->kvstore)->setEntityByID(_sub_id, _sub);
                    this->sub_num ++;
                }
                /*
                 * For predicate
                 * */
                string _pre = triple_array[i].predicate;
                int _pre_id = (this->kvstore)->getIDByPredicate(_pre);
                if(_pre_id == -1) {
                    _pre_id = this->pre_num;
                    (this->kvstore)->setIDByPredicate(_pre, _pre_id);
                    (this->kvstore)->setPredicateByID(_pre_id, _pre);
                    this->pre_num ++;
                }

                /*
                 * For id_tuples
                 */
                _p_id_tuples[_id_tuples_size] = new int[3];
                _p_id_tuples[_id_tuples_size][0] = _sub_id;
                _p_id_tuples[_id_tuples_size][1] = _pre_id;
                _p_id_tuples[_id_tuples_size][2] = -1;
                _id_tuples_size ++;
            }
        }/* end while(true) for sub2id and pre2id */
        delete[] triple_array;
        _fin.close();
    }

    {   /* final process */
        this->entity_num = this->sub_num;
    }

    {
        stringstream _ss;
        _ss << "finish sub2id pre2id" << endl;
        _ss << "tripleNum is " << this->triples_num << endl;
        _ss << "subNum is " << this->sub_num << endl;
        _ss << "preNum is " << this->pre_num << endl;
        Util::logging(_ss.str());
        cout << _ss.str() << endl;
    }

    return true;
}

//NOTICE: below are the the new ones
bool
Database::s2p_s2o_s2po_sp2o(int** _p_id_tuples, int _id_tuples_max)
//Database::s2p_s2o_s2po_sp2o_sp2n(int** _p_id_tuples, int _id_tuples_max)
{
    qsort(_p_id_tuples, this->triples_num, sizeof(int*), Database:: _spo_cmp);

	int* _oidlist_s = NULL;
	int* _pidlist_s = NULL;
    int* _oidlist_sp = NULL;
    int* _pidoidlist_s = NULL;

	int _oidlist_s_len = 0;
	int _pidlist_s_len = 0;
    int _oidlist_sp_len = 0;
    int _pidoidlist_s_len = 0;

    // only _oidlist_s will be assigned with space, _oidlist_sp is always a part of _oidlist_s, just a pointer is enough
    int _pidlist_max = 0;
    int _pidoidlist_max = 0;
	int _oidlist_max = 0;

	//true means next sub is a different one from the previous one 
    bool _sub_change = true;
	//true means next <sub,pre> is different from the previous pair 
    bool _sub_pre_change = true;
	//true means next pre is different from the previous one 
    bool _pre_change = true;

    Util::logging("finish s2p_sp2o_s2po initial");

    (this->kvstore)->open_subID2objIDlist(KVstore::CREATE_MODE);
    (this->kvstore)->open_subID2preIDlist(KVstore::CREATE_MODE);
    (this->kvstore)->open_subIDpreID2objIDlist(KVstore::CREATE_MODE);
    (this->kvstore)->open_subID2preIDobjIDlist(KVstore::CREATE_MODE);

	//(this->kvstore)->open_subIDpreID2num(KVstore::CREATE_MODE);

    for(int i = 0; i < this->triples_num;i++)
    if (i + 1 == this->triples_num || (_p_id_tuples[i][0] != _p_id_tuples[i + 1][0] || _p_id_tuples[i][1] != _p_id_tuples[i + 1][1] || _p_id_tuples[i][2] != _p_id_tuples[i + 1][2]))
    {
		//cout<<"this is the "<<i<<"th loop"<<endl;
        if(_sub_change)
        {
			//cout<<"subject changed!"<<endl;
			//
			//QUERY:pidlist and oidlist maybe different size, not always use 1000 as start?
			//
			//pidlist 
            _pidlist_max = 1000;
            _pidlist_s = new int[_pidlist_max];
            _pidlist_s_len = 0;
			//pidoidlist 
            //_pidoidlist_max = 1000 * 2;
            //_pidoidlist_s = new int[_pidoidlist_max];
            //_pidoidlist_s_len = 0;

			//NOTICE:when debug, can change 1000 to 10 to discover problem
			 //oidlist 
            _oidlist_max = 1000;
            _oidlist_s = new int[_oidlist_max];
            _oidlist_sp = _oidlist_s;
            _oidlist_s_len = 0;
            _oidlist_sp_len = 0;
            /* pidoidlist */
            _pidoidlist_max = 1000 * 2;
            _pidoidlist_s = new int[_pidoidlist_max];
            _pidoidlist_s_len = 0;
        }
		//if(_sub_pre_change)
		//{
			////oidlist
			//_oidlist_max = 1000;
            //_oidlist_sp = new int[_oidlist_max];
            //_oidlist_sp_len = 0;
		//}

		//enlarge the space when needed 
        if(_pidlist_s_len == _pidlist_max)
        {
            _pidlist_max *= 10;
            int* _new_pidlist_s = new int[_pidlist_max];
            memcpy(_new_pidlist_s, _pidlist_s, sizeof(int) * _pidlist_s_len);
            delete[] _pidlist_s;
            _pidlist_s = _new_pidlist_s;
        }

		 //enlarge the space when needed 
        if(_oidlist_s_len == _oidlist_max)
        {
			//cout<<"s2o need to enlarge"<<endl;
            _oidlist_max *= 10;
            int * _new_oidlist_s = new int[_oidlist_max];
            memcpy(_new_oidlist_s, _oidlist_s, sizeof(int) * _oidlist_s_len);
            /* (_oidlist_sp-_oidlist_s) is the offset of _oidlist_sp */
            _oidlist_sp = _new_oidlist_s + (_oidlist_sp-_oidlist_s);
            delete[] _oidlist_s;
            _oidlist_s = _new_oidlist_s;
        }
		//enalrge the space when needed
		//if(_oidlist_sp_len == _oidlist_max)
		//{
		//	_oidlist_max *= 10;
		//	int* _new_oidlist_sp = new int[_oidlist_max];
		//	memcpy(_new_oidlist_sp, _oidlist_sp, sizeof(int) * _oidlist_sp_len);
		//	delete[] _oidlist_sp;
		//	_oidlist_sp = _new_oidlist_sp;
		//}

		//enlarge the space when needed 
        if(_pidoidlist_s_len == _pidoidlist_max)
        {
            _pidoidlist_max *= 10;
            int* _new_pidoidlist_s = new int[_pidoidlist_max];
            memcpy(_new_pidoidlist_s, _pidoidlist_s, sizeof(int) * _pidoidlist_s_len);
            delete[] _pidoidlist_s;
            _pidoidlist_s = _new_pidoidlist_s;
        }

        int _sub_id = _p_id_tuples[i][0];
        int _pre_id = _p_id_tuples[i][1];
        int _obj_id = _p_id_tuples[i][2];
//		{
//			stringstream _ss;
//			_ss << _sub_id << "\t" << _pre_id << "\t" << _obj_id << endl;
//			Util::logging(_ss.str());
//		}

        _oidlist_s[_oidlist_s_len] = _obj_id;
        if(_sub_pre_change) 
		{
            _oidlist_sp = _oidlist_s + _oidlist_s_len;
        }
        _oidlist_s_len ++;
        _oidlist_sp_len ++;

		//add objid to list
		//_oidlist_sp[_oidlist_sp_len++] = _obj_id;
		//WARN:this is wrong!

		//add <preid, objid> to list 
        _pidoidlist_s[_pidoidlist_s_len] = _pre_id;
        _pidoidlist_s[_pidoidlist_s_len+1] = _obj_id;
        _pidoidlist_s_len += 2;

		//whether sub in new triple changes or not 
        _sub_change = (i+1 == this->triples_num) ||
                      (_p_id_tuples[i][0] != _p_id_tuples[i+1][0]);

		//whether pre in new triple changes or not 
        _pre_change = (i+1 == this->triples_num) ||
                      (_p_id_tuples[i][1] != _p_id_tuples[i+1][1]);

		//whether <sub,pre> in new triple changes or not 
        _sub_pre_change = _sub_change || _pre_change;

        if(_sub_pre_change)
        {
			//add preid to list 
			_pidlist_s[_pidlist_s_len++] = _pre_id;

			//map subid&preid 2 triple num
			//(this->kvstore)->setNumBysubIDpreID(_sub_id, _pre_id, _oidlist_sp_len);
			//
			//map subid&preid 2 objidlist
            (this->kvstore)->setobjIDlistBysubIDpreID(_sub_id, _pre_id, _oidlist_sp, _oidlist_sp_len);
			//if not use s2o memory
			//delete[] _oidlist_sp;
            _oidlist_sp = NULL;
            _oidlist_sp_len = 0;
        }

        if(_sub_change)
        {
			(this->kvstore)->setpreIDlistBysubID(_sub_id, _pidlist_s, _pidlist_s_len);
            delete[] _pidlist_s;
            _pidlist_s = NULL;
            _pidlist_s_len = 0;
			//map subid 2 preid&objidlist 
            (this->kvstore)->setpreIDobjIDlistBysubID(_sub_id, _pidoidlist_s, _pidoidlist_s_len);
            delete[] _pidoidlist_s;
            _pidoidlist_s = NULL;
            _pidoidlist_s_len = 0;

            Util::sort(_oidlist_s, _oidlist_s_len);
			_oidlist_s_len = Util::removeDuplicate(_oidlist_s, _oidlist_s_len);
            (this->kvstore)->setobjIDlistBysubID(_sub_id, _oidlist_s, _oidlist_s_len);
            delete[] _oidlist_s;
            _oidlist_sp = _oidlist_s = NULL;
            _oidlist_sp_len = _oidlist_s_len = 0;
        }

    }//end for( 0 to this->triple_num)


    Util::logging("OUT s2po...");

	return true;
}

bool
Database::o2p_o2s_o2ps_op2s(int** _p_id_tuples, int _id_tuples_max)
//Database::o2p_o2s_o2ps_op2s_op2n(int** _p_id_tuples, int _id_tuples_max)
{
    Util::logging("IN o2ps...");

    qsort(_p_id_tuples, this->triples_num, sizeof(int**), Database::_ops_cmp);
    int* _sidlist_o = NULL;
    int* _sidlist_op = NULL;
    int* _pidsidlist_o = NULL;
	int* _pidlist_o = NULL;
    int _sidlist_o_len = 0;
    int _sidlist_op_len = 0;
    int _pidsidlist_o_len = 0;
	int _pidlist_o_len = 0;

    //only _sidlist_o will be assigned with space _sidlist_op is always a part of _sidlist_o just a pointer is enough 
    int _sidlist_max = 0;
    int _pidsidlist_max = 0;
	int _pidlist_max = 0;

	//true means next obj is a different one from the previous one 
    bool _obj_change = true;

	//true means next pre is a different one from the previous one 
    bool _pre_change = true;

	//true means next <obj,pre> is different from the previous pair 
    bool _obj_pre_change = true;

    (this->kvstore)->open_objID2subIDlist(KVstore::CREATE_MODE);
    (this->kvstore)->open_objIDpreID2subIDlist(KVstore::CREATE_MODE);
    (this->kvstore)->open_objID2preIDsubIDlist(KVstore::CREATE_MODE);
    (this->kvstore)->open_objID2preIDlist(KVstore::CREATE_MODE);

	//(this->kvstore)->open_objIDpreID2num(KVstore::CREATE_MODE);

    for(int i = 0; i < this->triples_num; i ++)
    if (i + 1 == this->triples_num || (_p_id_tuples[i][0] != _p_id_tuples[i + 1][0] || _p_id_tuples[i][1] != _p_id_tuples[i + 1][1] || _p_id_tuples[i][2] != _p_id_tuples[i + 1][2]))
    {
        if(_obj_change)
        {
			 //sidlist 
            _sidlist_max = 1000;
            _sidlist_o = new int[_sidlist_max];
            _sidlist_op = _sidlist_o;
            _sidlist_o_len = 0;
            _sidlist_op_len = 0;
			 //pidsidlist 
            _pidsidlist_max = 1000 * 2;
            _pidsidlist_o = new int[_pidsidlist_max];
            _pidsidlist_o_len = 0;
			//pidlist
			_pidlist_max = 1000;
			_pidlist_o = new int[_pidlist_max];
			_pidlist_o_len = 0;
        }

		 //enlarge the space when needed 
        if(_sidlist_o_len == _sidlist_max)
        {
            _sidlist_max *= 10;
            int * _new_sidlist_o = new int[_sidlist_max];
            memcpy(_new_sidlist_o, _sidlist_o, sizeof(int)*_sidlist_o_len);
            /* (_sidlist_op-_sidlist_o) is the offset of _sidlist_op */
            _sidlist_op = _new_sidlist_o + (_sidlist_op-_sidlist_o);
            delete[] _sidlist_o;
            _sidlist_o = _new_sidlist_o;
        }

		 //enlarge the space when needed 
        if(_pidsidlist_o_len == _pidsidlist_max)
        {
            _pidsidlist_max *= 10;
            int* _new_pidsidlist_o = new int[_pidsidlist_max];
            memcpy(_new_pidsidlist_o, _pidsidlist_o, sizeof(int) * _pidsidlist_o_len);
            delete[] _pidsidlist_o;
            _pidsidlist_o = _new_pidsidlist_o;
        }

		//enlarge the space when needed
		if(_pidlist_o_len == _pidlist_max)
		{
			_pidlist_max *= 10;
			int* _new_pidlist_o = new int[_pidlist_max];
			memcpy(_new_pidlist_o, _pidlist_o, sizeof(int) * _pidlist_o_len);
			delete[] _pidlist_o;
			_pidlist_o = _new_pidlist_o;
		}

        int _sub_id = _p_id_tuples[i][0];
        int _pre_id = _p_id_tuples[i][1];
        int _obj_id = _p_id_tuples[i][2];

		 //add subid to list 
        _sidlist_o[_sidlist_o_len] = _sub_id;

		 //if <objid, preid> changes, _sidlist_op should be adjusted 
        if(_obj_pre_change) 
		{
            _sidlist_op = _sidlist_o + _sidlist_o_len;
        }

        _sidlist_o_len ++;
        _sidlist_op_len ++;

		//add <preid, subid> to list 
        _pidsidlist_o[_pidsidlist_o_len] = _pre_id;
        _pidsidlist_o[_pidsidlist_o_len+1] = _sub_id;;
        _pidsidlist_o_len += 2;

		 //whether sub in new triple changes or not 
        _obj_change = (i+1 == this->triples_num) ||
                      (_p_id_tuples[i][2] != _p_id_tuples[i+1][2]);

		 //whether pre in new triple changes or not 
        _pre_change = (i+1 == this->triples_num) ||
                      (_p_id_tuples[i][1] != _p_id_tuples[i+1][1]);

		 //whether <sub,pre> in new triple changes or not 
        _obj_pre_change = _obj_change || _pre_change;

        if(_obj_pre_change)
        {
			//add preid to list
			_pidlist_o[_pidlist_o_len++] = _pre_id;

			//map objid&preid 2 triple num
			//(this->kvstore)->setNumByobjIDpreID(_obj_id, _pre_id, _sidlist_op_len);
			//
			//map objid&preid 2 subidlist
            (this->kvstore)->setsubIDlistByobjIDpreID(_obj_id, _pre_id, _sidlist_op, _sidlist_op_len);
            _sidlist_op = NULL;
            _sidlist_op_len = 0;
        }

        if(_obj_change)
        {
			 //map objid 2 subidlist 
            Util::sort(_sidlist_o, _sidlist_o_len);
			_sidlist_o_len = Util::removeDuplicate(_sidlist_o, _sidlist_o_len);
            (this->kvstore)->setsubIDlistByobjID(_obj_id, _sidlist_o, _sidlist_o_len);
            delete[] _sidlist_o;
            _sidlist_o = NULL;
            _sidlist_op = NULL;
            _sidlist_o_len = 0;

			 //map objid 2 preid&subidlist 
            (this->kvstore)->setpreIDsubIDlistByobjID(_obj_id, _pidsidlist_o, _pidsidlist_o_len);
            delete[] _pidsidlist_o;
            _pidsidlist_o = NULL;
            _pidsidlist_o_len = 0;

			//map objid 2 preidlist
            (this->kvstore)->setpreIDlistByobjID(_obj_id, _pidlist_o, _pidlist_o_len);
            delete[] _pidlist_o;
            _pidlist_o = NULL;
            _pidlist_o_len = 0;
        }

    }//end for( 0 to this->triple_num)


    Util::logging("OUT o2ps...");

	return true;
}

bool
Database::p2s_p2o_p2so(int** _p_id_tuples, int _id_tuples_max)
//Database::p2s_p2o_p2so_p2n(int** _p_id_tuples, int _id_tuples_max)
{
    qsort(_p_id_tuples, this->triples_num, sizeof(int*), Database:: _pso_cmp);
	int* _sidlist_p = NULL;
    int* _oidlist_p = NULL;
    int* _sidoidlist_p = NULL;
	int _sidlist_p_len = 0;
    int _oidlist_p_len = 0;
    int _sidoidlist_p_len = 0;
    int _sidlist_max = 0;
    int _sidoidlist_max = 0;
	int _oidlist_max = 0;

	//true means next pre is different from the previous one 
    bool _pre_change = true;
	bool _sub_change = true;
	bool _pre_sub_change = true;

    Util::logging("finish p2s_p2o_p2so initial");

    (this->kvstore)->open_preID2subIDlist(KVstore::CREATE_MODE);
    (this->kvstore)->open_preID2objIDlist(KVstore::CREATE_MODE);
    (this->kvstore)->open_preID2subIDobjIDlist(KVstore::CREATE_MODE);

	//(this->kvstore)->open_preID2num(KVstore::CREATE_MODE);

    for(int i = 0; i < this->triples_num;i++)
    if (i + 1 == this->triples_num || (_p_id_tuples[i][0] != _p_id_tuples[i + 1][0] || _p_id_tuples[i][1] != _p_id_tuples[i + 1][1] || _p_id_tuples[i][2] != _p_id_tuples[i + 1][2]))
    {
        if(_pre_change)
        {
			//sidlist 
            _sidlist_max = 1000;
            _sidlist_p = new int[_sidlist_max];
            _sidlist_p_len = 0;
			//sidoidlist 
            _sidoidlist_max = 1000 * 2;
            _sidoidlist_p = new int[_sidoidlist_max];
            _sidoidlist_p_len = 0;
			//oidlist
			_oidlist_max = 1000;
            _oidlist_p = new int[_oidlist_max];
            _oidlist_p_len = 0;
        }

		//enlarge the space when needed 
        if(_sidlist_p_len == _sidlist_max)
        {
            _sidlist_max *= 10;
            int* _new_sidlist_p = new int[_sidlist_max];
            memcpy(_new_sidlist_p, _sidlist_p, sizeof(int) * _sidlist_p_len);
            delete[] _sidlist_p;
            _sidlist_p = _new_sidlist_p;
        }

		//enalrge the space when needed
		if(_oidlist_p_len == _oidlist_max)
		{
			_oidlist_max *= 10;
			int* _new_oidlist_p = new int[_oidlist_max];
			memcpy(_new_oidlist_p, _oidlist_p, sizeof(int) * _oidlist_p_len);
			delete[] _oidlist_p;
			_oidlist_p = _new_oidlist_p;
		}

		//enlarge the space when needed 
        if(_sidoidlist_p_len == _sidoidlist_max)
        {
            _sidoidlist_max *= 10;
            int* _new_sidoidlist_p = new int[_sidoidlist_max];
            memcpy(_new_sidoidlist_p, _sidoidlist_p, sizeof(int) * _sidoidlist_p_len);
            delete[] _sidoidlist_p;
            _sidoidlist_p = _new_sidoidlist_p;
        }

        int _sub_id = _p_id_tuples[i][0];
        int _pre_id = _p_id_tuples[i][1];
        int _obj_id = _p_id_tuples[i][2];
//		{
//			stringstream _ss;
//			_ss << _sub_id << "\t" << _pre_id << "\t" << _obj_id << endl;
//			Util::logging(_ss.str());
//		}

		//add objid to list
		_oidlist_p[_oidlist_p_len++] = _obj_id;

		//add <subid, objid> to list 
        _sidoidlist_p[_sidoidlist_p_len] = _sub_id;
        _sidoidlist_p[_sidoidlist_p_len+1] = _obj_id;
        _sidoidlist_p_len += 2;

		//whether sub in new triple changes or not 
        _pre_change = (i+1 == this->triples_num) ||
                      (_p_id_tuples[i][1] != _p_id_tuples[i+1][1]);
        _sub_change = (i+1 == this->triples_num) ||
                      (_p_id_tuples[i][0] != _p_id_tuples[i+1][0]);
		_pre_sub_change = _pre_change || _sub_change;

		if(_pre_sub_change)
		{
			//add subid to list 
			_sidlist_p[_sidlist_p_len++] = _sub_id;
		}

        if(_pre_change)
        {
			//map preid 2 subidlist
			(this->kvstore)->setsubIDlistBypreID(_pre_id, _sidlist_p, _sidlist_p_len);
            delete[] _sidlist_p;
            _sidlist_p = NULL;
            _sidlist_p_len = 0;
			//map preid 2 objidlist
            Util::sort(_oidlist_p, _oidlist_p_len);
			_oidlist_p_len = Util::removeDuplicate(_oidlist_p, _oidlist_p_len);
			(this->kvstore)->setobjIDlistBypreID(_pre_id, _oidlist_p, _oidlist_p_len);
            delete[] _oidlist_p;
            _oidlist_p = NULL;
            _oidlist_p_len = 0;
			//map preid 2 triple num
			//(this->kvstore)->setNumBypreID(_pre_id, _sidoidlist_p_len/2);
			//
			//map preid 2 subid&objidlist 
            (this->kvstore)->setsubIDobjIDlistBypreID(_pre_id, _sidoidlist_p, _sidoidlist_p_len);
            delete[] _sidoidlist_p;
            _sidoidlist_p = NULL;
            _sidoidlist_p_len = 0;
        }

    }//end for( 0 to this->triple_num)

    Util::logging("OUT p2so...");
	return true;
}

bool
Database::so2p_s2o(int** _p_id_tuples, int _id_tuples_max)
{
    qsort(_p_id_tuples, this->triples_num, sizeof(int*), Database:: _sop_cmp);
	int* _pidlist_so = NULL;
    int* _oidlist_s = NULL;
	int _pidlist_so_len = 0;
    int _oidlist_s_len = 0;
    int _pidlist_max = 0;
    int _oidlist_max = 0;

	//true means next sub is a different one from the previous one 
    bool _sub_change = true;
	//true means next <sub,obj> is different from the previous pair 
    bool _sub_obj_change = true;
	//true means next pre is different from the previous one 
    bool _obj_change = true;

	bool _pre_change = true;
	bool _sub_obj_pre_change = true;

    Util::logging("finish so2p_s2o initial");

    (this->kvstore)->open_subIDobjID2preIDlist(KVstore::CREATE_MODE);
    (this->kvstore)->open_subID2objIDlist(KVstore::CREATE_MODE);

    for(int i = 0; i < this->triples_num;i++)
    if (i + 1 == this->triples_num || (_p_id_tuples[i][0] != _p_id_tuples[i + 1][0] || _p_id_tuples[i][1] != _p_id_tuples[i + 1][1] || _p_id_tuples[i][2] != _p_id_tuples[i + 1][2]))
    {
        if(_sub_change)
        {
			//oidlist 
            _oidlist_max = 1000 * 2;
            _oidlist_s = new int[_oidlist_max];
            _oidlist_s_len = 0;
        }
		if(_sub_obj_change)
		{
			//pidlist 
            _pidlist_max = 1000;
            _pidlist_so = new int[_pidlist_max];
            _pidlist_so_len = 0;
		}

		//enlarge the space when needed 
        if(_pidlist_so_len == _pidlist_max)
        {
            _pidlist_max *= 10;
            int* _new_pidlist_so = new int[_pidlist_max];
            memcpy(_new_pidlist_so, _pidlist_so, sizeof(int) * _pidlist_so_len);
            delete[] _pidlist_so;
            _pidlist_so = _new_pidlist_so;
        }

		//enalrge the space when needed
		if(_oidlist_s_len == _oidlist_max)
		{
			_oidlist_max *= 10;
			int* _new_oidlist_s = new int[_oidlist_max];
			memcpy(_new_oidlist_s, _oidlist_s, sizeof(int) * _oidlist_s_len);
			delete[] _oidlist_s;
			_oidlist_s = _new_oidlist_s;
		}

        int _sub_id = _p_id_tuples[i][0];
        int _pre_id = _p_id_tuples[i][1];
        int _obj_id = _p_id_tuples[i][2];
#ifdef DEBUG
		//cout << kvstore->getEntityByID(_sub_id) << endl;
		//cout << kvstore->getPredicateByID(_pre_id) << endl;
		//if(_obj_id < Util::LITERAL_FIRST_ID)
			//cout<<kvstore->getEntityByID(_obj_id)<<endl;
		//else
			//cout<<kvstore->getLiteralByID(_obj_id)<<endl;
#endif

		//whether sub in new triple changes or not 
        _sub_change = (i+1 == this->triples_num) ||
                      (_p_id_tuples[i][0] != _p_id_tuples[i+1][0]);

		//whether pre in new triple changes or not 
        _obj_change = (i+1 == this->triples_num) ||
                      (_p_id_tuples[i][2] != _p_id_tuples[i+1][2]);

		_pre_change = (i+1 == this->triples_num) ||
					  (_p_id_tuples[i][1] != _p_id_tuples[i+1][1]);

		//whether <sub,pre> in new triple changes or not 
        _sub_obj_change = _sub_change || _obj_change;

		_sub_obj_pre_change = _sub_obj_change || _pre_change;

		if(_sub_obj_pre_change)
		{
			//cout << "add pre to so2p" << endl;
			//add preid to list
			_pidlist_so[_pidlist_so_len++] = _pre_id;
		}

        if(_sub_obj_change)
        {
			//cout<<"add obj to s2o" << endl;
			//add objid to list 
			_oidlist_s[_oidlist_s_len++] = _obj_id;

			//cout<<"set so2p list"<<endl;
			//cout<<"objid: "<<_obj_id<<endl;
            (this->kvstore)->setpreIDlistBysubIDobjID(_sub_id, _obj_id, _pidlist_so, _pidlist_so_len);
			delete[] _pidlist_so;
            _pidlist_so = NULL;
            _pidlist_so_len = 0;
        }

        if(_sub_change)
        {
			//cout<<"set s2o list"<<endl;
			(this->kvstore)->setobjIDlistBysubID(_sub_id, _oidlist_s, _oidlist_s_len);
            delete[] _oidlist_s;
            _oidlist_s = NULL;
            _oidlist_s_len = 0;
        }
    }//end for( 0 to this->triple_num)
	//int* list = NULL;
	//int len = 0;
	//kvstore->getpreIDlistBysubIDobjID(4, 1000000004, list, len);
	//for(int i = 0; i < len; ++i)
	//{
		  //cout << "predicate "  << list[i] << endl;
		  //cout << kvstore->getPredicateByID(list[i]) << endl;
	//}
	//cout << "the end" << endl;

	Util::logging("OUT so2p...");
	return true;
}

//compare function for qsort 
int
Database::_spo_cmp(const void* _a, const void* _b)
{
    int** _p_a = (int**)_a;
    int** _p_b = (int**)_b;

    {   /* compare subid first */
        int _sub_id_a = (*_p_a)[0];
        int _sub_id_b = (*_p_b)[0];
        if(_sub_id_a != _sub_id_b)
        {
            return _sub_id_a - _sub_id_b;
        }
    }

    {   /* then preid */
        int _pre_id_a = (*_p_a)[1];
        int _pre_id_b = (*_p_b)[1];
        if(_pre_id_a != _pre_id_b)
        {
            return _pre_id_a - _pre_id_b;
        }
    }

    {   /* objid at last */
        int _obj_id_a = (*_p_a)[2];
        int _obj_id_b = (*_p_b)[2];
        if(_obj_id_a != _obj_id_b)
        {
            return _obj_id_a - _obj_id_b;
        }
    }
    return 0;
}

int
Database::_ops_cmp(const void* _a, const void* _b)
{
    int** _p_a = (int**)_a;
    int** _p_b = (int**)_b;
    {   /* compare objid first */
        int _obj_id_a = (*_p_a)[2];
        int _obj_id_b = (*_p_b)[2];
        if(_obj_id_a != _obj_id_b)
        {
            return _obj_id_a - _obj_id_b;
        }
    }

    {   /* then preid */
        int _pre_id_a = (*_p_a)[1];
        int _pre_id_b = (*_p_b)[1];
        if(_pre_id_a != _pre_id_b)
        {
            return _pre_id_a - _pre_id_b;
        }
    }

    {   /* subid at last */
        int _sub_id_a = (*_p_a)[0];
        int _sub_id_b = (*_p_b)[0];
        if(_sub_id_a != _sub_id_b)
        {
            return _sub_id_a - _sub_id_b;
        }
    }

    return 0;
}

int
Database::_pso_cmp(const void* _a, const void* _b)
{
    int** _p_a = (int**)_a;
    int** _p_b = (int**)_b;
    {   /* compare preid first */
        int _pre_id_a = (*_p_a)[1];
        int _pre_id_b = (*_p_b)[1];
        if(_pre_id_a != _pre_id_b)
        {
            return _pre_id_a - _pre_id_b;
        }
    }

    {   /* then subid */
        int _sub_id_a = (*_p_a)[0];
        int _sub_id_b = (*_p_b)[0];
        if(_sub_id_a != _sub_id_b)
        {
            return _sub_id_a - _sub_id_b;
        }
    }

    {   /* objid at last */
        int _obj_id_a = (*_p_a)[2];
        int _obj_id_b = (*_p_b)[2];
        if(_obj_id_a != _obj_id_b)
        {
            return _obj_id_a - _obj_id_b;
        }
    }

    return 0;
}

int
Database::_sop_cmp(const void* _a, const void* _b)
{
    int** _p_a = (int**)_a;
    int** _p_b = (int**)_b;
    {   /* compare subid first */
        int _sub_id_a = (*_p_a)[0];
        int _sub_id_b = (*_p_b)[0];
        if(_sub_id_a != _sub_id_b)
        {
            return _sub_id_a - _sub_id_b;
        }
    }

    {   /* then objid */
        int _obj_id_a = (*_p_a)[2];
        int _obj_id_b = (*_p_b)[2];
        if(_obj_id_a != _obj_id_b)
        {
            return _obj_id_a - _obj_id_b;
        }
    }

    {   /* preid at last */
        int _pre_id_a = (*_p_a)[1];
        int _pre_id_b = (*_p_b)[1];
        if(_pre_id_a != _pre_id_b)
        {
            return _pre_id_a - _pre_id_b;
        }
    }

    return 0;
}

int
Database::insertTriple(const TripleWithObjType& _triple)
{
    //long tv_kv_store_begin = Util::get_cur_time();

    int _sub_id = (this->kvstore)->getIDByEntity(_triple.subject);
    bool _is_new_sub = false;
    /* if sub does not exist */
    if(_sub_id == -1)
    {
        _is_new_sub = true;
        _sub_id = this->entity_num ++;;
        (this->kvstore)->setIDByEntity(_triple.subject, _sub_id);
        (this->kvstore)->setEntityByID(_sub_id, _triple.subject);
    }

    int _pre_id = (this->kvstore)->getIDByPredicate(_triple.predicate);
    bool _is_new_pre = false;
    if(_pre_id == -1)
    {
        _is_new_pre = true;
        _pre_id = this->pre_num ++;
        (this->kvstore)->setIDByPredicate(_triple.predicate, _pre_id);
        (this->kvstore)->setPredicateByID(_pre_id, _triple.predicate);
    }

    /* object is either entity or literal */
    int _obj_id = -1;
    bool _is_new_obj = false;
    bool _is_obj_entity = _triple.isObjEntity();
    if (_is_obj_entity)
    {
        _obj_id = (this->kvstore)->getIDByEntity(_triple.object);

        if (_obj_id == -1)
        {
            _is_new_obj = true;
            _obj_id = this->entity_num ++;
            (this->kvstore)->setIDByEntity(_triple.object, _obj_id);
            (this->kvstore)->setEntityByID(_obj_id, _triple.object);
        }
    }
    else
    {
        _obj_id = (this->kvstore)->getIDByLiteral(_triple.object);

        if (_obj_id == -1)
        {
            _is_new_obj = true;
            _obj_id = Util::LITERAL_FIRST_ID + this->literal_num;
            this->literal_num ++;
            (this->kvstore)->setIDByLiteral(_triple.object, _obj_id);
            (this->kvstore)->setLiteralByID(_obj_id, _triple.object);
        }
    }

    /* if this is not a new triple, return directly */
    bool _triple_exist = false;
    if(!_is_new_sub &&
            !_is_new_pre &&
            !_is_new_obj   )
    {
        _triple_exist = this->exist_triple(_sub_id, _pre_id, _obj_id);
    }

    //debug
//  {
//      stringstream _ss;
//      _ss << this->literal_num << endl;
//      _ss <<"ids: " << _sub_id << " " << _pre_id << " " << _obj_id << " " << _triple_exist << endl;
//      Util::logging(_ss.str());
//  }

    if(_triple_exist)
    {
        return 0;
    }
    else
    {
        this->triples_num ++;
    }

    /* update sp2o op2s s2po o2ps s2o o2s etc.*/
    int updateLen = (this->kvstore)->updateTupleslist_insert(_sub_id, _pre_id, _obj_id);

    //long tv_kv_store_end = Util::get_cur_time();

    EntityBitSet _sub_entity_bitset;
    _sub_entity_bitset.reset();

    this->encodeTriple2SubEntityBitSet(_sub_entity_bitset, &_triple);

    /* if new entity then insert it, else update it. */
    if(_is_new_sub)
    {
        SigEntry _sig(_sub_id, _sub_entity_bitset);
        (this->vstree)->insertEntry(_sig);
    }
    else
    {
        (this->vstree)->updateEntry(_sub_id, _sub_entity_bitset);
    }

    /* if the object is an entity, then update or insert this entity's entry. */
    if (_is_obj_entity)
    {
        EntityBitSet _obj_entity_bitset;
        _obj_entity_bitset.reset();

        this->encodeTriple2ObjEntityBitSet(_obj_entity_bitset, &_triple);

        if (_is_new_obj)
        {
            SigEntry _sig(_obj_id, _obj_entity_bitset);
            (this->vstree)->insertEntry(_sig);
        }
        else
        {
            (this->vstree)->updateEntry(_obj_id, _obj_entity_bitset);
        }
    }

    //long tv_vs_store_end = Util::get_cur_time();

    //debug
//    {
//        cout << "update kv_store, used " << (tv_kv_store_end - tv_kv_store_begin) << "ms." << endl;
//        cout << "update vs_store, used " << (tv_vs_store_end - tv_kv_store_end) << "ms." << endl;
//    }

    return updateLen;
}

// need debug and test...
bool
Database::removeTriple(const TripleWithObjType& _triple)
{
    int _sub_id = (this->kvstore)->getIDByEntity(_triple.subject);
    int _pre_id = (this->kvstore)->getIDByPredicate(_triple.predicate);
    int _obj_id = (this->kvstore)->getIDByEntity(_triple.object);
    if(_obj_id == -1) {
        _obj_id = (this->kvstore)->getIDByLiteral(_triple.object);
    }

    if(_sub_id == -1 || _pre_id == -1 || _obj_id == -1)
    {
        return false;
    }
    bool _exist_triple = this->exist_triple(_sub_id, _pre_id, _obj_id);
    if(! _exist_triple)
    {
        return false;
    }

    /* remove from sp2o op2s s2po o2ps s2o o2s
     * sub2id, pre2id and obj2id will not be updated */
    (this->kvstore)->updateTupleslist_remove(_sub_id, _pre_id, _obj_id);


    int _sub_degree = (this->kvstore)->getEntityDegree(_sub_id);

    /* if subject become an isolated point, remove its corresponding entry */
    if(_sub_degree == 0)
    {
        (this->vstree)->removeEntry(_sub_id);
    }
    /* else re-calculate the signature of subject & replace that in vstree */
    else
    {
        EntityBitSet _entity_bitset;
        _entity_bitset.reset();
        this->calculateEntityBitSet(_sub_id, _entity_bitset);
        (this->vstree)->replaceEntry(_sub_id, _entity_bitset);
    }

    return true;
}

bool
Database::objIDIsEntityID(int _id)
{
    return _id < Util::LITERAL_FIRST_ID;
}

bool
Database::getFinalResult(SPARQLquery& _sparql_q, ResultSet& _result_set)
{
#ifdef DEBUG_PRECISE
	printf("getFinalResult:begins\n");
#endif
	// this is only selected var num
    int _var_num = _sparql_q.getQueryVarNum();
    _result_set.setVar(_sparql_q.getQueryVar());
    vector<BasicQuery*>& query_vec = _sparql_q.getBasicQueryVec();

    //sum the answer number
    int _ans_num = 0;
#ifdef DEBUG_PRECISE
	printf("getFinalResult:before ansnum loop\n");
#endif
    for(unsigned i = 0; i < query_vec.size(); i ++)
    {
        _ans_num += query_vec[i]->getResultList().size();
    }
#ifdef DEBUG_PRECISE
	printf("getFinalResult:after ansnum loop\n");
#endif

    _result_set.ansNum = _ans_num;
#ifndef STREAM_ON
    _result_set.answer = new string*[_ans_num];
    for(int i = 0; i < _result_set.ansNum; i ++)
    {
        _result_set.answer[i] = NULL;
    }
#else
    vector<int> keys;
    vector<bool> desc;
    _result_set.openStream(keys, desc, 0, -1);
#ifdef DEBUG_PRECISE
	printf("getFinalResult:after open stream\n");
#endif
#endif
#ifdef DEBUG_PRECISE
	printf("getFinalResult:before main loop\n");
#endif
    int tmp_ans_count = 0;
    //map int ans into string ans
    //union every basic result into total result
    for(unsigned i = 0; i < query_vec.size(); i++)
    {
        vector<int*>& tmp_vec = query_vec[i]->getResultList();
		//ensure the spo order is right, but the triple order is still reversed
        //for every result group in resultlist
        //for(vector<int*>::reverse_iterator itr = tmp_vec.rbegin(); itr != tmp_vec.rend(); ++itr)
        for(vector<int*>::iterator itr = tmp_vec.begin(); itr != tmp_vec.end(); ++itr)
        {
			//to ensure the order so do reversely in two nested loops
#ifndef STREAM_ON
            _result_set.answer[tmp_ans_count] = new string[_var_num];
#endif
#ifdef DEBUG_PRECISE
	printf("getFinalResult:before map loop\n");
#endif
	//NOTICE: in new join method only selec_var_num columns, 
	//but before in shenxuchuan's join method, not like this.
	//though there is all graph_var_num columns in result_list,
	//we only consider the former selected vars
            //map every ans_id into ans_str
            for(int v = 0; v < _var_num; ++v)
            {
                int ans_id = (*itr)[v];
                string ans_str;
                if(this->objIDIsEntityID(ans_id))
                {
                    ans_str = (this->kvstore)->getEntityByID(ans_id);
                }
                else
                {
                    ans_str = (this->kvstore)->getLiteralByID(ans_id);
                }
#ifndef STREAM_ON
                _result_set.answer[tmp_ans_count][v] = ans_str;
#else
                _result_set.writeToStream(ans_str);
#endif
#ifdef DEBUG_PRECISE
	printf("getFinalResult:after copy/write\n");
#endif
            }
            tmp_ans_count++;
        }
    }
#ifdef STREAM_ON
    _result_set.resetStream();
#endif
#ifdef DEBUG_PRECISE
	printf("getFinalResult:ends\n");
#endif

    return true;
}


void
Database::printIDlist(int _i, int* _list, int _len, string _log)
{
    stringstream _ss;
    _ss << "[" << _i << "] ";
    for(int i = 0; i < _len; i ++) {
        _ss << _list[i] << "\t";
    }
    Util::logging("=="+_log + ":");
    Util::logging(_ss.str());
}

void
Database::printPairList(int _i, int* _list, int _len, string _log)
{
    stringstream _ss;
    _ss << "[" << _i << "] ";
    for(int i = 0; i < _len; i += 2) {
        _ss << "[" << _list[i] << "," << _list[i+1] << "]\t";
    }
    Util::logging("=="+_log + ":");
    Util::logging(_ss.str());
}

void
Database::test()
{
    int subNum = 9, preNum = 20, objNum = 90;

    int* _id_list = NULL;
    int _list_len = 0;
    {   /* x2ylist */
        for (int i = 0; i < subNum; i++)
        {

            (this->kvstore)->getobjIDlistBysubID(i, _id_list, _list_len);
            if (_list_len != 0)
            {
                stringstream _ss;
                this->printIDlist(i, _id_list, _list_len, "s2olist["+_ss.str()+"]");
                delete[] _id_list;
            }

            /* o2slist */
            (this->kvstore)->getsubIDlistByobjID(i, _id_list, _list_len);
            if (_list_len != 0)
            {
                stringstream _ss;
                this->printIDlist(i, _id_list, _list_len, "o(sub)2slist["+_ss.str()+"]");
                delete[] _id_list;
            }
        }

        for (int i = 0; i < objNum; i++)
        {
            int _i = Util::LITERAL_FIRST_ID + i;
            (this->kvstore)->getsubIDlistByobjID(_i, _id_list, _list_len);
            if (_list_len != 0)
            {
                stringstream _ss;
                this->printIDlist(_i, _id_list, _list_len, "o(literal)2slist["+_ss.str()+"]");
                delete[] _id_list;
            }
        }
    }
    {   /* xy2zlist */
        for (int i = 0; i < subNum; i++)
        {
            for (int j = 0; j < preNum; j++)
            {
                (this->kvstore)->getobjIDlistBysubIDpreID(i, j, _id_list,
                        _list_len);
                if (_list_len != 0)
                {
                    stringstream _ss;
                    _ss << "preid:" << j ;
                    this->printIDlist(i, _id_list, _list_len, "sp2olist["+_ss.str()+"]");
                    delete[] _id_list;
                }

                (this->kvstore)->getsubIDlistByobjIDpreID(i, j, _id_list,
                        _list_len);
                if (_list_len != 0)
                {
                    stringstream _ss;
                    _ss << "preid:" << j ;
                    this->printIDlist(i, _id_list, _list_len, "o(sub)p2slist["+_ss.str()+"]");
                    delete[] _id_list;
                }
            }
        }

        for (int i = 0; i < objNum; i++)
        {
            int _i = Util::LITERAL_FIRST_ID + i;
            for (int j = 0; j < preNum; j++)
            {
                (this->kvstore)->getsubIDlistByobjIDpreID(_i, j, _id_list,
                        _list_len);
                if (_list_len != 0)
                {
                    stringstream _ss;
                    _ss << "preid:" << j ;
                    this->printIDlist(_i, _id_list, _list_len,
                                      "*o(literal)p2slist["+_ss.str()+"]");
                    delete[] _id_list;
                }
            }
        }
    }
    {   /* x2yzlist */
        for (int i = 0; i < subNum; i++)
        {
            (this->kvstore)->getpreIDobjIDlistBysubID(i, _id_list, _list_len);
            if (_list_len != 0)
            {
                this->printPairList(i, _id_list, _list_len, "s2polist");
                delete[] _id_list;
                _list_len = 0;
            }
        }

        for (int i = 0; i < subNum; i++)
        {
            (this->kvstore)->getpreIDsubIDlistByobjID(i, _id_list, _list_len);
            if (_list_len != 0)
            {
                this->printPairList(i, _id_list, _list_len, "o(sub)2pslist");
                delete[] _id_list;
            }
        }

        for (int i = 0; i < objNum; i++)
        {
            int _i = Util::LITERAL_FIRST_ID + i;
            (this->kvstore)->getpreIDsubIDlistByobjID(_i, _id_list, _list_len);
            if (_list_len != 0)
            {
                this->printPairList(_i, _id_list, _list_len,
                                    "o(literal)2pslist");
                delete[] _id_list;
            }
        }
    }
}

void
Database::test_build_sig()
{
    BasicQuery* _bq = new BasicQuery("");
    /*
     * <!!!>	y:created	<!!!_(album)>.
     *  <!!!>	y:created	<Louden_Up_Now>.
     *  <!!!_(album)>	y:hasSuccessor	<Louden_Up_Now>
     * <!!!_(album)>	rdf:type	<wordnet_album_106591815>
     *
     * id of <!!!> is 0
     * id of <!!!_(album)> is 2
     *
     *
     * ?x1	y:created	?x2.
     *  ?x1	y:created	<Louden_Up_Now>.
     *  ?x2	y:hasSuccessor	<Louden_Up_Now>.
     * ?x2	rdf:type	<wordnet_album_106591815>
     */
    {
        Triple _triple("?x1", "y:created", "?x2");
        _bq->addTriple(_triple);
    }
    {
        Triple _triple("?x1", "y:created", "<Louden_Up_Now>");
        _bq->addTriple(_triple);
    }
    {
        Triple _triple("?x2", "y:hasSuccessor", "<Louden_Up_Now>");
        _bq->addTriple(_triple);
    }
    {
        Triple _triple("?x2", "rdf:type", "<wordnet_album_106591815>");
        _bq->addTriple(_triple);
    }
    vector<string> _v;
    _v.push_back("?x1");
    _v.push_back("?x2");

    _bq->encodeBasicQuery(this->kvstore, _v);
    Util::logging(_bq->to_str());
    SPARQLquery _q;
    _q.addBasicQuery(_bq);

    (this->vstree)->retrieve(_q);

    Util::logging("\n\n");
    Util::logging("candidate:\n\n"+_q.candidate_str());
}

//void
//Database::test_join()
//{
    //BasicQuery* _bq = new BasicQuery("");
//
     //* <!!!>	y:created	<!!!_(album)>.
     //*  <!!!>	y:created	<Louden_Up_Now>.
     //*  <!!!_(album)>	y:hasSuccessor	<Louden_Up_Now>
     //* <!!!_(album)>	rdf:type	<wordnet_album_106591815>
     //*
     //* id of <!!!> is 0
     //* id of <!!!_(album)> is 2
     //*
     //*
     //* ?x1	y:created	?x2.
     //*  ?x1	y:created	<Louden_Up_Now>.
     //*  ?x2	y:hasSuccessor	<Louden_Up_Now>.
     //* ?x2	rdf:type	<wordnet_album_106591815>
	 //
    //{
        ////Triple _triple("?x1", "y:created", "?x2");
        //_bq->addTriple(_triple);
    //}
    //{
        //Triple _triple("?x1", "y:created", "<Louden_Up_Now>");
        //_bq->addTriple(_triple);
    //}
    //{
        //Triple _triple("?x2", "y:hasSuccessor", "<Louden_Up_Now>");
        //_bq->addTriple(_triple);
    //}
    //{
        //Triple _triple("?x2", "rdf:type", "<wordnet_album_106591815>");
        //_bq->addTriple(_triple);
    //}
    //vector<string> _v;
    //_v.push_back("?x1");
    //_v.push_back("?x2");

    //_bq->encodeBasicQuery(this->kvstore, _v);
    //Util::logging(_bq->to_str());
    //SPARQLquery _q;
    //_q.addBasicQuery(_bq);

    //(this->vstree)->retrieve(_q);

    //Util::logging("\n\n");
    //Util::logging("candidate:\n\n"+_q.candidate_str());
    //_q.print(cout);

    //this->join(_q);
    //ResultSet _rs;
    //this->getFinalResult(_q, _rs);
    //cout << _rs.to_str() << endl;
//}

