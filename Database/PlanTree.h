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

using namespace std;

// for yanglei's paper, will not used in gStore v1.0
enum class NodeJoinType{ JoinANode, JoinTwoTable, LeafNode };

std::string NodeJoinTypeStr(NodeJoinType node_join_type);

struct Old_Tree_Node{
	NodeJoinType joinType;

	//    just for joinType == JoinANode
	int node_to_join;


	Old_Tree_Node *left_node;
	Old_Tree_Node *right_node;


	Old_Tree_Node(){};

	Old_Tree_Node(int next_node){
		joinType = NodeJoinType::LeafNode;
		node_to_join = next_node;
		left_node = nullptr;
		right_node = nullptr;
	}

	Old_Tree_Node(Old_Tree_Node *node_need_copy){
		joinType = node_need_copy->joinType;
		if(joinType == NodeJoinType::LeafNode){
			node_to_join = node_need_copy->node_to_join;
		}

		if(node_need_copy->left_node != nullptr){
			left_node = new Old_Tree_Node(node_need_copy->left_node);
		} else{
			left_node = nullptr;
		}

		if(node_need_copy->right_node != nullptr){
			right_node = new Old_Tree_Node(node_need_copy->right_node);
		} else{
			right_node = nullptr;
		}
	}
};

class OldPlanTree {

public:
	Old_Tree_Node *root_node;
	unsigned long plan_cost;

	OldPlanTree()= default;;
	OldPlanTree(int first_node);
	OldPlanTree(OldPlanTree *last_plantree, int next_node);
	OldPlanTree(OldPlanTree *left_plan, OldPlanTree *right_plan);

	//    for create plan manually
	OldPlanTree(const vector<int> nodes_order);

	// todo: use this to generate query plan for giving join node order
	OldPlanTree(const vector<unsigned> nodes_order, BGPQuery *bgpquery);

	void delete_tree_node(Old_Tree_Node* root_node);
	~OldPlanTree();

	void print_tree_node(Old_Tree_Node* node, BasicQuery* basicquery);
	void print(BasicQuery* basicquery);
};
// codes above are for yanglei's paper



// codes below will be used in gStore v1.0
// todo: just use node, *left_node, *right_node
struct Tree_node{
    NodeJoinType joinType;
    OneStepJoin node;
    // vector<unsigned >

//    just for joinType == JoinANode
    int node_to_join;

    Tree_node *left_node;
    Tree_node *right_node;

    Tree_node(){};

    Tree_node(int next_node){
        joinType = NodeJoinType::LeafNode;
        node_to_join = next_node;
        left_node = nullptr;
        right_node = nullptr;
    }

    Tree_node(Tree_node *node_need_copy){
        joinType = node_need_copy->joinType;
        if(joinType == NodeJoinType::LeafNode){
            node_to_join = node_need_copy->node_to_join;
        }

        if(node_need_copy->left_node != nullptr){
            left_node = new Tree_node(node_need_copy->left_node);
        } else{
            left_node = nullptr;
        }

        if(node_need_copy->right_node != nullptr){
            right_node = new Tree_node(node_need_copy->right_node);
        } else{
            right_node = nullptr;
        }
    }


};

class PlanTree {

public:
    Tree_node *root_node;
    unsigned long plan_cost;

    PlanTree()= default;;
    PlanTree(int first_node);
    PlanTree(PlanTree *last_plantree, int next_node);
    PlanTree(PlanTree *left_plan, PlanTree *right_plan);

//    for create plan manually
    PlanTree(const vector<int> nodes_order);

    // todo: use this to generate query plan for giving join node order
    PlanTree(const vector<unsigned> nodes_order, BGPQuery *bgpquery);

    void delete_tree_node(Tree_node* root_node);
    ~PlanTree();

    void print_tree_node(Tree_node* node, BasicQuery* basicquery);
    void print(BasicQuery* basicquery);
};


#endif //GSTORE_PLANTREE_H
