/*=============================================================================
# Filename: BGPQuery.h
# Author: Linglin Yang
# Mail: linglinyang@stu.pku.edu.cn
=============================================================================*/


// READ THIS FIRST
// According to need of rewriting, the whole BGP(including all triples) invokes EncodeBGPQuery,
// the split small BGP invokes EncodeSmallBGPQuery(the id is same as above, so maybe not begin with 0 or not continuous).
// For example, the whole query is:
// select ?s ?o ?o2 ?o3 where{
// 	?s p1 ?o.
// 	?s p2 ?o2.
// 	Optional{?s p3 ?o3}
// }
// Then after EncodeBGPQuery, the var-encode map is: {{?s:0}, {?o:1}, {?o2:2}, {?o3:3}},
// and in small optional BGP(?s p3 ?o3), the var-encode map is {{?s:0}, {?o3:3}}, not {{?s:0}, {?o3:1}}.

#ifndef GSTORE_BGPQUERY_H
#define GSTORE_BGPQUERY_H

#include "BasicQuery.h"
#include "../Util/Util.h"
#include "../Util/Triple.h"
#include "../KVstore/KVstore.h"

#include <cstdlib>
#include <utility>
#include <vector>


using namespace std;



class VarDescriptor{

public:
	// cannot descriminate literal with entity
	// note: NotDecided only used in PlanGenerator
	enum class VarType{Entity, Predicate, NotDecided};
	enum class EntiType{VarEntiType, ConEntiType};
	enum class ItemType{SubType, PreType, ObjType};
	enum class PreType{VarPreType, ConPreType};

	unsigned id_;
	VarType var_type_;
	std::string var_name_;

	// todo: use this to denote whether this var is selected or not
	bool selected_;
	bool link_with_const;

	// degree_ is the num of how many triples include this item,
	// include entitytype or pretype
	unsigned degree_;

	// only used when var_type_ == Entity
	// unsigned so_var_var_edge_num;
	// unsigned so_var_con_edge_num;


	vector<ItemType> so_var_item_type;
	// only used when var_type_ == Entity
	vector<char> so_edge_type_;
	vector<unsigned> so_edge_index_;
	vector<unsigned> so_edge_nei_;
	vector<EntiType> so_edge_nei_type_;
	vector<unsigned> so_edge_pre_id_;
	vector<PreType> so_edge_pre_type_;

	// connect to var so, only used when var_type_ == Entity,
	// var_edge_pre_type_ records whether the pre is var or not
	// vector<char> var_edge_type_;
	// vector<unsigned > var_edge_index_;
	// vector<unsigned > var_edge_nei_;
	// vector<TYPE_PREDICATE_ID> var_edge_pre_id_;
	// vector<PreType> var_edge_pre_type_;


	// connect to constant so, only used when var_type == Entity,
	// con_edge_pre_type_ records whether the pre is var or not
	// vector<char> con_edge_type_;
	// vector<unsigned > con_edge_index_;
	// vector<unsigned > con_edge_nei_;
	// vector<TYPE_PREDICATE_ID> con_edge_pre_id_;
	// vector<PreType> con_edge_pre_type_;


	//pre_var edge info, only used when var_type == Predicate
	vector<unsigned> s_id_;
	vector<EntiType> s_type_;
	vector<unsigned> o_id_;
	vector<EntiType> o_type_;
	vector<unsigned> pre_edge_index_;

	// zhouyuqi, if use this, please add initial this in VarDescriptor(*****)
	int rewriting_position_;
	// -1 if not allocated a id in BasicQuery
	int basic_query_id;

	// bool IsSatellite() const{return this->degree_ == 1 && (!selected_);};
	// int RewritingPosition(){return this->rewriting_position_;};
	// calculate the exact position in final result

	VarDescriptor(unsigned id, VarType var_type, const string &var_name);


	// bool get_edge_type(int edge_id);
	// int get_edge_nei(int edge_id);
	// int get_edge_index(int edge_id);

	void update_so_var_edge_info(unsigned edge_nei_id, TYPE_PREDICATE_ID pre_id, char edge_type,
								 unsigned edge_index, bool pre_is_var, bool edge_nei_is_var);

	void update_pre_var_edge_info(unsigned s_id, unsigned o_id, bool s_is_var, bool o_is_var, unsigned edge_index);

	void update_statistics_num();

	//TODO, default appoint a var selected_ = false to true
	void update_select_status(bool selected);

	void print(KVstore *kvstore);

};


class SOVarDescriptor:public VarDescriptor{

};

// How to use this class?
// First, all triples need insert one by one by AddTriple
// Then, invoke the function EncodeBGPQuery, this will give every var(selected or not, predicate or not predicate) an id
// If you have special need to encode one small BGP, please invoke the function EncodeSmallBGPQuery,
// and pass big BGPQuery pointer to it, it will keep the var with the same string name having same.
class BGPQuery {
public:

	bool distinct_query;

	// all item, including s, p, o, whether var or not
	map<string, unsigned > item_to_freq;

	// map var item to its position in var_vector
	map<string, unsigned > var_item_to_position;
	map<string, unsigned > var_item_to_id;

	map<unsigned, unsigned> id_position_map;
	map<unsigned, unsigned> position_id_map;


	vector<unsigned> s_id_;
	vector<unsigned> p_id_;
	vector<unsigned> o_id_;

	vector<bool> s_is_constant_;
	vector<bool> p_is_constant_;
	vector<bool> o_is_constant_;

	// all var, whether pre or not_pre, whether selected or not_selected
	vector<shared_ptr<VarDescriptor>> var_vector;

	// vector indicate so_var position in var_vector
	vector<unsigned> so_var_id;
	vector<unsigned> pre_var_id;
	vector<unsigned> var_id_vec;

	// vector indicate selected var position in var_vector, whether pre_var or so_var
	vector<unsigned> selected_var_id;

	vector<string> pre_var_names;

//	maybe not used
//	this record all pre_var position in item
//	vector<int> pre_var_position;

	int selected_pre_var_num;
	int selected_so_var_num;
	int total_selected_var_num;

	unsigned total_pre_var_num;
	unsigned total_so_var_num;
	unsigned total_var_num;

//	maybe not use join_pre_var_num
	int join_pre_var_num;
	int join_so_var_num;
	int total_join_var_num;

	shared_ptr<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>> var_candidates_cache;


	BGPQuery();
	~BGPQuery();
	void initial();


	void AddTriple(const Triple& _triple);

	// return var_id in var_vector, if not find, return -1;
	unsigned get_var_id_by_name(const string& var_name);
	string get_var_name_by_id(unsigned var_id);
	unsigned get_var_index_by_name(const string& var_name);


	unsigned get_var_id_by_index(unsigned index);
	unsigned get_var_position_by_id(unsigned id);

	const vector<unsigned> &get_var_id_vec();

	const shared_ptr<VarDescriptor> &get_vardescrip_by_index(unsigned index);
	const shared_ptr<VarDescriptor> &get_vardescrip_by_id(unsigned id);

	// void update_so_var_edge_info(uns);

	void ScanAllVar(const vector<string>& _query_var);
	bool build_edge_info(KVstore *_kvstore);
	void count_statistics_num();

	bool EncodeBGPQuery(KVstore* _kvstore, const vector<string>& _query_var, bool distinct = false);



	void ScanAllVarByBigBGPID(BGPQuery *big_bgpquery, const vector<string>& _query_var);
	bool EncodeSmallBGPQuery(BGPQuery *big_bgpquery_, KVstore* _kvstore,
							 const vector<string>& _query_var, bool distinct = false);

	static bool CheckConstBGPExist(const vector<Triple> &triple_vt, KVstore *kvstore);

	unsigned get_triple_num();
	unsigned get_total_var_num();
	unsigned get_pre_var_num();


	unsigned get_var_degree_by_id(unsigned var_id);
	VarDescriptor::VarType get_var_type_by_id(unsigned var_id);

	bool is_var_selected_by_id(unsigned var_id);

	// for EntiType var
	unsigned get_so_var_edge_index(unsigned var_id, int edge_id);
	bool get_so_var_edge_type(unsigned var_id, unsigned edge_id);
	unsigned get_so_var_edge_nei(unsigned var_id, unsigned edge_id);
	VarDescriptor::EntiType get_so_var_edge_nei_type(unsigned var_id, unsigned edge_id);
	unsigned get_so_var_edge_pre_id(unsigned var_id, unsigned edge_id);
	VarDescriptor::PreType get_so_var_edge_pre_type(unsigned var_id, unsigned edge_id);


	// for PreType var
	unsigned get_pre_var_edge_index(unsigned var_id, unsigned edge_id);
	unsigned get_pre_var_s_id(unsigned var_id, unsigned edge_id);
	VarDescriptor::EntiType get_pre_var_s_type(unsigned var_id, unsigned edge_id);
	unsigned get_pre_var_o_id(unsigned var_id, unsigned edge_id);
	VarDescriptor::EntiType get_pre_var_o_type(unsigned var_id, unsigned edge_id);

	bool check_already_joined_pre_var(vector<unsigned> &already_node, unsigned pre_var_id);

	const vector<Triple> &get_triple_vt();
	const Triple &get_triple_by_index(unsigned index);


	bool is_var_satellite_by_index(unsigned index);

	void print(KVstore * kvstore);

	void set_var_candidate_cache(unsigned var_id, shared_ptr<IDList> candidate_cache);

	shared_ptr<IDList> get_candidate_list_by_id(unsigned var_id);
	shared_ptr<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>> get_all_candidates();


		vector<unsigned*>* get_result_list_pointer();
    unique_ptr<unsigned[]>& resultPositionToId();
    /* tells if the var_id appears in the position of
     * subject / predicate / object
     * @returns (is_entity,is_literal,is_predicate)*/
    std::tuple<bool,bool,bool> GetOccurPosition(int var_id);
private:
	vector<Triple> triple_vt;
    vector<unsigned*> result_list;
    unique_ptr<unsigned[]> result_position_to_id;
};


#endif //GSTORE_BGPQUERY_H
