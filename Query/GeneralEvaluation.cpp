/*=============================================================================
# Filename: GeneralEvaluation.cpp
# Author: Jiaqi, Chen
# Mail: 1181955272@qq.com
# Last Modified: 2016-03-02 20:35
# Description: implement functions in GeneralEvaluation.h
=============================================================================*/

#include "GeneralEvaluation.h"

using namespace std;

QueryParser& GeneralEvaluation::getQueryParser()
{
	return this->_query_parser;
}

QueryTree& GeneralEvaluation::getQueryTree()
{
	return this->_query_tree;
}

SPARQLquery& GeneralEvaluation::getSPARQLQuery()
{
	return this->_sparql_query;
}


vector< vector< string > > GeneralEvaluation::getSPARQLQueryVarset()
{
	vector< vector< string > > res;
	for (int i = 0; i < (int)this->_sparql_query_varset.size(); i++)
		res.push_back(this->_sparql_query_varset[i].varset);
	return res;
}


bool GeneralEvaluation::parseQuery(const string &query)
{
	try
	{
		this->_query_parser.sparqlParser(query, this->_query_tree);
	}
	catch(const char* e)
	{
		cerr << e << endl;
		return false;
	}

	getVarset(this->_query_tree.getPatternGroup());
	getBasicQuery(this->_query_tree.getPatternGroup());

	return true;
}

void GeneralEvaluation::getVarset(QueryTree::PatternGroup &patterngroup)
{
	for (int i = 0; i < (int)patterngroup.patterns.size(); i++)
	{
		patterngroup.pattern_varset.push_back(Varset());
		if (patterngroup.patterns[i].subject.value[0] == '?')
			patterngroup.pattern_varset[i].addVar(patterngroup.patterns[i].subject.value);
		if (patterngroup.patterns[i].object.value[0] == '?')
			patterngroup.pattern_varset[i].addVar(patterngroup.patterns[i].object.value);
		patterngroup.patterngroup_varset = patterngroup.patterngroup_varset + patterngroup.pattern_varset[i];
	}

	for (int i = 0; i < (int)patterngroup.unions.size(); i++)
	{
		Varset r;
		for (int j = 0; j < (int)patterngroup.unions[i].size(); j++)
		{
			getVarset(patterngroup.unions[i][j]);
			if (j == 0)	r = r + patterngroup.unions[i][j].patterngroup_varset;
			else r = r * patterngroup.unions[i][j].patterngroup_varset;
		}
		patterngroup.patterngroup_varset = patterngroup.patterngroup_varset + r;
	}
	for (int i = 0; i < (int)patterngroup.optionals.size(); i++)
	{
		getVarset(patterngroup.optionals[i].patterngroup);
	}
	for(int i = 0; i < (int)patterngroup.filter_exists_patterngroups.size(); i++)
		for (int j = 0; j < (int)patterngroup.filter_exists_patterngroups[i].size(); j++)
		{
			getVarset(patterngroup.filter_exists_patterngroups[i][j]);
		}

	//patterngroup.patterngroup_varset.print();
}


void GeneralEvaluation::getBasicQuery(QueryTree::PatternGroup &patterngroup)
{
	for (int i = 0; i < (int)patterngroup.unions.size(); i++)
		for (int j = 0; j < (int)patterngroup.unions[i].size(); j++)
			getBasicQuery(patterngroup.unions[i][j]);
	for (int i = 0; i < (int)patterngroup.optionals.size(); i++)
		getBasicQuery(patterngroup.optionals[i].patterngroup);

	int current_optional = 0;
	int first_patternid = 0;

	patterngroup.initPatternBlockid();
	vector<int> basicqueryid((int)patterngroup.patterns.size(), 0);

	for (int i = 0; i < (int)patterngroup.patterns.size(); i++)
	{
		for (int j = first_patternid; j < i; j++)
		if ((int)(patterngroup.pattern_varset[j] * patterngroup.pattern_varset[i]).varset.size() > 0)
			patterngroup.mergePatternBlockid(j, i);

		if ((current_optional != (int)patterngroup.optionals.size() && i == patterngroup.optionals[current_optional].lastpattern) || i + 1 == (int)patterngroup.patterns.size())
		{
			for (int j = first_patternid; j <= i; j++)
				if ((int)patterngroup.pattern_varset[j].varset.size() > 0)
				{
					if (patterngroup.getRootPatternBlockid(j) == j)			//root node
					{
						this->_sparql_query.addBasicQuery();
						this->_sparql_query_varset.push_back(Varset());

						for (int k = first_patternid; k <= i; k++)
							if (patterngroup.getRootPatternBlockid(k) == j)
							{
								this->_sparql_query.addTriple(Triple(
									patterngroup.patterns[k].subject.value,
									patterngroup.patterns[k].predicate.value,
									patterngroup.patterns[k].object.value));

									basicqueryid[k] = this->_sparql_query.getBasicQueryNum() - 1;
									this->_sparql_query_varset[(int)this->_sparql_query_varset.size() - 1] = this->_sparql_query_varset[(int)this->_sparql_query_varset.size() - 1] + patterngroup.pattern_varset[k];
							}
					}
				}
				else	basicqueryid[j] = -1;

			for (int j = first_patternid; j <= i; j++)
				patterngroup.pattern_blockid[j] = basicqueryid[j];

			if (current_optional != (int)patterngroup.optionals.size())	current_optional++;
			first_patternid = i + 1;
		}
	}

	for(int i = 0; i < (int)patterngroup.filter_exists_patterngroups.size(); i++)
		for (int j = 0; j < (int)patterngroup.filter_exists_patterngroups[i].size(); j++)
			getBasicQuery(patterngroup.filter_exists_patterngroups[i][j]);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void GeneralEvaluation::TempResult::release()
{
	for (int i = 0; i < (int)this->res.size(); i++)
		delete[] res[i];
}

int GeneralEvaluation::TempResult::compareFunc(int *a, std::vector<int> &p, int *b, std::vector<int> &q)			//compare a[p] & b[q]
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

int GeneralEvaluation::TempResult::findLeftBounder(std::vector<int> &p, int *b, std::vector<int> &q)
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

int GeneralEvaluation::TempResult::findRightBounder(std::vector<int> &p, int *b, std::vector<int> &q)
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
	else
	if ((int)x.res.size() > 0)
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

void GeneralEvaluation::TempResult::doOptional(std::vector<bool> &binding, TempResult &x, TempResult &rn, TempResult &ra, bool add_no_binding)
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
	else
	if ((int)x.res.size() > 0)
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



void GeneralEvaluation::TempResult::mapFilterTree2Varset(QueryTree::FilterTree& filter, Varset &v)
{
	if (filter.type == QueryTree::FilterTree::Not_type)
	{
		mapFilterTree2Varset(*filter.child[0].ptr, v);
	}
	else if (filter.type == QueryTree::FilterTree::Or_type || filter.type == QueryTree::FilterTree::And_type)
	{
		mapFilterTree2Varset(*filter.child[0].ptr, v);
		mapFilterTree2Varset(*filter.child[1].ptr, v);
	}
	else if (QueryTree::FilterTree::Equal_type <= filter.type && filter.type <= QueryTree::FilterTree::GreaterOrEqual_type)
	{
		if (filter.child[0].type == 't')
			mapFilterTree2Varset(*filter.child[0].ptr, v);
		else if (filter.child[0].type == 's' && filter.child[0].arg[0] == '?')
			filter.child[0].pos = Varset(filter.child[0].arg).mapTo(v)[0];

		if (filter.child[1].type == 't')
			mapFilterTree2Varset(*filter.child[1].ptr, v);
		else if (filter.child[1].type == 's' && filter.child[1].arg[0] == '?')
			filter.child[1].pos = Varset(filter.child[1].arg).mapTo(v)[0];
	}
	else if (filter.type == QueryTree::FilterTree::Builtin_regex_type || filter.type == QueryTree::FilterTree::Builtin_lang_type || filter.type == QueryTree::FilterTree::Builtin_langmatches_type || filter.type == QueryTree::FilterTree::Builtin_bound_type || filter.type == QueryTree::FilterTree::Builtin_in_type)
	{
		if (filter.child[0].type == 't')
			mapFilterTree2Varset(*filter.child[0].ptr, v);
		else if (filter.child[0].type == 's' && filter.child[0].arg[0] == '?')
			filter.child[0].pos = Varset(filter.child[0].arg).mapTo(v)[0];
	}
}

void GeneralEvaluation::TempResult::doFilter(QueryTree::FilterTree& filter, FilterExistsPatternGroupResultSetRecord &filter_exists_patterngroup_resultset_record, TempResult &r, KVstore *kvstore)
{
	mapFilterTree2Varset(filter, this->var);

	r.var = this->var;

	int varnum = (int)this->var.varset.size();
	for (int i = 0; i < (int)this->res.size(); i++)
	{
		if (matchFilterTree(this->res[i], filter, filter_exists_patterngroup_resultset_record, kvstore))
		{
			int *a = new int[varnum];
			memcpy(a, this->res[i], sizeof(int) * varnum);
			r.res.push_back(a);
		}
	}
}

void GeneralEvaluation::TempResult::getFilterString(int* x, QueryTree::FilterTree::FilterTreeChild &child, string &str, KVstore *kvstore)
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
		getFilterString(x, child.ptr->child[0], str, kvstore);
		if (child.ptr->type == QueryTree::FilterTree::Builtin_lang_type)
		{
			int p = str.rfind('@');
			if (p != -1)
				str = "\"" + str.substr(p+1) + "\"";
			else
				str = "";
		}
	}
}

bool GeneralEvaluation::TempResult::matchFilterTree(int* x, QueryTree::FilterTree& filter, FilterExistsPatternGroupResultSetRecord &filter_exists_patterngroup_resultset_record, KVstore *kvstore)
{
	if (filter.type == QueryTree::FilterTree::Not_type)
	{
		return !matchFilterTree(x, *filter.child[0].ptr, filter_exists_patterngroup_resultset_record, kvstore);
	}
	if (filter.type == QueryTree::FilterTree::Or_type)
	{
		if (matchFilterTree(x, *filter.child[0].ptr, filter_exists_patterngroup_resultset_record, kvstore))			return true;
		else if (matchFilterTree(x, *filter.child[1].ptr, filter_exists_patterngroup_resultset_record, kvstore))		return true;
		else return false;
	}
	if (filter.type == QueryTree::FilterTree::And_type)
	{
		if (!matchFilterTree(x, *filter.child[0].ptr, filter_exists_patterngroup_resultset_record, kvstore))			return false;
		else if (!matchFilterTree(x, *filter.child[1].ptr, filter_exists_patterngroup_resultset_record, kvstore))		return false;
		return true;
	}
	if (filter.type == QueryTree::FilterTree::Equal_type)
	{
		string a, b;
		getFilterString(x, filter.child[0], a, kvstore);
		getFilterString(x, filter.child[1], b, kvstore);
		return a == b;
	}
	if (filter.type == QueryTree::FilterTree::NotEqual_type)
	{
		string a, b;
		getFilterString(x, filter.child[0], a, kvstore);
		getFilterString(x, filter.child[1], b, kvstore);
		return a != b;
	}
	if (filter.type == QueryTree::FilterTree::Less_type)
	{
		string a, b;
		getFilterString(x, filter.child[0], a, kvstore);
		getFilterString(x, filter.child[1], b, kvstore);
		return a < b;
	}
	if (filter.type == QueryTree::FilterTree::LessOrEqual_type)
	{
		string a, b;
		getFilterString(x, filter.child[0], a, kvstore);
		getFilterString(x, filter.child[1], b, kvstore);
		return a <= b;
	}
	if (filter.type == QueryTree::FilterTree::Greater_type)
	{
		string a, b;
		getFilterString(x, filter.child[0], a, kvstore);
		getFilterString(x, filter.child[1], b, kvstore);
		return a > b;
	}
	if (filter.type == QueryTree::FilterTree::GreaterOrEqual_type)
	{
		string a, b;
		getFilterString(x, filter.child[0], a, kvstore);
		getFilterString(x, filter.child[1], b, kvstore);
		return a >= b;
	}

	if (filter.type == QueryTree::FilterTree::Builtin_regex_type)
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

	if (filter.type == QueryTree::FilterTree::Builtin_langmatches_type)
	{
		string a, b;
		getFilterString(x, filter.child[0], a, kvstore);
		getFilterString(x, filter.child[1], b, kvstore);
		return a == b || (a.length() > 2 && b == "\"*\"");
	}

	if (filter.type == QueryTree::FilterTree::Builtin_bound_type)
	{
		return filter.child[0].pos != -1;
	}

	if (filter.type == QueryTree::FilterTree::Builtin_in_type)
	{
		string a;
		getFilterString(x, filter.child[0], a, kvstore);

		for (int i = 1; i < (int)filter.child.size(); i++)
			if (filter.child[i].type == 's' && a == filter.child[i].arg)
				return true;
		return false;
	}

	if (filter.type == QueryTree::FilterTree::Builtin_exists_type)
	{
		int id = filter.exists_patterngroup_id;
		for (int i = 0; i < (int)filter_exists_patterngroup_resultset_record.resultset[id]->results.size(); i++)
		{
			if (((int)filter_exists_patterngroup_resultset_record.resultset[id]->results[i].res.size() > 0) &&
				((int)filter_exists_patterngroup_resultset_record.common[id][i].varset.size() == 0 ||
				filter_exists_patterngroup_resultset_record.resultset[id]->results[i].findLeftBounder(
					filter_exists_patterngroup_resultset_record.common2resultset[id][i].first, x,
					filter_exists_patterngroup_resultset_record.common2resultset[id][i].second) != -1))
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

	//printf("temp result:\n");
	for (int i = 0; i < (int)this->res.size(); i++)
	{
		for (int j = 0; j < varnum; j++)
			printf("%d ", this->res[i][j]);
		//printf("\n");
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
	TempResultSet* temp = new TempResultSet;
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

void GeneralEvaluation::TempResultSet::doFilter(QueryTree::FilterTree& filter, FilterExistsPatternGroupResultSetRecord &filter_exists_patterngroup_resultset_record, TempResultSet &r, KVstore *kvstore)
{
	for (int i = 0; i < (int)this->results.size(); i++)
	{
		filter_exists_patterngroup_resultset_record.common.clear();
		filter_exists_patterngroup_resultset_record.common2resultset.clear();

		Varset &var_r = this->results[i].var;

		for (int j = 0; j < (int)filter_exists_patterngroup_resultset_record.resultset.size(); j++)
		{
			filter_exists_patterngroup_resultset_record.common.push_back(vector<Varset>());
			filter_exists_patterngroup_resultset_record.common2resultset.push_back(vector< pair< vector<int>, vector<int> > >());

			for (int k = 0; k < (int)filter_exists_patterngroup_resultset_record.resultset[j]->results.size(); k++)
			{
				TempResult &filter_result = filter_exists_patterngroup_resultset_record.resultset[j]->results[k];

				Varset common = var_r * filter_result.var;
				filter_exists_patterngroup_resultset_record.common[j].push_back(common);
				vector<int> common2filter_result = common.mapTo(filter_result.var);
				filter_exists_patterngroup_resultset_record.common2resultset[j].push_back(make_pair(common2filter_result, common.mapTo(var_r)));

				if (common.varset.size() > 0 && filter_result.res.size() > 0)
					filter_result.sort(0, filter_result.res.size() - 1, common2filter_result);
			}
		}

		int pos_r = r.findCompatibleResult(var_r);
		this->results[i].doFilter(filter, filter_exists_patterngroup_resultset_record, r.results[pos_r], kvstore);
	}
}


void GeneralEvaluation::TempResultSet::print()
{
	//printf("total temp result : %d\n", (int)this->results.size());
	for (int i = 0; i < (int)this->results.size(); i++)
	{
		//printf("temp result no.%d\n", i);
		this->results[i].print();
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void GeneralEvaluation::generateEvaluationPlan(QueryTree::PatternGroup &patterngroup)
{
	if ((int)patterngroup.patterns.size() == 0 && (int)patterngroup.unions.size() == 0 && (int)patterngroup.optionals.size() == 0)
	{
		TempResultSet* temp = new TempResultSet();
		temp->results.push_back(TempResult());
		this->evaluationPlan.push_back(EvaluationUnit('r', temp));

		return;
	}

	int current_pattern = 0, current_unions = 0;
	Varset current_result_varset;

	patterngroup.addOneOptionalOrMinus(' ');	//for convenience

	for (int i = 0; i < (int)patterngroup.optionals.size(); i++)
	{
		if (current_pattern <= patterngroup.optionals[i].lastpattern || current_unions <= patterngroup.optionals[i].lastunions)
		{
			vector < pair<char, int> > node_info;
			vector <Varset> node_varset;
			vector < vector<int> > edge;

			if ((int)current_result_varset.varset.size() > 0)
			{
				node_info.push_back(make_pair('r', -1));
				node_varset.push_back(current_result_varset);
			}

			while (current_pattern <= patterngroup.optionals[i].lastpattern)
			{
				int current_blockid = patterngroup.pattern_blockid[current_pattern];
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
						node_varset.push_back(this->_sparql_query_varset[current_blockid]);
					}
				}
				current_pattern ++;
			}

			while (current_unions <= patterngroup.optionals[i].lastunions)
			{
				node_info.push_back(make_pair('u', current_unions));
				Varset varset = patterngroup.unions[current_unions][0].patterngroup_varset;
				for (int j = 1; j < (int)patterngroup.unions[current_unions].size(); j++)
					varset = varset * patterngroup.unions[current_unions][j].patterngroup_varset;
				node_varset.push_back(varset);
				current_unions ++;
			}

			for (int i = 0; i < (int)node_varset.size(); i++)
			{
				edge.push_back(vector<int>());
				for (int j = 0; j < (int)node_varset.size(); j++)
					if (i != j && (int)(node_varset[i] * node_varset[j]).varset.size() > 0)
						edge[i].push_back(j);
			}

			for (int i = 0; i < (int)node_info.size(); i++)
				if (node_info[i].first != 'v')					//visited
				{
					dfsJoinableResultGraph(i, node_info, edge, patterngroup);
					if (i != 0)
						this->evaluationPlan.push_back(EvaluationUnit('j'));
				}

			for (int i = 0; i < (int)node_varset.size(); i++)
				current_result_varset = current_result_varset + node_varset[i];
		}

		if (i + 1 != (int)patterngroup.optionals.size())
		{
			generateEvaluationPlan(patterngroup.optionals[i].patterngroup);
			if (i != 0 || patterngroup.optionals[i].lastpattern != -1 || patterngroup.optionals[i].lastunions != -1)
				this->evaluationPlan.push_back(EvaluationUnit(patterngroup.optionals[i].type));
		}
	}

	patterngroup.optionals.pop_back();

	for (int i = 0; i < (int)patterngroup.filters.size(); i++)
	{
		for (int j = 0; j < (int)patterngroup.filter_exists_patterngroups[i].size(); j++)
			generateEvaluationPlan(patterngroup.filter_exists_patterngroups[i][j]);

		this->evaluationPlan.push_back(EvaluationUnit('f', &patterngroup.filters[i]));
	}
}


void GeneralEvaluation::dfsJoinableResultGraph(int x, vector < pair<char, int> > &node_info, vector < vector<int> > &edge, QueryTree::PatternGroup &patterngroup)
{
	if (node_info[x].first == 'b')
	{
		int blockid = node_info[x].second;

		TempResultSet* temp = new TempResultSet();
		temp->results.push_back(TempResult());
		temp->results[0].var = this->_sparql_query_varset[blockid];

		int varnum = (int)temp->results[0].var.varset.size();

		std::vector<int*>& basicquery_result =this->_sparql_query.getBasicQuery(blockid).getResultList();
		int basicquery_result_num = (int)basicquery_result.size();

		temp->results[0].res.reserve(basicquery_result_num);
		for (int i = 0; i < basicquery_result_num; i++)
		{
			int* result_vec = new int [varnum];
			memcpy(result_vec, basicquery_result[i], sizeof(int) * varnum);
			temp->results[0].res.push_back(result_vec);
		}
		this->evaluationPlan.push_back(EvaluationUnit('r', temp));
	}
	if (node_info[x].first == 'u')
	{
		int unionsid = node_info[x].second;
		for (int i = 0; i < (int)patterngroup.unions[unionsid].size(); i++)
		{
			generateEvaluationPlan(patterngroup.unions[unionsid][i]);
			if (i > 0)	this->evaluationPlan.push_back(EvaluationUnit('u'));
		}
	}

	node_info[x].first = 'v';

	for (int i = 0; i < (int)edge[x].size(); i++)
	{
		int y = edge[x][i];
		if (node_info[y].first != 'v')
		{
			dfsJoinableResultGraph(y, node_info, edge, patterngroup);
			this->evaluationPlan.push_back(EvaluationUnit('j'));
		}
	}
}

int GeneralEvaluation::countFilterExistsPatternGroup(QueryTree::FilterTree& filter)
{
	int count = 0;
	if (filter.type == QueryTree::FilterTree::Builtin_exists_type)
		count = 1;
	for (int i = 0; i < (int)filter.child.size(); i++)
		if (filter.child[i].type == 't')
			count += countFilterExistsPatternGroup(*filter.child[i].ptr);
	return count;
}

void GeneralEvaluation::doEvaluationPlan()
{
	for (int i = 0; i < (int)this->evaluationPlan.size(); i++)
	{
		if (evaluationPlan[i].getType() == 'r')
			this->evaluationStack.push((TempResultSet*)evaluationPlan[i].getPointer());
		if (evaluationPlan[i].getType() == 'j' || evaluationPlan[i].getType() == 'o' || evaluationPlan[i].getType() == 'm' || evaluationPlan[i].getType() == 'u')
		{
			TempResultSet* b = evaluationStack.top();
			evaluationStack.pop();
			TempResultSet* a = evaluationStack.top();
			evaluationStack.pop();
			TempResultSet* r = new TempResultSet;

			if (evaluationPlan[i].getType() == 'j')
				a->doJoin(*b, *r);
			if (evaluationPlan[i].getType() == 'o')
				a->doOptional(*b, *r);
			if (evaluationPlan[i].getType() == 'm')
				a->doMinus(*b, *r);
			if (evaluationPlan[i].getType() == 'u')
				a->doUnion(*b, *r);

			a->release();
			b->release();
			delete a;
			delete b;

			evaluationStack.push(r);
		}

		if (evaluationPlan[i].getType() == 'f')
		{
			int filter_exists_patterngroup_size = countFilterExistsPatternGroup(*(QueryTree::FilterTree *)evaluationPlan[i].getPointer());

			if (filter_exists_patterngroup_size > 0)
				for (int i = 0; i < filter_exists_patterngroup_size; i++)
				{
					this->filter_exists_patterngroup_resultset_record.resultset.push_back(evaluationStack.top());
					evaluationStack.pop();
				}

			TempResultSet* a = evaluationStack.top();
			evaluationStack.pop();

			TempResultSet * r = new TempResultSet;

			a->doFilter(*(QueryTree::FilterTree *)evaluationPlan[i].getPointer(), this->filter_exists_patterngroup_resultset_record, *r, this->kvstore);

			if (filter_exists_patterngroup_size > 0)
			{
				for (int i = 0; i < filter_exists_patterngroup_size; i++)
					this->filter_exists_patterngroup_resultset_record.resultset[i]->release();

				this->filter_exists_patterngroup_resultset_record.resultset.clear();
			}

			a->release();
			delete a;

			evaluationStack.push(r);
		}
	}
}


void GeneralEvaluation::getFinalResult(ResultSet& result_str)
{
	TempResultSet* results_id = this->evaluationStack.top();
	this->evaluationStack.pop();

	if (this->_query_tree.getQueryForm() == QueryTree::Select_Query)
	{
		if (this->_query_tree.getProjectionModifier() == QueryTree::Modifier_Distinct)
		{
			TempResultSet* results_id_distinct = new TempResultSet;

			results_id->doDistinct(this->_query_tree.getProjection(), *results_id_distinct);

			results_id->release();
			delete results_id;

			results_id = results_id_distinct;
		}

		if (this->_query_tree.checkProjectionAsterisk())
			for (int i = 0 ; i < (int)results_id->results.size(); i++)
			{
				this->_query_tree.getProjection() = this->_query_tree.getProjection() + results_id->results[i].var;
			}

		int var_num = this->_query_tree.getProjectionNum();

		result_str.select_var_num = var_num;
		result_str.setVar(this->_query_tree.getProjection().varset);

		for (int i = 0; i < (int)results_id->results.size(); i++)
			result_str.ansNum += (int)results_id->results[i].res.size();

#ifndef STREAM_ON
		result_str.answer = new string* [result_str.ansNum];
		for (int i = 0; i < result_str.ansNum; i++)
			result_str.answer[i] = NULL;
#else
		vector <int> keys;
		vector <bool> desc;
		for (int i = 0; i < (int)this->_query_tree.getOrder().size(); i++)
		{
			int var_id = Varset(this->_query_tree.getOrder()[i].var).mapTo(this->_query_tree.getProjection())[0];
			if (var_id != -1)
			{
				keys.push_back(var_id);
				desc.push_back(this->_query_tree.getOrder()[i].descending);
			}
		}
		result_str.openStream(keys, desc, this->_query_tree.getOffset(), this->_query_tree.getLimit());
#endif

		int current_result = 0;
		for (int i = 0; i < (int)results_id->results.size(); i++)
		{
			vector<int> result_str2id = this->_query_tree.getProjection().mapTo(results_id->results[i].var);
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
	else if (this->_query_tree.getQueryForm() == QueryTree::Ask_Query)
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

