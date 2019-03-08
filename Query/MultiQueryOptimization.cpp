/*=============================================================================
# Filename: MultiQueryOptimization.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Description:
=============================================================================*/

#include "MultiQueryOptimization.h"

using namespace std;

bool MultiQueryOptimization::checkTriplePatternEquivalency(const QueryTree::GroupPattern::Pattern &x, const QueryTree::GroupPattern::Pattern &y)
{
    bool eq = true;
    eq &= ((x.subject.isVariable() && y.subject.isVariable()) ||
        (!x.subject.isVariable() && !y.subject.isVariable() && x.subject.getValue() == y.subject.getValue()));
    eq &= (!x.predicate.isVariable() && !y.predicate.isVariable() && x.predicate.getValue() == y.predicate.getValue());
    eq &= ((x.object.isVariable() && y.object.isVariable()) ||
        (!x.object.isVariable() && !y.object.isVariable() && x.object.getValue() == y.object.getValue()));
    if (x.subject.isVariable() && x.object.isVariable() && y.subject.isVariable() && y.object.isVariable())
        eq &= ((x.subject.getValue() == x.object.getValue() && y.subject.getValue() == y.object.getValue()) ||
            (x.subject.getValue() != x.object.getValue() && y.subject.getValue() != y.object.getValue()));
    return eq;
}

void MultiQueryOptimization::enumerateSubSequenceTupleOfTriplePattern(const vector<vector<int> > &sequence_tuple_of_triple_pattern,
    const QueryTree::GroupPattern &group_pattern,
    vector<int> &select,
    set<vector<vector<int> > > &sequence_tuple_of_triple_pattern_set)
{
    unordered_set<string> varset;
    for (int x : select)
    {
        const QueryTree::GroupPattern::Pattern &pattern = group_pattern.sub_group_pattern[x].pattern;

        if (pattern.subject.isVariable())
            varset.insert(pattern.subject.getValue());
        if (pattern.object.isVariable())
            varset.insert(pattern.object.getValue());
    }

    vector<int> cand;
    if (select.empty())
    {
        cand = sequence_tuple_of_triple_pattern[0];
    }
    else
    {
        for (int x : sequence_tuple_of_triple_pattern[0])
        {
            const QueryTree::GroupPattern::Pattern &pattern = group_pattern.sub_group_pattern[x].pattern;

            bool has_common = ((pattern.subject.isVariable() && varset.count(pattern.subject.getValue()) > 0) ||
                (pattern.object.isVariable() && varset.count(pattern.object.getValue()) > 0));

            if (has_common && count(select.begin(), select.end(), x) == 0)
                cand.push_back(x);
        }
    }

    for (int x : cand)
    {
        select.push_back(x);

        vector<vector<int> > new_sequence_tuple_of_triple_pattern(sequence_tuple_of_triple_pattern.size(), vector<int>());

        new_sequence_tuple_of_triple_pattern[0] = select;
        sort(new_sequence_tuple_of_triple_pattern[0].begin(), new_sequence_tuple_of_triple_pattern[0].end());

        for (int i = 0; i < (int)new_sequence_tuple_of_triple_pattern[0].size(); i++)
            for (int j = 0; j < (int)sequence_tuple_of_triple_pattern[0].size(); j++)
                if (new_sequence_tuple_of_triple_pattern[0][i] == sequence_tuple_of_triple_pattern[0][j])
                    for (int k = 1; k < (int)sequence_tuple_of_triple_pattern.size(); k++)
                        new_sequence_tuple_of_triple_pattern[k].push_back(sequence_tuple_of_triple_pattern[k][j]);

        if (sequence_tuple_of_triple_pattern_set.count(new_sequence_tuple_of_triple_pattern) == 0)
        {
            sequence_tuple_of_triple_pattern_set.insert(new_sequence_tuple_of_triple_pattern);
            enumerateSubSequenceTupleOfTriplePattern(sequence_tuple_of_triple_pattern, group_pattern, select, sequence_tuple_of_triple_pattern_set);
        }

        select.pop_back();
    }
}

bool MultiQueryOptimization::checkRemainingTriplePatternConnectivity(const vector<QueryTree::GroupPattern> &query_batch,
    const vector<int> &query_ids,
    const unordered_map<int, vector<bool> > &cover)
{
    for (int qid : query_ids)
    {
        const QueryTree::GroupPattern &query = query_batch[qid];
        QueryTree::GroupPattern triple_pattern;

        for (int i = 0; i < (int)query.sub_group_pattern.size(); i++)
            if (query.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type && !cover.find(qid)->second[i])
                triple_pattern.addOnePattern(query.sub_group_pattern[i].pattern);

        if (triple_pattern.sub_group_pattern.empty())
            continue;

        triple_pattern.getVarset();
        triple_pattern.initPatternBlockid();
        triple_pattern.getTriplePatternSpanningTree(0, triple_pattern.sub_group_pattern.size());

        int root = triple_pattern.getRootPatternBlockID(0);

        for (int i = 1; i < (int)triple_pattern.sub_group_pattern.size(); i++)
            if (triple_pattern.getRootPatternBlockID(i) != root)
                return false;
    }

    return true;
}

int MultiQueryOptimization::calculateTotalCost(const vector<QueryTree::GroupPattern> &query_batch,
    const vector<SequenceTupleOfTriplePatternOnQuery> &selected_sequence_tuple_of_triple_pattern,
    const unordered_map<int, vector<bool> > &cover)
{
    int total_cost = 0;

    for (auto &stotpoq : selected_sequence_tuple_of_triple_pattern)
    {
        unsigned min_sel = UINT32_MAX;

        const QueryTree::GroupPattern &query = query_batch[stotpoq.query[0]];
        for (int tid : stotpoq.sequence[0])
            min_sel = min(min_sel, this->tp_selectivity_.getTPSelectivity(query.sub_group_pattern[tid].pattern));

        total_cost += min_sel * stotpoq.sequence[0].size();
    }

    for (auto &it : cover)
    {
        unsigned min_sel = UINT32_MAX;
        int count = 0;

        const QueryTree::GroupPattern &query = query_batch[it.first];
        for (int i = 0; i < (int)it.second.size(); i++)
            if (!it.second[i])
            {
                min_sel = min(min_sel, this->tp_selectivity_.getTPSelectivity(query.sub_group_pattern[i].pattern));
                count++;
            }

        total_cost += min_sel * count;
    }

    return total_cost;
}

void MultiQueryOptimization::queryClustering(const vector<QueryTree::GroupPattern> &query_batch, vector<vector<int> > &cluster)
{
    for (int i = 0; i < (int)query_batch.size(); i++)
        cluster.push_back({ i });

    while (true)
    {
        bool found = false;

        for (int i = 0; i < (int)cluster.size(); i++)
            for (int j = i + 1; j < (int)cluster.size(); j++)
                if ((int)cluster[i].size() + (int)cluster[j].size() <= this->CLUSTER_CARDINALITY_UPPER)
                {
                    double jaccard = 0;

                    for (int x : cluster[i])
                        for (int y : cluster[j])
                        {
                            unordered_set<string> predx, cap, cup;

                            for (auto &t : query_batch[x].sub_group_pattern)
                                if (t.type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                                {
                                    const string &p = t.pattern.predicate.getValue();
                                    predx.insert(p);
                                    cup.insert(p);
                                }

                            for (auto &t : query_batch[y].sub_group_pattern)
                                if (t.type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                                {
                                    const string &p = t.pattern.predicate.getValue();
                                    if (predx.count(p) > 0)
                                        cap.insert(p);
                                    cup.insert(p);
                                }

                            jaccard += 1.0 * cap.size() / (cup.empty() ? 1 : cup.size());
                        }

                    jaccard /= cluster[i].size();
                    jaccard /= cluster[j].size();

                    /*
                    {
                        cout << "Jaccard({";
                        for (int k = 0; k < (int)cluster[i].size(); k++)
                        {
                            if (k != 0)
                                cout << ", ";
                            cout << cluster[i][k];
                        }
                        cout << "},{";
                        for (int k = 0; k < (int)cluster[j].size(); k++)
                        {
                            if (k != 0)
                                cout << ", ";
                            cout << cluster[j][k];
                        }
                        cout << "}) = " << jaccard << endl;
                    }
                    */

                    if (jaccard > this->CLUSTER_SIMILARITY_THRESHOLD)
                    {
                        cluster[i].insert(cluster[i].end(), cluster[j].begin(), cluster[j].end());
                        sort(cluster[i].begin(), cluster[i].end());

                        if (j + 1 != (int)cluster.size())
                            cluster[j] = cluster.back();
                        cluster.pop_back();

                        found = true;
                    }
                }

        if (!found) break;
    }

    for (int c = 0; c < (int)cluster.size(); c++)
    {
        cout << "Cluster[" << c << "] = {";
        for (int i = 0; i < (int)cluster[c].size(); i++)
        {
            if (i != 0)
                cout << ", ";
            cout << cluster[c][i];
        }
        cout << "}" << endl;
    }
}

void MultiQueryOptimization::evaluate(vector<QueryTree::GroupPattern> &query_batch, vector<TempResultSet*> &query_batch_result)
{
    for (auto &query : query_batch)
        for (auto &t : query.sub_group_pattern)
            if (t.type != QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                throw "[ERROR]	MultiQueryOptimization can handle BGP query only.";

    vector<vector<int> > cluster;
    queryClustering(query_batch, cluster);

    for (auto &c : cluster)
    {
        vector<SequenceTupleOfTriplePatternOnQuery> maximal_sequence_tuple_of_triple_pattern;
        unordered_map<int, vector<SequenceTupleOfTriplePatternOnQuery> > query0_maximal_sequence_pair_of_triple_pattern;

        for (int i = 0; i < (int)c.size(); i++)
            for (int j = i + 1; j < (int)c.size(); j++)
            {
                vector<SequenceTupleOfTriplePatternOnQuery> maximal_sequence_pair_of_triple_pattern;

                deque<SequenceTupleOfTriplePatternOnQuery> queue;
                set<pair<vector<int>, vector<int> > > sequence_pair_of_triple_pattern_bool;

                QueryTree::GroupPattern &group_pattern0 = query_batch[c[i]];
                QueryTree::GroupPattern &group_pattern1 = query_batch[c[j]];

                for (int x = 0; x < (int)group_pattern0.sub_group_pattern.size(); x++)
                    for (int y = 0; y < (int)group_pattern1.sub_group_pattern.size(); y++)
                        if (checkTriplePatternEquivalency(group_pattern0.sub_group_pattern[x].pattern, group_pattern1.sub_group_pattern[y].pattern))
                        {
                            SequenceTupleOfTriplePatternOnQuery stotpoq;
                            stotpoq.query = { c[i], c[j] };
                            stotpoq.sequence.push_back({ x });
                            stotpoq.sequence.push_back({ y });

                            queue.push_back(stotpoq);
                            sequence_pair_of_triple_pattern_bool.insert(make_pair(stotpoq.sequence[0], stotpoq.sequence[1]));
                        }

                while (!queue.empty())
                {
                    SequenceTupleOfTriplePatternOnQuery h = queue.front();
                    queue.pop_front();

                    vector<int> query0_cand, query1_cand;
                    {
                        unordered_set<int> used;

                        for (int x : h.sequence[0])
                            used.insert(x);
                        for (int x = 0; x < (int)group_pattern0.sub_group_pattern.size(); x++)
                            if (used.count(x) == 0)
                                query0_cand.push_back(x);

                        used.clear();

                        for (int x : h.sequence[1])
                            used.insert(x);
                        for (int x = 0; x < (int)group_pattern1.sub_group_pattern.size(); x++)
                            if (used.count(x) == 0)
                                query1_cand.push_back(x);
                    }

                    unordered_map<string, string> query0_varmap, query1_varmap;
                    for (int k = 0; k < (int)h.sequence[0].size(); k++)
                    {
                        QueryTree::GroupPattern::Pattern &query0_triple_pattern = group_pattern0.sub_group_pattern[h.sequence[0][k]].pattern;
                        QueryTree::GroupPattern::Pattern &query1_triple_pattern = group_pattern1.sub_group_pattern[h.sequence[1][k]].pattern;

                        if (query0_triple_pattern.subject.isVariable() && query1_triple_pattern.subject.isVariable())
                        {
                            query0_varmap[query0_triple_pattern.subject.getValue()] = query1_triple_pattern.subject.getValue();
                            query1_varmap[query1_triple_pattern.subject.getValue()] = query0_triple_pattern.subject.getValue();
                        }

                        if (query0_triple_pattern.object.isVariable() && query1_triple_pattern.object.isVariable())
                        {
                            query0_varmap[query0_triple_pattern.object.getValue()] = query1_triple_pattern.object.getValue();
                            query1_varmap[query1_triple_pattern.object.getValue()] = query0_triple_pattern.object.getValue();
                        }
                    }

                    bool expended = false;

                    for (int x : query0_cand)
                        for (int y : query1_cand)
                            if (checkTriplePatternEquivalency(group_pattern0.sub_group_pattern[x].pattern, group_pattern1.sub_group_pattern[y].pattern))
                            {
                                QueryTree::GroupPattern::Pattern &query0_triple_pattern = group_pattern0.sub_group_pattern[x].pattern;
                                QueryTree::GroupPattern::Pattern &query1_triple_pattern = group_pattern1.sub_group_pattern[y].pattern;

                                if (query0_triple_pattern.subject.isVariable() && query1_triple_pattern.subject.isVariable())
                                {
                                    if (query0_varmap.count(query0_triple_pattern.subject.getValue()) > 0 &&
                                        query0_varmap[query0_triple_pattern.subject.getValue()] != query1_triple_pattern.subject.getValue())
                                        continue;
                                    if (query1_varmap.count(query1_triple_pattern.subject.getValue()) > 0 &&
                                        query1_varmap[query1_triple_pattern.subject.getValue()] != query0_triple_pattern.subject.getValue())
                                        continue;
                                }

                                if (query0_triple_pattern.object.isVariable() && query1_triple_pattern.object.isVariable())
                                {
                                    if (query0_varmap.count(query0_triple_pattern.object.getValue()) > 0 &&
                                        query0_varmap[query0_triple_pattern.object.getValue()] != query1_triple_pattern.object.getValue())
                                        continue;
                                    if (query1_varmap.count(query1_triple_pattern.object.getValue()) > 0 &&
                                        query1_varmap[query1_triple_pattern.object.getValue()] != query0_triple_pattern.object.getValue())
                                        continue;
                                }

                                if ((query0_triple_pattern.subject.isVariable() && query0_varmap.count(query0_triple_pattern.subject.getValue())) ||
                                    (query0_triple_pattern.object.isVariable() && query0_varmap.count(query0_triple_pattern.object.getValue())))
                                {
                                    SequenceTupleOfTriplePatternOnQuery t = h;
                                    t.sequence[0].push_back(x);
                                    t.sequence[1].push_back(y);

                                    for (int k = (int)t.sequence[0].size() - 1; k > 0; k--)
                                        if (t.sequence[0][k - 1] > t.sequence[0][k])
                                        {
                                            swap(t.sequence[0][k - 1], t.sequence[0][k]);
                                            swap(t.sequence[1][k - 1], t.sequence[1][k]);
                                        }
                                        else break;

                                    expended = true;

                                    if (sequence_pair_of_triple_pattern_bool.count(make_pair(t.sequence[0], t.sequence[1])) == 0)
                                    {
                                        sequence_pair_of_triple_pattern_bool.insert(make_pair(t.sequence[0], t.sequence[1]));
                                        queue.push_back(t);
                                    }
                                }
                            }

                    if (!expended)
                        maximal_sequence_pair_of_triple_pattern.push_back(h);
                }

                /*
                {
                    cout << "MaximalSequencePairOfTriplePattern:" << endl;
                    for (auto &mspotp : maximal_sequence_pair_of_triple_pattern)
                    {
                        cout << "Q(";
                        for (int q = 0; q < (int)mspotp.query.size(); q++)
                        {
                            if (q != 0)
                                cout << ", ";
                            cout << mspotp.query[q];
                        }
                        cout << ")\t(";
                        for (int q = 0; q < (int)mspotp.sequence.size(); q++)
                        {
                            if (q != 0)
                                cout << ",";
                            cout << "(";
                            for (int k = 0; k < (int)mspotp.sequence[q].size(); k++)
                            {
                                if (k != 0)
                                    cout << ", ";
                                cout << mspotp.sequence[q][k];
                            }
                            cout << ")";
                        }
                        cout << ")" << endl;
                    }
                }
                */

                maximal_sequence_tuple_of_triple_pattern.insert(maximal_sequence_tuple_of_triple_pattern.end(),
                    maximal_sequence_pair_of_triple_pattern.begin(), maximal_sequence_pair_of_triple_pattern.end());

                for (auto &mspotp : maximal_sequence_pair_of_triple_pattern)
                {
                    int q0 = mspotp.query[0];
                    if (query0_maximal_sequence_pair_of_triple_pattern.count(q0) == 0)
                        query0_maximal_sequence_pair_of_triple_pattern[q0] = vector<SequenceTupleOfTriplePatternOnQuery>();
                    query0_maximal_sequence_pair_of_triple_pattern[q0].push_back(mspotp);
                }
            }

        deque<SequenceTupleOfTriplePatternOnQuery> queue;
        map<vector<int>, set<vector<vector<int> > > > sequence_tuple_of_triple_pattern_bool;

        for (auto &mstotpoq : maximal_sequence_tuple_of_triple_pattern)
            queue.push_back(mstotpoq);

        while (!queue.empty())
        {
            SequenceTupleOfTriplePatternOnQuery h = queue.front();
            queue.pop_front();

            int queryk = h.query.back();
            vector<int> &sequencek = h.sequence.back();

            for (auto &mspotp : query0_maximal_sequence_pair_of_triple_pattern[queryk])
            {
                vector<int> &sequence0 = mspotp.sequence[0];
                vector<bool> sequencek_comm;

                bool has_comm = false;
                for (int x : sequencek)
                {
                    sequencek_comm.push_back(find(sequence0.begin(), sequence0.end(), x) != sequence0.end());
                    has_comm = has_comm || sequencek_comm.back();
                }

                if (!has_comm)
                    continue;

                SequenceTupleOfTriplePatternOnQuery t;
                t.query = h.query;
                t.query.push_back(mspotp.query.back());
                t.sequence.resize(h.query.size() + 1);
                for (int k = 0; k < (int)h.query.size(); k++)
                    for (int p = 0; p < (int)h.sequence[k].size(); p++)
                        if (sequencek_comm[p])
                            t.sequence[k].push_back(h.sequence[k][p]);
                for (int k = 0; k < (int)sequencek.size(); k++)
                    for (int p = 0; p < (int)sequence0.size(); p++)
                        if (sequencek[k] == sequence0[p])
                            t.sequence.back().push_back(mspotp.sequence.back()[p]);

                if (sequence_tuple_of_triple_pattern_bool.count(t.query) == 0)
                    sequence_tuple_of_triple_pattern_bool[t.query] = set<vector<vector<int> > >();

                if (sequence_tuple_of_triple_pattern_bool[t.query].count(t.sequence) == 0)
                {
                    sequence_tuple_of_triple_pattern_bool[t.query].insert(t.sequence);
                    queue.push_back(t);
                    maximal_sequence_tuple_of_triple_pattern.push_back(t);
                }
            }
        }

        /*
        {
            cout << "MaximalSequenceTupleOfTriplePattern:" << endl;
            for (auto &mstotpoq : maximal_sequence_tuple_of_triple_pattern)
            {
                cout << "Q(";
                for (int q = 0; q < (int)mstotpoq.query.size(); q++)
                {
                    if (q != 0)
                        cout << ", ";
                    cout << mstotpoq.query[q];
                }
                cout << ")\t(";
                for (int q = 0; q < (int)mstotpoq.sequence.size(); q++)
                {
                    if (q != 0)
                        cout << ",";
                    cout << "(";
                    for (int k = 0; k < (int)mstotpoq.sequence[q].size(); k++)
                    {
                        if (k != 0)
                            cout << ", ";
                        cout << mstotpoq.sequence[q][k];
                    }
                    cout << ")";
                }
                cout << ")" << endl;
            }
        }
        */

        sequence_tuple_of_triple_pattern_bool.clear();
        for (auto &mstotpoq : maximal_sequence_tuple_of_triple_pattern)
        {
            if (sequence_tuple_of_triple_pattern_bool.count(mstotpoq.query) == 0)
                sequence_tuple_of_triple_pattern_bool[mstotpoq.query] = set<vector<vector<int> > >();
            sequence_tuple_of_triple_pattern_bool[mstotpoq.query].insert(mstotpoq.sequence);
        }

        for (auto &it : sequence_tuple_of_triple_pattern_bool)
        {
            vector<int> select;
            QueryTree::GroupPattern &group_pattern0 = query_batch[it.first[0]];
            set<vector<vector<int> > > maximal_sequence_tuple_of_triple_pattern_tmp = it.second;

            for (auto &mstotp : maximal_sequence_tuple_of_triple_pattern_tmp)
                enumerateSubSequenceTupleOfTriplePattern(mstotp, group_pattern0, select, it.second);
        }

        /*
        {
            cout << "SequenceTupleOfTriplePattern:" << endl;
            for (auto &it : sequence_tuple_of_triple_pattern_bool)
                for (auto &stotp : it.second)
                {
                    cout << "Q(";
                    for (int q = 0; q < (int)it.first.size(); q++)
                    {
                        if (q != 0)
                            cout << ", ";
                        cout << it.first[q];
                    }
                    cout << ")\t(";
                    for (int q = 0; q < (int)stotp.size(); q++)
                    {
                        if (q != 0)
                            cout << ",";
                        cout << "(";
                        for (int k = 0; k < (int)stotp[q].size(); k++)
                        {
                            if (k != 0)
                                cout << ", ";
                            cout << stotp[q][k];
                        }
                        cout << ")";
                    }
                    cout << ")" << endl;
                }
        }
        */

        vector<SequenceTupleOfTriplePatternOnQuery> selected_sequence_tuple_of_triple_pattern;

        unordered_map<int, vector<bool> > cover;
        for (int queryk : c)
            cover[queryk] = vector<bool>(query_batch[queryk].sub_group_pattern.size(), false);

        if (!sequence_tuple_of_triple_pattern_bool.empty())
            while (true)
            {
                double min_delta = 0;
                SequenceTupleOfTriplePatternOnQuery best_stotpoq;

                double old_cost = calculateTotalCost(query_batch, selected_sequence_tuple_of_triple_pattern, cover);

                for (auto &it : sequence_tuple_of_triple_pattern_bool)
                    for (auto &stotp : it.second)
                        if (stotp[0].size() > 1)
                        {
                            bool valid = true;
                            for (int q = 0; q < (int)stotp.size(); q++)
                                for (int x : stotp[q])
                                    if (cover[it.first[q]][x])
                                        valid = false;

                            if (valid)
                            {
                                for (int q = 0; q < (int)stotp.size(); q++)
                                    for (int x : stotp[q])
                                        cover[it.first[q]][x] = true;

                                if (checkRemainingTriplePatternConnectivity(query_batch, it.first, cover))
                                {
                                    SequenceTupleOfTriplePatternOnQuery stotpoq;
                                    stotpoq.query = it.first;
                                    stotpoq.sequence = stotp;
                                    selected_sequence_tuple_of_triple_pattern.push_back(stotpoq);

                                    double new_cost = calculateTotalCost(query_batch, selected_sequence_tuple_of_triple_pattern, cover);
                                    double delta = new_cost - old_cost;
                                    if (delta < min_delta)
                                    {
                                        min_delta = delta;
                                        best_stotpoq = stotpoq;
                                    }

                                    selected_sequence_tuple_of_triple_pattern.pop_back();
                                }

                                for (int q = 0; q < (int)stotp.size(); q++)
                                    for (int x : stotp[q])
                                        cover[it.first[q]][x] = false;
                            }
                        }

                if (min_delta < 0)
                {
                    for (int q = 0; q < (int)best_stotpoq.query.size(); q++)
                        for (int x : best_stotpoq.sequence[q])
                            cover[best_stotpoq.query[q]][x] = true;

                    selected_sequence_tuple_of_triple_pattern.push_back(best_stotpoq);
                }
                else
                    break;
            }

        {
            cout << "SelectedSequenceTupleOfTriplePattern:" << endl;
            for (auto &stotpoq : selected_sequence_tuple_of_triple_pattern)
            {
                cout << "Q(";
                for (int q = 0; q < (int)stotpoq.query.size(); q++)
                {
                    if (q != 0)
                        cout << ", ";
                    cout << stotpoq.query[q];
                }
                cout << ")\t(";
                for (int q = 0; q < (int)stotpoq.sequence.size(); q++)
                {
                    if (q != 0)
                        cout << ",";
                    cout << "(";
                    for (int k = 0; k < (int)stotpoq.sequence[q].size(); k++)
                    {
                        if (k != 0)
                            cout << ", ";
                        cout << stotpoq.sequence[q][k];
                    }
                    cout << ")";
                }
                cout << ")" << endl;
            }
        }

        vector<TempResultSet*> selected_sequence_tuple_of_triple_pattern_result;
        {
            SPARQLquery sparql_query;
            vector<vector<string> > encode_varset;
            for (auto &stotpoq : selected_sequence_tuple_of_triple_pattern)
            {
                int query0 = stotpoq.query[0];

                sparql_query.addBasicQuery();
                QueryTree::GroupPattern group_pattern;

                for (int x : stotpoq.sequence[0])
                {
                    QueryTree::GroupPattern::Pattern &pattern = query_batch[query0].sub_group_pattern[x].pattern;
                    sparql_query.addTriple(Triple(
                        pattern.subject.getValue(),
                        pattern.predicate.getValue(),
                        pattern.object.getValue()));
                    group_pattern.addOnePattern(query_batch[query0].sub_group_pattern[x].pattern);
                }

                group_pattern.print(0);

                group_pattern.getVarset();                
                encode_varset.push_back(group_pattern.group_pattern_resultset_maximal_varset.vars);
            }

            sparql_query.encodeQuery(this->kv_vstore_, encode_varset);
            this->strategy_.handle(sparql_query);

            //TODO BasicQueryEvaluation
            for (int i = 0; i < sparql_query.getBasicQueryNum(); i++)
            {
                TempResultSet *temp = new TempResultSet();
                temp->results.push_back(TempResult());

                temp->results[0].id_varset = Varset(encode_varset[i]);
                int var_num = encode_varset[i].size();

                vector<unsigned*> &basic_query_result = sparql_query.getBasicQuery(i).getResultList();
                int result_num = basic_query_result.size();

                temp->results[0].result.reserve(result_num);
                for (int j = 0; j < result_num; j++)
                {
                    unsigned *l = new unsigned[var_num];
                    memcpy(l, basic_query_result[j], sizeof(int) * var_num);
                    temp->results[0].result.push_back(TempResult::ResultPair());
                    temp->results[0].result.back().id = l;
                }

                selected_sequence_tuple_of_triple_pattern_result.push_back(temp);
            }
        }

        for (int queryk : c)
        {
            query_batch_result[queryk] = new TempResultSet();

            vector<bool> cover(query_batch[queryk].sub_group_pattern.size(), false);

            for (auto &stotp : selected_sequence_tuple_of_triple_pattern)
            {
                auto it = find(stotp.query.begin(), stotp.query.end(), queryk);
                if (it != stotp.query.end())
                {
                    int k = it - stotp.query.begin();
                    for (int x : stotp.sequence[k])
                        cover[x] = true;
                }
            }

            QueryTree::GroupPattern group_pattern;
            for (int i = 0; i < (int)query_batch[queryk].sub_group_pattern.size(); i++)
                if (!cover[i])
                    group_pattern.addOnePattern(query_batch[queryk].sub_group_pattern[i].pattern);

            group_pattern.print(0);

            if (!group_pattern.sub_group_pattern.empty())
            {
                unordered_map<string, pair<bool, int> > common_var_cand_limit;
                for (auto &t : group_pattern.sub_group_pattern)
                    if (t.type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                    {
                        int sel = this->tp_selectivity_.getTPSelectivity(t.pattern);

                        if (t.pattern.subject.isVariable())
                        {
                            string subject = t.pattern.subject.getValue();
                            if (common_var_cand_limit.count(subject) == 0)
                                common_var_cand_limit[subject] = make_pair(false, INT_MAX);
                            common_var_cand_limit[subject].second = min(common_var_cand_limit[subject].second, sel);
                        }
                        
                        if (t.pattern.object.isVariable())
                        {
                            string object = t.pattern.object.getValue();
                            if (common_var_cand_limit.count(object) == 0)
                                common_var_cand_limit[object] = make_pair(false, INT_MAX);
                            common_var_cand_limit[object].second = min(common_var_cand_limit[object].second, sel);
                        }
                    }

                unordered_map<string, vector<unsigned> > var_cand;
                for (int i = 0; i < (int)selected_sequence_tuple_of_triple_pattern.size(); i++)
                {
                    auto &stotp = selected_sequence_tuple_of_triple_pattern[i];
                    auto &stotpr = selected_sequence_tuple_of_triple_pattern_result[i]->results[0];

                    auto it = find(stotp.query.begin(), stotp.query.end(), queryk);
                    if (it != stotp.query.end())
                    {
                        int k = it - stotp.query.begin();

                        QueryTree::GroupPattern &group_pattern0 = query_batch[stotp.query[0]];
                        QueryTree::GroupPattern &group_patternk = query_batch[stotp.query[k]];

                        unordered_map<string, string> mapk;

                        for (int j = 0; j < (int)stotp.sequence[0].size(); j++)
                        {
                            QueryTree::GroupPattern::Pattern &pattern0 = group_pattern0.sub_group_pattern[stotp.sequence[0][j]].pattern;
                            QueryTree::GroupPattern::Pattern &patternk = group_patternk.sub_group_pattern[stotp.sequence[k][j]].pattern;

                            if (pattern0.subject.isVariable() && patternk.subject.isVariable())
                            {
                                string subject0 = pattern0.subject.getValue();
                                string subjectk = patternk.subject.getValue();
                                if (common_var_cand_limit.count(subjectk) > 0)
                                    common_var_cand_limit[subjectk].first = true;
                                mapk[subjectk] = subject0;
                            }

                            if (pattern0.object.isVariable() && patternk.object.isVariable())
                            {
                                string object0 = pattern0.object.getValue();
                                string objectk = patternk.object.getValue();
                                if (common_var_cand_limit.count(objectk) > 0)
                                    common_var_cand_limit[objectk].first = true;
                                mapk[objectk] = object0;
                            }
                        }

                        for (auto &it : common_var_cand_limit)
                            if (it.second.first)
                            {
                                auto jt = find(stotpr.id_varset.vars.begin(), stotpr.id_varset.vars.end(), mapk[it.first]);
                                if (jt != stotpr.id_varset.vars.end())
                                {
                                    int v = jt - stotpr.id_varset.vars.begin();
                                    int result_num = stotpr.result.size();

                                    if (result_num <= it.second.second)
                                    {
                                        long bf_fillcand = Util::get_cur_time();

                                        vector<unsigned> unique_var_cand;
                                        unique_var_cand.reserve(result_num);
                                        for (auto &l : stotpr.result)
                                            unique_var_cand.push_back(l.id[v]);
                                        sort(unique_var_cand.begin(), unique_var_cand.end());
                                        unique_var_cand.erase(unique(unique_var_cand.begin(), unique_var_cand.end()), unique_var_cand.end());

                                        if (var_cand.count(it.first) == 0)
                                        {
                                            var_cand[it.first] = unique_var_cand;
                                        }
                                        else
                                        {
                                            vector<unsigned> new_var_cand;
                                            set_intersection(var_cand[it.first].begin(), var_cand[it.first].end(),
                                                unique_var_cand.begin(), unique_var_cand.end(),
                                                inserter(new_var_cand, new_var_cand.begin()));
                                            var_cand[it.first] = new_var_cand;
                                        }

                                        long af_fillcand = Util::get_cur_time();
                                        printf("after MakeCand %s, used %ld ms.\n", it.first.c_str(), af_fillcand - bf_fillcand);
                                    }
                                    else
                                        printf("not MakeCand %s, %d > %d.\n", it.first.c_str(), result_num, it.second.second);
                                }
                            }
                    }
                }

                unordered_map<string, int> var_degree;
                for (auto &t : group_pattern.sub_group_pattern)
                    if (t.type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                    {
                        if (t.pattern.subject.isVariable())
                        {
                            string subject = t.pattern.subject.getValue();
                            if (var_degree.count(subject) == 0)
                                var_degree[subject] = 0;
                            var_degree[subject]++;
                        }
                        if (t.pattern.object.isVariable())
                        {
                            string object = t.pattern.object.getValue();
                            if (var_degree.count(object) == 0)
                                var_degree[object] = 0;
                            var_degree[object]++;
                        }
                    }

                for (auto &it : var_degree)
                    if (var_cand.count(it.first) > 0 && it.second == 1)
                        for (auto &t : group_pattern.sub_group_pattern)
                            if (t.type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                                if (t.pattern.subject.isVariable() && t.pattern.object.isVariable())
                                {
                                    string subject = t.pattern.subject.getValue();
                                    string predicate = t.pattern.predicate.getValue();
                                    string object = t.pattern.object.getValue();

                                    if (subject == it.first && var_cand.count(object) == 0)
                                    {
                                        long bf_fillcand = Util::get_cur_time();

                                        unsigned predicate_id = this->kv_vstore_->getIDByPredicate(predicate);

                                        vector<unsigned> &subject_cand = var_cand[subject];
                                        vector<unsigned> object_cand;

                                        for (unsigned x : subject_cand)                                        
                                        {
                                            unsigned *obj_id_list = NULL, list_len;

                                            this->kv_vstore_->getobjIDlistBysubIDpreID(x, predicate_id, obj_id_list, list_len, false);

                                            for (unsigned i = 0; i < list_len; i++)
                                                object_cand.push_back(obj_id_list[i]);

                                            delete[] obj_id_list;
                                        }

                                        sort(object_cand.begin(), object_cand.end());
                                        object_cand.erase(unique(object_cand.begin(), object_cand.end()), object_cand.end());

                                        var_cand[object] = object_cand;

                                        long af_fillcand = Util::get_cur_time();
                                        printf("after MakeCand %s, used %ld ms.\n", object.c_str(), af_fillcand - bf_fillcand);
                                    }

                                    if (object == it.first && var_cand.count(subject) == 0)
                                    {
                                        long bf_fillcand = Util::get_cur_time();

                                        unsigned predicate_id = this->kv_vstore_->getIDByPredicate(predicate);

                                        vector<unsigned> subject_cand;
                                        vector<unsigned> &object_cand = var_cand[object];

                                        for (unsigned x : object_cand)
                                        {
                                            unsigned *sub_id_list = NULL, list_len;

                                            this->kv_vstore_->getsubIDlistByobjIDpreID(x, predicate_id, sub_id_list, list_len, false);

                                            for (unsigned i = 0; i < list_len; i++)
                                                subject_cand.push_back(sub_id_list[i]);

                                            delete[] sub_id_list;
                                        }

                                        sort(subject_cand.begin(), subject_cand.end());
                                        subject_cand.erase(unique(subject_cand.begin(), subject_cand.end()), subject_cand.end());

                                        var_cand[subject] = subject_cand;

                                        long af_fillcand = Util::get_cur_time();
                                        printf("after MakeCand %s, used %ld ms.\n", subject.c_str(), af_fillcand - bf_fillcand);
                                    }
                                }

                if (group_pattern.sub_group_pattern.size() > 1)
                    for (auto it = var_cand.begin(); it != var_cand.end();)
                    {
                        if (var_degree[it->first] == 1)
                            it = var_cand.erase(it);
                        else
                            it = it++;
                    }

                {
                    SPARQLquery sparql_query;
                    vector<vector<string> > encode_varset;

                    sparql_query.addBasicQuery();
                    for (auto &t : group_pattern.sub_group_pattern)
                        if (t.type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                            sparql_query.addTriple(Triple(
                                t.pattern.subject.getValue(),
                                t.pattern.predicate.getValue(),
                                t.pattern.object.getValue()));

                    group_pattern.getVarset();
                    encode_varset.push_back(group_pattern.group_pattern_resultset_maximal_varset.vars);

                    sparql_query.encodeQuery(this->kv_vstore_, encode_varset);

                    BasicQuery &basic_query = sparql_query.getBasicQuery(0);
                    for (int i = 0; i < (int)encode_varset[0].size(); i++)
                    {
                        string &v = encode_varset[0][i];
                        if (var_cand.count(v) > 0)
                        {
                            basic_query.getCandidateList(i).copy(var_cand[v]);
                            basic_query.setReady(i);

                            printf("fill var %s CandidateList size %d\n", v.c_str(), (int)var_cand[v].size());
                        }
                    }

                    this->strategy_.handle(sparql_query);

                    {
                        query_batch_result[queryk]->results.push_back(TempResult());
                        TempResult &temp = query_batch_result[queryk]->results[0];

                        temp.id_varset = Varset(encode_varset[0]);
                        int var_num = encode_varset[0].size();

                        vector<unsigned*> &basic_query_result = sparql_query.getBasicQuery(0).getResultList();
                        int result_num = basic_query_result.size();

                        temp.result.reserve(result_num);
                        for (int i = 0; i < result_num; i++)
                        {
                            unsigned *l = new unsigned[var_num];
                            memcpy(l, basic_query_result[i], sizeof(int) * var_num);
                            temp.result.push_back(TempResult::ResultPair());
                            temp.result.back().id = l;
                        }
                    }
                }
            }

            vector<bool> valid(selected_sequence_tuple_of_triple_pattern.size(), true);
            while (true)
            {
                int best = -1, min_result_num = INT_MAX;

                for (int i = 0; i < (int)selected_sequence_tuple_of_triple_pattern.size(); i++)
                    if (valid[i])
                    {
                        vector<int> &queries = selected_sequence_tuple_of_triple_pattern[i].query;
                        
                        auto it = find(queries.begin(), queries.end(), queryk);
                        if (it != queries.end())
                        {
                            int k = it - queries.begin();
                            QueryTree::GroupPattern &group_pattern0 = query_batch[selected_sequence_tuple_of_triple_pattern[i].query[0]];
                            QueryTree::GroupPattern &group_patternk = query_batch[selected_sequence_tuple_of_triple_pattern[i].query[k]];
                            
                            unordered_map<string, string> map0, mapk;
                            for (int j = 0; j < (int)selected_sequence_tuple_of_triple_pattern[i].sequence[0].size(); j++)
                            {
                                QueryTree::GroupPattern::Pattern &pattern0 = group_pattern0.sub_group_pattern[selected_sequence_tuple_of_triple_pattern[i].sequence[0][j]].pattern;
                                QueryTree::GroupPattern::Pattern &patternk = group_patternk.sub_group_pattern[selected_sequence_tuple_of_triple_pattern[i].sequence[k][j]].pattern;

                                if (pattern0.subject.isVariable() && patternk.subject.isVariable())
                                {
                                    string subject0 = pattern0.subject.getValue();
                                    string subjectk = patternk.subject.getValue();                                    
                                    map0[subject0] = subjectk;
                                    mapk[subjectk] = subject0;
                                }

                                if (pattern0.object.isVariable() && patternk.object.isVariable())
                                {
                                    string object0 = pattern0.object.getValue();
                                    string objectk = patternk.object.getValue();
                                    map0[object0] = objectk;
                                    mapk[objectk] = object0;
                                }
                            }

                            TempResult &resulti = selected_sequence_tuple_of_triple_pattern_result[i]->results[0];
                            TempResultSet *resultk = query_batch_result[queryk];

                            for (int j = 0; j < (int)resulti.id_varset.vars.size(); j++)
                                resulti.id_varset.vars[j] = map0[resulti.id_varset.vars[j]];

                            if (resultk->results.empty() || resultk->results[0].id_varset.hasCommonVar(resulti.id_varset))
                                if ((int)resulti.result.size() < min_result_num)
                                {
                                    best = i;
                                    min_result_num = resulti.result.size();
                                }

                            for (int j = 0; j < (int)resulti.id_varset.vars.size(); j++)
                                resulti.id_varset.vars[j] = mapk[resulti.id_varset.vars[j]];
                        }
                    }

                if (best == -1)
                    break;

                valid[best] = false;

                vector<int> &queries = selected_sequence_tuple_of_triple_pattern[best].query;
                int k = find(queries.begin(), queries.end(), queryk) - queries.begin();
                QueryTree::GroupPattern &group_pattern0 = query_batch[selected_sequence_tuple_of_triple_pattern[best].query[0]];
                QueryTree::GroupPattern &group_patternk = query_batch[selected_sequence_tuple_of_triple_pattern[best].query[k]];

                unordered_map<string, string> map0, mapk;
                for (int j = 0; j < (int)selected_sequence_tuple_of_triple_pattern[best].sequence[0].size(); j++)
                {
                    QueryTree::GroupPattern::Pattern &pattern0 = group_pattern0.sub_group_pattern[selected_sequence_tuple_of_triple_pattern[best].sequence[0][j]].pattern;
                    QueryTree::GroupPattern::Pattern &patternk = group_patternk.sub_group_pattern[selected_sequence_tuple_of_triple_pattern[best].sequence[k][j]].pattern;

                    if (pattern0.subject.isVariable() && patternk.subject.isVariable())
                    {
                        string subject0 = pattern0.subject.getValue();
                        string subjectk = patternk.subject.getValue();
                        map0[subject0] = subjectk;
                        mapk[subjectk] = subject0;
                    }

                    if (pattern0.object.isVariable() && patternk.object.isVariable())
                    {
                        string object0 = pattern0.object.getValue();
                        string objectk = patternk.object.getValue();
                        map0[object0] = objectk;
                        mapk[objectk] = object0;
                    }
                }

                for (int j = 0; j < (int)selected_sequence_tuple_of_triple_pattern_result[best]->results[0].id_varset.vars.size(); j++)
                    selected_sequence_tuple_of_triple_pattern_result[best]->results[0].id_varset.vars[j] =
                        map0[selected_sequence_tuple_of_triple_pattern_result[best]->results[0].id_varset.vars[j]];

                TempResultSet *new_result = new TempResultSet();

                query_batch_result[queryk]->doJoin(*selected_sequence_tuple_of_triple_pattern_result[best], *new_result, this->string_index_, query_batch[queryk].group_pattern_subject_object_maximal_varset);

                query_batch_result[queryk]->release();
                delete query_batch_result[queryk];

                query_batch_result[queryk] = new_result;

                for (int j = 0; j < (int)selected_sequence_tuple_of_triple_pattern_result[best]->results[0].id_varset.vars.size(); j++)
                    selected_sequence_tuple_of_triple_pattern_result[best]->results[0].id_varset.vars[j] =
                        mapk[selected_sequence_tuple_of_triple_pattern_result[best]->results[0].id_varset.vars[j]];
            }
        }
    }
}
