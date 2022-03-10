/*=============================================================================
# Filename: PlanGenerator.cpp
# Author: Linglin Yang
# Mail: linglinyang@stu.pku.edu.cn
=============================================================================*/

// something need clear:
// 1. s ?p ?o. If we use ?p in the future, then s2po is similar to s2o in efficiency, because the result size is same.
// 			   However, we need to re-design the cost function to fit the new added pre_var ?p.

// s ?p ?o
// ?s ?p o do not need generate ?p candidate, even pre num of one s_o is small
// however, ?o ?s's candidate should be consider

// plan_cache only contains plans about s_o var, because in ?s ?p ?o, after considering ?s and ?o, then ?p has also considered.
// in binary join, only consider s_o var

// scan var(candidate set generation) -> add var by WCO_join or binary_join -> add satellite var

// NOTICE:
// random_device rd;
// mt19937 eng(rd());
// may cause SIGILL in valgrind. when encounter this prob, please change it to other sample method

#include "PlanGenerator.h"

const unsigned PlanGenerator::SAMPLE_CACHE_MAX = 50;

PlanGenerator::PlanGenerator(KVstore *kvstore_, BGPQuery *bgpquery_, Statistics *statistics_, IDCachesSharePtr &id_caches_,
							 TYPE_TRIPLE_NUM triples_num_, TYPE_PREDICATE_ID limitID_predicate_,
							 TYPE_ENTITY_LITERAL_ID limitID_literal_, TYPE_ENTITY_LITERAL_ID limitID_entity_,
							 TYPE_TRIPLE_NUM* pre2num_, TYPE_TRIPLE_NUM* pre2sub_, TYPE_TRIPLE_NUM* pre2obj_, shared_ptr<Transaction> txn_):
					kvstore(kvstore_), bgpquery(bgpquery_), statistics(statistics_), id_caches(id_caches_), triples_num(triples_num_),
					limitID_predicate(limitID_predicate_), limitID_literal(limitID_literal_), limitID_entity(limitID_entity_),
					pre2num(pre2num_), pre2sub(pre2sub_), pre2obj(pre2obj_), txn(txn_){};

PlanGenerator::~PlanGenerator() {
	for(auto &map_plan_list : plan_cache){
		for(auto &nodes_plan_list_pair : map_plan_list){
			for(auto x : nodes_plan_list_pair.first) cout << x << " ";
			cout << endl;
			auto plan_ = nodes_plan_list_pair.second.begin();
			while(plan_ != nodes_plan_list_pair.second.end()){
				auto temp = *plan_;
				plan_ = nodes_plan_list_pair.second.erase(plan_);
				delete temp;

			}
			// for(auto &plan : nodes_plan_list_pair.second){
			// 	delete plan;
			// }
		}
	}
}

JoinMethod PlanGenerator::get_join_strategy(bool s_is_var, bool p_is_var, bool o_is_var, unsigned var_num) {

	if(var_num == 2){
		if(!s_is_var){
			if(!bgpquery->get_vardescrip_by_id(bgpquery->pre_var_id[0])->selected_)
				return JoinMethod::s2o;
			else{
				if(!bgpquery->get_vardescrip_by_id(bgpquery->so_var_id[0])->selected_)
					return JoinMethod::s2p;
				else return JoinMethod::s2po;
			}
		}
		if(!o_is_var){
			if(!bgpquery->get_vardescrip_by_id(bgpquery->pre_var_id[0])->selected_)
				return JoinMethod::o2s;
			else{
				if(!bgpquery->get_vardescrip_by_id(bgpquery->so_var_id[0])->selected_)
					return JoinMethod::o2p;
				else return JoinMethod::o2ps;
			}
		}
		if(!p_is_var){
			if(!bgpquery->get_vardescrip_by_id(bgpquery->s_id_[0])->selected_)
				return JoinMethod::p2o;
			else{
				if(!bgpquery->get_vardescrip_by_id(bgpquery->o_id_[0])->selected_)
					return JoinMethod::p2s;
				else return JoinMethod::p2so;
			}
		}
	} else {
		if (var_num == 1) {
			if (s_is_var) return JoinMethod::po2s;
			if (o_is_var) return JoinMethod::sp2o;
			if (p_is_var) return JoinMethod::so2p;
		} else {
			cout << "error: var_num not equal to 1 or 2" << endl;
			exit(-1);
		}
	}
	cout << "error: var_num not equal to 1 or 2" << endl;
	exit(-1);

}


unsigned PlanGenerator::get_sample_size(unsigned id_cache_size){
	if(id_cache_size <= 100){
		if(SAMPLE_CACHE_MAX < id_cache_size)
			return SAMPLE_CACHE_MAX;
		else
			return id_cache_size;
	} else{
		return (unsigned )(log(id_cache_size)*11);
	}
}


////  Copied from Database::exist_triple in Database.cpp
////  If there exist a triple (s_id, p_id, o_id) in db, return true;
////  else, return false
bool PlanGenerator::check_exist_this_triple(TYPE_ENTITY_LITERAL_ID s_id, TYPE_PREDICATE_ID p_id, TYPE_ENTITY_LITERAL_ID o_id){

	unsigned* _objidlist = nullptr;
	unsigned _list_len = 0;
	bool is_exist = false;
	//get exclusive before update!
	if(p_id >= 0)
		kvstore->getobjIDlistBysubIDpreID(s_id, p_id, _objidlist, _list_len, false, this->txn);
	else
		kvstore->getobjIDlistBysubID(s_id, _objidlist, _list_len, false, this->txn);

	if (Util::bsearch_int_uporder(o_id, _objidlist, _list_len) != INVALID){
		is_exist = true;
	}
	delete[] _objidlist;

	return is_exist;
}

void PlanGenerator::insert_edge_selectivity_to_cache(unsigned from_id, unsigned to_id, unsigned linked_num) {

	double selectivity = max(1.0, ((double) linked_num / var_to_sample_cache[from_id].size()));
	// cout << "from id: " << from_id << ", to id: " << to_id << ", linked_num = " << linked_num  <<
	// 	", var_cache_num = " << var_to_sample_cache[from_id].size() << " selectivity: " << selectivity << endl;
	if (s_o_list_average_size.find(from_id) == s_o_list_average_size.end()) {
		map<unsigned, double > this_node_selectivity_map;
		this_node_selectivity_map.insert(
				make_pair(to_id, selectivity));
		s_o_list_average_size.insert(make_pair(from_id, this_node_selectivity_map));
	} else {
		s_o_list_average_size[from_id].insert(
				make_pair(to_id, selectivity));
	}

}

void PlanGenerator::insert_card_estimation_to_cache(const vector<unsigned> &now_plan_nodes,
													long long card_estimation, vector<vector<unsigned>> &result_sample) {
	if (card_cache.size() < now_plan_nodes.size() - 1) {
		map<vector<unsigned>, long long> this_var_num_card_map;
		this_var_num_card_map.insert(make_pair(now_plan_nodes, card_estimation));
		card_cache.push_back(this_var_num_card_map);

		map<vector<unsigned>, vector<vector<unsigned>>> this_var_num_sample_map;
		this_var_num_sample_map.insert(make_pair(now_plan_nodes, result_sample));
		sample_cache.push_back(this_var_num_sample_map);

	} else {
		card_cache[now_plan_nodes.size() - 2].insert(make_pair(now_plan_nodes, card_estimation));
		sample_cache[now_plan_nodes.size() - 2].insert(make_pair(now_plan_nodes, result_sample));
	}
}

long long PlanGenerator::card_estimator_two_nodes(unsigned last_node, unsigned next_join_node, const vector<unsigned> &now_plan_nodes) {

	auto var_descrip = bgpquery->get_vardescrip_by_id(next_join_node);

	if(card_cache.size() == 0 || card_cache[0].find(now_plan_nodes) == card_cache[0].end()){


		// if(!var_sampled_from_candidate[next_join_node]){
		// 	;
		// 	// todo: sample from last_plan
		// }
		bool changed_two_nodes = false;
		if(var_to_num_map[last_node] > var_to_num_map[next_join_node]){
			unsigned tmp = last_node;
			last_node = next_join_node;
			next_join_node = tmp;
			changed_two_nodes = true;
			var_descrip = bgpquery->get_vardescrip_by_id(next_join_node);
		}


		long long card_estimation;
		vector<vector<unsigned>> this_sample;

		unsigned now_sample_num = 0;

		vector<char> edge_type;
		vector<TYPE_PREDICATE_ID> p_list;

		for(unsigned i = 0; i < var_descrip->degree_; ++i){
			if(var_descrip->so_edge_nei_type_[i] == VarDescriptor::EntiType::VarEntiType &&
			var_descrip->so_edge_nei_[i] == last_node){
				edge_type.push_back(var_descrip->so_edge_type_[i]);
				if(var_descrip->so_edge_pre_type_[i] == VarDescriptor::PreType::VarPreType)
					p_list.push_back(-1);
				else
					p_list.push_back(var_descrip->so_edge_pre_id_[i]);
			}
		}

		map<unsigned, unsigned> new_id_pos_map;
		if (last_node < next_join_node) {
			new_id_pos_map[last_node] = 0;
			new_id_pos_map[next_join_node] = 1;
		} else {
			new_id_pos_map[next_join_node] = 0;
			new_id_pos_map[last_node] = 1;
		}

		long long s_o_list1_total_num = 0;
		long long s_o_list2_total_num = 0;

		random_device rd;
		mt19937 eng(rd());
		uniform_real_distribution<double> dis(0.0, 1.0);

		if (edge_type[0] == Util::EDGE_IN) {
			// not need to sample, because sampled in considering all scans
			for (unsigned i = 0; i < var_to_sample_cache[last_node].size(); ++i) {
				unsigned *s_o_list = nullptr;
				unsigned s_o_list_len = 0;

				if(p_list[0] >= 0)
					kvstore->getobjIDlistBysubIDpreID(var_to_sample_cache[last_node][i], p_list[0],
													  s_o_list,s_o_list_len, false, this->txn);
				else
					kvstore->getobjIDlistBysubID(var_to_sample_cache[last_node][i],
												 s_o_list,s_o_list_len, bgpquery->distinct_query, this->txn);

				s_o_list1_total_num += s_o_list_len;

				for (unsigned j = 0; j < s_o_list_len; ++j) {
					if(var_sampled_from_candidate[next_join_node] and
							!binary_search((*id_caches)[next_join_node]->begin(),
								   	(*id_caches)[next_join_node]->end(), s_o_list[j]))
						continue;



					unsigned k = 1;
					for (; k < edge_type.size(); ++k) {
						if (edge_type[k] == Util::EDGE_IN) {
							if (!check_exist_this_triple(var_to_sample_cache[last_node][i],
														 p_list[k], s_o_list[j])) {
								break;
							}
						} else {
							if (!check_exist_this_triple(s_o_list[j], p_list[k],
														 var_to_sample_cache[last_node][i])) {
								break;
							}
						}
					}

					if (k == edge_type.size()) {
						++now_sample_num;
						if (now_sample_num < SAMPLE_CACHE_MAX) {
							vector<unsigned> this_pass_sample(2);
							this_pass_sample[new_id_pos_map[last_node]] = var_to_sample_cache[last_node][i];
							this_pass_sample[new_id_pos_map[next_join_node]] = s_o_list[j];

							this_sample.push_back(move(this_pass_sample));
						} else {
							// if (rand() / double(RAND_MAX) < SAMPLE_PRO) {
							if (dis(eng) < SAMPLE_PRO) {


								vector<unsigned> this_pass_sample(2);
								this_pass_sample[new_id_pos_map[last_node]] = var_to_sample_cache[last_node][i];
								this_pass_sample[new_id_pos_map[next_join_node]] = s_o_list[j];

								this_sample.push_back(move(this_pass_sample));

							}
						}
					}
				}

				delete[] s_o_list;

			}

			for (unsigned i = 0; i < var_to_sample_cache[next_join_node].size(); ++i) {
				unsigned *s_o_list = nullptr;
				unsigned s_o_list_len = 0;

				if(p_list[0] >= 0)
					kvstore->getsubIDlistByobjIDpreID(var_to_sample_cache[next_join_node][i], p_list[0],
													  s_o_list,s_o_list_len, false, this->txn);
				else
					kvstore->getsubIDlistByobjID(var_to_sample_cache[next_join_node][i],
												 s_o_list, s_o_list_len, bgpquery->distinct_query, this->txn);

				s_o_list2_total_num += s_o_list_len;

				delete[] s_o_list;

			}

			insert_edge_selectivity_to_cache(last_node, next_join_node, s_o_list1_total_num);
			insert_edge_selectivity_to_cache(next_join_node, last_node, s_o_list2_total_num);

		} else {

			for (unsigned i = 0; i < var_to_sample_cache[next_join_node].size(); ++i) {
				unsigned *s_o_list = nullptr;
				unsigned s_o_list_len = 0;

				if(p_list[0] >= 0)
					kvstore->getobjIDlistBysubIDpreID(var_to_sample_cache[next_join_node][i], p_list[0],
													  s_o_list,s_o_list_len, false, this->txn);
				else
					kvstore->getobjIDlistBysubID(var_to_sample_cache[next_join_node][i],
												 s_o_list,s_o_list_len, bgpquery->distinct_query, this->txn);

				s_o_list1_total_num += s_o_list_len;

				for (unsigned j = 0; j < s_o_list_len; ++j) {
					if(var_sampled_from_candidate[last_node] and
							!binary_search((*id_caches)[last_node]->begin(),
										   (*id_caches)[last_node]->end(), s_o_list[j]))
						continue;

					unsigned k = 1;
					for (; k < edge_type.size(); ++k) {
						if (edge_type[k] == Util::EDGE_IN) {
							if (!check_exist_this_triple(s_o_list[j], p_list[k],
														 var_to_sample_cache[next_join_node][i])) {
								break;
							}
						} else {
							if (!check_exist_this_triple(var_to_sample_cache[next_join_node][i], p_list[k],
														 s_o_list[j])) {
								break;
							}
						}

					}

					if (k == edge_type.size()) {
						++now_sample_num;
						if (now_sample_num < SAMPLE_CACHE_MAX) {
							vector<unsigned> this_pass_sample(2);
							this_pass_sample[new_id_pos_map[last_node]] = s_o_list[j];
							this_pass_sample[new_id_pos_map[next_join_node]] = var_to_sample_cache[next_join_node][i];

							this_sample.push_back(move(this_pass_sample));
						} else {
							// if (rand() / double(RAND_MAX) < SAMPLE_PRO) {
							if (dis(eng) < SAMPLE_PRO) {

								vector<unsigned> this_pass_sample(2);
								this_pass_sample[new_id_pos_map[last_node]] = s_o_list[j];
								this_pass_sample[new_id_pos_map[next_join_node]] = var_to_sample_cache[next_join_node][i];

								this_sample.push_back(move(this_pass_sample));

							}
						}
					}
				}

				delete[] s_o_list;
			}

			for (unsigned i = 0; i < var_to_sample_cache[last_node].size(); ++i) {
				unsigned *s_o_list = nullptr;
				unsigned s_o_list_len = 0;

				if(p_list[0] >= 0)
					kvstore->getsubIDlistByobjIDpreID(var_to_sample_cache[last_node][i], p_list[0],
													  s_o_list,s_o_list_len, false, this->txn);
				else
					kvstore->getsubIDlistByobjID(var_to_sample_cache[last_node][i],
												 s_o_list, s_o_list_len, bgpquery->distinct_query, this->txn);

				s_o_list2_total_num += s_o_list_len;

				delete[] s_o_list;


			}

			insert_edge_selectivity_to_cache(next_join_node, last_node, s_o_list1_total_num);
			insert_edge_selectivity_to_cache(last_node, next_join_node, s_o_list2_total_num);

		}

		card_estimation = max((edge_type[0] == Util::EDGE_IN) ?
				(long long) ((double) (now_sample_num * var_to_num_map[last_node]) / var_to_sample_cache[last_node].size()) :
				(long long) ((double) (now_sample_num * var_to_num_map[next_join_node]) / var_to_sample_cache[next_join_node].size() )
					, (long long) 1);

		insert_card_estimation_to_cache(now_plan_nodes, card_estimation, this_sample);

		if(changed_two_nodes)
			return var_to_num_map[next_join_node]*s_o_list_average_size[next_join_node][last_node];
		else
			return var_to_num_map[last_node]*s_o_list_average_size[last_node][next_join_node];

	}  else{
		return var_to_num_map[last_node]*s_o_list_average_size[last_node][next_join_node];//+var_to_num_map[next_join_node];
	}
}


long long PlanGenerator::card_estimator_more_than_three_nodes(const vector<unsigned> &last_plan_nodes, unsigned next_join_node, const vector<unsigned> &now_plan_nodes) {

	unsigned last_plan_nodes_num = last_plan_nodes.size();
	auto var_descrip = bgpquery->get_vardescrip_by_id(next_join_node);

	if (card_cache.size() < last_plan_nodes_num ||
			card_cache[last_plan_nodes_num - 1].find(now_plan_nodes) == card_cache[last_plan_nodes_num - 1].end()) {

		long long card_estimation;
		long long last_card_estimation = card_cache[last_plan_nodes_num - 2][last_plan_nodes];

		vector<vector<unsigned>> this_sample;
		vector<vector<unsigned>> &last_sample = sample_cache[last_plan_nodes_num - 2][last_plan_nodes];

		if (last_sample.size() != 0) {
			// cout << "card estimation from " ;
			// for(auto x : last_plan_nodes) cout << x << " ";
			// cout << "to " << next_join_node;

			unsigned now_sample_num = 0;

			vector<unsigned> linked_nei_pos;
			vector<char> edge_type;
			vector<TYPE_PREDICATE_ID> p_list;


			for(unsigned i = 0; i < var_descrip->degree_; ++i){
				if(var_descrip->so_edge_nei_type_[i] == VarDescriptor::EntiType::VarEntiType){

					for(unsigned j = 0; j < last_plan_nodes.size(); ++j){
						if(var_descrip->so_edge_nei_[i] == last_plan_nodes[j]){
							linked_nei_pos.emplace_back(j);
							edge_type.emplace_back(var_descrip->so_edge_type_[i]);
							if(var_descrip->so_edge_pre_type_[i] == VarDescriptor::PreType::VarPreType)
								p_list.push_back(-1);
							else
								p_list.push_back(var_descrip->so_edge_pre_id_[i]);
						}
					}
				}
			}

			map<unsigned , unsigned > new_id_pos_map;
			for (unsigned i = 0, index = 0, used = 0; i <= last_plan_nodes_num; ++i) {
				if ((index == last_plan_nodes_num) || (!used && next_join_node < last_plan_nodes[index])) {
					new_id_pos_map[next_join_node] = i;
					used = 1;
				} else {
					new_id_pos_map[last_plan_nodes[index]] = i;
					++index;
				}
			}

			random_device rd;
			mt19937 eng(rd());
			uniform_real_distribution<double> dis(0.0, 1.0);

			if (edge_type[0] == Util::EDGE_IN) {
				for (unsigned i = 0; i < last_sample.size(); ++i) {
					unsigned *s_o_list = nullptr;
					unsigned s_o_list_len = 0;

					if(p_list[0] >= 0 )
						kvstore->getobjIDlistBysubIDpreID(last_sample[i][linked_nei_pos[0]], p_list[0],
														  s_o_list,s_o_list_len, false, this->txn);
					else
						kvstore->getobjIDlistBysubID(last_sample[i][linked_nei_pos[0]],
													 s_o_list,s_o_list_len, bgpquery->distinct_query, this->txn);

					for (unsigned j = 0; j < s_o_list_len; ++j) {
						if(var_sampled_from_candidate[next_join_node] and
								!binary_search((*id_caches)[next_join_node]->begin(),
									   (*id_caches)[next_join_node]->end(), s_o_list[j]))
							continue;


						unsigned k = 1;
						for (; k < edge_type.size(); ++k) {
							if (edge_type[k] == Util::EDGE_IN) {
								if (!check_exist_this_triple(last_sample[i][linked_nei_pos[k]], p_list[k],
															 s_o_list[j])) {
									break;
								}
							} else {
								if (!check_exist_this_triple(s_o_list[j], p_list[k],
															 last_sample[i][linked_nei_pos[k]])) {
									break;
								}
							}

						}

						if (k == edge_type.size()) {
							++now_sample_num;
							if (now_sample_num < SAMPLE_CACHE_MAX) {
								vector<unsigned> this_pass_sample(last_plan_nodes_num+1);
								for (unsigned l = 0; l < last_plan_nodes_num; ++l) {
									this_pass_sample[new_id_pos_map[last_plan_nodes[l]]] = last_sample[i][l];
								}
								this_pass_sample[new_id_pos_map[next_join_node]] = s_o_list[j];

								this_sample.push_back(move(this_pass_sample));
							} else {

								// if (rand() % 1000 < SAMPLE_CACHE_MAX) {
								if (dis(eng) < SAMPLE_PRO) {

									vector<unsigned> this_pass_sample(last_plan_nodes_num+1);
									for (unsigned l = 0; l < last_plan_nodes_num; ++l) {
										this_pass_sample[new_id_pos_map[last_plan_nodes[l]]] = last_sample[i][l];
									}
									this_pass_sample[new_id_pos_map[next_join_node]] = s_o_list[j];

									this_sample.push_back(move(this_pass_sample));
								}
							}
						}

					}

					delete[] s_o_list;

				}
			} else {

				for (unsigned i = 0; i < last_sample.size(); ++i) {
					unsigned *s_o_list = nullptr;
					unsigned s_o_list_len = 0;

					if(p_list[0] >= 0)
						kvstore->getsubIDlistByobjIDpreID(last_sample[i][linked_nei_pos[0]], p_list[0],
														  s_o_list,s_o_list_len, false, this->txn);
					else
						kvstore->getsubIDlistByobjID(last_sample[i][linked_nei_pos[0]],
													 s_o_list,s_o_list_len, bgpquery->distinct_query, this->txn);

					for (unsigned j = 0; j < s_o_list_len; ++j) {
						if(var_sampled_from_candidate[next_join_node] and
								!binary_search((*id_caches)[next_join_node]->begin(),
											   (*id_caches)[next_join_node]->end(), s_o_list[j]))
							continue;

						unsigned k = 1;
						for (; k < edge_type.size(); ++k) {
							if (edge_type[k] == Util::EDGE_IN) {
								if (!check_exist_this_triple(last_sample[i][linked_nei_pos[k]], p_list[k],
															 s_o_list[j])) {
									break;
								}
							} else {
								if (!check_exist_this_triple(s_o_list[j], p_list[k],
															 last_sample[i][linked_nei_pos[k]])) {
									break;
								}
							}

						}

						if (k == edge_type.size()) {
							++now_sample_num;
							if (now_sample_num < SAMPLE_CACHE_MAX) {
								vector<unsigned> this_pass_sample(last_plan_nodes_num+1);
								for (unsigned l = 0; l < last_plan_nodes_num; ++l) {
									this_pass_sample[new_id_pos_map[last_plan_nodes[l]]] = last_sample[i][l];
								}
								this_pass_sample[new_id_pos_map[next_join_node]] = s_o_list[j];

								this_sample.push_back(move(this_pass_sample));
							} else {

								// if (rand() % 1000 < SAMPLE_CACHE_MAX) {
								if (dis(eng) < SAMPLE_PRO) {

									vector<unsigned> this_pass_sample(last_plan_nodes_num+1);
									for (unsigned l = 0; l < last_plan_nodes_num; ++l) {
										this_pass_sample[new_id_pos_map[last_plan_nodes[l]]] = last_sample[i][l];
									}
									this_pass_sample[new_id_pos_map[next_join_node]] = s_o_list[j];

									this_sample.push_back(move(this_pass_sample));
								}
							}
						}

					}

					delete[] s_o_list;

				}

			}

			// cout << " pass num: " << now_sample_num << endl;

			card_estimation = max(
					(long long) ((double) (now_sample_num * last_card_estimation ) / last_sample.size() ),
					(long long) 1);

		} else {
			card_estimation = 1;
		}

		insert_card_estimation_to_cache(now_plan_nodes, card_estimation, this_sample);

		vector<unsigned> linked_nei_id;
		for(unsigned i = 0; i < var_descrip->degree_; ++i){
			if(var_descrip->so_edge_nei_type_[i] == VarDescriptor::EntiType::VarEntiType and
					find(last_plan_nodes.begin(), last_plan_nodes.end(), var_descrip->so_edge_nei_[i])
						!= last_plan_nodes.end())
				linked_nei_id.push_back(var_descrip->so_edge_nei_[i]);

		}

		double multiple = s_o_list_average_size[linked_nei_id[0]][next_join_node];
		for(auto x : linked_nei_id){
			multiple = min(multiple, s_o_list_average_size[x][next_join_node]);
			//					multiple += s_o_list_average_size[x][next_join_node];

		}
		// cout << "    card esti: " << card_estimation << endl;
		return card_cache[last_plan_nodes_num-2][last_plan_nodes]*multiple;

	} else{

		vector<unsigned> linked_nei_id;
		for(unsigned i = 0; i < var_descrip->degree_; ++i){
			if(var_descrip->so_edge_nei_type_[i] == VarDescriptor::EntiType::VarEntiType and
					find(last_plan_nodes.begin(), last_plan_nodes.end(), var_descrip->so_edge_nei_[i])
						!= last_plan_nodes.end())
				linked_nei_id.push_back(var_descrip->so_edge_nei_[i]);

		}
		double multiple = s_o_list_average_size[linked_nei_id[0]][next_join_node];
		for(auto x : linked_nei_id){
			multiple = min(multiple, s_o_list_average_size[x][next_join_node]);
		}

		return card_cache[last_plan_nodes_num-2][last_plan_nodes]*multiple;//+var_to_num_map[next_join_node];
	}
}


long long PlanGenerator::card_estimator(const vector<unsigned> &last_plan_nodes, unsigned next_join_node, const vector<unsigned> &now_plan_nodes) {

	unsigned last_plan_nodes_num = last_plan_nodes.size();

	if(last_plan_nodes_num == 1)
		return card_estimator_two_nodes(last_plan_nodes[0], next_join_node, now_plan_nodes);
	else
		return card_estimator_more_than_three_nodes(last_plan_nodes, next_join_node, now_plan_nodes);

	// return 1;
}

long long PlanGenerator::get_card(const vector<unsigned> &nodes){
	return card_cache[nodes.size()-2][nodes];
}

long long PlanGenerator::cost_model_for_wco(PlanTree *last_plan, const vector<unsigned int> &last_plan_node,
															unsigned int next_node, const vector<unsigned int> &now_plan_node) {
	return last_plan->plan_cost + card_estimator(last_plan_node, next_node, now_plan_node);
}

long long PlanGenerator::cost_model_for_binary(const vector<unsigned> &plan_a_nodes, const vector<unsigned> &plan_b_nodes,
											   PlanTree* plan_a, PlanTree* plan_b){

	long long plan_a_card = get_card(plan_a_nodes);
	long long plan_b_card = get_card(plan_b_nodes);
	long long min_card = min(plan_a_card, plan_b_card);
	long long max_card = max(plan_a_card, plan_b_card);

	return min_card + 2*max_card + plan_a->plan_cost + plan_b->plan_cost;
}


PlanTree* PlanGenerator::get_best_plan(const vector<unsigned> &nodes){

	PlanTree* best_plan = nullptr;
	long long min_cost = LLONG_MAX;


	for(const auto &plan : plan_cache[nodes.size()-1][nodes]){
		if(plan->plan_cost < min_cost){
			best_plan = plan;
			min_cost = plan->plan_cost;
		}
	}

	return best_plan;
}

// only used when get best plan for all nodes in basicquery
PlanTree* PlanGenerator::get_best_plan_by_num(int total_var_num){

	PlanTree* best_plan = nullptr;
	long long min_cost = LLONG_MAX	;


	//	for(int i =0;i < plan_cache.size();++i){
	//		for(auto x:plan_cache[i]){
	//			for(int j = 0;j<x.first.size();++j){
	//				cout << x.first[j]<< " ";
	//			}
	//			cout <<endl;
	//			for(auto y:x.second){
	//				y->print(basicquery);
	//			}
	//		}
	//	}

	int count = 0;
	for(const auto &nodes_plan : plan_cache[total_var_num - 1]){
		for(const auto &plan_tree : nodes_plan.second){
			//    	plan_tree->print(basicquery);
			count ++;
			if(plan_tree->plan_cost < min_cost){
				best_plan = plan_tree;
				min_cost = plan_tree->plan_cost;
			}
		}
	}
	cout << "during enumerate plans, get " << count << " possible best plans." << endl;
	return best_plan;
}


JoinMethod PlanGenerator::get_join_method(bool s_const, bool p_const, bool o_const, VarDescriptor::ItemType item_type) {
	switch (item_type) {
		case VarDescriptor::ItemType::SubType:
			if(p_const)
				if(o_const) return JoinMethod::po2s;
				else return JoinMethod::p2s;
			else
				if(o_const) return JoinMethod::o2s;
				else{
					cout << "error! error when try to generate candidates from two vars in one triple!" << endl;
					exit(-1);
				}
		case VarDescriptor::ItemType::ObjType:
			if(p_const)
				if(s_const) return JoinMethod::sp2o;
				else return JoinMethod::p2o;
			else
				if(s_const) return JoinMethod::s2o;
				else{
					cout << "error! error when try to generate candidates from two vars in one triple!" << endl;
					exit(-1);
				}
		case VarDescriptor::ItemType::PreType:
			if(s_const)
				if(o_const) return JoinMethod::so2p;
				else return JoinMethod::s2p;
			else
				if(o_const) return JoinMethod::o2p;
				else{
					cout << "error! error when try to generate candidates from two vars in one triple!" << endl;
					exit(-1);
				}
		default:{
			cout << "error! error when try to get join method in PlanGenerator::get_join_method! " << endl;
			exit(-1);
		}
	}
}


/**
 * Generate var candidates_generation plan.
 * Edges below will be considered in this function:
 * 1. Triple with only one var: ?s p o, s ?p o, s p ?o.
 * 	We will tell the executor to generate candidate for the var from the other two const.
 * 2. Triple with two var and one const.
 * 	We will generate candidate for the var from the const.
 * 	Todo: generate all items from the db when the triples include the var is like ?s ?p ?o.
 */
void PlanGenerator::get_candidate_generate_plan() {
	// include all posible vars candidate_generate_plan, refer to Vardescriptor.link_with_const.
	// If it is true, then this function will generate plan for getting its candidates,
	// else, candidate_generate_plan_vec[i] will be nullptr
	// need_join_var_index contains var index which will generate candidates,
	// ie. which position in candidate_generate_plan_vec is not nullptr
	vector<shared_ptr<StepOperation>> candidate_generate_plan_vec;
	vector<unsigned> need_join_var_index;

	for(unsigned var_index = 0; var_index < bgpquery->get_total_var_num(); ++var_index){
		if(!bgpquery->is_var_satellite_by_index(var_index)){
			// if this var is not select and degree is one,
			// then we will not generate candidates of it,
			// instead, we will check edge after all other vars have joined

			need_join_var_index.push_back(var_index);
			auto var_descrip = bgpquery->get_vardescrip_by_index(var_index);

			auto edge_info = make_shared<vector<EdgeInfo>>();
			auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();


			if(var_descrip->var_type_ == VarDescriptor::VarType::Entity){
				for(unsigned i_th_edge = 0; i_th_edge < var_descrip->degree_; ++i_th_edge){

					unsigned triple_index = var_descrip->so_edge_index_[i_th_edge];
					// todo: get join method by s_const, p_const, o_const
					bool s_const = bgpquery->s_is_constant_[triple_index];
					bool p_const = bgpquery->p_is_constant_[triple_index];
					bool o_const = bgpquery->o_is_constant_[triple_index];

					// todo: add so_var_item_type to bgpquery.build()
					if(s_const || p_const || o_const){
						// at least have one const
						JoinMethod join_method = get_join_method(s_const, p_const, o_const, var_descrip->so_var_item_type[i_th_edge]);
						edge_info->emplace_back(bgpquery->s_id_[triple_index], bgpquery->p_id_[triple_index], bgpquery->o_id_[triple_index], join_method);
						edge_constant_info->emplace_back(s_const, p_const, o_const);
					}

				}
			} else{
				// pre var type
				for(unsigned i_th_edge = 0; i_th_edge < var_descrip->degree_; ++i_th_edge){

					unsigned triple_index = var_descrip->pre_edge_index_[i_th_edge];
					// todo: get join method by s_const, p_const, o_const
					bool s_const = bgpquery->s_is_constant_[triple_index];
					bool p_const = bgpquery->p_is_constant_[triple_index];
					bool o_const = bgpquery->o_is_constant_[triple_index];

					// todo: add so_var_item_type to bgpquery.build()
					if(s_const || p_const || o_const){
						// at least have one const
						JoinMethod join_method = get_join_method(s_const, p_const, o_const, VarDescriptor::ItemType::PreType);
						edge_info->emplace_back(bgpquery->s_id_[triple_index], bgpquery->p_id_[triple_index], bgpquery->o_id_[triple_index], join_method);
						edge_constant_info->emplace_back(s_const, p_const, o_const);
					}

				}
			}

			auto candidate_node = make_shared<FeedOneNode>(var_descrip->id_, edge_info, edge_constant_info);

			candidate_generate_plan_vec.emplace_back(make_shared<StepOperation>(StepOperation::JoinType::GenerateCandidates, nullptr, nullptr, nullptr, candidate_node));

		} else
			candidate_generate_plan_vec.push_back(nullptr);
	}
}


void PlanGenerator::get_join_nodes(const vector<unsigned int> &plan_a_nodes,
								   vector<unsigned int> &other_nodes, set<unsigned int> &join_nodes) {
	for(unsigned node : other_nodes){
		auto var_descrip = bgpquery->get_vardescrip_by_id(node);
		for(unsigned i = 0; i < var_descrip->degree_; ++i){
			if(var_descrip->so_edge_nei_type_[i] == VarDescriptor::EntiType::VarEntiType and
					find(plan_a_nodes.begin(), plan_a_nodes.end(), var_descrip->so_edge_nei_[i]) != plan_a_nodes.end()){
				join_nodes.insert(var_descrip->so_edge_nei_[i]);
			}
		}
	}
}

void PlanGenerator::insert_this_plan_to_cache(PlanTree *new_plan, const vector<unsigned> &new_node_vec, unsigned var_num) {
	if(plan_cache.size() < var_num){
		map<vector<unsigned>, list<PlanTree*>> this_num_node_plan;
		list<PlanTree*> this_nodes_plan;
		this_nodes_plan.push_back(new_plan);
		this_num_node_plan.insert(make_pair(new_node_vec, this_nodes_plan));
		plan_cache.push_back(this_num_node_plan);
	} else{
		if(plan_cache[var_num-1].find(new_node_vec) == plan_cache[var_num-1].end()){
			list<PlanTree*> this_nodes_plan;
			this_nodes_plan.push_back(new_plan);
			plan_cache[var_num-1].insert(make_pair(new_node_vec, this_nodes_plan));
		}else{
			plan_cache[var_num-1][new_node_vec].push_back(new_plan);
		}
	}
}


// todo: 这一步先不抽样，在两个点Join的时候生成侯选集
unsigned PlanGenerator::get_sample_from_whole_database(unsigned var_id, vector<unsigned> &so_sample_cache){

	random_device rd;
	mt19937 eng(rd());

	auto var_descrip = bgpquery->get_vardescrip_by_id(var_id);

	bool not_literal = false;
	for(unsigned i = 0; i < var_descrip->degree_; ++i){
		if(var_descrip->so_edge_type_[i] == Util::EDGE_OUT){
			not_literal = true;
			break;
		}
	}

	unsigned sample_from_num = not_literal ? limitID_entity : limitID_entity + limitID_literal;


	unsigned sample_size = get_sample_size(sample_from_num);
	unsigned sample_entity_size = not_literal ? sample_size : ((double )sample_size * limitID_entity / (limitID_literal + limitID_entity) + 1);
	unsigned sample_literal_size = not_literal ? 0 : (sample_size * limitID_literal / (limitID_literal + limitID_entity) + 1);
	sample_entity_size = limitID_entity == 0 ? 0 : sample_entity_size;
	sample_literal_size = limitID_literal == 0 ? 0 : sample_literal_size;
	so_sample_cache.reserve(sample_entity_size + sample_literal_size);

	unsigned already_sampled_num = 0;
	uniform_int_distribution<unsigned> dis(0, limitID_entity-1);
	while (already_sampled_num < sample_entity_size){
		// unsigned index_need_insert = rand() % limitID_entity;
		unsigned index_need_insert = dis(eng);
		auto entity_str = kvstore->getEntityByID(index_need_insert);
		if(entity_str != ""){
			so_sample_cache.emplace_back(index_need_insert);
			++already_sampled_num;
		}
	}


	already_sampled_num = 0;
	dis = uniform_int_distribution<unsigned>(0, limitID_literal-1);
	while (already_sampled_num < sample_literal_size){
		// unsigned index_need_insert = rand() % limitID_literal + Util::LITERAL_FIRST_ID;
		unsigned index_need_insert = dis(eng) + Util::LITERAL_FIRST_ID;
		auto literal_str = kvstore->getLiteralByID(index_need_insert);
		if(literal_str != ""){
			so_sample_cache.emplace_back(index_need_insert);
			++already_sampled_num;
		}
	}

	return sample_from_num;

}


long long PlanGenerator::cost_model_for_p2so_optimization(unsigned node_1_id, unsigned node_2_id) {
	auto var1_descrip = bgpquery->get_vardescrip_by_id(node_1_id);
	auto var2_descrip = bgpquery->get_vardescrip_by_id(node_2_id);

	// unsigned linked_edge_count = 0;
	vector<unsigned> linked_edge_pre_const_index;
	// vector<unsigned> linked_edge_pre_var_index;
	for(unsigned edge_index = 0; edge_index < var1_descrip->degree_; ++edge_index){
		if(var1_descrip->so_edge_nei_type_[edge_index] == VarDescriptor::EntiType::VarEntiType and
				var1_descrip->so_edge_nei_[edge_index] == node_2_id){
			if(var1_descrip->so_edge_pre_type_[edge_index] == VarDescriptor::PreType::ConPreType)
				linked_edge_pre_const_index.push_back(edge_index);
			// else
			// 	linked_edge_pre_var_index.push_back(edge_index);
		}
	}
	bool both_not_linked_const = (var_sampled_from_candidate[node_1_id] && var_sampled_from_candidate[node_2_id]);

	if(!linked_edge_pre_const_index.empty()){
		// cout << "linked pre size = " << kvstore->getPreListSize(var1_descrip->so_edge_pre_id_[linked_edge_pre_const_index[0]]) << endl;
		// cout << "guess pre size = " << (kvstore->getPreListSize(var1_descrip->so_edge_pre_id_[linked_edge_pre_const_index[0]])-4)/8 << endl;
		// cout << "my_fun return pre list size = " << kvstore->getSubObjListLenthByPre(var1_descrip->so_edge_pre_id_[linked_edge_pre_const_index[0]]) << endl;

		// unsigned *s_o_list = nullptr;
		// unsigned s_o_list_len = 0;
		//
		//
		// kvstore->getsubIDobjIDlistBypreID(var1_descrip->so_edge_pre_id_[linked_edge_pre_const_index[0]],s_o_list,s_o_list_len);
		// cout << "true size = " << s_o_list_len << endl;
		// delete[] s_o_list;

		return max((unsigned long long)1,(unsigned long long)(pre2num[var1_descrip->so_edge_pre_id_[linked_edge_pre_const_index[0]]]/(both_not_linked_const ? 1.0 : 0.5)));
	} else{
		return max((unsigned long long)1, (unsigned long long)(triples_num/(both_not_linked_const ? 1.0 : 0.5)));
	}


}

vector<shared_ptr<FeedOneNode>> PlanGenerator::completecandidate(){
	vector<shared_ptr<FeedOneNode>> need_candidate;
	for(unsigned var_index = 0 ; var_index < bgpquery->get_total_var_num(); ++ var_index) {
		if(bgpquery->is_var_satellite_by_index(var_index)){
			// cout << "var_id: " << bgpquery->get_vardescrip_by_index(var_index)->var_name_ << "  satellite" << endl;
			// satellite_nodes.push_back(bgpquery->get_var_id_by_index(var_index));
			continue;
		}

		auto var_descrip = bgpquery->get_vardescrip_by_index(var_index);
		unsigned var_id = var_descrip->id_;
		cout << "consider var id = " << var_id << endl;

		if(var_descrip->var_type_ == VarDescriptor::VarType::Predicate){
			continue;
		}

		bool no_candidate =  (*id_caches).find(var_id) == (*id_caches).end();
		auto candidate_edge_info = make_shared<vector<EdgeInfo>>();
		auto candidate_edge_const_info = make_shared<vector<EdgeConstantInfo>>();

		for(unsigned i = 0; i < var_descrip->degree_; ++i){
			bool pre_const = var_descrip->so_edge_pre_type_[i] == VarDescriptor::PreType::ConPreType;
			bool nei_var = var_descrip->so_edge_nei_type_[i] == VarDescriptor::EntiType::VarEntiType;
			unsigned size = (no_candidate ? limitID_entity + limitID_literal : max(((*id_caches)[var_id]->size()), (unsigned)2));

			if(pre_const && nei_var){
				cout << "size = " << size << endl;
				cout << "pre2num = " << pre2num[var_descrip->so_edge_pre_id_[i]] << endl;
				cout << "border = " << size / (Util::logarithm(2, size) + 1) << endl;

				double border = size / (Util::logarithm(2, size) + 1);
				if((no_candidate) || (double)(pre2num[var_descrip->so_edge_pre_id_[i]]) < border)
				{
					unsigned estimate_num = min((unsigned)10000000, size);
					auto pre_id = var_descrip->so_edge_pre_id_[i];
					cout << "add candidate for var " << var_id << endl;
					if((var_descrip->so_edge_type_[i] == Util::EDGE_IN && pre2obj[pre_id] > estimate_num) ||
					   (var_descrip->so_edge_type_[i] == Util::EDGE_OUT && pre2sub[pre_id] > estimate_num)){
						cout << "skip the prefilter because the constant filter is strong enough" << endl;
						continue;
					}
					cout << "need this prefilter!" << endl;
					if(bgpquery->is_var_satellite_by_index(bgpquery->id_position_map[var_descrip->so_edge_nei_[i]])){
						already_done_satellite.insert(var_descrip->so_edge_nei_[i]);
					}
					unsigned triple_index = var_descrip->so_edge_index_[i];
					candidate_edge_info->emplace_back(bgpquery->s_id_[triple_index], bgpquery->p_id_[triple_index], bgpquery->o_id_[triple_index],
													  (var_descrip->so_edge_type_[i] == Util::EDGE_IN ? JoinMethod::p2o : JoinMethod::p2s));
					candidate_edge_const_info->emplace_back(bgpquery->s_is_constant_[triple_index], bgpquery->p_is_constant_[triple_index],bgpquery->o_is_constant_[triple_index]);

				}


			}

		}

		cout << "candidate edge info for var " << var_id << " is " << (*candidate_edge_info).size() << endl;
		if(!candidate_edge_info->empty()){
			need_candidate.emplace_back(make_shared<FeedOneNode>(var_id, candidate_edge_info, candidate_edge_const_info));
		}

	}
	return need_candidate;
}


/**
 * Generate sample set of every var.
 * If a var has no const linked to it, then sample from the whole database.
 * Todo: SO_type var sample in so_var_to_sample_cache; pre_type var sample in pre_var_to_sample_cache.
 * Todo: should tell me whether the query only contain pre_var.
 */
void PlanGenerator::considervarscan() {

	// random_device rd;
	// mt19937 eng(rd());

	// todo: directly use BGPQuery::so_var_id
	for(unsigned var_index = 0 ; var_index < bgpquery->get_total_var_num(); ++ var_index) {

		if(bgpquery->is_var_satellite_by_index(var_index)){
			cout << "var_id: " << bgpquery->get_vardescrip_by_index(var_index)->var_name_ << "  satellite" << endl;
			satellite_nodes.push_back(bgpquery->get_var_id_by_index(var_index));
			continue;
		}

		auto var_descrip = bgpquery->get_vardescrip_by_index(var_index);
		unsigned var_id = var_descrip->id_;

		if(var_descrip->var_type_ == VarDescriptor::VarType::Predicate){
			continue;
		}
		if(var_descrip->var_type_ == VarDescriptor::VarType::Entity)
			join_nodes.push_back(var_id);

		vector<unsigned> this_node{var_id};
		vector<unsigned> satellite_edge_index;
		unsigned estimate = triples_num;
		PlanTree *new_scan;// = new PlanTree(var_id, bgpquery);
		bool no_candidate =  (*id_caches).find(var_id) == (*id_caches).end();
		auto candidate_edge_info = make_shared<vector<EdgeInfo>>();
		auto candidate_edge_const_info = make_shared<vector<EdgeConstantInfo>>();
		vector<unsigned> nei_id_vec;

		for(unsigned i = 0; i < var_descrip->degree_; ++i){
			bool pre_const = var_descrip->so_edge_pre_type_[i] == VarDescriptor::PreType::ConPreType;
			bool nei_var = var_descrip->so_edge_nei_type_[i] == VarDescriptor::EntiType::VarEntiType;
			unsigned size = (no_candidate ? limitID_entity + limitID_literal : ((*id_caches)[var_id]->size()));

			if(pre_const && nei_var && already_done_satellite.count(var_descrip->so_edge_nei_[i])){
				cout << "size = " << size << endl;
				cout << "pre2num = " << pre2num[var_descrip->so_edge_pre_id_[i]] << endl;
				cout << "border = " << size / (Util::logarithm(2, size) + 1) << endl;

				estimate = min(estimate, (unsigned)pre2num[var_descrip->so_edge_pre_id_[i]]);
				cout << "need this prefilter!" << endl;
				already_done_satellite.insert(var_descrip->so_edge_pre_id_[i]);
				satellite_edge_index.emplace_back(i);
				unsigned triple_index = var_descrip->so_edge_index_[i];
				candidate_edge_info->emplace_back(bgpquery->s_id_[triple_index], bgpquery->p_id_[triple_index], bgpquery->o_id_[triple_index],
												  (var_descrip->so_edge_type_[i] == Util::EDGE_IN ? JoinMethod::p2s : JoinMethod::p2o));
				candidate_edge_const_info->emplace_back(bgpquery->s_is_constant_[triple_index], bgpquery->p_is_constant_[triple_index],bgpquery->o_is_constant_[triple_index]);
				nei_id_vec.emplace_back(var_descrip->so_edge_nei_[i]);
			}


		}


		new_scan = new PlanTree(var_id, bgpquery, satellite_edge_index, candidate_edge_info, candidate_edge_const_info, nei_id_vec);

		list<PlanTree *> this_node_plan;
		this_node_plan.push_back(new_scan);

		// insert to plan_cache, todo: insert subtract as a function
		if (plan_cache.size() < 1) {
			map<vector<unsigned>, list<PlanTree *>> one_node_plan_map;
			one_node_plan_map.insert(make_pair(this_node, this_node_plan));
			plan_cache.push_back(one_node_plan_map);
		} else {
			plan_cache[0].insert(make_pair(this_node, this_node_plan));
		}

		// sample for id_cache

		vector<unsigned> need_insert_vec;

		if((*id_caches).find(var_id) != (*id_caches).end()) {

			get_idcache_sample((*id_caches)[var_id], need_insert_vec);
			var_to_num_map[var_id] = min((*id_caches)[var_id]->size(), estimate);
			var_sampled_from_candidate[var_id] = true;

		} else{
			var_to_num_map[var_id] = min(get_sample_from_whole_database(var_id, need_insert_vec), estimate);
			var_sampled_from_candidate[var_id] = false;

		}
		var_to_sample_cache[var_id] = std::move(need_insert_vec);

		new_scan->plan_cost = max((unsigned)1, var_to_num_map[var_id]/2);


	}

}


// I think this fun has completed, I only need to deal with s_o_var
void PlanGenerator::get_nei_by_sub_plan_nodes(const vector<unsigned int> &last_plan_node, set<unsigned int> &nei_node) {
	for(unsigned node_in_plan : last_plan_node){
		auto var_descrip = bgpquery->get_vardescrip_by_id(node_in_plan);
		for(unsigned i = 0; i < var_descrip->degree_; ++i){
			if(var_descrip->so_edge_nei_type_[i] == VarDescriptor::EntiType::ConEntiType)
				continue;
			// satellite_node, not need to add to join_plan
			if(find(join_nodes.begin(), join_nodes.end(), var_descrip->so_edge_nei_[i]) == join_nodes.end())
				continue;
			if(find(last_plan_node.begin(), last_plan_node.end(), var_descrip->so_edge_nei_[i]) == last_plan_node.end())
				nei_node.insert(var_descrip->so_edge_nei_[i]);
		}
	}
}

// add one node, the added node need to be linked by nodes in plan before
void PlanGenerator::considerwcojoin(unsigned int var_num) {
	auto plan_tree_list = plan_cache[var_num - 2];
	for(const auto &last_node_plan : plan_tree_list){
		// if(last_node_plan.first == vector<unsigned>{0}) continue;
		set<unsigned> nei_node;

		get_nei_by_sub_plan_nodes(last_node_plan.first, nei_node);

		PlanTree* last_best_plan = get_best_plan(last_node_plan.first);

		for(unsigned next_node : nei_node) {

			vector<unsigned> new_node_vec(last_node_plan.first);
			new_node_vec.push_back(next_node);
			sort(new_node_vec.begin(), new_node_vec.end());

			PlanTree* new_plan = new PlanTree(last_best_plan, bgpquery, next_node);
			long long cost = cost_model_for_wco(last_best_plan, last_node_plan.first,
													next_node, new_node_vec);
			new_plan->plan_cost = cost;

			// for(auto x:last_node_plan.first) cout << x << " ";
			// cout << "to node " << next_node << " , cost: " << cost << endl;
			if(var_num == 2){
				long long this_cost = cost_model_for_p2so_optimization(last_node_plan.first[0], next_node);
				cout << "in wcojoin, " << last_node_plan.first[0] << " to " << next_node << endl;
				cout << "\t" << "normal cost: " << cost << ", p2so cost: " << this_cost << endl;
				if(this_cost < cost){
					delete new_plan;
					new_plan = new PlanTree(last_node_plan.first[0], next_node, bgpquery);
					new_plan->plan_cost = this_cost;
				}
			}


			insert_this_plan_to_cache(new_plan, new_node_vec, var_num);

		}
	}
}

// todo: should consider the condition when using this carefully
// not add nodes, but to consider if binaryjoin could decrease cost
void PlanGenerator::considerbinaryjoin(unsigned int var_num)  {

	unsigned min_size = 3;
	unsigned max_size = min(min_size, var_num - 2);


	for(const auto &need_considerbinaryjoin_nodes_plan : plan_cache[var_num - 1]){

		long long last_plan_smallest_cost = get_best_plan(need_considerbinaryjoin_nodes_plan.first)->plan_cost;

		// for example. Input query: ?s1->?p1->?o1, ?s2->?p1->?o2, ?s1->?p3->?o2.
		// we have plan1 of ?s1->?p1->?o1, plan2 of ?s2->?p1->?o2.
		// but we need to check ?s1->?p3->?o2 when using binary join plan1 and plan2
		for(unsigned small_plan_nodes_num = min_size; small_plan_nodes_num <= max_size; ++ small_plan_nodes_num){
			for(const auto &small_nodes_plan : plan_cache[small_plan_nodes_num-1]){
				if(OrderedVector::contain_sub_vec(need_considerbinaryjoin_nodes_plan.first, small_nodes_plan.first)) {
					vector<unsigned> other_nodes;
					OrderedVector::subtract(need_considerbinaryjoin_nodes_plan.first, small_nodes_plan.first,other_nodes);
					set<unsigned> join_nodes;
					get_join_nodes(small_nodes_plan.first, other_nodes, join_nodes);

					if (join_nodes.size() >= 1 && join_nodes.size() + other_nodes.size() < var_num - 1) {

						OrderedVector::vec_set_union(other_nodes, join_nodes);

						if (plan_cache[other_nodes.size() - 1].find(other_nodes) !=
								plan_cache[other_nodes.size() - 1].end()) {

							PlanTree *small_best_plan = get_best_plan(small_nodes_plan.first);
							PlanTree *another_small_best_plan = get_best_plan(other_nodes);

							long long now_cost = cost_model_for_binary(small_nodes_plan.first,
																		   other_nodes, small_best_plan,
																		   another_small_best_plan);
							// long long now_cost = 1;

							if (now_cost < last_plan_smallest_cost) {
								PlanTree *new_plan = new PlanTree(small_best_plan, another_small_best_plan, bgpquery, join_nodes);
								new_plan->plan_cost = now_cost;

								insert_this_plan_to_cache(new_plan, need_considerbinaryjoin_nodes_plan.first, var_num);
								// plan_cache[var_num - 1][need_considerbinaryjoin_nodes_plan.first].push_back(new_plan);
								last_plan_smallest_cost = now_cost;

							}
						}
					}
				}

			}
		}
	}
}

bool compare_pair_vector(pair<double, unsigned> a, pair<double, unsigned> b) {
	return a.first < b.first;
}

void PlanGenerator::addsatellitenode(PlanTree* best_plan) {

	vector<pair<double, unsigned >> satellitenode_score;
	vector<unsigned> already_node = best_plan->already_so_var;

	// satellite_nodes must be so_type?
	// yes! If s ?p o. ... then we only need to treat this one triple. Because it has not linked with other GP by var.
	for(unsigned satellitenode_index = 0; satellitenode_index < satellite_nodes.size(); ++satellitenode_index){
		unsigned satellitenode_id = satellite_nodes[satellitenode_index];
		if(find(already_node.begin(), already_node.end(), satellitenode_id) != already_node.end()) continue;
		if(bgpquery->distinct_query && already_done_satellite.count(satellite_nodes[satellitenode_index])) continue;

		auto var_descrip = bgpquery->get_vardescrip_by_id(satellitenode_id);

		// todo: predegree or pre_list; nei must give it a estimatation num
		if(var_descrip->so_edge_pre_type_[0] == VarDescriptor::PreType::ConPreType)
			satellitenode_score.emplace_back((double)(pre2num[var_descrip->so_edge_pre_id_[0]]) /
												var_to_num_map[var_descrip->so_edge_nei_[0]], satellitenode_index);
		else
			satellitenode_score.emplace_back((double)(triples_num) / var_to_num_map[var_descrip->so_edge_nei_[0]],
											 	satellitenode_index);

	}

	sort(satellitenode_score.begin(), satellitenode_score.end(), compare_pair_vector);

	for(unsigned satellitenode_index = 0; satellitenode_index < satellitenode_score.size(); ++satellitenode_index){
		best_plan->add_satellitenode(bgpquery, satellite_nodes[satellitenode_score[satellitenode_index].second]);
	}

}


// 我在想这样一个问题：什么时候会有谓词变量？
// 如果有谓词变量，感觉一定会有s_o_var连接上，不然会被拆分开。
// 是这样吗？

PlanTree *PlanGenerator::get_plan(bool use_binary_join) {

	cout << "-------print var and id--------" << endl;
	for(unsigned i = 0; i < bgpquery->var_vector.size(); ++i){
		cout << "\t" << bgpquery->get_vardescrip_by_index(i)->var_name_ << "\t\t" << bgpquery->get_vardescrip_by_index(i)->id_ << endl;
	}


	considervarscan();

	// cout << "print for var_to_num_map:" << endl;
	// for(auto x:var_to_num_map)
	// 	cout << x.first << "   " << x.second<<endl;

	// should be var num not include satellite node
	// should not include pre_var num
	for(unsigned var_num = 2; var_num <= join_nodes.size(); ++var_num) {

		// if i want to complete this, i need to know whether the input query is linded or not
		// answer: yes, input query is linked by var
		considerwcojoin(var_num);

		if(use_binary_join)
			if(var_num >= 5)
				considerbinaryjoin(var_num);
	}

	for(auto x:card_cache){
		for(auto y:x){
			for(auto z:y.first) cout << z << " ";
			cout << "card: " << y.second << endl;
		}
	}

	PlanTree* best_plan = get_best_plan_by_num(join_nodes.size());

	// todo: 这个卫星点应该也有卫星谓词变量
	// s ?p ?o. 在之前的计划中已经加入了?o, 则这一步也需要加入?p
	addsatellitenode(best_plan);

	cout << endl << endl;
	print_plan_generator_info();
	print_sample_info();
	cout << endl << endl;


	return best_plan;

}


// Codes below is for generating random plan



/**
 * Get first node id when generate random plan.
 * linked_with_const s_o_var >> linked_with_const pre_var >> s_o_var >> pre_var
 * In each class, choose node with max_degree.
 * @return the first node id
 */
unsigned PlanGenerator::choose_first_var_id_random() {

	unsigned first_var_id = UINT_MAX;
	unsigned degree = 0;

	auto first_var_type = VarDescriptor::VarType::NotDecided;

	// choose var linked with const and large degree, so_var is better than pre_var
	for(unsigned var_index = 0; var_index < bgpquery->get_total_var_num(); ++ var_index){

		auto var_descrip = bgpquery->get_vardescrip_by_index(var_index);
		if(!var_descrip->link_with_const)
			continue;

		if(first_var_type == VarDescriptor::VarType::NotDecided){
			first_var_type = var_descrip->var_type_;
			first_var_id = var_descrip->id_;
			degree = var_descrip->degree_;
			continue;
		}

		if(first_var_type == var_descrip->var_type_){
			if(var_descrip->degree_ > degree){
				first_var_id = var_descrip->id_;
				degree = var_descrip->degree_;
			}
		} else{
			if(first_var_type == VarDescriptor::VarType::Predicate and var_descrip->var_type_ == VarDescriptor::VarType::Entity){
				first_var_id = var_descrip->id_;
				degree = var_descrip->degree_;
				first_var_type = VarDescriptor::VarType::Entity;
			}
			// if first_var_type == VarDescriptor::VarType::Entity,
			// and var_descrip->var_type_ == VarDescriptor::VarType::Predicate,
			// do not choose that var
		}
	}

	// choose var with large degree, so_var is better than pre_var
	if(degree == 0){
		for(unsigned var_index = 0; var_index < bgpquery->get_total_var_num(); ++ var_index){

			auto var_descrip = bgpquery->get_vardescrip_by_index(var_index);

			if(first_var_type == VarDescriptor::VarType::NotDecided){
				first_var_type = var_descrip->var_type_;
				first_var_id = var_descrip->id_;
				degree = var_descrip->degree_;
				continue;
			}

			if(first_var_type == var_descrip->var_type_){
				if(var_descrip->degree_ > degree){
					first_var_id = var_descrip->id_;
					degree = var_descrip->degree_;
				}
			} else{
				if(first_var_type == VarDescriptor::VarType::Predicate and var_descrip->var_type_ == VarDescriptor::VarType::Entity){
					first_var_id = var_descrip->id_;
					degree = var_descrip->degree_;
					first_var_type = VarDescriptor::VarType::Entity;
				}
				// if first_var_type == VarDescriptor::VarType::Entity,
				// and var_descrip->var_type_ == VarDescriptor::VarType::Predicate,
				// do not choose that var
			}
		}

		if(degree == 0){
			cout << "error: find first node to join error" << endl;
			exit(-1);
		}
	}

	return first_var_id;
}

/**
 * Get which nei from nei_id_set to expand
 * @param nei_id_set node id can been chosen to expand
 * @return the chosen node id
 */
unsigned int PlanGenerator::choose_next_nei_id(set<unsigned int> nei_id_set) {

	unsigned next_id = UINT_MAX;
	unsigned degree = 0;

	for(auto x : nei_id_set){
		auto var_descrip = bgpquery->get_vardescrip_by_id(x);

		if(var_descrip->degree_ > degree){
			next_id = var_descrip->id_;
			degree = var_descrip->degree_;
		}
	}

	if(degree == 0){
		cout << "error: choose next nei id error" << endl;
		exit(-1);
	} else{
		return next_id;
	}
}


/**
 * insert next_id's nei to neibor_id, remove next_id from neibor_id, insert next_id to already_id
 * @param neibor_id neighbor of processed node, which have not been processed
 * @param already_id already processed node id
 * @param next_id next processed node id
 */
void PlanGenerator::update_nei_id_set(set<unsigned> &neibor_id, set<unsigned> &already_id, unsigned next_id) {

	auto var_descrip = bgpquery->get_vardescrip_by_id(next_id);

	if(var_descrip->var_type_ == VarDescriptor::VarType::Entity){
		for(unsigned i = 0; i < var_descrip->degree_; ++i){
			// TODO: pre var
			if(var_descrip->so_edge_nei_type_[i] == VarDescriptor::EntiType::VarEntiType &&
					already_id.count(var_descrip->so_edge_nei_[i]) == 0)
				neibor_id.insert(var_descrip->so_edge_nei_[i]);

			if(var_descrip->so_edge_pre_type_[i] == VarDescriptor::PreType::VarPreType &&
					already_id.count(var_descrip->so_edge_pre_id_[i]) == 0)
				neibor_id.insert(var_descrip->so_edge_pre_id_[i]);

		}
	} else{
		for(unsigned i = 0; i < var_descrip->degree_; ++i){
			if(var_descrip->s_type_[i] == VarDescriptor::EntiType::VarEntiType &&
					already_id.count(var_descrip->s_id_[i]) == 0)
				neibor_id.insert(var_descrip->s_id_[i]);

			if(var_descrip->o_type_[i] == VarDescriptor::EntiType::VarEntiType &&
					already_id.count(var_descrip->o_id_[i]) == 0)
				neibor_id.insert(var_descrip->o_id_[i]);
		}
	}

	already_id.insert(next_id);
	neibor_id.erase(next_id);

}


// get a plan for debug, DFS_based plan.
// First choose a max_degreed so_var with const,
// then choose its nei by degree.
// todo: this fun has a bug: what about a query has only pre_var?
PlanTree *PlanGenerator::get_random_plan() {

	unsigned first_var_id = choose_first_var_id_random();
	auto first_var_descrip = bgpquery->get_vardescrip_by_id(first_var_id);
	unsigned degree = first_var_descrip->degree_;


	PlanTree* plan_tree_p = new PlanTree(first_var_id, bgpquery, true);
	PlanTree* plan_tree_q = plan_tree_p;

	set<unsigned> neibor_id;
	set<unsigned> already_id{first_var_id};
	if(first_var_descrip->var_type_ == VarDescriptor::VarType::Entity){
		for(unsigned i = 0; i < degree; ++i){
			if(first_var_descrip->so_edge_nei_type_[i] == VarDescriptor::EntiType::VarEntiType)
				neibor_id.insert(first_var_descrip->so_edge_nei_[i]);
		}
	} else{
		for(unsigned i = 0; i < degree; ++i){
			if(first_var_descrip->s_type_[i] == VarDescriptor::EntiType::VarEntiType)
				neibor_id.insert(first_var_descrip->s_id_[i]);
			if(first_var_descrip->o_type_[i] == VarDescriptor::EntiType::VarEntiType)
				neibor_id.insert(first_var_descrip->o_id_[i]);
		}
	}

	while(neibor_id.size() > 0){
		unsigned next_id = choose_next_nei_id(neibor_id);
		plan_tree_q = new PlanTree(plan_tree_p, next_id, already_id, bgpquery);
		update_nei_id_set(neibor_id, already_id, next_id);

		plan_tree_p = plan_tree_q;
	}

	return plan_tree_q;

}

PlanTree *PlanGenerator::get_special_one_triple_plan() {
	bool s_is_var = !(bgpquery->s_is_constant_[0]);
	bool p_is_var = !(bgpquery->p_is_constant_[0]);
	bool o_is_var = !(bgpquery->o_is_constant_[0]);

	unsigned var_num = bgpquery->get_total_var_num();

	if(var_num == 3){
		auto edge_info = make_shared<vector<EdgeInfo>>();
		edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], JoinMethod::p2so);

		auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
		edge_constant_info->emplace_back(false, false, false);


		auto plan_node = make_shared<StepOperation>(StepOperation::JoinType::GetAllTriples,
													make_shared<FeedOneNode>(bgpquery->p_id_[0], edge_info, edge_constant_info),
													nullptr, nullptr, nullptr);

		return (new PlanTree(plan_node));
	} else{
		JoinMethod join_method = get_join_strategy(s_is_var, p_is_var, o_is_var, var_num);

		auto first_var = bgpquery->get_vardescrip_by_index(0);

		switch (join_method) {

			case JoinMethod::sp2o:
			case JoinMethod::po2s:
			case JoinMethod::so2p:{
				auto edge_info = make_shared<vector<EdgeInfo>>();
				edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], join_method);

				auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
				edge_constant_info->emplace_back(!s_is_var, !p_is_var, !o_is_var);


				auto plan_node = make_shared<StepOperation>(StepOperation::JoinType::JoinNode,
															make_shared<FeedOneNode>(first_var->id_, edge_info, edge_constant_info),
															nullptr, nullptr, nullptr);

				return (new PlanTree(plan_node));
			}
			case JoinMethod::s2po: {
				auto edge_info = make_shared<EdgeInfo>(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0] , join_method);

				auto edge_constant_info = make_shared<EdgeConstantInfo>(!s_is_var, !p_is_var, !o_is_var);

				auto plan_node = make_shared<StepOperation>(StepOperation::JoinType::JoinTwoNodes, nullptr,
															make_shared<FeedTwoNode>(bgpquery->p_id_[0], bgpquery->o_id_[0], *edge_info, *edge_constant_info),
															nullptr, nullptr);
				return (new PlanTree(plan_node));
			}
			case JoinMethod::p2so: {
				auto edge_info = make_shared<EdgeInfo>(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0] , join_method);

				auto edge_constant_info = make_shared<EdgeConstantInfo>(!s_is_var, !p_is_var, !o_is_var);

				auto plan_node = make_shared<StepOperation>(StepOperation::JoinType::JoinTwoNodes, nullptr,
															make_shared<FeedTwoNode>(bgpquery->s_id_[0], bgpquery->o_id_[0], *edge_info, *edge_constant_info),
															nullptr, nullptr);
				return (new PlanTree(plan_node));
			}
			case JoinMethod::o2ps: {
				auto edge_info = make_shared<EdgeInfo>(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0] ,join_method);

				auto edge_constant_info = make_shared<EdgeConstantInfo>(!s_is_var, !p_is_var, !o_is_var);

				auto plan_node = make_shared<StepOperation>(StepOperation::JoinType::JoinTwoNodes, nullptr,
															make_shared<FeedTwoNode>(bgpquery->p_id_[0], bgpquery->s_id_[0], *edge_info, *edge_constant_info),
															nullptr, nullptr);

				return (new PlanTree(plan_node));
			}
			case JoinMethod::p2s:{
				auto edge_info = make_shared<vector<EdgeInfo>>();
				edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], join_method);

				auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
				edge_constant_info->emplace_back(!s_is_var, !p_is_var, !o_is_var);


				auto plan_node = make_shared<StepOperation>(StepOperation::JoinType::JoinNode,
															make_shared<FeedOneNode>(bgpquery->s_id_[0], edge_info, edge_constant_info),
															nullptr, nullptr, nullptr, bgpquery->distinct_query);

				return (new PlanTree(plan_node));
			}
			case JoinMethod::p2o:{
				auto edge_info = make_shared<vector<EdgeInfo>>();
				edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], join_method);

				auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
				edge_constant_info->emplace_back(!s_is_var, !p_is_var, !o_is_var);


				auto plan_node = make_shared<StepOperation>(StepOperation::JoinType::JoinNode,
															make_shared<FeedOneNode>(bgpquery->o_id_[0], edge_info, edge_constant_info),
															nullptr, nullptr, nullptr, bgpquery->distinct_query);

				return (new PlanTree(plan_node));
			}
			case JoinMethod::s2p:{
				auto edge_info = make_shared<vector<EdgeInfo>>();
				edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], join_method);

				auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
				edge_constant_info->emplace_back(!s_is_var, !p_is_var, !o_is_var);


				auto plan_node = make_shared<StepOperation>(StepOperation::JoinType::JoinNode,
															make_shared<FeedOneNode>(bgpquery->p_id_[0], edge_info, edge_constant_info),
															nullptr, nullptr, nullptr, bgpquery->distinct_query);

				return (new PlanTree(plan_node));
			}
			case JoinMethod::s2o:{
				auto edge_info = make_shared<vector<EdgeInfo>>();
				edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], join_method);

				auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
				edge_constant_info->emplace_back(!s_is_var, !p_is_var, !o_is_var);


				auto plan_node = make_shared<StepOperation>(StepOperation::JoinType::JoinNode,
															make_shared<FeedOneNode>(bgpquery->o_id_[0], edge_info, edge_constant_info),
															nullptr, nullptr, nullptr, bgpquery->distinct_query);

				return (new PlanTree(plan_node));
			}
			case JoinMethod::o2s:{
				auto edge_info = make_shared<vector<EdgeInfo>>();
				edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], join_method);

				auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
				edge_constant_info->emplace_back(!s_is_var, !p_is_var, !o_is_var);


				auto plan_node = make_shared<StepOperation>(StepOperation::JoinType::JoinNode,
															make_shared<FeedOneNode>(bgpquery->s_id_[0], edge_info, edge_constant_info),
															nullptr, nullptr, nullptr, bgpquery->distinct_query);

				return (new PlanTree(plan_node));
			}
			case JoinMethod::o2p:{
				auto edge_info = make_shared<vector<EdgeInfo>>();
				edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], join_method);

				auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
				edge_constant_info->emplace_back(!s_is_var, !p_is_var, !o_is_var);


				auto plan_node = make_shared<StepOperation>(StepOperation::JoinType::JoinNode,
															make_shared<FeedOneNode>(bgpquery->p_id_[0], edge_info, edge_constant_info),
															nullptr, nullptr, nullptr, bgpquery->distinct_query);

				return (new PlanTree(plan_node));
			}
			default:{
				cout << "error: joinmethod error" << endl;
				exit(-1);
			}
		}
	}

}


// Codes belows for TOPK

void PlanGenerator::get_idcache_sample(shared_ptr<IDList> &so_cache, vector<unsigned> &so_sample_cache) {
	random_device rd;
	mt19937 eng(rd());

	auto cache_size = so_cache->size();

	if (cache_size <= SAMPLE_CACHE_MAX) {
		so_sample_cache.assign(so_cache->getList()->begin(), so_cache->getList()->end());
	} else {
		unsigned sample_size = get_sample_size(cache_size);
		// need_insert_vec = new IDList(sample_size);
		so_sample_cache.reserve(sample_size);

		// auto id_cache_list = so_cache->getList();
		uniform_int_distribution<unsigned> dis(0, cache_size-1);

		for (unsigned sample_num = 0; sample_num < sample_size; ++sample_num) {
			// unsigned index_need_insert = rand()% cache_size;
			unsigned index_need_insert = dis(eng);
			// so_sample_cache.push_back((*id_cache_list)[index_need_insert]);
			so_sample_cache.push_back(so_cache->getID(index_need_insert));
		}
	}
}

double PlanGenerator::estimate_one_edge_selectivity(TYPE_PREDICATE_ID pre_id, bool pre_constant, KVstore *kvstore,
													shared_ptr<IDList> &s_cache, shared_ptr<IDList> &o_cache) {
	vector<unsigned> s_sample_cache;
	vector<unsigned> o_sample_cache;

	unsigned pass_num = 0;
	unsigned small_cache_size;

	if(s_cache->size() < o_cache->size()){
		get_idcache_sample(s_cache, s_sample_cache);
		small_cache_size = s_cache->size();
	}
	else{
		get_idcache_sample(o_cache, o_sample_cache);
		small_cache_size = o_cache->size();
	}

	if(pre_constant){
		if(s_cache->size() < o_cache->size()){
			for (unsigned i = 0; i < s_sample_cache.size(); ++i) {
				unsigned *o_list = nullptr;
				unsigned o_list_len = 0;

				kvstore->getobjIDlistBysubIDpreID(s_sample_cache[i], pre_id,
												  o_list,o_list_len);

				for (unsigned j = 0; j < o_list_len; ++j)
					if (binary_search(o_cache->begin(), o_cache->end(), o_list[j]))
						pass_num += 1;

				delete[] o_list;

			}
		} else{
			for (unsigned i = 0; i < o_sample_cache.size(); ++i) {
				unsigned *s_list = nullptr;
				unsigned s_list_len = 0;

				kvstore->getsubIDlistByobjIDpreID(o_sample_cache[i], pre_id,
												  s_list,s_list_len);

				for (unsigned j = 0; j < s_list_len; ++j)
					if (binary_search(s_cache->begin(), s_cache->end(), s_list[j]))
						pass_num += 1;

					delete[] s_list;

			}
		}

	} else{
		if(s_cache->size() < o_cache->size()){
			for (unsigned i = 0; i < s_sample_cache.size(); ++i) {
				unsigned *o_list = nullptr;
				unsigned o_list_len = 0;

				kvstore->getobjIDlistBysubID(s_sample_cache[i],
												  o_list,o_list_len);

				for (unsigned j = 0; j < o_list_len; ++j)
					if (binary_search(o_cache->begin(), o_cache->end(), o_list[j]))
						pass_num += 1;

					delete[] o_list;

			}
		} else{
			for (unsigned i = 0; i < o_sample_cache.size(); ++i) {
				unsigned *s_list = nullptr;
				unsigned s_list_len = 0;

				kvstore->getsubIDlistByobjID(o_sample_cache[i],
												  s_list,s_list_len);

				for (unsigned j = 0; j < s_list_len; ++j)
					if (binary_search(s_cache->begin(), s_cache->end(), s_list[j]))
						pass_num += 1;

					delete[] s_list;

			}
		}
	}

	return (double)pass_num/small_cache_size;
}


// Codes belows for print debug_info
void PlanGenerator::print_plan_generator_info(){
	cout << "----print plan_generator_info----" << endl;
	cout << "triple_num = " << triples_num << endl;
	cout << "limit_literal = " << limitID_literal << endl;
	cout << "limit_entity = " << limitID_entity << endl;
	cout << "limit_predicate = " << limitID_predicate << endl;
}


void PlanGenerator::print_sample_info(){

	cout << "----print var_to_num_map----" << endl;
	for(auto var_num_pair : var_to_num_map){
		cout << "var: " << var_num_pair.first << ", num: " << var_num_pair.second << endl;
	}

	cout << "----print var_sampled_from_candidate----" << endl;
	for(auto var_sampled_pair : var_sampled_from_candidate){
		cout << "var: " << var_sampled_pair.first << ", sampled: " << (var_sampled_pair.second ? "true" : "false") << endl;
	}

	cout << "----print var_to_sample_cache----" << endl;
	for(auto var_sample_pair : var_to_sample_cache){
		cout << "var: " << var_sample_pair.first << ", sample_num: " << var_sample_pair.second.size() << endl;
	}

	cout << "----print s_o_list_average_size----" << endl;
	for(auto s_pair : s_o_list_average_size){
		for(auto o_pair : s_pair.second){
			cout << s_pair.first << " to " << o_pair.first << " average size: " << o_pair.second << endl;
		}
	}

}