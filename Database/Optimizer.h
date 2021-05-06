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
#include <unordered_map>
#include <map>
#include <cstring>
#include <climits>
#include <algorithm>


using namespace std;
using TableContent = list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>;
using TableContentShardPtr = shared_ptr<list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>;
using PositionValueSharedPtr = std::shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>>;
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
  tuple<bool,TableContentShardPtr> JoinTwoTable(const shared_ptr<OneStepJoinTable>&,const TableContentShardPtr&,const PositionValueSharedPtr&,const TableContentShardPtr&,const PositionValueSharedPtr&);
  tuple<bool,TableContentShardPtr> ANodeEdgesConstraintFilter(const shared_ptr<OneStepJoinNode>&, TableContentShardPtr,const PositionValueSharedPtr&,const IDCachesSharePtr&);
  tuple<bool,TableContentShardPtr> OneEdgeConstraintFilter(EdgeInfo, EdgeConstantInfo, const TableContentShardPtr&,const PositionValueSharedPtr&,const IDCachesSharePtr&);
  tuple<bool,TableContentShardPtr> FilterAVariableOnIDList(const shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>&,TYPE_ENTITY_LITERAL_ID ,const TableContentShardPtr&,const PositionValueSharedPtr&);
  shared_ptr<IntermediateResult> NormalJoin(shared_ptr<BasicQuery>,shared_ptr<QueryPlan>);
  bool CacheConstantCandidates(const shared_ptr<OneStepJoinNode>& one_step, const IDCachesSharePtr& id_caches);
  bool AddConstantCandidates(EdgeInfo edge_info,EdgeConstantInfo edge_table_info,TYPE_ENTITY_LITERAL_ID targetID, const IDCachesSharePtr& id_caches);



//  You can change this,
//  but to make sure SAMPLE_CACHE_MAX <= SAMPLE_NUM_UPBOUND (in Statistics.h)
  static const int SAMPLE_CACHE_MAX = 500;

  bool check_exist_this_triple(TYPE_ENTITY_LITERAL_ID s_id, TYPE_PREDICATE_ID p_id, TYPE_ENTITY_LITERAL_ID o_id);
  bool check_past(shared_ptr<BasicQuery> basicquery, vector<int> &join_need_estimator,
                  unsigned *last_sample, unsigned this_var_sample, int this_join_var_num);
  unsigned get_small_query_card_estimation(shared_ptr<BasicQuery> basicquery, map<int, unsigned > var_to_num_map,
                                           map<int, TYPE_ENTITY_LITERAL_ID> var_to_type_map,
                                           map<int, vector<TYPE_ENTITY_LITERAL_ID >> &var_to_sample_cache,
                                           vector<int> &join_need_estimator, int this_join_var_num,
                                           vector<map<vector<int>, unsigned >> &_cardinality_cache,
                                           vector<map<vector<int>, vector<unsigned *> >> &_sample_cache);

  unsigned cardinality_estimator(shared_ptr<BasicQuery> basicquery, vector<vector<int>> join_need_estimator,
                                 map<int, unsigned> var_to_num_map, map<int, vector<TYPE_ENTITY_LITERAL_ID >> var_to_sample_cache,
                                 map<int, TYPE_ENTITY_LITERAL_ID> var_to_type_map,
                                 vector<map<vector<int>, unsigned >> &_cardinality_cache,
                                 vector<map<vector<int>, vector<unsigned*> >> &_sample_cache);
  unsigned card_estimator(shared_ptr<BasicQuery> basicquery, vector<int> last_plan, int this_join_node,
                            vector<map<vector<int>, unsigned>> &_card_cache);
  void get_nei_by_subplan(shared_ptr<BasicQuery> basicquery, vector<int> &last_plan_node,
                            set<int> &nei_node);
  void get_last_plan_node(vector<vector<int>> last_plan, vector<int> &last_plan_node);

  void considerallscan(shared_ptr<BasicQuery> basicquery,
                       vector<map<vector<int>, unsigned >> &card_cache_ref,
                       vector<map<vector<vector<int>>, unsigned >> &cost_cache_ref,
                       map<int, unsigned > &var_to_num_map_ref,
                       map<int, TYPE_ENTITY_LITERAL_ID> &var_to_type_map_ref);

  void considerwcojoin(shared_ptr<BasicQuery> basicquery,
                         int node_num, vector<map<vector<int>, unsigned >> &card_cache,
                         vector<map<vector<vector<int>>, unsigned>> &cost_cache);
  void cost_model_for_wco(shared_ptr<BasicQuery> basicquery, vector<vector<int>> &new_plan, int next_node,
                            vector<int> last_plan_node, const vector<vector<int>> &last_plan,
                            vector<map<vector<int>, unsigned>> &card_cache, vector<map<vector<vector<int>>, unsigned>> &cost_cache);
  int check_two_plan_could_binary_join(vector<int> &last_plan_node,
                                         const vector<vector<int>> &small_plan, const vector<vector<int>> &another_small_plan,
                                         vector<map<vector<int>, unsigned>> &card_cache);
  vector<vector<int>> get_binary_plan(const vector<vector<int>> &small_plan, const vector<vector<int>> &another_small_plan);

  void considerbinaryjoin(shared_ptr<BasicQuery> basicquery, int node_num,
                          vector<map<vector<int>, unsigned>> &card_cache,
                          vector<map<vector<vector<int>>, unsigned>> &cost_cache);
  unsigned cost_model_for_binary(int small_plan_node_num, const vector<vector<int>> small_plan,
                                   int another_small_plan_node_num, const vector<vector<int>> another_small_plan,
                                   vector<map<vector<vector<int>>, unsigned >> &cost_cache);

  void enum_query_plan(shared_ptr<BasicQuery> basicquery, KVstore *kvstore,
                         vector<map<vector<vector<int>>, unsigned>> &cost_cache);
  vector<vector<int>> get_best_plan(int var_num, vector<map<vector<vector<int>>, unsigned>> &cost_cache);
  vector<vector<int>> get_plan(shared_ptr<BasicQuery> basicquery, KVstore *kvstore, IDCachesSharePtr& id_caches);


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
  tuple<bool,QueryPlanSharedPtr> ExecutionBreathFirst(BasicQuery* basic_query, const PositionValueSharedPtr& id_pos_mapping);


 private:
  KVstore* kv_store_;
  Statistics* statistics;

  shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> order_by_list_; // empty if not using 'orderby'
  TYPE_ENTITY_LITERAL_ID limit_num_; // -1 if not limit result size

  bool is_edge_case; //Strategy 1-6 or Strategy 0
  int current_basic_query_; // updated by result_list.size()

  shared_ptr<vector<shared_ptr<BasicQuery>>> basic_query_list_; //fork from SPARQLQuery, I dont know why
  shared_ptr<vector<tuple<shared_ptr<BasicQuery>, shared_ptr<vector<QueryPlan>>>>> candidate_plans_;
  shared_ptr<vector<QueryPlan>> execution_plan_;
  shared_ptr<vector<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>> result_list_; // vector<unsigned*>* result_list;

  //TODO: shared_ptr<BasicQuery> may brings wrong matching
  shared_ptr<vector<map<shared_ptr<BasicQuery>,vector<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>>> join_cache_; // map(sub-structure, result_list)
  shared_ptr<vector<map<shared_ptr<BasicQuery>,shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>> cardinality_cache_; // map(sub-structure, cardinality), not in statistics

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