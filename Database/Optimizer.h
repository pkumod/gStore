/*=============================================================================
# Filename: Optimizer.h
# Author: Lei Yang, Yuqi Zhou
# Mail: yangop8@pku.edu.cn. zhouyuqi@pku.edu.cn
=============================================================================*/

#ifndef _DATABASE_OPTIMIZER_H
#define _DATABASE_OPTIMIZER_H

#include "../Util/Util.h"
#include "../Query/SPARQLquery.h"
#include "../Query/BasicQuery.h"
#include "../Query/IDList.h"
#include "../KVstore/KVstore.h"
#include "../VSTree/VSTree.h"
#include "TableOperator.h"
#include "ResultTrigger.h"
#include "../Query/QueryPlan.h"
#include "../Query/QueryTree.h"
#include "Join.h"
#include "./Statistics.h"
#include "./PlanTree.h"
#include "../Util/OrderedVector.h"
#include <unordered_map>
#include <map>
#include <cstring>
#include <climits>
#include <algorithm>
#include <list>
#include <cstdlib>
#include <cmath>


using namespace std;
using TableContent = list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>;
using TableContentShardPtr = shared_ptr<list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>;
using PositionValue = std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>;
using PositionValueSharedPtr = std::shared_ptr<PositionValue>;
using IDCachesSharePtr = shared_ptr<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>>;
using QueryPlanSharedPtr = shared_ptr<QueryPlan>;

enum class BasicQueryStrategy{
  Normal,
  Special// method 1-5
};

struct QueryInfo{
  bool limit_;
  int limit_num_;
  bool is_distinct_;
  shared_ptr<vector<QueryTree::Order>> ordered_by_vars_;
};
/*
 *
 * 还差一个 copyToResult 的操作
 *
1. Optimizer类在generalevaluation里面初始化，并且在generalevaluation调用do_query()
----------
 2. do_query()首先判断handler0或者handler1-5
3. 在handler0里面首先调用enum_query_plan()，按照BFS或者DFS生成所有执行计划
4. 生成完执行计划后调用choose_exec_plan()选择一个计划执行
5. 在choose_exec_plan()中每一个执行计划需要层次调用cost_model()->cardinality_estimator()->update_cardinality_cache()
 ----------
6. 按照点的顺序首先需要对起始点调用gen_filter获得该点的候选集 [done]
    而后的每一个点调用join_one_step()
7. 在每一步的join_one_step()中需要根据queryplan调用三种基本join之一，同时需要update中间结果，并且每一步的开始需要check一下中间结果能不能利用
8. 每一步都做完之后，copytoResult()
*/

class Optimizer
{
 public:

  Optimizer(KVstore* kv_store, VSTree* vs_tree, Statistics *statistics, TYPE_TRIPLE_NUM* pre2num, TYPE_TRIPLE_NUM* pre2sub,
             TYPE_TRIPLE_NUM* pre2obj, TYPE_PREDICATE_ID limitID_predicate, TYPE_ENTITY_LITERAL_ID limitID_literal,
             TYPE_ENTITY_LITERAL_ID limitID_entity, shared_ptr<Transaction> txn
             // ,SPARQLquery& sparql_query,shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> order_by_list,TYPE_ENTITY_LITERAL_ID limit_num
             );
  ~Optimizer()=default;
  tuple<bool,shared_ptr<IntermediateResult>> DoQuery(SPARQLquery&,QueryInfo); // the whole process
  tuple<bool,shared_ptr<IntermediateResult>> MergeBasicQuery(SPARQLquery &sparql_query);


  tuple<bool,TableContentShardPtr> GenerateColdCandidateList(const shared_ptr<vector<EdgeInfo>>&,const shared_ptr<vector<EdgeConstantInfo>>&);
  tuple<bool,TableContentShardPtr> JoinANode(const shared_ptr<OneStepJoinNode>&,const TableContentShardPtr&,const PositionValueSharedPtr&,const IDCachesSharePtr&);
  tuple<bool,PositionValueSharedPtr,TableContentShardPtr> JoinTwoTable(const shared_ptr<OneStepJoinTable>& one_step_join_table,
  																	 const TableContentShardPtr& table_a,
  																	 const PositionValueSharedPtr& table_a_id_pos,
  																	 const PositionValueSharedPtr& table_a_pos_id,
  																	 const TableContentShardPtr& table_b,
  																	 const PositionValueSharedPtr& table_b_id_pos,
  																	 const PositionValueSharedPtr& table_b_pos_id);
  tuple<bool,TableContentShardPtr> ANodeEdgesConstraintFilter(const shared_ptr<OneStepJoinNode>&, TableContentShardPtr,const PositionValueSharedPtr&,const IDCachesSharePtr&);
  tuple<bool,TableContentShardPtr> OneEdgeConstraintFilter(EdgeInfo, EdgeConstantInfo, const TableContentShardPtr&,const PositionValueSharedPtr&,const IDCachesSharePtr&);
  tuple<bool,TableContentShardPtr> FilterAVariableOnIDList(const shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>&,TYPE_ENTITY_LITERAL_ID ,const TableContentShardPtr&,const PositionValueSharedPtr&);
  shared_ptr<IntermediateResult> NormalJoin(shared_ptr<BasicQuery>,shared_ptr<QueryPlan>);
  bool CacheConstantCandidates(const shared_ptr<OneStepJoinNode>& one_step, const IDCachesSharePtr& id_caches);
  bool AddConstantCandidates(EdgeInfo edge_info,EdgeConstantInfo edge_table_info,TYPE_ENTITY_LITERAL_ID targetID, const IDCachesSharePtr& id_caches);
  tuple<bool, TableContentShardPtr> getAllSubObjID();

//  You can change this,
//  but to make sure SAMPLE_CACHE_MAX <= SAMPLE_NUM_UPBOUND (in Statistics.h)
  const int SAMPLE_CACHE_MAX = 50;
  const double SAMPLE_PRO = 0.05;

  int get_strategy(unsigned id_cache_size);
  bool check_exist_this_triple(TYPE_ENTITY_LITERAL_ID s_id, TYPE_PREDICATE_ID p_id, TYPE_ENTITY_LITERAL_ID o_id);
  bool check_pass(vector<int> &linked_nei_pos, vector<char> &edge_type,
				  vector<TYPE_PREDICATE_ID> &p_list, vector<unsigned> &last_sample, unsigned this_var_sample);
    unsigned long card_estimator(BasicQuery* basicquery, IDCachesSharePtr &id_caches,
							map<int, unsigned> &var_to_num_map, map<int, TYPE_ENTITY_LITERAL_ID> &var_to_type_map,
							map<int, vector<TYPE_ENTITY_LITERAL_ID >> &var_to_sample_map,map<int, map<int, unsigned >> &s_o_list_average_size,
							const vector<int> &last_plan_nodes, int next_join_node, const vector<int> &now_plan_nodes,
							vector<map<vector<int>, unsigned long>> &card_cache,
							vector<map<vector<int>, vector<vector<unsigned>> >> &sample_cache,
							bool use_sample = true);

    unsigned long get_card(vector<map<vector<int>, unsigned long>> &card_cache, const vector<int> &nodes);

    unsigned long cost_model_for_wco(BasicQuery* basicquery, IDCachesSharePtr &id_caches, PlanTree* last_plan,
								const vector<int> &last_plan_node, int next_node, const vector<int> &now_plan_node,
								map<int, unsigned> &var_to_num_map, map<int, TYPE_ENTITY_LITERAL_ID> &var_to_type_map,
								map<int, vector<unsigned>> &var_to_sample_cache, map<int, map<int, unsigned >> &s_o_list_average_size,
								vector<map<vector<int>, unsigned long>> &card_cache, vector<map<vector<int>, vector<vector<unsigned>> >> &sample_cache);

    unsigned long cost_model_for_binary(vector<map<vector<int>, unsigned long>> &card_cache,
                                   const vector<int> &plan_a_nodes, const vector<int> &plan_b_nodes,
                                   PlanTree* plan_a, PlanTree* plan_b);

    void get_nei_by_subplan_nodes(BasicQuery* basicquery, const vector<int> &need_join_nodes,
								  const vector<int> &last_plan_node, set<int> &nei_node);

    void get_join_nodes(BasicQuery* basicquery, const vector<int> &plan_a_nodes,
                        vector<int> &other_nodes, set<int> &join_nodes);

	void considerallscan(BasicQuery* basicquery, IDCachesSharePtr &id_caches,
					     vector<map<vector<int>, list<PlanTree*>>> &plan_cache,
					     map<int, unsigned > &var_to_num_map,
					     map<int, TYPE_ENTITY_LITERAL_ID> &var_to_type_map,
					     map<int, vector<unsigned>> &var_to_sample_cache, vector<int> &need_join_nodes, bool use_sample = true);

    void considerwcojoin(BasicQuery* basicquery, IDCachesSharePtr &id_caches, int node_num, const vector<int> &need_join_nodes,
						 vector<map<vector<int>, list<PlanTree*>>> &plan_cache,
						 map<int, unsigned> &var_to_num_map, map<int, TYPE_ENTITY_LITERAL_ID> &var_to_type_map,
						 map<int, vector<unsigned>> &var_to_sample_cache, map<int, map<int, unsigned >> &s_o_list_average_size,
						 vector<map<vector<int>, unsigned long>> &card_cache, vector<map<vector<int>, vector<vector<unsigned>> >> &sample_cache);

    void considerbinaryjoin(BasicQuery* basicquery, int node_num,
                            vector<map<vector<int>, unsigned long>> &card_cache,
                            vector<map<vector<int>, list<PlanTree*>>> &plan_cache);

    int enum_query_plan(BasicQuery* basicquery, KVstore *kvstore, IDCachesSharePtr& id_caches,
						vector<int> &need_join_nodes,
						vector<map<vector<int>, list<PlanTree*>>> &plan_cache);
    PlanTree* get_best_plan(const vector<int> &nodes, vector<map<vector<int>, list<PlanTree*>>> &plan_cache);
	PlanTree* get_best_plan_by_num(BasicQuery* basicquery, int total_var_num, vector<map<vector<int>, list<PlanTree*>>> &plan_cache);
    PlanTree* get_plan(BasicQuery* basicquery, KVstore *kvstore, IDCachesSharePtr& id_caches);


    std::shared_ptr<IDList> ExtendRecord(const shared_ptr<OneStepJoinNode> &one_step_join_node_,
                    const PositionValueSharedPtr &id_pos_mapping,
                    const IDCachesSharePtr &id_caches,
                    TYPE_ENTITY_LITERAL_ID new_id,
                    list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>::iterator &record_iterator) const;

  shared_ptr<IDList> CandidatesWithConstantEdge(const shared_ptr<vector<EdgeInfo>> &edge_info_vector) const;


    /*以下代码暂且不写
  std::shared_ptr<IDList> ExtendRecord(const shared_ptr<OneStepJoinNode> &one_step_join_node_,
                    const PositionValueSharedPtr &id_pos_mapping,
                    const IDCachesSharePtr &id_caches,
                    TYPE_ENTITY_LITERAL_ID new_id,
                    list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>::iterator &record_iterator) const;

  shared_ptr<IDList> CandidatesWithConstantEdge(const shared_ptr<vector<EdgeInfo>> &edge_info_vector) const;

  /*以下代码暂且不写
  bool update_cardinality_cache(shared_ptr<BasicQuery>,vector<map<shared_ptr<BasicQuery>,unsigned*> >);
  bool enum_query_plan(shared_ptr<BasicQuery>, KVstore* kvstore, bool _is_topk_query);// Join::multi_join() BFS
  bool choose_exec_plan(vector<map<shared_ptr<BasicQuery>, QueryPlan*> > _candidate_plans,
                        vector<QueryPlan> _execution_plan);     //TODO: DP
  IDList gen_filter(unsigned _id, KVstore* kvstore, shared_ptr<TYPE_TRIPLE_NUM*> pre2num,
                    shared_ptr<TYPE_TRIPLE_NUM*> pre2sub, shared_ptr<TYPE_TRIPLE_NUM*> pre2obj, shared_ptr<bool*> dealed_triple); // Strategy::pre_handler()
  unsigned cardinality_estimator(shared_ptr<BasicQuery>, KVstore* kvstore, vector<map<shared_ptr<BasicQuery>,unsigned*> > _cardinality_cache);
  unsigned cost_model(shared_ptr<BasicQuery>, shared_ptr<QueryPlan>); // TODO: other parameters used in cost model
  // change exec_plan if |real cardinality - estimated cardinality| > eps
  bool choose_exec_plan(vector<map<shared_ptr<BasicQuery>, QueryPlan*> > _candidate_plans,
                        map<shared_ptr<BasicQuery>,vector<unsigned*> > _current_result,  vector<QueryPlan>);
  //TODO: re-choose plan in every iteration
  // 先实现手动喂计划的方案
  tuple<bool,shared_ptr<IntermediateResult>> execution(BasicQuery*, shared_ptr<QueryPlan>);*/


  BasicQueryStrategy ChooseStrategy(BasicQuery*);


  tuple<bool,TableContentShardPtr> ExecutionDepthFirst(BasicQuery* basic_query, const shared_ptr<QueryPlan>& query_plan,
                                                                 const QueryInfo& query_info,const PositionValueSharedPtr& id_pos_mapping);

  tuple<bool,TableContentShardPtr> DepthSearchOneLayer(const shared_ptr<QueryPlan>& query_plan,
                                                                 int layer_count,
                                                                 int &result_number_till_now,
                                                                 int limit_number,
                                                                 const TableContentShardPtr& tmp_result, const PositionValueSharedPtr& id_pos_mapping,
                                                                 const IDCachesSharePtr& id_caches);
  static void UpdateIDList(const shared_ptr<IDList>& valid_id_list, unsigned* id_list, unsigned id_list_len,bool id_list_prepared);

  /*copy the result to vector<unsigned*> & */
  bool CopyToResult(vector<unsigned*> *target, BasicQuery *basic_query, const shared_ptr<IntermediateResult>& result);
  void Cartesian(int, int,int,unsigned*,const shared_ptr<vector<Satellite>>&,vector<unsigned*>*,BasicQuery *);

  //tuple<bool,TableContentShardPtr> BreathSearch(BasicQuery* basic_query, const shared_ptr<QueryPlan>& query_plan, 
  //                                                              const PositionValueSharedPtr& id_pos_mapping, shared_ptr<vector<double>>& weight_list);
  tuple<bool,PositionValueSharedPtr, TableContentShardPtr> ExecutionBreathFirst(BasicQuery* basic_query,const QueryInfo& query_info,Tree_node* plan_tree,IDCachesSharePtr id_caches);


 private:
  KVstore* kv_store_;
  Statistics* statistics;

  shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> order_by_list_; // empty if not using 'orderby'
  TYPE_ENTITY_LITERAL_ID limit_num_; // -1 if not limit result size

  bool is_edge_case; //Strategy 1-6 or Strategy 0
  int current_basic_query_; // updated by result_list.size()

  shared_ptr<vector<BasicQuery*>> basic_query_list_; //fork from SPARQLQuery, I dont know why
  shared_ptr<vector<tuple<BasicQuery*, shared_ptr<vector<QueryPlan>>>>> candidate_plans_;
  shared_ptr<vector<QueryPlan>> execution_plan_;
  shared_ptr<vector<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>> result_list_; // vector<unsigned*>* result_list;

  //TODO: shared_ptr<BasicQuery> may brings wrong matching
  shared_ptr<vector<map<BasicQuery*,vector<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>>> join_cache_; // map(sub-structure, result_list)
  shared_ptr<vector<map<BasicQuery*,shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>> cardinality_cache_; // map(sub-structure, cardinality), not in statistics

  FILE* fp_;
  TYPE_TRIPLE_NUM* pre2num_;
  TYPE_TRIPLE_NUM* pre2sub_;
  TYPE_TRIPLE_NUM* pre2obj_;
  TYPE_PREDICATE_ID limitID_predicate_;
  TYPE_ENTITY_LITERAL_ID limitID_literal_;
  TYPE_ENTITY_LITERAL_ID limitID_entity_;
  shared_ptr<Transaction> txn_;

  // 因为BasicQuery不给谓词变量编码，所以搞了一个抽象的类
  shared_ptr<vector<VarDescriptor>> var_descriptors_;

  //静态情况下可以存储weight
  shared_ptr<vector<double>> weight_list_;
};

template <typename Container> // we can make this generic for any container [1]
struct container_hash {
  std::size_t operator()(Container const& c) const {
    return boost::hash_range(c.begin(), c.end());
  }
};

#endif