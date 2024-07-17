/*=============================================================================
# Filename: PlanTree.cpp
# Author: Linglin Yang
# Mail: fyulingi@gmail.com
=============================================================================*/

#include "PlanTree.h"

using namespace std;


JoinMethod PlanTree::get_join_strategy(BGPQuery *bgp_query, shared_ptr<VarDescriptor> var_descrip, unsigned int edge_index, bool join_two_node) {
	if(join_two_node){
			return var_descrip->so_edge_type_[edge_index] == Util::EDGE_IN ? JoinMethod::s2po : JoinMethod::o2ps;
	} else{
		SLOG_ERROR("not support");
		exit(-1);
	}
}

// Let the first_node be ?s
// ?s p o. Not consider, already considered in constant filter.
// ?s p ?o. Not consider. 1. ?o join node, join in next steps; 2. ?o satellite node, join in final steps.
// ?s ?p o. Consider.
// ?s ?p ?o. Not consider.
// Todo: ?p join order
PlanTree::PlanTree(unsigned first_node, BGPQuery *bgpquery) {
	root_node = nullptr;

	auto var_descrip = bgpquery->get_vardescrip_by_id(first_node);
	vector<unsigned> need_join_two_nodes_edge_index;
	auto edge_info = make_shared<vector<EdgeInfo>>();
	auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();

	for (unsigned i_th_edge = 0; i_th_edge < var_descrip->degree_; ++ i_th_edge){
		if (var_descrip->so_edge_pre_type_[i_th_edge] == VarDescriptor::PreType::VarPreType &&
			var_descrip->so_edge_nei_type_[i_th_edge] == VarDescriptor::EntiType::ConEntiType) {
			unsigned triple_id = var_descrip->so_edge_index_[i_th_edge];

			auto pre_var_descrip = bgpquery->get_vardescrip_by_id(var_descrip->so_edge_pre_id_[i_th_edge]);

			JoinMethod join_method;
			if (!pre_var_descrip->selected_ && pre_var_descrip->degree_ == 1) {
				if (var_descrip->so_edge_type_[i_th_edge] == Util::EDGE_IN)
					join_method = JoinMethod::s2o;
				else
					join_method = JoinMethod::o2s;

				edge_info->emplace_back(bgpquery->s_id_[triple_id], bgpquery->p_id_[triple_id], bgpquery->o_id_[triple_id], join_method);
				edge_constant_info->emplace_back(bgpquery->s_is_constant_[triple_id], bgpquery->p_is_constant_[triple_id], bgpquery->o_is_constant_[triple_id]);
			} else{
				need_join_two_nodes_edge_index.push_back(i_th_edge);
			}
		}
	}

	if(!edge_info->empty()){
		auto join_node = make_shared<AffectOneNode>(first_node, edge_info, edge_constant_info);
		root_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode, join_node, nullptr, nullptr, bgpquery->distinct_query));
	}

	if(!need_join_two_nodes_edge_index.empty()){
		unsigned beg = 0;

		if(edge_info->empty()){
			unsigned edge_index = var_descrip->so_edge_index_[need_join_two_nodes_edge_index[0]];
			shared_ptr<AffectTwoNode> join_two_nodes = make_shared<AffectTwoNode>(var_descrip->so_edge_pre_id_[need_join_two_nodes_edge_index[0]], first_node,
                                                                                  EdgeInfo(bgpquery->s_id_[edge_index], bgpquery->p_id_[edge_index], bgpquery->o_id_[edge_index],
																					   get_join_strategy(bgpquery, var_descrip, need_join_two_nodes_edge_index[0])),
                                                                                  EdgeConstantInfo(bgpquery->s_is_constant_[edge_index], bgpquery->p_is_constant_[edge_index], bgpquery->o_is_constant_[edge_index]));
			Tree_node *new_tree_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::TwoNode,
																				nullptr, join_two_nodes, nullptr, bgpquery->distinct_query));
			new_tree_node->left_node = root_node;
			root_node = new_tree_node;
			already_joined_pre_var.emplace_back(bgpquery->p_id_[edge_index]);
			beg = 1;

		}

		for(unsigned i = beg; i < need_join_two_nodes_edge_index.size(); ++i){
			unsigned edge_index = var_descrip->so_edge_index_[need_join_two_nodes_edge_index[i]];
			if(find(already_joined_pre_var.begin(), already_joined_pre_var.end(), bgpquery->p_id_[edge_index]) != already_joined_pre_var.end()){
				continue;
			} else{
				// selected_ == true or degree >= 2
				auto pre_var_descrip = bgpquery->get_vardescrip_by_id(var_descrip->so_edge_pre_id_[need_join_two_nodes_edge_index[i]]);
				shared_ptr<vector<EdgeInfo>> edges_info = make_shared<vector<EdgeInfo>>();
				shared_ptr<vector<EdgeConstantInfo>> edges_const = make_shared<vector<EdgeConstantInfo>>();

				for(unsigned j = i; j < need_join_two_nodes_edge_index.size(); ++j){
					// need_join_two_nodes_index include only edges with pre_var
					if(var_descrip->so_edge_pre_id_[need_join_two_nodes_edge_index[j]] != pre_var_descrip->id_)
						continue;
					edge_index = var_descrip->so_edge_index_[need_join_two_nodes_edge_index[j]];
					edges_info->emplace_back(bgpquery->s_id_[edge_index], bgpquery->p_id_[edge_index], bgpquery->o_id_[edge_index], JoinMethod::so2p);
					edges_const->emplace_back(bgpquery->s_is_constant_[edge_index], bgpquery->p_is_constant_[edge_index], bgpquery->o_is_constant_[edge_index]);
				}
				shared_ptr<AffectOneNode> join_pre_node = make_shared<AffectOneNode>(pre_var_descrip->id_, edges_info, edges_const);
				Tree_node *new_join_a_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode,
																					  join_pre_node, nullptr, nullptr, bgpquery->distinct_query));

				new_join_a_node->left_node = root_node;
				root_node = new_join_a_node;
				already_joined_pre_var.emplace_back(pre_var_descrip->id_);
			}
		}
	}

	if(edge_info->empty() and need_join_two_nodes_edge_index.empty()){
		auto join_node = make_shared<AffectOneNode>(first_node, make_shared<vector<EdgeInfo>>(), make_shared<vector<EdgeConstantInfo>>());
		root_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode, join_node, nullptr, nullptr, bgpquery->distinct_query));

	}
	already_so_var = {first_node};
}

// We want to join ?o.
// case1 s p ?o. This is done in candidate generation.
// case2 s ?p ?o. ?p is degree_one, s2o or so2p or s2po.then s2o, else s2po(?p not alrealy) or s2po(?p already).
// case3 ?s p ?o. this should be done first by sp2o.
// case4 ?s ?p ?o. ?p is degree_one pre_var(only exists in this triple), then we only need s2o on s.
// 	      If ?p is not degree one, then join two_node(?p not already) by s2po, or join one_node(?p already) by sp2o.
// First sp2o, then join other pre_var by join_a_node
PlanTree::PlanTree(PlanTree *last_plantree, BGPQuery *bgpquery, unsigned next_node, bool used_in_heuristic_plan) {
	auto var_descrip = bgpquery->get_vardescrip_by_id(next_node);
	if (used_in_heuristic_plan)
		root_node = last_plantree->root_node;
	else
		root_node = new Tree_node(last_plantree->root_node);

	already_so_var = last_plantree->already_so_var;
	already_joined_pre_var = last_plantree->already_joined_pre_var;

	auto join_a_node_edge_info = make_shared<vector<EdgeInfo>>();
	auto join_a_node_edge_const_info = make_shared<vector<EdgeConstantInfo>>();
	vector<unsigned> join_pre_var_index_vec;
	vector<unsigned> need_join_two_nodes_index;

	for(unsigned i = 0; i < var_descrip->degree_; ++i){

		// ?s (?)p ?o, join ?o, however, ?s is not already, then donnot need deal with this triple.
		// This triple will be dealed when join ?s next time.
		// After this if, we can believe (?)s is ready for join.
		if(var_descrip->so_edge_nei_type_[i] == VarDescriptor::EntiType::VarEntiType and
				find(already_so_var.begin(), already_so_var.end(), var_descrip->so_edge_nei_[i])
					== already_so_var.end())
			continue;

		// s p ?o. This edge will be done in candidate generation
		if(var_descrip->so_edge_nei_type_[i] == VarDescriptor::EntiType::ConEntiType and
				var_descrip->so_edge_pre_type_[i] == VarDescriptor::PreType::ConPreType)
			continue;

		if(var_descrip->so_edge_pre_type_[i] == VarDescriptor::PreType::ConPreType or
				find(already_joined_pre_var.begin(), already_joined_pre_var.end(), var_descrip->so_edge_pre_id_[i])
					!= already_joined_pre_var.end()){
			// s ?p ?o, ?p ready.
			// ?s p ?o, ?s ready.
			// ?s ?p ?o, ?s ready, ?p ready.
			unsigned edge_index = var_descrip->so_edge_index_[i];
			join_a_node_edge_info->emplace_back(bgpquery->s_id_[edge_index], bgpquery->p_id_[edge_index], bgpquery->o_id_[edge_index],
												(var_descrip->so_edge_type_[i] == Util::EDGE_IN ? JoinMethod::sp2o : JoinMethod::po2s));
			join_a_node_edge_const_info->emplace_back(bgpquery->s_is_constant_[edge_index], bgpquery->p_is_constant_[edge_index], bgpquery->o_is_constant_[edge_index]);
		} else{
			// s ?p ?o, ?p not ready.
			// Or ?s ?p ?o, ?s ready, ?p not ready.

			// if ?p degree_one, not select, only need s2o, consider distinct or not
			auto pre_descrip = bgpquery->get_vardescrip_by_id(var_descrip->so_edge_pre_id_[i]);
			if(pre_descrip->degree_ == 1 and !pre_descrip->selected_){
				join_pre_var_index_vec.emplace_back(i);
			} else{
				need_join_two_nodes_index.emplace_back(i);
			}
		}
	}

	if(!join_a_node_edge_info->empty()){
		shared_ptr<AffectOneNode> join_a_node = make_shared<AffectOneNode>(next_node, join_a_node_edge_info, join_a_node_edge_const_info);
		Tree_node *new_tree_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode,
																			join_a_node, nullptr, nullptr, bgpquery->distinct_query));
		new_tree_node->left_node = root_node;
		root_node = new_tree_node;
	}

	if(!need_join_two_nodes_index.empty()){

		unsigned beg = 0;

		if(join_a_node_edge_info->empty()){
			unsigned edge_index = var_descrip->so_edge_index_[need_join_two_nodes_index[0]];
			shared_ptr<AffectTwoNode> join_two_nodes = make_shared<AffectTwoNode>(var_descrip->so_edge_pre_id_[need_join_two_nodes_index[0]], next_node,
                                                                                  EdgeInfo(bgpquery->s_id_[edge_index], bgpquery->p_id_[edge_index], bgpquery->o_id_[edge_index],
																					   get_join_strategy(bgpquery, var_descrip, need_join_two_nodes_index[0])),
                                                                                  EdgeConstantInfo(bgpquery->s_is_constant_[edge_index], bgpquery->p_is_constant_[edge_index], bgpquery->o_is_constant_[edge_index]));
			Tree_node *new_tree_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::TwoNode,
																				nullptr, join_two_nodes, nullptr, bgpquery->distinct_query));
			new_tree_node->left_node = root_node;
			root_node = new_tree_node;
			already_joined_pre_var.emplace_back(bgpquery->p_id_[edge_index]);
			beg = 1;
		}

		for(unsigned i = beg; i < need_join_two_nodes_index.size(); ++i){
			unsigned edge_index = var_descrip->so_edge_index_[need_join_two_nodes_index[i]];
			if(find(already_joined_pre_var.begin(), already_joined_pre_var.end(), bgpquery->p_id_[edge_index]) != already_joined_pre_var.end()){
				continue;
			} else{
				// selected_ == true or degree >= 2
				auto pre_var_descrip = bgpquery->get_vardescrip_by_id(var_descrip->so_edge_pre_id_[need_join_two_nodes_index[i]]);
				shared_ptr<vector<EdgeInfo>> edges_info = make_shared<vector<EdgeInfo>>();
				shared_ptr<vector<EdgeConstantInfo>> edges_const = make_shared<vector<EdgeConstantInfo>>();

				for(unsigned j = i; j < need_join_two_nodes_index.size(); ++j){
					// need_join_two_nodes_index include only edges with pre_var
					if(var_descrip->so_edge_pre_id_[need_join_two_nodes_index[j]] != pre_var_descrip->id_)
						continue;
					edge_index = var_descrip->so_edge_index_[need_join_two_nodes_index[j]];
					edges_info->emplace_back(bgpquery->s_id_[edge_index], bgpquery->p_id_[edge_index], bgpquery->o_id_[edge_index], JoinMethod::so2p);
					edges_const->emplace_back(bgpquery->s_is_constant_[edge_index], bgpquery->p_is_constant_[edge_index], bgpquery->o_is_constant_[edge_index]);
				}
				shared_ptr<AffectOneNode> join_pre_node = make_shared<AffectOneNode>(pre_var_descrip->id_, edges_info, edges_const);
				Tree_node *new_join_a_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode,
																					  join_pre_node, nullptr, nullptr, bgpquery->distinct_query));

				new_join_a_node->left_node = root_node;
				root_node = new_join_a_node;
				already_joined_pre_var.emplace_back(pre_var_descrip->id_);
			}
		}
	}

	already_so_var.emplace_back(next_node);

	if(!join_pre_var_index_vec.empty()){
		unsigned beg = 0;
		if(join_a_node_edge_info->empty() && need_join_two_nodes_index.empty()){
			unsigned index = var_descrip->so_edge_index_[join_pre_var_index_vec[0]];
			join_a_node_edge_info->emplace_back(bgpquery->s_id_[index], bgpquery->p_id_[index], bgpquery->o_id_[index],
											   (var_descrip->so_edge_type_[join_pre_var_index_vec[0]] == Util::EDGE_IN ? JoinMethod::s2o : JoinMethod::o2s));
			join_a_node_edge_const_info->emplace_back(bgpquery->s_is_constant_[index], bgpquery->p_is_constant_[index], bgpquery->o_is_constant_[index]);
			shared_ptr<AffectOneNode> join_a_node = make_shared<AffectOneNode>(next_node, join_a_node_edge_info, join_a_node_edge_const_info);
			Tree_node *new_tree_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode,
																				join_a_node, nullptr, nullptr, bgpquery->distinct_query));
			new_tree_node->left_node = root_node;
			root_node = new_tree_node;
			++beg;
		}
		for(unsigned i = beg; i < join_pre_var_index_vec.size(); ++i){
			auto join_satellite_pre_edge_info = make_shared<vector<EdgeInfo>>();
			auto join_satellite_pre_edge_const_info = make_shared<vector<EdgeConstantInfo>>();
			unsigned edge_index = var_descrip->so_edge_index_[join_pre_var_index_vec[i]];
			join_satellite_pre_edge_info->emplace_back(bgpquery->s_id_[edge_index], bgpquery->p_id_[edge_index], bgpquery->o_id_[edge_index], JoinMethod::so2p);
			join_satellite_pre_edge_const_info->emplace_back(bgpquery->s_is_constant_[edge_index], bgpquery->p_is_constant_[edge_index], bgpquery->o_is_constant_[edge_index]);
			shared_ptr<AffectOneNode> join_satellite = make_shared<AffectOneNode>(bgpquery->p_id_[edge_index], join_satellite_pre_edge_info, join_satellite_pre_edge_const_info);
			Tree_node *new_tree_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Satellite, StepOperation::OpRangeType::OneNode,
																				join_satellite, nullptr, nullptr, bgpquery->distinct_query));
			new_tree_node->left_node = root_node;
			root_node = new_tree_node;
		}
	}
}

// only used when consider_p2so_optimization
void PlanTree::add_prevar_neicon(unsigned node_id, BGPQuery *bgpquery, bool is_first_node){
	auto var_descrip = bgpquery->get_vardescrip_by_id(node_id);
	auto last_var_descrip = bgpquery->get_vardescrip_by_id(already_so_var[0]);
	vector<unsigned> need_join_pre_var_edge_index;
	vector<unsigned> need_check_pre_var_edge_index;
	vector<unsigned> already_done_pre_var;

	for (unsigned i_th_edge = 0; i_th_edge < var_descrip->degree_; ++ i_th_edge) {
		if (var_descrip->so_edge_pre_type_[i_th_edge] == VarDescriptor::PreType::VarPreType and
				var_descrip->so_edge_nei_type_[i_th_edge] == VarDescriptor::EntiType::ConEntiType) {
			if (find(already_joined_pre_var.begin(), already_joined_pre_var.end(),
					var_descrip->so_edge_pre_id_[i_th_edge]) != already_joined_pre_var.end())
				need_check_pre_var_edge_index.emplace_back(i_th_edge);
			else
				need_join_pre_var_edge_index.emplace_back(i_th_edge);
		}
	}

	if (!need_check_pre_var_edge_index.empty()) {
		for (unsigned i = 0; i < need_check_pre_var_edge_index.size(); ++i) {
			unsigned edge_index = var_descrip->so_edge_index_[need_check_pre_var_edge_index[i]];
			auto edge_info = make_shared<vector<EdgeInfo>>();
			auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
			edge_info->emplace_back(bgpquery->s_id_[edge_index], bgpquery->p_id_[edge_index], bgpquery->o_id_[edge_index],
									var_descrip->so_edge_type_[need_check_pre_var_edge_index[i]] == Util::EDGE_IN ? JoinMethod::sp2o : JoinMethod::po2s);
			edge_constant_info->emplace_back(bgpquery->s_is_constant_[edge_index], bgpquery->p_is_constant_[edge_index], bgpquery->o_is_constant_[edge_index]);
			auto edge_check = make_shared<AffectOneNode>(node_id, edge_info, edge_constant_info);
			Tree_node* edge_check_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Check, StepOperation::OpRangeType::OneNode,
																				  edge_check, nullptr, nullptr,bgpquery->distinct_query));
			edge_check_node->left_node = root_node;
			root_node = edge_check_node;
		}
	}

	if (!need_join_pre_var_edge_index.empty()) {
		for (unsigned i = 0; i < need_join_pre_var_edge_index.size(); ++i) {
			unsigned edge_index = var_descrip->so_edge_index_[need_join_pre_var_edge_index[i]];
			if (find(already_joined_pre_var.begin(), already_joined_pre_var.end(), bgpquery->p_id_[edge_index]) != already_joined_pre_var.end()) {
				auto edge_info = make_shared<vector<EdgeInfo>>();
				auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
				edge_info->emplace_back(bgpquery->s_id_[edge_index], bgpquery->p_id_[edge_index], bgpquery->o_id_[edge_index],
										var_descrip->so_edge_type_[need_check_pre_var_edge_index[i]] == Util::EDGE_IN ? JoinMethod::sp2o : JoinMethod::po2s);
				edge_constant_info->emplace_back(bgpquery->s_is_constant_[edge_index], bgpquery->p_is_constant_[edge_index], bgpquery->o_is_constant_[edge_index]);
				auto edge_check = make_shared<AffectOneNode>(node_id, edge_info, edge_constant_info);
				Tree_node* edge_check_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Check, StepOperation::OpRangeType::OneNode,
																					  edge_check, nullptr, nullptr, bgpquery->distinct_query));
				edge_check_node->left_node = root_node;
				root_node = edge_check_node;
			} else {
				auto pre_var_descrip = bgpquery->get_vardescrip_by_id(var_descrip->so_edge_pre_id_[need_join_pre_var_edge_index[i]]);
				shared_ptr<vector<EdgeInfo>> edges_info = make_shared<vector<EdgeInfo>>();
				shared_ptr<vector<EdgeConstantInfo>> edges_const = make_shared<vector<EdgeConstantInfo>>();
				edges_info->emplace_back(bgpquery->s_id_[edge_index], bgpquery->p_id_[edge_index], bgpquery->o_id_[edge_index], JoinMethod::so2p);
				edges_const->emplace_back(bgpquery->s_is_constant_[edge_index], bgpquery->p_is_constant_[edge_index], bgpquery->o_is_constant_[edge_index]);
				shared_ptr<AffectOneNode> join_pre_node = make_shared<AffectOneNode>(pre_var_descrip->id_, edges_info, edges_const);
				Tree_node *new_join_a_node;
				if(!pre_var_descrip->selected_ and pre_var_descrip->degree_ == 1)
					new_join_a_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Satellite, StepOperation::OpRangeType::OneNode,
																					  join_pre_node, nullptr, nullptr, bgpquery->distinct_query));
				else
					new_join_a_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode,
																			   join_pre_node, nullptr, nullptr, bgpquery->distinct_query));

				new_join_a_node->left_node = root_node;
				root_node = new_join_a_node;
				if(pre_var_descrip->selected_ || pre_var_descrip->degree_ != 1)
					already_joined_pre_var.emplace_back(pre_var_descrip->id_);
			}
		}
	}
}

// ?s ?p ?o
// ?s p ?o
// s ?p ?o
PlanTree::PlanTree(unsigned node_1_id, unsigned node_2_id, BGPQuery *bgpquery){
	auto var1_descrip = bgpquery->get_vardescrip_by_id(node_1_id);
	auto var2_descrip = bgpquery->get_vardescrip_by_id(node_2_id);

	vector<unsigned> linked_edge_pre_const_vec;
	vector<unsigned> linked_edge_pre_var_vec;
	for(unsigned edge_index = 0; edge_index < var1_descrip->degree_; ++edge_index){
		if(var1_descrip->so_edge_nei_type_[edge_index] == VarDescriptor::EntiType::VarEntiType and
				var1_descrip->so_edge_nei_[edge_index] == node_2_id){
			if(var1_descrip->so_edge_pre_type_[edge_index] == VarDescriptor::PreType::VarPreType)
				linked_edge_pre_var_vec.push_back(edge_index);
			else
				linked_edge_pre_const_vec.push_back(edge_index);
		}
	}

	if(!linked_edge_pre_const_vec.empty()){
		unsigned edge_index = var1_descrip->so_edge_index_[linked_edge_pre_const_vec[0]];
		shared_ptr<AffectTwoNode> first_join_two_node = make_shared<AffectTwoNode>(bgpquery->s_id_[edge_index], bgpquery->o_id_[edge_index],
                                                                                   EdgeInfo(bgpquery->s_id_[edge_index], bgpquery->p_id_[edge_index], bgpquery->o_id_[edge_index],
																						JoinMethod::p2so),
                                                                                   EdgeConstantInfo(false, true, false));
		shared_ptr<StepOperation> first_node = make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::TwoNode,
																		  nullptr, first_join_two_node, nullptr, bgpquery->distinct_query);
		root_node = new Tree_node(first_node);

		if (linked_edge_pre_const_vec.size() > 1) {
			shared_ptr<vector<EdgeInfo>> edge_check_edge_vec = make_shared<vector<EdgeInfo>>();
			shared_ptr<vector<EdgeConstantInfo>> edge_check_edge_const_vec = make_shared<vector<EdgeConstantInfo>>();

			for (unsigned index = 1; index < linked_edge_pre_const_vec.size(); ++index) {
				unsigned edge_check_index = var1_descrip->so_edge_index_[linked_edge_pre_const_vec[index]];
				edge_check_edge_vec->emplace_back(bgpquery->s_id_[edge_check_index], bgpquery->p_id_[edge_check_index], bgpquery->o_id_[edge_check_index],
												  (bgpquery->o_id_[edge_check_index] == node_1_id ? JoinMethod::sp2o : JoinMethod::po2s));
				edge_check_edge_const_vec->emplace_back(false, true, false);
			}

			shared_ptr<StepOperation> edge_check_node = make_shared<StepOperation>(StepOperation::StepOpType::Check, StepOperation::OpRangeType::OneNode,
																				   make_shared<AffectOneNode>(node_1_id, edge_check_edge_vec, edge_check_edge_const_vec), nullptr, nullptr, bgpquery->distinct_query);
			Tree_node* new_edge_check = new Tree_node(edge_check_node);
			new_edge_check->left_node = root_node;
			root_node = new_edge_check;
		}

		for (unsigned i = 0; i < linked_edge_pre_var_vec.size(); ++i){
			unsigned join_a_node_edge_index = var1_descrip->so_edge_index_[linked_edge_pre_var_vec[i]];

			auto pre_var_descrip = bgpquery->get_vardescrip_by_id(bgpquery->p_id_[join_a_node_edge_index]);

			shared_ptr<vector<EdgeInfo>> edges_info = make_shared<vector<EdgeInfo>>();
			shared_ptr<vector<EdgeConstantInfo>> edges_const = make_shared<vector<EdgeConstantInfo>>();

			edges_info->emplace_back(bgpquery->s_id_[join_a_node_edge_index], bgpquery->p_id_[join_a_node_edge_index], bgpquery->o_id_[join_a_node_edge_index], JoinMethod::so2p);
			edges_const->emplace_back(bgpquery->s_is_constant_[join_a_node_edge_index], bgpquery->p_is_constant_[join_a_node_edge_index], bgpquery->o_is_constant_[join_a_node_edge_index]);

			shared_ptr<AffectOneNode> join_pre_node = make_shared<AffectOneNode>(pre_var_descrip->id_, edges_info, edges_const);
			Tree_node *new_join_a_node;
			if (pre_var_descrip->degree_ == 1 and !pre_var_descrip->selected_)
				new_join_a_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Satellite, StepOperation::OpRangeType::OneNode,
																				  join_pre_node, nullptr, nullptr, bgpquery->distinct_query));
			else
				new_join_a_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode,
																		   join_pre_node, nullptr, nullptr, bgpquery->distinct_query));

			new_join_a_node->left_node = root_node;
			root_node = new_join_a_node;
			if (pre_var_descrip->degree_ != 1 || pre_var_descrip->selected_)
				already_joined_pre_var.emplace_back(pre_var_descrip->id_);
		}
	} else {
		unsigned edge_index = var1_descrip->so_edge_index_[linked_edge_pre_var_vec[0]];

		shared_ptr<vector<EdgeInfo>> edges_info = make_shared<vector<EdgeInfo>>();
		shared_ptr<vector<EdgeConstantInfo>> edges_const = make_shared<vector<EdgeConstantInfo>>();
		// getalltriple
		edges_info->emplace_back(bgpquery->s_id_[edge_index], bgpquery->p_id_[edge_index], bgpquery->o_id_[edge_index], JoinMethod::so2p);
		edges_const->emplace_back(bgpquery->s_is_constant_[edge_index], bgpquery->p_is_constant_[edge_index], bgpquery->o_is_constant_[edge_index]);
		shared_ptr<AffectOneNode> get_all_triple_node = make_shared<AffectOneNode>(node_1_id, edges_info, edges_const);

		root_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::GetAllTriples,
															 get_all_triple_node, nullptr, nullptr, bgpquery->distinct_query));
		already_joined_pre_var.emplace_back(bgpquery->p_id_[edge_index]);

		for(unsigned i = 1; i < linked_edge_pre_var_vec.size(); ++i){
			unsigned join_a_node_edge_index = var1_descrip->so_edge_index_[linked_edge_pre_var_vec[i]];
			auto pre_var_descrip = bgpquery->get_vardescrip_by_id(bgpquery->p_id_[join_a_node_edge_index]);

			shared_ptr<vector<EdgeInfo>> edges_info = make_shared<vector<EdgeInfo>>();
			shared_ptr<vector<EdgeConstantInfo>> edges_const = make_shared<vector<EdgeConstantInfo>>();

			edges_info->emplace_back(bgpquery->s_id_[join_a_node_edge_index], bgpquery->p_id_[join_a_node_edge_index], bgpquery->o_id_[join_a_node_edge_index], JoinMethod::so2p);
			edges_const->emplace_back(bgpquery->s_is_constant_[join_a_node_edge_index], bgpquery->p_is_constant_[join_a_node_edge_index], bgpquery->o_is_constant_[join_a_node_edge_index]);

			shared_ptr<AffectOneNode> join_pre_node = make_shared<AffectOneNode>(pre_var_descrip->id_, edges_info, edges_const);

			Tree_node *new_join_a_node;
			if (pre_var_descrip->degree_ == 1 and !pre_var_descrip->selected_)
				new_join_a_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Satellite, StepOperation::OpRangeType::OneNode,
																		   join_pre_node, nullptr, nullptr, bgpquery->distinct_query));
			else
				new_join_a_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode,
																		   join_pre_node, nullptr, nullptr, bgpquery->distinct_query));

			new_join_a_node->left_node = root_node;
			root_node = new_join_a_node;
			if(pre_var_descrip->degree_ != 1 || pre_var_descrip->selected_)
				already_joined_pre_var.emplace_back(pre_var_descrip->id_);
		}
	}

	already_so_var.push_back(node_1_id);
	already_so_var.push_back(node_2_id);

	add_prevar_neicon(node_1_id, bgpquery, true);
	add_prevar_neicon(node_2_id, bgpquery, false);
}

// todo: check these three cases would not apprear
// After join left and right table, we need consider some edges between them:
// case1 ?s p ?o, ?s in left, ?o in right
// case2 ?s ?p ?o, join ?p(not appear in left and right) or edgecheck(?p ready)
// case3 ?s1 ?p ?o1 in left, ?s2 ?p ?o2 in right, this can be done in JoinTwoTable
// todo: 用Plangenerator的参数，直接算出哪些是不被join的
PlanTree::PlanTree(PlanTree *left_plan, PlanTree *right_plan, BGPQuery *bgpquery, set<unsigned> &join_nodes) {
	shared_ptr<vector<unsigned>> public_variables = make_shared<vector<unsigned>>(join_nodes.begin(), join_nodes.end());

	already_so_var = left_plan->already_so_var;
	for(auto x : right_plan->already_so_var)
		if(find(already_so_var.begin(), already_so_var.end(), x) == already_so_var.end())
			already_so_var.emplace_back(x);

	already_joined_pre_var = left_plan->already_joined_pre_var;
	for(auto x : right_plan->already_joined_pre_var)
		if(find(already_joined_pre_var.begin(), already_joined_pre_var.end(), x) == already_joined_pre_var.end())
			already_joined_pre_var.emplace_back(x);

	for(auto x : left_plan->already_joined_pre_var){
		if(find(right_plan->already_joined_pre_var.begin(), right_plan->already_joined_pre_var.end(), x) != right_plan->already_so_var.end()){
			public_variables->emplace_back(x);
		}
	}

	root_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::TableJoin, StepOperation::OpRangeType::TwoTable,
												nullptr, nullptr, make_shared<JoinTwoTable>(public_variables), bgpquery->distinct_query));

	root_node->left_node = new Tree_node(left_plan->root_node);
	root_node->right_node = new Tree_node(right_plan->root_node);
}

// used in special one triple plan
PlanTree::PlanTree(shared_ptr<StepOperation> &first_node) {
	root_node = new Tree_node(first_node);
}

// ?s p ?o joinanode sp2o, false
// ?s ?p ?o, ?p already, sp2o  false
// below: ?p not ready
// ?s ?p ?o, ?p degree1 notselected,  s2o false
// ?s ?p ?o, ?p degree1 selected or degree > 1, joinanode s2p
void PlanTree::AddSatelliteNode(BGPQuery *bgpquery, unsigned int satellite_node_id, bool save) {
	auto var_descrip = bgpquery->get_vardescrip_by_id(satellite_node_id);
	unsigned edge_index = var_descrip->so_edge_index_[0];

	if(var_descrip->so_edge_pre_type_[0] == VarDescriptor::PreType::ConPreType or
			find(already_joined_pre_var.begin(), already_joined_pre_var.end(), var_descrip->so_edge_pre_id_[0])
				!= already_joined_pre_var.end()){

		auto join_a_node_edge_info = make_shared<vector<EdgeInfo>>();
		auto join_a_node_edge_const_info = make_shared<vector<EdgeConstantInfo>>();

		join_a_node_edge_info->emplace_back(bgpquery->s_id_[edge_index], bgpquery->p_id_[edge_index], bgpquery->o_id_[edge_index],
											var_descrip->so_edge_type_[0] == Util::EDGE_IN ? JoinMethod::sp2o : JoinMethod::po2s);
		join_a_node_edge_const_info->emplace_back(bgpquery->s_is_constant_[edge_index], bgpquery->p_is_constant_[edge_index], bgpquery->o_is_constant_[edge_index]);

		auto join_node = make_shared<AffectOneNode>(satellite_node_id, join_a_node_edge_info, join_a_node_edge_const_info);
		Tree_node *new_join_node = nullptr;
        if (save)
            new_join_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode, join_node,
																			nullptr, nullptr, bgpquery->distinct_query));
        else
            new_join_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Satellite, StepOperation::OpRangeType::OneNode, join_node,
                                                                     nullptr, nullptr, bgpquery->distinct_query));
		new_join_node->left_node = root_node;
		root_node = new_join_node;
	} else{
		auto pre_var_descrip = bgpquery->get_vardescrip_by_id(var_descrip->so_edge_pre_id_[0]);
		if(!pre_var_descrip->selected_ and pre_var_descrip->degree_ == 1){
			auto join_a_node_edge_info = make_shared<vector<EdgeInfo>>();
			auto join_a_node_edge_const_info = make_shared<vector<EdgeConstantInfo>>();

			join_a_node_edge_info->emplace_back(bgpquery->s_id_[edge_index], bgpquery->p_id_[edge_index], bgpquery->o_id_[edge_index],
												var_descrip->so_edge_type_[0] == Util::EDGE_IN ? JoinMethod::s2o : JoinMethod::o2s);
			join_a_node_edge_const_info->emplace_back(bgpquery->s_is_constant_[edge_index], bgpquery->p_is_constant_[edge_index], bgpquery->o_is_constant_[edge_index]);

			auto join_node = make_shared<AffectOneNode>(satellite_node_id, join_a_node_edge_info, join_a_node_edge_const_info);
			Tree_node *new_join_node = nullptr;
            if (save)
                new_join_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode, join_node,
																				nullptr, nullptr, bgpquery->distinct_query));
            else
                new_join_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Satellite, StepOperation::OpRangeType::OneNode, join_node,
                                                                         nullptr, nullptr, bgpquery->distinct_query));
			new_join_node->left_node = root_node;
			root_node = new_join_node;
		} else{
            if (!save) {
                auto join_a_node_edge_info = make_shared<vector<EdgeInfo>>();
                auto join_a_node_edge_const_info = make_shared<vector<EdgeConstantInfo>>();

                join_a_node_edge_info->emplace_back(bgpquery->s_id_[edge_index], bgpquery->p_id_[edge_index],
                                                    bgpquery->o_id_[edge_index],
                                                    var_descrip->so_edge_type_[0] == Util::EDGE_IN ? JoinMethod::s2p
                                                                                                   : JoinMethod::o2p);
                join_a_node_edge_const_info->emplace_back(bgpquery->s_is_constant_[edge_index],
                                                          bgpquery->p_is_constant_[edge_index],
                                                          bgpquery->o_is_constant_[edge_index]);

                auto join_node = make_shared<AffectOneNode>(bgpquery->p_id_[edge_index], join_a_node_edge_info,
                                                            join_a_node_edge_const_info);
                Tree_node *new_join_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend,
                                                                                    StepOperation::OpRangeType::OneNode,
                                                                                    join_node, nullptr, nullptr,
                                                                                    bgpquery->distinct_query));

                new_join_node->left_node = root_node;
                root_node = new_join_node;
            } else {
                shared_ptr<AffectTwoNode> join_two_nodes = make_shared<AffectTwoNode>(bgpquery->p_id_[edge_index], satellite_node_id,
                                                                                      EdgeInfo(bgpquery->s_id_[edge_index], bgpquery->p_id_[edge_index], bgpquery->o_id_[edge_index],
                                                                                               get_join_strategy(bgpquery, var_descrip, 0)),
                                                                                      EdgeConstantInfo(bgpquery->s_is_constant_[edge_index], bgpquery->p_is_constant_[edge_index], bgpquery->o_is_constant_[edge_index]));
                Tree_node *new_join_node = new Tree_node(make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::TwoNode,
                                                                                    nullptr, join_two_nodes, nullptr, bgpquery->distinct_query));
                new_join_node->left_node = root_node;
                root_node = new_join_node;
            }
			already_joined_pre_var.emplace_back(pre_var_descrip->id_);
		}
	}
	already_so_var.emplace_back(satellite_node_id);
}

void PlanTree::print_tree_node(Tree_node *node, BGPQuery *bgpquery) {
	if(node == nullptr)
		return;
	if(node->left_node != nullptr)
		print_tree_node(node->left_node, bgpquery);
	if(node->right_node != nullptr)
		print_tree_node(node->right_node, bgpquery);

	std::string code_print;
	if(!node->left_node && !node->right_node) code_print = "leaf node ";

	SLOG_CORE(code_print << node->node->GetString());
}

void PlanTree::print(BGPQuery* bgpquery) {
	SLOG_CORE("Plan: " << (bgpquery->distinct_query ? "distinct" : "not distinct"));
	print_tree_node(root_node, bgpquery);
}