/*=============================================================================
# Filename: GeneralEvaluation.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-05-05
# Description: implement functions in GeneralEvaluation.h
=============================================================================*/

#include "GeneralEvaluation.h"

using namespace std;

void *preread_from_index(void *argv)
{
    vector<StringIndexFile*> * indexfile = (vector<StringIndexFile*> *)*(long*)argv;
    ResultSet *ret_result = (ResultSet*)*((long*)argv + 1);
    vector<int>* proj2temp = (vector<int>*)*((long*)argv + 2);
    int id_cols = *((long*)argv + 3);
    TempResult* result0 = (TempResult*)*((long*)argv + 4);
    vector<bool>* isel = (vector<bool>*)*((long*)argv + 5);
    StringIndexFile* 	entity = (*indexfile)[0];
    StringIndexFile* 	literal = (*indexfile)[1];
    StringIndexFile* 	predicate = (*indexfile)[2];
    unsigned total = ret_result->ansNum;
    int var_num = ret_result->select_var_num;
    unsigned thelta = total / 4096;
    char tmp;
    char *entityc = entity->Mmap;
    char *literalc = literal->Mmap;
    char *prec = predicate->Mmap;
    for (unsigned i = 0; i < total; i++)
    {
        for (int j = 0; j < var_num; j++)
        {
            int k = (*proj2temp)[j];
            long offset = -1;
            if (k != -1)
            {
                if (k < id_cols)
                {
                    unsigned ans_id = result0->result[i].id[k];
                    if (ans_id != INVALID)
                    {
                        if ((*isel)[k])
                        {
                            if (ans_id < Util::LITERAL_FIRST_ID)
                            {
                                offset = entity->GetOffsetbyID(ans_id);
                                if (offset != -1)
                                {
                                    tmp = entityc[offset];
                                }
                            }
                            else
                            {
                                offset = literal->GetOffsetbyID(ans_id - Util::LITERAL_FIRST_ID);
                                if (offset != -1)
                                {
                                    tmp = literalc[offset];
                                }
                            }
                        }
                        else
                        {
                            offset = predicate->GetOffsetbyID(ans_id);
                            if (offset != -1)
                            {
                                tmp = prec[offset];
                            }
                        }
                    }
                }
            }
        }
    }

    return NULL;
}

void GeneralEvaluation::setStringIndexPointer(StringIndex* _tmpsi)
{
    this->stringindex = _tmpsi;
}

bool GeneralEvaluation::parseQuery(const string &_query)
{
    try
    {
        this->query_parser.SPARQLParse(_query, this->query_tree);
    }
    catch (const char *e)
    {
        printf("%s\n", e);
        return false;
    }

    return true;
}

QueryTree& GeneralEvaluation::getQueryTree()
{
    return this->query_tree;
}

bool GeneralEvaluation::doQuery()
{
    if (!this->query_tree.checkProjectionAsterisk() && this->query_tree.getProjection().empty())
        return false;

    this->query_tree.getGroupPattern().getVarset();
    this->query_tree.getGroupByVarset() = this->query_tree.getGroupByVarset() * this->query_tree.getGroupPattern().group_pattern_resultset_maximal_varset;

    if (this->query_tree.checkProjectionAsterisk() && this->query_tree.getProjection().empty() && !this->query_tree.getGroupByVarset().empty())
    {
        printf("[ERROR]	Select * and Group By can't appear at the same time.\n");
        return false;
    }

    if (!this->query_tree.checkSelectAggregateFunctionGroupByValid())
    {
        printf("[ERROR]	The vars/aggregate functions in the Select Clause are invalid.\n");
        return false;
    }

    if (this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset.hasCommonVar(this->query_tree.getGroupPattern().group_pattern_predicate_maximal_varset))
    {
        printf("[ERROR]	There are some vars occur both in subject/object and predicate.\n");
        return false;
    }

    this->strategy = Strategy(this->kvstore, this->vstree, this->pre2num, this->pre2sub, this->pre2obj,
        this->limitID_predicate, this->limitID_literal, this->limitID_entity);
    if (this->query_tree.checkWellDesigned())
    {
        printf("=================\n");
        printf("||well-designed||\n");
        printf("=================\n");

        if (this->filterRewriting(this->query_tree.getGroupPattern()))
        {
            printf("====================\n");
            printf("||filter-rewriting||\n");
            printf("====================\n");
            this->query_tree.getGroupPattern().getVarset();
            this->query_tree.print();
        }

        this->rewriting_evaluation_stack.clear();
        this->rewriting_evaluation_stack.push_back(EvaluationStackStruct());
        this->rewriting_evaluation_stack.back().group_pattern = this->query_tree.getGroupPattern();
        this->rewriting_evaluation_stack.back().result = NULL;

        this->temp_result = this->rewritingBasedQueryEvaluation(0);
    }
    else
    {
        printf("=====================\n");
        printf("||not well-designed||\n");
        printf("=====================\n");

        this->temp_result = this->semanticBasedQueryEvaluation(this->query_tree.getGroupPattern());
    }

    return true;
}

bool GeneralEvaluation::filterRewriting(QueryTree::GroupPattern &group_pattern)
{
    bool res = false;

    vector<bool> keep(group_pattern.sub_group_pattern.size(), true);
    QueryTree::GroupPattern new_group_pattern;

    for (int i = 0; i < (int)group_pattern.sub_group_pattern.size(); i++)
        if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
        {
            QueryTree::GroupPattern::FilterTree &filter = group_pattern.sub_group_pattern[i].filter;

            vector<vector<pair<string, string> > > dis_con_eq;
            bool is_eq_dis_nf = dfsDisjunctiveNormalForm(filter.root, dis_con_eq);

            if (is_eq_dis_nf)
            {
                for (auto &con : dis_con_eq)
                    for (auto &eq : con)
                    {
                        if (eq.first[0] != '?' && eq.second[0] == '?')
                            swap(eq.first, eq.second);
                        if (eq.first[0] != '?' || eq.second[0] == '?')
                            is_eq_dis_nf = false;
                    }
            }

            if (is_eq_dis_nf)
            {
                for (auto &con : dis_con_eq)
                {
                    unordered_map<string, string> varmap;

                    for (auto &eq : con)
                        if (varmap.count(eq.first) == 0)
                            varmap[eq.first] = eq.second;
                        else if (varmap[eq.first] != eq.second)
                            is_eq_dis_nf = false;
                }
            }

            unordered_map<string, bool> occur_var;

            if (is_eq_dis_nf)
            {
                for (auto &v : dis_con_eq[0])
                    occur_var[v.first] = false;

                for (int j = 1; j < (int)dis_con_eq.size(); j++)
                {
                    for (auto &it : occur_var)
                        it.second = false;

                    for (auto &eq : dis_con_eq[j])
                        if (occur_var.count(eq.first) == 0)
                            is_eq_dis_nf = false;
                        else
                            occur_var[eq.first] = true;

                    for (auto &it : occur_var)
                        if (!it.second)
                            is_eq_dis_nf = false;
                }
            }

            if (is_eq_dis_nf)
            {
                keep[i] = false;
                res = true;

                if (dis_con_eq.size() > 1)
                    new_group_pattern.addOneGroupUnion();

                for (int j = 0; j < (int)dis_con_eq.size(); j++)
                {
                    if (dis_con_eq.size() > 1)
                        new_group_pattern.addOneUnion();

                    QueryTree::GroupPattern &target_group_pattern = (dis_con_eq.size() == 1 ? new_group_pattern : new_group_pattern.getLastUnion());

                    for (int k = 0; k < (int)group_pattern.sub_group_pattern.size(); k++)
                    {
                        if (group_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                        {
                            QueryTree::GroupPattern::Pattern &pattern = group_pattern.sub_group_pattern[k].pattern;
                            if ((pattern.subject.isVariable() && occur_var.count(pattern.subject.getValue()) > 0) ||
                                (pattern.predicate.isVariable() && occur_var.count(pattern.predicate.getValue()) > 0) ||
                                (pattern.object.isVariable() && occur_var.count(pattern.object.getValue()) > 0))
                            {
                                QueryTree::GroupPattern::Pattern new_pattern = pattern;
                                int var_remain = (int)new_pattern.subject.isVariable() + (int)new_pattern.predicate.isVariable() + (int)new_pattern.object.isVariable();
                                bool has_changed = false;
                                //TODO var_remain == 1

                                if (new_pattern.subject.isVariable())
                                    for (auto &eq : dis_con_eq[j])
                                        if (new_pattern.subject.getValue() == eq.first && var_remain > 1)
                                        {
                                            new_pattern.subject.setValue(eq.second);
                                            var_remain--;
                                            has_changed = true;
                                            break;
                                        }

                                if (new_pattern.predicate.isVariable())
                                    for (auto &eq : dis_con_eq[j])
                                        if (new_pattern.predicate.getValue() == eq.first && var_remain > 1)
                                        {
                                            new_pattern.predicate.setValue(eq.second);
                                            var_remain--;
                                            has_changed = true;
                                            break;
                                        }

                                if (new_pattern.object.isVariable())
                                    for (auto &eq : dis_con_eq[j])
                                        if (new_pattern.object.getValue() == eq.first && var_remain > 1)
                                        {
                                            new_pattern.object.setValue(eq.second);
                                            var_remain--;
                                            has_changed = true;
                                            break;
                                        }

                                if (has_changed)
                                {
                                    keep[k] = false;
                                    target_group_pattern.addOnePattern(new_pattern);
                                }
                            }
                        }
                    }

                    for (auto &eq : dis_con_eq[j])
                    {
                        target_group_pattern.addOneBind();
                        target_group_pattern.getLastBind() = QueryTree::GroupPattern::Bind(eq.second, eq.first);
                    }
                }
            }
        }

    for (int i = 0; i < (int)group_pattern.sub_group_pattern.size(); i++)
        if (keep[i])
            new_group_pattern.sub_group_pattern.push_back(group_pattern.sub_group_pattern[i]);

    group_pattern.sub_group_pattern = new_group_pattern.sub_group_pattern;

    for (auto &t : group_pattern.sub_group_pattern)
        if (t.type == QueryTree::GroupPattern::SubGroupPattern::Union_type)
        {
            for (auto &u : t.unions)
                res |= filterRewriting(u);
        }
        else if (t.type == QueryTree::GroupPattern::SubGroupPattern::Optional_type)
        {
            res |= filterRewriting(t.optional);
        }

    return res;
}

bool GeneralEvaluation::dfsDisjunctiveNormalForm(QueryTree::GroupPattern::FilterTree::FilterTreeNode &filter_node,
    vector<vector<pair<string, string> > > &dis_con_eq)
{
    if (filter_node.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterOperationType::Or_type)
    {
        bool res = true;
        for (auto &c : filter_node.child)
            res &= dfsDisjunctiveNormalForm(c.node, dis_con_eq);
        return res;
    }
    else if (filter_node.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterOperationType::And_type ||
        filter_node.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterOperationType::Equal_type)
    {
        dis_con_eq.push_back(vector<pair<string, string> >());
        return dfsConjunctiveNormalForm(filter_node, dis_con_eq);
    }
    else
        return false;
}

bool GeneralEvaluation::dfsConjunctiveNormalForm(QueryTree::GroupPattern::FilterTree::FilterTreeNode &filter_node,
    vector<vector<pair<string, string> > > &dis_con_eq)
{
    if (filter_node.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterOperationType::And_type)
    {
        bool res = true;
        for (auto &c : filter_node.child)
            res &= dfsConjunctiveNormalForm(c.node, dis_con_eq);
        return res;
    }
    else if (filter_node.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterOperationType::Equal_type)
    {
        vector<string> child_value;

        for (auto &c : filter_node.child)
            if (c.node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::FilterTreeChildNodeType::String_type)
                child_value.push_back(c.str);
            else
                return false;

        if (child_value.size() == 2)
        {
            dis_con_eq.back().push_back(make_pair(child_value[0], child_value[1]));
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

TempResultSet* GeneralEvaluation::semanticBasedQueryEvaluation(QueryTree::GroupPattern &group_pattern)
{
    TempResultSet *result = new TempResultSet();

    group_pattern.initPatternBlockid();

    for (int i = 0; i < (int)group_pattern.sub_group_pattern.size(); i++)
        if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
        {
            int st = i;
            while (st > 0 && (group_pattern.sub_group_pattern[st - 1].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type || group_pattern.sub_group_pattern[st - 1].type == QueryTree::GroupPattern::SubGroupPattern::Union_type))
                st--;

            for (int j = st; j < i; j++)
                if (group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                {
                    if (group_pattern.sub_group_pattern[i].pattern.subject_object_varset.hasCommonVar(group_pattern.sub_group_pattern[j].pattern.subject_object_varset))
                        group_pattern.mergePatternBlockID(i, j);
                }

            if (i + 1 == (int)group_pattern.sub_group_pattern.size() ||
                (group_pattern.sub_group_pattern[i + 1].type != QueryTree::GroupPattern::SubGroupPattern::Pattern_type && group_pattern.sub_group_pattern[i + 1].type != QueryTree::GroupPattern::SubGroupPattern::Union_type))
            {
                SPARQLquery sparql_query;
                vector<vector<string> > encode_varset;
                vector<vector<QueryTree::GroupPattern::Pattern> > basic_query_handle;

                for (int j = st; j <= i; j++)
                    if (group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                        if (!group_pattern.sub_group_pattern[j].pattern.varset.empty())
                        {
                            if (group_pattern.getRootPatternBlockID(j) == j)		//root node
                            {
                                Varset occur;
                                vector<QueryTree::GroupPattern::Pattern> basic_query;

                                for (int k = st; k <= i; k++)
                                    if (group_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                                        if (group_pattern.getRootPatternBlockID(k) == j)
                                        {
                                            occur += group_pattern.sub_group_pattern[k].pattern.varset;
                                            basic_query.push_back(group_pattern.sub_group_pattern[k].pattern);
                                        }

                                printf("select vars: ");
                                occur.print();

                                printf("triple patterns: \n");
                                for (int k = 0; k < (int)basic_query.size(); k++)
                                    printf("%s\t%s\t%s\n", basic_query[k].subject.value.c_str(),
                                        basic_query[k].predicate.value.c_str(),
                                        basic_query[k].object.value.c_str()
                                    );

                                TempResultSet *temp = new TempResultSet();
                                temp->results.push_back(TempResult());

                                bool success = false;
                                if (this->query_cache != NULL)
                                {
                                    long tv_bfcheck = Util::get_cur_time();
                                    success = this->query_cache->checkCached(basic_query, occur, temp->results[0]);
                                    long tv_afcheck = Util::get_cur_time();
                                    printf("after checkCache, used %ld ms.\n", tv_afcheck - tv_bfcheck);
                                }

                                if (success)
                                {
                                    printf("QueryCache hit\n");
                                    printf("Final result size: %d\n", (int)temp->results[0].result.size());

                                    TempResultSet *new_result = new TempResultSet();
                                    result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

                                    result->release();
                                    delete result;

                                    result = new_result;
                                }
                                else
                                {
                                    printf("QueryCache miss\n");

                                    sparql_query.addBasicQuery();
                                    for (int k = 0; k < (int)basic_query.size(); k++)
                                        sparql_query.addTriple(Triple(basic_query[k].subject.value,
                                            basic_query[k].predicate.value,
                                            basic_query[k].object.value));

                                    encode_varset.push_back(occur.vars);
                                    basic_query_handle.push_back(basic_query);
                                }

                                temp->release();
                                delete temp;
                            }
                        }

                long tv_begin = Util::get_cur_time();
                sparql_query.encodeQuery(this->kvstore, encode_varset);
                long tv_encode = Util::get_cur_time();
                printf("after Encode, used %ld ms.\n", tv_encode - tv_begin);

                this->strategy.handle(sparql_query);
                long tv_handle = Util::get_cur_time();
                printf("after Handle, used %ld ms.\n", tv_handle - tv_encode);

                //collect and join the result of each BasicQuery
                for (int j = 0; j < sparql_query.getBasicQueryNum(); j++)
                {
                    TempResultSet *temp = new TempResultSet();
                    temp->results.push_back(TempResult());

                    temp->results[0].id_varset = Varset(encode_varset[j]);
                    int varnum = (int)encode_varset[j].size();

                    vector<unsigned*> &basicquery_result = sparql_query.getBasicQuery(j).getResultList();
                    int basicquery_result_num = (int)basicquery_result.size();

                    temp->results[0].result.reserve(basicquery_result_num);
                    for (int k = 0; k < basicquery_result_num; k++)
                    {
                        unsigned *v = new unsigned[varnum];
                        memcpy(v, basicquery_result[k], sizeof(int) * varnum);
                        temp->results[0].result.push_back(TempResult::ResultPair());
                        temp->results[0].result.back().id = v;
                    }

                    if (this->query_cache != NULL)
                    {
                        //if unconnected, time is incorrect
                        int time = tv_handle - tv_begin;

                        long tv_bftry = Util::get_cur_time();
                        bool success = this->query_cache->tryCaching(basic_query_handle[j], temp->results[0], time);
                        if (success)	printf("QueryCache cached\n");
                        else			printf("QueryCache didn't cache\n");
                        long tv_aftry = Util::get_cur_time();
                        printf("after tryCache, used %ld ms.\n", tv_aftry - tv_bftry);
                    }

                    if (result->results.empty())
                    {
                        delete result;
                        result = temp;
                    }
                    else
                    {
                        TempResultSet *new_result = new TempResultSet();
                        result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

                        temp->release();
                        result->release();
                        delete temp;
                        delete result;

                        result = new_result;
                    }
                }
            }
        }
        else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Union_type)
        {
            TempResultSet *sub_result = new TempResultSet();

            for (int j = 0; j < (int)group_pattern.sub_group_pattern[i].unions.size(); j++)
            {
                TempResultSet *temp = semanticBasedQueryEvaluation(group_pattern.sub_group_pattern[i].unions[j]);

                if (sub_result->results.empty())
                {
                    delete sub_result;
                    sub_result = temp;
                }
                else
                {
                    TempResultSet *new_result = new TempResultSet();
                    sub_result->doUnion(*temp, *new_result);

                    temp->release();
                    sub_result->release();
                    delete temp;
                    delete sub_result;

                    sub_result = new_result;
                }
            }

            if (result->results.empty())
            {
                delete result;
                result = sub_result;
            }
            else
            {
                TempResultSet *new_result = new TempResultSet();
                result->doJoin(*sub_result, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

                sub_result->release();
                result->release();
                delete sub_result;
                delete result;

                result = new_result;
            }
        }
        else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Optional_type || group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Minus_type)
        {
            TempResultSet *temp = semanticBasedQueryEvaluation(group_pattern.sub_group_pattern[i].optional);

            {
                TempResultSet *new_result = new TempResultSet();

                if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Optional_type)
                    result->doOptional(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);
                else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Minus_type)
                    result->doMinus(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

                temp->release();
                result->release();
                delete temp;
                delete result;

                result = new_result;
            }
        }
        else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
        {
            TempResultSet *new_result = new TempResultSet();
            result->doFilter(group_pattern.sub_group_pattern[i].filter.root, *new_result, this->stringindex, group_pattern.group_pattern_subject_object_maximal_varset);

            result->release();
            delete result;

            result = new_result;
        }
        else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Bind_type)
        {
            TempResultSet *temp = new TempResultSet();
            temp->results.push_back(TempResult());

            temp->results[0].str_varset = group_pattern.sub_group_pattern[i].bind.varset;

            temp->results[0].result.push_back(TempResult::ResultPair());
            temp->results[0].result[0].str.push_back(group_pattern.sub_group_pattern[i].bind.str);

            {
                TempResultSet *new_result = new TempResultSet();
                result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

                temp->release();
                result->release();
                delete temp;
                delete result;

                result = new_result;
            }
        }

    return result;
}

bool GeneralEvaluation::expanseFirstOuterUnionGroupPattern(QueryTree::GroupPattern &group_pattern, deque<QueryTree::GroupPattern> &queue)
{
    int first_union_pos = -1;

    for (int i = 0; i < (int)group_pattern.sub_group_pattern.size(); i++)
        if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Union_type)
        {
            first_union_pos = i;
            break;
        }

    if (first_union_pos == -1)
        return false;

    for (int i = 0; i < (int)group_pattern.sub_group_pattern[first_union_pos].unions.size(); i++)
    {
        QueryTree::GroupPattern new_group_pattern;

        for (int j = 0; j < first_union_pos; j++)
            new_group_pattern.sub_group_pattern.push_back(group_pattern.sub_group_pattern[j]);

        for (int j = 0; j < (int)group_pattern.sub_group_pattern[first_union_pos].unions[i].sub_group_pattern.size(); j++)
            new_group_pattern.sub_group_pattern.push_back(group_pattern.sub_group_pattern[first_union_pos].unions[i].sub_group_pattern[j]);

        for (int j = first_union_pos + 1; j < (int)group_pattern.sub_group_pattern.size(); j++)
            new_group_pattern.sub_group_pattern.push_back(group_pattern.sub_group_pattern[j]);

        queue.push_back(new_group_pattern);
    }

    return true;
}

TempResultSet* GeneralEvaluation::rewritingBasedQueryEvaluation(int dep)
{
    deque<QueryTree::GroupPattern> queue;
    queue.push_back(this->rewriting_evaluation_stack[dep].group_pattern);
    vector<QueryTree::GroupPattern> group_pattern_union;

    while (!queue.empty())
    {
        if (!this->expanseFirstOuterUnionGroupPattern(queue.front(), queue))
            group_pattern_union.push_back(queue.front());
        queue.pop_front();
    }

    TempResultSet *result = new TempResultSet();

    vector<TempResultSet*> query_batch_result;
    if (dep == 0)
    {
        vector<QueryTree::GroupPattern> query_batch(group_pattern_union.size());
        query_batch_result.resize(group_pattern_union.size(), NULL);

        for (int i = 0; i < (int)group_pattern_union.size(); i++)
        {
            for (auto &t : group_pattern_union[i].sub_group_pattern)
                if (t.type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                    query_batch[i].addOnePattern(t.pattern);

            query_batch_result[i] = new TempResultSet();
        }

        MultiQueryOptimization multi_query_optimization(this->tp_selectivity, this->kvstore, this->strategy, this->stringindex);
        multi_query_optimization.evaluate(query_batch, query_batch_result);
    }

    for (int i = 0; i < (int)group_pattern_union.size(); i++)
    {
        this->rewriting_evaluation_stack[dep].group_pattern = group_pattern_union[i];
        QueryTree::GroupPattern *group_pattern = &this->rewriting_evaluation_stack[dep].group_pattern;
        group_pattern->getVarset();

        for (int j = 0; j < 80; j++)
            printf("=");
        printf("\n");
        group_pattern->print(dep);
        for (int j = 0; j < 80; j++)
            printf("=");
        printf("\n");

        TempResultSet *sub_result;

        if (dep == 0)
        {
            sub_result = query_batch_result[i];
            query_batch_result[i] = NULL;
        }
        else
            sub_result = new TempResultSet();

        if (dep > 0)
        {
            QueryTree::GroupPattern triple_pattern;
            int group_pattern_triple_num = 0;
            for (int j = 0; j < (int)group_pattern->sub_group_pattern.size(); j++)
                if (group_pattern->sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                {
                    triple_pattern.addOnePattern(QueryTree::GroupPattern::Pattern(
                        QueryTree::GroupPattern::Pattern::Element(group_pattern->sub_group_pattern[j].pattern.subject.value),
                        QueryTree::GroupPattern::Pattern::Element(group_pattern->sub_group_pattern[j].pattern.predicate.value),
                        QueryTree::GroupPattern::Pattern::Element(group_pattern->sub_group_pattern[j].pattern.object.value)
                    ));
                    group_pattern_triple_num++;
                }
            triple_pattern.getVarset();

            /*
            //add extra triple pattern
            {
                Varset need_add;

                map<string, int> var_count;
                for (int j = 0; j < (int)triple_pattern.sub_group_pattern.size(); j++)
                    if (triple_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                    {
                        string subject = triple_pattern.sub_group_pattern[j].pattern.subject.value;
                        string object = triple_pattern.sub_group_pattern[j].pattern.object.value;

                        if (subject[0] == '?')
                        {
                            if (var_count.count(subject) == 0)
                                var_count[subject] = 0;
                            var_count[subject]++;
                        }
                        if (object[0] == '?')
                        {
                            if (var_count.count(object) == 0)
                                var_count[object] = 0;
                            var_count[object]++;
                        }
                    }

                for (map<string, int>::iterator iter = var_count.begin(); iter != var_count.end(); iter++)
                    if (iter->second == 1)
                        need_add.addVar(iter->first);

                for (int j = 0; j < dep; j++)
                {
                    QueryTree::GroupPattern &parent_group_pattern = this->rewriting_evaluation_stack[j].group_pattern;

                    for (int k = 0; k < (int)parent_group_pattern.sub_group_pattern.size(); k++)
                        if (parent_group_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                            if (need_add.hasCommonVar(parent_group_pattern.sub_group_pattern[k].pattern.subject_object_varset) &&
                                parent_group_pattern.sub_group_pattern[k].pattern.varset.getVarsetSize() == 1)
                            {
                                triple_pattern.addOnePattern(QueryTree::GroupPattern::Pattern(
                                    QueryTree::GroupPattern::Pattern::Element(parent_group_pattern.sub_group_pattern[k].pattern.subject.value),
                                    QueryTree::GroupPattern::Pattern::Element(parent_group_pattern.sub_group_pattern[k].pattern.predicate.value),
                                    QueryTree::GroupPattern::Pattern::Element(parent_group_pattern.sub_group_pattern[k].pattern.object.value)
                                ));
                                need_add = need_add - parent_group_pattern.sub_group_pattern[k].pattern.subject_object_varset;
                            }
                }

                for (int j = 0; j < dep; j++)
                {
                    QueryTree::GroupPattern &parent_group_pattern = this->rewriting_evaluation_stack[j].group_pattern;

                    for (int k = 0; k < (int)parent_group_pattern.sub_group_pattern.size(); k++)
                        if (parent_group_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                            if (need_add.hasCommonVar(parent_group_pattern.sub_group_pattern[k].pattern.subject_object_varset) &&
                                parent_group_pattern.sub_group_pattern[k].pattern.varset.getVarsetSize() == 2)
                            {
                                triple_pattern.addOnePattern(QueryTree::GroupPattern::Pattern(
                                    QueryTree::GroupPattern::Pattern::Element(parent_group_pattern.sub_group_pattern[k].pattern.subject.value),
                                    QueryTree::GroupPattern::Pattern::Element(parent_group_pattern.sub_group_pattern[k].pattern.predicate.value),
                                    QueryTree::GroupPattern::Pattern::Element(parent_group_pattern.sub_group_pattern[k].pattern.object.value)
                                ));
                                need_add = need_add - parent_group_pattern.sub_group_pattern[k].pattern.subject_object_varset;
                            }
                }
            }
            triple_pattern.getVarset();
            */

            //get useful varset
            Varset useful = this->query_tree.getResultProjectionVarset() + this->query_tree.getGroupByVarset();
            if (!this->query_tree.checkProjectionAsterisk())
            {
                for (int j = 0; j < dep; j++)
                {
                    QueryTree::GroupPattern &parent_group_pattern = this->rewriting_evaluation_stack[j].group_pattern;

                    for (int k = 0; k < (int)parent_group_pattern.sub_group_pattern.size(); k++)
                    {
                        if (parent_group_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                            useful += parent_group_pattern.sub_group_pattern[k].pattern.varset;
                        else if (parent_group_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
                            useful += parent_group_pattern.sub_group_pattern[k].filter.varset;
                    }
                }

                for (int j = 0; j < (int)group_pattern->sub_group_pattern.size(); j++)
                {
                    if (group_pattern->sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Optional_type)
                        useful += group_pattern->sub_group_pattern[j].optional.group_pattern_resultset_maximal_varset;
                    else if (group_pattern->sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
                        useful += group_pattern->sub_group_pattern[j].filter.varset;
                }
            }

            SPARQLquery sparql_query;
            vector<vector<string> > encode_varset;
            vector<vector<QueryTree::GroupPattern::Pattern> > basic_query_handle;

            //get connected block
            triple_pattern.initPatternBlockid();

            for (int j = 0; j < (int)triple_pattern.sub_group_pattern.size(); j++)
                if (triple_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                {
                    for (int k = 0; k < j; k++)
                        if (triple_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                            if (triple_pattern.sub_group_pattern[j].pattern.subject_object_varset.hasCommonVar(triple_pattern.sub_group_pattern[k].pattern.subject_object_varset))
                                triple_pattern.mergePatternBlockID(j, k);
                }

            //each connected block is a BasicQuery
            for (int j = 0; j < (int)triple_pattern.sub_group_pattern.size(); j++)
                if (triple_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                    if (triple_pattern.getRootPatternBlockID(j) == j)
                    {
                        Varset occur;
                        vector<QueryTree::GroupPattern::Pattern> basic_query;

                        for (int k = 0; k < (int)triple_pattern.sub_group_pattern.size(); k++)
                            if (triple_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
                                if (triple_pattern.getRootPatternBlockID(k) == j)
                                {
                                    if (k < group_pattern_triple_num)
                                        occur += triple_pattern.sub_group_pattern[k].pattern.varset;

                                    basic_query.push_back(triple_pattern.sub_group_pattern[k].pattern);
                                }

                        //reduce return result vars
                        if (!this->query_tree.checkProjectionAsterisk() && useful.hasCommonVar(occur))
                            useful = useful * occur;
                        else
                            useful = occur;

                        printf("select vars: ");
                        useful.print();

                        printf("triple patterns: \n");
                        for (int k = 0; k < (int)basic_query.size(); k++)
                            printf("%s\t%s\t%s\n", basic_query[k].subject.value.c_str(),
                                basic_query[k].predicate.value.c_str(),
                                basic_query[k].object.value.c_str()
                            );

                        TempResultSet *temp = new TempResultSet();
                        temp->results.push_back(TempResult());

                        bool success = false;
                        if (this->query_cache != NULL && dep == 0)
                        {
                            long tv_bfcheck = Util::get_cur_time();
                            success = this->query_cache->checkCached(basic_query, useful, temp->results[0]);
                            long tv_afcheck = Util::get_cur_time();
                            printf("after checkCache, used %ld ms.\n", tv_afcheck - tv_bfcheck);
                        }

                        if (success)
                        {
                            printf("QueryCache hit\n");
                            printf("Final result size: %d\n", (int)temp->results[0].result.size());

                            TempResultSet *new_result = new TempResultSet();
                            sub_result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

                            sub_result->release();
                            delete sub_result;

                            sub_result = new_result;
                        }
                        else
                        {
                            if (dep == 0)
                                printf("QueryCache miss\n");

                            sparql_query.addBasicQuery();
                            for (int k = 0; k < (int)basic_query.size(); k++)
                                sparql_query.addTriple(Triple(basic_query[k].subject.value,
                                    basic_query[k].predicate.value,
                                    basic_query[k].object.value));

                            encode_varset.push_back(useful.vars);
                            basic_query_handle.push_back(basic_query);
                        }

                        temp->release();
                        delete temp;
                    }

            long tv_begin = Util::get_cur_time();
            sparql_query.encodeQuery(this->kvstore, encode_varset);
            long tv_encode = Util::get_cur_time();
            printf("after Encode, used %ld ms.\n", tv_encode - tv_begin);

            if (dep > 0)
            {
                TempResultSet *&last_result = this->rewriting_evaluation_stack[dep - 1].result;

                for (int j = 0; j < sparql_query.getBasicQueryNum(); j++)
                {
                    BasicQuery &basic_query = sparql_query.getBasicQuery(j);

                    unordered_map<string, int> var_degree;
                    for (int k = 0; k < basic_query.getTripleNum(); k++)
                    {
                        string sub = basic_query.getTriple(k).getSubject();
                        string obj = basic_query.getTriple(k).getObject();

                        if (sub[0] == '?')
                        {
                            if (var_degree.count(sub) == 0)
                                var_degree[sub] = 0;
                            var_degree[sub]++;
                        }

                        if (obj[0] == '?')
                        {
                            if (var_degree.count(obj) == 0)
                                var_degree[obj] = 0;
                            var_degree[obj]++;
                        }
                    }

                    Varset satellite_varset;
                    for (auto it : var_degree)
                        if (it.second == 1)
                            satellite_varset.addVar(it.first);

                    vector<string> &basic_query_encode_varset = encode_varset[j];
                    Varset fillcand_varset;

                    for (int k = 0; k < (int)basic_query_encode_varset.size(); k++)
                    {
                        const string &var = basic_query_encode_varset[k];

                        unordered_set<unsigned> result_set;
                        bool result_set_valid = false;

                        for (int t = 0; t < (int)last_result->results.size(); t++)
                        {
                            vector<TempResult::ResultPair> &result = last_result->results[t].result;

                            int pos = Varset(var).mapTo(last_result->results[t].id_varset)[0];
                            if (pos != -1)
                            {
                                for (int l = 0; l < (int)result.size(); l++)
                                    result_set.insert(result[l].id[pos]);
                                result_set_valid = true;
                            }
                        }

                        if (result_set_valid)
                        {
                            vector<unsigned> result_vector;
                            result_vector.reserve(result_set.size());

                            for (auto x : result_set)
                                result_vector.push_back(x);
                            sort(result_vector.begin(), result_vector.end());

                            basic_query.getCandidateList(k).copy(result_vector);

                            fillcand_varset.addVar(var);
                            if (!satellite_varset.findVar(var))
                                basic_query.setReady(k);

                            printf("fill var %s CandidateList size %d\n", var.c_str(), (int)result_vector.size());
                        }
                    }

                    if (fillcand_varset.belongTo(satellite_varset))
                    {
                        for (int k = 0; k < (int)basic_query_encode_varset.size(); k++)
                        {
                            const string &var = basic_query_encode_varset[k];

                            if (!satellite_varset.findVar(var) && !fillcand_varset.findVar(var))
                            {
                                int min_cand_size = INT32_MAX, min_triple_id = -1;

                                for (int t = 0; t < basic_query.getTripleNum(); t++)
                                {
                                    const string &sub = basic_query.getTriple(t).getSubject();
                                    const string &pre = basic_query.getTriple(t).getPredicate();
                                    const string &obj = basic_query.getTriple(t).getObject();

                                    if (sub[0] == '?' && pre[0] != '?' && obj[0] == '?')
                                    {
                                        int pos = -1;

                                        if (var == sub && satellite_varset.findVar(obj) && fillcand_varset.findVar(obj))
                                            pos = Varset(obj).mapTo(basic_query_encode_varset)[0];
                                        if (var == obj && satellite_varset.findVar(sub) && fillcand_varset.findVar(sub))
                                            pos = Varset(sub).mapTo(basic_query_encode_varset)[0];

                                        if (pos != -1 && basic_query.getCandidateSize(pos) < min_cand_size)
                                            min_cand_size = basic_query.getCandidateSize(pos), min_triple_id = t;
                                    }
                                }

                                if (min_triple_id != -1)
                                {
                                    const string &sub = basic_query.getTriple(min_triple_id).getSubject();
                                    const string &pre = basic_query.getTriple(min_triple_id).getPredicate();
                                    const string &obj = basic_query.getTriple(min_triple_id).getObject();

                                    if (var == sub)
                                    {
                                        int pos = Varset(obj).mapTo(basic_query_encode_varset)[0];
                                        IDList &obj_cand_list = basic_query.getCandidateList(pos);
                                        unsigned pre_id = this->kvstore->getIDByPredicate(pre);
                                        int fill_cand_k = Varset(sub).mapTo(basic_query_encode_varset)[0];

                                        unordered_set<unsigned> result_set;
                                        for (unsigned t = 0; t < obj_cand_list.size(); t++)
                                        {
                                            unsigned obj_id = obj_cand_list.getID(t);

                                            unsigned *sub_cand_list, sub_cand_list_len;
                                            this->kvstore->getsubIDlistByobjIDpreID(obj_id, pre_id, sub_cand_list, sub_cand_list_len);

                                            for (unsigned l = 0; l < sub_cand_list_len; l++)
                                                result_set.insert(sub_cand_list[l]);

                                            delete[] sub_cand_list;
                                        }

                                        vector<unsigned> result_vector;
                                        result_vector.reserve(result_set.size());

                                        for (auto x : result_set)
                                            result_vector.push_back(x);
                                        sort(result_vector.begin(), result_vector.end());

                                        basic_query.getCandidateList(fill_cand_k).copy(result_vector);

                                        fillcand_varset.addVar(sub);
                                        basic_query.setReady(fill_cand_k);

                                        printf("fill extra var %s CandidateList size %d\n", sub.c_str(), (int)result_vector.size());
                                    }

                                    if (var == obj)
                                    {
                                        int pos = Varset(sub).mapTo(basic_query_encode_varset)[0];
                                        IDList &sub_cand_list = basic_query.getCandidateList(pos);
                                        unsigned pre_id = this->kvstore->getIDByPredicate(pre);
                                        int fill_cand_k = Varset(obj).mapTo(basic_query_encode_varset)[0];

                                        unordered_set<unsigned> result_set;
                                        for (unsigned t = 0; t < sub_cand_list.size(); t++)
                                        {
                                            unsigned sub_id = sub_cand_list.getID(t);

                                            unsigned *obj_cand_list, obj_cand_list_len;
                                            this->kvstore->getobjIDlistBysubIDpreID(sub_id, pre_id, obj_cand_list, obj_cand_list_len);

                                            for (unsigned l = 0; l < obj_cand_list_len; l++)
                                                result_set.insert(obj_cand_list[l]);

                                            delete[] obj_cand_list;
                                        }

                                        vector<unsigned> result_vector;
                                        result_vector.reserve(result_set.size());

                                        for (auto x : result_set)
                                            result_vector.push_back(x);
                                        sort(result_vector.begin(), result_vector.end());

                                        basic_query.getCandidateList(fill_cand_k).copy(result_vector);

                                        fillcand_varset.addVar(obj);
                                        basic_query.setReady(fill_cand_k);

                                        printf("fill extra var %s CandidateList size %d\n", obj.c_str(), (int)result_vector.size());
                                    }
                                }
                            }
                        }
                    }
                }
            }
            long tv_fillcand = Util::get_cur_time();
            printf("after FillCand, used %ld ms.\n", tv_fillcand - tv_encode);

            this->strategy.handle(sparql_query);
            long tv_handle = Util::get_cur_time();
            printf("after Handle, used %ld ms.\n", tv_handle - tv_fillcand);

            //collect and join the result of each BasicQuery
            for (int j = 0; j < sparql_query.getBasicQueryNum(); j++)
            {
                TempResultSet *temp = new TempResultSet();
                temp->results.push_back(TempResult());

                temp->results[0].id_varset = Varset(encode_varset[j]);
                int varnum = (int)encode_varset[j].size();

                vector<unsigned*> &basicquery_result = sparql_query.getBasicQuery(j).getResultList();
                int basicquery_result_num = (int)basicquery_result.size();

                temp->results[0].result.reserve(basicquery_result_num);
                for (int k = 0; k < basicquery_result_num; k++)
                {
                    unsigned *v = new unsigned[varnum];
                    memcpy(v, basicquery_result[k], sizeof(int) * varnum);
                    temp->results[0].result.push_back(TempResult::ResultPair());
                    temp->results[0].result.back().id = v;
                }

                if (this->query_cache != NULL && dep == 0)
                {
                    //if unconnected, time is incorrect
                    int time = tv_handle - tv_begin;

                    long tv_bftry = Util::get_cur_time();
                    bool success = this->query_cache->tryCaching(basic_query_handle[j], temp->results[0], time);
                    if (success)	printf("QueryCache cached\n");
                    else			printf("QueryCache didn't cache\n");
                    long tv_aftry = Util::get_cur_time();
                    printf("after tryCache, used %ld ms.\n", tv_aftry - tv_bftry);
                }

                if (sub_result->results.empty())
                {
                    delete sub_result;
                    sub_result = temp;
                }
                else
                {
                    TempResultSet *new_result = new TempResultSet();
                    sub_result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

                    temp->release();
                    sub_result->release();
                    delete temp;
                    delete sub_result;

                    sub_result = new_result;
                }
            }
        }

        for (int j = 0; j < (int)group_pattern->sub_group_pattern.size(); j++)
            if (group_pattern->sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Bind_type)
            {
                TempResultSet *temp = new TempResultSet();
                temp->results.push_back(TempResult());

                temp->results[0].str_varset = group_pattern->sub_group_pattern[j].bind.varset;

                temp->results[0].result.push_back(TempResult::ResultPair());
                temp->results[0].result[0].str.push_back(group_pattern->sub_group_pattern[j].bind.str);

                TempResultSet *new_result = new TempResultSet();
                sub_result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

                temp->release();
                sub_result->release();
                delete temp;
                delete sub_result;

                sub_result = new_result;
            }

        for (int j = 0; j < (int)group_pattern->sub_group_pattern.size(); j++)
            if (group_pattern->sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
                if (!group_pattern->sub_group_pattern[j].filter.done && group_pattern->sub_group_pattern[j].filter.varset.belongTo(group_pattern->group_pattern_resultset_minimal_varset))
                {
                    group_pattern->sub_group_pattern[j].filter.done = true;

                    TempResultSet *new_result = new TempResultSet();
                    sub_result->doFilter(group_pattern->sub_group_pattern[j].filter.root, *new_result, this->stringindex, group_pattern->group_pattern_subject_object_maximal_varset);

                    sub_result->release();
                    delete sub_result;

                    sub_result = new_result;
                }

        if (sub_result->results.empty() || !sub_result->results[0].result.empty())
            for (int j = 0; j < (int)group_pattern->sub_group_pattern.size(); j++)
                if (group_pattern->sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Optional_type)
                {
                    if ((int)this->rewriting_evaluation_stack.size() == dep + 1)
                    {
                        this->rewriting_evaluation_stack.push_back(EvaluationStackStruct());
                        this->rewriting_evaluation_stack.back().result = NULL;

                        //rebind group_pattern avoiding failure due to reallocation of vector memory
                        group_pattern = &this->rewriting_evaluation_stack[dep].group_pattern;
                    }

                    this->rewriting_evaluation_stack[dep].result = sub_result;
                    this->rewriting_evaluation_stack[dep + 1].group_pattern = group_pattern->sub_group_pattern[j].optional;

                    TempResultSet *temp = rewritingBasedQueryEvaluation(dep + 1);

                    //rebind group_pattern avoiding failure due to reallocation of vector memory
                    group_pattern = &this->rewriting_evaluation_stack[dep].group_pattern;

                    TempResultSet *new_result = new TempResultSet();
                    sub_result->doOptional(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

                    temp->release();
                    sub_result->release();
                    delete temp;
                    delete sub_result;

                    sub_result = new_result;
                }

        for (int j = 0; j < (int)group_pattern->sub_group_pattern.size(); j++)
            if (group_pattern->sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
                if (!group_pattern->sub_group_pattern[j].filter.done)
                {
                    group_pattern->sub_group_pattern[j].filter.done = true;

                    TempResultSet *new_result = new TempResultSet();
                    sub_result->doFilter(group_pattern->sub_group_pattern[j].filter.root, *new_result, this->stringindex, group_pattern->group_pattern_subject_object_maximal_varset);

                    sub_result->release();
                    delete sub_result;

                    sub_result = new_result;
                }

        if (result->results.empty())
        {
            delete result;
            result = sub_result;
        }
        else
        {
            TempResultSet *new_result = new TempResultSet();
            result->doUnion(*sub_result, *new_result);

            sub_result->release();
            result->release();
            delete sub_result;
            delete result;

            result = new_result;
        }
    }

    return result;
}

void GeneralEvaluation::getFinalResult(ResultSet &ret_result)
{
    if (this->temp_result == NULL)
        return;

    if (this->query_tree.getQueryForm() == QueryTree::Select_Query)
    {
        long t0 = Util::get_cur_time();

        if (this->temp_result->results.empty())
        {
            this->temp_result->results.push_back(TempResult());
            this->temp_result->results[0].id_varset += this->query_tree.getGroupPattern().group_pattern_resultset_maximal_varset;
        }

        Varset useful = this->query_tree.getResultProjectionVarset() + this->query_tree.getGroupByVarset();

        if (this->query_tree.checkProjectionAsterisk())
        {
            for (int i = 0; i < (int)this->temp_result->results.size(); i++)
                useful += this->temp_result->results[i].getAllVarset();
        }

        if ((int)this->temp_result->results.size() > 1 || this->query_tree.getProjectionModifier() == QueryTree::Modifier_Distinct)
        {
            TempResultSet *new_temp_result = new TempResultSet();

            this->temp_result->doProjection1(useful, *new_temp_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

            this->temp_result->release();
            delete this->temp_result;

            this->temp_result = new_temp_result;
        }

        if (this->query_tree.checkAtLeastOneAggregateFunction() || !this->query_tree.getGroupByVarset().empty())
        {
            vector<QueryTree::ProjectionVar> &proj = this->query_tree.getProjection();

            TempResultSet *new_temp_result = new TempResultSet();
            new_temp_result->results.push_back(TempResult());

            TempResult &result0 = this->temp_result->results[0];
            TempResult &new_result0 = new_temp_result->results[0];

            for (int i = 0; i < (int)proj.size(); i++)
                if (proj[i].aggregate_type == QueryTree::ProjectionVar::None_type)
                {
                    if (result0.id_varset.findVar(proj[i].var))
                        new_result0.id_varset.addVar(proj[i].var);
                    else if (result0.str_varset.findVar(proj[i].var))
                        new_result0.str_varset.addVar(proj[i].var);
                }
                else
                    new_result0.str_varset.addVar(proj[i].var);

            vector<int> proj2temp((int)proj.size(), -1);
            for (int i = 0; i < (int)proj.size(); i++)
                if (proj[i].aggregate_type == QueryTree::ProjectionVar::None_type)
                    proj2temp[i] = Varset(proj[i].var).mapTo(result0.getAllVarset())[0];
                else if (proj[i].aggregate_var != "*")
                    proj2temp[i] = Varset(proj[i].aggregate_var).mapTo(result0.getAllVarset())[0];

            vector<int> proj2new = this->query_tree.getProjectionVarset().mapTo(new_result0.getAllVarset());

            int result0_size = (int)result0.result.size();
            vector<int> group2temp;

            if (!this->query_tree.getGroupByVarset().empty())
            {
                group2temp = this->query_tree.getGroupByVarset().mapTo(result0.getAllVarset());
                result0.sort(0, result0_size - 1, group2temp);
            }

            TempResultSet *temp_result_distinct = NULL;
            vector<int> group2distinct;

            for (int i = 0; i < (int)proj.size(); i++)
                if (proj[i].aggregate_type == QueryTree::ProjectionVar::Count_type && proj[i].distinct && proj[i].aggregate_var == "*")
                {
                    temp_result_distinct = new TempResultSet();

                    this->temp_result->doDistinct1(*temp_result_distinct);
                    group2distinct = this->query_tree.getGroupByVarset().mapTo(temp_result_distinct->results[0].getAllVarset());
                    temp_result_distinct->results[0].sort(0, (int)temp_result_distinct->results[0].result.size() - 1, group2distinct);

                    break;
                }

            int result0_id_cols = result0.id_varset.getVarsetSize();
            int new_result0_id_cols = new_result0.id_varset.getVarsetSize();
            int new_result0_str_cols = new_result0.str_varset.getVarsetSize();
            for (int begin = 0; begin < result0_size;)
            {
                int end;
                if (group2temp.empty())
                    end = result0_size - 1;
                else
                    end = result0.findRightBounder(group2temp, result0.result[begin], result0_id_cols, group2temp);

                new_result0.result.push_back(TempResult::ResultPair());
                new_result0.result.back().id = new unsigned[new_result0_id_cols];
                new_result0.result.back().str.resize(new_result0_str_cols);

                for (int i = 0; i < new_result0_id_cols; i++)
                    new_result0.result.back().id[i] = INVALID;

                for (int i = 0; i < (int)proj.size(); i++)
                    if (proj[i].aggregate_type == QueryTree::ProjectionVar::None_type)
                    {
                        if (proj2temp[i] < result0_id_cols)
                            new_result0.result.back().id[proj2new[i]] = result0.result[begin].id[proj2temp[i]];
                        else
                            new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = result0.result[begin].str[proj2temp[i] - result0_id_cols];
                    }
                    else if (proj[i].aggregate_type == QueryTree::ProjectionVar::Count_type)
                    {
                        int count = 0;

                        if (proj[i].aggregate_var != "*")
                        {
                            if (proj[i].distinct)
                            {
                                if (proj2temp[i] < result0_id_cols)
                                {
                                    unordered_set<int> count_set;
                                    for (int j = begin; j <= end; j++)
                                        if (result0.result[j].id[proj2temp[i]] != INVALID)
                                            count_set.insert(result0.result[j].id[proj2temp[i]]);
                                    count = (int)count_set.size();
                                }
                                else
                                {
                                    unordered_set<string> count_set;
                                    for (int j = begin; j <= end; j++)
                                        if (result0.result[j].str[proj2temp[i] - result0_id_cols].length() > 0)
                                            count_set.insert(result0.result[j].str[proj2temp[i] - result0_id_cols]);
                                    count = (int)count_set.size();
                                }
                            }
                            else
                            {
                                if (proj2temp[i] < result0_id_cols)
                                {
                                    for (int j = begin; j <= end; j++)
                                        if (result0.result[j].id[proj2temp[i]] != INVALID)
                                            count++;
                                }
                                else
                                {
                                    for (int j = begin; j <= end; j++)
                                        if (result0.result[j].str[proj2temp[i] - result0_id_cols].length() > 0)
                                            count++;
                                }
                            }
                        }
                        else
                        {
                            if (proj[i].distinct)
                            {
                                count = temp_result_distinct->results[0].findRightBounder(group2distinct, result0.result[begin], result0_id_cols, group2temp) -
                                    temp_result_distinct->results[0].findLeftBounder(group2distinct, result0.result[begin], result0_id_cols, group2temp) + 1;
                            }
                            else
                            {
                                count = end - begin + 1;
                            }
                        }

                        stringstream ss;
                        ss << "\"";
                        ss << count;
                        ss << "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
                        ss >> new_result0.result.back().str[proj2new[i] - new_result0_id_cols];
                    }

                begin = end + 1;
            }

            if (temp_result_distinct != NULL)
            {
                temp_result_distinct->release();
                delete temp_result_distinct;
            }

            this->temp_result->release();
            delete this->temp_result;

            this->temp_result = new_temp_result;
        }

        //temp_result --> ret_result

        if (this->query_tree.getProjectionModifier() == QueryTree::Modifier_Distinct)
        {
            TempResultSet *new_temp_result = new TempResultSet();

            this->temp_result->doDistinct1(*new_temp_result);

            this->temp_result->release();
            delete this->temp_result;

            this->temp_result = new_temp_result;
        }

        TempResult &result0 = this->temp_result->results[0];
        Varset ret_result_varset;

        if (this->query_tree.checkProjectionAsterisk() && this->query_tree.getProjectionVarset().empty())
        {
            ret_result.select_var_num = result0.getAllVarset().getVarsetSize();
            ret_result.setVar(result0.getAllVarset().vars);
            ret_result_varset = result0.getAllVarset();
        }
        else
        {
            ret_result.select_var_num = this->query_tree.getProjectionVarset().getVarsetSize();
            ret_result.setVar(this->query_tree.getProjectionVarset().vars);
            ret_result_varset = this->query_tree.getProjectionVarset();
        }

        ret_result.ansNum = (int)result0.result.size();
        ret_result.setOutputOffsetLimit(this->query_tree.getOffset(), this->query_tree.getLimit());

#ifdef STREAM_ON
        long long ret_result_size = (long long)ret_result.ansNum * (long long)ret_result.select_var_num * 100 / Util::GB;
        if (Util::memoryLeft() < ret_result_size || !this->query_tree.getOrderVarVector().empty())
        {
            ret_result.setUseStream();
            printf("set use Stream\n");
        }
#endif

        if (!ret_result.checkUseStream())
        {
            ret_result.answer = new string*[ret_result.ansNum];
            for (unsigned i = 0; i < ret_result.ansNum; i++)
                ret_result.answer[i] = NULL;
        }
        else
        {
            vector <unsigned> keys;
            vector <bool> desc;
            for (int i = 0; i < (int)this->query_tree.getOrderVarVector().size(); i++)
            {
                int var_id = Varset(this->query_tree.getOrderVarVector()[i].var).mapTo(ret_result_varset)[0];
                if (var_id != -1)
                {
                    keys.push_back(var_id);
                    desc.push_back(this->query_tree.getOrderVarVector()[i].descending);
                }
            }
            ret_result.openStream(keys, desc);
        }

        vector<int> proj2temp = ret_result_varset.mapTo(result0.getAllVarset());
        int id_cols = result0.id_varset.getVarsetSize();

        vector<bool> isel;
        for (int i = 0; i < result0.id_varset.getVarsetSize(); i++)
            isel.push_back(this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset.findVar(result0.id_varset.vars[i]));

        if (!ret_result.checkUseStream())
        {
            //pthread_t tidp;
            //long arg[6];
            vector<StringIndexFile* > a = this->stringindex->get_three_StringIndexFile();
            /*
            arg[0] = (long)&a;
            arg[1] = (long)&ret_result;
            arg[2] = (long)&proj2temp;
            arg[3] = (long)id_cols;
            arg[4] = (long)&result0;
            arg[5] = (long)&isel;
            pthread_create(&tidp, NULL, &preread_from_index, arg);
            */

            unsigned retAnsNum = ret_result.ansNum;
            unsigned selectVar = ret_result.select_var_num;

            /*
            int counterEntity = 0;
            int counterLiteral = 0;
            int counterPredicate = 0;
            for (int j = 0; j < selectVar; j++)
            {
                int k = proj2temp[j];
                if (k != -1)
                {
                    if (k < id_cols)
                    {
                        if (isel[k])
                        {
                            for (unsigned i = 0; i < retAnsNum; i++)
                            {
                                unsigned ans_id = result0.result[i].id[k];
                                if (ans_id == INVALID)
                                    continue;
                                if (ans_id < Util::LITERAL_FIRST_ID)
                                    counterEntity++;
                                else
                                    counterLiteral++;
                            }
                        }
                        else
                            for (unsigned i = 0; i < retAnsNum; i++)
                            {
                                unsigned ans_id = result0.result[i].id[k];
                                if (ans_id == INVALID)
                                    continue;
                                counterPredicate++;
                            }
                    }
                }
            }
            a[0]->request_reserve(counterEntity);
            a[1]->request_reserve(counterLiteral);
            a[2]->request_reserve(counterPredicate);
            */

            ret_result.delete_another_way = 1;
            string *t = new string[retAnsNum*selectVar];
            for (unsigned int i = 0, off = 0; i < retAnsNum; i++, off += selectVar)
                ret_result.answer[i] = t + off;

            a[0]->set_string_base(t);
            a[1]->set_string_base(t);
            a[2]->set_string_base(t);

            for (int j = 0; j < selectVar; j++)
            {
                int k = proj2temp[j];
                if (k != -1)
                {
                    if (k < id_cols)
                    {
                        if (isel[k])
                        {
                            for (unsigned i = 0; i < retAnsNum; i++)
                            {
                                unsigned ans_id = result0.result[i].id[k];
                                if (ans_id != INVALID)
                                {
                                    if (ans_id < Util::LITERAL_FIRST_ID)
                                        a[0]->addRequest(ans_id, i * selectVar + j);
                                    else
                                        a[1]->addRequest(ans_id - Util::LITERAL_FIRST_ID, i * selectVar + j);
                                }
                            }
                        }
                        else
                        {
                            for (unsigned i = 0; i < retAnsNum; i++)
                            {
                                unsigned ans_id = result0.result[i].id[k];
                                if (ans_id != INVALID)
                                    a[2]->addRequest(ans_id, i * selectVar + j);
                            }
                        }
                    }
                    else
                    {
                        for (unsigned i = 0; i < retAnsNum; i++)
                            ret_result.answer[i][j] = result0.result[i].str[k - id_cols];
                    }
                }
            }
            cout << "in getFinal Result the first half use " << Util::get_cur_time() - t0 << "  ms" << endl;
            //pthread_join(tidp, NULL);
            this->stringindex->trySequenceAccess(true, -1);
        }
        else
        {
            for (unsigned i = 0; i < ret_result.ansNum; i++)
                for (int j = 0; j < ret_result.select_var_num; j++)
                {
                    int k = proj2temp[j];
                    if (k != -1)
                    {
                        if (k < id_cols)
                        {
                            string ans_str;

                            unsigned ans_id = result0.result[i].id[k];
                            if (ans_id != INVALID)
                            {
                                this->stringindex->randomAccess(ans_id, &ans_str, isel[k]);
                            }
                            ret_result.writeToStream(ans_str);
                        }
                        else
                        {
                            string ans_str = result0.result[i].str[k - id_cols];
                            ret_result.writeToStream(ans_str);
                        }
                    }
                }

            ret_result.resetStream();
        }
    }
    else if (this->query_tree.getQueryForm() == QueryTree::Ask_Query)
    {
        ret_result.select_var_num = 1;
        ret_result.setVar(vector<string>(1, "?_askResult"));
        ret_result.ansNum = 1;

        if (!ret_result.checkUseStream())
        {
            ret_result.answer = new string*[ret_result.ansNum];
            ret_result.answer[0] = new string[ret_result.select_var_num];

            ret_result.answer[0][0] = "false";
            for (int i = 0; i < (int)this->temp_result->results.size(); i++)
                if (!this->temp_result->results[i].result.empty())
                    ret_result.answer[0][0] = "true";
        }
    }

    this->releaseResult();
}

void GeneralEvaluation::releaseResult()
{
    if (this->temp_result == NULL)
        return;

    this->temp_result->release();
    delete this->temp_result;
    this->temp_result = NULL;
}

void GeneralEvaluation::prepareUpdateTriple(QueryTree::GroupPattern &update_pattern, TripleWithObjType *&update_triple, unsigned &update_triple_num)
{
    update_pattern.getVarset();

    if (update_triple != NULL)
    {
        delete[] update_triple;
        update_triple = NULL;
    }

    if (this->temp_result == NULL)
        return;

    update_triple_num = 0;

    for (int i = 0; i < (int)update_pattern.sub_group_pattern.size(); i++)
        if (update_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
        {
            for (int j = 0; j < (int)this->temp_result->results.size(); j++)
                if (update_pattern.sub_group_pattern[i].pattern.varset.belongTo(this->temp_result->results[j].getAllVarset()))
                    update_triple_num += this->temp_result->results[j].result.size();
        }

    update_triple = new TripleWithObjType[update_triple_num];

    int update_triple_count = 0;
    for (int i = 0; i < (int)update_pattern.sub_group_pattern.size(); i++)
        if (update_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
        {
            for (int j = 0; j < (int)this->temp_result->results.size(); j++)
            {
                int id_cols = this->temp_result->results[j].id_varset.getVarsetSize();

                if (update_pattern.sub_group_pattern[i].pattern.varset.belongTo(this->temp_result->results[j].getAllVarset()))
                {
                    int subject_id = -1, predicate_id = -1, object_id = -1;
                    if (update_pattern.sub_group_pattern[i].pattern.subject.value[0] == '?')
                        subject_id = Varset(update_pattern.sub_group_pattern[i].pattern.subject.value).mapTo(this->temp_result->results[j].getAllVarset())[0];
                    if (update_pattern.sub_group_pattern[i].pattern.predicate.value[0] == '?')
                        predicate_id = Varset(update_pattern.sub_group_pattern[i].pattern.predicate.value).mapTo(this->temp_result->results[j].getAllVarset())[0];
                    if (update_pattern.sub_group_pattern[i].pattern.object.value[0] == '?')
                        object_id = Varset(update_pattern.sub_group_pattern[i].pattern.object.value).mapTo(this->temp_result->results[j].getAllVarset())[0];

                    string subject, predicate, object;
                    TripleWithObjType::ObjectType object_type;

                    if (subject_id == -1)
                        subject = update_pattern.sub_group_pattern[i].pattern.subject.value;
                    if (predicate_id == -1)
                        predicate = update_pattern.sub_group_pattern[i].pattern.predicate.value;
                    if (object_id == -1)
                        object = update_pattern.sub_group_pattern[i].pattern.object.value;

                    for (int k = 0; k < (int)this->temp_result->results[j].result.size(); k++)
                    {
                        if (subject_id != -1)
                        {
                            if (subject_id < id_cols)
                                this->stringindex->randomAccess(this->temp_result->results[j].result[k].id[subject_id], &subject, true);
                            else
                                subject = this->temp_result->results[j].result[k].str[subject_id - id_cols];
                        }

                        if (predicate_id != -1)
                        {
                            if (predicate_id < id_cols)
                                this->stringindex->randomAccess(this->temp_result->results[j].result[k].id[predicate_id], &predicate, false);
                            else
                                predicate = this->temp_result->results[j].result[k].str[predicate_id - id_cols];
                        }

                        if (object_id != -1)
                        {
                            if (object_id < id_cols)
                                this->stringindex->randomAccess(this->temp_result->results[j].result[k].id[object_id], &object, true);
                            else
                                object = this->temp_result->results[j].result[k].str[object_id - id_cols];
                        }

                        if (object[0] == '<')
                            object_type = TripleWithObjType::Entity;
                        else
                            object_type = TripleWithObjType::Literal;

                        update_triple[update_triple_count++] = TripleWithObjType(subject, predicate, object, object_type);
                    }
                }
            }
        }
}
