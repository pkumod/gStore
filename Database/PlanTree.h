//
// Created by fyuling on 2021/5/6.
//

#ifndef GSTORE_PLANTREE_H
#define GSTORE_PLANTREE_H

#include <vector>

using namespace std;

enum class NodeJoinType{ JoinANode ,JoinTwoTable };

struct Tree_node{
    NodeJoinType joinType;

//    just for joinType == JoinANode
    int node_to_join;

    Tree_node *left_node;
    Tree_node *right_node;
};

class PlanTree {

public:
    Tree_node *root_node;

    PlanTree(vector<vector<int>> &best_plan_vec);
};


#endif //GSTORE_PLANTREE_H
