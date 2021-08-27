/*=============================================================================
# Filename: Statistics.cpp
# Author: Linglin Yang
# Mail: fyulingi@gmail.com
=============================================================================*/

#include "Statistics.h"

Statistics::Statistics(const string &path, int limitID_pre):
		filename(path + "/statistics/"), pre_num(limitID_pre),
		type_pre_id(-1),
        one_edge_type_num(0), one_edge_type_pre_num(0),
        two_edges_type1_num(0), two_edges_type1_pre_num(0),
        two_edges_type2_num(0), two_edges_type2_pre_num(0),
        two_edges_type3_num(0), two_edges_type3_pre_num(0){}

// multimap 版本
bool Statistics::build_entity_to_type_unorder_map(KVstore *kv_store) {
//    先找到 <type> 这个谓词

    long t1 = Util::get_cur_time();
    unsigned *type_s_o_id_list = nullptr;
    unsigned type_s_o_id_list_len = 0;
    string type_pre = "type";
    for(TYPE_PREDICATE_ID pre_id= 0; pre_id < pre_num; pre_id++){
        if(kv_store->getPredicateByID(pre_id, true).find(type_pre) != kv_store->getPredicateByID(pre_id, true).npos){
//            "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>"

            this->type_pre_id = pre_id;
            cout << "type_pre_id: " << type_pre_id << "  " << kv_store->getPredicateByID(type_pre_id) << endl;

//            存储实体和其 type 的关系
            kv_store->getsubIDobjIDlistBypreID(pre_id, type_s_o_id_list, type_s_o_id_list_len, true, nullptr);
            break;
        }
    }

//    构造 entity-type unordered_mulitimap 方便查找
//    为什么是 multimap: 有些实体可能有多个 type, 例如 小明 是 '学生', 也是某门课的 '助教'
    for(unsigned i = 0; i < type_s_o_id_list_len; i += 2){
        entityid_to_typeid_map.insert(make_pair(type_s_o_id_list[i], type_s_o_id_list[i+1]));

        if(type_to_num_map.find(type_s_o_id_list[i+1]) == type_to_num_map.end()){
            type_to_num_map.insert(make_pair(type_s_o_id_list[i+1], 1));

            vector<TYPE_ENTITY_LITERAL_ID> this_type_sample_vector;
            this_type_sample_vector.push_back(type_s_o_id_list[i]);
            type_to_sample_map.insert(make_pair(type_s_o_id_list[i+1], this_type_sample_vector));
        }else{
            type_to_num_map[type_s_o_id_list[i+1]] += 1;

            if(type_to_sample_map[type_s_o_id_list[i+1]].size() < SAMPLE_NUM_UPBOUND) {
                type_to_sample_map[type_s_o_id_list[i+1]].push_back(type_s_o_id_list[i]);
            } else{
                if((double )rand() / (double)RAND_MAX < 1.0/type_to_num_map[type_s_o_id_list[i+1]]){
//                                    超出部分做替换
                    int index_to_replace = rand() % SAMPLE_NUM_UPBOUND;
                    type_to_sample_map[type_s_o_id_list[i+1]][index_to_replace] = type_s_o_id_list[i];
                }
            }
        }
    }

    delete[] type_s_o_id_list;
    long t2 = Util::get_cur_time();

    cout << "build statistics for type, used " << (t2 - t1) << "ms." << endl;
    return true;
}

//    获取单边的统计信息
bool Statistics::build_Statistics_for_one_edge_type(KVstore *kv_store) {

    long t1 = Util::get_cur_time();
    unsigned* id_list = nullptr;
    unsigned id_list_len = 0;
    for(TYPE_PREDICATE_ID pre_id = 0; pre_id < pre_num; pre_id++){
        kv_store->getsubIDobjIDlistBypreID(pre_id, id_list, id_list_len, true);
        for(unsigned i = 0; i < id_list_len; i += 2){

//            需要 s 和 o 都有 type
            if(entityid_to_typeid_map.count(id_list[i]) > 0 &&
               entityid_to_typeid_map.count(id_list[i+1]) > 0){

                S_to_O_id now_s_o_id(id_list[i], id_list[i+1]);
                pair<unordered_multimap<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>::iterator, unordered_multimap<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>::iterator> s_range = entityid_to_typeid_map.equal_range(id_list[i]);
                pair<unordered_multimap<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>::iterator, unordered_multimap<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>::iterator> o_range = entityid_to_typeid_map.equal_range(id_list[i+1]);

                for(auto s = s_range.first; s != s_range.second; ++s){
                    for(auto o = o_range.first; o != o_range.second; ++o){

//                        取出 type 的 id
                        S_to_O_id now_stype_to_otype(s->second, o->second);

                        if(one_edge_typeid_to_num.find(pre_id) == one_edge_typeid_to_num.end()){

                            one_edge_type_pre_num += 1;
                            one_edge_type_num += 1;
                            //                    auto a = std::make_shared<unordered_map<S_to_O_id, int, S_to_O_id::S_to_O_id_Hash>>(now_stype_to_otype, 1);
                            unordered_map<S_to_O_id, int, S_to_O_id::S_to_O_id_Hash> this_type_num;
                            this_type_num.insert(make_pair(now_stype_to_otype, 1));
                            one_edge_typeid_to_num.insert(make_pair(pre_id, this_type_num));

                            vector<S_to_O_id> stype_pretype_otype_sample_vector;
                            stype_pretype_otype_sample_vector.push_back(now_s_o_id);
                            unordered_map<S_to_O_id, vector<S_to_O_id>, S_to_O_id::S_to_O_id_Hash> this_type_sample;
                            this_type_sample.insert(make_pair(now_stype_to_otype, stype_pretype_otype_sample_vector));
                            one_edge_typeid_sample.insert(make_pair(pre_id, this_type_sample));

                        } else{

                            if(one_edge_typeid_to_num[pre_id].find(now_stype_to_otype) != one_edge_typeid_to_num[pre_id].end()){
                                one_edge_typeid_to_num[pre_id][now_stype_to_otype] += 1;
                                if(one_edge_typeid_sample[pre_id][now_stype_to_otype].size() < SAMPLE_NUM_UPBOUND) {
                                    one_edge_typeid_sample[pre_id][now_stype_to_otype].push_back(now_s_o_id);
                                } else{
                                    if((double )rand() / (double)RAND_MAX < 1.0/one_edge_typeid_to_num[pre_id][now_stype_to_otype]){
//                                    超出部分做替换
                                        int index_to_replace = rand() % SAMPLE_NUM_UPBOUND;
                                        one_edge_typeid_sample[pre_id][now_stype_to_otype][index_to_replace] = now_s_o_id;
                                    }
                                }
                            } else{
                                one_edge_type_num += 1;
                                one_edge_typeid_to_num[pre_id].insert(make_pair(now_stype_to_otype, 1));
                                vector<S_to_O_id> stype_pretype_otype_sample_vector;
                                stype_pretype_otype_sample_vector.push_back(now_s_o_id);
                                one_edge_typeid_sample[pre_id].insert(make_pair(now_stype_to_otype, stype_pretype_otype_sample_vector));
                            }
                        }
                    }
                }
            }
        }
        delete[] id_list;
    }

    long t2 = Util::get_cur_time();
    cout << "build statistics for one edge, used " << (t2 - t1) << "ms." << endl;
    return true;
}

//    type1:
//    (s1)-[p1]->(s2)-[p2]->(s3)
//    思路:
//    p2so 读出一条边, 再用 s2po 拓展 s2
bool Statistics::build_Statistics_for_twe_edges_type1(KVstore *kv_store) {

    long t1 = Util::get_cur_time();

    unsigned* p1_so_id_list = nullptr;
    unsigned p1_so_id_list_len = 0;
    for(TYPE_PREDICATE_ID pre_id = 0; pre_id < pre_num; pre_id++) {
        kv_store->getsubIDobjIDlistBypreID(pre_id, p1_so_id_list, p1_so_id_list_len, true);
        unsigned up_bound = (p1_so_id_list_len > 4000 ? (p1_so_id_list_len/25) : p1_so_id_list_len);
        if (up_bound < p1_so_id_list_len) {
            int now_num = 0;
            int increase_num = p1_so_id_list_len/(2*up_bound);
            unordered_set<unsigned> index;
            while (now_num < up_bound) {
                unsigned i = 2 * (rand() % (p1_so_id_list_len / 2));
                if (index.count(i)) {
                    continue;
                } else {
                    if (entityid_to_typeid_map.count(p1_so_id_list[i]) > 0 &&
                        entityid_to_typeid_map.count(p1_so_id_list[i + 1]) > 0) {
                        unsigned *po_list;
                        unsigned list_len = 0;
                        kv_store->getpreIDobjIDlistBysubID(p1_so_id_list[i + 1], po_list, list_len, true);
                        for (unsigned j = 0; j < list_len; j += 2) {
//                    找到有 type 的 (s3)
                            if (entityid_to_typeid_map.count(po_list[j + 1]) > 0) {
//                        这是这个函数的主体

                                TWO_EDGES_ENTITY_TYPE_ID now_two_edges_entity_id(p1_so_id_list[i], p1_so_id_list[i+1], po_list[j+1]);
                                auto s1_range = entityid_to_typeid_map.equal_range(p1_so_id_list[i]);
                                auto s2_range = entityid_to_typeid_map.equal_range(p1_so_id_list[i + 1]);
                                auto s3_range = entityid_to_typeid_map.equal_range(po_list[j + 1]);

                                for (auto s1 = s1_range.first; s1 != s1_range.second; ++s1) {
                                    for (auto s2 = s2_range.first; s2 != s2_range.second; ++s2) {
                                        for (auto s3 = s3_range.first; s3 != s3_range.second; ++s3) {

                                            TWO_PRE_ID now_two_pre_id(pre_id, po_list[j]);
                                            TWO_EDGES_ENTITY_TYPE_ID now_two_edges_entity_type_id(s1->second,
                                                                                                  s2->second,
                                                                                                  s3->second);

                                            if (two_edges_type1id_to_num.find(now_two_pre_id) ==
                                                two_edges_type1id_to_num.end()) {

                                                two_edges_type1_pre_num += 1;

                                                two_edges_type1_num += 1;

                                                unordered_map<TWO_EDGES_ENTITY_TYPE_ID, int, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash> this_type_num;
                                                this_type_num.insert(make_pair(now_two_edges_entity_type_id, increase_num));
                                                two_edges_type1id_to_num.insert(
                                                        make_pair(now_two_pre_id, this_type_num));

                                                vector<TWO_EDGES_ENTITY_TYPE_ID> this_type_sample_vector;
                                                this_type_sample_vector.push_back(now_two_edges_entity_id);
                                                unordered_map<TWO_EDGES_ENTITY_TYPE_ID, vector<TWO_EDGES_ENTITY_TYPE_ID>, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash> this_type_sample;
                                                this_type_sample.insert(make_pair(now_two_edges_entity_type_id, this_type_sample_vector));
                                                two_edges_type1id_sample.insert(make_pair(now_two_pre_id, this_type_sample));
                                            } else {
                                                if (two_edges_type1id_to_num[now_two_pre_id].find(
                                                        now_two_edges_entity_type_id) !=
                                                    two_edges_type1id_to_num[now_two_pre_id].end()) {
//                                            如果有这样的 two_pre_id
                                                    two_edges_type1id_to_num[now_two_pre_id][now_two_edges_entity_type_id] += increase_num;
                                                    if(two_edges_type1id_sample[now_two_pre_id][now_two_edges_entity_type_id].size() < SAMPLE_NUM_UPBOUND){
                                                        two_edges_type1id_sample[now_two_pre_id][now_two_edges_entity_type_id].push_back(now_two_edges_entity_id);
                                                    } else{
                                                        if((double )rand() / (double)RAND_MAX < two_edges_type1id_to_num[now_two_pre_id][now_two_edges_entity_type_id]){
//                                                    超出部分做替换
                                                            int index_to_replace = rand() % SAMPLE_NUM_UPBOUND;
                                                            two_edges_type1id_sample[now_two_pre_id][now_two_edges_entity_type_id][index_to_replace] = now_two_edges_entity_id;
                                                        }
                                                    }
                                                } else {
                                                    two_edges_type1_num += 1;

                                                    two_edges_type1id_to_num[now_two_pre_id].insert(
                                                            make_pair(now_two_edges_entity_type_id, increase_num));
                                                    vector<TWO_EDGES_ENTITY_TYPE_ID> sample_vector;
                                                    sample_vector.push_back(now_two_edges_entity_id);
                                                    two_edges_type1id_sample[now_two_pre_id].insert(make_pair(now_two_edges_entity_type_id, sample_vector));

                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        delete[] po_list;
                    }

                    now_num++;
                    index.insert(i);

                }
            }
        }else {
            for(unsigned i = 0; i < p1_so_id_list_len; i += 2){
//            找到 (s1)-[p1]->(s2)
                if (entityid_to_typeid_map.count(p1_so_id_list[i]) > 0 &&
                    entityid_to_typeid_map.count(p1_so_id_list[i + 1]) > 0) {
                    unsigned *po_list;
                    unsigned list_len = 0;
                    kv_store->getpreIDobjIDlistBysubID(p1_so_id_list[i + 1], po_list, list_len, true);
                    for(unsigned j = 0; j < list_len; j += 2) {
//                    找到有 type 的 (s3)
                        if(entityid_to_typeid_map.count(po_list[j + 1]) > 0) {
//                        这是这个函数的主体

                            TWO_EDGES_ENTITY_TYPE_ID now_two_edges_entity_id(p1_so_id_list[i], p1_so_id_list[i+1], po_list[j+1]);
                            auto s1_range = entityid_to_typeid_map.equal_range(p1_so_id_list[i]);
                            auto s2_range = entityid_to_typeid_map.equal_range(p1_so_id_list[i + 1]);
                            auto s3_range = entityid_to_typeid_map.equal_range(po_list[j + 1]);

                            for(auto s1 = s1_range.first; s1 != s1_range.second; ++s1) {
                                for(auto s2 = s2_range.first; s2 != s2_range.second; ++s2) {
                                    for(auto s3 = s3_range.first; s3 != s3_range.second; ++s3) {

                                        TWO_PRE_ID now_two_pre_id(pre_id, po_list[j]);
                                        TWO_EDGES_ENTITY_TYPE_ID now_two_edges_entity_type_id(s1->second, s2->second, s3->second);

                                        if (two_edges_type1id_to_num.find(now_two_pre_id) ==
                                            two_edges_type1id_to_num.end()) {

                                            two_edges_type1_pre_num += 1;

                                            two_edges_type1_num += 1;

                                            unordered_map<TWO_EDGES_ENTITY_TYPE_ID, int, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash> this_type_num;
                                            this_type_num.insert(make_pair(now_two_edges_entity_type_id, 1));
                                            two_edges_type1id_to_num.insert(
                                                    make_pair(now_two_pre_id, this_type_num));

                                            vector<TWO_EDGES_ENTITY_TYPE_ID> this_type_sample_vector;
                                            this_type_sample_vector.push_back(now_two_edges_entity_id);
                                            unordered_map<TWO_EDGES_ENTITY_TYPE_ID, vector<TWO_EDGES_ENTITY_TYPE_ID>, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash> this_type_sample;
                                            this_type_sample.insert(make_pair(now_two_edges_entity_type_id, this_type_sample_vector));
                                            two_edges_type1id_sample.insert(make_pair(now_two_pre_id, this_type_sample));
                                        } else {
                                            if (two_edges_type1id_to_num[now_two_pre_id].find(
                                                    now_two_edges_entity_type_id) !=
                                                two_edges_type1id_to_num[now_two_pre_id].end()) {
//                                            如果有这样的 two_pre_id
                                                two_edges_type1id_to_num[now_two_pre_id][now_two_edges_entity_type_id] += 1;
                                                if(two_edges_type1id_sample[now_two_pre_id][now_two_edges_entity_type_id].size() < SAMPLE_NUM_UPBOUND){
                                                    two_edges_type1id_sample[now_two_pre_id][now_two_edges_entity_type_id].push_back(now_two_edges_entity_id);
                                                } else{
                                                    if((double )rand() / (double)RAND_MAX < two_edges_type1id_to_num[now_two_pre_id][now_two_edges_entity_type_id]){
//                                                    超出部分做替换
                                                        int index_to_replace = rand() % SAMPLE_NUM_UPBOUND;
                                                        two_edges_type1id_sample[now_two_pre_id][now_two_edges_entity_type_id][index_to_replace] = now_two_edges_entity_id;
                                                    }
                                                }
                                            } else {
                                                two_edges_type1_num += 1;

                                                two_edges_type1id_to_num[now_two_pre_id].insert(
                                                        make_pair(now_two_edges_entity_type_id, 1));
                                                vector<TWO_EDGES_ENTITY_TYPE_ID> sample_vector;
                                                sample_vector.push_back(now_two_edges_entity_id);
                                                two_edges_type1id_sample[now_two_pre_id].insert(make_pair(now_two_edges_entity_type_id, sample_vector));

                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    delete[] po_list;
                }

            }
        }
        delete[] p1_so_id_list;

    }


    long t2 = Util::get_cur_time();
    cout << "two edges type1 statistics build, used: " << (t2 - t1) << "ms" << endl;
    return true;
}


////    type2:
////    (s2)<-[p1]-(s1)-[p2]->(s3)
bool Statistics::build_Statistics_for_twe_edges_type2(KVstore *kv_store) {

    long t1 = Util::get_cur_time();

    unsigned* p1_so_id_list = nullptr;
    unsigned p1_so_id_list_len = 0;
    for(TYPE_PREDICATE_ID pre_id = 0; pre_id < pre_num; pre_id++) {
        kv_store->getsubIDobjIDlistBypreID(pre_id, p1_so_id_list, p1_so_id_list_len, true);
        unsigned up_bound = (p1_so_id_list_len > 4000 ? (p1_so_id_list_len/25) : p1_so_id_list_len);
        if (up_bound < p1_so_id_list_len) {
            int now_num = 0;
            int increase_num = p1_so_id_list_len/(2*up_bound);
            unordered_set<unsigned> index;
            while (now_num < up_bound) {
                unsigned i = 2 * (rand() % (p1_so_id_list_len / 2));
                if (index.count(i)) {
                    continue;
                } else {
                    if (entityid_to_typeid_map.count(p1_so_id_list[i]) > 0 &&
                        entityid_to_typeid_map.count(p1_so_id_list[i + 1]) > 0) {
                        unsigned *po_list;
                        unsigned list_len = 0;
                        kv_store->getpreIDobjIDlistBysubID(p1_so_id_list[i], po_list, list_len, true);
                        for (unsigned j = 0; j < list_len; j += 2) {
//                    找到有 type 的 (s3)
                            if (entityid_to_typeid_map.count(po_list[j + 1]) > 0) {
//                        这是这个函数的主体

                                TWO_EDGES_ENTITY_TYPE_ID now_two_edges_entity_id(p1_so_id_list[i], p1_so_id_list[i+1], po_list[j+1]);
                                auto s1_range = entityid_to_typeid_map.equal_range(p1_so_id_list[i]);
                                auto s2_range = entityid_to_typeid_map.equal_range(p1_so_id_list[i + 1]);
                                auto s3_range = entityid_to_typeid_map.equal_range(po_list[j + 1]);

                                for (auto s1 = s1_range.first; s1 != s1_range.second; ++s1) {
                                    for (auto s2 = s2_range.first; s2 != s2_range.second; ++s2) {
                                        for (auto s3 = s3_range.first; s3 != s3_range.second; ++s3) {

                                            TWO_PRE_ID now_two_pre_id(pre_id, po_list[j]);
                                            TWO_EDGES_ENTITY_TYPE_ID now_two_edges_entity_type_id(s1->second,
                                                                                                  s2->second,
                                                                                                  s3->second);

                                            if (two_edges_type2id_to_num.find(now_two_pre_id) ==
                                                two_edges_type2id_to_num.end()) {

                                                two_edges_type2_pre_num += 1;

                                                two_edges_type2_num += 1;

                                                unordered_map<TWO_EDGES_ENTITY_TYPE_ID, int, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash> this_type_num;
                                                this_type_num.insert(make_pair(now_two_edges_entity_type_id, increase_num));
                                                two_edges_type2id_to_num.insert(
                                                        make_pair(now_two_pre_id, this_type_num));

                                                vector<TWO_EDGES_ENTITY_TYPE_ID> this_type_sample_vector;
                                                this_type_sample_vector.push_back(now_two_edges_entity_id);
                                                unordered_map<TWO_EDGES_ENTITY_TYPE_ID, vector<TWO_EDGES_ENTITY_TYPE_ID>, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash> this_type_sample;
                                                this_type_sample.insert(make_pair(now_two_edges_entity_type_id, this_type_sample_vector));
                                                two_edges_type2id_sample.insert(make_pair(now_two_pre_id, this_type_sample));
                                            } else {
                                                if (two_edges_type2id_to_num[now_two_pre_id].find(
                                                        now_two_edges_entity_type_id) !=
                                                    two_edges_type2id_to_num[now_two_pre_id].end()) {
//                                            如果有这样的 two_pre_id
                                                    two_edges_type2id_to_num[now_two_pre_id][now_two_edges_entity_type_id] += increase_num;
                                                    if(two_edges_type2id_sample[now_two_pre_id][now_two_edges_entity_type_id].size() < SAMPLE_NUM_UPBOUND){
                                                        two_edges_type2id_sample[now_two_pre_id][now_two_edges_entity_type_id].push_back(now_two_edges_entity_id);
                                                    } else{
                                                        if((double )rand() / (double)RAND_MAX < two_edges_type2id_to_num[now_two_pre_id][now_two_edges_entity_type_id]){
//                                                    超出部分做替换
                                                            int index_to_replace = rand() % SAMPLE_NUM_UPBOUND;
                                                            two_edges_type2id_sample[now_two_pre_id][now_two_edges_entity_type_id][index_to_replace] = now_two_edges_entity_id;
                                                        }
                                                    }
                                                } else {
                                                    two_edges_type2_num += 1;

                                                    two_edges_type2id_to_num[now_two_pre_id].insert(
                                                            make_pair(now_two_edges_entity_type_id, increase_num));
                                                    vector<TWO_EDGES_ENTITY_TYPE_ID> sample_vector;
                                                    sample_vector.push_back(now_two_edges_entity_id);
                                                    two_edges_type2id_sample[now_two_pre_id].insert(make_pair(now_two_edges_entity_type_id, sample_vector));

                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        delete[] po_list;
                    }

                    now_num++;
                    index.insert(i);

                }
            }
        }else {
            for(unsigned i = 0; i < p1_so_id_list_len; i += 2){
//            找到 (s1)-[p1]->(s2)
                if (entityid_to_typeid_map.count(p1_so_id_list[i]) > 0 &&
                    entityid_to_typeid_map.count(p1_so_id_list[i + 1]) > 0) {
                    unsigned *po_list;
                    unsigned list_len = 0;
                    kv_store->getpreIDobjIDlistBysubID(p1_so_id_list[i + 1], po_list, list_len, true);
                    for (unsigned j = 0; j < list_len; j += 2) {
//                    找到有 type 的 (s3)
                        if (entityid_to_typeid_map.count(po_list[j + 1]) > 0) {
//                        这是这个函数的主体

                            TWO_EDGES_ENTITY_TYPE_ID now_two_edges_entity_id(p1_so_id_list[i], p1_so_id_list[i+1], po_list[j+1]);
                            auto s1_range = entityid_to_typeid_map.equal_range(p1_so_id_list[i]);
                            auto s2_range = entityid_to_typeid_map.equal_range(p1_so_id_list[i + 1]);
                            auto s3_range = entityid_to_typeid_map.equal_range(po_list[j + 1]);

                            for (auto s1 = s1_range.first; s1 != s1_range.second; ++s1) {
                                for (auto s2 = s2_range.first; s2 != s2_range.second; ++s2) {
                                    for (auto s3 = s3_range.first; s3 != s3_range.second; ++s3) {

                                        TWO_PRE_ID now_two_pre_id(pre_id, po_list[j]);
                                        TWO_EDGES_ENTITY_TYPE_ID now_two_edges_entity_type_id(s1->second, s2->second, s3->second);

                                        if (two_edges_type2id_to_num.find(now_two_pre_id) ==
                                            two_edges_type2id_to_num.end()) {

                                            two_edges_type2_pre_num += 1;

                                            two_edges_type2_num += 1;

                                            unordered_map<TWO_EDGES_ENTITY_TYPE_ID, int, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash> this_type_num;
                                            this_type_num.insert(make_pair(now_two_edges_entity_type_id, 1));
                                            two_edges_type2id_to_num.insert(
                                                    make_pair(now_two_pre_id, this_type_num));

                                            vector<TWO_EDGES_ENTITY_TYPE_ID> this_type_sample_vector;
                                            this_type_sample_vector.push_back(now_two_edges_entity_id);
                                            unordered_map<TWO_EDGES_ENTITY_TYPE_ID, vector<TWO_EDGES_ENTITY_TYPE_ID>, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash> this_type_sample;
                                            this_type_sample.insert(make_pair(now_two_edges_entity_type_id, this_type_sample_vector));
                                            two_edges_type2id_sample.insert(make_pair(now_two_pre_id, this_type_sample));
                                        } else {
                                            if (two_edges_type2id_to_num[now_two_pre_id].find(
                                                    now_two_edges_entity_type_id) !=
                                                two_edges_type2id_to_num[now_two_pre_id].end()) {
//                                            如果有这样的 two_pre_id
                                                two_edges_type2id_to_num[now_two_pre_id][now_two_edges_entity_type_id] += 1;
                                                if(two_edges_type2id_sample[now_two_pre_id][now_two_edges_entity_type_id].size() < SAMPLE_NUM_UPBOUND){
                                                    two_edges_type2id_sample[now_two_pre_id][now_two_edges_entity_type_id].push_back(now_two_edges_entity_id);
                                                } else{
                                                    if((double )rand() / (double)RAND_MAX < two_edges_type2id_to_num[now_two_pre_id][now_two_edges_entity_type_id]){
//                                                    超出部分做替换
                                                        int index_to_replace = rand() % SAMPLE_NUM_UPBOUND;
                                                        two_edges_type2id_sample[now_two_pre_id][now_two_edges_entity_type_id][index_to_replace] = now_two_edges_entity_id;
                                                    }
                                                }
                                            } else {
                                                two_edges_type2_num += 1;

                                                two_edges_type2id_to_num[now_two_pre_id].insert(
                                                        make_pair(now_two_edges_entity_type_id, 1));
                                                vector<TWO_EDGES_ENTITY_TYPE_ID> sample_vector;
                                                sample_vector.push_back(now_two_edges_entity_id);
                                                two_edges_type2id_sample[now_two_pre_id].insert(make_pair(now_two_edges_entity_type_id, sample_vector));

                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    delete[] po_list;
                }

            }
        }
        delete[] p1_so_id_list;

    }


    long t2 = Util::get_cur_time();
    cout << "two edges type2 statistics build, used: " << (t2 - t1) << "ms" << endl;
    return true;
}


////    type3:
////    (s1)-[p1]->(s2)<-[p2]-(s3)
bool Statistics::build_Statistics_for_twe_edges_type3(KVstore *kv_store) {

    long t1 = Util::get_cur_time();

    unsigned* p1_so_id_list = nullptr;
    unsigned p1_so_id_list_len = 0;
    for(TYPE_PREDICATE_ID pre_id = 0; pre_id < pre_num; pre_id++) {
        kv_store->getsubIDobjIDlistBypreID(pre_id, p1_so_id_list, p1_so_id_list_len, true);
        unsigned up_bound = (p1_so_id_list_len > 4000 ? (p1_so_id_list_len/25) : p1_so_id_list_len);
        if (up_bound < p1_so_id_list_len) {
            int now_num = 0;
            int increase_num = p1_so_id_list_len/(2*up_bound);
            unordered_set<unsigned> index;
            while (now_num < up_bound) {
                unsigned i = 2 * (rand() % (p1_so_id_list_len / 2));
                if (index.count(i)) {
                    continue;
                } else {
                    if (entityid_to_typeid_map.count(p1_so_id_list[i]) > 0 &&
                        entityid_to_typeid_map.count(p1_so_id_list[i + 1]) > 0) {
                        unsigned *ps_list;
                        unsigned list_len = 0;
                        kv_store->getpreIDsubIDlistByobjID(p1_so_id_list[i + 1], ps_list, list_len, true);
                        for (unsigned j = 0; j < list_len; j += 2) {
//                    找到有 type 的 (s3)
                            if (entityid_to_typeid_map.count(ps_list[j + 1]) > 0) {
//                        这是这个函数的主体

                                TWO_EDGES_ENTITY_TYPE_ID now_two_edges_entity_id(p1_so_id_list[i], p1_so_id_list[i+1], ps_list[j+1]);
                                auto s1_range = entityid_to_typeid_map.equal_range(p1_so_id_list[i]);
                                auto s2_range = entityid_to_typeid_map.equal_range(p1_so_id_list[i + 1]);
                                auto s3_range = entityid_to_typeid_map.equal_range(ps_list[j + 1]);

                                for (auto s1 = s1_range.first; s1 != s1_range.second; ++s1) {
                                    for (auto s2 = s2_range.first; s2 != s2_range.second; ++s2) {
                                        for (auto s3 = s3_range.first; s3 != s3_range.second; ++s3) {

                                            TWO_PRE_ID now_two_pre_id(pre_id, ps_list[j]);
                                            TWO_EDGES_ENTITY_TYPE_ID now_two_edges_entity_type_id(s1->second,
                                                                                                  s2->second,
                                                                                                  s3->second);

                                            if (two_edges_type3id_to_num.find(now_two_pre_id) ==
                                                two_edges_type3id_to_num.end()) {

                                                two_edges_type3_pre_num += 1;

                                                two_edges_type3_num += 1;

                                                unordered_map<TWO_EDGES_ENTITY_TYPE_ID, int, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash> this_type_num;
                                                this_type_num.insert(make_pair(now_two_edges_entity_type_id, increase_num));
                                                two_edges_type3id_to_num.insert(
                                                        make_pair(now_two_pre_id, this_type_num));

                                                vector<TWO_EDGES_ENTITY_TYPE_ID> this_type_sample_vector;
                                                this_type_sample_vector.push_back(now_two_edges_entity_id);
                                                unordered_map<TWO_EDGES_ENTITY_TYPE_ID, vector<TWO_EDGES_ENTITY_TYPE_ID>, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash> this_type_sample;
                                                this_type_sample.insert(make_pair(now_two_edges_entity_type_id, this_type_sample_vector));
                                                two_edges_type3id_sample.insert(make_pair(now_two_pre_id, this_type_sample));
                                            } else {
                                                if (two_edges_type3id_to_num[now_two_pre_id].find(
                                                        now_two_edges_entity_type_id) !=
                                                    two_edges_type3id_to_num[now_two_pre_id].end()) {
//                                            如果有这样的 two_pre_id
                                                    two_edges_type3id_to_num[now_two_pre_id][now_two_edges_entity_type_id] += increase_num;
                                                    if(two_edges_type3id_sample[now_two_pre_id][now_two_edges_entity_type_id].size() < SAMPLE_NUM_UPBOUND){
                                                        two_edges_type3id_sample[now_two_pre_id][now_two_edges_entity_type_id].push_back(now_two_edges_entity_id);
                                                    } else{
                                                        if((double )rand() / (double)RAND_MAX < two_edges_type3id_to_num[now_two_pre_id][now_two_edges_entity_type_id]){
//                                                    超出部分做替换
                                                            int index_to_replace = rand() % SAMPLE_NUM_UPBOUND;
                                                            two_edges_type3id_sample[now_two_pre_id][now_two_edges_entity_type_id][index_to_replace] = now_two_edges_entity_id;
                                                        }
                                                    }
                                                } else {
                                                    two_edges_type3_num += 1;

                                                    two_edges_type3id_to_num[now_two_pre_id].insert(
                                                            make_pair(now_two_edges_entity_type_id, increase_num));
                                                    vector<TWO_EDGES_ENTITY_TYPE_ID> sample_vector;
                                                    sample_vector.push_back(now_two_edges_entity_id);
                                                    two_edges_type3id_sample[now_two_pre_id].insert(make_pair(now_two_edges_entity_type_id, sample_vector));

                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        delete[] ps_list;
                    }

                    now_num++;
                    index.insert(i);

                }
            }
        }else {
            for(unsigned i = 0; i < p1_so_id_list_len; i += 2){
//            找到 (s1)-[p1]->(s2)
                if (entityid_to_typeid_map.count(p1_so_id_list[i]) > 0 &&
                    entityid_to_typeid_map.count(p1_so_id_list[i + 1]) > 0) {
                    unsigned *ps_list;
                    unsigned list_len = 0;
                    kv_store->getpreIDsubIDlistByobjID(p1_so_id_list[i + 1], ps_list, list_len, true);
                    for (unsigned j = 0; j < list_len; j += 2) {
//                    找到有 type 的 (s3)
                        if (entityid_to_typeid_map.count(ps_list[j + 1]) > 0) {
//                        这是这个函数的主体

                            TWO_EDGES_ENTITY_TYPE_ID now_two_edges_entity_id(p1_so_id_list[i], p1_so_id_list[i+1], ps_list[j+1]);
                            auto s1_range = entityid_to_typeid_map.equal_range(p1_so_id_list[i]);
                            auto s2_range = entityid_to_typeid_map.equal_range(p1_so_id_list[i + 1]);
                            auto s3_range = entityid_to_typeid_map.equal_range(ps_list[j + 1]);

                            for (auto s1 = s1_range.first; s1 != s1_range.second; ++s1) {
                                for (auto s2 = s2_range.first; s2 != s2_range.second; ++s2) {
                                    for (auto s3 = s3_range.first; s3 != s3_range.second; ++s3) {

                                        TWO_PRE_ID now_two_pre_id(pre_id, ps_list[j]);
                                        TWO_EDGES_ENTITY_TYPE_ID now_two_edges_entity_type_id(s1->second, s2->second, s3->second);

                                        if (two_edges_type3id_to_num.find(now_two_pre_id) ==
                                            two_edges_type3id_to_num.end()) {

                                            two_edges_type3_pre_num += 1;

                                            two_edges_type3_num += 1;

                                            unordered_map<TWO_EDGES_ENTITY_TYPE_ID, int, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash> this_type_num;
                                            this_type_num.insert(make_pair(now_two_edges_entity_type_id, 1));
                                            two_edges_type3id_to_num.insert(
                                                    make_pair(now_two_pre_id, this_type_num));

                                            vector<TWO_EDGES_ENTITY_TYPE_ID> this_type_sample_vector;
                                            this_type_sample_vector.push_back(now_two_edges_entity_id);
                                            unordered_map<TWO_EDGES_ENTITY_TYPE_ID, vector<TWO_EDGES_ENTITY_TYPE_ID>, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash> this_type_sample;
                                            this_type_sample.insert(make_pair(now_two_edges_entity_type_id, this_type_sample_vector));
                                            two_edges_type3id_sample.insert(make_pair(now_two_pre_id, this_type_sample));
                                        } else {
                                            if (two_edges_type3id_to_num[now_two_pre_id].find(
                                                    now_two_edges_entity_type_id) !=
                                                two_edges_type3id_to_num[now_two_pre_id].end()) {
//                                            如果有这样的 two_pre_id
                                                two_edges_type3id_to_num[now_two_pre_id][now_two_edges_entity_type_id] += 1;
                                                if(two_edges_type3id_sample[now_two_pre_id][now_two_edges_entity_type_id].size() < SAMPLE_NUM_UPBOUND){
                                                    two_edges_type3id_sample[now_two_pre_id][now_two_edges_entity_type_id].push_back(now_two_edges_entity_id);
                                                } else{
                                                    if((double )rand() / (double)RAND_MAX < two_edges_type3id_to_num[now_two_pre_id][now_two_edges_entity_type_id]){
//                                                    超出部分做替换
                                                        int index_to_replace = rand() % SAMPLE_NUM_UPBOUND;
                                                        two_edges_type3id_sample[now_two_pre_id][now_two_edges_entity_type_id][index_to_replace] = now_two_edges_entity_id;
                                                    }
                                                }
                                            } else {
                                                two_edges_type3_num += 1;

                                                two_edges_type3id_to_num[now_two_pre_id].insert(
                                                        make_pair(now_two_edges_entity_type_id, 1));
                                                vector<TWO_EDGES_ENTITY_TYPE_ID> sample_vector;
                                                sample_vector.push_back(now_two_edges_entity_id);
                                                two_edges_type3id_sample[now_two_pre_id].insert(make_pair(now_two_edges_entity_type_id, sample_vector));

                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    delete[] ps_list;
                }

            }
        }
        delete[] p1_so_id_list;

    }


    long t2 = Util::get_cur_time();
    cout << "two edges type3 statistics build, used: " << (t2 - t1) << "ms" << endl;
    return true;
}

//    TODO: 若要优化 build 速度, 可以考虑将四个 build 合在一个循环
bool Statistics::build_Statistics(KVstore *kv_store) {

//    TODO: 这个东西可能很占内存, 直接读 sp2o 会不会比这个好？
    this->build_entity_to_type_unorder_map(kv_store);

    cout << "type to num map size: " << type_to_num_map.size() << endl;

//    cout << "build_entity_to_type_map success" << endl;

    if(!this->build_Statistics_for_one_edge_type(kv_store)){
        cout << "Build statistics for one edge type false!" << endl;
        return false;
    }

    cout << "one edge type count: " << one_edge_type_num << endl;

    if(!this->build_Statistics_for_twe_edges_type1(kv_store) || !this->build_Statistics_for_twe_edges_type2(kv_store)
       || !this->build_Statistics_for_twe_edges_type3(kv_store)){
        cout << "Build statistics for two edges type false!" << endl;
        return false;
    }

    cout << "two edges type1 count: " << two_edges_type1_num << endl;
    cout << "two edges type2 count: " << two_edges_type2_num << endl;
    cout << "two edges type3 count: " << two_edges_type3_num << endl;
    cout << "Build statistics success!" << endl;

    entityid_to_typeid_map.clear();

    return true;
}

bool Statistics::save_type_statistics(){

    long t1 = Util::get_cur_time();
    string type_num_file = filename + "type_num_map";
    string type_sample_file = filename + "type_sample";

    FILE *num_file = fopen(type_num_file.c_str(), "wb");
    FILE *sample_file = fopen(type_sample_file.c_str(), "wb");

    if(num_file == nullptr || sample_file == nullptr){
        cout << "error, cannot create type statistics save file" << endl;
        return false;
    }

    fwrite(&pre_num, sizeof(TYPE_PREDICATE_ID), 1, num_file);
    fwrite(&type_pre_id, sizeof(TYPE_PREDICATE_ID), 1, num_file);

    int size = type_to_num_map.size();
    fwrite(&size, sizeof(int), 1, num_file);


    for(auto & x : type_to_num_map){

        fwrite(&x.first, sizeof(TYPE_ENTITY_LITERAL_ID), 1, num_file);
        unsigned this_type_num = x.second;
        unsigned this_sample_size = type_to_sample_map[x.first].size();
        fwrite(&this_type_num, sizeof(unsigned ), 1, num_file);
        fwrite(&this_sample_size, sizeof(unsigned ), 1, num_file);

        for(unsigned y = 0; y < this_sample_size; ++y) {
            fwrite(&type_to_sample_map[x.first][y], sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
        }

    }

    fclose(num_file);
    fclose(sample_file);
    long t2 = Util::get_cur_time();
    cout << "save statistics for type, used " << (t2 - t1) << "ms." << endl;
    return true;
}

bool Statistics::save_one_edge_type_statistics() {

//    ./statistics/one_edge_type_num_map文件内容：
//    one_edge_type_num, one_edge_type_pre_num;
//    pre1_id, pre1_edge_n,
//    s1_type_id,  o1_type_id, s1_p1_o1_num, sample_num
//    s2_type_id,  o2_type_id, s2_p1_o2_num, ...
//
//    ./statistics/one_edge_type_sample文件内容：
//    p1_stype1_otype1_s1_id, p1_stype1_otype1_o1_id,
//    p1_stype1_otype1_s2_id, p1_stype1_otype1_o2_id, ...
//    注意: 因为 num_map 中已经存储了每个 p_stype_otype 对应抽样的数量, 故直接顺序存储


    long t1 = Util::get_cur_time();
    string one_edge_type_num_file = filename + "one_edge_type_num_map";
    string one_edge_type_sample_file = filename + "one_edge_type_sample";

    FILE *num_file = fopen(one_edge_type_num_file.c_str(), "wb");
    FILE *sample_file = fopen(one_edge_type_sample_file.c_str(), "wb");

    if(num_file == nullptr || sample_file == nullptr){
        cout << "error, cannot create one edge type statistics save file" << endl;
        return false;
    }


    fwrite(&one_edge_type_num, sizeof(int), 1, num_file);
    fwrite(&one_edge_type_pre_num, sizeof(int), 1, num_file);



    for(auto & x : one_edge_typeid_to_num){

        fwrite(&x.first, sizeof(TYPE_PREDICATE_ID), 1, num_file);
        int size = (int)(x.second.size());
        fwrite(&size, sizeof(int), 1, num_file);

        for(auto y = x.second.begin(); y != x.second.end(); ++y){
            fwrite(&y->first.s_type_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, num_file);
            fwrite(&y->first.o_type_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, num_file);
            fwrite(&y->second, sizeof(int), 1, num_file);
            int sample_size = one_edge_typeid_sample[x.first][y->first].size();
            fwrite(&sample_size, sizeof(int), 1, num_file);

            for(int i = 0; i < sample_size; ++i){
                fwrite(&(one_edge_typeid_sample[x.first][y->first][i].s_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
                fwrite(&(one_edge_typeid_sample[x.first][y->first][i].o_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
            }

//            可以通过以下语句输出结果, 需要传入 kvstore 指针
//            cout << kv_store->getEntityByID(y->first.s_type_id) << "  " << kv_store->getPredicateByID(x->first) << "  " << kv_store->getEntityByID(y->first.o_type_id) << "  " << y->second << endl;

        }
    }

    fclose(num_file);
    fclose(sample_file);
    long t2 = Util::get_cur_time();
    cout << "save statistics one edge, used " << (t2 - t1) << "ms." << endl;
    return true;
}

bool Statistics::save_two_edges_type1_statistics() {
//    ./statistics/two_edges_type1_num_map 文件内容:
//    two_edges_type1_num, two_edges_type1_pre_num,
//    pre1_id, pre2_id, pre1_pre2_edges_n,
//    s1_type_id, s2_type_id, s3_type_id, s1_s2_s3_p1_p2_num, s1_s2_s3_p1_p2_sample_num,
//    s4_type_id, s5_type_id, s6_type_id, s4_s5_s6_p1_p2_num, ...
//    注意:  pre_num, type_pre_id 没有存储, 因为已经存储在 ./statistics_one_edge_type_num_map 中

//    ./statistics/two_edges_type1_sample 文件内容:
//    p1_p2_s1_s2_s3_s1id, p1_p2_s1_s2_s3_s2id, p1_p2_s1_s2_s3_s3id,
//    p1_p2_s1_s2_s3_s4id, p1_p2_s1_s2_s3_s5id, p1_p2_s1_s2_s3_s6id,...
//    注意: 只存了抽样结果表, 该条抽样归于哪一类可以有 s1_s2_s3_p1_p2_sample_num 来判断
    long t1 = Util::get_cur_time();
    string two_edges_type1_num_file = filename + "two_edges_type1_num_map";
    string two_edges_type1_sample_file = filename + "two_edges_type1_sample";

    FILE *num_file = fopen(two_edges_type1_num_file.c_str(), "wb");
    FILE *sample_file = fopen(two_edges_type1_sample_file.c_str(), "wb");

    if(num_file == nullptr || sample_file == nullptr){
        cout << "error, cannot create one edge type statistics save file" << endl;
        return false;
    }

    fwrite(&two_edges_type1_num, sizeof(int), 1, num_file);
    fwrite(&two_edges_type1_pre_num, sizeof(int), 1, num_file);

    for(auto &x : two_edges_type1id_to_num ){

        fwrite(&x.first.p1_id, sizeof(TYPE_PREDICATE_ID), 1, num_file);
        fwrite(&x.first.p2_id, sizeof(TYPE_PREDICATE_ID), 1, num_file);
        int size = (int)x.second.size();
        fwrite(&size, sizeof(int), 1, num_file);

        for(auto y : x.second){
            fwrite(&y.first.s1_type_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, num_file);
            fwrite(&y.first.s2_type_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, num_file);
            fwrite(&y.first.s3_type_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, num_file);
            fwrite(&y.second, sizeof(int), 1, num_file);
            int sample_size = two_edges_type1id_sample[x.first][y.first].size();
            fwrite(&sample_size, sizeof(int), 1, num_file);

            for(int i = 0; i < sample_size; ++i){
                fwrite(&(two_edges_type1id_sample[x.first][y.first][i].s1_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
                fwrite(&(two_edges_type1id_sample[x.first][y.first][i].s2_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
                fwrite(&(two_edges_type1id_sample[x.first][y.first][i].s3_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
            }
//            cout << kv_store->getEntityByID(y.first.s1_type_id) << "  " << kv_store->getPredicateByID(x.first.p1_id) << "  " << kv_store->getEntityByID(y.first.s2_type_id) << "  " << kv_store->getPredicateByID(x.first.p2_id) << "  " << kv_store->getEntityByID(y.first.s3_type_id) << "  " << y.second << endl;


        }
    }

    fclose(num_file);
    fclose(sample_file);
    long t2 = Util::get_cur_time();
    cout << "save statistics type1, used " << (t2 - t1) << "ms." << endl;
    return true;
}

bool Statistics::save_two_edges_type2_statistics() {
//    ./statistics/two_edges_type2_num_map 文件内容:
//    two_edges_type2_num, two_edges_type2_pre_num,
//    pre1_id, pre2_id, pre1_pre2_edges_n,
//    s1_type_id, s2_type_id, s3_type_id, s1_s2_s3_p1_p2_num, s1_s2_s3_p1_p2_sample_num,
//    s4_type_id, s5_type_id, s6_type_id, s4_s5_s6_p1_p2_num, ...
//    注意:  pre_num, type_pre_id 没有存储, 因为已经存储在 ./statistics_one_edge_type_num_map 中

//    ./statistics/two_edges_type2_sample 文件内容:
//    p1_p2_s1_s2_s3_s1id, p1_p2_s1_s2_s3_s2id, p1_p2_s1_s2_s3_s3id,
//    p1_p2_s1_s2_s3_s4id, p1_p2_s1_s2_s3_s5id, p1_p2_s1_s2_s3_s6id,...

    long t1 = Util::get_cur_time();
    string two_edges_type2_num_file = filename + "two_edges_type2_num_map";
    string two_edges_type2_sample_file = filename + "two_edges_type2_sample";

    FILE *num_file = fopen(two_edges_type2_num_file.c_str(), "wb");
    FILE *sample_file = fopen(two_edges_type2_sample_file.c_str(), "wb");

    if(num_file == nullptr || sample_file == nullptr){
        cout << "error, cannot create one edge type statistics save file" << endl;
        return false;
    }

    fwrite(&two_edges_type2_num, sizeof(int), 1, num_file);
    fwrite(&two_edges_type2_pre_num, sizeof(int), 1, num_file);

    for(auto &x : two_edges_type2id_to_num ){

        fwrite(&x.first.p1_id, sizeof(TYPE_PREDICATE_ID), 1, num_file);
        fwrite(&x.first.p2_id, sizeof(TYPE_PREDICATE_ID), 1, num_file);
        int size = (int)x.second.size();
        fwrite(&size, sizeof(int), 1, num_file);

        for(auto y : x.second){
            fwrite(&y.first.s1_type_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, num_file);
            fwrite(&y.first.s2_type_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, num_file);
            fwrite(&y.first.s3_type_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, num_file);
            fwrite(&y.second, sizeof(int), 1, num_file);
            int sample_size = two_edges_type2id_sample[x.first][y.first].size();
            fwrite(&sample_size, sizeof(int), 1, num_file);

            for(int i = 0; i < two_edges_type2id_sample[x.first][y.first].size(); ++i){
                fwrite(&(two_edges_type2id_sample[x.first][y.first][i].s1_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
                fwrite(&(two_edges_type2id_sample[x.first][y.first][i].s2_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
                fwrite(&(two_edges_type2id_sample[x.first][y.first][i].s3_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
            }
//            cout << kv_store->getEntityByID(y.first.s1_type_id) << "  " << kv_store->getPredicateByID(x.first.p1_id) << "  " << kv_store->getEntityByID(y.first.s2_type_id) << "  " << kv_store->getPredicateByID(x.first.p2_id) << "  " << kv_store->getEntityByID(y.first.s3_type_id) << "  " << y.second << endl;


        }
    }

    fclose(num_file);
    fclose(sample_file);

    long t2 = Util::get_cur_time();
    cout << "save statistics type2, used " << (t2 - t1) << "ms." << endl;
    return true;
}


bool Statistics::save_two_edges_type3_statistics() {
//    ./statistics/two_edges_type3_num_map 文件内容:
//    two_edges_type3_num, two_edges_type3_pre_num,
//    pre1_id, pre2_id, pre1_pre2_edges_n,
//    s1_type_id, s2_type_id, s3_type_id, s1_s2_s3_p1_p2_num,
//    s4_type_id, s5_type_id, s6_type_id, s4_s5_s6_p1_p2_num, ...
//    注意:  pre_num, type_pre_id 没有存储, 因为已经存储在 ./statistics_one_edge_type_num_map 中

//    ./statistics/two_edges_type3_sample 文件内容:
//    p1_p2_s1_s2_s3_s1id, p1_p2_s1_s2_s3_s2id, p1_p2_s1_s2_s3_s3id,
//    p1_p2_s1_s2_s3_s4id, p1_p2_s1_s2_s3_s5id, p1_p2_s1_s2_s3_s6id,...

    long t1 = Util::get_cur_time();
    string two_edges_type3_num_file = filename + "two_edges_type3_num_map";
    string two_edges_type3_sample_file = filename + "two_edges_type3_sample";

    FILE *num_file = fopen(two_edges_type3_num_file.c_str(), "wb");
    FILE *sample_file = fopen(two_edges_type3_sample_file.c_str(), "wb");

    if(num_file == nullptr || sample_file == nullptr){
        cout << "error, cannot create one edge type statistics save file" << endl;
        return false;
    }

    fwrite(&two_edges_type3_num, sizeof(int), 1, num_file);
    fwrite(&two_edges_type3_pre_num, sizeof(int), 1, num_file);

    for(auto &x : two_edges_type3id_to_num ){

        fwrite(&x.first.p1_id, sizeof(TYPE_PREDICATE_ID), 1, num_file);
        fwrite(&x.first.p2_id, sizeof(TYPE_PREDICATE_ID), 1, num_file);
        int size = (int)x.second.size();
        fwrite(&size, sizeof(int), 1, num_file);

        for(auto y : x.second){
            fwrite(&y.first.s1_type_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, num_file);
            fwrite(&y.first.s2_type_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, num_file);
            fwrite(&y.first.s3_type_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, num_file);
            fwrite(&y.second, sizeof(int), 1, num_file);
            int sample_size = two_edges_type3id_sample[x.first][y.first].size();
            fwrite(&sample_size, sizeof(int), 1, num_file);

            for(int i = 0; i < two_edges_type3id_sample[x.first][y.first].size(); ++i){
                fwrite(&(two_edges_type3id_sample[x.first][y.first][i].s1_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
                fwrite(&(two_edges_type3id_sample[x.first][y.first][i].s2_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
                fwrite(&(two_edges_type3id_sample[x.first][y.first][i].s3_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
            }
//            cout << kv_store->getEntityByID(y.first.s1_type_id) << "  " << kv_store->getPredicateByID(x.first.p1_id) << "  " << kv_store->getEntityByID(y.first.s2_type_id) << "  " << kv_store->getPredicateByID(x.first.p2_id) << "  " << kv_store->getEntityByID(y.first.s3_type_id) << "  " << y.second << endl;


        }
    }

    fclose(num_file);
    fclose(sample_file);
    long t2 = Util::get_cur_time();
    cout << "save statistics type3, used " << (t2 - t1) << "ms." << endl;
    return true;
}

bool Statistics::save_Statistics() {

    Util::create_dir(filename);

    if(!save_type_statistics()){
        cout << "save type statistics false!" << endl;
        return false;
    }

    if(!save_one_edge_type_statistics()){
//if(!save_one_edge_type_statistics(kv_store)){
        cout << "save one edge type statistics false!" << endl;
        return false;
    }


    if(!save_two_edges_type1_statistics()){
//if(!save_two_edges_type1_statistics(kv_store)){
        cout << "save two edges type1 statistics false!" << endl;
        return false;
    }
    if(!save_two_edges_type2_statistics()){
        cout << "save two edges type2 statistics false!" << endl;
        return false;
    }
    if(!save_two_edges_type3_statistics()){
        cout << "save two edges type3 statistics false!" << endl;
        return false;
    }
    return true;
}

unsigned Statistics::get_type_num_by_type_id(TYPE_ENTITY_LITERAL_ID type_id) {
    if(type_to_num_map.find(type_id) != type_to_num_map.end()){
        return type_to_num_map[type_id];
    } else{
        return 0;
    }
}

int Statistics::get_type_one_edge_typeid_num_by_id(TYPE_ENTITY_LITERAL_ID s_type, TYPE_PREDICATE_ID p_type,
                                                   TYPE_ENTITY_LITERAL_ID o_type) {
    S_to_O_id s_to_o_id = {s_type, o_type};
    if(one_edge_typeid_to_num.find(p_type) != one_edge_typeid_to_num.end()){
        if(one_edge_typeid_to_num[p_type].find(s_to_o_id) != one_edge_typeid_to_num[p_type].end()) {
            return one_edge_typeid_to_num[p_type][s_to_o_id];
        }
    }
//        如果找不到, 则这样的 (s_type_id, pre_id, o_type_id) 数目为 0
    return 0;
}
int Statistics::get_type_one_edge_typeid_num_by_id(TYPE_PREDICATE_ID p_type, S_to_O_id s_o_type_id) {
    if(one_edge_typeid_to_num.find(p_type) != one_edge_typeid_to_num.end()){
        if(one_edge_typeid_to_num[p_type].find(s_o_type_id) != one_edge_typeid_to_num[p_type].end()){
            return one_edge_typeid_to_num[p_type][s_o_type_id];
        }
    }
    return 0;
}

int Statistics::get_type_two_edges_type1id_num_by_id(TYPE_ENTITY_LITERAL_ID s1_type, TYPE_PREDICATE_ID p1_type,
                                                     TYPE_ENTITY_LITERAL_ID s2_type, TYPE_PREDICATE_ID p2_type,
                                                     TYPE_ENTITY_LITERAL_ID s3_type) {
    TWO_PRE_ID this_two_pre_id(p1_type, p2_type);
    if(two_edges_type1id_to_num.find(this_two_pre_id) != two_edges_type1id_to_num.end()){
        TWO_EDGES_ENTITY_TYPE_ID this_two_edges_entity_id(s1_type, s2_type, s3_type);
        if(two_edges_type1id_to_num[this_two_pre_id].find(this_two_edges_entity_id) != two_edges_type1id_to_num[this_two_pre_id].end()){
            return two_edges_type1id_to_num[this_two_pre_id][this_two_edges_entity_id];
        }
    }
    return 0;
}
int Statistics::get_type_two_edges_type1id_num_by_id(TWO_PRE_ID p1_p2_id, TWO_EDGES_ENTITY_TYPE_ID s1_s2_s3_type_id) {
    if(two_edges_type1id_to_num.find(p1_p2_id) != two_edges_type1id_to_num.end()){
        if(two_edges_type1id_to_num[p1_p2_id].find(s1_s2_s3_type_id) != two_edges_type1id_to_num[p1_p2_id].end()){
            return two_edges_type1id_to_num[p1_p2_id][s1_s2_s3_type_id];
        }
    }
    return 0;
}

int Statistics::get_type_two_edges_type2id_num_by_id(TYPE_ENTITY_LITERAL_ID s1_type, TYPE_PREDICATE_ID p1_type,
                                                     TYPE_ENTITY_LITERAL_ID s2_type, TYPE_PREDICATE_ID p2_type,
                                                     TYPE_ENTITY_LITERAL_ID s3_type) {
    TWO_PRE_ID this_two_pre_id(p1_type, p2_type);
    if(two_edges_type2id_to_num.find(this_two_pre_id) != two_edges_type2id_to_num.end()){
        TWO_EDGES_ENTITY_TYPE_ID this_two_edges_entity_id(s1_type, s2_type, s3_type);
        if(two_edges_type2id_to_num[this_two_pre_id].find(this_two_edges_entity_id) != two_edges_type2id_to_num[this_two_pre_id].end()){
            return two_edges_type2id_to_num[this_two_pre_id][this_two_edges_entity_id];
        }
    }
    return 0;

}
int Statistics::get_type_two_edges_type2id_num_by_id(TWO_PRE_ID p1_p2_id, TWO_EDGES_ENTITY_TYPE_ID s1_s2_s3_type_id) {
    if(two_edges_type2id_to_num.find(p1_p2_id) != two_edges_type2id_to_num.end()){
        if(two_edges_type2id_to_num[p1_p2_id].find(s1_s2_s3_type_id) != two_edges_type2id_to_num[p1_p2_id].end()){
            return two_edges_type2id_to_num[p1_p2_id][s1_s2_s3_type_id];
        }
    }
    return 0;
}

int Statistics::get_type_two_edges_type3id_num_by_id(TYPE_ENTITY_LITERAL_ID s1_type, TYPE_PREDICATE_ID p1_type,
                                                     TYPE_ENTITY_LITERAL_ID s2_type, TYPE_PREDICATE_ID p2_type,
                                                     TYPE_ENTITY_LITERAL_ID s3_type) {
    TWO_PRE_ID this_two_pre_id(p1_type, p2_type);
    if(two_edges_type3id_to_num.find(this_two_pre_id) != two_edges_type3id_to_num.end()){
        TWO_EDGES_ENTITY_TYPE_ID this_two_edges_entity_id(s1_type, s2_type, s3_type);
        if(two_edges_type3id_to_num[this_two_pre_id].find(this_two_edges_entity_id) != two_edges_type3id_to_num[this_two_pre_id].end()){
            return two_edges_type3id_to_num[this_two_pre_id][this_two_edges_entity_id];
        }
    }
    return 0;
}
int Statistics::get_type_two_edges_type3id_num_by_id(TWO_PRE_ID p1_p2_id, TWO_EDGES_ENTITY_TYPE_ID s1_s2_s3_type_id) {
    if(two_edges_type3id_to_num.find(p1_p2_id) != two_edges_type3id_to_num.end()){
        if(two_edges_type3id_to_num[p1_p2_id].find(s1_s2_s3_type_id) != two_edges_type3id_to_num[p1_p2_id].end()){
            return two_edges_type3id_to_num[p1_p2_id][s1_s2_s3_type_id];
        }
    }
    return 0;
}

void Statistics::load_Statistics_for_type(){

    string type_num_map = filename + "type_num_map";
    string type_sample = filename + "type_sample";
    FILE *num_file = fopen(type_num_map.c_str(), "rb");
    FILE *sample_file = fopen(type_sample.c_str(), "rb");

    fread(&pre_num, sizeof(int), 1, num_file);
    fread(&type_pre_id, sizeof(TYPE_PREDICATE_ID), 1, num_file);

    int type_num;
    fread(&type_num, sizeof(int), 1, num_file);

    for(int i = 0; i < type_num; ++i){

        TYPE_ENTITY_LITERAL_ID now_type[3];
        fread(now_type, sizeof(TYPE_ENTITY_LITERAL_ID), 3, num_file);

        type_to_num_map[now_type[0]] = now_type[1];

        vector<TYPE_ENTITY_LITERAL_ID> this_type_sample(now_type[2]);
        for(unsigned j = 0; j < now_type[2]; ++j){
            unsigned this_sample;
            fread(&this_sample, sizeof(unsigned ), 1, sample_file);
            this_type_sample.push_back(this_sample);
        }
        type_to_sample_map[now_type[0]] = this_type_sample;

    }


    fclose(num_file);
    fclose(sample_file);
}

void Statistics::load_Statistics_for_one_edge_type() {
    string one_edge_num_map = filename + "one_edge_type_num_map";
    string one_edge_sample = filename + "one_edge_type_sample";
    FILE *num_file = fopen(one_edge_num_map.c_str(), "rb");
    FILE *sample_file = fopen(one_edge_sample.c_str(), "rb");

//    fread(&pre_num, sizeof(TYPE_PREDICATE_ID), 1, num_file);
//    fread(&type_pre_id, sizeof(TYPE_PREDICATE_ID), 1, num_file);
    fread(&one_edge_type_num, sizeof(int), 1, num_file);
    fread(&one_edge_type_pre_num, sizeof(int), 1, num_file);

    for(int i = 0; i < one_edge_type_pre_num; ++i){

//        pre_id, pre_num
        TYPE_PREDICATE_ID now_pre_id_and_num[2];
        fread(&now_pre_id_and_num, sizeof(TYPE_PREDICATE_ID), 2, num_file);

        for(int j = 0; j < now_pre_id_and_num[1]; ++j){
//            s_type_id, o_type_id
            TYPE_ENTITY_LITERAL_ID s_o_type[2];
            fread(s_o_type, sizeof(TYPE_ENTITY_LITERAL_ID), 2, num_file);

            S_to_O_id this_entity_type(s_o_type[0], s_o_type[1]);
            int this_type_num;
            fread(&this_type_num, sizeof(int), 1, num_file);
            one_edge_typeid_to_num[now_pre_id_and_num[0]].insert(make_pair(this_entity_type, this_type_num));

            int this_type_sample_num;
            fread(&this_type_sample_num, sizeof(int), 1, num_file);
            vector<S_to_O_id> this_type_sample(this_type_sample_num);
            for(int sample_index = 0; sample_index < this_type_sample_num; ++sample_index) {
                fread(&(this_type_sample[sample_index].s_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
                fread(&(this_type_sample[sample_index].o_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
            }
            one_edge_typeid_sample[now_pre_id_and_num[0]].insert(make_pair(this_entity_type, this_type_sample));
        }
    }

    fclose(num_file);
    fclose(sample_file);

//    load 时打印检查, 需传入 *kv_store
//    for(auto x = one_edge_typeid_to_num.begin(); x != one_edge_typeid_to_num.end(); ++x ){
//        for(auto y = x->second.begin(); y != x->second.end(); ++y){
//            cout << kv_store->getEntityByID(y->first.s_type_id) << "  " << kv_store->getPredicateByID(x->first) << "  " << kv_store->getEntityByID(y->first.o_type_id) << "  " << y->second << endl;
//        }
//    }

}

void Statistics::load_Statistics_for_two_edge_type1() {
    string two_edges_type1_num_map = filename + "two_edges_type1_num_map";
    string two_edges_type1_sample = filename + "two_edges_type1_sample";
    FILE *num_file = fopen(two_edges_type1_num_map.c_str(), "rb");
    FILE *sample_file = fopen(two_edges_type1_sample.c_str(), "rb");

    fread(&two_edges_type1_num, sizeof(int), 1, num_file);
    fread(&two_edges_type1_pre_num, sizeof(int), 1, num_file);

    for(int i = 0; i < two_edges_type1_pre_num; ++i){
//        pre_id, pre_num
        TYPE_PREDICATE_ID now_pre_id_and_num[3];
        fread(&now_pre_id_and_num, sizeof(TYPE_PREDICATE_ID), 3, num_file);
        TWO_PRE_ID now_pre_id(now_pre_id_and_num[0], now_pre_id_and_num[1]);

        for(int j = 0; j < now_pre_id_and_num[2]; ++j){
//            s_type_id, o_type_id
            TYPE_ENTITY_LITERAL_ID s1_s2_s3_type[3];
            fread(s1_s2_s3_type, sizeof(TYPE_ENTITY_LITERAL_ID), 3, num_file);
            TWO_EDGES_ENTITY_TYPE_ID this_entity_type(s1_s2_s3_type[0], s1_s2_s3_type[1], s1_s2_s3_type[2]);

            int this_type_num;
            fread(&this_type_num, sizeof(int), 1, num_file);
            two_edges_type1id_to_num[now_pre_id].insert(make_pair(this_entity_type, this_type_num));

            int this_type_sample_num;
            fread(&this_type_sample_num, sizeof(int), 1, num_file);
            vector<TWO_EDGES_ENTITY_TYPE_ID> this_type_sample(this_type_sample_num);
            for(int sample_index = 0; sample_index < this_type_sample_num; ++sample_index) {
                fread(&(this_type_sample[sample_index].s1_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
                fread(&(this_type_sample[sample_index].s2_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
                fread(&(this_type_sample[sample_index].s3_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
            }
            two_edges_type1id_sample[now_pre_id].insert(make_pair(this_entity_type, this_type_sample));
        }
    }

    fclose(num_file);
    fclose(sample_file);

//    load 时打印检查, 需传入 *kv_store
//    for(auto x = two_edges_type1id_to_num.begin(); x != two_edges_type1id_to_num.end(); ++x ){
//        for(auto y = x->second.begin(); y != x->second.end(); ++y){
//            cout << kv_store->getEntityByID(y->first.s1_type_id) << "  " << kv_store->getPredicateByID(x->first.p1_id) << "  " << kv_store->getEntityByID(y->first.s2_type_id) << "  " << kv_store->getPredicateByID(x->first.p2_id) << "  " << kv_store->getEntityByID(y->first.s3_type_id) << "  " << y->second << endl;
//        }
//    }
}

void Statistics::load_Statistics_for_two_edge_type2() {
    string two_edges_type2_num_map = filename + "two_edges_type2_num_map";
    string two_edges_type2_sample = filename + "two_edges_type2_sample";
    FILE *num_file = fopen(two_edges_type2_num_map.c_str(), "rb");
    FILE *sample_file = fopen(two_edges_type2_sample.c_str(), "rb");

    fread(&two_edges_type2_num, sizeof(int), 1, num_file);
    fread(&two_edges_type2_pre_num, sizeof(int), 1, num_file);

    for(int i = 0; i < two_edges_type2_pre_num; ++i){
//        pre_id, pre_num
        TYPE_PREDICATE_ID now_pre_id_and_num[3];
        fread(&now_pre_id_and_num, sizeof(TYPE_PREDICATE_ID), 3, num_file);
        TWO_PRE_ID now_pre_id(now_pre_id_and_num[0], now_pre_id_and_num[1]);

        for(int j = 0; j < now_pre_id_and_num[2]; ++j){
//            s_type_id, o_type_id
            TYPE_ENTITY_LITERAL_ID s1_s2_s3_type[3];
            fread(s1_s2_s3_type, sizeof(TYPE_ENTITY_LITERAL_ID), 3, num_file);
            TWO_EDGES_ENTITY_TYPE_ID this_entity_type(s1_s2_s3_type[0], s1_s2_s3_type[1], s1_s2_s3_type[2]);

            int this_type_num;
            fread(&this_type_num, sizeof(int), 1, num_file);
            two_edges_type2id_to_num[now_pre_id].insert(make_pair(this_entity_type, this_type_num));

            int this_type_sample_num;
            fread(&this_type_sample_num, sizeof(int), 1, num_file);
            vector<TWO_EDGES_ENTITY_TYPE_ID> this_type_sample(this_type_sample_num);
            for(int sample_index = 0; sample_index < this_type_sample_num; ++sample_index) {
                fread(&(this_type_sample[sample_index].s1_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
                fread(&(this_type_sample[sample_index].s2_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
                fread(&(this_type_sample[sample_index].s3_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
            }
            two_edges_type2id_sample[now_pre_id].insert(make_pair(this_entity_type, this_type_sample));
        }
    }
    fclose(num_file);
    fclose(sample_file);
}

void Statistics::load_Statistics_for_two_edge_type3() {
    string two_edges_type3_num_map = filename + "two_edges_type3_num_map";
    string two_edges_type3_sample = filename + "two_edges_type3_sample";
    FILE *num_file = fopen(two_edges_type3_num_map.c_str(), "rb");
    FILE *sample_file = fopen(two_edges_type3_sample.c_str(), "rb");

    fread(&two_edges_type3_num, sizeof(int), 1, num_file);
    fread(&two_edges_type3_pre_num, sizeof(int), 1, num_file);

    for(int i = 0; i < two_edges_type3_pre_num; ++i){
//        pre_id, pre_num
        TYPE_PREDICATE_ID now_pre_id_and_num[3];
        fread(&now_pre_id_and_num, sizeof(TYPE_PREDICATE_ID), 3, num_file);
        TWO_PRE_ID now_pre_id(now_pre_id_and_num[0], now_pre_id_and_num[1]);

        for(int j = 0; j < now_pre_id_and_num[2]; ++j){
//            s_type_id, o_type_id
            TYPE_ENTITY_LITERAL_ID s1_s2_s3_type[3];
            fread(s1_s2_s3_type, sizeof(TYPE_ENTITY_LITERAL_ID), 3, num_file);
            TWO_EDGES_ENTITY_TYPE_ID this_entity_type(s1_s2_s3_type[0], s1_s2_s3_type[1], s1_s2_s3_type[2]);

            int this_type_num;
            fread(&this_type_num, sizeof(int), 1, num_file);
            two_edges_type3id_to_num[now_pre_id].insert(make_pair(this_entity_type, this_type_num));

            int this_type_sample_num;
            fread(&this_type_sample_num, sizeof(int), 1, num_file);
            vector<TWO_EDGES_ENTITY_TYPE_ID> this_type_sample(this_type_sample_num);
            for(int sample_index = 0; sample_index < this_type_sample_num; ++sample_index) {
                fread(&(this_type_sample[sample_index].s1_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
                fread(&(this_type_sample[sample_index].s2_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
                fread(&(this_type_sample[sample_index].s3_type_id), sizeof(TYPE_ENTITY_LITERAL_ID), 1, sample_file);
            }
            two_edges_type3id_sample[now_pre_id].insert(make_pair(this_entity_type, this_type_sample));
        }
    }
    fclose(num_file);
    fclose(sample_file);
}

void Statistics::load_Statistics() {
    this->load_Statistics_for_type();
    this->load_Statistics_for_one_edge_type();
    this->load_Statistics_for_two_edge_type1();
    this->load_Statistics_for_two_edge_type2();
    this->load_Statistics_for_two_edge_type3();
}
