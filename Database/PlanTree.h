/*=============================================================================
# Filename: PlanTree.h
# Author: Linglin Yang
# Mail: fyulingi@gmail.com
=============================================================================*/

#ifndef GSTORE_PLANTREE_H
#define GSTORE_PLANTREE_H

#include <vector>
#include <string>
#include <iostream>
#include "../Query/BasicQuery.h"
#include "../Query/BGPQuery.h"
#include "./TableOperator.h"
#include "../Util/Util.h"

using namespace std;


// todo: just use node, *left_node, *right_node
struct Tree_node{
    shared_ptr<StepOperation> node;


    Tree_node *left_node;
    Tree_node *right_node;

    Tree_node(){
    	left_node = nullptr;
    	right_node = nullptr;
    };


    // below will be used in gstore v1.0



    // not used
    JoinMethod get_join_method(bool s_is_var, bool p_is_var, bool o_is_var);

    // new_fun_for_new_version is a redundant param,
    // just distinguish it from Tree_node(Tree_node *node_need_copy)
    Tree_node(Tree_node *node_need_copy, bool new_fun_for_new_version){
    	node = node_need_copy->node;

    	if(node_need_copy->left_node)
    		left_node = new Tree_node(node_need_copy->left_node, true);
		else
			left_node = nullptr;

		if(node_need_copy->right_node)
			right_node = new Tree_node(node_need_copy->right_node, true);
		else
			right_node = nullptr;

    }
	Tree_node(Tree_node *node_need_copy){
		node = node_need_copy->node;

		if(node_need_copy->left_node)
			left_node = new Tree_node(node_need_copy->left_node);
		else
			left_node = nullptr;

		if(node_need_copy->right_node)
			right_node = new Tree_node(node_need_copy->right_node);
		else
			right_node = nullptr;

	}

    Tree_node(unsigned node_id, BGPQuery *bgpquery, bool is_first_node);

    Tree_node(unsigned node_id, set<unsigned> already_in, BGPQuery *bgpquery);

    Tree_node(shared_ptr<StepOperation> next_node){
    	node = next_node;
    	left_node = nullptr;
    	right_node = nullptr;
    }

	~Tree_node(){
		if(left_node){
			delete left_node;
			left_node = nullptr;
		}
		if(right_node){
			delete right_node;
			right_node = nullptr;
		}
	}


};

class PlanTree {

public:
    Tree_node *root_node = nullptr;
    // todo: this two vec need build
    vector<unsigned> already_joined_pre_var;
    vector<unsigned> already_so_var;
    long long plan_cost;

    PlanTree()= default;
	~PlanTree(){
		delete root_node;
	}


    JoinMethod get_join_strategy(BGPQuery *bgp_query, shared_ptr<VarDescriptor> var_descrip, unsigned edge_index, bool join_two_node = true );

    PlanTree(unsigned first_node, BGPQuery *bgpquery, vector<unsigned> satellite_index,
			 shared_ptr<vector<EdgeInfo>> edge_info, shared_ptr<vector<EdgeConstantInfo>> edge_constant_info,
			 vector<unsigned> &nei_id_vec);
    PlanTree(PlanTree *last_plantree, BGPQuery *bgpquery, unsigned next_node);
    void add_prevar_neicon(unsigned node_id, BGPQuery *bgpquery, bool is_first_node);
    PlanTree(unsigned node_1_id, unsigned node_2_id, BGPQuery *bgpquery);
    PlanTree(PlanTree *left_plan, PlanTree *right_plan, BGPQuery *bgpquery, set<unsigned> &join_nodes);
    PlanTree(PlanTree *last_plan_tree, unsigned next_join_var_id, set<unsigned> already_id, BGPQuery *bgpquery);

    void add_satellitenode(BGPQuery* bgpquery, unsigned satellitenode_id);
    PlanTree(shared_ptr<StepOperation> &first_node);

    PlanTree(unsigned first_node, BGPQuery *bgpquery, bool used_in_random_plan);

    void delete_tree_node(Tree_node* root_node);

    void print_tree_node(Tree_node* node, BGPQuery* bgpquery);
    void print(BGPQuery* bgpquery);
};


#endif //GSTORE_PLANTREE_H
