/*=============================================================================
# Filename: MultiQueryOptimization.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Description:
=============================================================================*/

#ifndef _MULTI_QUERY_OPTIMIZATION_H
#define _MULTI_QUERY_OPTIMIZATION_H

#include "../Util/Util.h"
#include "QueryTree.h"
#include "TempResult.h"
#include "TPSelectivity.h"
#include "SPARQLquery.h"
#include "../KVstore/KVstore.h"
#include "../Database/Strategy.h"
#include "../StringIndex/StringIndex.h"

class MultiQueryOptimization
{
private:
    const int CLUSTER_CARDINALITY_UPPER = 10;
    const double CLUSTER_SIMILARITY_THRESHOLD = 0.5;

    TPSelectivity &tp_selectivity_;
    KVstore *kv_vstore_;
    Strategy &strategy_;
    StringIndex *string_index_;

    struct SequenceTupleOfTriplePatternOnQuery
    {
        std::vector<int> query;
        std::vector<std::vector<int> > sequence;
    };

    bool checkTriplePatternEquivalency(const QueryTree::GroupPattern::Pattern &x, const QueryTree::GroupPattern::Pattern &y);
    void enumerateSubSequenceTupleOfTriplePattern(const std::vector<std::vector<int> > &sequence_tuple_of_triple_pattern,
        const QueryTree::GroupPattern &group_pattern,
        std::vector<int> &select,
        std::set<std::vector<std::vector<int> > > &sequence_tuple_of_triple_pattern_set);
    bool checkRemainingTriplePatternConnectivity(const std::vector<QueryTree::GroupPattern> &query_batch,
        const std::vector<int> &query_ids,
        const std::unordered_map<int, std::vector<bool> > &cover);
    int calculateTotalCost(const std::vector<QueryTree::GroupPattern> &query_batch,
        const std::vector<SequenceTupleOfTriplePatternOnQuery> &selected_sequence_tuple_of_triple_pattern,
        const std::unordered_map<int, std::vector<bool> > &cover);
    void queryClustering(const std::vector<QueryTree::GroupPattern> &query_batch,
        std::vector<std::vector<int> > &cluster);

public:
    explicit MultiQueryOptimization(TPSelectivity &tp_selectivity, KVstore *kv_vstore, Strategy &strategy, StringIndex *string_index)
        :tp_selectivity_(tp_selectivity), kv_vstore_(kv_vstore), strategy_(strategy), string_index_(string_index) {};
    void evaluate(std::vector<QueryTree::GroupPattern> &query_batch, 
        std::vector<TempResultSet*> &query_batch_result);
};

#endif // _MULTI_QUERY_OPTIMIZATION_H
