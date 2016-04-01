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

//BETTER?:use vector<int> with predefined size in inner
//needed function is same, and cache hits are high
typedef vector<int> RecordType;
typedef vector<int>::iterator RecordIterator;
typedef list<RecordType> TableType;
typedef list<RecordType>::iterator TableIterator;
typedef list<RecordType>::reverse_iterator TableReverseIterator;
//typedef list< vector<int> > TableType;
//typedef list< vector<int> >::iterator TableIterator;
//typedef list< vector<int> >::reverse_iterator TableReverseIterator;
typedef vector< vector<int*> > IdLists;
typedef vector< vector<int> > IdListsLen;

//Database new Join and pass something like kvstore
class Join
{
private:
	int start_id;
	int var_num;
	BasicQuery* basic_query;
	KVstore* kvstore;
	//used by score_node for parameters
	static const unsigned PARAM_DEGREE = 1;
	static const unsigned PARAM_SIZE = 10000;
	static const unsigned PARAM_DENSE = 1;
	static const double JUDGE_LIMIT = 0.5;
	static const int LIMIT_CANDIDATE_LIST_SIZE = 1000;
	//BETTER:inner vector?predefine size to avoid copy cost
	TableType current_table;
	TableIterator new_start;   //keep to end() as default
	//list<bool> table_row_new;
	
	//keep the mapping for disordered ids in vector<int> table
	int* id2pos; 
	int id_pos;   //the num of id put into id2pos currently
	int* pos2id; 
	bool* dealed_triple;
	stack<int> mystack;

	void init(BasicQuery* _basic_query);
	void clear();
	void add_id_pos_mapping(int _id);

	//judge which method should be used according to 
	//the size of candidates and structure of quering graph
	int judge(int _smallest, int _biggest);

	//select the start point and search order
	void select();

	//score the cost to link two tables and the efficience
	//of filtering
	//int score(List1, List2);
	
	//score the node according to degree and size
	double score_node(unsigned _degree, unsigned _size);

	void filter_before_join();
	void literal_edge_filter(int _var_i);
	void preid_filter(int _var_i);
	void only_pre_filter_after_join();
	void add_literal_candidate();

	//functions for help
	//copy/add to the end of current_table and set true
	void add_new_to_results(TableIterator it, int id);

	//void set_results_old(list<bool>::iterator it);
	int choose_next_node(bool* _dealed_triple, int id);
	void acquire_all_id_lists(IdLists& _id_lists, IdListsLen& _id_lists_len, IDList& _can_list, vector<int>& _edges, bool* _dealed_triple, int _id, int _can_list_size);
	bool is_literal_var(int id);
	bool is_literal_ele(int _id);
	bool if_prepare_idlist(int _can_list_size, bool _is_literal);
	bool new_join_with_multi_vars_prepared(IdLists& _id_lists, IdListsLen& _id_lists_len, vector<int>& _edges, IDList& _can_list, int _can_list_size);
	bool new_join_with_multi_vars_not_prepared(vector<int>& _edges, IDList& _can_list, int _can_list_size, int _id, bool _is_literal);

	void multi_join();

	void index_join();

	//NOTICE:this is only used to join a BasicQuery
	bool join();

public:
	Join();
	Join(KVstore* _kvstore);
	//this function can be called by Database
	bool join_sparql(SPARQLquery& _sparql_query);
	~Join();
};

#endif //_JOIN_JOIN_H

