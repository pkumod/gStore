/*=============================================================================
# Filename: BasicQuery.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-31 19:18
# Description: originally written by liyouhuan, modified by zengli
=============================================================================*/

#ifndef _QUERY_BASICQUERY_H
#define _QUERY_BASICQUERY_H

#include "../Util/Util.h"
#include "../Util/Triple.h"
#include "../Signature/Signature.h"
#include "../KVstore/KVstore.h"
#include "IDList.h"

//NOTICE: the query graph must be linked
//var_id == -1: constant(string), entity or literal
//All constants should be dealed before joining tables!
//A var in query can be subject or object, and both. Once in subject,
//it cannot contain literal, while in object it may contain entity,
//literal, or both
//The vars not in join process are also encoded, so not -1
//
//a subject cannot be literal, but an object can be entity or literal
//not supported: ?v1 and this is a predicate
//pre_id == -1: the query graph is not valid and the result should be empty
//
//DEFINE:
//literal variable - no edge out(only occur in objects)
//(after we retrive all candidates from vstree, only entities are considered, the vars
//which only present in objects are possible to contain literals, so we must mark this!)
//free literal variable - a literal variable and all its neighbor id != -1
//(i.e. no constant neighbor which will restrict this variable, otherwise, we can acquire 
//this var's can_list by querying in kvstore according to the constant and pre)

//It is ok for var in select to be free var, but this can not be used as start point.
//(we assume candidates of the start point is all ok and then search deeply)
//However, we can always find a start point because not all vars are all in objects!
//(otherwise, no edge in query graph)
//
//What is more, some graphs will be regarded as not-connected, such as:
//A-c0-B, c0 is a constant, we should do a A x B here!
//two-part-matching, ABC and c1c2, each node connects with this two constants.
//(edge maybe different)

//+---------------------------------------------------------------------------+
//|elements in BasicQuery(all are strings)                                    |
//||                                                                          |
//|+---constants(all need to be dealed before join)                           |
//||   |                                                                      |
//||   +---literals(quoted in "")                                             |
//||   |                               graph_var_num, the num of vars to join |
//||   +---entities(included in <>, prefix is allowed)                      | |
//||                                                                        | |
//|+---variables(all begin with '?')                                        | |
//|    |                                                                    | |
//|    +---selected vars(the former select_var_num ones) <<<<<<<<<<<<<<<<<<<+ |
//|    |                                                                    | |
//|    +---not selected vars                                                | |
//|        |                                                                | |
//|        +---degree > 1     <<<<<<<<<<<as bridge<<<<<<<<<<<<<<<<<<<<<<<<<<+ |
//|        |   |                                                              |
//|        |   +---exist in subjects(cannot be literal)                       |
//|        |   |                                                              |
//|        |   +---just in objects(all edges in, may include literals)        |
//|        |       |                                                          |
//|        |       +---free(all neighbors are vars)                           |
//|        |       |                                                          |
//|        |       +---not-free(exist constant neighbors)                     |
//|        |                                                                  |
//|        +---degree == 1(dealed after join)                                 |
//|            |                                                              |
//|            +---subject                                                    |
//|            |                                                              |
//|            +---object                                                     |
//+---------------------------------------------------------------------------+

class PreVar
{
public:
	string name;
	vector<int> triples;
	bool selected;
	PreVar()
	{
		this->selected = false;
	}
	PreVar(string _name)
	{
		this->name = _name;
		this->selected = false;
	}
	PreVar(string _name, bool _flag)
	{
		this->name = _name;
		this->selected = _flag;
	}
};

class BasicQuery
{
private:
	vector<Triple> triple_vt;
	// mapping from variables' name to their assigned id
	map<std::string, int> var_str2id;
	// record each tuple's(subject, predicate, object) number of occurrences in this BasicQuery
	map<std::string, int> tuple2freq;
	set<std::string> selected_var_set;

	// id < select_var_num means in select
	int select_var_num;
	// var_num is different from that in SPARQLquery
	// because there are some variable not in select
	int graph_var_num;
	int total_var_num;
	int retrieve_var_num;
	string* var_name;
	IDList* candidate_list;
	vector<unsigned*> result_list;
	int* var_degree;

	//whether has added the variable's literal candidate
	//bool* is_literal_candidate_added;
	bool* ready;
	//if need to be retrieved by vstree or generate when join(first is graph var)
	bool* need_retrieve;
	bool* link_with_constant;


	//save the result of encodeBasicQuery
	bool encode_result;

	// edge_id[var_id][i] : the line id of the i-th edge of the var
	int**    edge_id;
	
	// edge_id[var_id][i] : the neighbor id of the i-th edge of the var 
	int**    edge_nei_id;

	// edge_pre_id[var_id][i] : the preID of the i-th edge of the var
	int**    edge_pre_id;

	// denote the type of edge, assigned with
	// BasicQuery::IN or BasicQuery::OUT
	// edge_type[var_id][i]
	char**   edge_type;

	EntityBitSet* var_sig;

	// BETTER:edge sig is of little importance 
	// edge_sig[sub_id][obj_id]
	//EdgeBitSet**  edge_sig;

	void addInVarNotInSelect();
	void buildTuple2Freq();
	void initial();

	//void updateSubSig(int _sub_id, int _pre_id, int _obj_id, std::string _obj, int _line_id);
	//void updateObjSig(int _obj_id, int _pre_id, int _sub_id, std::string _sub, int _line_id);
	void updateSubSig(int _sub_var_id, TYPE_PREDICATE_ID _pre_id, int _line_id, int _obj_var_id);
	void updateObjSig(int _obj_var_id, TYPE_PREDICATE_ID _pre_id, int _line_id, int _sub_var_id);

	//infos for predicate variables
	vector<PreVar> pre_var;
	int selected_pre_var_num;

	//to manage the selected vars order(considering selected pre var)
	//NOTICE:the order is always same as query_var in encodeBasicQuery, but not always same as select-clause
	//positive key id for normal var, -1-id for pre var
	map<int, int> selected_var_position;

	// refer to
	shared_ptr<list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>> intermediate_result_values_;
public:
	// static const int MAX_VAR_NUM = 20;
	static const int MAX_VAR_NUM = 50;
	static const int MAX_PRE_VAR_NUM = 20;
	static const char NOT_JUST_SELECT = 'a';
	static const char SELECT_VAR = 's';

	// _query is a SPARQL query string
	BasicQuery(const string _query="");
	~BasicQuery();
	void clear();

	//get the number of variables which are in join
	int getVarNum();

	//get selected number of variadbles
	int getSelectVarNum();

	//get the total number of variables
	int getTotalVarNum();

	//get the retrieved number of variables
	int getRetrievedVarNum();

	// get the name of _var in the query graph
	std::string getVarName(int _var);

	int getIDByVarName(const string& _name);

	// get triples number, also sentences number
	unsigned getTripleNum();

	//check if a normal var is in select
	bool isVarSelected(const std::string& _name) const;

	std::string to_str();

	// get the ID of the i-th triple
	const Triple& getTriple(int _i_th_triple);

	// get the ID of the i-th edge of _var
	int getEdgeID(int _var, int _i_th_edge);

	// get the ID of var, where the i-th edge of _var points to 
	int getEdgeNeighborID(int _var, int _i_th_edge);
	
	// get the preID of the i-th edge of _var
	TYPE_PREDICATE_ID getEdgePreID(int _var, int _i_th_edge);

	// get the type of the i-th edge of _var
	char getEdgeType(int _var, int _i_th_edge);

	//get the degree of _var in the query graph
	int getVarDegree(int _var);

	//get the index of edge between two var ids
	vector<int> getEdgeIndex(int _id0, int _id);

	const EntityBitSet& getVarBitSet(int _i)const;

	// get the candidate list of _var in the query graph
	IDList& getCandidateList(int _var);

	unsigned getCandidateSize(int _var);

	// get the result list of _var in the query graph
	vector<unsigned*>& getResultList();
	vector<unsigned*>* getResultListPointer();

	// get the entity signature of _var in the query graph
	const EntityBitSet& getEntitySignature(int _var);

	// check whether the i-th edge of _var is IN edge
	bool isInEdge(int _var, int _i_th_edge)const;

	// check whether the i-th edge of _var is OUT edge
	bool isOutEdge(int _var, int _i_th_edge)const;

	bool isOneDegreeVar(string& _no_sense_var);
	bool isOneDegreeNotJoinVar(std::string& _not_select_var);

	// check whether _var may include some literal results
	bool isLiteralVariable(int _var);
	// check whether _var is literal variable and do not have any entity neighbors 
	bool isFreeLiteralVariable(int _var);

	// check whether has added _var's literal candidates
	//bool isAddedLiteralCandidate(int _var);

	// set _var's literal candidates has been added 
	//void setAddedLiteralCandidate(int _var);

	//check if need to be retrieved
	bool if_need_retrieve(int _var);
	bool isSatelliteInJoin(int _var);

	//if the candidates of var(in join) is all ready
	bool isReady(int _var) const;
	void setReady(int _var);

	// encode relative signature data of the query graph 
	bool encodeBasicQuery(KVstore* _p_kvstore, const std::vector<std::string>& _query_var);
	bool getEncodeBasicQueryResult() const;

	unsigned getPreVarNum() const;
	const PreVar& getPreVarByID(unsigned) const;
	int getPreVarID(const string& _name) const;
	unsigned getSelectedPreVarNum() const;
	bool isPreVarSelected(unsigned _pid) const;

	int getSelectedVarPosition(int _var);
	int getSelectedPreVarPosition(int _var);
	int getSelectedVarPosition(std::string _var);
	int getSelectedPreVarPosition(std::string _var);

	void addTriple(const Triple& _triple);
	void print(ostream& _out_stream);

	int getVarID_MaxCandidateList();
	int getVarID_FirstProcessWhenJoin();

	std::string candidate_str();
	std::string result_str();
	std::string triple_str();
};

#endif //_QUERY_BASICQUERY_H

