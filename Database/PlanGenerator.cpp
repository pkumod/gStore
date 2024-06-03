/*=============================================================================
# Filename: PlanGenerator.cpp
# Author: Linglin Yang
# Mail: linglinyang@stu.pku.edu.cn
=============================================================================*/

// something need clear:
// 1. s ?p ?o. If we use ?p in the future, then s2po is similar to s2o in efficiency, because the result size is same.
// 			   However, we need to re-design the cost function to fit the new added pre_var ?p.

// s ?p ?o
// ?s ?p o do not need to generate ?p candidate, even pre num of one s_o is small
// however, ?o ?s's candidate should be considered

// plan_cache only contains plans about s_o var, because in ?s ?p ?o, after considering ?s and ?o, then ?p has also considered.
// in binary join, only consider s_o var

// scan var(candidate set generation) -> add var by WCO_join or binary_join -> add satellite var

// NOTICE:
// random_device rd;
// mt19937 eng(rd());
// may cause SIGILL in valgrind. when encounter this prob, please change it to other sample method

#include "PlanGenerator.h"

const unsigned PlanGenerator::SAMPLE_CACHE_MAX = 50;
const double PlanGenerator::SAMPLE_PRO = 0.05;
const unsigned PlanGenerator::SMALL_QUERY_VAR_NUM = 4;
const double PlanGenerator::CANDIDATE_RATIO_MAX = 15.0;
const unsigned PlanGenerator::PARAM_SIZE = 1000000;
const unsigned PlanGenerator::PARAM_PRE = 10000;
const unsigned PlanGenerator::HEURISTIC_CANDIDATE_MAX = 100;

PlanGenerator::PlanGenerator(KVstore *kvstore_, BGPQuery *bgpquery_, IDCachesSharePtr &id_caches_,
							 TYPE_TRIPLE_NUM triples_num_, TYPE_PREDICATE_ID limitID_predicate_,
							 TYPE_ENTITY_LITERAL_ID limitID_literal_, TYPE_ENTITY_LITERAL_ID limitID_entity_,
							 TYPE_TRIPLE_NUM* pre2num_, TYPE_TRIPLE_NUM* pre2sub_, TYPE_TRIPLE_NUM* pre2obj_, shared_ptr<Transaction> txn_):
					kvstore(kvstore_), bgpquery(bgpquery_), id_caches(id_caches_), triples_num(triples_num_),
					limitID_predicate(limitID_predicate_), limitID_literal(limitID_literal_), limitID_entity(limitID_entity_),
					pre2num(pre2num_), pre2sub(pre2sub_), pre2obj(pre2obj_), txn(std::move(txn_)) { };

PlanGenerator::~PlanGenerator() {
	for(auto &map_plan_list : plan_cache){
		for(auto &nodes_plan_list_pair : map_plan_list){
			auto plan_ = nodes_plan_list_pair.second.begin();
			while(plan_ != nodes_plan_list_pair.second.end()){
				auto temp = *plan_;
				plan_ = nodes_plan_list_pair.second.erase(plan_);
				delete temp;
			}
		}
	}
}

JoinMethod PlanGenerator::GetJoinStrategy(bool s_is_var, bool p_is_var, bool o_is_var, unsigned int var_num) const {
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

unsigned PlanGenerator::GetCandidateSizeFromWholeDB(unsigned int var_id) const {
	auto var_descrip = bgpquery->get_vardescrip_by_id(var_id);

	bool not_literal = false;
	for(unsigned i = 0; i < var_descrip->degree_; ++i){
		if (var_descrip->so_edge_type_[i] == Util::EDGE_OUT) {
			not_literal = true;
			break;
		}
	}

	return max((not_literal ? limitID_entity : limitID_entity + limitID_literal), (unsigned)1);
}

unsigned PlanGenerator::GetSampleSize(unsigned int id_cache_size) {
	if(id_cache_size <= 100){
		if(SAMPLE_CACHE_MAX < id_cache_size)
			return SAMPLE_CACHE_MAX;
		else
			return id_cache_size;
	} else{
		return (unsigned )(log(id_cache_size)*11);
	}
}

void PlanGenerator::InsertEdgeSelectivityToCache(unsigned int from_id, unsigned int to_id, unsigned int linked_num) {
	double selectivity = max(1.0, ((double) linked_num / var_to_sample_cache[from_id].size()));
	if (s_o_list_average_size.find(from_id) == s_o_list_average_size.end()) {
		map<unsigned, double > this_node_selectivity_map;
		this_node_selectivity_map.insert(make_pair(to_id, selectivity));
		s_o_list_average_size.insert(make_pair(from_id, this_node_selectivity_map));
	} else {
		s_o_list_average_size[from_id].insert(make_pair(to_id, selectivity));
	}
}

void PlanGenerator::InsertCardEstimationToCache(const vector<unsigned> &now_plan_nodes, long long card_estimation,
												vector<vector<unsigned>> &result_sample) {
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

long long PlanGenerator::CardEstimatorTwoNodes(unsigned int last_node, unsigned int next_join_node, const vector<unsigned int> &now_plan_nodes) {
	auto var_descrip = bgpquery->get_vardescrip_by_id(next_join_node);

	if(card_cache.empty() || card_cache[0].find(now_plan_nodes) == card_cache[0].end()){
		// todo: sample from last_plan
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
							if (!kvstore->existThisTriple(var_to_sample_cache[last_node][i],
														  p_list[k], s_o_list[j])) {
								break;
							}
						} else {
							if (!kvstore->existThisTriple(s_o_list[j], p_list[k],
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

							this_sample.push_back(std::move(this_pass_sample));
						} else {
							// if (rand() / double(RAND_MAX) < SAMPLE_PRO) {
							if (dis(eng) < SAMPLE_PRO) {
								vector<unsigned> this_pass_sample(2);
								this_pass_sample[new_id_pos_map[last_node]] = var_to_sample_cache[last_node][i];
								this_pass_sample[new_id_pos_map[next_join_node]] = s_o_list[j];

								this_sample.push_back(std::move(this_pass_sample));
							}
						}
					}
				}
				delete[] s_o_list;
			}

			for (const unsigned int &var_sample : var_to_sample_cache[next_join_node]) {
				unsigned *s_o_list = nullptr;
				unsigned s_o_list_len = 0;

				if(p_list[0] >= 0)
					kvstore->getsubIDlistByobjIDpreID(var_sample, p_list[0],
													  s_o_list,s_o_list_len, false, this->txn);
				else
					kvstore->getsubIDlistByobjID(var_sample,
												 s_o_list, s_o_list_len, bgpquery->distinct_query, this->txn);

				s_o_list2_total_num += s_o_list_len;
				delete[] s_o_list;
			}

			InsertEdgeSelectivityToCache(last_node, next_join_node, s_o_list1_total_num);
			InsertEdgeSelectivityToCache(next_join_node, last_node, s_o_list2_total_num);

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
							if (!kvstore->existThisTriple(s_o_list[j], p_list[k],
														  var_to_sample_cache[next_join_node][i])) {
								break;
							}
						} else {
							if (!kvstore->existThisTriple(var_to_sample_cache[next_join_node][i], p_list[k],
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

							this_sample.push_back(std::move(this_pass_sample));
						} else {
							// if (rand() / double(RAND_MAX) < SAMPLE_PRO) {
							if (dis(eng) < SAMPLE_PRO) {

								vector<unsigned> this_pass_sample(2);
								this_pass_sample[new_id_pos_map[last_node]] = s_o_list[j];
								this_pass_sample[new_id_pos_map[next_join_node]] = var_to_sample_cache[next_join_node][i];

								this_sample.push_back(std::move(this_pass_sample));

							}
						}
					}
				}
				delete[] s_o_list;
			}

			for (const unsigned int &var_sample : var_to_sample_cache[last_node]) {
				unsigned *s_o_list = nullptr;
				unsigned s_o_list_len = 0;

				if(p_list[0] >= 0)
					kvstore->getsubIDlistByobjIDpreID(var_sample, p_list[0],
													  s_o_list,s_o_list_len, false, this->txn);
				else
					kvstore->getsubIDlistByobjID(var_sample,
												 s_o_list, s_o_list_len, bgpquery->distinct_query, this->txn);

				s_o_list2_total_num += s_o_list_len;
				delete[] s_o_list;
			}

			InsertEdgeSelectivityToCache(next_join_node, last_node, s_o_list1_total_num);
			InsertEdgeSelectivityToCache(last_node, next_join_node, s_o_list2_total_num);
		}

		card_estimation = max((edge_type[0] == Util::EDGE_IN) ?
				(long long) ((double) (now_sample_num * var_to_num_map[last_node]) / var_to_sample_cache[last_node].size()) :
				(long long) ((double) (now_sample_num * var_to_num_map[next_join_node]) / var_to_sample_cache[next_join_node].size() )
					, (long long) 1);

		InsertCardEstimationToCache(now_plan_nodes, card_estimation, this_sample);

		if (changed_two_nodes)
			return var_to_num_map[next_join_node]*s_o_list_average_size[next_join_node][last_node];
		else
			return var_to_num_map[last_node]*s_o_list_average_size[last_node][next_join_node];
	}  else {
		return var_to_num_map[last_node]*s_o_list_average_size[last_node][next_join_node];//+var_to_num_map[next_join_node];
	}
}

long long PlanGenerator::CardEstimatorMoreThanTwoNodes(const vector<unsigned> &last_plan_nodes, unsigned next_join_node, const vector<unsigned> &now_plan_nodes) {
	unsigned last_plan_nodes_num = last_plan_nodes.size();
	auto var_descrip = bgpquery->get_vardescrip_by_id(next_join_node);

	if (card_cache.size() < last_plan_nodes_num ||
			card_cache[last_plan_nodes_num - 1].find(now_plan_nodes) == card_cache[last_plan_nodes_num - 1].end()) {

		long long card_estimation;
		long long last_card_estimation = card_cache[last_plan_nodes_num - 2][last_plan_nodes];

		vector<vector<unsigned>> this_sample;
		vector<vector<unsigned>> &last_sample = sample_cache[last_plan_nodes_num - 2][last_plan_nodes];

		if (!last_sample.empty()) {
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
								if (!kvstore->existThisTriple(last_sample[i][linked_nei_pos[k]], p_list[k],
															  s_o_list[j])) {
									break;
								}
							} else {
								if (!kvstore->existThisTriple(s_o_list[j], p_list[k],
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

								this_sample.push_back(std::move(this_pass_sample));
							} else {

								// if (rand() % 1000 < SAMPLE_CACHE_MAX) {
								if (dis(eng) < SAMPLE_PRO) {

									vector<unsigned> this_pass_sample(last_plan_nodes_num+1);
									for (unsigned l = 0; l < last_plan_nodes_num; ++l) {
										this_pass_sample[new_id_pos_map[last_plan_nodes[l]]] = last_sample[i][l];
									}
									this_pass_sample[new_id_pos_map[next_join_node]] = s_o_list[j];

									this_sample.push_back(std::move(this_pass_sample));
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
								if (!kvstore->existThisTriple(last_sample[i][linked_nei_pos[k]], p_list[k],
															  s_o_list[j])) {
									break;
								}
							} else {
								if (!kvstore->existThisTriple(s_o_list[j], p_list[k],
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

								this_sample.push_back(std::move(this_pass_sample));
							} else {

								// if (rand() % 1000 < SAMPLE_CACHE_MAX) {
								if (dis(eng) < SAMPLE_PRO) {

									vector<unsigned> this_pass_sample(last_plan_nodes_num+1);
									for (unsigned l = 0; l < last_plan_nodes_num; ++l) {
										this_pass_sample[new_id_pos_map[last_plan_nodes[l]]] = last_sample[i][l];
									}
									this_pass_sample[new_id_pos_map[next_join_node]] = s_o_list[j];

									this_sample.push_back(std::move(this_pass_sample));
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

		InsertCardEstimationToCache(now_plan_nodes, card_estimation, this_sample);

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

long long PlanGenerator::CardEstimator(const vector<unsigned> &last_plan_nodes, unsigned next_join_node, const vector<unsigned> &now_plan_nodes) {
	unsigned last_plan_nodes_num = last_plan_nodes.size();
	if(last_plan_nodes_num == 1)
		return CardEstimatorTwoNodes(last_plan_nodes[0], next_join_node, now_plan_nodes);
	else
		return CardEstimatorMoreThanTwoNodes(last_plan_nodes, next_join_node, now_plan_nodes);
}

long long PlanGenerator::GetCard(const vector<unsigned int> &nodes) {
	return card_cache[nodes.size()-2][nodes];
}

long long PlanGenerator::CostModelForWCOJoin(PlanTree *last_plan, const vector<unsigned int> &last_plan_node,
											 unsigned int next_node, const vector<unsigned int> &now_plan_node) {
	return last_plan->plan_cost + CardEstimator(last_plan_node, next_node, now_plan_node);
}

long long PlanGenerator::CostModelForBinaryJoin(const vector<unsigned int> &plan_a_nodes, const vector<unsigned int> &plan_b_nodes,
												PlanTree *plan_a, PlanTree *plan_b) {
	long long plan_a_card = GetCard(plan_a_nodes);
	long long plan_b_card = GetCard(plan_b_nodes);
	long long min_card = min(plan_a_card, plan_b_card);
	long long max_card = max(plan_a_card, plan_b_card);

	return min_card + 2*max_card + plan_a->plan_cost + plan_b->plan_cost;
}

PlanTree* PlanGenerator::GetBestPlanByNodes(const vector<unsigned int> &nodes) {
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
PlanTree* PlanGenerator::GetBestPlanByNum(int total_var_num) {
	PlanTree* best_plan = nullptr;
	long long min_cost = LLONG_MAX	;
	int count = 0;

	for(const auto &nodes_plan : plan_cache[total_var_num - 1]){
		for(const auto &plan_tree : nodes_plan.second){
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

void PlanGenerator::GetJoinNodes(const vector<unsigned int> &plan_a_nodes, vector<unsigned int> &other_nodes, set<unsigned int> &join_nodes_set) const {
	for(unsigned node : other_nodes){
		auto var_descrip = bgpquery->get_vardescrip_by_id(node);
		for(unsigned i = 0; i < var_descrip->degree_; ++i){
			if(var_descrip->so_edge_nei_type_[i] == VarDescriptor::EntiType::VarEntiType and
					find(plan_a_nodes.begin(), plan_a_nodes.end(), var_descrip->so_edge_nei_[i]) != plan_a_nodes.end()){
				join_nodes_set.insert(var_descrip->so_edge_nei_[i]);
			}
		}
	}
}

void PlanGenerator::InsertThisPlanToCache(PlanTree *new_plan, const vector<unsigned int> &new_node_vec, unsigned int var_num) {
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
unsigned PlanGenerator::GetSampleFromWholeDatabase(unsigned int var_id, vector<unsigned int> &so_sample_cache) const {

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


	unsigned sample_size = GetSampleSize(sample_from_num);
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
		if(!entity_str.empty()){
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
		if(!literal_str.empty()){
			so_sample_cache.emplace_back(index_need_insert);
			++already_sampled_num;
		}
	}

	return sample_from_num;

}

long long PlanGenerator::CostModelForp2soOptimization(unsigned int node_1_id, unsigned int node_2_id) {
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

/**
 * Generate var candidates_generation plan.
 * Edges below will be considered in this function:
 * 1. Triple with only one var: ?s p o, s ?p o, s p ?o.
 * 	We will tell the executor to generate candidate for the var from the other two const.
 * 2. Triple with two var and one const.
 * 	We will generate candidate for the var from the const.
 * 	Todo: generate all items from the db when the triples include the var is like ?s ?p ?o.
 */
vector<shared_ptr<AffectOneNode>> PlanGenerator::CompleteCandidate() {
	vector<shared_ptr<AffectOneNode>> need_candidate;
    if (bgpquery->get_triple_num() == 1) return need_candidate;
	for (unsigned var_index = 0 ; var_index < bgpquery->get_total_var_num(); ++ var_index) {
		if (bgpquery->is_var_satellite_by_index(var_index)) {
			satellite_nodes.emplace_back(bgpquery->get_var_id_by_index(var_index));
			continue;
		}

		auto var_descrip = bgpquery->get_vardescrip_by_index(var_index);
		unsigned var_id = var_descrip->id_;

		if (var_descrip->var_type_ == VarDescriptor::VarType::Predicate) continue;
		join_nodes.emplace_back(var_id);

		bool no_candidate =  (*id_caches).find(var_id) == (*id_caches).end();
		auto candidate_edge_info = make_shared<vector<EdgeInfo>>();
		auto candidate_edge_const_info = make_shared<vector<EdgeConstantInfo>>();
		unsigned size = (no_candidate ? limitID_entity + limitID_literal : max(((*id_caches)[var_id]->size()), (unsigned)2));
		unsigned estimate_num = min((unsigned)10000000, size);

		for (unsigned i = 0; i < var_descrip->degree_; ++i) {
			bool pre_const = var_descrip->so_edge_pre_type_[i] == VarDescriptor::PreType::ConPreType;
			bool nei_var = var_descrip->so_edge_nei_type_[i] == VarDescriptor::EntiType::VarEntiType;

			if(pre_const && nei_var){
				double border = size / (Util::logarithm(2, size) + 1);
				unsigned pre_id = var_descrip->so_edge_pre_id_[i];
				if ((!no_candidate) && (double)(pre2num[pre_id]) > border) {
					continue;
				}
				if ((var_descrip->so_edge_type_[i] == Util::EDGE_IN && pre2obj[pre_id] > estimate_num) ||
					(var_descrip->so_edge_type_[i] == Util::EDGE_OUT && pre2sub[pre_id] > estimate_num)) {
					continue;
				}
				unsigned triple_index = var_descrip->so_edge_index_[i];
				candidate_edge_info->emplace_back(bgpquery->s_id_[triple_index], bgpquery->p_id_[triple_index], bgpquery->o_id_[triple_index],
												  (var_descrip->so_edge_type_[i] == Util::EDGE_IN ? JoinMethod::p2o : JoinMethod::p2s));
				candidate_edge_const_info->emplace_back(bgpquery->s_is_constant_[triple_index], bgpquery->p_is_constant_[triple_index],bgpquery->o_is_constant_[triple_index]);
			}
		}
		if(!candidate_edge_info->empty()){
			need_candidate.emplace_back(make_shared<AffectOneNode>(var_id, candidate_edge_info, candidate_edge_const_info));
		}

	}
	return need_candidate;
}

// I think this fun has completed, I only need to deal with s_o_var
void PlanGenerator::GetNeighborBySubPlanNodes(const vector<unsigned int> &last_plan_node, set<unsigned int> &nei_node) {
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
void PlanGenerator::ConsiderWCOJoin(unsigned int var_num) {
	auto plan_tree_list = plan_cache[var_num - 2];
	for(const auto &last_node_plan : plan_tree_list){
		set<unsigned> nei_node;
		GetNeighborBySubPlanNodes(last_node_plan.first, nei_node);

		PlanTree* last_best_plan = GetBestPlanByNodes(last_node_plan.first);

		for(unsigned next_node : nei_node) {
			vector<unsigned> new_node_vec(last_node_plan.first);
			new_node_vec.push_back(next_node);
			sort(new_node_vec.begin(), new_node_vec.end());

			PlanTree* new_plan = new PlanTree(last_best_plan, bgpquery, next_node);
			long long cost = CostModelForWCOJoin(last_best_plan, last_node_plan.first, next_node, new_node_vec);
			new_plan->plan_cost = cost;

			if(var_num == 2){
				long long this_cost = CostModelForp2soOptimization(last_node_plan.first[0], next_node);
				if(this_cost < cost){
					delete new_plan;
					new_plan = new PlanTree(last_node_plan.first[0], next_node, bgpquery);
					new_plan->plan_cost = this_cost;
				}
			}
			InsertThisPlanToCache(new_plan, new_node_vec, var_num);
		}
	}
}

// todo: should consider the condition when using this carefully
// not add nodes, but to consider if binaryjoin could decrease cost
void PlanGenerator::ConsiderBinaryJoin(unsigned int var_num)  {
	unsigned min_size = 3;
	unsigned max_size = min(min_size, var_num - 2);

	for(const auto &need_considerbinaryjoin_nodes_plan : plan_cache[var_num - 1]){
		long long last_plan_smallest_cost = GetBestPlanByNodes(need_considerbinaryjoin_nodes_plan.first)->plan_cost;

		// for example. Input query: ?s1->?p1->?o1, ?s2->?p1->?o2, ?s1->?p3->?o2.
		// we have plan1 of ?s1->?p1->?o1, plan2 of ?s2->?p1->?o2.
		// but we need to check ?s1->?p3->?o2 when using binary join plan1 and plan2
		for(unsigned small_plan_nodes_num = min_size; small_plan_nodes_num <= max_size; ++ small_plan_nodes_num){
			for(const auto &small_nodes_plan : plan_cache[small_plan_nodes_num-1]){
				if(OrderedVector::contain_sub_vec(need_considerbinaryjoin_nodes_plan.first, small_nodes_plan.first)) {
					vector<unsigned> other_nodes;
					OrderedVector::subtract(need_considerbinaryjoin_nodes_plan.first, small_nodes_plan.first,other_nodes);
					set<unsigned> join_nodes_set;
					GetJoinNodes(small_nodes_plan.first, other_nodes, join_nodes_set);
					if (!join_nodes_set.empty() && join_nodes_set.size() + other_nodes.size() < var_num - 1) {
						OrderedVector::vec_set_union(other_nodes, join_nodes_set);
						if (plan_cache[other_nodes.size() - 1].find(other_nodes) !=
								plan_cache[other_nodes.size() - 1].end()) {

							PlanTree *small_best_plan = GetBestPlanByNodes(small_nodes_plan.first);
							PlanTree *another_small_best_plan = GetBestPlanByNodes(other_nodes);

							long long now_cost = CostModelForBinaryJoin(small_nodes_plan.first,
																		other_nodes, small_best_plan,
																		another_small_best_plan);

							if (now_cost < last_plan_smallest_cost) {
								auto *new_plan = new PlanTree(small_best_plan, another_small_best_plan, bgpquery, join_nodes_set);
								new_plan->plan_cost = now_cost;

								InsertThisPlanToCache(new_plan, need_considerbinaryjoin_nodes_plan.first, var_num);
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

// satellite_nodes must be so_type?
// yes! If s ?p o. ... then we only need to treat this one triple. Because it has not linked with other GP by var.
void PlanGenerator::AddSatelliteNode(PlanTree* best_plan) {
	vector<pair<double, unsigned >> satellitenode_score;
	vector<unsigned> already_node = best_plan->already_so_var;

	for(unsigned satellitenode_index = 0; satellitenode_index < satellite_nodes.size(); ++satellitenode_index){
		unsigned satellitenode_id = satellite_nodes[satellitenode_index];
		if(find(already_node.begin(), already_node.end(), satellitenode_id) != already_node.end()) continue;
		// if(bgpquery->distinct_query && already_done_satellite.count(satellite_nodes[satellitenode_index])) continue;

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

	for(const auto &satellite_node_score_id_par : satellitenode_score){
        if (bgpquery->is_var_selected_by_id(satellite_nodes[satellite_node_score_id_par.second]))
            best_plan->AddSatelliteNode(bgpquery, satellite_nodes[satellite_node_score_id_par.second], true);
        else
		    best_plan->AddSatelliteNode(bgpquery, satellite_nodes[satellite_node_score_id_par.second], false);
	}
}

// 1. If a query contains a small num of query vairables (SMALL_QUERY_VAR_NUM),
//    we use heuristic optimization strategy.
// 2. If a query contains variables which candidate sizes varies large (CANDIDATE_RATIO_MAX),
//    we use heuristic optimization strategy.
// 3. If the max candidate size is small (HEURISTIC_CANDIDATE_MAX),
// 	  we use heuristic optimization strategy.
// 4. Else, we use complex DP optimization strategy.
BGPQueryStrategy PlanGenerator::PlanStrategy(bool use_binary_join) {
	cout << "small query var num = " << SMALL_QUERY_VAR_NUM << ", total var num = " << bgpquery->get_total_var_num() << endl;
	if(bgpquery->get_total_var_num() <= SMALL_QUERY_VAR_NUM) return BGPQueryStrategy::Heuristic;

	unsigned candidate_max = 1 << 5;
	unsigned candidate_min = 1 << 5;
	for(unsigned var_index = 0 ; var_index < bgpquery->get_total_var_num(); ++ var_index) {
		auto var_descrip = bgpquery->get_vardescrip_by_index(var_index);
		unsigned var_id = var_descrip->id_;
		if((*id_caches).find(var_id) != (*id_caches).end()) {
			candidate_max = max(candidate_max, (*id_caches)[var_id]->size());
			candidate_min = min(candidate_min, (*id_caches)[var_id]->size());
		}
	}
	// cout << "(double)candidate_max/candidate_min = " << (double)candidate_max/candidate_min << endl;
	// cout << "CANDIDATE_RATIO_MAX = " << CANDIDATE_RATIO_MAX << endl;
	if(((double)candidate_max/candidate_min) > CANDIDATE_RATIO_MAX) return BGPQueryStrategy::Heuristic;
	if(candidate_max <= HEURISTIC_CANDIDATE_MAX) return BGPQueryStrategy::Heuristic;
	return BGPQueryStrategy::DP;
}

// Only need to consider SO_var.
// Pre_var should not be passed into this function.
// If a query only contain pre_var, then it must be like s ?p o.
double PlanGenerator::NodeScore(unsigned int var_id) {
	auto var_descrip = bgpquery->get_vardescrip_by_id(var_id);
	unsigned candidate_size = var_to_num_map[var_id];
	double node_score = 1.0;

	for(unsigned nei_index = 0; nei_index < var_descrip->degree_; ++nei_index) {
		// Skip const linked neighbor (handled before) or satellite node (handle at last),
		if (!var_descrip->IsIthEdgeLinkedVarSO(nei_index)) continue;
		unsigned nei_id = var_descrip->so_edge_nei_[nei_index];
        if (find(plan_var_vec.begin(), plan_var_vec.end(), nei_id) == plan_var_vec.end()) continue;

		unsigned pre_size = (var_descrip->IsIthEdgePreVar(nei_index) ? max((unsigned)triples_num/limitID_predicate, (unsigned)2) : pre2num[var_descrip->so_edge_pre_id_[nei_index]]);
		node_score += PlanGenerator::PARAM_PRE / (double)(pre_size+1);
	}

	node_score += PlanGenerator::PARAM_SIZE / ((double)candidate_size+1);
	return node_score;
}

void PlanGenerator::InsertVarScanToCache(unsigned var_id, PlanTree* var_scan_plan) {
	list<PlanTree *> this_node_plan{var_scan_plan};
	if (plan_cache.empty()) {
		map<vector<unsigned>, list<PlanTree *>> one_node_plan_map;
		one_node_plan_map.insert(make_pair(vector<unsigned>{var_id}, this_node_plan));
		plan_cache.push_back(one_node_plan_map);
	} else {
		plan_cache[0].insert(make_pair(vector<unsigned>{var_id}, this_node_plan));
	}
}

void PlanGenerator::InsertVarNumToCache(unsigned int var_id) {
	if ((*id_caches).find(var_id) != (*id_caches).end()) {
		// todo: estimate
		var_to_num_map[var_id] = (*id_caches)[var_id]->size();
		var_sampled_from_candidate[var_id] = true;
	} else {
		var_to_num_map[var_id] = GetCandidateSizeFromWholeDB(var_id);
		var_sampled_from_candidate[var_id] = false;
	}
}

void PlanGenerator::InsertVarNumAndSampleToCache(unsigned int var_id) {
	vector<unsigned> need_insert_vec;
	if ((*id_caches).find(var_id) != (*id_caches).end()) {
		GetIdCacheSample((*id_caches)[var_id], need_insert_vec);
		// todo: estimate
		var_to_num_map[var_id] = (*id_caches)[var_id]->size();
		var_sampled_from_candidate[var_id] = true;
	} else {
		var_to_num_map[var_id] = GetSampleFromWholeDatabase(var_id, need_insert_vec);
		var_sampled_from_candidate[var_id] = false;
	}
	var_to_sample_cache[var_id] = std::move(need_insert_vec);
}

/**
 * Generate sample set of every var.
 * If a var has no const linked to it, then sample from the whole database.
 * Todo: SO_type var sample in so_var_to_sample_cache; pre_type var sample in pre_var_to_sample_cache.
 * Todo: should tell me whether the query only contain pre_var.
 */
void PlanGenerator::ConsiderVarScan(BGPQueryStrategy strategy) {
	// todo: directly use BGPQuery::so_var_id
	for(unsigned var_id : join_nodes) {
		auto var_descrip = bgpquery->get_vardescrip_by_id(var_id);
		vector<unsigned> this_node{var_id};
		switch (strategy) {
			case BGPQueryStrategy::Heuristic: {
				InsertVarNumToCache(var_id);
				break;
			}
			case BGPQueryStrategy::DP: {
				auto* new_scan = new PlanTree(var_id, bgpquery);
				InsertVarScanToCache(var_id, new_scan);
				InsertVarNumAndSampleToCache(var_id);
				new_scan->plan_cost = max((unsigned)1, var_to_num_map[var_id]/2);
				break;
			}
			default:{
				cout << "Error in PlanGenerator::ConsiderVarScan, unknown BGPQueryStrategy!" << endl;
				assert(false);
			}
		}
	}
}

unsigned PlanGenerator::HeuristicFirstNode() {
	unsigned first_node_id = UINT_MAX;
	double first_node_score = 0;
	for (unsigned int join_node : join_nodes) {
		// todo: change node_index to node_id in NodeScore func
		double this_node_score = NodeScore(join_node);
		if(this_node_score > first_node_score) {
			first_node_id = join_node;
			first_node_score = this_node_score;
		}
	}
	return first_node_id;
}

// Choose next join node from last_node' neighbors.
// The next_node must not be a satellite node.
// If no such node exists, then return UINT_MAX.
unsigned PlanGenerator::HeuristicNextNode(unsigned last_node_id) {
	auto var_descrip = bgpquery->get_vardescrip_by_id(last_node_id);
	unsigned best_next_node_id = UINT_MAX;
	double best_next_node_score = 0.0;
	for (unsigned neighbor_index = 0; neighbor_index < var_descrip->degree_; ++neighbor_index) {
		if (var_descrip->so_edge_nei_type_[neighbor_index] == VarDescriptor::EntiType::ConEntiType) continue;
		unsigned neighbor_var_id = var_descrip->so_edge_nei_[neighbor_index];
		if (find(plan_var_vec.begin(), plan_var_vec.end(), neighbor_var_id) != plan_var_vec.end()) continue;
		if (bgpquery->is_var_satellite_by_id(neighbor_var_id)) continue;
		double neighbor_score = NodeScore(neighbor_var_id);
		if (neighbor_score > best_next_node_score) {
			best_next_node_id = neighbor_var_id;
			best_next_node_score = neighbor_score;
		}
	}
	return best_next_node_id;
}

unsigned PlanGenerator::HeuristicNextNodeFromVec(const set<unsigned> &neighbor_nodes) {
    unsigned next_node_id = UINT_MAX;
    double next_node_score = 0;
    for (auto node : neighbor_nodes) {
        double this_node_score = NodeScore(node);
        if (this_node_score > next_node_score) {
            next_node_id = node;
            next_node_score = this_node_score;
        }
    }
    return next_node_id;
}

void PlanGenerator::RemoveNodeAddNeighbor(unsigned node_id, set<unsigned> &neighbor_nodes) {
    auto var_descrip = bgpquery->get_vardescrip_by_id(node_id);
    for (unsigned i = 0; i < var_descrip->degree_; ++i) {
        if (var_descrip->so_edge_nei_type_[i] == VarDescriptor::EntiType::ConEntiType) continue;
        if (find(plan_var_vec.begin(), plan_var_vec.end(), var_descrip->so_edge_nei_[i]) == plan_var_vec.end() &&
            find(join_nodes.begin(), join_nodes.end(), var_descrip->so_edge_nei_[i]) != join_nodes.end())
            neighbor_nodes.insert(var_descrip->so_edge_nei_[i]);
    }
    neighbor_nodes.erase(node_id);
}

// 1. Scan all vars, count join nodes num.
// 2. Choose first node.
// 3. While there are remaining join nodes, add one node.
// 4. Add satellite nodes.
PlanTree *PlanGenerator::HeuristicPlan(bool use_binary_join) {
	ConsiderVarScan(BGPQueryStrategy::Heuristic);

	unsigned first_node_id = HeuristicFirstNode();
	plan_var_vec.emplace_back(first_node_id);
	plan_var_degree.emplace_back(0);

    set<unsigned> neighbor_nodes{first_node_id};
    PlanTree *plan = nullptr;
    PlanTree *temp_plan = nullptr;

    while (!neighbor_nodes.empty()) {
        unsigned this_node_id = HeuristicNextNodeFromVec(neighbor_nodes);
        if (plan == nullptr)
            plan = new PlanTree(this_node_id, bgpquery);
        else {
            temp_plan = new PlanTree(plan, bgpquery, this_node_id, false);
            swap(temp_plan, plan);
            plan_var_vec.emplace_back(this_node_id);
            plan_var_degree.emplace_back(1);
			if (temp_plan != nullptr)
			{
				delete temp_plan;
				temp_plan = nullptr;
			}
        }
        RemoveNodeAddNeighbor(this_node_id, neighbor_nodes);
    }

	AddSatelliteNode(plan);

	list<PlanTree *> this_query_plan{plan};
	map<vector<unsigned>, list<PlanTree *>> plan_map = {{vector<unsigned>{0}, this_query_plan}};
	plan_cache.emplace_back(plan_map);

	return plan;
}

PlanTree *PlanGenerator::DPPlan(bool use_binary_join) {
	ConsiderVarScan(BGPQueryStrategy::DP);

	// should be var num not include satellite node
	// should not include pre_var num
	for(unsigned var_num = 2; var_num <= join_nodes.size(); ++var_num) {
		ConsiderWCOJoin(var_num);
		if(use_binary_join)
			if(var_num >= 5)
				ConsiderBinaryJoin(var_num);
	}
	PlanTree* best_plan = GetBestPlanByNum(join_nodes.size());

	// todo: 这个卫星点应该也有卫星谓词变量
	// s ?p ?o. 在之前的计划中已经加入了?o, 则这一步也需要加入?p
	AddSatelliteNode(best_plan);
	return best_plan;
}

PlanTree *PlanGenerator::GetPlan(bool use_binary_join) {
	if (bgpquery->get_triple_num() == 1) return GetSpecialOneTriplePlan();
	switch (PlanStrategy(use_binary_join)) {
		case BGPQueryStrategy::Heuristic:
			return HeuristicPlan(use_binary_join);
		case BGPQueryStrategy::DP:
			return DPPlan(use_binary_join);
		default:
			cout << "Error in PlanGenerator::get_plan, query strategy error!" << endl;
			assert(false);
	}
}

PlanTree *PlanGenerator::GetSpecialOneTriplePlan() {
	bool s_is_var = !(bgpquery->s_is_constant_[0]);
	bool p_is_var = !(bgpquery->p_is_constant_[0]);
	bool o_is_var = !(bgpquery->o_is_constant_[0]);
	unsigned var_num = bgpquery->get_total_var_num();

	PlanTree* return_plan_tree = nullptr;
	if(var_num == 3){
		auto edge_info = make_shared<vector<EdgeInfo>>();
		edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], JoinMethod::p2so);

		auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
		edge_constant_info->emplace_back(false, false, false);

		auto plan_node = make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::GetAllTriples,
													make_shared<AffectOneNode>(bgpquery->p_id_[0], edge_info, edge_constant_info),
													nullptr, nullptr, bgpquery->distinct_query);
		return_plan_tree = new PlanTree(plan_node);
	} else{
		JoinMethod join_method = GetJoinStrategy(s_is_var, p_is_var, o_is_var, var_num);

		auto first_var = bgpquery->get_vardescrip_by_index(0);

		switch (join_method) {
			case JoinMethod::sp2o:
			case JoinMethod::po2s:
			case JoinMethod::so2p: {
				auto edge_info = make_shared<vector<EdgeInfo>>();
				edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], join_method);
				auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
				edge_constant_info->emplace_back(!s_is_var, !p_is_var, !o_is_var);
				auto plan_node = make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode,
															make_shared<AffectOneNode>(first_var->id_, edge_info, edge_constant_info),
															nullptr, nullptr, bgpquery->distinct_query);
				return_plan_tree = new PlanTree(plan_node);
				break;
			}
			case JoinMethod::s2po: {
				auto edge_info = make_shared<EdgeInfo>(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0] , join_method);
				auto edge_constant_info = make_shared<EdgeConstantInfo>(!s_is_var, !p_is_var, !o_is_var);
				auto plan_node = make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::TwoNode, nullptr,
															make_shared<AffectTwoNode>(bgpquery->p_id_[0], bgpquery->o_id_[0], *edge_info, *edge_constant_info),
															nullptr, bgpquery->distinct_query);
				return_plan_tree = new PlanTree(plan_node);
				break;
			}
			case JoinMethod::p2so: {
				auto edge_info = make_shared<EdgeInfo>(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0] , join_method);
				auto edge_constant_info = make_shared<EdgeConstantInfo>(!s_is_var, !p_is_var, !o_is_var);
				auto plan_node = make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::TwoNode, nullptr,
															make_shared<AffectTwoNode>(bgpquery->s_id_[0], bgpquery->o_id_[0], *edge_info, *edge_constant_info),
															nullptr, bgpquery->distinct_query);
				return_plan_tree = new PlanTree(plan_node);
				break;
			}
			case JoinMethod::o2ps: {
				auto edge_info = make_shared<EdgeInfo>(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0] ,join_method);
				auto edge_constant_info = make_shared<EdgeConstantInfo>(!s_is_var, !p_is_var, !o_is_var);
				auto plan_node = make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::TwoNode, nullptr,
															make_shared<AffectTwoNode>(bgpquery->p_id_[0], bgpquery->s_id_[0], *edge_info, *edge_constant_info),
															nullptr, bgpquery->distinct_query);
				return_plan_tree = new PlanTree(plan_node);
				break;
			}
			case JoinMethod::p2s: {
				auto edge_info = make_shared<vector<EdgeInfo>>();
				edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], join_method);
				auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
				edge_constant_info->emplace_back(!s_is_var, !p_is_var, !o_is_var);
				auto plan_node = make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode,
															make_shared<AffectOneNode>(bgpquery->s_id_[0], edge_info, edge_constant_info),
															nullptr, nullptr, bgpquery->distinct_query);
				return_plan_tree = new PlanTree(plan_node);
				break;
			}
			case JoinMethod::p2o: {
				auto edge_info = make_shared<vector<EdgeInfo>>();
				edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], join_method);
				auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
				edge_constant_info->emplace_back(!s_is_var, !p_is_var, !o_is_var);
				auto plan_node = make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode,
															make_shared<AffectOneNode>(bgpquery->o_id_[0], edge_info, edge_constant_info),
															nullptr, nullptr, bgpquery->distinct_query);
				return_plan_tree = new PlanTree(plan_node);
				break;
			}
			case JoinMethod::s2p: {
				auto edge_info = make_shared<vector<EdgeInfo>>();
				edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], join_method);
				auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
				edge_constant_info->emplace_back(!s_is_var, !p_is_var, !o_is_var);
				auto plan_node = make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode,
															make_shared<AffectOneNode>(bgpquery->p_id_[0], edge_info, edge_constant_info),
															nullptr, nullptr, bgpquery->distinct_query);
				return_plan_tree = new PlanTree(plan_node);
				break;
			}
			case JoinMethod::s2o: {
				auto edge_info = make_shared<vector<EdgeInfo>>();
				edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], join_method);
				auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
				edge_constant_info->emplace_back(!s_is_var, !p_is_var, !o_is_var);
				auto plan_node = make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode,
															make_shared<AffectOneNode>(bgpquery->o_id_[0], edge_info, edge_constant_info),
															nullptr, nullptr, bgpquery->distinct_query);
				return_plan_tree = new PlanTree(plan_node);
				break;
			}
			case JoinMethod::o2s: {
				auto edge_info = make_shared<vector<EdgeInfo>>();
				edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], join_method);
				auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
				edge_constant_info->emplace_back(!s_is_var, !p_is_var, !o_is_var);
				auto plan_node = make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode,
															make_shared<AffectOneNode>(bgpquery->s_id_[0], edge_info, edge_constant_info),
															nullptr, nullptr, bgpquery->distinct_query);
				return_plan_tree = new PlanTree(plan_node);
				break;
			}
			case JoinMethod::o2p: {
				auto edge_info = make_shared<vector<EdgeInfo>>();
				edge_info->emplace_back(bgpquery->s_id_[0], bgpquery->p_id_[0], bgpquery->o_id_[0], join_method);
				auto edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
				edge_constant_info->emplace_back(!s_is_var, !p_is_var, !o_is_var);
				auto plan_node = make_shared<StepOperation>(StepOperation::StepOpType::Extend, StepOperation::OpRangeType::OneNode,
															make_shared<AffectOneNode>(bgpquery->p_id_[0], edge_info, edge_constant_info),
															nullptr, nullptr, bgpquery->distinct_query);
				return_plan_tree = new PlanTree(plan_node);
				break;
			}
			default: {
				cout << "error: joinmethod error" << endl;
				exit(-1);
			}
		}
	}
	list<PlanTree *> this_query_plan{return_plan_tree};
	map<vector<unsigned>, list<PlanTree *>> plan_map = {{vector<unsigned>{0}, this_query_plan}};
	plan_cache.emplace_back(plan_map);

	return return_plan_tree;
}


// Codes belows for TOPK

void PlanGenerator::GetIdCacheSample(shared_ptr<IDList> &so_cache, vector<unsigned int> &so_sample_cache) {
	random_device rd;
	mt19937 eng(rd());

	auto cache_size = so_cache->size();

	if (cache_size <= SAMPLE_CACHE_MAX) {
		so_sample_cache.assign(so_cache->getList()->begin(), so_cache->getList()->end());
	} else {
		unsigned sample_size = GetSampleSize(cache_size);
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

double PlanGenerator::EstimateOneEdgeSelectivity(TYPE_PREDICATE_ID pre_id, bool pre_constant, KVstore *kvstore,
												 shared_ptr<IDList> &s_cache, shared_ptr<IDList> &o_cache) {
	vector<unsigned> s_sample_cache;
	vector<unsigned> o_sample_cache;

	unsigned pass_num = 0;
	unsigned small_cache_size;

	if(s_cache->size() < o_cache->size()){
		GetIdCacheSample(s_cache, s_sample_cache);
		small_cache_size = s_cache->size();
	}
	else{
		GetIdCacheSample(o_cache, o_sample_cache);
		small_cache_size = o_cache->size();
	}

	if(pre_constant){
		if(s_cache->size() < o_cache->size()){
			for (const unsigned int &var_sample : s_sample_cache) {
				unsigned *o_list = nullptr;
				unsigned o_list_len = 0;

				kvstore->getobjIDlistBysubIDpreID(var_sample, pre_id, o_list, o_list_len);

				for (unsigned j = 0; j < o_list_len; ++j)
					if (binary_search(o_cache->begin(), o_cache->end(), o_list[j]))
						pass_num += 1;

				delete[] o_list;
			}
		} else{
			for (const unsigned int &var_sample : o_sample_cache) {
				unsigned *s_list = nullptr;
				unsigned s_list_len = 0;

				kvstore->getsubIDlistByobjIDpreID(var_sample, pre_id, s_list, s_list_len);

				for (unsigned j = 0; j < s_list_len; ++j)
					if (binary_search(s_cache->begin(), s_cache->end(), s_list[j]))
						pass_num += 1;

				delete[] s_list;
			}
		}
	} else{
		if(s_cache->size() < o_cache->size()){
			for (const unsigned int &var_sample : s_sample_cache) {
				unsigned *o_list = nullptr;
				unsigned o_list_len = 0;

				kvstore->getobjIDlistBysubID(var_sample, o_list, o_list_len);

				for (unsigned j = 0; j < o_list_len; ++j)
					if (binary_search(o_cache->begin(), o_cache->end(), o_list[j]))
						pass_num += 1;

				delete[] o_list;
			}
		} else{
			for (const unsigned int &var_sample : o_sample_cache) {
				unsigned *s_list = nullptr;
				unsigned s_list_len = 0;

				kvstore->getsubIDlistByobjID(var_sample, s_list, s_list_len);

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

void PlanGenerator::print_plan_generator_info() const {
	cout << "----print plan_generator_info----" << endl;
	cout << "triple_num = " << triples_num << endl;
	cout << "limit_literal = " << limitID_literal << endl;
	cout << "limit_entity = " << limitID_entity << endl;
	cout << "limit_predicate = " << limitID_predicate << endl;
	cout << "-------print var and id--------" << endl;
	for(unsigned i = 0; i < bgpquery->var_vector.size(); ++i){
		cout << "\t" << bgpquery->get_vardescrip_by_index(i)->var_name_ << "\t\t" << bgpquery->get_vardescrip_by_index(i)->id_ << endl;
	}
}

void PlanGenerator::print_sample_info() {
	cout << "----print var_to_num_map----" << endl;
	for(auto var_num_pair : var_to_num_map){
		cout << "var: " << var_num_pair.first << ", num: " << var_num_pair.second << endl;
	}

	cout << "----print var_sampled_from_candidate----" << endl;
	for(auto var_sampled_pair : var_sampled_from_candidate){
		cout << "var: " << var_sampled_pair.first << ", sampled: " << (var_sampled_pair.second ? "true" : "false") << endl;
	}

	cout << "----print var_to_sample_cache----" << endl;
	for(const auto &var_sample_pair : var_to_sample_cache){
		cout << "var: " << var_sample_pair.first << ", sample_num: " << var_sample_pair.second.size() << endl;
	}

	cout << "----print s_o_list_average_size----" << endl;
	for(const auto &s_pair : s_o_list_average_size){
		for(auto o_pair : s_pair.second){
			cout << s_pair.first << " to " << o_pair.first << " average size: " << o_pair.second << endl;
		}
	}
}