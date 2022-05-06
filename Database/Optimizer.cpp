/*=============================================================================
# Filename: Optimizer.cpp
# Author: Yuqi Zhou, Linglin Yang
# Mail: zhouyuqi@pku.edu.cn, fyulingi@gmail.com
=============================================================================*/

#include "Optimizer.h"
// #define FEED_PLAN

Optimizer::Optimizer(KVstore *kv_store,
                     Statistics *statistics,
                     TYPE_TRIPLE_NUM *pre2num,
                     TYPE_TRIPLE_NUM *pre2sub,
                     TYPE_TRIPLE_NUM *pre2obj,
                     TYPE_TRIPLE_NUM triples_num,
                     TYPE_PREDICATE_ID limitID_predicate,
                     TYPE_ENTITY_LITERAL_ID limitID_literal,
                     TYPE_ENTITY_LITERAL_ID limitID_entity,
                     shared_ptr<Transaction> txn
):
    kv_store_(kv_store), statistics(statistics), pre2num_(pre2num),
    pre2sub_(pre2obj),pre2obj_(pre2obj),triples_num_(triples_num),
    limitID_predicate_(limitID_predicate), limitID_literal_(limitID_literal),limitID_entity_(limitID_entity),
    txn_(std::move(txn)), executor_(kv_store,txn,limitID_predicate,limitID_literal,limitID_entity_){}

BasicQueryStrategy Optimizer::ChooseStrategy(std::shared_ptr<BGPQuery> bgp_query,QueryInfo *query_info){
  if (!query_info->limit_)
  {
  	return BasicQueryStrategy::Normal;
  }
  else
  {
    if(query_info->ordered_by_expressions_->size() == 1) {
      // TODO: more accurate decision of algebraic expressions that can be processed by topk
      if((*query_info->ordered_by_expressions_)[0].comp_tree_root.children.empty())
        return BasicQueryStrategy::Normal;
      auto search_plan = make_shared<TopKSearchPlan>(bgp_query, this->kv_store_,this->statistics,
                                                     (*query_info->ordered_by_expressions_)[0],nullptr);
      if(search_plan->SuggestTopK())
        return BasicQueryStrategy::TopK;
      else
        return BasicQueryStrategy::Normal;
    }
    else if (query_info->ordered_by_expressions_->size() > 1)
      return BasicQueryStrategy::Normal;
    else {
      if (bgp_query->get_triple_num() == 1)
        return BasicQueryStrategy::Normal;
      else
        return BasicQueryStrategy::limitK;
    }
  }
}

/**
 * Generate a list of Intermediate templates, so each layer doesn't have to
 * calculate it each time
 */
std::shared_ptr<std::vector<IntermediateResult>> Optimizer::GenerateResultTemplate(shared_ptr<QueryPlan> query_plan)
{
  auto table_template = make_shared<std::vector<IntermediateResult>>();
  table_template->emplace_back();
  auto &tables = *table_template;
  auto &dfs_operation = *query_plan->join_order_;

  auto join_type = dfs_operation[0].join_type_;
  if(join_type==StepOperation::JoinType::JoinNode)
    tables[0].AddNewNode(dfs_operation[0].join_node_->node_to_join_);
  else if(join_type==StepOperation::JoinType::JoinTwoNodes) {
    tables[0].AddNewNode(dfs_operation[0].join_two_node_->node_to_join_1_);
    tables[0].AddNewNode(dfs_operation[0].join_two_node_->node_to_join_2_);
  }

  auto it = dfs_operation.begin();
  it++;
  while(it!=dfs_operation.end())
  {
    tables.emplace_back( IntermediateResult::OnlyPositionCopy(tables.back()));
    auto join_type = it->join_type_;
    if(join_type==StepOperation::JoinType::JoinNode)
      tables.back().AddNewNode( it->join_node_->node_to_join_);
    else if(join_type==StepOperation::JoinType::JoinTwoNodes) {
      tables.back().AddNewNode(it->join_two_node_->node_to_join_1_);
      tables.back().AddNewNode(it->join_two_node_->node_to_join_2_);
    }
    it++;
  }
  return table_template;
}

tuple<bool, IntermediateResult> Optimizer:: ExecutionDepthFirst(shared_ptr<BGPQuery> bgp_query,
                                                               shared_ptr<QueryPlan> query_plan,
                                                               QueryInfo query_info,
                                                               IDCachesSharePtr id_caches) {
  auto limit_num = query_info.limit_num_;
  cout<<"Optimizer::ExecutionDepthFirst query_info.limit_num_="<<query_info.limit_num_<<endl;
  auto &first_operation = (*query_plan->join_order_)[0];
  tuple<bool, IntermediateResult> step_result;
  IntermediateResult first_table;
  if(first_operation.join_type_==StepOperation::JoinType::JoinNode)
  {
    auto property_array = PrepareInitial(bgp_query,first_operation.join_node_);
    auto is_entity = get<0>(property_array);
    auto is_literal = get<1>(property_array);
    auto is_predicate = get<2>(property_array);
    step_result = executor_.InitialTableOneNode(first_operation.join_node_,is_entity,
                                                is_literal,is_predicate,first_operation.distinct_ , id_caches);
    first_table = get<1>(step_result);
  }
  else // Two Nodes
  {
    step_result  = executor_.InitialTableTwoNode(first_operation.join_two_node_,id_caches);
    first_table = get<1>(step_result);
  }
  if( query_plan->join_order_->size()==1 || first_table.values_->empty())
    return make_tuple(true, first_table);

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

  cout<<"Optimizer::ExecutionDepthFirst result_container.size()="<<result_container.size()<<endl;
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
  cout<<"Optimizer::ExecutionDepthFirst final_result.size()="<<final_result.values_->size()<<endl;
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
tuple<bool,IntermediateResult> Optimizer::DepthSearchOneLayer(shared_ptr<QueryPlan> query_plan,
                                                              int layer_count,
                                                              int &result_number_till_now,
                                                              int limit_number,
                                                              TableContentShardPtr table_content_ptr,
                                                              IDCachesSharePtr id_caches,
                                                              shared_ptr<vector<IntermediateResult>> table_template) {
  IntermediateResult old_table;
  old_table.values_ = table_content_ptr;
  old_table.pos_id_map = (*table_template)[layer_count-1].pos_id_map;
  old_table.id_pos_map = (*table_template)[layer_count-1].id_pos_map;

  IntermediateResult layer_result;
  layer_result.pos_id_map = (*table_template)[layer_count].pos_id_map;
  layer_result.id_pos_map = (*table_template)[layer_count].id_pos_map;

  auto one_step = (*(query_plan->join_order_))[layer_count];
  TableContentShardPtr step_table;
  switch (one_step.join_type_) {
    case StepOperation::JoinType::JoinNode: {
      auto step_result = executor_.JoinANode(old_table,id_caches,one_step.distinct_,false,one_step.join_node_);
      step_table = get<1>(step_result).values_;
      break;
    }
    case StepOperation::JoinType::JoinTable: {
      throw string("Optimizer::DepthSearchOneLayer not suitable for JoinTable");
    }
    case StepOperation::JoinType::GenerateCandidates : {
      throw string("Optimizer::DepthSearchOneLayer not suitable for GenerateCandidates");
    }
    case StepOperation::JoinType::EdgeCheck: {
      auto step_result = executor_.ANodeEdgesConstraintFilter(one_step.edge_filter_, old_table,false);
      step_table = get<1>(step_result).values_;
      break;
    }
    case StepOperation::JoinType::JoinTwoNodes: {
      auto step_result = executor_.JoinTwoNode(one_step.join_two_node_, old_table,id_caches,false);
      step_table = get<1>(step_result).values_;
      break;
    }
  }


  if(step_table->empty())
    return make_tuple(false, layer_result);

  auto& all_result = layer_result.values_;

  /* deep in bottom, update the 'result_number_till_now' */
  if( (unsigned )layer_count + 1 == query_plan->join_order_->size()) {
    result_number_till_now += step_table->size();
    for(auto it = step_table->begin();it!=step_table->end();it++)
      all_result->push_back(std::move(*it));
    return make_tuple(true, layer_result);
  }

  /*  go deeper by filling a node */
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
  shared_ptr<QueryPlan> query_plan;
  auto strategy = this->ChooseStrategy(bgp_query,&query_info);
  auto distinct = bgp_query->distinct_query;
  if(strategy == BasicQueryStrategy::Normal)
  {

    PlanTree* best_plan_tree;
	PlanGenerator plan_generator(kv_store_, bgp_query.get(), statistics, var_candidates_cache, triples_num_,
								 limitID_predicate_, limitID_literal_, limitID_entity_, pre2num_, pre2sub_, pre2obj_, txn_);

    long t1 =Util::get_cur_time();

    if(bgp_query->get_triple_num()==1)
      best_plan_tree = plan_generator.get_special_one_triple_plan();
    else{
      auto const_candidates = QueryPlan::OnlyConstFilter(bgp_query,this->kv_store_);
      for (auto &constant_generating_step: *const_candidates)
        executor_.CacheConstantCandidates(constant_generating_step, true, var_candidates_cache);

      long t2 = Util::get_cur_time();
      cout << "get var cache, used " << (t2 - t1) << "ms." << endl;
      long t3 = Util::get_cur_time();
      cout << "id_list.size = " << var_candidates_cache->size() << endl;

	  cout << "limited literal  = " << limitID_literal_ << ", limited entity =  " << limitID_entity_ << endl;

	  auto second_run_candidates_plan = plan_generator.completecandidate();
	  long t3_ = Util::get_cur_time();
	  cout << "complete candidate done, size = " << second_run_candidates_plan.size() << endl;

      for(const auto& constant_generating_step: second_run_candidates_plan)
        executor_.CacheConstantCandidates(constant_generating_step, true, var_candidates_cache);

	  long t4_ = Util::get_cur_time();
      best_plan_tree = plan_generator.get_plan(true);
      long t4 = Util::get_cur_time();
      cout << "plan get, used " << (t4 - t4_) + (t3_ - t3) << "ms." << endl;
    }

    best_plan_tree->print(bgp_query.get());
    cout << "plan print done" << endl;

    long t_ = Util::get_cur_time();
    auto bfs_result = this->ExecutionBreathFirst(bgp_query,query_info,best_plan_tree->root_node,var_candidates_cache);

    // todo: Destructor of PlanGenerator here
    long t5 = Util::get_cur_time();
    cout << "execution, used " << (t5 - t_) << "ms." << endl;

    auto bfs_table = get<1>(bfs_result);
    auto pos_var_mapping = bfs_table.pos_id_map;
    auto var_pos_mapping = bfs_table.id_pos_map;

    long t6 = Util::get_cur_time();
    CopyToResult(bgp_query, bfs_table);
#ifdef OPTIMIZER_DEBUG_INFO
    cout<<"after copy bfs result size "<<bgp_query->get_result_list_pointer()->size()<<endl;
#endif
    long t7 = Util::get_cur_time();
    cout << "copy to result, used " << (t7 - t6) <<"ms." <<endl;
    cout << "total execution, used " << (t7 - t1) <<"ms."<<endl;
  }
  else if(strategy ==BasicQueryStrategy::Special){
    printf("BasicQueryStrategy::Special not supported yet\n");
  }
  else if(strategy == BasicQueryStrategy::TopK)
  {
    ranked = true;
#ifdef TOPK_SUPPORT
    auto const_candidates = QueryPlan::OnlyConstFilter(bgp_query, this->kv_store_);
    for (auto &constant_generating_step: *const_candidates) {
      executor_.CacheConstantCandidates(constant_generating_step, distinct,var_candidates_cache);
    };
    if(const_candidates->empty())
    {
      auto predicate_filter = QueryPlan::PredicateFilter(bgp_query,this->kv_store_);
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
                                                   this->statistics, (*query_info.ordered_by_expressions_)[0],
                                                   var_candidates_cache);
    search_plan->GetPlan(bgp_query, this->kv_store_,
                         this->statistics, (*query_info.ordered_by_expressions_)[0],
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
    PlanTree* best_plan_tree;
	PlanGenerator plan_generator(kv_store_, bgp_query.get(), statistics, var_candidates_cache, triples_num_,
								 limitID_predicate_, limitID_literal_, limitID_entity_, pre2num_, pre2sub_, pre2obj_, txn_);

	long t1 =Util::get_cur_time();

	if(bgp_query->get_triple_num()==1)
	  best_plan_tree = plan_generator.get_special_one_triple_plan();
	else{
	  auto const_candidates = QueryPlan::OnlyConstFilter(bgp_query,this->kv_store_);
	  for (auto &constant_generating_step: *const_candidates)
			  executor_.CacheConstantCandidates(constant_generating_step, true, var_candidates_cache);

	  long t2 = Util::get_cur_time();
	  cout << "get var cache, used " << (t2 - t1) << "ms." << endl;
	  long t3 = Util::get_cur_time();
	  cout << "id_list.size = " << var_candidates_cache->size() << endl;

	  auto second_run_candidates_plan = plan_generator.completecandidate();
	  long t3_ = Util::get_cur_time();
	  cout << "complete candidate done, size = " << second_run_candidates_plan.size() << endl;

	  for(const auto& constant_generating_step: second_run_candidates_plan)
			  executor_.CacheConstantCandidates(constant_generating_step, true, var_candidates_cache);

	  long t4_ = Util::get_cur_time();
	  best_plan_tree = plan_generator.get_plan(false);
	  long t4 = Util::get_cur_time();
	  cout << "plan get, used " << (t4 - t4_) + (t3_ - t3) << "ms." << endl;
	}

    best_plan_tree->print(bgp_query.get());
    cout << "plan print done" << endl;

    long t_ = Util::get_cur_time();
    auto dfs_result = this->ExecutionDepthFirst(bgp_query, make_shared<QueryPlan>(best_plan_tree->root_node),
                                                query_info,var_candidates_cache);

    // todo: Destructor of PlanGenerator here
    long t5 = Util::get_cur_time();
    cout << "execution, used " << (t5 - t_) << "ms." << endl;

    auto dfs_table = get<1>(dfs_result);
    auto pos_var_mapping = dfs_table.pos_id_map;
    auto var_pos_mapping = dfs_table.id_pos_map;

    long t6 = Util::get_cur_time();
    CopyToResult(bgp_query, dfs_table);
#ifdef OPTIMIZER_DEBUG_INFO
    cout<<"after copy bfs result size "<<bgp_query->get_result_list_pointer()->size()<<endl;
#endif
    long t7 = Util::get_cur_time();
    cout << "copy to result, used " << (t7 - t6) <<"ms." <<endl;
    cout << "total execution, used " << (t7 - t1) <<"ms."<<endl;
  }
  else if(strategy ==BasicQueryStrategy::Special){
    // if(bgp_query->get_triple_num() == 1 && bgp_query->get_total_var_num() == 3){
    // todo: get all triples in database
    //;

    printf("BasicQueryStrategy::Special not supported yet\n");
  }
  return make_tuple<bool, bool>(true,std::move(ranked));

}

tuple<bool, shared_ptr<IntermediateResult>> Optimizer::MergeBasicQuery(SPARQLquery &sparql_query) {
  return tuple<bool, shared_ptr<IntermediateResult>>();
}

bool Optimizer::CopyToResult(shared_ptr<BGPQuery> bgp_query,
                             IntermediateResult result) {

  auto target = bgp_query->get_result_list_pointer();
  assert(target->empty());

#ifdef OPTIMIZER_DEBUG_INFO
  cout << "position to var des size: " << result.pos_id_map->size() << endl;
  cout << "total var_num: " << bgp_query->get_total_var_num()<<endl;
  cout << "selected var num: " << bgp_query->selected_so_var_num<<endl;
  cout<<"Before Copy, result size:"<<result.values_->size()<<endl;
  for(auto &pos_id:*result.pos_id_map)
    cout<<"pos["<<pos_id.first<<"] "<<pos_id.second<<"\t";
  cout<<endl;
#endif
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

  for (const auto&  record_ptr : *(result.values_))
  {
    auto new_record = new unsigned[record_len];
    for (int column_index = 0; column_index < record_len; ++column_index)
    {
      auto old_position = position_map[column_index];
      new_record[column_index] = (*record_ptr)[old_position];
    }
    target->push_back(new_record);
  }

  delete[] position_map;
  return false;
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
  auto operation_type = step_operation->join_type_;
  auto remain_old = step_operation->remain_old_result_;
  // leaf node : create a table
  if (plan_tree_node->left_node == nullptr && plan_tree_node->right_node == nullptr)
  {
    auto position_id_map = make_shared<PositionValue>();
    IntermediateResult leaf_table;
    if(operation_type== StepOperation::JoinType::JoinNode){
      auto r = this->PrepareInitial(bgp_query,step_operation->join_node_);
      bool is_entity= get<0>(r);
      bool is_literal = get<1>(r);
      bool is_predicate = get<2>(r);
#ifdef OPTIMIZER_DEBUG_INFO
      long t1 = Util::get_cur_time();
#endif
      auto initial_result = executor_.InitialTableOneNode(step_operation->join_node_,is_entity,is_literal,is_predicate,
                                                          step_operation->distinct_,id_caches);
      leaf_table = get<1>(initial_result);
#ifdef OPTIMIZER_DEBUG_INFO
      // cout<<"join node ["<<bgp_query->get_var_name_by_id(step_operation->join_node_->node_to_join_)<<"]" << endl;
	  cout<< "\tresult size:"<<leaf_table.values_->size();
      long t2 = Util::get_cur_time();
      cout<< ",  used " << (t2 - t1) << "ms." <<endl;
#endif
    }
    else if(operation_type== StepOperation::JoinType::GenerateCandidates)
    {
      executor_.UpdateIDCache(step_operation,id_caches);
    }
    else if(operation_type==StepOperation::JoinType::JoinTable)
      throw string("StepOperation::JoinType::JoinTable cannot happened in leaf node");
    else if(operation_type==StepOperation::JoinType::EdgeCheck)
      throw string("StepOperation::JoinType::EdgeCheck cannot happened in leaf node");
    else if(operation_type==StepOperation::JoinType::JoinTwoNodes){
#ifdef OPTIMIZER_DEBUG_INFO
      long t1 = Util::get_cur_time();
#endif
      auto initial_result = executor_.InitialTableTwoNode(step_operation->join_two_node_,id_caches);
      leaf_table = get<1>(initial_result);
#ifdef OPTIMIZER_DEBUG_INFO
	  auto node1 = step_operation->join_two_node_->node_to_join_1_;
	  auto node2 = step_operation->join_two_node_->node_to_join_2_;
	  cout<<"join two nodes ["<<bgp_query->get_var_name_by_id(node1)<<"]"<<",  [";
	  cout<<bgp_query->get_var_name_by_id(node2)<<"]"<<endl;
      cout<<"\tresult size:"<<leaf_table.values_->size();
      long t2 = Util::get_cur_time();
      cout<< ",  used " << (t2 - t1) << "ms." <<endl;
#endif
    }
    else if(operation_type==StepOperation::JoinType::GetAllTriples)
    {
#ifdef OPTIMIZER_DEBUG_INFO
      long t1 = Util::get_cur_time();
#endif
      auto all_triple_result = executor_.GetAllTriple(step_operation->join_node_);
      leaf_table = get<1>(all_triple_result);
#ifdef OPTIMIZER_DEBUG_INFO
      cout<<"GetAllTriple result size:"<<leaf_table.values_->size();
      long t2 = Util::get_cur_time();
      cout<< ",  used " << (t2 - t1) << "ms." <<endl;
#endif
    }
    else
      throw string("unexpected JoinType");

    return make_tuple(true,leaf_table);
  }


  //inner node
  tuple<bool,IntermediateResult> left_r;
  tuple<bool,IntermediateResult> right_r;

  if(plan_tree_node->left_node != nullptr)
    left_r = this->ExecutionBreathFirst(bgp_query,query_info,plan_tree_node->left_node,id_caches);

  if(plan_tree_node->right_node != nullptr)
    right_r = this->ExecutionBreathFirst(bgp_query,query_info,plan_tree_node->right_node,id_caches);

  if(operation_type == StepOperation::JoinType::GenerateCandidates){
#ifdef OPTIMIZER_DEBUG_INFO
    long t1 = Util::get_cur_time();
#endif
    executor_.UpdateIDCache(step_operation,id_caches);
#ifdef OPTIMIZER_DEBUG_INFO
    long t2 = Util::get_cur_time();
    cout<< "UpdateIDCache ,  used " << (t2 - t1) << "ms." <<endl;
#endif
    return left_r;
  }

  auto left_table = get<1>(left_r);
  // reading the left child result
  auto left_records = left_table.values_;

  // JoinNode/EdgeCheck/JoinTwoNodes has only ONE left child
  if(operation_type== StepOperation::JoinType::JoinNode) {
    // create a new table
    if(left_table.GetColumns()==0){
      auto position_id_map = make_shared<PositionValue>();
      auto r = this->PrepareInitial(bgp_query,step_operation->join_node_);
      bool is_entity= get<0>(r);
      bool is_literal = get<1>(r);
      bool is_predicate = get<2>(r);
#ifdef OPTIMIZER_DEBUG_INFO
      long t1 = Util::get_cur_time();
#endif
      auto initial_result = executor_.InitialTableOneNode(step_operation->join_node_,is_entity,is_literal,is_predicate,
                                                          step_operation->distinct_,id_caches);
      auto leaf_table = get<1>(initial_result);
#ifdef OPTIMIZER_DEBUG_INFO
      cout<<"InitialTableOneNode result size:"<<leaf_table.values_->size();
      long t2 = Util::get_cur_time();
      cout<< ",  used " << (t2 - t1) << "ms." <<endl;
#endif
      return make_tuple(true,leaf_table);
    }

    auto one_step_join = plan_tree_node->node;
    auto node_to_join = one_step_join->join_node_->node_to_join_;
    cout<<"join node ["<<bgp_query->get_var_name_by_id(node_to_join)<<"]"<<endl;
    auto join_node = one_step_join->join_node_;
    auto edges = *join_node->edges_;
    auto edge_c = *join_node->edges_constant_info_;
#ifdef TABLE_OPERATOR_DEBUG_INFO
    for(int i=0;i<edges.size();i++)
      {
        std::cout<<"edge["<<i<<"] "<<edges[i].toString()<<std::endl;
        std::cout<<"constant["<<i<<"] "<<edge_c[i].toString()<<std::endl;
      }
#endif
#ifdef OPTIMIZER_DEBUG_INFO
    long t1 = Util::get_cur_time();
#endif
    auto step_result = executor_.JoinANode(left_table,id_caches,one_step_join->distinct_,remain_old,one_step_join->join_node_);

#ifdef OPTIMIZER_DEBUG_INFO
    cout<<"\tresult size:"<<get<1>(step_result).values_->size();
    long t2 = Util::get_cur_time();
    cout<< ",  used " << (t2 - t1) << "ms." <<endl;
#endif
    return step_result;
  }
  else if(operation_type==StepOperation::JoinType::EdgeCheck){

    auto edge_filter = step_operation->edge_filter_;
#ifdef OPTIMIZER_DEBUG_INFO
    long t1 = Util::get_cur_time();
#endif
    auto step_result =executor_.ANodeEdgesConstraintFilter(edge_filter, left_table,remain_old);
#ifdef OPTIMIZER_DEBUG_INFO
    cout<<"result size "<<get<1>(step_result).values_->size();
    long t2 = Util::get_cur_time();
    cout<< ",  used " << (t2 - t1) << "ms." <<endl;
#endif
    return step_result;
  }
  else if(operation_type==StepOperation::JoinType::JoinTwoNodes){
    if(left_table.GetColumns()==0){
      auto position_id_map = make_shared<PositionValue>();
      auto initial_result = executor_.InitialTableTwoNode(step_operation->join_two_node_,id_caches);
#ifdef OPTIMIZER_DEBUG_INFO
      cout<<"JoinTwoNodes result size:"<<get<1>(initial_result).values_->size()<<endl;
#endif
      return initial_result;
    }
    auto one_step_join = plan_tree_node->node;
    auto join_two_plan = one_step_join->join_two_node_;
#ifdef OPTIMIZER_DEBUG_INFO
    auto node1 = join_two_plan->node_to_join_1_;
    auto node2 = join_two_plan->node_to_join_2_;
    cout<<"join two nodes ["<<bgp_query->get_var_name_by_id(node1)<<"]"<<",  [";
    cout<<bgp_query->get_var_name_by_id(node2)<<"]"<<endl;

    long t1 = Util::get_cur_time();
#endif
    auto step_result = executor_.JoinTwoNode(join_two_plan, left_table, id_caches,remain_old);
#ifdef OPTIMIZER_DEBUG_INFO
    long t2 = Util::get_cur_time();
    cout<<"\tresult size:"<<get<1>(step_result).values_->size();
	cout<< ",  used " << (t2 - t1) << "ms." <<endl;
#endif
    return step_result;
  }

  auto right_table = get<1>(right_r);
  if(operation_type==StepOperation::JoinType::JoinTable){
#ifndef OPTIMIZER_DEBUG_INFO
    return executor_.JoinTable(step_operation->join_table_, left_table, right_table);
#endif
#ifdef OPTIMIZER_DEBUG_INFO
    long t1 = Util::get_cur_time();
#endif
    auto join_result =  executor_.JoinTable(step_operation->join_table_, left_table, right_table,remain_old);
#ifdef OPTIMIZER_DEBUG_INFO
    cout<<"\tresult size:"<<get<1>(join_result).values_->size();
    long t2 = Util::get_cur_time();
    cout<< ",  used " << (t2 - t1) << "ms." <<endl;
#endif
    return std::move(join_result);
  }
  else
    throw string("unexpected JoinType");
  return make_tuple(false,left_table);
}

/**
 * calculator which role a variable node maybe
 * @param bgp_query
 * @param join_a_node_plan
 * @return is_entity,is_literal,is_predicate
 */
tuple<bool,bool,bool>
Optimizer::PrepareInitial(shared_ptr<BGPQuery> bgp_query,
                          shared_ptr<FeedOneNode> join_a_node_plan) const {
  auto target_var_id = join_a_node_plan->node_to_join_;
  cout << "leaf node [" << bgp_query->get_var_name_by_id(target_var_id) << "],  ";
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
  auto result_list = make_shared<list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>();

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

    auto record = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>();
    record->reserve(var_num);
    root_fr->GetResult(i-1,record);

    bool success = true;
    if(tree_search_plan->HasCycle())
    {
      auto &non_tree_edge = tree_search_plan->GetNonTreeEdges();
      auto temp_content_ptr = make_shared<TableContent>();
      temp_content_ptr->push_back(record);
      temp_table.values_ = temp_content_ptr;
      auto filter_result = this->executor_.ANodeEdgesConstraintFilter(non_tree_edge.edge_filter_,temp_table,false);
      auto filter_result_size = get<1>(filter_result).values_->size();
      success = filter_result_size != 0;
    }
    if(query_info.is_distinct_)
    {
      vector<TYPE_ENTITY_LITERAL_ID> select_id;
      for_each(select_position.begin(),select_position.end(),[&select_id,&record](unsigned int pos){
        select_id.push_back(record->at(pos));
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
    auto rec = *it;
    cout<<" record["<<i<<"]"<<" score:"<<score_list[i];

    for(unsigned j =0;j<var_num;j++)
      if(is_predicate_var[(*pos_var_mapping)[j]])
        cout<<" "<<kv_store_->getPredicateByID((*rec)[j])<<"["<<(*rec)[j]<<"]";
      else
        cout<<" "<<kv_store_->getStringByID((*rec)[j])<<"["<<(*rec)[j]<<"]";;
    cout<<endl;
    it++;
  }
#endif
  delete root_fr;
  delete env;
  return std::make_tuple(true,result_table);
}

#endif