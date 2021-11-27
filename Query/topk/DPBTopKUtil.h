//
// Created by Yuqi Zhou on 2021/7/10.
//

#ifndef GSTORELIMITK_QUERY_TOPK_TOPKUTIL_H_
#define GSTORELIMITK_QUERY_TOPK_TOPKUTIL_H_


#include "TopKUtil.h"


namespace DPBUtil{

std::map<TYPE_ENTITY_LITERAL_ID,std::shared_ptr<FQIterator>>  AssemblingFrOw(set<TYPE_ENTITY_LITERAL_ID> &fq_ids,
                                                                    std::shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID,double>> node_scores, int k,
                                                                    vector<std::map<TYPE_ENTITY_LITERAL_ID, std::shared_ptr<FRIterator>>> &descendents_FRs,
                                                                    std::vector<std::map<TYPE_ENTITY_LITERAL_ID,std::pair<std::shared_ptr<OWIterator>,NodeOneChildVarPredicatesPtr>>>&descendents_OWs);


std::map<TYPE_ENTITY_LITERAL_ID, // parent id
         std::pair<std::shared_ptr<OWIterator>, // its OW
                   NodeOneChildVarPredicatesPtr>> // predicate correspond to the OW
GenerateOWs(int parent_var,int child_var,std::shared_ptr<TopKPlanUtil::TreeEdge> tree_edges_,
            std::set<TYPE_ENTITY_LITERAL_ID>& parent_var_candidates,
            std::set<TYPE_ENTITY_LITERAL_ID>& deleted_parents,
            TopKUtil::Env *env);

std::map<TYPE_ENTITY_LITERAL_ID,std::shared_ptr<FRIterator>>  GenerateFRs(int parent_var, int child_var, std::shared_ptr<TopKPlanUtil::TreeEdge> tree_edges_,
                                                            std::set<TYPE_ENTITY_LITERAL_ID>& parent_var_candidates,
                                                            std::set<TYPE_ENTITY_LITERAL_ID>& deleted_parents,
                                                            TopKTreeNode *child_tree_node, TopKUtil::Env *env);

FRIterator* BuildIteratorTree(const shared_ptr<TopKSearchPlan> tree_search_plan, TopKUtil::Env *env);

shared_ptr<IDList> GetAllSubObjId(KVstore *kv_store_,
                                  unsigned int limitID_entity_,
                                  unsigned int limitID_literal_,
                                  bool need_literal);

}
#endif //GSTORELIMITK_QUERY_TOPK_TOPKUTIL_H_
