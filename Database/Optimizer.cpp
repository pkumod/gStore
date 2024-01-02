/*=============================================================================
# Filename: Optimizer.cpp
# Author: Yuqi Zhou, Linglin Yang
# Mail: zhouyuqi@pku.edu.cn, fyulingi@gmail.com
=============================================================================*/

#include "Optimizer.h"

// #define FEED_PLAN

Optimizer::Optimizer(KVstore *kv_store,
                     TYPE_TRIPLE_NUM *pre2num,
                     __attribute__((unused)) TYPE_TRIPLE_NUM *pre2sub,
                     TYPE_TRIPLE_NUM *pre2obj,
                     TYPE_TRIPLE_NUM triples_num,
                     TYPE_PREDICATE_ID limitID_predicate,
                     TYPE_ENTITY_LITERAL_ID limitID_literal,
                     TYPE_ENTITY_LITERAL_ID limitID_entity,
                     shared_ptr<Transaction> txn
):
    kv_store_(kv_store), pre2num_(pre2num), pre2sub_(pre2sub),pre2obj_(pre2obj),triples_num_(triples_num),
    limitID_predicate_(limitID_predicate), limitID_literal_(limitID_literal),limitID_entity_(limitID_entity),
    txn_(txn), executor_(kv_store,txn,limitID_predicate,limitID_literal,limitID_entity_){}

BasicQueryStrategy Optimizer::ChooseStrategy(const std::shared_ptr<BGPQuery>& bgp_query,QueryInfo *query_info){
	if (!query_info->limit_)
		return BasicQueryStrategy::Normal;
	if(query_info->ordered_by_expressions_->size() == 1) {
		// TODO: more accurate decision of algebraic expressions that can be processed by topk
		if ((*query_info->ordered_by_expressions_)[0].comp_tree_root.children.empty())
			return BasicQueryStrategy::Normal;
		auto search_plan = make_shared<TopKSearchPlan>(bgp_query, this->kv_store_,
													   (*query_info->ordered_by_expressions_)[0], nullptr);
		if (search_plan->SuggestTopK())
			return BasicQueryStrategy::TopK;
		else
			return BasicQueryStrategy::Normal;
	} else {
		if (query_info->ordered_by_expressions_->empty())
			return BasicQueryStrategy::limitK;
		else
			return BasicQueryStrategy::Normal;
	}
}

/**
 * Generate a list of Intermediate templates, so each layer doesn't have to
 * calculate it each time
 */
std::shared_ptr<std::vector<IntermediateResult>> Optimizer::GenerateResultTemplate(const shared_ptr<DFSPlan>& query_plan)
{
  auto table_template = make_shared<std::vector<IntermediateResult>>();
  table_template->emplace_back();
  auto &tables = *table_template;
  auto &dfs_operation = *query_plan->join_order_;

  if(dfs_operation[0]->op_type_==StepOperation::StepOpType::Extend){
    if( dfs_operation[0]->GetRange() == StepOperation::OpRangeType::OneNode)
      tables[0].AddNewNode(dfs_operation[0]->GetOneNodePlan()->node_to_join_);
    else if( dfs_operation[0]->GetRange() == StepOperation::OpRangeType::TwoNode){
      tables[0].AddNewNode(dfs_operation[0]->GetTwoNodePlan()->node_to_join_1_);
      tables[0].AddNewNode(dfs_operation[0]->GetTwoNodePlan()->node_to_join_2_);
    }
    else if( dfs_operation[0]->GetRange() == StepOperation::OpRangeType::GetAllTriples){
      auto &the_only_edge = (*dfs_operation[0]->GetOneNodePlan()->edges_)[0];
      tables[0].AddNewNode(the_only_edge.s_);
      tables[0].AddNewNode(the_only_edge.p_);
      tables[0].AddNewNode(the_only_edge.o_);
    }
  }

  auto it = dfs_operation.begin();
  it++;
  for (; it!=dfs_operation.end(); ++it)
  {
    tables.emplace_back( IntermediateResult::OnlyPositionCopy(tables.back()));
    if((*it)->op_type_ != StepOperation::StepOpType::Extend)
      continue;
    if( (*it)->GetRange() == StepOperation::OpRangeType::OneNode)
      tables.back().AddNewNode( (*it)->GetOneNodePlan()->node_to_join_);
    else if((*it)->GetRange() == StepOperation::OpRangeType::TwoNode) {
      tables.back().AddNewNode( (*it)->GetTwoNodePlan()->node_to_join_1_);
      tables.back().AddNewNode( (*it)->GetTwoNodePlan()->node_to_join_2_);
    }
    else if((*it)->GetRange() == StepOperation::OpRangeType::GetAllTriples){
      auto &the_only_edge = (*((*it)->GetOneNodePlan()->edges_))[0];
      tables.back().AddNewNode(the_only_edge.s_);
      tables.back().AddNewNode(the_only_edge.p_);
      tables.back().AddNewNode(the_only_edge.o_);
    }
  }
  return table_template;
}

tuple<bool, IntermediateResult> Optimizer:: ExecutionDepthFirst(shared_ptr<BGPQuery>& bgp_query,
                                                               shared_ptr<DFSPlan>& query_plan,
                                                               const QueryInfo& query_info,
                                                               const IDCachesSharePtr& id_caches) {
  auto limit_num = query_info.limit_num_;
  PrintDebugInfoLine(g_format("Optimizer::ExecutionDepthFirst query_info.limit_num_= %d", query_info.limit_num_));
  auto &first_operation = (*query_plan->join_order_)[0];
  tuple<bool, IntermediateResult> step_result;
  IntermediateResult first_table;
  if(first_operation->op_type_ ==StepOperation::StepOpType::Extend)
  {
    auto first_result = InitialTable(bgp_query,id_caches,first_operation, query_plan->join_order_->size()==1 ? query_info.limit_num_ : gstore::Executor::NO_LIMIT_OUTPUT);
    first_table = get<1>(first_result);
    if( query_plan->join_order_->size()==1 || first_table.values_->empty())
      return make_tuple(true, first_table);
  }
  else
    throw "The First Operation in DepthFirst Must Be StepOperation::StepOpType::Extend";

  auto first_candidates_list = first_table.values_;
  int now_result = 0;
  vector<TableContentShardPtr> result_container;
  auto table_template = GenerateResultTemplate(query_plan);

  while (now_result <= limit_num) {
    auto tmp_result = make_shared<TableContent>();
    tmp_result->push_back(first_candidates_list->front());
    first_candidates_list->pop_front();
    auto first_var_one_point_result =
        this->DepthSearchOneLayer(query_plan, 1, now_result, limit_num, tmp_result,id_caches,table_template);

    if (get<0>(first_var_one_point_result))
    {
      auto one_point_inter_result = get<1>(first_var_one_point_result).values_;
      if (!one_point_inter_result->empty())
        result_container.push_back(one_point_inter_result);
      if (now_result >= limit_num)
        break;
    }
    if (first_candidates_list->empty())
      break;
  }

  IntermediateResult final_result;
  final_result.pos_id_map = table_template->back().pos_id_map;
  final_result.id_pos_map = table_template->back().id_pos_map;

  PrintDebugInfoLine(g_format("Optimizer::ExecutionDepthFirst result_container.size()= %d", result_container.size()));
  if(result_container.empty())
    return make_tuple(true,final_result);
  int counter = 0;
  /* merge result */
  for(auto& result_item:result_container)
    for(auto &record: *result_item)
    {
      final_result.values_->push_back(std::move(record));
      if( ++counter == limit_num)
        break;
    }
  return make_tuple(true,final_result);
}

/**
 * 可能在传结果这里有问题
 * @param query_plan
 * @param layer_count
 * @param result_number_till_now
 * @param limit_number
 * @param table_content_ptr
 * @param id_pos_mapping
 * @param id_caches
 * @return
 */
tuple<bool,IntermediateResult> Optimizer::DepthSearchOneLayer(shared_ptr<DFSPlan>& query_plan,
                                                              int layer_count,
                                                              int &result_number_till_now,
                                                              int limit_number,
                                                              TableContentShardPtr table_content_ptr,
                                                              const IDCachesSharePtr& id_caches,
                                                              const shared_ptr<vector<IntermediateResult>>& table_template) {
  IntermediateResult old_table;
  old_table.values_ = std::move(table_content_ptr);
  old_table.pos_id_map = (*table_template)[layer_count-1].pos_id_map;
  old_table.id_pos_map = (*table_template)[layer_count-1].id_pos_map;

  IntermediateResult layer_result;
  layer_result.pos_id_map = (*table_template)[layer_count].pos_id_map;
  layer_result.id_pos_map = (*table_template)[layer_count].id_pos_map;

  auto one_step = (*(query_plan->join_order_))[layer_count];
  TableContentShardPtr step_table;

  bool the_last_op = (unsigned )layer_count + 1 == query_plan->join_order_->size();
  size_t max_output_num = the_last_op? limit_number - result_number_till_now : gstore::Executor::NO_LIMIT_OUTPUT;

  if(one_step->op_type_ == StepOperation::StepOpType::Extend) {
    switch (one_step->GetRange()) {
      case StepOperation::OpRangeType::OneNode: {
            auto step_result = executor_.AffectANode(old_table, id_caches, true, one_step->distinct_, false,
                                                     max_output_num, one_step->GetOneNodePlan());
            step_table = get<1>(step_result).values_;
            break;
        }

        case StepOperation::OpRangeType::TwoNode: {
            auto step_result = executor_.JoinTwoNode(one_step->GetTwoNodePlan(), old_table, id_caches, false,
                                                     max_output_num);
            step_table = get<1>(step_result).values_;
            break;
        }
        default:
          throw "Err StepOperation::OpRangeType in Depth First Search in StepOpType::Extend";
    }
  }
  else if (one_step->op_type_ == StepOperation::StepOpType::Check){
    auto step_result = executor_.ANodeEdgesConstraintFilter(one_step->GetOneNodePlan(), old_table,false);
    step_table = get<1>(step_result).values_;
  }
  else if (one_step->op_type_ == StepOperation::StepOpType::Satellite){
    auto step_result = executor_.AffectANode(old_table, id_caches, false, one_step->distinct_, false,
                                             max_output_num, one_step->GetOneNodePlan());
    step_table = get<1>(step_result).values_;
  }

  if(step_table->empty())
    return make_tuple(false, layer_result);

  auto& all_result = layer_result.values_;

  /* deep in bottom, update the 'result_number_till_now' */
  if( the_last_op ) {
    result_number_till_now += step_table->size();
    for(auto it = step_table->begin();it!=step_table->end();it++)
      all_result->push_back(std::move(*it));
    return make_tuple(true, layer_result);
  }

  /*  go deeper by doing the next operation */
  for(const auto& one_result:*step_table) {
    auto one_record_table = make_shared<TableContent>();
    one_record_table->push_back(one_result);
    auto next_layer_result  =this->DepthSearchOneLayer(query_plan, layer_count + 1,result_number_till_now,  limit_number,
                                                       one_record_table, id_caches,table_template);

    /* if not success */
    if(!get<0>(next_layer_result))
      continue;

    auto record_extended = get<1>(next_layer_result).values_;
    if(record_extended->empty())
      continue;

    /* record the result, move the next layer result to this layer result */
    for(const auto& next_layer_one_result: *record_extended) {
      all_result->push_back(std::move(next_layer_one_result));
    }
    if(limit_number!=-1) {
      /* examine the number*/
      if (result_number_till_now >= limit_number)
        break;
    }
  }
  return make_tuple(true,layer_result);
}


tuple<bool,shared_ptr<IntermediateResult>> Optimizer::DoQuery(SPARQLquery &sparql_query,QueryInfo query_info) {
  return MergeBasicQuery(sparql_query);
}

/**
 * Query over bgp_query
 * @param bgp_query
 * @param query_info , information of limit number and order by expression
 * @return <if success, if the result ranked>
 */
tuple<bool, bool> Optimizer::DoQuery(std::shared_ptr<BGPQuery> bgp_query,QueryInfo query_info) {

#ifdef TOPK_DEBUG_INFO
  std::cout<<"Optimizer:: limit used:"<<query_info.limit_<<std::endl;
  std::cout<<"Optimizer::DoQuery limit num:"<<query_info.limit_num_<<std::endl;
#endif
  bool ranked = false;
  auto var_candidates_cache = bgp_query->get_all_candidates();
  // auto var_candidates_cache = make_shared<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>>();
  shared_ptr<DFSPlan> query_plan;
  auto strategy = this->ChooseStrategy(bgp_query,&query_info);
  auto distinct = bgp_query->distinct_query;
  if(strategy == BasicQueryStrategy::Normal)
  {
	PlanGenerator plan_generator(kv_store_, bgp_query.get(), var_candidates_cache, triples_num_,
								 limitID_predicate_, limitID_literal_, limitID_entity_, pre2num_, pre2sub_, pre2obj_, txn_);

    long t1 =Util::get_cur_time();
    auto const_candidates = FilterPlan::OnlyConstFilter(bgp_query, this->kv_store_);
    for (auto &constant_generating_step : *const_candidates)
      executor_.CacheConstantCandidates(constant_generating_step, true, var_candidates_cache);
    long t2 = Util::get_cur_time();
    cout << "get var cache, used " << (t2 - t1) << "ms." << endl;
    cout << "id_list.size = " << var_candidates_cache->size() << endl;
    cout << "limited literal  = " << limitID_literal_ << ", limited entity =  " << limitID_entity_ << endl;

    auto second_run_candidates_plan = plan_generator.CompleteCandidate();
    long t3 = Util::get_cur_time();
    cout << "complete candidate done, size = " << second_run_candidates_plan.size() << endl;
    for(const auto& constant_generating_step: second_run_candidates_plan)
      executor_.CacheConstantCandidates(constant_generating_step, true, var_candidates_cache);

    long t4 = Util::get_cur_time();
    PlanTree* best_plan_tree = plan_generator.GetPlan(true);
    long t5 = Util::get_cur_time();
    cout << "plan get, used " << (t5 - t4) + (t3 - t2) << "ms." << endl;
    best_plan_tree->print(bgp_query.get());
    cout << "plan print done" << endl;

    long t6 = Util::get_cur_time();
    auto bfs_result = this->ExecutionBreathFirst(bgp_query,query_info,best_plan_tree->root_node,var_candidates_cache);
    long t7 = Util::get_cur_time();
    cout << "execution, used " << (t7 - t6) << "ms." << endl;

    auto bfs_table = get<1>(bfs_result);
    auto pos_var_mapping = bfs_table.pos_id_map;
    auto var_pos_mapping = bfs_table.id_pos_map;
    long t8 = Util::get_cur_time();
    CopyToResult(bgp_query, bfs_table);
    long t9 = Util::get_cur_time();
    cout << "copy to result, used " << (t9 - t8) <<"ms." <<endl;
    cout << "total execution, used " << (t9 - t1) <<"ms."<<endl;
  }
  else if(strategy == BasicQueryStrategy::TopK)
  {
    ranked = true;
#ifdef TOPK_SUPPORT
    auto const_candidates = FilterPlan::OnlyConstFilter(bgp_query, this->kv_store_);
    for (auto &constant_generating_step: *const_candidates) {
      executor_.CacheConstantCandidates(constant_generating_step, distinct,var_candidates_cache);
    };
    if(const_candidates->empty())
    {
      auto predicate_filter = FilterPlan::PredicateFilter(bgp_query,this->kv_store_);
      for (auto &predicate_step: *predicate_filter) {
        executor_.CacheConstantCandidates(predicate_step,distinct, var_candidates_cache);
      }
    }

#ifdef TOPK_DEBUG_INFO
    std::cout<<"Top-k Constant Filtering Candidates Info"<<std::endl;
    for(const auto& pair:*var_candidates_cache)
      std::cout<<"var["<<pair.first<<"]  "<<pair.second->size()<<std::endl;
    std::cout<<"Top-k Constant Filtering Candidates Info End"<<std::endl;
#endif

    auto search_plan = make_shared<TopKSearchPlan>(bgp_query, this->kv_store_,
                                                   (*query_info.ordered_by_expressions_)[0],
                                                   var_candidates_cache);
    search_plan->GetPlan(bgp_query, this->kv_store_,
                         (*query_info.ordered_by_expressions_)[0],
                         var_candidates_cache);
#ifdef TOPK_DEBUG_INFO
    std::cout<<"Top-k Search Plan"<<std::endl;
    search_plan->DebugInfo(bgp_query,kv_store_);
#endif
    auto top_k_result = this->ExecutionTopK(bgp_query,search_plan,query_info,var_candidates_cache);
    auto result_table = get<1>(top_k_result);
    CopyToResult(bgp_query, result_table);
#endif
  }
  else if(strategy == BasicQueryStrategy::limitK)
  {
    PlanGenerator plan_generator(kv_store_, bgp_query.get(), var_candidates_cache, triples_num_,
                                 limitID_predicate_, limitID_literal_, limitID_entity_, pre2num_, pre2sub_, pre2obj_, txn_);

    long t1 =Util::get_cur_time();
    auto const_candidates = FilterPlan::OnlyConstFilter(bgp_query, this->kv_store_);
    for (auto &constant_generating_step : *const_candidates)
      executor_.CacheConstantCandidates(constant_generating_step, true, var_candidates_cache);
    long t2 = Util::get_cur_time();
    cout << "get var cache, used " << (t2 - t1) << "ms." << endl;
    cout << "id_list.size = " << var_candidates_cache->size() << endl;
    cout << "limited literal  = " << limitID_literal_ << ", limited entity =  " << limitID_entity_ << endl;

    auto second_run_candidates_plan = plan_generator.CompleteCandidate();
    long t3 = Util::get_cur_time();
    cout << "complete candidate done, size = " << second_run_candidates_plan.size() << endl;
    for(const auto& constant_generating_step: second_run_candidates_plan)
      executor_.CacheConstantCandidates(constant_generating_step, true, var_candidates_cache);

    long t4 = Util::get_cur_time();
    PlanTree* best_plan_tree = plan_generator.GetPlan(false);
    long t5 = Util::get_cur_time();
    cout << "plan get, used " << (t5 - t4) + (t3 - t2) << "ms." << endl;
    best_plan_tree->print(bgp_query.get());
    cout << "plan print done" << endl;

    long t6 = Util::get_cur_time();
    auto dfs_query_plan = make_shared<DFSPlan>(best_plan_tree->root_node);
    auto dfs_result = this->ExecutionDepthFirst(bgp_query, dfs_query_plan,
                                                query_info,var_candidates_cache);
    long t7 = Util::get_cur_time();
    cout << "execution, used " << (t7 - t6) << "ms." << endl;

    auto bfs_table = get<1>(dfs_result);
    auto pos_var_mapping = bfs_table.pos_id_map;
    auto var_pos_mapping = bfs_table.id_pos_map;
    long t8 = Util::get_cur_time();
    CopyToResult(bgp_query, bfs_table);
    long t9 = Util::get_cur_time();
    cout << "copy to result, used " << (t9 - t8) <<"ms." <<endl;
    cout << "total execution, used " << (t9 - t1) <<"ms."<<endl;
  }
  return make_tuple<bool, bool>(true,std::move(ranked));

}

tuple<bool, shared_ptr<IntermediateResult>> Optimizer::MergeBasicQuery(SPARQLquery &sparql_query) {
  return tuple<bool, shared_ptr<IntermediateResult>>();
}

bool Optimizer::CopyToResult(shared_ptr<BGPQuery> bgp_query,
                             IntermediateResult result) {

  auto target = bgp_query->get_result_list_pointer1();
  assert(target->empty());
  target->reserve(result.values_->size());
  auto record_len = bgp_query->selected_so_var_num + bgp_query->selected_pre_var_num;
  auto id_position_map_ptr = result.id_pos_map;
  // position_map[i] means in the new table, the i-th column
  // is the  position_map[i]-th column from old table
  const auto& result_position_to_id = bgp_query->resultPositionToId();
  auto position_map = new TYPE_ENTITY_LITERAL_ID[record_len];

  for(decltype(record_len) pos = 0;pos<record_len;pos++ )
  {
    auto var_id = result_position_to_id[pos];
    auto old_position = (*id_position_map_ptr)[var_id];
    position_map[pos] = old_position;
  }
  // Data flows from record_ptr to tmp_record and then back to record_ptr
  // all vector in result.values_ are moved to target
  auto tmp_record = new unsigned[record_len];
  for (auto&  record_ref : *(result.values_))
  {
    for (int column_index = 0; column_index < record_len; ++column_index){
      auto old_position = position_map[column_index];
      tmp_record[column_index] = record_ref[old_position];
    }
    for (int column_index = 0; column_index < record_len; ++column_index){
      record_ref[column_index] = tmp_record[column_index];
    }
    record_ref.resize(record_len);
    target->emplace_back(std::move(record_ref));
  }
  delete[] tmp_record;
  delete[] position_map;
  return false;
}

// Note: one triple query, do not need edge_constant_info
void Optimizer::GetResultListByEdgeInfo(const EdgeInfo &edge_info,
										unsigned int *&result_list_pointer,
										unsigned int &result_list_len,
										bool distinct) {

	switch (edge_info.join_method_) {
		case JoinMethod::s2p: { // Because if we don't add a pair of '{}', the editor will report a error of redefinition
			unsigned const_s_id = edge_info.s_;
			this->kv_store_->getpreIDlistBysubID(const_s_id,
												 result_list_pointer,
												 result_list_len,
												 distinct,
												 this->txn_);
			break;
		}
		case JoinMethod::s2o: {
			unsigned const_s_id = edge_info.s_;
			this->kv_store_->getobjIDlistBysubID(const_s_id,
												 result_list_pointer,
												 result_list_len,
												 distinct,
												 this->txn_);
			break;
		}
		case JoinMethod::p2s: {
			unsigned const_p_id = edge_info.p_;
			this->kv_store_->getsubIDlistBypreID(const_p_id,
												 result_list_pointer,
												 result_list_len,
												 distinct,
												 this->txn_);
			break;
		}
		case JoinMethod::p2o: {
			unsigned const_p_id = edge_info.p_;
			this->kv_store_->getobjIDlistBypreID(const_p_id,
												 result_list_pointer,
												 result_list_len,
												 distinct,
												 this->txn_);
			break;
		}
		case JoinMethod::o2s: {
			unsigned const_o_id = edge_info.o_;
			this->kv_store_->getsubIDlistByobjID(const_o_id,
												 result_list_pointer,
												 result_list_len,
												 distinct,
												 this->txn_);
			break;
		}
		case JoinMethod::o2p: {
			unsigned const_o_id = edge_info.o_;
			this->kv_store_->getpreIDlistByobjID(const_o_id,
												 result_list_pointer,
												 result_list_len,
												 distinct,
												 this->txn_);
			break;
		}
		case JoinMethod::so2p: {
			unsigned const_s_id = edge_info.s_;
			unsigned const_o_id = edge_info.o_;
			this->kv_store_->getpreIDlistBysubIDobjID(const_s_id,
													  const_o_id,
													  result_list_pointer,
													  result_list_len,
													  distinct,
													  this->txn_);
			break;
		}
		case JoinMethod::sp2o: {
			unsigned const_s_id = edge_info.s_;
			unsigned const_p_id = edge_info.p_;
			this->kv_store_->getobjIDlistBysubIDpreID(const_s_id,
													  const_p_id,
													  result_list_pointer,
													  result_list_len,
													  distinct,
													  this->txn_);
			break;
		}
		case JoinMethod::po2s: {
			unsigned const_o_id = edge_info.o_;
			unsigned const_p_id = edge_info.p_;
			this->kv_store_->getsubIDlistByobjIDpreID(const_o_id,
													  const_p_id,
													  result_list_pointer,
													  result_list_len,
													  distinct,
													  this->txn_);
			break;
		}
		case JoinMethod::s2po: {
			unsigned const_s_id = edge_info.s_;
			this->kv_store_->getpreIDobjIDlistBysubID(const_s_id,
													  result_list_pointer,
													  result_list_len,
													  distinct,
													  this->txn_);
			break;
		}
		case JoinMethod::p2so: {
			unsigned const_p_id = edge_info.p_;
			this->kv_store_->getsubIDobjIDlistBypreID(const_p_id,
													  result_list_pointer,
													  result_list_len,
													  distinct,
													  this->txn_);
			break;
		}
		case JoinMethod::o2ps: {
			unsigned const_o_id = edge_info.o_;
			this->kv_store_->getpreIDsubIDlistByobjID(const_o_id,
													  result_list_pointer,
													  result_list_len,
													  distinct,
													  this->txn_);
			break;
		}
		default:
			throw "ExtendRecordOneNode only support add 1 node each time";
	}
}

/**
 *
 * @param basic_query
 * @param query_info
 * @param plan_tree_node
 * @param id_caches
 * @return IntermediateResult
 */
tuple<bool,IntermediateResult> Optimizer::ExecutionBreathFirst(shared_ptr<BGPQuery> bgp_query,
                                                               QueryInfo query_info,
                                                               Tree_node* plan_tree_node,
                                                               IDCachesSharePtr id_caches)
{
  // leaf node
  auto step_operation = plan_tree_node->node;
  // auto operation_type = step_operation->join_type_;
  auto operation = step_operation->op_type_;
  auto range = step_operation->GetRange();
  // leaf node : create a table
  if (plan_tree_node->left_node == nullptr && plan_tree_node->right_node == nullptr)
  {
    if(operation == StepOperation::StepOpType::Extend){
      return InitialTable(bgp_query, id_caches, step_operation, gstore::Executor::NO_LIMIT_OUTPUT);
    }
    else if(operation == StepOperation::StepOpType::Filter){
      return UpdateCandidates(id_caches,step_operation);
    }
    else if(operation == StepOperation::StepOpType::TableJoin)
      throw string("StepOperation::JoinType::JoinTable cannot happened in leaf node");
    else if(operation == StepOperation::StepOpType::Check)
      throw string("StepOperation::JoinType::EdgeCheck cannot happened in leaf node");
    else
      throw string("unexpected JoinType");
    return make_tuple(true,IntermediateResult());
  }

  //inner node
  tuple<bool,IntermediateResult> left_r;
  tuple<bool,IntermediateResult> right_r;

  if(plan_tree_node->left_node != nullptr)
    left_r = this->ExecutionBreathFirst(bgp_query,query_info, plan_tree_node->left_node,id_caches);

  if(plan_tree_node->right_node != nullptr)
    right_r = this->ExecutionBreathFirst(bgp_query,query_info, plan_tree_node->right_node,id_caches);

  if(operation == StepOperation::StepOpType::Filter){
    return UpdateCandidates(id_caches,step_operation);
  }

  auto left_table = get<1>(left_r);
  // reading the left child result
  auto left_records = left_table.values_;

  // JoinNode/EdgeCheck/JoinTwoNodes has only ONE left child
  if(operation==StepOperation::StepOpType::Extend || operation==StepOperation::StepOpType::Satellite) {
    // create a new table
    if(left_table.GetColumns()==0){
      return InitialTable(bgp_query, id_caches, step_operation, gstore::Executor::NO_LIMIT_OUTPUT);
    }
    if (range == StepOperation::OpRangeType::OneNode)
    {
      return OperateOneNode(bgp_query, step_operation, id_caches, left_table, gstore::Executor::NO_LIMIT_OUTPUT);
    }
    else if(range == StepOperation::OpRangeType::TwoNode)
    {
      return ExtendTwoNode(bgp_query, plan_tree_node->node, id_caches, left_table, gstore::Executor::NO_LIMIT_OUTPUT);
    }
  }
  else if(operation==StepOperation::StepOpType::Check){
    return TableCheck(bgp_query,step_operation, left_table);
  }

  auto right_table = get<1>(right_r);
  if(operation==StepOperation::StepOpType::TableJoin){
    return JoinTable(bgp_query,step_operation, left_table, right_table);
  }
  else
    throw string("unexpected StepOperation::StepOpType");
  return make_tuple(false,left_table);
}



/**
 * calculator which role a variable node maybe
 * @param bgp_query
 * @param join_a_node_plan
 * @return is_entity,is_literal,is_predicate
 */
tuple<bool,bool,bool>
Optimizer::PrepareInitial(const shared_ptr<BGPQuery>& bgp_query,
                          shared_ptr<AffectOneNode> join_a_node_plan) const {
  auto target_var_id = join_a_node_plan->node_to_join_;
  PrintDebugInfoLine(g_format("leaf node [ %s ]", bgp_query->get_var_name_by_id(target_var_id).c_str()));
  return bgp_query->GetOccurPosition(target_var_id);
}

#ifdef TOPK_SUPPORT
tuple<bool,IntermediateResult> Optimizer::ExecutionTopK(shared_ptr<BGPQuery> bgp_query, shared_ptr<TopKSearchPlan> &tree_search_plan,
                                             const QueryInfo& query_info,IDCachesSharePtr id_caches){

  auto pos_var_mapping = TopKUtil::CalculatePosVarMapping(tree_search_plan);
  auto k = query_info.limit_num_;
  auto first_item = (*query_info.ordered_by_expressions_)[0];
  auto var_coefficients = TopKUtil::getVarCoefficients(first_item);
  if(first_item.descending)
    std::for_each(var_coefficients->begin(),
                  var_coefficients->end(),
                  [](decltype(*var_coefficients->begin()) pair_it)
                  {
                    pair_it.second = -pair_it.second;
                  });
  auto a = *var_coefficients->begin();
  // Build Iterator tree
  auto env = new TopKUtil::Env();
  env->kv_store= this->kv_store_;
  env->limitID_entity = this->limitID_entity_;
  env->limitID_literal = this->limitID_literal_;
  env->bgp_query = bgp_query;
  env->id_caches = id_caches;
  cout<<" Optimizer::ExecutionTopK  env->id_caches "<<  env->id_caches->size()<<endl;
  env->k = query_info.limit_num_;
  env->coefficients = var_coefficients;
  env->txn = this->txn_;
  env->ss = make_shared<stringstream>();

  auto root_fr = DPBUtil::BuildIteratorTree(tree_search_plan, env);

  decltype(query_info.limit_num_) deleted_num = 0;
  auto result_list = make_shared<list<vector<TYPE_ENTITY_LITERAL_ID>>>();

  auto result_table = IntermediateResult();
  result_table.values_ = result_list;
  result_table.pos_id_map = pos_var_mapping;
  for(auto pos_id_pair:*pos_var_mapping)
    (*result_table.id_pos_map)[pos_id_pair.second] = pos_id_pair.first;

  auto temp_table = IntermediateResult();
  temp_table.pos_id_map = result_table.pos_id_map;
  temp_table.id_pos_map = result_table.id_pos_map;

#ifdef TOPK_DEBUG_INFO
  for(const auto& pos_id_pair:*pos_var_mapping)
  {
    cout<<"pos["<<pos_id_pair.first<<"]"<<"|"<<pos_id_pair.second<<"| "<<bgp_query->get_var_name_by_id(pos_id_pair.second)<<"\t";
  }
  cout<<endl;
#endif

  auto& select_var = bgp_query->selected_var_id;
  vector<unsigned int> select_position;
  for_each(select_var.begin(),select_var.end(),[&select_position,&result_table](unsigned int x){
    select_position.push_back(result_table.id_pos_map->operator[](x));
  });
  vector<double> score_list;
  // for checking the distinct property
  std::set<vector<TYPE_ENTITY_LITERAL_ID>> selected_id_set;
  score_list.reserve(query_info.limit_num_);
  for(int i =1; i<=query_info.limit_num_ + deleted_num; i++)
  {
    root_fr->TryGetNext(k+ deleted_num);
    if(root_fr->pool_.size()!=(unsigned int)i)
      break;
#ifdef TOPK_DEBUG_INFO
    else {
      cout << "get top-" << i << " "<<root_fr->pool_[i-1].cost;
    }
#endif
    auto temp_score = root_fr->pool_[i-1].cost;
    // can't get any more
    if((unsigned)i>root_fr->pool_.size())
      break;

    auto var_num = pos_var_mapping->size();

    auto record = vector<TYPE_ENTITY_LITERAL_ID>();
    record.reserve(var_num);
    root_fr->GetResult(i-1,record);

    bool success = true;
    if(tree_search_plan->HasCycle())
    {
      auto &non_tree_edge = tree_search_plan->GetNonTreeEdges();
      auto temp_content_ptr = make_shared<TableContent>();
      temp_content_ptr->push_back(record);
      temp_table.values_ = temp_content_ptr;
      auto filter_result = this->executor_.ANodeEdgesConstraintFilter(non_tree_edge.GetOneNodePlan(),temp_table,false);
      auto filter_result_size = get<1>(filter_result).values_->size();
      success = filter_result_size != 0;
    }
    if(query_info.is_distinct_)
    {
      vector<TYPE_ENTITY_LITERAL_ID> select_id;
      for_each(select_position.begin(),select_position.end(),[&select_id,&record](unsigned int pos){
        select_id.push_back(record.at(pos));
      });
      if(selected_id_set.count(select_id))
        success = false;
      else
        selected_id_set.insert(std::move(select_id));
    }
#ifdef TOPK_DEBUG_INFO
    if(!success)
      cout << "  deleted";
    cout<<endl;
#endif
    if(success)
    {
      score_list.push_back(temp_score);
      result_list->push_back(record);
    }
    else
      deleted_num += 1;
  }
#ifdef TOPK_DEBUG_RESULT_INFO
  auto var_num = bgp_query->get_total_var_num();
  vector<bool> is_predicate_var(var_num,false);
  for(unsigned i =0;i<var_num;i++)
    is_predicate_var[i] = bgp_query->get_vardescrip_by_index(i)->var_type_ == VarDescriptor::VarType::Predicate;

  auto it = result_list->begin();
  for(decltype(result_list->size()) i =0;i<result_list->size();i++)
  {
    auto& rec = *it;
    cout<<" record["<<i<<"]"<<" score:"<<score_list[i];

    for(unsigned j =0;j<var_num;j++)
      if(is_predicate_var[(*pos_var_mapping)[j]])
        cout<<" "<<kv_store_->getPredicateByID(rec[j])<<"["<<rec[j]<<"]";
      else
        cout<<" "<<kv_store_->getStringByID(rec[j])<<"["<<rec[j]<<"]";;
    cout<<endl;
    it++;
  }
#endif
  delete root_fr;
  delete env;
  return std::make_tuple(true,result_table);
}


tuple<bool,IntermediateResult> Optimizer::InitialTable(shared_ptr<BGPQuery> &bgp_query,
                                            const IDCachesSharePtr &id_caches,
                                            shared_ptr<StepOperation> &step_operation,
                                            size_t max_output) {
  auto operation = step_operation->op_type_;
  auto range = step_operation->GetRange();
  long t1 = GetTimeDebug();

  IntermediateResult leaf_table;
  if(range == StepOperation::OpRangeType::OneNode)
  {
    auto r = PrepareInitial(bgp_query, step_operation->GetOneNodePlan());
    bool is_entity = get<0>(r);
    bool is_literal = get<1>(r);
    bool is_predicate = get<2>(r);
    auto initial_result =
        executor_.InitialTableOneNode(step_operation->GetOneNodePlan(), is_entity, is_literal, is_predicate,
                                      step_operation->distinct_,max_output, id_caches);
    leaf_table = get<1>(initial_result);
    PrintDebugInfoLine(g_format("result size: %d", leaf_table.values_->size()));
    PrintTimeOpRange(operation,range,t1);
  }
  else if(range == StepOperation::OpRangeType::TwoNode)
  {
    auto two_node_plan = step_operation->GetTwoNodePlan();
    auto initial_result = executor_.InitialTableTwoNode(two_node_plan, max_output, id_caches);
    leaf_table = get<1>(initial_result);
    auto node1 = two_node_plan->node_to_join_1_;
    auto node2 = two_node_plan->node_to_join_2_;
    PrintDebugInfoLine(g_format("join two nodes [ %s, %s ] result size: %d",
                                bgp_query->get_var_name_by_id(node1).c_str(),
                                bgp_query->get_var_name_by_id(node2).c_str(),
                                leaf_table.values_->size()));
    PrintTimeOpRange(operation,range,t1);
  }
  else if (range == StepOperation::OpRangeType::GetAllTriples) {
    auto all_triple_result = executor_.GetAllTriple(max_output, step_operation->GetOneNodePlan());
    leaf_table = get<1>(all_triple_result);
    PrintDebugInfoLine(g_format("result size: %d", leaf_table.values_->size()));
    PrintTimeOpRange(operation,range,t1);
  }
  else
    throw string("StepOperation::StepOpType::Extend: invalid range type");
  PrintTableDebug(leaf_table,bgp_query);
  return make_tuple(true,std::move(leaf_table));
}

tuple<bool,IntermediateResult> Optimizer::UpdateCandidates(IDCachesSharePtr &id_caches,
                                               shared_ptr<StepOperation> &step_operation)
                                               {
  long t1 = GetTimeDebug();
  auto operation = step_operation->op_type_;
  auto range = step_operation->GetRange();
  executor_.UpdateIDCache(step_operation->GetOneNodePlan(),id_caches,step_operation->distinct_);
  PrintTimeOpRange(operation,range,t1);
  return make_tuple(true,IntermediateResult());
}

/**
 * Add a column into the left_table, or just check a column (which may enlarge the result)
 * @param left_table the table operated on
 * @param max_output the limited size new table should be
 */
tuple<bool, IntermediateResult> Optimizer::OperateOneNode(shared_ptr<BGPQuery> &bgp_query,
                                                          shared_ptr<StepOperation> &step_operation,
                                                          const IDCachesSharePtr &id_caches,
                                                          IntermediateResult &left_table,
                                                          size_t max_output) {
  auto operation = step_operation->op_type_;
  auto range = step_operation->GetRange();
  auto remain_old = step_operation->remain_old_result_;
  auto one_node_plan = step_operation->GetOneNodePlan();
  auto node_to_join = one_node_plan->node_to_join_;
  PrintDebugInfoLine(g_format("join node [ %s ] ", bgp_query->get_var_name_by_id(node_to_join).c_str()));
  long t1 = GetTimeDebug();
  auto step_result =
      executor_.AffectANode(left_table, id_caches, operation != StepOperation::StepOpType::Satellite ,
                            step_operation->distinct_, remain_old,max_output, one_node_plan);
  PrintTimeOpRange(operation, range, t1);
  PrintTableDebug(get<1>(step_result),bgp_query);
  return step_result;
}

tuple<bool, IntermediateResult> Optimizer::ExtendTwoNode(shared_ptr<BGPQuery> &bgp_query,
                                                         shared_ptr<StepOperation> &step_operation,
                                                         const IDCachesSharePtr &id_caches,
                                                         IntermediateResult& left_table,
                                                         size_t max_output) {
  auto join_two_plan = step_operation->GetTwoNodePlan();
  auto operation = step_operation->op_type_;
  auto range = step_operation->GetRange();
  auto node1 = join_two_plan->node_to_join_1_;
  auto node2 = join_two_plan->node_to_join_2_;
  long t1 = GetTimeDebug();
  PrintDebugInfoLine(g_format("join two nodes [ %s, %s ] ",
                              bgp_query->get_var_name_by_id(node1).c_str(),
                              bgp_query->get_var_name_by_id(node2).c_str()));
  auto step_result = executor_.JoinTwoNode(join_two_plan,
                                           left_table, id_caches,
                                           step_operation->remain_old_result_,max_output);
  PrintTimeOpRange(operation,range,t1);
  PrintTableDebug(get<1>(step_result),bgp_query);
  return step_result;
}

tuple<bool, IntermediateResult> Optimizer::TableCheck(shared_ptr<BGPQuery> &bgp_query,
                                                      shared_ptr<StepOperation> &step_operation,
                                                      IntermediateResult &left_table) {
  auto edge_filter = step_operation->GetOneNodePlan();
  long t1 = GetTimeDebug();
  auto operation = step_operation->op_type_;
  auto range = step_operation->GetRange();
  auto step_result = executor_.ANodeEdgesConstraintFilter(edge_filter,
                                                          left_table,
                                                          step_operation->remain_old_result_);
  PrintTimeOpRange(operation,range,t1);
  PrintTableDebug(get<1>(step_result),bgp_query);
  return step_result;
}

tuple<bool, IntermediateResult> Optimizer::JoinTable(shared_ptr<BGPQuery> &bgp_query,
                                                     shared_ptr<StepOperation> &step_operation,
                                                     IntermediateResult &left_table,
                                                     IntermediateResult &right_table) {
  long t1 = GetTimeDebug();
  auto operation = step_operation->op_type_;
  auto range = step_operation->GetRange();
  auto join_result =  executor_.JoinTable(step_operation->GetTwoTablePlan(),
                                          left_table, right_table, step_operation->remain_old_result_);
  PrintTimeOpRange(operation,range,t1);
  PrintTableDebug(get<1>(join_result),bgp_query);
  return join_result;
}
#ifdef OPTIMIZER_DEBUG_INFO
void Optimizer::PrintTable(IntermediateResult& result,
                                shared_ptr<BGPQuery> &bgp_query){
  cout << "\tresult size = " << result.values_->size() << endl;
  auto &table = *result.values_;
  auto result_size = result.values_->size();
  PrintDebugInfoLine(g_format("Print Table of size %d ",result_size));
  auto pos_id = result.pos_id_map;
  auto col_size = pos_id->size();
  vector<bool> pos_predicate(col_size,false);
  for(auto &pos_id:*result.pos_id_map) {
    auto var_desc = bgp_query->get_vardescrip_by_id(pos_id.second);
    auto var_name = var_desc->var_name_;
    auto var_type = VarDescriptor::GetString(var_desc->var_type_);
    PrintDebugInfoLine(g_format("pos2id[%d]=%d %s %s", pos_id.first,
                                pos_id.second,var_name.c_str(),var_type.c_str()));
    if(var_desc->var_type_==VarDescriptor::VarType::Predicate)
      pos_predicate[pos_id.first] = true;
  }
  PrintDebugInfoLine("The first 10 records:");
  size_t c = 0;
  for(auto it = table.begin();it !=table.end() && c<10 ;c++,it++)
  {
    cout<<"    record["<<c<<"] ";
    auto& record = *it;
    for(size_t j = 0;j < col_size;j++)
    {
      string s;
      if(pos_predicate[j])
        s = kv_store_->getPredicateByID(record[j]);
      else
        s = kv_store_->getEntityByID(record[j]);
      cout<<s<<"\t";
    }
    cout<<endl;
  }
  return;
}
#endif // OPTIMIZER_DEBUG_INFO

#endif