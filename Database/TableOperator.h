/*=============================================================================
# Filename: TableOperator.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
=============================================================================*/

#include "../Util/Util.h"
#include "../Query/BasicQuery.h"
#include "../Query/BGPQuery.h"


#ifndef GSTORELIMITK_DATABASE_TABLEOPERATOR_H_
#define GSTORELIMITK_DATABASE_TABLEOPERATOR_H_

/* only consider extend 1 node per step */
enum class JoinMethod{s2p,s2o,p2s,p2o,o2s,o2p,so2p,sp2o,po2s,
					  s2po,p2so,o2ps};

std::string JoinMethodToString(JoinMethod x);

// It will be abandoned
class OldVarDescriptor{
 public:
  enum class VarType{EntityType,PredicateType};
  bool selected_;
  int degree_;
  int rewriting_position_;
  // -1 if not allocated a id in BasicQuery
  int basic_query_id;
  VarType var_type_;
  std::string var_name_;

  bool IsSatellite(){return this->degree_ == 1 && (!selected_);};
  int RewritingPosition(){return this->rewriting_position_;};
  // calculate the exact position in final result

  OldVarDescriptor(VarType var_type,std::string var_name,BasicQuery* basic_query):
  var_type_(var_type),var_name_(var_name)
  {
    if(basic_query== nullptr)
      return;
    this->basic_query_id = basic_query->getIDByVarName(var_name);
    this->selected_ = basic_query->isVarSelected(var_name);
    this->degree_ = basic_query->getVarDegree(basic_query_id);

    if(!this->selected_)
      this->rewriting_position_ = -1;
    // record_len = select_var_num + selected_pre_var_num;
    if(this->var_type_ == VarType::EntityType)
      this->rewriting_position_ = basic_query->getSelectedVarPosition(this->var_name_);

    if(this->var_type_ == VarType::PredicateType)
      this->rewriting_position_ = basic_query->getSelectedPreVarPosition(this->var_name_);
  };
};

class IntermediateResult{
 private:
 public:

  TableContentShardPtr values_;
  PositionValueSharedPtr id_pos_map;
  PositionValueSharedPtr pos_id_map;

  IntermediateResult(const PositionValueSharedPtr& id_to_position,
                     const PositionValueSharedPtr& position_to_id,
                     TableContentShardPtr values);
  IntermediateResult(const IntermediateResult& other) = default;
  IntermediateResult();
  static IntermediateResult OnlyPositionCopy(const IntermediateResult& other);
  static std::shared_ptr<IntermediateResult> JoinTwoStructure(const std::shared_ptr<IntermediateResult>& result_a,
                                                              const std::shared_ptr<IntermediateResult>& result_b,
                                                              const std::shared_ptr< std::vector<TYPE_ENTITY_LITERAL_ID> >& public_variables);
  TYPE_ENTITY_LITERAL_ID AddNewNode(TYPE_ENTITY_LITERAL_ID node_id);
  size_t GetColumns(){return this->id_pos_map->size();}
};

class EdgeConstantInfo;
class EdgeInfo{
 public:
  // VarDescriptor ID
  TYPE_ENTITY_LITERAL_ID s_;
  TYPE_ENTITY_LITERAL_ID p_;
  TYPE_ENTITY_LITERAL_ID o_;
  EdgeInfo(TYPE_ENTITY_LITERAL_ID s,TYPE_ENTITY_LITERAL_ID p,TYPE_ENTITY_LITERAL_ID o,JoinMethod method):
      s_(s),p_(p),o_(o),join_method_(method){}

  // return one var's index edge info, now only used in special case when triple_num == 1 query
  EdgeInfo(shared_ptr<VarDescriptor> var_descrip, unsigned index, bool is_pre_var){
  	if(is_pre_var){
  		// var_descrip is a pre_var
  		s_ = var_descrip->s_id_[index];
  		o_ = var_descrip->o_id_[index];
  		p_ = var_descrip->id_;
  		if(var_descrip->s_type_[index] == VarDescriptor::EntiType::VarEntiType){
  			if(var_descrip->o_type_[index] == VarDescriptor::EntiType::VarEntiType){
  				cout << "error: sub, pre, obj in this triple are all var" << endl;
  				exit(-1);
  			}
			else
				join_method_ = JoinMethod::o2ps;
  		} else{
  			if(var_descrip->o_type_[index] == VarDescriptor::EntiType::VarEntiType)
  				join_method_ = JoinMethod::s2po;
			else
				join_method_ = JoinMethod::so2p;
  		}
  	} else{

  		// var_descrip is a so_var

  		if(var_descrip->so_edge_type_[index] == Util::EDGE_IN){
  			s_ = var_descrip->so_edge_nei_[index];
  			o_ = var_descrip->id_;
  		} else{
  			s_ = var_descrip->id_;
  			o_ = var_descrip->so_edge_nei_[index];
  		}

  		p_ = var_descrip->so_edge_pre_id_[index];

  		if(var_descrip->so_edge_nei_type_[index] == VarDescriptor::EntiType::VarEntiType){
  			if(var_descrip->so_edge_pre_type_[index] == VarDescriptor::PreType::VarPreType){
  				cout << "error: sub, pre, obj in this triple are all var" << endl;
  				exit(-1);
  			} else{
  				// nei is var, pre is const
  				join_method_ = JoinMethod::p2so;
  			}
  		} else{
  			if(var_descrip->so_edge_pre_type_[index] == VarDescriptor::PreType::VarPreType){
  				// nei is const, pre is var
  				if(var_descrip->so_edge_type_[index] == Util::EDGE_IN)
  					join_method_ = JoinMethod::s2po;
				else
					join_method_ = JoinMethod::o2ps;
  			} else{
  				// nei is const, pre is const
  				if(var_descrip->so_edge_type_[index] == Util::EDGE_IN)
  					join_method_ = JoinMethod::sp2o;
				else
					join_method_ = JoinMethod::po2s;
  			}
  		}
  	}
  }

  JoinMethod join_method_;
  TYPE_ENTITY_LITERAL_ID getVarToFilter() const;
  std::pair<bool,TYPE_ENTITY_LITERAL_ID> getVarToFilter(EdgeConstantInfo constant_info) const;
  std::string toString() const;
  EdgeInfo() = default;
};

class EdgeConstantInfo{
 public:
  bool s_constant_;
  bool p_constant_;
  bool o_constant_;
  EdgeConstantInfo(bool s_constant, bool p_constant, bool o_constant):
      s_constant_(s_constant), p_constant_(p_constant), o_constant_(o_constant){
  }
  bool ConstantToVar(EdgeInfo edge_info);
  EdgeConstantInfo():s_constant_(false), p_constant_(false), o_constant_(false){};
  std::string toString();
};

std::string EdgeToString(KVstore *kv_store,EdgeInfo edge_info,EdgeConstantInfo edge_constant_info);

/* Extend One Table, add a new Node.
 * The Node can have a candidate list
 * or to check if a variable has a certain edge */
class FeedOneNode{
 public:
  TYPE_ENTITY_LITERAL_ID node_to_join_;
  bool node_should_be_added_into_table;
  std::shared_ptr<std::vector<EdgeInfo>> edges_;
  std::shared_ptr<std::vector<EdgeConstantInfo>> edges_constant_info_;
  FeedOneNode():node_to_join_(-1),node_should_be_added_into_table(true){
    edges_ = std::make_shared<std::vector<EdgeInfo>>();
    edges_constant_info_ = std::make_shared<std::vector<EdgeConstantInfo>>();
  };

  FeedOneNode(unsigned join_node_id,
              EdgeInfo edge_info,
              EdgeConstantInfo edge_constant,
              bool added_into_table=true):
      node_to_join_(join_node_id),node_should_be_added_into_table(added_into_table)
  {
    edges_ = make_shared<vector<EdgeInfo>>(vector<EdgeInfo>{edge_info});
    edges_constant_info_ = make_shared<vector<EdgeConstantInfo>>(vector<EdgeConstantInfo>{edge_constant});
  };
  FeedOneNode(unsigned join_node_id,
              shared_ptr<vector<EdgeInfo>> edge_info,
              shared_ptr<vector<EdgeConstantInfo>> edge_constant,
              bool added_into_table=true):
      node_to_join_(join_node_id),node_should_be_added_into_table(added_into_table),edges_(edge_info), edges_constant_info_(edge_constant)
  {};

  void ChangeOrder(std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> already_in);
};

class FeedTwoNode{
public:
	TYPE_ENTITY_LITERAL_ID node_to_join_1_;
	TYPE_ENTITY_LITERAL_ID node_to_join_2_;
	EdgeInfo edges_;
	EdgeConstantInfo edges_constant_info_;
	FeedTwoNode()=default;
	FeedTwoNode(unsigned node_1, unsigned node_2, EdgeInfo edge_info, EdgeConstantInfo edge_constant):
		node_to_join_1_(node_1), node_to_join_2_(node_2), edges_(edge_info), edges_constant_info_(edge_constant){};
};

/* Join Two Table on Public Variables*/
class JoinTwoTable{
 public:
  // VarDescriptor ID
  std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> public_variables_;
  JoinTwoTable(std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> public_variables): public_variables_(public_variables){};
  JoinTwoTable(){ this->public_variables_=std::make_shared<std::vector<TYPE_ENTITY_LITERAL_ID>>();};
};

class StepOperation{
 public:
  // TODO merge the four together
  std::shared_ptr<FeedOneNode> join_node_; // GetAllTriples uses this field
  std::shared_ptr<FeedTwoNode> join_two_node_;
  std::shared_ptr<JoinTwoTable> join_table_;
  std::shared_ptr<FeedOneNode> edge_filter_; // GenerateCandidates & EdgeCheck use this field

  // ConstCandidatesCheck
  enum class JoinType{
    // join a node to table, or create a table with one node
    JoinNode,
    // change the candidates list for one node
    GenerateCandidates,
    // join two table together
    JoinTable,
    // check if an edge exist. the nodes in the edge should already in the table
    EdgeCheck,
    // join two node to table,instead of joining twice
    JoinTwoNodes,
    // get all triples in the dataset
    GetAllTriples
  };

  JoinType join_type_;
  bool distinct_;
  bool remain_old_result_;

  StepOperation(): join_node_(nullptr), join_two_node_(nullptr), join_table_(nullptr), edge_filter_(nullptr),distinct_(false),remain_old_result_(false){};

  StepOperation(JoinType join_type, shared_ptr<FeedOneNode> join_node, shared_ptr<FeedTwoNode> join_two_nodes,
			  shared_ptr<JoinTwoTable> join_table, shared_ptr<FeedOneNode> edge_filter, bool distinct = false,bool remain_old_result=false):
			  join_node_(join_node), join_two_node_(join_two_nodes),
			  join_table_(join_table), edge_filter_(edge_filter),
			  join_type_(join_type), distinct_(distinct),
			  remain_old_result_(remain_old_result){};

  std::string static JoinTypeToString(JoinType x){
    switch (x) {
      case JoinType::JoinNode: return "JoinType::JoinNode";
      case JoinType::JoinTable: return "JoinType::JoinTable";
      case JoinType::GenerateCandidates: return "JoinType::GenerateCandidates";
      case JoinType::EdgeCheck: return "JoinType::EdgeCheck";
      case JoinType::JoinTwoNodes: return "JoinType::JoinTwoNodes";
      case JoinType::GetAllTriples: return "JoinType::GetAllTriples";
    }
    return "err in JoinTypeToString";
  };
};

/**
 * Requirements  from cpp reference:
 * For all a, comp(a,a)==false
 * If comp(a,b)==true then comp(b,a)==false
 * if comp(a,b)==true and comp(b,c)==true then comp(a,c)==true
 */
struct IndexedRecordResultCompare
{
  bool operator()(const std::vector<TYPE_ENTITY_LITERAL_ID> &a, const std::vector<TYPE_ENTITY_LITERAL_ID> &b)
  {
    for(decltype(a.size()) i = 0;i<a.size();i++)
    {
      if(a[i]<b[i])
        return true;
      else if(a[i]>b[i])
        return false;
    }
    /* a==b */
    return false;
  }
};

#endif //GSTORELIMITK_DATABASE_TABLEOPERATOR_H_
