/*=============================================================================
# Filename: BGPQuery.cpp
# Author: Linglin Yang
# Mail: linglinyang@stu.pku.edu.cn
=============================================================================*/

#include "BGPQuery.h"

using namespace std;


VarDescriptor::VarDescriptor(VarType var_type, const string &var_name):
	var_type_(var_type), var_name_(var_name), selected_(false){};

// bool VarDescriptor::get_edge_type(int edge_id) {
// 	return this->edge_type_[edge_id];
// }
//
// int VarDescriptor::get_edge_nei(int edge_id) {
// 	return this->edge_nei_[edge_id];
// }
//
// int VarDescriptor::get_edge_index(int edge_id) {
// 	return this->edge_index_[edge_id];
// }

void VarDescriptor::update_so_var_edge_info(unsigned int edge_nei_id, TYPE_PREDICATE_ID pre_id, char edge_type,
											unsigned int edge_index, bool pre_is_var, bool edge_nei_is_var) {
	if(edge_nei_is_var){
		this->var_edge_nei_.push_back(edge_nei_id);
		this->var_edge_pre_id_.push_back(pre_id);
		this->var_edge_type_.push_back(edge_type);
		this->var_edge_index_.push_back(edge_index);

		if(pre_is_var){
			this->var_edge_pre_type_.push_back(PreType::VarPreType);
		} else{
			this->var_edge_pre_type_.push_back(PreType::ConPreType);
		}
	} else{
		this->con_edge_nei_.push_back(edge_nei_id);
		this->con_edge_pre_id_.push_back(pre_id);
		this->con_edge_type_.push_back(edge_type);
		this->con_edge_index_.push_back(edge_index);

		if(pre_is_var){
			this->con_edge_pre_type_.push_back(PreType::VarPreType);
		} else{
			this->con_edge_pre_type_.push_back(PreType::ConPreType);
		}
	}
}

void VarDescriptor::update_pre_var_edge_info(unsigned int s_id, unsigned int o_id, bool s_is_var, bool o_is_var,
											 unsigned int edge_index) {
	this->s_id_.push_back(s_id);
	this->o_id_.push_back(o_id);

	if(s_is_var){
		this->s_type_.push_back(EntiType::VarEntiType);
	} else{
		this->s_type_.push_back(EntiType::ConEntiType);
	}

	if(o_is_var){
		this->o_type_.push_back(EntiType::VarEntiType);
	} else{
		this->o_type_.push_back(EntiType::ConEntiType);
	}

	this->edge_index_.push_back(edge_index);
}

void VarDescriptor::update_statistics_num() {
	if(this->var_type_ == VarDescriptor::VarType::Entity){
		this->so_var_var_edge_num = this->var_edge_type_.size();
		this->so_var_con_edge_num = this->con_edge_type_.size();

		this->degree_ = this->so_var_var_edge_num + this->so_var_con_edge_num;
	} else{
		this->degree_ = this->edge_index_.size();
	}
}

void VarDescriptor::update_select_status(bool selected) {
	this->selected_ = selected;
}

BGPQuery::BGPQuery() {
	this->initial();

}


// general_evalutial first new one BGPQuery, than initial it.
// than add Triple one by one.
void BGPQuery::initial() {
	;
}


void BGPQuery::AddTriple(const Triple& _triple)
{
	this->triple_vt.push_back(_triple);
}

unsigned BGPQuery::get_var_id_by_name(const string& var_name) {
	return this->var_item_to_position[var_name];
	// for(unsigned i = 0; i < this->var_vector.size(); ++i){
	// 	if(var_vector[i]->var_name_ == var_name){
	// 		return i;
	// 	}
	// }
	//
	// // not found, return -1
	// return -1;
}


void BGPQuery::ScanAllVar() {

	bool not_found;
	unsigned index = 0;
	for(const auto &triple : triple_vt){
//		sub
		not_found = (this->item_to_freq.find(triple.subject) == this->item_to_freq.end());
		if(not_found){
			this->item_to_freq[triple.subject] = 1;
			if(triple.subject.at(0) == '?') {
				auto new_sub_var = make_shared<VarDescriptor>(VarDescriptor::VarType::Entity, triple.subject);
				this->var_vector.push_back(new_sub_var);
				this->var_item_to_position[triple.subject] = index;
				this->so_var_id.push_back(index);
				this->total_so_var_num += 1;
				index += 1;
			}
		} else{
			++(this->item_to_freq[triple.subject]);
		}

//		pre
		not_found = (this->item_to_freq.find(triple.predicate) == this->item_to_freq.end());
		if(not_found){
			this->item_to_freq[triple.predicate] = 1;
			if(triple.predicate.at(0) == '?') {
				auto new_pre_var = make_shared<VarDescriptor>(VarDescriptor::VarType::Predicate, triple.predicate);
				this->var_vector.push_back(new_pre_var);
				this->var_item_to_position[triple.predicate] = index;
				this->pre_var_id.push_back(index);
				this->total_pre_var_num += 1;
				index += 1;
			}

		} else{
			++(this->item_to_freq[triple.predicate]);
		}

//		obj
		not_found = (this->item_to_freq.find(triple.object) == this->item_to_freq.end());
		if(not_found){
			this->item_to_freq[triple.object] = 1;
			if(triple.object.at(0) == '?') {
				auto new_obj_var = make_shared<VarDescriptor>(VarDescriptor::VarType::Entity, triple.object);
				this->var_vector.push_back(new_obj_var);
				this->var_item_to_position[triple.object] = index;
				this->so_var_id.push_back(index);
				this->total_so_var_num += 1;
				index += 1;
			}

		} else{
			++(this->item_to_freq[triple.object]);
		}
	}

}

void BGPQuery::build_edge_info(KVstore *_kvstore) {

	for(unsigned i = 0; i < this->triple_vt.size(); ++i){
		// TODO:not support pre var
		string s_string = triple_vt[i].subject;
		string p_string = triple_vt[i].predicate;
		string o_string = triple_vt[i].object;

		bool s_is_var = true;
		bool p_is_var = true;
		bool o_is_var = true;

		TYPE_ENTITY_LITERAL_ID s_id;
		TYPE_ENTITY_LITERAL_ID o_id;
		TYPE_PREDICATE_ID p_id;


		// TODO: if not found, return -1, query has no answer
		if(s_string.at(0) != '?'){
			s_id = _kvstore->getIDByString(s_string);
			s_is_var = false;
		} else{
			s_id = this->get_var_id_by_name(s_string);
		}

		if(o_string.at(0) != '?'){
			o_id = _kvstore->getIDByString(o_string);
			o_is_var = false;
		} else{
			o_id = this->get_var_id_by_name(o_string);
		}

		if(p_string.at(0) != '?'){
			p_id = _kvstore->getIDByPredicate(p_string);
			p_is_var = false;
		} else{
			p_id = this->get_var_id_by_name(s_string);
		}

		// deal with s_var
		if(s_is_var)
			this->var_vector[s_id]->update_so_var_edge_info(o_id,p_id,Util::EDGE_OUT,i,p_is_var,o_is_var);

		// deal with o_var
		if(o_is_var)
			this->var_vector[o_id]->update_so_var_edge_info(s_id,p_id,Util::EDGE_IN,i,p_is_var,o_is_var);

		// deal with p_var
		if(p_is_var)
			this->var_vector[p_id]->update_pre_var_edge_info(s_id,o_id,s_is_var,o_is_var,i);

	}
}

void BGPQuery::count_statistics_num() {

	// TODO: need to check whether this = var_so_num + var_pre_num
	this->total_var_num = this->var_vector.size();

	for(unsigned i = 0; i < this->var_vector.size(); ++ i){
		this->var_vector[i]->update_statistics_num();
	}
}

/**
 *
 * @param _kvstore
 * @param _query_var the
 * @return
 */
bool BGPQuery::EncodeBGPQuery(KVstore *_kvstore, const vector<string> &_query_var) {


	// I want this function scan all vars, incluing all pre_var and subject_or_object_var
	this->ScanAllVar();

	this->build_edge_info(_kvstore);

	this->count_statistics_num();



//	I want to see what is _query_var
	for(unsigned i = 0; i < _query_var.size(); ++i){
		cout << "_query_var[" << i << "] = " << _query_var[i] << endl;
	}

	return true;


}


unsigned int BGPQuery::get_triple_num() {
	return this->triple_vt.size();
}

unsigned int BGPQuery::get_total_var_num() {
	return this->total_var_num;
}

unsigned int BGPQuery::get_pre_var_num() {
	return this->total_pre_var_num;
}

// bool BGPQuery::get_edge_type(int var_id, int edge_id) {
// 	return var_vector[var_id]->edge_type_[edge_id];
// }
//
// int BGPQuery::get_edge_nei(int var_id, int edge_id) {
// 	return var_vector[var_id]->edge_nei_[edge_id];
// }
//
// int BGPQuery::get_edge_index(int var_id, int edge_id) {
// 	return var_vector[var_id]->edge_index_[edge_id];
// }

