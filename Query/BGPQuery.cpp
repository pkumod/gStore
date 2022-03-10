/*=============================================================================
# Filename: BGPQuery.cpp
# Author: Linglin Yang
# Mail: linglinyang@stu.pku.edu.cn
=============================================================================*/

#include "BGPQuery.h"

using namespace std;


VarDescriptor::VarDescriptor(unsigned id, VarType var_type, const string &var_name):
	id_(id), var_type_(var_type), var_name_(var_name), selected_(false), link_with_const(false), degree_(0){};


void VarDescriptor::update_so_var_edge_info(unsigned int edge_nei_id, TYPE_PREDICATE_ID pre_id, char edge_type,
											unsigned int edge_index, bool pre_is_var, bool edge_nei_is_var) {


	this->so_edge_nei_.push_back(edge_nei_id);
	this->so_edge_pre_id_.push_back(pre_id);
	this->so_edge_type_.push_back(edge_type);
	this->so_edge_index_.push_back(edge_index);
	if(pre_is_var)
		this->so_edge_pre_type_.push_back(PreType::VarPreType);
	else{
		this->so_edge_pre_type_.push_back(PreType::ConPreType);
		this->link_with_const = true;
	}

	if(edge_nei_is_var)
		this->so_edge_nei_type_.push_back(EntiType::VarEntiType);
	else{
		this->so_edge_nei_type_.push_back(EntiType::ConEntiType);
		this->link_with_const = true;
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
		this->link_with_const = true;
	}

	if(o_is_var){
		this->o_type_.push_back(EntiType::VarEntiType);
	} else{
		this->o_type_.push_back(EntiType::ConEntiType);
		this->link_with_const = true;
	}

	this->pre_edge_index_.push_back(edge_index);
}

void VarDescriptor::update_statistics_num() {
	if(this->var_type_ == VarDescriptor::VarType::Entity)
		this->degree_ = this->so_edge_index_.size();
	else
		this->degree_ = this->pre_edge_index_.size();
}

void VarDescriptor::update_select_status(bool selected) {
	this->selected_ = selected;
}

void VarDescriptor::print(KVstore *kvstore) {

	cout << "var: " << var_name_ << " , id is " << id_ << ", var type: " << (var_type_ == VarType::Entity ? "Entity" : "Predicate") << endl;
	cout << "degree = " << degree_ << ", selected: " << (selected_ ? "true" : "false") << endl;

	if(var_type_ == VarType::Entity){

		cout << "edge type, edge index, edge nei, edge nei type, var edge pre, var edge pre type: " << endl;
		for(unsigned i = 0; i < so_edge_type_.size(); ++i){
			cout << so_edge_type_[i] << '\t' << so_edge_index_[i] << '\t' << so_edge_nei_[i] << '\t';

			if(so_edge_nei_type_[i] == EntiType::VarEntiType)
				cout << so_edge_nei_[i] << '\t' << "varsotype" << '\t';
			else
				cout << kvstore->getStringByID(so_edge_nei_[i]) << '\t' << "consotype" << '\t';

			if(so_edge_pre_type_[i] == PreType::VarPreType){
				cout << so_edge_pre_id_[i] << '\t' << "varpretype" << endl;
			} else{
				cout << kvstore->getPredicateByID(so_edge_pre_id_[i]) << '\t' << "conpretype" << endl;
			}
		}

	} else{
		cout << "s_type_.size() = " << s_type_.size() << endl;
		for(unsigned i = 0; i < s_type_.size(); ++i){

			if(s_type_[i] == EntiType::ConEntiType){
				cout << "s is con, " << kvstore->getStringByID(s_id_[i]) << "\t";
			} else{
				cout << "s is var, " << s_id_[i] << "\t";
			}

			if(o_type_[i] == EntiType::ConEntiType){
				cout << "o is con, " << kvstore->getStringByID(o_id_[i]) << endl;
			} else{
				cout << "o is var, " << o_id_[i] << endl;
			}
		}
	}
}

BGPQuery::BGPQuery() {
	this->initial();
	var_candidates_cache = make_shared<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>>();

}

BGPQuery::~BGPQuery() {
	for(auto p:result_list) delete[] p;
}


// general_evalutial first new one BGPQuery, than initial it.
// than add Triple one by one.
void BGPQuery::initial() {

	this->total_var_num = 0;
	this->total_so_var_num = 0;
	this->total_pre_var_num = 0;

	this->total_join_var_num = 0;
	this->total_selected_var_num = 0;

	selected_pre_var_num = 0;
	selected_so_var_num = 0;
	total_selected_var_num = 0;
}


void BGPQuery::AddTriple(const Triple& _triple)
{
	this->triple_vt.push_back(_triple);
}

unsigned BGPQuery::get_var_id_by_name(const string& var_name) {
	return this->var_item_to_id[var_name];
	// for(unsigned i = 0; i < this->var_vector.size(); ++i){
	// 	if(var_vector[i]->var_name_ == var_name){
	// 		return i;
	// 	}
	// }
	//
	// // not found, return -1
	// return -1;
}

string BGPQuery::get_var_name_by_id(unsigned int var_id) {
	return var_vector[id_position_map[var_id]]->var_name_;
}

unsigned int BGPQuery::get_var_index_by_name(const string &var_name) {
	return this->var_item_to_position[var_name];
}

unsigned int BGPQuery::get_var_id_by_index(unsigned int index) {
	return this->var_id_vec[index];
}

unsigned int BGPQuery::get_var_position_by_id(unsigned int id) {
	return this->id_position_map[id];
}

const vector<unsigned int> &BGPQuery::get_var_id_vec() {
	return this->var_id_vec;
}

const shared_ptr<VarDescriptor> &BGPQuery::get_vardescrip_by_index(unsigned index) {
	return this->var_vector[index];
}

const shared_ptr<VarDescriptor> &BGPQuery::get_vardescrip_by_id(unsigned id) {
	return this->var_vector[this->get_var_position_by_id(id)];
}

void BGPQuery::ScanAllVar(const vector<string>& _query_var) {

	bool not_found;
	unsigned index = 0;
	for(const auto &triple : triple_vt){
//		sub
		not_found = (this->item_to_freq.find(triple.subject) == this->item_to_freq.end());
		if(not_found){
			this->item_to_freq[triple.subject] = 1;
			if(triple.subject.at(0) == '?') {
				auto new_sub_var = make_shared<VarDescriptor>(index, VarDescriptor::VarType::Entity, triple.subject);
				// todo: whether this should use std::move?
				this->var_vector.push_back(new_sub_var);

				this->var_item_to_position[triple.subject] = index;
				this->var_item_to_id[triple.subject] = index;

				this->id_position_map[index] = index;
				this->position_id_map[index] = index;

				this->so_var_id.push_back(index);
				this->var_id_vec.push_back(index);
				this->total_so_var_num += 1;

				// if selected
				if(find(_query_var.begin(), _query_var.end(), triple.subject) != _query_var.end()){
					this->selected_var_id.push_back(index);
					selected_so_var_num += 1;
				}

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
				auto new_pre_var = make_shared<VarDescriptor>(index, VarDescriptor::VarType::Predicate, triple.predicate);
				this->var_vector.push_back(new_pre_var);

				this->var_item_to_position[triple.predicate] = index;
				this->var_item_to_id[triple.predicate] = index;

				this->id_position_map[index] = index;
				this->position_id_map[index] = index;

				this->pre_var_id.push_back(index);
				this->var_id_vec.push_back(index);
				this->total_pre_var_num += 1;

				// if selected
				if(find(_query_var.begin(), _query_var.end(), triple.predicate) != _query_var.end()){
					this->selected_var_id.push_back(index);
					selected_pre_var_num += 1;
				}

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
				auto new_obj_var = make_shared<VarDescriptor>(index, VarDescriptor::VarType::Entity, triple.object);
				this->var_vector.push_back(new_obj_var);

				this->var_item_to_position[triple.object] = index;
				this->var_item_to_id[triple.object] = index;

				this->id_position_map[index] = index;
				this->position_id_map[index] = index;

				this->so_var_id.push_back(index);
				this->var_id_vec.push_back(index);
				this->total_so_var_num += 1;

				// if selected
				if(find(_query_var.begin(), _query_var.end(), triple.object) != _query_var.end()){
					this->selected_var_id.push_back(index);
					selected_so_var_num += 1;
				}

				index += 1;
			}

		} else{
			++(this->item_to_freq[triple.object]);
		}
	}

}

bool BGPQuery::build_edge_info(KVstore *_kvstore) {

	bool legal_bgp = true;

	for(unsigned i = 0; i < this->triple_vt.size(); ++i){
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
			if(s_id == INVALID) legal_bgp = false;
		} else{
			s_id = this->get_var_id_by_name(s_string);
		}

		if(o_string.at(0) != '?'){
			o_id = _kvstore->getIDByString(o_string);
			o_is_var = false;
			if(o_id == INVALID) legal_bgp = false;
		} else{
			o_id = this->get_var_id_by_name(o_string);
		}

		if(p_string.at(0) != '?'){
			p_id = _kvstore->getIDByPredicate(p_string);
			p_is_var = false;
			if(p_id == INVALID) legal_bgp = false;
		} else{
			p_id = this->get_var_id_by_name(p_string);
		}

		s_id_.push_back(s_id);
		p_id_.push_back(p_id);
		o_id_.push_back(o_id);

		s_is_constant_.push_back(!s_is_var);
		p_is_constant_.push_back(!p_is_var);
		o_is_constant_.push_back(!o_is_var);

		// deal with s_var
		if(s_is_var)
			this->var_vector[id_position_map[s_id]]->update_so_var_edge_info(o_id,p_id,Util::EDGE_OUT,i,p_is_var,o_is_var);

		// deal with o_var
		if(o_is_var)
			this->var_vector[id_position_map[o_id]]->update_so_var_edge_info(s_id,p_id,Util::EDGE_IN,i,p_is_var,s_is_var);

		// deal with p_var
		if(p_is_var)
			this->var_vector[id_position_map[p_id]]->update_pre_var_edge_info(s_id,o_id,s_is_var,o_is_var,i);

	}

	return legal_bgp;
}

void BGPQuery::count_statistics_num() {

	// TODO: need to check whether this = var_so_num + var_pre_num
	this->total_var_num = this->var_vector.size();
	this->total_selected_var_num = this->selected_var_id.size();

	for(unsigned i = 0; i < this->var_vector.size(); ++ i){
		this->var_vector[i]->update_statistics_num();
	}

	for(unsigned i = 0; i < selected_var_id.size(); ++i)
		this->var_vector[id_position_map[selected_var_id[i]]]->update_select_status(true);
}

/**
 *
 * @param _kvstore
 * @param _query_var the
 * @return
 */
bool BGPQuery::EncodeBGPQuery(KVstore *_kvstore, const vector<string> &_query_var, bool distinct) {


	this->distinct_query = distinct;

	// I want this function scan all vars, incluing all pre_var and subject_or_object_var
	this->ScanAllVar(_query_var);

	bool legal_bgp = this->build_edge_info(_kvstore);

	this->count_statistics_num();



//	I want to see what is _query_var
	for(unsigned i = 0; i < _query_var.size(); ++i){
		cout << "_query_var[" << i << "] = " << _query_var[i] << endl;
	}

	return legal_bgp;


}


void BGPQuery::ScanAllVarByBigBGPID(BGPQuery *big_bgpquery, const vector<string>& _query_var) {
	bool not_found;
	unsigned index = 0;
	for(const auto &triple : triple_vt){
		//		sub
		not_found = (this->item_to_freq.find(triple.subject) == this->item_to_freq.end());
		if(not_found){
			this->item_to_freq[triple.subject] = 1;
			if(triple.subject.at(0) == '?') {
				auto sub_id = big_bgpquery->get_var_id_by_name(triple.subject);
				auto new_sub_var = make_shared<VarDescriptor>(sub_id,VarDescriptor::VarType::Entity, triple.subject);
				this->var_vector.push_back(new_sub_var);

				this->var_item_to_position[triple.subject] = index;
				this->var_item_to_id[triple.subject] = sub_id;

				this->id_position_map[sub_id] = index;
				this->position_id_map[index] = sub_id;

				this->so_var_id.push_back(sub_id);
				this->var_id_vec.push_back(sub_id);
				this->total_so_var_num += 1;

				// if selected
				if(find(_query_var.begin(), _query_var.end(), triple.subject) != _query_var.end()){
					this->selected_var_id.push_back(sub_id);
					selected_so_var_num += 1;
				}

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
				auto pre_id = big_bgpquery->get_var_id_by_name(triple.predicate);
				auto new_pre_var = make_shared<VarDescriptor>(pre_id,VarDescriptor::VarType::Predicate, triple.predicate);
				this->var_vector.push_back(new_pre_var);

				this->var_item_to_position[triple.predicate] = index;
				this->var_item_to_id[triple.predicate] = pre_id;

				this->id_position_map[pre_id] = index;
				this->position_id_map[index] = pre_id;

				this->pre_var_id.push_back(pre_id);
				this->var_id_vec.push_back(pre_id);
				this->total_pre_var_num += 1;

				// if selected
				if(find(_query_var.begin(), _query_var.end(), triple.predicate) != _query_var.end()){
					this->selected_var_id.push_back(pre_id);
					selected_pre_var_num += 1;
				}

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
				auto obj_id = big_bgpquery->get_var_id_by_name(triple.object);
				auto new_obj_var = make_shared<VarDescriptor>(obj_id,VarDescriptor::VarType::Entity, triple.object);
				this->var_vector.push_back(new_obj_var);

				this->var_item_to_position[triple.object] = index;
				this->var_item_to_id[triple.object] = obj_id;

				this->id_position_map[obj_id] = index;
				this->position_id_map[index] = obj_id;

				this->so_var_id.push_back(obj_id);
				this->var_id_vec.push_back(obj_id);
				this->total_so_var_num += 1;

				// if selected
				if(find(_query_var.begin(), _query_var.end(), triple.object) != _query_var.end()){
					this->selected_var_id.push_back(obj_id);
					selected_so_var_num += 1;
				}

				index += 1;
			}

		} else{
			++(this->item_to_freq[triple.object]);
		}
	}
}


// this function is invoked after adding all triples of small BGP
bool BGPQuery::EncodeSmallBGPQuery(BGPQuery *big_bgpquery_, KVstore *_kvstore,
								   const vector<string> &_query_var, bool distinct) {

	this->distinct_query = distinct;

	this->ScanAllVarByBigBGPID(big_bgpquery_, _query_var);

	bool legal_bgp = this->build_edge_info(_kvstore);


	this->count_statistics_num();

	// todo: return false imply parse error
	return legal_bgp;

}



bool BGPQuery::CheckConstBGPExist(const vector<Triple> &triple_vt, KVstore *_kvstore) {
	for(unsigned i = 0; i < triple_vt.size(); ++i) {
		string s_string = triple_vt[i].subject;
		string p_string = triple_vt[i].predicate;
		string o_string = triple_vt[i].object;

		TYPE_ENTITY_LITERAL_ID s_id;
		TYPE_ENTITY_LITERAL_ID o_id;
		TYPE_PREDICATE_ID p_id;

		if(s_string.at(0) != '?'){
			s_id = _kvstore->getIDByString(s_string);
			if(s_id == INVALID) return false;
		} else{
			cout << "error: pass an var in CheckConstBGPExist!" << endl;
			exit(-1);
		}

		if(o_string.at(0) != '?'){
			o_id = _kvstore->getIDByString(o_string);
			if(o_id == INVALID) return false;
		} else{
			cout << "error: pass an var in CheckConstBGPExist!" << endl;
			exit(-1);
		}

		if(p_string.at(0) != '?'){
			p_id = _kvstore->getIDByPredicate(p_string);
			if(p_id == INVALID) return false;
		} else{
			cout << "error: pass an var in CheckConstBGPExist!" << endl;
			exit(-1);
		}

		if(!_kvstore->existThisTriple(s_id, p_id, o_id)) return false;
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


unsigned int BGPQuery::get_var_degree_by_id(unsigned int var_id) {
	return var_vector[id_position_map[var_id]]->degree_;
}

VarDescriptor::VarType BGPQuery::get_var_type_by_id(unsigned int var_id) {
	return var_vector[id_position_map[var_id]]->var_type_;
}

bool BGPQuery::is_var_selected_by_id(unsigned int var_id) {
	return var_vector[id_position_map[var_id]]->selected_;
}

unsigned int BGPQuery::get_so_var_edge_index(unsigned int var_id, int edge_id) {
	return var_vector[id_position_map[var_id]]->so_edge_index_[edge_id];
}

bool BGPQuery::get_so_var_edge_type(unsigned int var_id, unsigned int edge_id) {
	return var_vector[id_position_map[var_id]]->so_edge_type_[edge_id];
}

unsigned int BGPQuery::get_so_var_edge_nei(unsigned int var_id, unsigned int edge_id) {
	return var_vector[id_position_map[var_id]]->so_edge_nei_[edge_id];
}

VarDescriptor::EntiType BGPQuery::get_so_var_edge_nei_type(unsigned int var_id, unsigned int edge_id) {
	return var_vector[id_position_map[var_id]]->so_edge_nei_type_[edge_id];
}

unsigned int BGPQuery::get_so_var_edge_pre_id(unsigned int var_id, unsigned int edge_id) {
	return var_vector[id_position_map[var_id]]->so_edge_pre_id_[edge_id];
}

VarDescriptor::PreType BGPQuery::get_so_var_edge_pre_type(unsigned int var_id, unsigned int edge_id) {
	return var_vector[id_position_map[var_id]]->so_edge_pre_type_[edge_id];
}


unsigned int BGPQuery::get_pre_var_edge_index(unsigned int var_id, unsigned int edge_id) {
	return var_vector[id_position_map[var_id]]->pre_edge_index_[edge_id];
}

unsigned int BGPQuery::get_pre_var_s_id(unsigned int var_id, unsigned int edge_id) {
	return var_vector[id_position_map[var_id]]->s_id_[edge_id];
}

VarDescriptor::EntiType BGPQuery::get_pre_var_s_type(unsigned int var_id, unsigned int edge_id) {
	return var_vector[id_position_map[var_id]]->s_type_[edge_id];
}

unsigned int BGPQuery::get_pre_var_o_id(unsigned int var_id, unsigned int edge_id) {
	return var_vector[id_position_map[var_id]]->o_id_[edge_id];
}

VarDescriptor::EntiType BGPQuery::get_pre_var_o_type(unsigned int var_id, unsigned int edge_id) {
	return var_vector[id_position_map[var_id]]->o_type_[edge_id];
}

bool BGPQuery::check_already_joined_pre_var(vector<unsigned int> &already_node, unsigned int pre_var_id) {
	for(auto x:already_node){
		auto var_descip = get_vardescrip_by_id(x);
		for(unsigned i = 0; i < var_descip->degree_; ++i){
			if(var_descip->so_edge_nei_type_[i] == VarDescriptor::EntiType::VarEntiType &&
				var_descip->so_edge_pre_type_[i] == VarDescriptor::PreType::VarPreType)
				if(find(already_node.begin(), already_node.end(), var_descip->so_edge_nei_[i]) != already_node.end() &&
					var_descip->so_edge_pre_id_[i] == pre_var_id)
					return true;
		}
	}
	return false;
}

const vector <Triple> &BGPQuery::get_triple_vt() {
	return this->triple_vt;
}

const Triple &BGPQuery::get_triple_by_index(unsigned int index) {
	return this->triple_vt[index];
}


bool BGPQuery::is_var_satellite_by_index(unsigned index) {
	auto var_descrip = var_vector[index];
	return (var_descrip->var_type_ == VarDescriptor::VarType::Entity and
				var_descrip->degree_ == 1 and var_descrip->selected_ == false);// && var_descrip->selected_ == false);
}

/**
 * Print this BGPQuery's info, just for debug, not for user.
 * Use VarDescriptor::print
 * @param kvstore kvstore's pointer
 */
void BGPQuery::print(KVstore *kvstore) {
	cout << (this->distinct_query ? "DISTINCT BGP" : "NOT DISTINCT BGP") << endl;
	cout << "this BGP has " << var_vector.size() << " vars, ie. total_var_num = " << this->total_var_num << ", "
			<< "triples num = " << triple_vt.size() << endl;

	cout << "total so var num = " << total_so_var_num << ", so_var_id.size() = " << so_var_id.size() << endl;
	for(unsigned i = 0; i < so_var_id.size(); ++ i ){
		cout << "var name: " << var_vector[id_position_map[so_var_id[i]]]->var_name_ << endl;
	}
	cout << "total pre vat num = " << total_pre_var_num << ", pre_var_id.size() = " << pre_var_id.size() << endl;
	for(unsigned i = 0; i < pre_var_id.size(); ++ i){
		cout << "var name: " << var_vector[id_position_map[pre_var_id[i]]]->var_name_ << endl;
	}

	cout << "id position map:" << endl;
	for(auto id_pos : id_position_map){
		cout << id_pos.first << " : " << id_pos.second << endl;
	}
	cout << "position id map:" << endl;
	for(auto pos_id : position_id_map){
		cout << pos_id.first << " : " << pos_id.second << endl;
	}

	cout << "item to freq map:" << endl;
	for(const auto& item_freq : item_to_freq){
		cout << item_freq.first << " : " << item_freq.second << endl;
	}

	cout << "item to position map:" << endl;
	for(const auto& item_pos : var_item_to_position){
		cout << item_pos.first << " : " << item_pos.second << endl;
	}

	cout << "item to id map:" << endl;
	for(const auto& item_id : var_item_to_id){
		cout << item_id.first << " : " << item_id.second << endl;
	}

	cout << "s_id, p_id, o_id:" << endl;
	for(unsigned i = 0; i < s_id_.size(); ++i){
		cout << s_id_[i] << (s_is_constant_[i] ? "const" : "var") << "    ";
		cout << p_id_[i] << (p_is_constant_[i] ? "const" : "var") << "    ";
		cout << o_id_[i] << (o_is_constant_[i] ? "const" : "var") << endl;

	}


	cout << endl << "VAR INFO:" << endl;
	for(unsigned i = 0; i < var_vector.size(); ++ i){
		var_vector[i]->print(kvstore);
	}
}


void BGPQuery::set_var_candidate_cache(unsigned var_id, shared_ptr<IDList> candidate_cache) {
	(*var_candidates_cache)[var_id] = std::move(candidate_cache);
}



shared_ptr<IDList> BGPQuery::get_candidate_list_by_id(unsigned var_id) {
	if((*var_candidates_cache).find(var_id) != (*var_candidates_cache).end())
		return nullptr;
	else
		return (*var_candidates_cache)[var_id];
}

shared_ptr<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>> BGPQuery::get_all_candidates() {
	return var_candidates_cache;
}



vector<unsigned*>*
BGPQuery::get_result_list_pointer()
{
  return &(this->result_list);
}

unique_ptr<unsigned[]>&
BGPQuery::resultPositionToId()
{
	return result_position_to_id;
}

std::tuple<bool, bool, bool> BGPQuery::GetOccurPosition(int var_id) {
  bool is_entity = false;
  bool is_literal = false;
  bool is_predicate = false;
  auto var_descriptor = this->get_vardescrip_by_id(var_id);
  if (var_descriptor->var_type_ == VarDescriptor::VarType::Predicate) {
    is_predicate = true;
  }
  else {
    is_entity = true;
    auto var_name = this->get_var_name_by_id(var_id);
    auto edge_ids = var_descriptor->so_edge_index_;
    for (auto edge_id : edge_ids) {
      auto triple = this->get_triple_by_index(edge_id);
      if (var_name == triple.getObject()) {
        is_literal = true;
        break;
      }
    }
  }
  return make_tuple(is_entity,is_literal,is_predicate);
}
