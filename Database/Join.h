/*=============================================================================
# Filename: Join.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-12-13 16:05
# Description: design join strategies and select/cost modules
=============================================================================*/

#ifndef _JOIN_JOIN_H
#define _JOIN_JOIN_H 

#include "../Query/IDList.h"
#include "../Query/BasicQuery.h"
#include "../Query/SPARQLquery.h"
#include "../KVstore/KVstore.h"
#include "../Util/Util.h"

typedef vector<unsigned> RecordType;
typedef vector<unsigned>::iterator RecordIterator;
typedef list<RecordType> TableType;
typedef list<RecordType>::iterator TableIterator;
typedef list<RecordType>::reverse_iterator TableReverseIterator;
//typedef list< vector<int> > TableType;
//typedef list< vector<int> >::iterator TableIterator;
//typedef list< vector<int> >::reverse_iterator TableReverseIterator;
//typedef vector< vector<int*> > IdLists;
//typedef vector< vector<int> > IdListsLen;

typedef struct Satellite
{
	int id;
	unsigned* idlist;
	unsigned idlist_len;
	Satellite(int _id, unsigned* _idlist, unsigned _idlist_len)
	{
		this->id = _id;
		this->idlist = _idlist;
		this->idlist_len = _idlist_len;
	}
}Satellite;

//Database new Join and pass something like kvstore
class Join
{
private:
	int start_id;
	int var_num;
	BasicQuery* basic_query;
	KVstore* kvstore;
	TYPE_TRIPLE_NUM* pre2num;
	TYPE_PREDICATE_ID limitID_predicate;
	TYPE_ENTITY_LITERAL_ID limitID_literal;
	TYPE_ENTITY_LITERAL_ID limitID_entity;
	//used by score_node for parameters
	static const unsigned PARAM_DEGREE = 1;
	static const unsigned PARAM_SIZE = 1000000;
	static const unsigned PARAM_PRE = 10000;
	static const unsigned PARAM_DENSE = 1;

	static const unsigned JUDGE_LIMIT = 2;
	//NOTICE+DEBUG: please use constexpr below instead of the phase above(constexpr is supported in C++11)
	//http://www.cnblogs.com/wanyuanchun/p/4041080.html
	//constexpr static const double JUDGE_LIMIT = 0.5;

	static const unsigned LIMIT_CANDIDATE_LIST_SIZE = 1000;
	//BETTER?:predefine size to avoid copy cost
	TableType current_table;
	TableIterator new_start;   //keep to end() as default
	//list<bool> table_row_new;
	
	//keep the mapping for disordered ids in vector<int> table
	int* id2pos; 
	int id_pos;   //the num of id put into id2pos currently
	int* pos2id; 
	bool* dealed_triple;
	stack<int> mystack;

	vector<unsigned*>* result_list;
	vector<Satellite> satellites;
	unsigned* record;
	unsigned record_len;

	void init(BasicQuery* _basic_query, bool* d_triple);
	void clear();
	void add_id_pos_mapping(int _id);
	void reset_id_pos_mapping();

	//judge which method should be used according to 
	//the size of candidates and structure of quering graph
	int judge(unsigned _smallest, unsigned _biggest);

	//select the start point and search order
	void select();

	//score the cost to link two tables and the efficience
	//of filtering
	//int score(List1, List2);
	
	//score the node according to degree and size
	double score_node(int var);

	void toStartJoin();

	bool filter_before_join();
	bool constant_edge_filter(int _var_i);
	void preid_filter(int _var_i);
	//new
	bool only_pre_filter_after_join();
	void add_literal_candidate();
	bool pre_var_handler();
	//bool filterBySatellites(int _var, int _ele);
	bool preFilter(int _var);
	bool allFilterByPres();
	void cartesian(int pos, int end);

	//functions for help
	//copy/add to the end of current_table and set true
	void add_new_to_results(TableIterator it, unsigned id);

	//void set_results_old(list<bool>::iterator it);
	int choose_next_node(int id);

	bool is_literal_var(int id);
	//bool is_literal_ele(int _id);
	
	void copyToResult();

	//BETTER?:change these params to members in class
	//void acquire_all_id_lists(IdLists& _id_lists, IdListsLen& _id_lists_len, IDList& _can_list, vector<int>& _edges, int _id, unsigned _can_list_size);
	void update_answer_list(IDList*& valid_ans_list, IDList& _can_list, unsigned* id_list, unsigned id_list_len, bool _is_literal);
	bool join_two(vector< vector<int> >& _edges, IDList& _can_list, unsigned _can_list_size, int _id, bool _is_ready);

	bool multi_join();
	//NOTICE:this is only used to join a BasicQuery
	bool join();

public:
	Join();
	Join(KVstore* _kvstore, TYPE_TRIPLE_NUM* _pre2num, TYPE_PREDICATE_ID _limitID_predicate, TYPE_ENTITY_LITERAL_ID _limitID_literal,
		TYPE_ENTITY_LITERAL_ID _limitID_entity);
	//these functions can be called by Database
	bool join_sparql(SPARQLquery& _sparql_query);
	bool join_basic(BasicQuery* _basic_query, bool* d_triple);
	~Join();
};

#endif //_JOIN_JOIN_H

