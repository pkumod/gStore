/*=============================================================================
# Filename: GeneralEvaluation.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-03-02 20:35
# Description: implement functions in GeneralEvaluation.h
# modified by zengli
=============================================================================*/

#include "GeneralEvaluation.h"

using namespace std;

vector<vector<string> > GeneralEvaluation::getSPARQLQueryVarset()
{
    vector<vector<string> > res;
    for (int i = 0; i < (int)this->sparql_query_varset.size(); i++)
        res.push_back(this->sparql_query_varset[i].varset);
    return res;
}

void GeneralEvaluation::doQuery(const string &_query, int myRank, string &internal_tag_str, string& lpm_str)
{
    long tv_begin = Util::get_cur_time();

    if (!this->parseQuery(_query))
        return;
    long tv_parse = Util::get_cur_time();
    //cout << "after Parsing, used " << (tv_parse - tv_begin) << "ms." << endl;

    this->query_tree.getGroupPattern().getVarset();

	{
		//cout << "=====================" << endl;
		//cout << "||not well-designed||" << endl;
		//cout << "=====================" << endl;

		this->getBasicQuery(this->query_tree.getGroupPattern());
		this->sparql_query.encodeQuery(this->kvstore, this->getSPARQLQueryVarset());
		
		Strategy stra(this->kvstore, this->vstree);
		stra.handle(this->sparql_query, myRank, internal_tag_str);
		long tv_handle = Util::get_cur_time();
		//cout << "after handle, used " << (tv_handle - tv_encode) << "ms." << endl;
		
		this->generateEvaluationPlan(this->query_tree.getGroupPattern());
		this->doEvaluationPlan();
		long tv_postpro = Util::get_cur_time();
		//cout << "after Postprocessing, used " << (tv_postpro - tv_handle) << "ms." << endl;
	}

	stringstream lpm_res_ss;	
	if(this->query_tree.getQueryForm() == QueryTree::Select_Query){
		vector<BasicQuery*>& queryList = this->sparql_query.getBasicQueryVec();
		vector<BasicQuery*>::iterator iter = queryList.begin();
		for(; iter != queryList.end(); iter++)
		{
			vector<int*>& result_list = (*iter)->getResultList();
			int varNum = (*iter)->getVarNum();
			for(unsigned i = 0; i < result_list.size(); i ++)
			{
				int* _p_int = result_list[i];			
				for(int j = 0; j < varNum; j++)
				{
					string tmp_ans;
					if (_p_int[j] == -1){
						lpm_res_ss << "-1\t";
					}else if (_p_int[j] < Util::LITERAL_FIRST_ID){
						lpm_res_ss << internal_tag_str.at(_p_int[j]) << this->kvstore->getEntityByID(_p_int[j]) << "\t";
					}else{
						lpm_res_ss << "1" << this->kvstore->getLiteralByID(_p_int[j]) << "\t";
					}
				}
				lpm_res_ss << endl;
			}
		}
		//cout << "lpm_res_str : " << lpm_res_ss.str() << endl;
	}else{
		vector<BasicQuery*>& queryList = this->sparql_query.getBasicQueryVec();
		vector<BasicQuery*>::iterator iter = queryList.begin();
		for(; iter != queryList.end(); iter++)
		{
			vector<int*>& all_result_list = (*iter)->getResultList();
			int var_num = (*iter)->getVarNum();
			
			char* dealed_internal_id_sign = new char[var_num + 1];
			set<string> LECF_set;
			
			for(vector<int*>::iterator it = all_result_list.begin(); it != all_result_list.end(); it++){
				memset(dealed_internal_id_sign, '0', sizeof(bool) * var_num);
				dealed_internal_id_sign[var_num] = 0;
				int* result_var = *it;		
				stringstream res_ss;
				
				for(int j = 0; j < var_num; j++){
					int var_degree = (*iter)->getVarDegree(j);
					
					if(result_var[j] == -1)
						continue;
					
					for (int i = 0; i < var_degree; i++)
					{
						if(result_var[j] >= Util::LITERAL_FIRST_ID || internal_tag_str.at(result_var[j]) == '1'){
							dealed_internal_id_sign[j] = '1';
						}else{
							continue;
						}
						// each triple/edge need to be processed only once.
						int edge_id = (*iter)->getEdgeID(j, i);				
						int var_id2 = (*iter)->getEdgeNeighborID(j, i);
						if (var_id2 == -1)
						{
							continue;
						}
						
						if(result_var[var_id2] != -1){
							if(result_var[var_id2] < Util::LITERAL_FIRST_ID && internal_tag_str.at(result_var[var_id2]) == '0'){
								string _tmp_1, _tmp_2;
								if(result_var[j] < Util::LITERAL_FIRST_ID){
									_tmp_1 = (this->kvstore)->getEntityByID(result_var[j]);
								}else{
									_tmp_1 = (this->kvstore)->getLiteralByID(result_var[j]);
								}
								
								if(result_var[var_id2] < Util::LITERAL_FIRST_ID){
									_tmp_2 = (this->kvstore)->getEntityByID(result_var[var_id2]);
								}else{
									_tmp_2 = (this->kvstore)->getLiteralByID(result_var[var_id2]);
								}
								if(j < var_id2){
									res_ss << j << "\t" << var_id2 << "\t" << _tmp_1 << "\t" << _tmp_2 << "\t";
								}else{
									res_ss << var_id2 << "\t" << j << "\t" << _tmp_2 << "\t" << _tmp_1 << "\t";
								}
							}
						}
					}
				}
				res_ss << dealed_internal_id_sign << endl;
				//log_output << res_ss.str() << endl;
				LECF_set.insert(res_ss.str());
			}
			delete[] dealed_internal_id_sign;
			
			for(set<string>::iterator it1 = LECF_set.begin(); it1 != LECF_set.end(); it1++){
				lpm_res_ss << *it1 << endl;
			}
			lpm_str = lpm_res_ss.str();
		}
	}
	
	 lpm_str = lpm_res_ss.str();
}

void GeneralEvaluation::doQuery(const string &_query)
{
    long tv_begin = Util::get_cur_time();

    if (!this->parseQuery(_query))
        return;
    long tv_parse = Util::get_cur_time();
    cout << "after Parsing, used " << (tv_parse - tv_begin) << "ms." << endl;

    this->query_tree.getGroupPattern().getVarset();

	//if(this->query_tree.getQueryForm() == QueryTree::Select_Query && this->query_tree.checkWellDesigned())
	//{
	//	cout << "=================" << endl;
	//	cout << "||well-designed||" << endl;
	//	cout << "=================" << endl;

	//	this->expansion_evaluation_stack.clear();
	//	this->expansion_evaluation_stack.reserve(100);
	//	this->expansion_evaluation_stack.push_back(ExpansionEvaluationStackUnit());
	//	this->expansion_evaluation_stack[0].grouppattern = this->query_tree.getGroupPattern();
	//	this->queryRewriteEncodeRetrieveJoin(0, this->result_filter);

	//	this->semantic_evaluation_result_stack.push(this->expansion_evaluation_stack[0].result);
	//}
	//else
	{
		cout << "=====================" << endl;
		cout << "||not well-designed||" << endl;
		cout << "=====================" << endl;

		this->getBasicQuery(this->query_tree.getGroupPattern());

		//vector< vector<string> > tmp;
		//tmp.push_back(this->query_tree.getProjection());
		//this->sparql_query.encodeQuery(this->kvstore, this->getProjection());
		this->sparql_query.encodeQuery(this->kvstore, this->getSPARQLQueryVarset());
		cout << "sparqlSTR:\t" << this->sparql_query.to_str() << endl;
		long tv_encode = Util::get_cur_time();
		cout << "after Encode, used " << (tv_encode - tv_parse) << "ms." << endl;

		//NOTICE: use this strategy instead of default filter-join way
		Strategy stra(this->kvstore, this->vstree);
		stra.handle(this->sparql_query);
		long tv_handle = Util::get_cur_time();
		cout << "after handle, used " << (tv_handle - tv_encode) << "ms." << endl;

		
		//this->vstree->retrieve(this->sparql_query);
		//long tv_retrieve = Util::get_cur_time();
		//cout << "after Retrieve, used " << (tv_retrieve - tv_encode) << "ms." << endl;

		//Join *join = new Join(kvstore);
		//join->join_sparql(this->sparql_query);
		//delete join;
		//long tv_join = Util::get_cur_time();
		//cout << "after Join, used " << (tv_join - tv_retrieve) << "ms." << endl;
		
		this->generateEvaluationPlan(this->query_tree.getGroupPattern());
		this->doEvaluationPlan();
		long tv_postpro = Util::get_cur_time();
		cout << "after Postprocessing, used " << (tv_postpro - tv_handle) << "ms." << endl;
	}

	long tv_bfget = Util::get_cur_time();
	this->getFinalResult(this->result_set);
	long tv_final = Util::get_cur_time();
	cout << "after getFinalResult, used " << (tv_final - tv_bfget) << "ms." << endl;

	//BETTER:output final size and total time at last
    cout << "Total time used: " << (tv_final - tv_begin) << "ms." << endl;
}

bool GeneralEvaluation::parseQuery(const string &_query)
{
    try
    {
        this->query_parser.sparqlParser(_query, this->query_tree);
    }
    catch(const char* e)
    {
        cerr << e << endl;
        return false;
    }
    return true;
}

bool GeneralEvaluation::onlyParseQuery(const string &_query, int& var_num, QueryTree::QueryForm& query_form)
{
    try
    {
        this->query_parser.sparqlParser(_query, this->query_tree);
    }
    catch(const char* e)
    {
        cerr << e << endl;
        return false;
    }
	
	var_num = this->query_tree.getGroupPattern().getVarNum();
	if(this->query_tree.getQueryForm() == QueryTree::Ask_Query){
		query_form = QueryTree::Ask_Query;
	}else{
		query_form = QueryTree::Select_Query;
	}
	
    return true;
}

void GeneralEvaluation::getBasicQuery(QueryTree::GroupPattern &grouppattern)
{
    for (int i = 0; i < (int)grouppattern.unions.size(); i++)
        for (int j = 0; j < (int)grouppattern.unions[i].grouppattern_vec.size(); j++)
            getBasicQuery(grouppattern.unions[i].grouppattern_vec[j]);
    for (int i = 0; i < (int)grouppattern.optionals.size(); i++)
        getBasicQuery(grouppattern.optionals[i].grouppattern);

    int current_optional = 0;
    int first_patternid = 0;

    grouppattern.initPatternBlockid();
    vector<int> basicqueryid((int)grouppattern.patterns.size(), 0);

    for (int i = 0; i < (int)grouppattern.patterns.size(); i++)
    {
        for (int j = first_patternid; j < i; j++)
            if (grouppattern.patterns[i].varset.hasCommonVar(grouppattern.patterns[j].varset))
                grouppattern.mergePatternBlockid(i, j);

        if ((current_optional != (int)grouppattern.optionals.size() && i == grouppattern.optionals[current_optional].lastpattern) || i + 1 == (int)grouppattern.patterns.size())
        {
            for (int j = first_patternid; j <= i; j++)
                if ((int)grouppattern.patterns[j].varset.varset.size() > 0)
                {
                    if (grouppattern.getRootPatternBlockid(j) == j)			//root node
                    {
                        this->sparql_query.addBasicQuery();
                        this->sparql_query_varset.push_back(Varset());

                        for (int k = first_patternid; k <= i; k++)
                            if (grouppattern.getRootPatternBlockid(k) == j)
                            {
                                this->sparql_query.addTriple(Triple(
                                                                 grouppattern.patterns[k].subject.value,
                                                                 grouppattern.patterns[k].predicate.value,
                                                                 grouppattern.patterns[k].object.value));

                                basicqueryid[k] = this->sparql_query.getBasicQueryNum() - 1;
                                this->sparql_query_varset[(int)this->sparql_query_varset.size() - 1] = this->sparql_query_varset[(int)this->sparql_query_varset.size() - 1] + grouppattern.patterns[k].varset;
                            }
                    }
                }
                else	basicqueryid[j] = -1;

            for (int j = first_patternid; j <= i; j++)
                grouppattern.pattern_blockid[j] = basicqueryid[j];

            if (current_optional != (int)grouppattern.optionals.size())	current_optional++;
            first_patternid = i + 1;
        }
    }

    for(int i = 0; i < (int)grouppattern.filter_exists_grouppatterns.size(); i++)
        for (int j = 0; j < (int)grouppattern.filter_exists_grouppatterns[i].size(); j++)
            getBasicQuery(grouppattern.filter_exists_grouppatterns[i][j]);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void GeneralEvaluation::TempResult::release()
{
    for (int i = 0; i < (int)this->res.size(); i++)
        delete[] res[i];
}

int GeneralEvaluation::TempResult::compareFunc(int *a, vector<int> &p, int *b, vector<int> &q)			//compare a[p] & b[q]
{
    int p_size = (int)p.size();
    for (int i = 0; i < p_size; i++)
    {
        if (a[p[i]] < b[q[i]])		return -1;
        if (a[p[i]] > b[q[i]])		return 1;
    }
    return 0;
}

void GeneralEvaluation::TempResult::sort(int l, int r, vector<int> &p)
{
    int i = l, j = r;
    int* x = this->res[(l + r) / 2];
    do
    {
        while (compareFunc(this->res[i], p, x, p) == -1)	i++;
        while(compareFunc(x, p, this->res[j], p) == -1)	j--;
        if (i <= j)
        {
            swap(this->res[i], this->res[j]);
            i++;
            j--;
        }
    }
    while (i <= j);
    if (l < j)	sort(l, j, p);
    if (i < r)	sort(i, r, p);
}

int GeneralEvaluation::TempResult::findLeftBounder(vector<int> &p, int *b, vector<int> &q)
{
    int l = 0, r = (int)this->res.size() - 1;

    if (r == -1)
        return -1;

    while (l < r)
    {
        int m = (l + r) / 2;
        if (compareFunc(this->res[m], p, b, q) >= 0)		r = m;
        else l = m + 1;
    }
    if (compareFunc(this->res[l], p, b, q) == 0)		return l;
    else return -1;
}

int GeneralEvaluation::TempResult::findRightBounder(vector<int> &p, int *b, vector<int> &q)
{
    int l = 0, r = (int)this->res.size() - 1;

    if (r == -1)
        return -1;

    while (l < r)
    {
        int m = (l + r) / 2 + 1;
        if (compareFunc(this->res[m], p, b, q) <= 0)		l = m;
        else r = m - 1;
    }
    if (compareFunc(this->res[l], p, b, q) == 0)		return l;
    else return -1;
}

void GeneralEvaluation::TempResult::doJoin(TempResult &x, TempResult &r)
{
    if ((int)r.var.varset.size() == 0)		return;

    vector <int> this2r = this->var.mapTo(r.var);
    vector <int> x2r = x.var.mapTo(r.var);

    Varset common_var = this->var * x.var;
    int r_varnum = (int)r.var.varset.size();
    int this_varnum = (int)this->var.varset.size();
    int x_varnum = (int)x.var.varset.size();
    if ((int)common_var.varset.size() == 0)
    {
        for (int i = 0; i < (int)this->res.size(); i++)
            for (int j = 0; j < (int)x.res.size(); j++)
            {
                int* a = new int [r_varnum];
                for (int k = 0; k < this_varnum; k++)
                    a[this2r[k]] = this->res[i][k];
                for (int k = 0; k < x_varnum; k++)
                    a[x2r[k]] = x.res[j][k];
                r.res.push_back(a);
            }
    }
    else if ((int)x.res.size() > 0)
    {
        vector<int> common2x = common_var.mapTo(x.var);
        x.sort(0, (int)x.res.size() - 1, common2x);

        vector<int> common2this = common_var.mapTo(this->var);
        for (int i = 0; i < (int)this->res.size(); i++)
        {
            int left, right;
            left = x.findLeftBounder(common2x, this->res[i], common2this);
            if (left == -1)	continue;
            right = x.findRightBounder(common2x, this->res[i], common2this);

            for (int j = left; j <= right; j++)
            {
                int* a = new int [r_varnum];
                for (int k = 0; k < this_varnum; k++)
                    a[this2r[k]] = this->res[i][k];
                for (int k = 0; k < x_varnum; k++)
                    a[x2r[k]] = x.res[j][k];
                r.res.push_back(a);
            }
        }
    }
}

void GeneralEvaluation::TempResult::doOptional(vector<bool> &binding, TempResult &x, TempResult &rn, TempResult &ra, bool add_no_binding)
{
    vector <int> this2rn = this->var.mapTo(rn.var);

    Varset common_var = this->var * x.var;
    if ((int)common_var.varset.size() != 0 && (int)x.res.size() != 0)
    {
        vector <int> this2ra = this->var.mapTo(ra.var);
        vector <int> x2ra = x.var.mapTo(ra.var);

        vector<int> common2x = common_var.mapTo(x.var);
        x.sort(0, (int)x.res.size() - 1, common2x);

        vector<int> common2this = common_var.mapTo(this->var);
        int ra_varnum = (int)ra.var.varset.size();
        int this_varnum = (int)this->var.varset.size();
        int x_varnum = (int)x.var.varset.size();
        for (int i = 0; i < (int)this->res.size(); i++)
        {
            int left, right;
            left = x.findLeftBounder(common2x, this->res[i], common2this);
            if (left != -1)
            {
                binding[i] = true;
                right = x.findRightBounder(common2x, this->res[i], common2this);
                for (int j = left; j <= right; j++)
                {
                    int* a = new int [ra_varnum];
                    for (int k = 0; k < this_varnum; k++)
                        a[this2ra[k]] = this->res[i][k];
                    for (int k = 0; k < x_varnum; k++)
                        a[x2ra[k]] = x.res[j][k];
                    ra.res.push_back(a);
                }
            }
        }
    }

    if (add_no_binding)
    {
        int rn_varnum = (int)rn.var.varset.size();
        int this_varnum = (int)this->var.varset.size();
        for (int i = 0; i < (int)this->res.size(); i++)
            if (!binding[i])
            {
                int* a = new int [rn_varnum];
                for (int j = 0; j < this_varnum; j++)
                    a[this2rn[j]] = this->res[i][j];
                rn.res.push_back(a);
            }
    }
}

void GeneralEvaluation::TempResult::doUnion(TempResult &x, TempResult &rt, TempResult &rx)
{
    vector <int> this2rt = this->var.mapTo(rt.var);
    vector <int> x2rx = x.var.mapTo(rx.var);

    int rt_varnum = (int)rt.var.varset.size();
    int this_varnum = (int)this->var.varset.size();
    for (int i = 0; i < (int)this->res.size(); i++)
    {
        int* a = new int [rt_varnum];
        for (int j = 0; j < this_varnum; j++)
            a[this2rt[j]] = this->res[i][j];
        rt.res.push_back(a);
    }

    int rx_varnum = (int)rx.var.varset.size();
    int x_varnum = (int)x.var.varset.size();
    for (int i = 0; i < (int)x.res.size(); i++)
    {
        int* a = new int[rx_varnum];
        for (int j = 0; j < x_varnum; j++)
            a[x2rx[j]] = x.res[i][j];
        rx.res.push_back(a);
    }
}

void GeneralEvaluation::TempResult::doMinus(TempResult &x, TempResult &r)
{
    vector <int> this2r = this->var.mapTo(r.var);

    Varset common_var = this->var * x.var;
    int r_varnum = (int)r.var.varset.size();
    int this_varnum = (int)this->var.varset.size();
    if ((int)common_var.varset.size() == 0)
    {
        for (int i = 0; i < (int)this->res.size(); i++)
        {
            int* a = new int [r_varnum];
            for (int j = 0; j < this_varnum; j++)
                a[this2r[j]] = this->res[i][j];
            r.res.push_back(a);
        }
    }
    else if ((int)x.res.size() > 0)
    {
        vector<int> common2x = common_var.mapTo(x.var);
        x.sort(0, (int)x.res.size() - 1, common2x);

        vector<int> common2this = common_var.mapTo(this->var);
        for (int i = 0; i < (int)this->res.size(); i++)
        {
            int left = x.findLeftBounder(common2x, this->res[i], common2this);
            if (left == -1)
            {
                int* a = new int [r_varnum];
                for (int j = 0; j < this_varnum; j++)
                    a[this2r[j]] = this->res[i][j];
                r.res.push_back(a);
            }
        }
    }
}

void GeneralEvaluation::TempResult::doDistinct(TempResult &r)
{
    vector <int> this2r = this->var.mapTo(r.var);

    if ((int)this->res.size() > 0)
    {
        vector<int> r2this = r.var.mapTo(this->var);
        this->sort(0, (int)this->res.size() - 1, r2this);

        int r_varnum = (int)r.var.varset.size();
        int this_varnum = (int)this->var.varset.size();
        for (int i = 0; i < (int)this->res.size(); i++)
            if (i == 0 || (i > 0 && compareFunc(this->res[i], r2this, this->res[i - 1], r2this) != 0))
            {
                int* a = new int [r_varnum];
                for (int j = 0; j < this_varnum; j++)
                    if (this2r[j] != -1)
                        a[this2r[j]] = this->res[i][j];
                r.res.push_back(a);
            }
    }
}



void GeneralEvaluation::TempResult::mapFilterTree2Varset(QueryTree::GroupPattern::FilterTreeNode& filter, Varset &v)
{
    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::Not_type)
    {
        mapFilterTree2Varset(filter.child[0].node, v);
    }
    else if (filter.type == QueryTree::GroupPattern::FilterTreeNode::Or_type || filter.type == QueryTree::GroupPattern::FilterTreeNode::And_type)
    {
        mapFilterTree2Varset(filter.child[0].node, v);
        mapFilterTree2Varset(filter.child[1].node, v);
    }
    else if (QueryTree::GroupPattern::FilterTreeNode::Equal_type <= filter.type && filter.type <= QueryTree::GroupPattern::FilterTreeNode::GreaterOrEqual_type)
    {
        if (filter.child[0].type == 't')
            mapFilterTree2Varset(filter.child[0].node, v);
        else if (filter.child[0].type == 's' && filter.child[0].arg[0] == '?')
            filter.child[0].pos = Varset(filter.child[0].arg).mapTo(v)[0];

        if (filter.child[1].type == 't')
            mapFilterTree2Varset(filter.child[1].node, v);
        else if (filter.child[1].type == 's' && filter.child[1].arg[0] == '?')
            filter.child[1].pos = Varset(filter.child[1].arg).mapTo(v)[0];
    }
    else if (filter.type == QueryTree::GroupPattern::FilterTreeNode::Builtin_regex_type || filter.type == QueryTree::GroupPattern::FilterTreeNode::Builtin_lang_type || filter.type == QueryTree::GroupPattern::FilterTreeNode::Builtin_langmatches_type || filter.type == QueryTree::GroupPattern::FilterTreeNode::Builtin_bound_type || filter.type == QueryTree::GroupPattern::FilterTreeNode::Builtin_in_type)
    {
        if (filter.child[0].type == 't')
            mapFilterTree2Varset(filter.child[0].node, v);
        else if (filter.child[0].type == 's' && filter.child[0].arg[0] == '?')
            filter.child[0].pos = Varset(filter.child[0].arg).mapTo(v)[0];
    }
}

void GeneralEvaluation::TempResult::doFilter(QueryTree::GroupPattern::FilterTreeNode& filter, FilterExistsGroupPatternResultSetRecord &filter_exists_grouppattern_resultset_record, TempResult &r, KVstore *kvstore)
{
    mapFilterTree2Varset(filter, this->var);

    r.var = this->var;

    int varnum = (int)this->var.varset.size();
    for (int i = 0; i < (int)this->res.size(); i++)
    {
        if (matchFilterTree(this->res[i], filter, filter_exists_grouppattern_resultset_record, kvstore))
        {
            int *a = new int[varnum];
            memcpy(a, this->res[i], sizeof(int) * varnum);
            r.res.push_back(a);
        }
    }
}

void GeneralEvaluation::TempResult::getFilterString(int* x, QueryTree::GroupPattern::FilterTreeNode::FilterTreeChild &child, string &str, KVstore *kvstore)
{
    if (child.type == 's')
    {
        if (child.arg[0] == '?')
        {
            int id = -1;
            if (child.pos != -1)	id = x[child.pos];
            if (id == -1)
                str = "";
            else if (id < Util::LITERAL_FIRST_ID)
                str = kvstore->getEntityByID(id);
            else
                str = kvstore->getLiteralByID(id);
        }
        else str = child.arg;

        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!support string only
        if (str[0]  == '\'')
        {
            str[0] = '"';
            str[str.rfind('\'')] = '"';
        }
    }
    else
    {
        getFilterString(x, child.node.child[0], str, kvstore);
        if (child.node.type == QueryTree::GroupPattern::FilterTreeNode::Builtin_lang_type)
        {
            int p = str.rfind('@');
            if (p != -1)
                str = "\"" + str.substr(p+1) + "\"";
            else
                str = "";
        }
    }
}

bool GeneralEvaluation::TempResult::matchFilterTree(int* x, QueryTree::GroupPattern::FilterTreeNode& filter, FilterExistsGroupPatternResultSetRecord &filter_exists_grouppattern_resultset_record, KVstore *kvstore)
{
    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::Not_type)
    {
        return !matchFilterTree(x, filter.child[0].node, filter_exists_grouppattern_resultset_record, kvstore);
    }
    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::Or_type)
    {
        if (matchFilterTree(x, filter.child[0].node, filter_exists_grouppattern_resultset_record, kvstore))			return true;
        else if (matchFilterTree(x, filter.child[1].node, filter_exists_grouppattern_resultset_record, kvstore))		return true;
        else return false;
    }
    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::And_type)
    {
        if (!matchFilterTree(x, filter.child[0].node, filter_exists_grouppattern_resultset_record, kvstore))			return false;
        else if (!matchFilterTree(x, filter.child[1].node, filter_exists_grouppattern_resultset_record, kvstore))		return false;
        return true;
    }
    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::Equal_type)
    {
        string a, b;
        getFilterString(x, filter.child[0], a, kvstore);
        getFilterString(x, filter.child[1], b, kvstore);
        return a == b;
    }
    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::NotEqual_type)
    {
        string a, b;
        getFilterString(x, filter.child[0], a, kvstore);
        getFilterString(x, filter.child[1], b, kvstore);
        return a != b;
    }
    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::Less_type)
    {
        string a, b;
        getFilterString(x, filter.child[0], a, kvstore);
        getFilterString(x, filter.child[1], b, kvstore);
        return a < b;
    }
    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::LessOrEqual_type)
    {
        string a, b;
        getFilterString(x, filter.child[0], a, kvstore);
        getFilterString(x, filter.child[1], b, kvstore);
        return a <= b;
    }
    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::Greater_type)
    {
        string a, b;
        getFilterString(x, filter.child[0], a, kvstore);
        getFilterString(x, filter.child[1], b, kvstore);
        return a > b;
    }
    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::GreaterOrEqual_type)
    {
        string a, b;
        getFilterString(x, filter.child[0], a, kvstore);
        getFilterString(x, filter.child[1], b, kvstore);
        return a >= b;
    }

    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::Builtin_regex_type)
    {
        string t, p, f;
        getFilterString(x, filter.child[0], t, kvstore);
        t = t.substr(1, t.rfind('"') - 1);
        getFilterString(x, filter.child[1], p, kvstore);
        p = p.substr(1, p.rfind('"') - 1);
        if ((int)filter.child.size() >= 3 && filter.child[2].type == 's')
        {
            getFilterString(x, filter.child[2], f, kvstore);
            f = f.substr(1, f.rfind('"') - 1);
        }

        RegexExpression re;

        if (!re.compile(p, f))	return false;
        return re.match(t);
    }

    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::Builtin_langmatches_type)
    {
        string a, b;
        getFilterString(x, filter.child[0], a, kvstore);
        getFilterString(x, filter.child[1], b, kvstore);
        return a == b || (a.length() > 2 && b == "\"*\"");
    }

    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::Builtin_bound_type)
    {
        return filter.child[0].pos != -1;
    }

    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::Builtin_in_type)
    {
        string a;
        getFilterString(x, filter.child[0], a, kvstore);

        for (int i = 1; i < (int)filter.child.size(); i++)
            if (filter.child[i].type == 's' && a == filter.child[i].arg)
                return true;
        return false;
    }

    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::Builtin_exists_type)
    {
        int id = filter.exists_grouppattern_id;
        for (int i = 0; i < (int)filter_exists_grouppattern_resultset_record.resultset[id]->results.size(); i++)
        {
            if (((int)filter_exists_grouppattern_resultset_record.resultset[id]->results[i].res.size() > 0) &&
                    ((int)filter_exists_grouppattern_resultset_record.common[id][i].varset.size() == 0 ||
                     filter_exists_grouppattern_resultset_record.resultset[id]->results[i].findLeftBounder(
                         filter_exists_grouppattern_resultset_record.common2resultset[id][i].first, x,
                         filter_exists_grouppattern_resultset_record.common2resultset[id][i].second) != -1))
                return true;
        }
        return false;
    }

    return false;
}



void GeneralEvaluation::TempResult::print()
{
    int varnum = (int)this->var.varset.size();

    this->var.print();

    printf("temp result:\n");
    for (int i = 0; i < (int)this->res.size(); i++)
    {
        for (int j = 0; j < varnum; j++)
            printf("%d ", this->res[i][j]);
        printf("\n");
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void GeneralEvaluation::TempResultSet::release()
{
    for (int i = 0; i < (int)this->results.size(); i++)
        results[i].release();
}

int GeneralEvaluation::TempResultSet::findCompatibleResult(Varset &_varset)
{
    for (int i = 0; i < (int)this->results.size(); i++)
        if (results[i].var == _varset)
            return i;

    int p = (int)this->results.size();
    this->results.push_back(TempResult());
    this->results[p].var = _varset;
    return p;
}


void GeneralEvaluation::TempResultSet::doJoin(TempResultSet &x, TempResultSet &r)
{
    for (int i = 0; i < (int)this->results.size(); i++)
        for (int j = 0; j < (int)x.results.size(); j++)
        {
            Varset var_r = this->results[i].var + x.results[j].var;
            int pos_r = r.findCompatibleResult(var_r);
            this->results[i].doJoin(x.results[j], r.results[pos_r]);
        }
}

void GeneralEvaluation::TempResultSet::doOptional(TempResultSet &x, TempResultSet &r)
{
    for (int i = 0; i < (int)this->results.size(); i++)
    {
        vector <bool> binding((int)this->results[i].res.size(), false);
        for (int j = 0; j < (int)x.results.size(); j++)
        {
            Varset &var_rn = this->results[i].var;
            Varset var_ra = this->results[i].var + x.results[j].var;
            int pos_rn = r.findCompatibleResult(var_rn);
            int pos_ra = r.findCompatibleResult(var_ra);
            this->results[i].doOptional(binding, x.results[j], r.results[pos_rn], r.results[pos_ra], j + 1 == (int)x.results.size());
        }
    }
}

void GeneralEvaluation::TempResultSet::doUnion(TempResultSet &x, TempResultSet &r)
{
    TempResult empty;

    for (int i = 0; i < (int)this->results.size(); i++)
    {
        Varset &var_r = this->results[i].var;
        int pos = r.findCompatibleResult(var_r);
        this->results[i].doUnion(empty, r.results[pos], empty);
    }

    for (int i = 0; i < (int)x.results.size(); i++)
    {
        Varset &var_r = x.results[i].var;
        int pos = r.findCompatibleResult(var_r);
        x.results[i].doUnion(empty, r.results[pos], empty);
    }
}

void GeneralEvaluation::TempResultSet::doMinus(TempResultSet &x, TempResultSet &r)
{
    for (int i = 0; i < (int)this->results.size(); i++)
    {
        vector <TempResult> tr;
        for (int j = 0; j < (int)x.results.size(); j++)
        {
            if (j == 0 && j + 1 == (int)x.results.size())
            {
                int pos_r = r.findCompatibleResult(this->results[i].var);
                this->results[i].doMinus(x.results[j], r.results[pos_r]);
            }
            else if (j == 0)
            {
                tr.push_back(TempResult());
                tr[0].var = this->results[i].var;
                this->results[i].doMinus(x.results[j], tr[0]);
            }
            else if (j + 1 == (int)x.results.size())
            {
                int pos_r = r.findCompatibleResult(this->results[i].var);
                tr[j - 1].doMinus(x.results[j], r.results[pos_r]);
            }
            else
            {
                tr.push_back(TempResult());
                tr[j].var = this->results[i].var;
                tr[j - 1].doMinus(x.results[j], tr[j]);
            }
        }
    }
}

void GeneralEvaluation::TempResultSet::doDistinct(Varset &projection, TempResultSet &r)
{
    TempResultSet* temp = new TempResultSet();
    temp->findCompatibleResult(projection);
    r.findCompatibleResult(projection);

    int projection_varnum = (int)projection.varset.size();
    for (int i = 0; i < (int)this->results.size(); i++)
    {
        vector<int> projection2this = projection.mapTo(this->results[i].var);
        for (int j = 0; j < (int)this->results[i].res.size(); j++)
        {
            int* a = new int[projection_varnum];
            for (int k = 0; k < projection_varnum; k++)
                if (projection2this[k] == -1)	a[k] = -1;
                else	a[k] = this->results[i].res[j][projection2this[k]];
            temp->results[0].res.push_back(a);
        }
    }

    temp->results[0].doDistinct(r.results[0]);

    temp->release();
    delete temp;
}

void GeneralEvaluation::TempResultSet::doFilter(QueryTree::GroupPattern::FilterTreeNode& filter, FilterExistsGroupPatternResultSetRecord &filter_exists_grouppattern_resultset_record, TempResultSet &r, KVstore *kvstore)
{
    for (int i = 0; i < (int)this->results.size(); i++)
    {
        filter_exists_grouppattern_resultset_record.common.clear();
        filter_exists_grouppattern_resultset_record.common2resultset.clear();

        Varset &var_r = this->results[i].var;

        for (int j = 0; j < (int)filter_exists_grouppattern_resultset_record.resultset.size(); j++)
        {
            filter_exists_grouppattern_resultset_record.common.push_back(vector<Varset>());
            filter_exists_grouppattern_resultset_record.common2resultset.push_back(vector< pair< vector<int>, vector<int> > >());

            for (int k = 0; k < (int)filter_exists_grouppattern_resultset_record.resultset[j]->results.size(); k++)
            {
                TempResult &filter_result = filter_exists_grouppattern_resultset_record.resultset[j]->results[k];

                Varset common = var_r * filter_result.var;
                filter_exists_grouppattern_resultset_record.common[j].push_back(common);
                vector<int> common2filter_result = common.mapTo(filter_result.var);
                filter_exists_grouppattern_resultset_record.common2resultset[j].push_back(make_pair(common2filter_result, common.mapTo(var_r)));

                if (common.varset.size() > 0 && filter_result.res.size() > 0)
                    filter_result.sort(0, filter_result.res.size() - 1, common2filter_result);
            }
        }

        int pos_r = r.findCompatibleResult(var_r);
        this->results[i].doFilter(filter, filter_exists_grouppattern_resultset_record, r.results[pos_r], kvstore);
    }
}


void GeneralEvaluation::TempResultSet::print()
{
    printf("total temp result : %d\n", (int)this->results.size());
    for (int i = 0; i < (int)this->results.size(); i++)
    {
        printf("temp result no.%d\n", i);
        this->results[i].print();
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void GeneralEvaluation::generateEvaluationPlan(QueryTree::GroupPattern &grouppattern)
{
    if ((int)grouppattern.patterns.size() == 0 && (int)grouppattern.unions.size() == 0 && (int)grouppattern.optionals.size() == 0)
    {
        TempResultSet* temp = new TempResultSet();
        temp->results.push_back(TempResult());
        this->semantic_evaluation_plan.push_back(EvaluationUnit('r', temp));

        return;
    }

    int current_pattern = 0, current_unions = 0;
    Varset current_result_varset;

    grouppattern.addOneOptionalOrMinus(' ');	//for convenience

    for (int i = 0; i < (int)grouppattern.optionals.size(); i++)
    {
        if (current_pattern <= grouppattern.optionals[i].lastpattern || current_unions <= grouppattern.optionals[i].lastunions)
        {
            vector <pair<char, int> > node_info;
            vector <Varset> node_varset;
            vector <vector<int> > edge;

            if ((int)current_result_varset.varset.size() > 0)
            {
                node_info.push_back(make_pair('r', -1));
                node_varset.push_back(current_result_varset);
            }

            while (current_pattern <= grouppattern.optionals[i].lastpattern)
            {
                int current_blockid = grouppattern.pattern_blockid[current_pattern];
                if (current_blockid != -1)
                {
                    bool found = false;
                    for (int i = 0; i < (int)node_info.size(); i++)
                        if (node_info[i].second == current_blockid)
                        {
                            found = true;
                            break;
                        }
                    if (!found)
                    {
                        node_info.push_back(make_pair('b', current_blockid));
                        node_varset.push_back(this->sparql_query_varset[current_blockid]);
                    }
                }
                current_pattern ++;
            }

            while (current_unions <= grouppattern.optionals[i].lastunions)
            {
                node_info.push_back(make_pair('u', current_unions));
                Varset varset = grouppattern.unions[current_unions].grouppattern_vec[0].grouppattern_resultset_minimal_varset;
                for (int j = 1; j < (int)grouppattern.unions[current_unions].grouppattern_vec.size(); j++)
                    varset = varset * grouppattern.unions[current_unions].grouppattern_vec[j].grouppattern_resultset_minimal_varset;
                node_varset.push_back(varset);
                current_unions ++;
            }

            for (int i = 0; i < (int)node_varset.size(); i++)
            {
                edge.push_back(vector<int>());
                for (int j = 0; j < (int)node_varset.size(); j++)
                    if (i != j && node_varset[i].hasCommonVar(node_varset[j]))
                        edge[i].push_back(j);
            }

            for (int i = 0; i < (int)node_info.size(); i++)
                if (node_info[i].first != 'v')					//visited
                {
                    dfsJoinableResultGraph(i, node_info, edge, grouppattern);
                    if (i != 0)
                        this->semantic_evaluation_plan.push_back(EvaluationUnit('j'));
                }

            for (int i = 0; i < (int)node_varset.size(); i++)
                current_result_varset = current_result_varset + node_varset[i];
        }

        if (i + 1 != (int)grouppattern.optionals.size())
        {
            generateEvaluationPlan(grouppattern.optionals[i].grouppattern);
            if (i != 0 || grouppattern.optionals[i].lastpattern != -1 || grouppattern.optionals[i].lastunions != -1)
                this->semantic_evaluation_plan.push_back(EvaluationUnit(grouppattern.optionals[i].type));
        }
    }

    grouppattern.optionals.pop_back();

    for (int i = 0; i < (int)grouppattern.filters.size(); i++)
    {
        for (int j = 0; j < (int)grouppattern.filter_exists_grouppatterns[i].size(); j++)
            generateEvaluationPlan(grouppattern.filter_exists_grouppatterns[i][j]);

        this->semantic_evaluation_plan.push_back(EvaluationUnit('f', &grouppattern.filters[i]));
    }
}


void GeneralEvaluation::dfsJoinableResultGraph(int x, vector < pair<char, int> > &node_info, vector < vector<int> > &edge, QueryTree::GroupPattern &grouppattern)
{
    if (node_info[x].first == 'b')
    {
        int blockid = node_info[x].second;

        TempResultSet* temp = new TempResultSet();
        temp->results.push_back(TempResult());
        temp->results[0].var = this->sparql_query_varset[blockid];

        int varnum = (int)temp->results[0].var.varset.size();

        vector<int*>& basicquery_result =this->sparql_query.getBasicQuery(blockid).getResultList();
        int basicquery_result_num = (int)basicquery_result.size();

        temp->results[0].res.reserve(basicquery_result_num);
        for (int i = 0; i < basicquery_result_num; i++)
        {
            int* result_vec = new int [varnum];
            memcpy(result_vec, basicquery_result[i], sizeof(int) * varnum);
            temp->results[0].res.push_back(result_vec);
        }
        this->semantic_evaluation_plan.push_back(EvaluationUnit('r', temp));
    }
    if (node_info[x].first == 'u')
    {
        int unionsid = node_info[x].second;
        for (int i = 0; i < (int)grouppattern.unions[unionsid].grouppattern_vec.size(); i++)
        {
            generateEvaluationPlan(grouppattern.unions[unionsid].grouppattern_vec[i]);
            if (i > 0)	this->semantic_evaluation_plan.push_back(EvaluationUnit('u'));
        }
    }

    node_info[x].first = 'v';

    for (int i = 0; i < (int)edge[x].size(); i++)
    {
        int y = edge[x][i];
        if (node_info[y].first != 'v')
        {
            dfsJoinableResultGraph(y, node_info, edge, grouppattern);
            this->semantic_evaluation_plan.push_back(EvaluationUnit('j'));
        }
    }
}

int GeneralEvaluation::countFilterExistsGroupPattern(QueryTree::GroupPattern::FilterTreeNode& filter)
{
    int count = 0;
    if (filter.type == QueryTree::GroupPattern::FilterTreeNode::Builtin_exists_type)
        count = 1;
    for (int i = 0; i < (int)filter.child.size(); i++)
        if (filter.child[i].type == 't')
            count += countFilterExistsGroupPattern(filter.child[i].node);
    return count;
}

void GeneralEvaluation::doEvaluationPlan()
{
    for (int i = 0; i < (int)this->semantic_evaluation_plan.size(); i++)
    {
        if (semantic_evaluation_plan[i].getType() == 'r')
            this->semantic_evaluation_result_stack.push((TempResultSet*)semantic_evaluation_plan[i].getPointer());
        if (semantic_evaluation_plan[i].getType() == 'j' || semantic_evaluation_plan[i].getType() == 'o' || semantic_evaluation_plan[i].getType() == 'm' || semantic_evaluation_plan[i].getType() == 'u')
        {
            TempResultSet* b = semantic_evaluation_result_stack.top();
            semantic_evaluation_result_stack.pop();
            TempResultSet* a = semantic_evaluation_result_stack.top();
            semantic_evaluation_result_stack.pop();
            TempResultSet* r = new TempResultSet();

            if (semantic_evaluation_plan[i].getType() == 'j')
                a->doJoin(*b, *r);
            if (semantic_evaluation_plan[i].getType() == 'o')
                a->doOptional(*b, *r);
            if (semantic_evaluation_plan[i].getType() == 'm')
                a->doMinus(*b, *r);
            if (semantic_evaluation_plan[i].getType() == 'u')
                a->doUnion(*b, *r);

            a->release();
            b->release();
            delete a;
            delete b;

            semantic_evaluation_result_stack.push(r);
        }

        if (semantic_evaluation_plan[i].getType() == 'f')
        {
            int filter_exists_grouppattern_size = countFilterExistsGroupPattern(*(QueryTree::GroupPattern::FilterTreeNode *)semantic_evaluation_plan[i].getPointer());

            if (filter_exists_grouppattern_size > 0)
                for (int i = 0; i < filter_exists_grouppattern_size; i++)
                {
                    this->filter_exists_grouppattern_resultset_record.resultset.push_back(semantic_evaluation_result_stack.top());
                    semantic_evaluation_result_stack.pop();
                }

            TempResultSet* a = semantic_evaluation_result_stack.top();
            semantic_evaluation_result_stack.pop();

            TempResultSet * r = new TempResultSet();

            a->doFilter(*(QueryTree::GroupPattern::FilterTreeNode *)semantic_evaluation_plan[i].getPointer(), this->filter_exists_grouppattern_resultset_record, *r, this->kvstore);

            if (filter_exists_grouppattern_size > 0)
            {
                for (int i = 0; i < filter_exists_grouppattern_size; i++)
                    this->filter_exists_grouppattern_resultset_record.resultset[i]->release();
                this->filter_exists_grouppattern_resultset_record.resultset.clear();
            }

            a->release();
            delete a;

            semantic_evaluation_result_stack.push(r);
        }
    }
}

bool GeneralEvaluation::expanseFirstOuterUnionGroupPattern(QueryTree::GroupPattern &grouppattern, deque<QueryTree::GroupPattern> &queue)
{
    if (grouppattern.unions.size() > 0)
    {
        QueryTree::GroupPattern copy = grouppattern;

        for (int union_id = 0; union_id < (int)copy.unions[0].grouppattern_vec.size(); union_id++)
        {
            grouppattern = QueryTree::GroupPattern();

            int lastpattern = -1, lastunions = -1, lastoptional = -1;
            while (lastpattern + 1 < (int)copy.patterns.size() || lastunions + 1 < (int)copy.unions.size() || lastoptional + 1 < (int)copy.optionals.size())
            {
                if (lastoptional + 1 < (int)copy.optionals.size() && copy.optionals[lastoptional + 1].lastpattern == lastpattern && copy.optionals[lastoptional + 1].lastunions == lastunions)
                    //optional
                {
                    grouppattern.addOneOptionalOrMinus('o');
                    grouppattern.getLastOptionalOrMinus() = copy.optionals[lastoptional + 1].grouppattern;
                    lastoptional++;
                }
                else if (lastunions + 1 < (int)copy.unions.size() && copy.unions[lastunions + 1].lastpattern == lastpattern)
                    //union
                {
                    //first union
                    if (lastunions == -1)
                    {
                        QueryTree::GroupPattern &inner_grouppattern = copy.unions[0].grouppattern_vec[union_id];

                        int inner_lastpattern = -1, inner_lastunions = -1, inner_lastoptional = -1;
                        while (inner_lastpattern + 1 < (int)inner_grouppattern.patterns.size() || inner_lastunions + 1 < (int)inner_grouppattern.unions.size() || inner_lastoptional + 1 < (int)inner_grouppattern.optionals.size())
                        {
                            if (inner_lastoptional + 1 < (int)inner_grouppattern.optionals.size() && inner_grouppattern.optionals[inner_lastoptional + 1].lastpattern == inner_lastpattern && inner_grouppattern.optionals[inner_lastoptional + 1].lastunions == inner_lastunions)
                                //inner optional
                            {
                                grouppattern.addOneOptionalOrMinus('o');
                                grouppattern.getLastOptionalOrMinus() = inner_grouppattern.optionals[inner_lastoptional + 1].grouppattern;
                                inner_lastoptional++;
                            }
                            else if (inner_lastunions + 1 < (int)inner_grouppattern.unions.size() && inner_grouppattern.unions[inner_lastunions + 1].lastpattern == inner_lastpattern)
                                //inner union
                            {
                                grouppattern.addOneGroupUnion();
                                for (int i = 0; i < (int)inner_grouppattern.unions[inner_lastunions + 1].grouppattern_vec.size(); i++)
                                {
                                    grouppattern.addOneUnion();
                                    grouppattern.getLastUnion() = inner_grouppattern.unions[inner_lastunions + 1].grouppattern_vec[i];
                                }
                                inner_lastunions++;
                            }
                            else
                                //inner triple pattern
                            {
                                grouppattern.patterns.push_back(inner_grouppattern.patterns[inner_lastpattern + 1]);
                                inner_lastpattern++;
                            }
                        }
                        //inner filter
                        for (int i = 0; i < (int)inner_grouppattern.filters.size(); i++)
                        {
                            grouppattern.addOneFilterTree();
                            grouppattern.getLastFilterTree() = inner_grouppattern.filters[i].root;
                        }
                    }
                    else
                    {
                        grouppattern.addOneGroupUnion();
                        for (int i = 0; i < (int)copy.unions[lastunions + 1].grouppattern_vec.size(); i++)
                        {
                            grouppattern.addOneUnion();
                            grouppattern.getLastUnion() = copy.unions[lastunions + 1].grouppattern_vec[i];
                        }
                    }
                    lastunions++;
                }
                else
                    //triple pattern
                {
                    grouppattern.patterns.push_back(copy.patterns[lastpattern + 1]);
                    lastpattern++;
                }
            }
            //filter
            for (int i = 0; i < (int)copy.filters.size(); i++)
            {
                grouppattern.addOneFilterTree();
                grouppattern.getLastFilterTree() = copy.filters[i].root;
            }

            queue.push_back(grouppattern);
        }

        grouppattern = copy;
        return true;
    }

    return false;
}

void GeneralEvaluation::queryRewriteEncodeRetrieveJoin(int dep, ResultFilter &result_filter)
{
    deque<QueryTree::GroupPattern> queue;
    queue.push_back(this->expansion_evaluation_stack[dep].grouppattern);
    vector<QueryTree::GroupPattern> cand;

    while(!queue.empty())
    {
        QueryTree::GroupPattern front = queue.front();
        queue.pop_front();

        if(!this->expanseFirstOuterUnionGroupPattern(front, queue))
            cand.push_back(front);
    }

    this->expansion_evaluation_stack[dep].result = new TempResultSet();
    for(int cand_id = 0; cand_id < (int)cand.size(); cand_id++)
    {
        this->expansion_evaluation_stack[dep].grouppattern = cand[cand_id];
        QueryTree::GroupPattern &grouppattern = this->expansion_evaluation_stack[dep].grouppattern;
        grouppattern.getVarset();

        for (int j = 0; j < 80; j++)			printf("=");
        printf("\n");
        grouppattern.print(dep);
        for (int j = 0; j < 80; j++)			printf("=");
        printf("\n");

        TempResultSet* temp = new TempResultSet();
        //get the result of grouppattern
        {
            this->expansion_evaluation_stack[dep].sparql_query = SPARQLquery();
            this->expansion_evaluation_stack[dep].sparql_query.addBasicQuery();

            Varset varset;
            for (int i = 0; i < (int)grouppattern.patterns.size(); i++)
            {
                this->expansion_evaluation_stack[dep].sparql_query.addTriple(Triple(
                            grouppattern.patterns[i].subject.value,
                            grouppattern.patterns[i].predicate.value,
                            grouppattern.patterns[i].object.value
                        ));
                varset = varset + grouppattern.patterns[i].varset;
            }

            for (int i = 0; i < dep; i++)
            {
                QueryTree::GroupPattern &upper_grouppattern = this->expansion_evaluation_stack[i].grouppattern;
                for (int j = 0; j < (int)upper_grouppattern.patterns.size(); j++)
                    if (varset.hasCommonVar(upper_grouppattern.patterns[j].varset))
                    {
                        this->expansion_evaluation_stack[dep].sparql_query.addTriple(Triple(
                                    upper_grouppattern.patterns[j].subject.value,
                                    upper_grouppattern.patterns[j].predicate.value,
                                    upper_grouppattern.patterns[j].object.value
                                ));
                    }
            }

            //reduce return result vars
            if (!this->query_tree.checkProjectionAsterisk())
            {
                Varset useful = this->query_tree.getProjection().varset;

                if (dep > 0)
                {
                    QueryTree::GroupPattern &upper_grouppattern = this->expansion_evaluation_stack[dep - 1].grouppattern;
                    for (int i = 0; i < (int)upper_grouppattern.patterns.size(); i++)
                        useful = useful + upper_grouppattern.patterns[i].varset;
                }

                for (int i = 0; i < (int)grouppattern.optionals.size(); i++)
                    useful = useful + grouppattern.optionals[i].grouppattern.grouppattern_resultset_maximal_varset;

                for (int i = 0; i < (int)grouppattern.filters.size(); i++)
                    useful = useful + grouppattern.filters[i].varset;

                varset = varset * useful;
            }

            printf("select vars : ");
            varset.print();

            this->expansion_evaluation_stack[dep].sparql_query.encodeQuery(this->kvstore, vector<vector<string> >(1, varset.varset));
            long tv_encode = Util::get_cur_time();

            this->vstree->retrieve(this->expansion_evaluation_stack[dep].sparql_query);
            long tv_retrieve = Util::get_cur_time();
            cout << "after Retrieve, used " << (tv_retrieve - tv_encode) << "ms." << endl;

            if (dep > 0)
                result_filter.candFilter(this->expansion_evaluation_stack[dep].sparql_query);

            Join *join = new Join(kvstore);
            join->join_sparql(this->expansion_evaluation_stack[dep].sparql_query);
            delete join;
            long tv_join = Util::get_cur_time();
            cout << "after Join, used " << (tv_join - tv_retrieve) << "ms." << endl;

            temp->results.push_back(TempResult());
            temp->results[0].var = varset;

            int varnum = (int)varset.varset.size();

            vector<int*>& basicquery_result = this->expansion_evaluation_stack[dep].sparql_query.getBasicQuery(0).getResultList();
            int basicquery_result_num = (int)basicquery_result.size();

            temp->results[0].res.reserve(basicquery_result_num);
            for (int i = 0; i < basicquery_result_num; i++)
            {
                int* result_vec = new int [varnum];
                memcpy(result_vec, basicquery_result[i], sizeof(int) * varnum);
                temp->results[0].res.push_back(result_vec);
            }
        }

        //don't use &grouppattern after there!
        if (temp->results[0].res.size() > 0 && grouppattern.optionals.size() > 0)
        {
            result_filter.change(this->expansion_evaluation_stack[dep].sparql_query, 1);

            for (int i = 0; i < (int)this->expansion_evaluation_stack[dep].grouppattern.optionals.size(); i++)
            {
                if ((int)this->expansion_evaluation_stack.size() > dep + 1)
                    this->expansion_evaluation_stack[dep + 1] = ExpansionEvaluationStackUnit();
                else
                    this->expansion_evaluation_stack.push_back(ExpansionEvaluationStackUnit());

                this->expansion_evaluation_stack[dep + 1].grouppattern = this->expansion_evaluation_stack[dep].grouppattern.optionals[i].grouppattern;
                queryRewriteEncodeRetrieveJoin(dep + 1, result_filter);

                TempResultSet *r = new TempResultSet();
                temp->doOptional(*this->expansion_evaluation_stack[dep + 1].result, *r);

                this->expansion_evaluation_stack[dep + 1].result->release();
                temp->release();
                delete this->expansion_evaluation_stack[dep + 1].result;
                delete temp;

                temp = r;
            }

            result_filter.change(this->expansion_evaluation_stack[dep].sparql_query, -1);
        }

        TempResultSet *r = new TempResultSet();
        this->expansion_evaluation_stack[dep].result->doUnion(*temp, *r);

        this->expansion_evaluation_stack[dep].result->release();
        temp->release();
        delete this->expansion_evaluation_stack[dep].result;
        delete temp;

        this->expansion_evaluation_stack[dep].result = r;

        for (int i = 0; i < (int)this->expansion_evaluation_stack[dep].grouppattern.filters.size(); i++)
        {
            TempResultSet *r = new TempResultSet();
            this->expansion_evaluation_stack[dep].result->doFilter(this->expansion_evaluation_stack[dep].grouppattern.filters[i].root, this->filter_exists_grouppattern_resultset_record, *r, this->kvstore);

            this->expansion_evaluation_stack[dep].result->release();
            delete this->expansion_evaluation_stack[dep].result;

            this->expansion_evaluation_stack[dep].result = r;
        }
    }
}

void GeneralEvaluation::getFinalResult(ResultSet& result_str)
{
    TempResultSet* results_id = this->semantic_evaluation_result_stack.top();
    this->semantic_evaluation_result_stack.pop();

    if (this->query_tree.getQueryForm() == QueryTree::Select_Query)
    {
        if (this->query_tree.getProjectionModifier() == QueryTree::Modifier_Distinct)
        {
            TempResultSet* results_id_distinct = new TempResultSet();

            results_id->doDistinct(this->query_tree.getProjection(), *results_id_distinct);

            results_id->release();
            delete results_id;

            results_id = results_id_distinct;
        }

        if (this->query_tree.checkProjectionAsterisk())
            for (int i = 0 ; i < (int)results_id->results.size(); i++)
            {
                this->query_tree.getProjection() = this->query_tree.getProjection() + results_id->results[i].var;
            }

        int var_num = this->query_tree.getProjectionNum();

        result_str.select_var_num = var_num;
        result_str.setVar(this->query_tree.getProjection().varset);

        for (int i = 0; i < (int)results_id->results.size(); i++)
            result_str.ansNum += (int)results_id->results[i].res.size();

#ifndef STREAM_ON
        result_str.answer = new string* [result_str.ansNum];
        for (int i = 0; i < result_str.ansNum; i++)
            result_str.answer[i] = NULL;
#else
        vector <int> keys;
        vector <bool> desc;
        for (int i = 0; i < (int)this->query_tree.getOrder().size(); i++)
        {
            int var_id = Varset(this->query_tree.getOrder()[i].var).mapTo(this->query_tree.getProjection())[0];
            if (var_id != -1)
            {
                keys.push_back(var_id);
                desc.push_back(this->query_tree.getOrder()[i].descending);
            }
        }
        result_str.openStream(keys, desc, this->query_tree.getOffset(), this->query_tree.getLimit());
#endif

        int current_result = 0;
        for (int i = 0; i < (int)results_id->results.size(); i++)
        {
            vector<int> result_str2id = this->query_tree.getProjection().mapTo(results_id->results[i].var);
            int size = results_id->results[i].res.size();
            for (int j = 0; j < size; ++j)
            {
#ifndef STREAM_ON
                result_str.answer[current_result] = new string[var_num];
#endif
                for (int v = 0; v < var_num; ++v)
                {
                    int ans_id = -1;
                    if (result_str2id[v] != -1)
                        ans_id =  results_id->results[i].res[j][result_str2id[v]];
                    string tmp_ans;
                    if (ans_id == -1)
                        tmp_ans = "";
                    else if (ans_id < Util::LITERAL_FIRST_ID)
                        tmp_ans = this->kvstore->getEntityByID(ans_id);
                    else
                        tmp_ans = this->kvstore->getLiteralByID(ans_id);
#ifndef STREAM_ON
                    result_str.answer[current_result][v] = tmp_ans;
#else
                    result_str.writeToStream(tmp_ans);
#endif
                }
                current_result++;
            }
        }
#ifdef STREAM_ON
        result_str.resetStream();
#endif
    }
    else if (this->query_tree.getQueryForm() == QueryTree::Ask_Query)
    {
        result_str.select_var_num = 1;
        result_str.ansNum = 1;
        string tmp_ans = "false";
#ifndef STREAM_ON
        result_str.answer = new string* [result_str.ansNum];
        result_str.answer[0] = new string[1];
#else
        vector <int> keys;
        vector <bool> desc;
        result_str.openStream(keys, desc, 0, -1);
#endif
        for (int i = 0; i < (int)results_id->results.size(); i++)
            if ((int)results_id->results[i].res.size() > 0)
                tmp_ans = "true";
#ifndef STREAM_ON
        result_str.answer[0][0] = tmp_ans;
#else
        result_str.writeToStream(tmp_ans);
        result_str.resetStream();
#endif
    }

    results_id->release();
    delete results_id;
}

