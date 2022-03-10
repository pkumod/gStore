/*=============================================================================
# Filename: PlanGenerator.h
# Author: Linglin Yang
# Mail: linglinyang@stu.pku.edu.cn
=============================================================================*/

#ifndef GSTORE_PLANGENERATOR_H
#define GSTORE_PLANGENERATOR_H

#include "../Util/Util.h"
#include "../Query/BasicQuery.h"
#include "../Query/BGPQuery.h"
#include "../Query/IDList.h"
#include "../KVstore/KVstore.h"
#include "./Statistics.h"
#include "./PlanTree.h"
#include "TableOperator.h"
#include "../Util/OrderedVector.h"

#include <unordered_map>
#include <map>
#include <cstring>
#include <climits>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <random>

class PlanGenerator {

public:
	KVstore *kvstore;
	BGPQuery *bgpquery;
	Statistics *statistics;
	IDCachesSharePtr & id_caches;
	TYPE_TRIPLE_NUM triples_num;
	TYPE_PREDICATE_ID limitID_predicate;
	TYPE_ENTITY_LITERAL_ID limitID_literal;
	TYPE_ENTITY_LITERAL_ID limitID_entity;
	TYPE_TRIPLE_NUM* pre2num;
	TYPE_TRIPLE_NUM* pre2sub;
	TYPE_TRIPLE_NUM* pre2obj;
	shared_ptr<Transaction> txn;

	// only contain plans joining not-satellite nodes
	// plan_cache[0] contains only one node scan plan
	// plan_cache[n] contains n-1 nodes join plan, and this n-1 nodes are connected
	vector<map<vector<unsigned>, list<PlanTree*>>> plan_cache;

	vector<map<vector<unsigned>, long long>> card_cache;
	vector<map<vector<unsigned>, vector<vector<unsigned>> >> sample_cache;

	// map var id to some thing
	map<unsigned, unsigned > var_to_num_map;
	map<unsigned, vector<unsigned>> var_to_sample_cache;
	map<unsigned, map<unsigned, double >> s_o_list_average_size;

	map<unsigned, bool> var_sampled_from_candidate;

	// store var id, not var index, only contains s_o var
	vector<unsigned> join_nodes;

	vector<unsigned> satellite_nodes;
	set<unsigned> already_done_satellite;

	PlanGenerator(KVstore *kvstore_, BGPQuery *bgpquery_, Statistics *statistics_, IDCachesSharePtr& id_caches_, TYPE_TRIPLE_NUM triples_num_,
				  	TYPE_PREDICATE_ID limitID_predicate_, TYPE_ENTITY_LITERAL_ID limitID_literal_, TYPE_ENTITY_LITERAL_ID limitID_entity_,
				  TYPE_TRIPLE_NUM* pre2num_, TYPE_TRIPLE_NUM* pre2sub_, TYPE_TRIPLE_NUM* pre2obj_, shared_ptr<Transaction> txn_);

	~PlanGenerator();


	//  You can change this, initialized in PlanGenerator.cpp
	//  but to make sure SAMPLE_CACHE_MAX <= SAMPLE_NUM_UPBOUND (in Statistics.h)
	static const unsigned SAMPLE_CACHE_MAX;
	const double SAMPLE_PRO = 0.05;

	JoinMethod get_join_strategy(bool s_is_var, bool p_is_var, bool o_is_var, unsigned var_num);

	static unsigned get_sample_size(unsigned id_cache_size);
	bool check_exist_this_triple(TYPE_ENTITY_LITERAL_ID s_id, TYPE_PREDICATE_ID p_id, TYPE_ENTITY_LITERAL_ID o_id);

	long long card_estimator_two_nodes(unsigned last_node, unsigned next_join_node, const vector<unsigned> &now_plan_nodes);
	long long card_estimator_more_than_three_nodes(const vector<unsigned> &last_plan_nodes, unsigned next_join_node, const vector<unsigned> &now_plan_nodes);
	// using sample method
	long long card_estimator(const vector<unsigned> &last_plan_nodes, unsigned next_join_node, const vector<unsigned> &now_plan_nodes);

	long long get_card(const vector<unsigned> &nodes);

	void insert_edge_selectivity_to_cache(unsigned from_id, unsigned to_id, unsigned linked_num);

	void insert_card_estimation_to_cache(const vector<unsigned> &now_plan_nodes,
										 long long card_estimation, vector<vector<unsigned>> &result_sample);

	long long cost_model_for_p2so_optimization(unsigned node_1_id, unsigned node_2_id);
	long long cost_model_for_wco(PlanTree* last_plan, const vector<unsigned> &last_plan_node,
												 unsigned next_node, const vector<unsigned> &now_plan_node);

	long long cost_model_for_binary(const vector<unsigned> &plan_a_nodes, const vector<unsigned> &plan_b_nodes,
										PlanTree* plan_a, PlanTree* plan_b);

	void insert_this_plan_to_cache(PlanTree *new_plan, const vector<unsigned> &new_node_vec, unsigned var_num);
	PlanTree* get_best_plan(const vector<unsigned> &nodes);
	PlanTree* get_best_plan_by_num(int total_var_num);


	JoinMethod get_join_method(bool s_const, bool p_const, bool o_const, VarDescriptor::ItemType item_type);
	void get_candidate_generate_plan();
	void get_join_nodes(const vector<unsigned> &plan_a_nodes,
									vector<unsigned> &other_nodes, set<unsigned> &join_nodes);

	unsigned get_sample_from_whole_database(unsigned var_id, vector<unsigned> &so_sample_cache);
	vector<shared_ptr<FeedOneNode>> completecandidate();
	void considervarscan();
	void get_nei_by_sub_plan_nodes(const vector<unsigned> &last_plan_node, set<unsigned> &nei_node);
	void considerwcojoin(unsigned var_num);
	void considerbinaryjoin(unsigned var_num);
	void addsatellitenode(PlanTree* best_plan);

	PlanTree* get_plan(bool use_binary_join = true);


	unsigned choose_first_var_id_random();
	unsigned choose_next_nei_id(set<unsigned> nei_id_set);

	void update_nei_id_set(set<unsigned> &nei_id_vec, set<unsigned> &already_id, unsigned next_id);

	PlanTree* get_random_plan();
	PlanTree* get_special_one_triple_plan();


	static double estimate_one_edge_selectivity(TYPE_PREDICATE_ID  pre_id,
										 		bool pre_constant, KVstore *kvstore,
										 		shared_ptr<IDList> &s_cache,
										 		shared_ptr<IDList> &o_cache);

	static void get_idcache_sample(shared_ptr<IDList> &so_cache, vector<unsigned> &so_sample_cache);


	void print_plan_generator_info();
	void print_sample_info();
};


#endif //GSTORE_PLANGENERATOR_H
