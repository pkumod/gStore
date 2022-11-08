/*=============================================================================
# Filename: PlanGenerator.h
# Author: Linglin Yang
# Mail: linglinyang@stu.pku.edu.cn
=============================================================================*/

#ifndef GSTORE_PLANGENERATOR_H
#define GSTORE_PLANGENERATOR_H

#include "../Query/IDList.h"
#include "./PlanTree.h"
#include "../Util/OrderedVector.h"

#include <map>
#include <cstring>
#include <climits>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <random>

enum class BGPQueryStrategy{
	Heuristic,
	DP,
};

class PlanGenerator {

private:
	KVstore *kvstore;
	BGPQuery *bgpquery;
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
	map<unsigned, bool> var_sampled_from_candidate;

	map<unsigned, map<unsigned, double >> s_o_list_average_size;

	// store var id, not var index, only contains s_o var
	vector<unsigned> join_nodes;

	vector<unsigned> satellite_nodes;
	// set<unsigned> already_done_satellite;

	// use these two vector to represent a PlanTree containing VarScan, WCOJoin and BinaryJoin.
	vector<unsigned> plan_var_vec;
	vector<unsigned> plan_var_degree;

  //  You can change this, initialized in PlanGenerator.cpp
  //  but to make sure SAMPLE_CACHE_MAX <= SAMPLE_NUM_UPBOUND (in Statistics.h)
  static const unsigned SAMPLE_CACHE_MAX;
  static const double SAMPLE_PRO;
  static const unsigned SMALL_QUERY_VAR_NUM;
  static const double CANDIDATE_RATIO_MAX;
  static const unsigned PARAM_SIZE;
  static const unsigned PARAM_PRE;
  static const unsigned HEURISTIC_CANDIDATE_MAX;

public:
	PlanGenerator(KVstore *kvstore_, BGPQuery *bgpquery_, IDCachesSharePtr& id_caches_, TYPE_TRIPLE_NUM triples_num_,
				  	TYPE_PREDICATE_ID limitID_predicate_, TYPE_ENTITY_LITERAL_ID limitID_literal_, TYPE_ENTITY_LITERAL_ID limitID_entity_,
				  TYPE_TRIPLE_NUM* pre2num_, TYPE_TRIPLE_NUM* pre2sub_, TYPE_TRIPLE_NUM* pre2obj_, shared_ptr<Transaction> txn_);

	~PlanGenerator();

	JoinMethod GetJoinStrategy(bool s_is_var, bool p_is_var, bool o_is_var, unsigned var_num) const;

	unsigned GetCandidateSizeFromWholeDB(unsigned var_id) const;
	unsigned GetCandidateSizeById(unsigned var_id);
	static unsigned GetSampleSize(unsigned id_cache_size);

	long long CardEstimatorTwoNodes(unsigned last_node, unsigned next_join_node, const vector<unsigned> &now_plan_nodes);
	long long CardEstimatorMoreThanTwoNodes(const vector<unsigned> &last_plan_nodes, unsigned next_join_node, const vector<unsigned> &now_plan_nodes);
	// using sample method
	long long CardEstimator(const vector<unsigned> &last_plan_nodes, unsigned next_join_node, const vector<unsigned> &now_plan_nodes);

	long long GetCard(const vector<unsigned> &nodes);

	void InsertEdgeSelectivityToCache(unsigned from_id, unsigned to_id, unsigned linked_num);

	void InsertCardEstimationToCache(const vector<unsigned> &now_plan_nodes,
										 long long card_estimation, vector<vector<unsigned>> &result_sample);

	long long CostModelForp2soOptimization(unsigned node_1_id, unsigned node_2_id);
	long long CostModelForWCOJoin(PlanTree* last_plan, const vector<unsigned> &last_plan_node,
												 unsigned next_node, const vector<unsigned> &now_plan_node);

	long long CostModelForBinaryJoin(const vector<unsigned> &plan_a_nodes, const vector<unsigned> &plan_b_nodes,
										PlanTree* plan_a, PlanTree* plan_b);

	void InsertThisPlanToCache(PlanTree *new_plan, const vector<unsigned> &new_node_vec, unsigned var_num);
	PlanTree* GetBestPlanByNodes(const vector<unsigned> &nodes);
	PlanTree* GetBestPlanByNum(int total_var_num);

	void GetJoinNodes(const vector<unsigned> &plan_a_nodes, vector<unsigned> &other_nodes, set<unsigned> &join_nodes_set) const;

	unsigned GetSampleFromWholeDatabase(unsigned var_id, vector<unsigned> &so_sample_cache) const;
	vector<shared_ptr<AffectOneNode>> CompleteCandidate();
	void GetNeighborBySubPlanNodes(const vector<unsigned> &last_plan_node, set<unsigned> &nei_node);
	void ConsiderWCOJoin(unsigned var_num);
	void ConsiderBinaryJoin(unsigned var_num);
	void AddSatelliteNode(PlanTree* best_plan);


	BGPQueryStrategy PlanStrategy(bool use_binary_join = true);

	double NodeScore(unsigned var_id);
	void InsertVarScanToCache(unsigned var_id, PlanTree* var_scan_plan);
	void InsertVarNumToCache(unsigned var_id);
	void InsertVarNumAndSampleToCache(unsigned var_id);
	void ConsiderVarScan(BGPQueryStrategy strategy);
	unsigned HeuristicFirstNode();
	unsigned HeuristicNextNode(unsigned last_node_id);

	PlanTree* HeuristicPlan(bool use_binary_join = true);
	PlanTree* DPPlan(bool use_binary_join = true);
	PlanTree* GetPlan(bool use_binary_join);


	PlanTree* GetSpecialOneTriplePlan();


	static double EstimateOneEdgeSelectivity(TYPE_PREDICATE_ID  pre_id, bool pre_constant, KVstore *kvstore,
											 shared_ptr<IDList> &s_cache, shared_ptr<IDList> &o_cache);

	static void GetIdCacheSample(shared_ptr<IDList> &so_cache, vector<unsigned> &so_sample_cache);


	void print_plan_generator_info() const;
	void print_sample_info();
};


#endif //GSTORE_PLANGENERATOR_H
