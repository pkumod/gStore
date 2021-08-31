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
	BasicQuery *basicquery;
	BGPQuery *bgpquery;
	Statistics *statistics;
	IDCachesSharePtr & id_caches;

	vector<map<vector<unsigned>, list<PlanTree*>>> plan_cache;

	vector<map<vector<unsigned>, unsigned long>> card_cache;
	vector<map<vector<unsigned>, vector<vector<unsigned>> >> sample_cache;

	// map var id to some thing
	map<unsigned, unsigned > var_to_num_map;
	map<unsigned, TYPE_ENTITY_LITERAL_ID > var_to_type_map;
	map<unsigned, vector<unsigned>> var_to_sample_cache;
	map<unsigned, map<unsigned, unsigned >> s_o_list_average_size;

	vector<unsigned> join_nodes;

	map<unsigned, vector<TYPE_ENTITY_LITERAL_ID>> so_var_to_sample_cache;
	map<unsigned, vector<TYPE_PREDICATE_ID>> pre_var_to_sample_cache;

	PlanGenerator(KVstore *kvstore_, BasicQuery *basicquery_, Statistics *statistics_, IDCachesSharePtr& id_caches_);
	PlanGenerator(KVstore *kvstore_, BGPQuery *bgpquery_, Statistics *statistics_, IDCachesSharePtr& id_caches_);


	//  You can change this,
	//  but to make sure SAMPLE_CACHE_MAX <= SAMPLE_NUM_UPBOUND (in Statistics.h)
	static const unsigned SAMPLE_CACHE_MAX = 50;
	const double SAMPLE_PRO = 0.05;

	JoinMethod get_join_strategy(bool s_is_var, bool p_is_var, bool o_is_var, unsigned var_num);

	static unsigned get_sample_size(unsigned id_cache_size);
	bool check_exist_this_triple(TYPE_ENTITY_LITERAL_ID s_id, TYPE_PREDICATE_ID p_id, TYPE_ENTITY_LITERAL_ID o_id);
	bool check_pass(vector<int> &linked_nei_pos, vector<char> &edge_type,
					vector<TYPE_PREDICATE_ID> &p_list, vector<unsigned> &last_sample, unsigned this_var_sample);

	unsigned long card_estimator(const vector<unsigned> &last_plan_nodes, unsigned next_join_node, const vector<unsigned> &now_plan_nodes,
								 bool use_sample = true);

	unsigned long get_card(const vector<unsigned> &nodes);

	unsigned long cost_model_for_wco(PlanTree* last_plan,
									 const vector<unsigned> &last_plan_node, unsigned next_node, const vector<unsigned> &now_plan_node);

	unsigned long cost_model_for_binary(const vector<unsigned> &plan_a_nodes, const vector<unsigned> &plan_b_nodes,
										PlanTree* plan_a, PlanTree* plan_b);

	void get_nei_by_subplan_nodes(const vector<unsigned> &need_join_nodes,
								  const vector<unsigned> &last_plan_node, set<unsigned> &nei_node);

	void get_join_nodes(const vector<unsigned> &plan_a_nodes,
						vector<unsigned> &other_nodes, set<unsigned> &join_nodes);

	void considerallscan(vector<unsigned> &need_join_nodes, bool use_sample = true);

	void considerwcojoin(unsigned node_num, const vector<unsigned> &need_join_nodes);

	void considerbinaryjoin(unsigned node_num);

	int enum_query_plan(vector<unsigned> &need_join_nodes);
	PlanTree* get_best_plan(const vector<unsigned> &nodes);
	PlanTree* get_best_plan_by_num(int total_var_num);


	PlanTree* get_normal_plan();


	JoinMethod get_join_method(bool s_const, bool p_const, bool o_const, VarDescriptor::ItemType item_type);
	void get_candidate_generate_plan();
	void considerallvarscan();
	void considerallwcojoin(unsigned var_num);
	void considerallbinaryjoin(unsigned var_num);

	PlanTree* get_plan();


	unsigned choose_next_nei_id(set<unsigned> nei_id_set);

	void update_nei_id_set(set<unsigned> &nei_id_vec, set<unsigned> &already_id, unsigned next_id);

	PlanTree* get_random_plan();
	PlanTree* get_special_no_pre_var_plan();
	PlanTree* get_special_one_triple_plan();


	PlanTree* get_plan_for_debug();

	static double estimate_one_edge_selectivity(TYPE_PREDICATE_ID  pre_id,
										 		bool pre_constant, KVstore *kvstore,
										 		shared_ptr<IDList> &s_cache,
										 		shared_ptr<IDList> &o_cache);

	static void get_idcache_sample(shared_ptr<IDList> &so_cache, vector<unsigned> &so_sample_cache);
};


#endif //GSTORE_PLANGENERATOR_H
