/*=============================================================================
# Filename: PlanGenerator.h
# Author: Linglin Yang
# Mail: linglinyang@stu.pku.edu.cn
=============================================================================*/

#ifndef GSTORE_PLANGENERATOR_H
#define GSTORE_PLANGENERATOR_H

#include "../Util/Util.h"
#include "../Query/BasicQuery.h"
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
	Statistics *statistics;
	IDCachesSharePtr & id_caches;

	vector<map<vector<int>, list<PlanTree*>>> plan_cache;

	vector<map<vector<int>, unsigned long>> card_cache;
	vector<map<vector<int>, vector<vector<unsigned>> >> sample_cache;

	map<int, unsigned > var_to_num_map;
	map<int, TYPE_ENTITY_LITERAL_ID > var_to_type_map;
	map<int, vector<unsigned>> var_to_sample_cache;
	map<int, map<int, unsigned >> s_o_list_average_size;

	PlanGenerator(KVstore *kvstore_, BasicQuery *basicquery_, Statistics *statistics_, IDCachesSharePtr& id_caches_);
	//  You can change this,
	//  but to make sure SAMPLE_CACHE_MAX <= SAMPLE_NUM_UPBOUND (in Statistics.h)
	const unsigned SAMPLE_CACHE_MAX = 50;
	const double SAMPLE_PRO = 0.05;

	unsigned get_sample_size(unsigned id_cache_size);
	bool check_exist_this_triple(TYPE_ENTITY_LITERAL_ID s_id, TYPE_PREDICATE_ID p_id, TYPE_ENTITY_LITERAL_ID o_id);
	bool check_pass(vector<int> &linked_nei_pos, vector<char> &edge_type,
					vector<TYPE_PREDICATE_ID> &p_list, vector<unsigned> &last_sample, unsigned this_var_sample);

	unsigned long card_estimator(const vector<int> &last_plan_nodes, int next_join_node, const vector<int> &now_plan_nodes,
								 bool use_sample = true);

	unsigned long get_card(const vector<int> &nodes);

	unsigned long cost_model_for_wco(PlanTree* last_plan,
									 const vector<int> &last_plan_node, int next_node, const vector<int> &now_plan_node);

	unsigned long cost_model_for_binary(const vector<int> &plan_a_nodes, const vector<int> &plan_b_nodes,
										PlanTree* plan_a, PlanTree* plan_b);

	void get_nei_by_subplan_nodes(const vector<int> &need_join_nodes,
								  const vector<int> &last_plan_node, set<int> &nei_node);

	void get_join_nodes(const vector<int> &plan_a_nodes,
						vector<int> &other_nodes, set<int> &join_nodes);

	void considerallscan(vector<int> &need_join_nodes, bool use_sample = true);

	void considerwcojoin(unsigned node_num, const vector<int> &need_join_nodes);

	void considerbinaryjoin(unsigned node_num);

	int enum_query_plan(vector<int> &need_join_nodes);
	PlanTree* get_best_plan(const vector<int> &nodes);
	PlanTree* get_best_plan_by_num(int total_var_num);
	PlanTree* get_normal_plan();
	PlanTree* get_special_no_pre_var_plan();
	PlanTree* get_special_one_triple_plan();
};


#endif //GSTORE_PLANGENERATOR_H
