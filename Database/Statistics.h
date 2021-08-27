/*=============================================================================
# Filename: Statistics.h
# Author: Linglin Yang
# Mail: fyulingi@gmail.com
=============================================================================*/

#ifndef GSTORE_STATISTICS_H
#define GSTORE_STATISTICS_H

#include <unordered_map>
#include <vector>
#include <functional>
#include <string>
#include "../Util/Util.h"
#include "../KVstore/KVstore.h"

//    Statistics 类主要考虑了一条边和两条边的信息,
//    以谓词 pre_id 作索引, 存储 (stype_id)-[pre_id]->(otype_id) 的数目和抽样信息
//    两边的情况则以 {pre1_id, pre2_id} 作索引,有下面三种情况:
//    (s1)-[p1]->(s2)-[p2]->(s3)
//    (s1)<-[p1]-(s2)-[p2]->(s3)
//    (s1)-[p1]->(s2)<-[p2]-(s3)

//    如果需要在 build statistics 时查看信息, 需要在 Database::encodeRDF_new 中
//    将 entity2id 推迟释放

using namespace std;

struct S_to_O_id{
    TYPE_ENTITY_LITERAL_ID s_type_id;
    TYPE_ENTITY_LITERAL_ID o_type_id;

    S_to_O_id(){}
    S_to_O_id(TYPE_ENTITY_LITERAL_ID type1, TYPE_ENTITY_LITERAL_ID type2):s_type_id(type1), o_type_id(type2){}

    bool operator==(const S_to_O_id &p) const {
        return (s_type_id == p.s_type_id) && (o_type_id == p.o_type_id);
    }

    struct S_to_O_id_Hash{
        size_t operator()(const S_to_O_id &so_type_id) const{
            size_t h1 = hash<unsigned int >()(so_type_id.s_type_id);
            size_t h2 = hash<unsigned int >()(so_type_id.o_type_id) << 1;
            return h1 ^ h2;
        }
    };
};

struct TWO_PRE_ID{
    TYPE_PREDICATE_ID p1_id;
    TYPE_PREDICATE_ID p2_id;

    TWO_PRE_ID(TYPE_PREDICATE_ID p1, TYPE_PREDICATE_ID p2):p1_id(p1), p2_id(p2){}

    bool operator==(const TWO_PRE_ID &p) const {
        return (p1_id == p.p1_id) && (p2_id == p.p2_id);
    }

    struct TWO_PRE_ID_Hash{
        size_t operator()(const TWO_PRE_ID &two_pre_id) const{
            size_t h1 = hash<int >()(two_pre_id.p1_id);
            size_t h2 = hash<int >()(two_pre_id.p2_id) << 1;
            return h1 ^ h2;
        }
    };
} ;

struct  TWO_EDGES_ENTITY_TYPE_ID{
    TYPE_ENTITY_LITERAL_ID s1_type_id;
    TYPE_ENTITY_LITERAL_ID s2_type_id;
    TYPE_ENTITY_LITERAL_ID s3_type_id;

    TWO_EDGES_ENTITY_TYPE_ID(){}
    TWO_EDGES_ENTITY_TYPE_ID(TYPE_ENTITY_LITERAL_ID s1, TYPE_ENTITY_LITERAL_ID s2, TYPE_ENTITY_LITERAL_ID s3):
                                s1_type_id(s1),s2_type_id(s2),s3_type_id(s3){}

    bool operator==(const TWO_EDGES_ENTITY_TYPE_ID &p) const {
        return (s1_type_id == p.s1_type_id) && (s2_type_id == p.s2_type_id) && (s3_type_id == p.s3_type_id);
    }

    struct TWO_EDGES_ENTITY_TYPE_ID_Hash{
        size_t operator()(const TWO_EDGES_ENTITY_TYPE_ID &two_edge_type_id) const{
            size_t h1 = hash<unsigned int >()(two_edge_type_id.s1_type_id);
            size_t h2 = hash<unsigned int >()(two_edge_type_id.s2_type_id) << 1;
            size_t h3 = hash<unsigned int >()(two_edge_type_id.s3_type_id) << 2;
            return h2 ^ (h1 & h3) ;
        }
    };
};


//    TODO: 最好和 db->triple_num 相关的变量
//    每种类型最多抽样 1000 项
const int SAMPLE_NUM_UPBOUND = 1000;

class Statistics {

public:
    string filename;

    TYPE_PREDICATE_ID pre_num;
    TYPE_PREDICATE_ID type_pre_id;

//    entity_to_type_map 在构建统计信息中间方便使用
//    每个 entity 最多只会有一个 type
//    statistics 构建完成便删除减小内存占用
//    若一个 entity 有多种 type, 则使用 multimap
//    unordered_map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID> entityid_to_typeid_map;
    unordered_multimap<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID> entityid_to_typeid_map;


    unordered_map<TYPE_ENTITY_LITERAL_ID , unsigned int> type_to_num_map;
    unordered_map<TYPE_ENTITY_LITERAL_ID, vector<TYPE_ENTITY_LITERAL_ID>> type_to_sample_map;

//    单边的统计信息和抽样
//    实验中 59 种 stype_preid_otype 只有 2 种实际数目小于 1000
//    TODO: 抽样时要不要存储 pre_id ?
//    TODO: 这个unordered_map 可能用的有问题
    int one_edge_type_num;
    int one_edge_type_pre_num;
    unordered_map<TYPE_PREDICATE_ID,  unordered_map<S_to_O_id , int, S_to_O_id::S_to_O_id_Hash>> one_edge_typeid_to_num;
    unordered_map<TYPE_PREDICATE_ID , unordered_map<S_to_O_id, vector<S_to_O_id>, S_to_O_id::S_to_O_id_Hash>> one_edge_typeid_sample;

//    type1:
//    (s1)-[p1]->(s2)-[p2]->(s3)
    int two_edges_type1_num;
    int two_edges_type1_pre_num;
    unordered_map<TWO_PRE_ID, unordered_map<TWO_EDGES_ENTITY_TYPE_ID, int, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash>, TWO_PRE_ID::TWO_PRE_ID_Hash> two_edges_type1id_to_num;
    unordered_map<TWO_PRE_ID, unordered_map<TWO_EDGES_ENTITY_TYPE_ID, vector<TWO_EDGES_ENTITY_TYPE_ID>, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash>, TWO_PRE_ID::TWO_PRE_ID_Hash> two_edges_type1id_sample;

//    type2:
//    (s2)<-[p1]-(s1)-[p2]->(s3)
    int two_edges_type2_num;
    int two_edges_type2_pre_num;
    unordered_map<TWO_PRE_ID, unordered_map<TWO_EDGES_ENTITY_TYPE_ID, int, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash>, TWO_PRE_ID::TWO_PRE_ID_Hash> two_edges_type2id_to_num;
    unordered_map<TWO_PRE_ID, unordered_map<TWO_EDGES_ENTITY_TYPE_ID, vector<TWO_EDGES_ENTITY_TYPE_ID>, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash>, TWO_PRE_ID::TWO_PRE_ID_Hash> two_edges_type2id_sample;

//    type3:
//    (s1)-[p1]->(s2)<-[p2]-(s3)
    int two_edges_type3_num;
    int two_edges_type3_pre_num;
    unordered_map<TWO_PRE_ID, unordered_map<TWO_EDGES_ENTITY_TYPE_ID, int, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash>, TWO_PRE_ID::TWO_PRE_ID_Hash> two_edges_type3id_to_num;
    unordered_map<TWO_PRE_ID, unordered_map<TWO_EDGES_ENTITY_TYPE_ID, vector<TWO_EDGES_ENTITY_TYPE_ID>, TWO_EDGES_ENTITY_TYPE_ID::TWO_EDGES_ENTITY_TYPE_ID_Hash>, TWO_PRE_ID::TWO_PRE_ID_Hash> two_edges_type3id_sample;





    Statistics(const string& path, int limitID_pre);


    bool save_Statistics();
    bool save_type_statistics();
    bool save_one_edge_type_statistics();
    bool save_two_edges_type1_statistics();
    bool save_two_edges_type2_statistics();
    bool save_two_edges_type3_statistics();

//    在 gbuild 中调用
    bool build_Statistics(KVstore *);
    bool build_entity_to_type_unorder_map(KVstore *);
    bool build_Statistics_for_one_edge_type(KVstore *);
    bool build_Statistics_for_twe_edges_type1(KVstore *);
    bool build_Statistics_for_twe_edges_type2(KVstore *);
    bool build_Statistics_for_twe_edges_type3(KVstore *);

//    在 Database::load 中调用
    void load_Statistics();
    void load_Statistics_for_type();
    void load_Statistics_for_one_edge_type();
    void load_Statistics_for_two_edge_type1();
    void load_Statistics_for_two_edge_type2();
    void load_Statistics_for_two_edge_type3();


    //    如何使用 Statistics 的接口
    unsigned get_type_num_by_type_id(TYPE_ENTITY_LITERAL_ID);
    int get_type_one_edge_typeid_num_by_id(TYPE_ENTITY_LITERAL_ID s_type,
                                           TYPE_PREDICATE_ID p_type, TYPE_ENTITY_LITERAL_ID o_type);
    int get_type_two_edges_type1id_num_by_id(TYPE_ENTITY_LITERAL_ID s1_type, TYPE_PREDICATE_ID p1_type,
                                             TYPE_ENTITY_LITERAL_ID s2_type, TYPE_PREDICATE_ID p2_type,
                                             TYPE_ENTITY_LITERAL_ID s3_type);
    int get_type_two_edges_type2id_num_by_id(TYPE_ENTITY_LITERAL_ID s1_type, TYPE_PREDICATE_ID p1_type,
                                             TYPE_ENTITY_LITERAL_ID s2_type, TYPE_PREDICATE_ID p2_type,
                                             TYPE_ENTITY_LITERAL_ID s3_type);
    int get_type_two_edges_type3id_num_by_id(TYPE_ENTITY_LITERAL_ID s1_type, TYPE_PREDICATE_ID p1_type,
                                             TYPE_ENTITY_LITERAL_ID s2_type, TYPE_PREDICATE_ID p2_type,
                                             TYPE_ENTITY_LITERAL_ID s3_type);

    int get_type_one_edge_typeid_num_by_id(TYPE_PREDICATE_ID p_id, S_to_O_id s_o_type_id);
    int get_type_two_edges_type1id_num_by_id(TWO_PRE_ID p1_p2_id, TWO_EDGES_ENTITY_TYPE_ID s1_s2_s3_type_id);
    int get_type_two_edges_type2id_num_by_id(TWO_PRE_ID p1_p2_id, TWO_EDGES_ENTITY_TYPE_ID s1_s2_s3_type_id);
    int get_type_two_edges_type3id_num_by_id(TWO_PRE_ID p1_p2_id, TWO_EDGES_ENTITY_TYPE_ID s1_s2_s3_type_id);

//    bool insert_one_edge_type(TYPE_ENTITY_LITERAL_ID s_type,
//                              TYPE_PREDICATE_ID p_type, TYPE_ENTITY_LITERAL_ID o_type);
//    bool insert_two_edges_type1(TYPE_ENTITY_LITERAL_ID s1_type, TYPE_PREDICATE_ID p1_type,
//                                TYPE_ENTITY_LITERAL_ID s2_type, TYPE_PREDICATE_ID p2_type,
//                                TYPE_ENTITY_LITERAL_ID s3_type);
//    bool insert_two_edges_type2(TYPE_ENTITY_LITERAL_ID s1_type, TYPE_PREDICATE_ID p1_type,
//                                TYPE_ENTITY_LITERAL_ID s2_type, TYPE_PREDICATE_ID p2_type,
//                                TYPE_ENTITY_LITERAL_ID s3_type);
//    bool insert_two_edges_type3(TYPE_ENTITY_LITERAL_ID s1_type, TYPE_PREDICATE_ID p1_type,
//                                TYPE_ENTITY_LITERAL_ID s2_type, TYPE_PREDICATE_ID p2_type,
//                                TYPE_ENTITY_LITERAL_ID s3_type);
//
//    bool sort_type_one_tuple();
//    bool sort_type_two_tuple();

};

#endif //GSTORE_STATISTICS_H
