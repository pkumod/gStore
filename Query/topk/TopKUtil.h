//
// Created by Yuqi Zhou on 2021/9/11.
//

#ifndef GSTOREGDB_QUERY_TOPK_DPB_TOPKUTIL_H_
#define GSTOREGDB_QUERY_TOPK_DPB_TOPKUTIL_H_

#include "../../Util/Util.h"
#include "../../KVstore/KVstore.h"
#include "DPB/OrderedList.h"
#include "../../Query/SPARQLquery.h"
#include "../../Query/BasicQuery.h"
#include "../../Database/Statistics.h"
#include "../../Query/QueryTree.h"
#include "../../Query/IDList.h"
#include "../../Database/TableOperator.h"
#include "TopKSearchPlan.h"

namespace TopKUtil {

struct Env{
  KVstore *kv_store;
  unsigned int limitID_entity;
  unsigned int limitID_literal;
  BasicQuery *basic_query;
  shared_ptr<BGPQuery> bgp_query;
  shared_ptr<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>> id_caches;
  int k;
  // store the non-tree edges to speed up result enumeration
  std::shared_ptr<std::vector<std::map<TYPE_ENTITY_LITERAL_ID,std::set<TYPE_ENTITY_LITERAL_ID> >>> non_tree_edges_lists_;
  std::shared_ptr<map<std::string, double>> coefficients;
  shared_ptr<Transaction> txn;
  std::shared_ptr<stringstream> ss;
};

void UpdateIDList(shared_ptr<IDList> valid_id_list, unsigned* id_list, unsigned id_list_len,bool id_list_prepared);

void UpdateIDListWithAppending(shared_ptr<IDListWithAppending> &ids_appending, unsigned* id_list,
                               unsigned id_list_len,unsigned  one_record_len,
                               bool id_list_prepared,unsigned main_key_position);

std::set<TYPE_ENTITY_LITERAL_ID> // child_candidates
ExtendTreeEdge(std::set<TYPE_ENTITY_LITERAL_ID>& parent_var_candidates,int child_var,
               std::set<TYPE_ENTITY_LITERAL_ID>& deleted_parents,
               std::map<TYPE_ENTITY_LITERAL_ID, shared_ptr<IDListWithAppending>  > &parent_child,
               std::map<TYPE_ENTITY_LITERAL_ID, std::set<TYPE_ENTITY_LITERAL_ID> > &child_parent,
               std::shared_ptr<TopKPlanUtil::TreeEdge> tree_edges_,
               Env *env);

void AddRelation(TYPE_ENTITY_LITERAL_ID x,TYPE_ENTITY_LITERAL_ID y, std::map<TYPE_ENTITY_LITERAL_ID ,std::set<TYPE_ENTITY_LITERAL_ID >>& mapping);


void CalculatePosVarMappingNode(TopKTreeNode* top_k_tree_node,shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>> pos_var_mapping);
shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>> CalculatePosVarMapping(shared_ptr<TopKSearchPlan> search_plan);

double GetScore(string &v, stringstream &ss);
void GetVarCoefficientsTreeNode(QueryTree::CompTreeNode *comp_tree_node,
                                std::map<std::string,double>& coefficients,
                                bool minus_signed=false);


std::shared_ptr<std::map<std::string,double>> getVarCoefficients(QueryTree::Order order);

std::shared_ptr< std::map<TYPE_ENTITY_LITERAL_ID,double>>
GetChildNodeScores(double coefficient,
                   std::set<TYPE_ENTITY_LITERAL_ID> &child_candidates,
                   bool has_parent,
                   std::set<TYPE_ENTITY_LITERAL_ID>* deleted_parents,
                   std::map<TYPE_ENTITY_LITERAL_ID, shared_ptr<IDListWithAppending>  > *parent_child,
                   std::map<TYPE_ENTITY_LITERAL_ID, std::set<TYPE_ENTITY_LITERAL_ID> > *child_parent,
                   Env *env);

};

#endif //GSTOREGDB_QUERY_TOPK_DPB_TOPKUTIL_H_
