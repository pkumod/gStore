/*=============================================================================
# Filename: PlanTree.h
# Author: Linglin Yang
# Mail: fyulingi@gmail.com
=============================================================================*/

#ifndef GSTORE_PLANTREE_H
#define GSTORE_PLANTREE_H

#include "./TableOperator.h"


struct Tree_node{
    shared_ptr<StepOperation> node;
    Tree_node *left_node;
    Tree_node *right_node;

    Tree_node(){
		node = nullptr;
    	left_node = nullptr;
    	right_node = nullptr;
    };

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
    std::vector<unsigned> already_joined_pre_var;
    std::vector<unsigned> already_so_var;
    long long plan_cost;

 public:
    PlanTree()= default;
	~PlanTree(){
		delete root_node;
	}

    JoinMethod get_join_strategy(BGPQuery *bgp_query, shared_ptr<VarDescriptor> var_descrip, unsigned edge_index, bool join_two_node = true );

	// Var Scan Operation
    PlanTree(unsigned first_node, BGPQuery *bgpquery);

	// WCO Join One Node
    PlanTree(PlanTree *last_plantree, BGPQuery *bgpquery, unsigned next_node, bool used_in_heuristic_plan = false);

	// Functions for p2so optimization
	void add_prevar_neicon(unsigned node_id, BGPQuery *bgpquery, bool is_first_node);
    PlanTree(unsigned node_1_id, unsigned node_2_id, BGPQuery *bgpquery);

	// Binary Join
    PlanTree(PlanTree *left_plan, PlanTree *right_plan, BGPQuery *bgpquery, set<unsigned> &join_nodes);

	// Function for satellite node
    void AddSatelliteNode(BGPQuery* bgpquery, unsigned satellitenode_id);

	// Plan for only one triple query
    PlanTree(shared_ptr<StepOperation> &first_node);

	// Functions for print
    void print_tree_node(Tree_node* node, BGPQuery* bgpquery);
    void print(BGPQuery* bgpquery);
};

#endif //GSTORE_PLANTREE_H
