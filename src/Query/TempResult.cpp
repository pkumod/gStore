/*=============================================================================
# Filename: TempResult.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-08-14
# Description: implement functions in TempResult.h
=============================================================================*/

#include "TempResult.h"

using namespace std;

TempResult::ResultPair::ResultPair()
{
	id = NULL;
	sz = 0;
}

TempResult::ResultPair::ResultPair(const ResultPair& that)
{
	if (that.id)
	{
		id = new unsigned[that.sz];
		// TODO: ResultPair cannot access id_varset of its parent TempResult
		memcpy(id, that.id, (that.sz) * sizeof(unsigned));
	}else{
		id = nullptr;
	}	sz = that.sz;
	str = that.str;
}

TempResult::ResultPair& TempResult::ResultPair::operator=(const ResultPair& that)
{
	if (that.id)
	{
		// unsigned *local_id = new unsigned[that.id_varset.getVarsetSize()];
		unsigned *local_id = new unsigned[that.sz];
		memcpy(local_id, that.id, (that.sz) * sizeof(unsigned));
		if (id)
			delete[] id;	// Prevent exception leaving original id in limbo
		id = local_id;
	}
	else
	{
		if (id)
			delete[] id;
		id = NULL;
	}
	sz = that.sz;
	str = that.str;

	return *this;
}

TempResult::TempResult()
{
	string_index_buffer = NULL;
	string_index_buffer_size = 0;
}

TempResult::TempResult(const TempResult& that)
{
	id_varset = that.id_varset;
	str_varset = that.str_varset;
	result = that.result;
	string_index_buffer = NULL;
	string_index_buffer_size = 0;
}

TempResult::~TempResult()
{
	if (this->string_index_buffer != NULL)
	{
		delete[] this->string_index_buffer;
		this->string_index_buffer = NULL;
	}
	this->string_index_buffer_size = 0;
}

TempResult& TempResult::operator=(const TempResult& that)
{
	id_varset = that.id_varset;
	str_varset = that.str_varset;
	result = that.result;

	return *this;
}

TempResultSet::TempResultSet()
{
	initial = true;
}

TempResultSet::TempResultSet(const TempResultSet& that)
{
	results = that.results;
	initial = that.initial;
}

TempResultSet& TempResultSet::operator=(const TempResultSet& that)
{
	results = that.results;
	initial = that.initial;

	return *this;
}

Varset TempResult::getAllVarset()
{
	return this->id_varset + this->str_varset;
}

void TempResult::release()
{
	for (int i = 0; i < (int)this->result.size(); i++)
	{
		if (result[i].id)
			delete[] result[i].id;
		vector<string>().swap(result[i].str);
	}
}

int TempResult::compareRow(const ResultPair &x, const int x_id_cols, const vector<int> &x_pos,
		  	  	  	  	  	  	  	  	  	  const ResultPair &y, const int y_id_cols, const vector<int> &y_pos)
{
	for (int i = 0; i < min((int)x_pos.size(), (int)y_pos.size()); i++)
	{
		int px = x_pos[i], py = y_pos[i];

		if (px < x_id_cols && py < y_id_cols)
		{
			if (x.id[px] < y.id[py])
				return -1;
			if (x.id[px] > y.id[py])
				return 1;
		}

		if (px >= x_id_cols && py >= y_id_cols)
		{
			if (x.str[px - x_id_cols] < y.str[py - y_id_cols])
				return -1;
			if (x.str[px - x_id_cols] > y.str[py - y_id_cols])
				return 1;
		}
	}

	return 0;
}

void TempResult::sort(int l, int r, const vector<int> &this_pos)
{
	if (r <= l)
		return;
	int i = l, j = r + 1;
	swap(this->result[l].id, this->result[(l + r) / 2].id);
	swap(this->result[l].str, this->result[(l + r) / 2].str);
	ResultPair &m = this->result[l];

	int this_id_cols = this->id_varset.getVarsetSize();

	while (true)
	{
		while (compareRow(this->result[++i], this_id_cols, this_pos, m, this_id_cols, this_pos) == -1)
		{
			if (i == r)
				break;
		}
		while (compareRow(m, this_id_cols, this_pos, this->result[--j], this_id_cols, this_pos) == -1)
		{
			if (j == l)
				break;
		}
		if (i >= j)
			break;
		swap(this->result[i].id, this->result[j].id);
		swap(this->result[i].str, this->result[j].str);
	}
	swap(m.id, this->result[j].id);
	swap(m.str, this->result[j].str);

	sort(l, j - 1, this_pos);
	sort(j + 1, r, this_pos);
}

int TempResult::findLeftBounder(const vector<int> &this_pos, const ResultPair &x, const int x_id_cols, const vector<int> &x_pos) const
{
	int l = 0, r = (int)this->result.size() - 1;
	if (r == -1)
		return -1;

	int this_id_cols = this->id_varset.getVarsetSize();
	while (l < r)
	{
		int m = (l + r) / 2;
		if (compareRow(this->result[m], this_id_cols, this_pos, x, x_id_cols, x_pos) >= 0)		r = m;
		else l = m + 1;
	}

	if (compareRow(this->result[l], this_id_cols, this_pos, x, x_id_cols, x_pos) == 0)
		return l;
	else
		return -1;
}

int TempResult::findRightBounder(const vector<int> &this_pos, const ResultPair &x, const int x_id_cols, const vector<int> &x_pos) const
{
	int l = 0, r = (int)this->result.size() - 1;

	if (r == -1)
		return -1;

	int this_id_cols = this->id_varset.getVarsetSize();
	while (l < r)
	{
		int m = (l + r) / 2 + 1;
		if (compareRow(this->result[m], this_id_cols, this_pos, x, x_id_cols, x_pos) <= 0)		l = m;
		else r = m - 1;
	}

	if (compareRow(this->result[l], this_id_cols, this_pos, x, x_id_cols, x_pos) == 0)
		return l;
	else
		return -1;
}

void TempResult::convertId2Str(Varset convert_varset, StringIndex *stringindex, Varset &entity_literal_varset)
{
	int this_id_cols = this->id_varset.getVarsetSize();

	Varset new_id_varset = this->id_varset - convert_varset;
	Varset new_str_varset = this->str_varset + convert_varset;
	int new_id_cols = new_id_varset.getVarsetSize();

	vector<int> this2new_id_pos = this->id_varset.mapTo(new_id_varset);

	for (int i = 0; i < (int)this->result.size(); i++)
	{
		unsigned *v = new unsigned [new_id_cols];

		for (int k = 0; k < this_id_cols; k++)
			if (this2new_id_pos[k] != -1)
			{
				v[this2new_id_pos[k]] = this->result[i].id[k];
			}
			else
			{
				string str;

				if (entity_literal_varset.findVar(this->id_varset.vars[k]))
					stringindex->randomAccess(this->result[i].id[k], &str, string_index_buffer, string_index_buffer_size, true);
				else
					stringindex->randomAccess(this->result[i].id[k], &str, string_index_buffer, string_index_buffer_size, false);

				this->result[i].str.push_back(str);
			}

		delete[] this->result[i].id;
		this->result[i].id = v;
        this->result[i].sz = new_id_cols;
	}

	this->id_varset = new_id_varset;
	this->str_varset = new_str_varset;
}

void TempResult::doJoin(TempResult &x, TempResult &r)
{
	// long large_begin, large_end;
	// long small_begin, small_end;
	// large_begin = Util::get_cur_time();
	// small_begin = Util::get_cur_time();

	int this_id_cols = this->id_varset.getVarsetSize();
	int x_id_cols = x.id_varset.getVarsetSize();

	int r_id_cols = r.id_varset.getVarsetSize();
	vector<int> this2r_id_pos = this->id_varset.mapTo(r.id_varset);
	vector<int> x2r_id_pos = x.id_varset.mapTo(r.id_varset);

	int this_str_cols = this->str_varset.getVarsetSize();
	int x_str_cols = x.str_varset.getVarsetSize();

	int r_str_cols = r.str_varset.getVarsetSize();
	vector<int> this2r_str_pos = this->str_varset.mapTo(r.str_varset);
	vector<int> x2r_str_pos = x.str_varset.mapTo(r.str_varset);

	Varset common = this->getAllVarset() * x.getAllVarset();

	// long totalJoinTime = 0, totalSortTime = 0, totalFindBounderTime = 0, totalPreTime = 0;
	// int iterLen = 0;

	// small_end = Util::get_cur_time();
	// totalPreTime = small_end - small_begin;

	if (common.empty())
	{
		// printf("Res size = %d, ", (int)this->result.size() * (int)x.result.size());
		r.result.resize(this->result.size() * x.result.size());
		// r.result.assign(this->result.size() * x.result.size(), TempResult::ResultPair(r_str_cols));
		bool x_larger = x.result.size() > this->result.size();
		if (r_id_cols > 0)
		{
			size_t curr = 0;

			if (x_larger)
			{
				for (int i = 0; i < (int)this->result.size(); i++)
					for (int j = 0; j < (int)x.result.size(); j++)
					{
						r.result[curr].id = new unsigned [r_id_cols];
						r.result[curr].sz = r_id_cols;
						unsigned *v = r.result[curr].id;

						for (int k = 0; k < this_id_cols; k++)
							v[this2r_id_pos[k]] = this->result[i].id[k];
						for (int k = 0; k < x_id_cols; k++)
							v[x2r_id_pos[k]] = x.result[j].id[k];

						curr++;
					}
			}
			else
			{
				for (int j = 0; j < (int)x.result.size(); j++)
					for (int i = 0; i < (int)this->result.size(); i++)
					{
						r.result[curr].id = new unsigned [r_id_cols];
						r.result[curr].sz = r_id_cols;
						unsigned *v = r.result[curr].id;

						for (int k = 0; k < this_id_cols; k++)
							v[this2r_id_pos[k]] = this->result[i].id[k];
						for (int k = 0; k < x_id_cols; k++)
							v[x2r_id_pos[k]] = x.result[j].id[k];

						curr++;
					}
			}
		}
		if (r_str_cols > 0)
		{
			size_t curr = 0;
			if (x_larger)
			{
				for (int i = 0; i < (int)this->result.size(); i++)
					for (int j = 0; j < (int)x.result.size(); j++)
					{
						r.result[curr].str.resize(r_str_cols);
						vector<string> &v = r.result[curr].str;

						for (int k = 0; k < this_str_cols; k++)
							// copy(this->result[i].str[k].begin(), this->result[i].str[k].end(), v[this2r_str_pos[k]].begin());
							v[this2r_str_pos[k]] = std::move(this->result[i].str[k]);
							// v[this2r_str_pos[k]] = this->result[i].str[k];	// move
						for (int k = 0; k < x_str_cols; k++)
							// copy(x.result[j].str[k].begin(), x.result[j].str[k].end(), v[x2r_str_pos[k]].begin());
							v[x2r_str_pos[k]] = std::move(x.result[j].str[k]);
							// v[x2r_str_pos[k]] = x.result[j].str[k];	// move

						curr++;
					}
			}
			else
			{
				for (int j = 0; j < (int)x.result.size(); j++)
					for (int i = 0; i < (int)this->result.size(); i++)
					{
						r.result[curr].str.resize(r_str_cols);
						vector<string> &v = r.result[curr].str;

						for (int k = 0; k < this_str_cols; k++)
							// copy(this->result[i].str[k].begin(), this->result[i].str[k].end(), v[this2r_str_pos[k]].begin());
							v[this2r_str_pos[k]] = std::move(this->result[i].str[k]);
							// v[this2r_str_pos[k]] = this->result[i].str[k];	// move
						for (int k = 0; k < x_str_cols; k++)
							// copy(x.result[j].str[k].begin(), x.result[j].str[k].end(), v[x2r_str_pos[k]].begin());
							v[x2r_str_pos[k]] = std::move(x.result[j].str[k]);
							// v[x2r_str_pos[k]] = x.result[j].str[k];	// move

						curr++;
					}
			}
		}

		// for (int i = 0; i < (int)this->result.size(); i++)
		// 	for (int j = 0; j < (int)x.result.size(); j++)
		// 	{
		// 		r.result.push_back(ResultPair());

		// 		if (r_id_cols > 0)	// out of for
		// 		{
		// 			r.result.back().id = new unsigned [r_id_cols];
		// 			r.result.back().sz = r_id_cols;
		// 			unsigned *v = r.result.back().id;

		// 			for (int k = 0; k < this_id_cols; k++)
		// 				v[this2r_id_pos[k]] = this->result[i].id[k];
		// 			for (int k = 0; k < x_id_cols; k++)
		// 				v[x2r_id_pos[k]] = x.result[j].id[k];
		// 		}

		// 		if (r_str_cols > 0)
		// 		{
		// 			r.result.back().str.resize(r_str_cols);	// remove
		// 			vector<string> &v = r.result.back().str;

		// 			for (int k = 0; k < this_str_cols; k++)
		// 				v[this2r_str_pos[k]] = this->result[i].str[k];	// move
		// 			for (int k = 0; k < x_str_cols; k++)
		// 				v[x2r_str_pos[k]] = x.result[j].str[k];	// move
		// 		}
		// 	}
	}
	else if (!x.result.empty())
	{
		vector<int> common2this = common.mapTo(this->getAllVarset());
		vector<int> common2x = common.mapTo(x.getAllVarset());

		// small_begin = Util::get_cur_time();
		x.sort(0, (int)x.result.size() - 1, common2x);
		// small_end = Util::get_cur_time();
		// totalSortTime += small_end - small_begin;

		int this_id_cols = this->id_varset.getVarsetSize();
		for (int i = 0; i < (int)this->result.size(); i++)
		{
			// small_begin = Util::get_cur_time();
			int left = x.findLeftBounder(common2x, this->result[i], this_id_cols, common2this);
			if (left == -1)	continue;
			int right = x.findRightBounder(common2x, this->result[i], this_id_cols, common2this);
			// iterLen += right - left + 1;
			// small_end = Util::get_cur_time();
			// totalFindBounderTime += small_end - small_begin;

			// small_begin = Util::get_cur_time();
			for (int j = left; j <= right; j++)
			{
				r.result.push_back(ResultPair());

				if (r_id_cols > 0)
				{
					r.result.back().id = new unsigned [r_id_cols];
					r.result.back().sz = r_id_cols;
					unsigned *v = r.result.back().id;

					for (int k = 0; k < this_id_cols; k++)
						v[this2r_id_pos[k]] = this->result[i].id[k];
					for (int k = 0; k < x_id_cols; k++)
						v[x2r_id_pos[k]] = x.result[j].id[k];
				}

				if (r_str_cols > 0)
				{
					r.result.back().str.resize(r_str_cols);
					vector<string> &v = r.result.back().str;

					for (int k = 0; k < this_str_cols; k++)
						v[this2r_str_pos[k]] = this->result[i].str[k];
					for (int k = 0; k < x_str_cols; k++)
						v[x2r_str_pos[k]] = x.result[j].str[k];
				}
			}
			// small_end = Util::get_cur_time();
			// totalJoinTime += small_end - small_begin;
		}
	}
	// If x.result.empty(), do nothing
	// large_end = Util::get_cur_time();
	// if (common.empty())
	// 	printf("Total time %ld ms\n", large_end - large_begin);
	// printf("Total time %ld ms, common %d, join %ld ms, sort %ld ms, find bounder %ld ms, iterLen = %d, r_id_cols = %d, r_str_cols = %d\n", large_end - large_begin, common.empty(), totalJoinTime, totalSortTime, totalFindBounderTime, iterLen, r_id_cols, r_str_cols);
}

void TempResult::doUnion(TempResult &r)
{
	int this_id_cols = this->id_varset.getVarsetSize();

	int r_id_cols = r.id_varset.getVarsetSize();
	vector<int> this2r_id_pos = this->id_varset.mapTo(r.id_varset);

	int this_str_cols = this->str_varset.getVarsetSize();

	int r_str_cols = r.str_varset.getVarsetSize();
	vector<int> this2r_str_pos = this->str_varset.mapTo(r.str_varset);

	for (int i = 0; i < (int)this->result.size(); i++)
	{
		r.result.push_back(ResultPair());

		if (r_id_cols > 0)
		{
			r.result.back().id = new unsigned [r_id_cols];
			r.result.back().sz = r_id_cols;
			unsigned *v = r.result.back().id;

			for (int k = 0; k < this_id_cols; k++)
				v[this2r_id_pos[k]] = this->result[i].id[k];
		}

		if (r_str_cols > 0)
		{
			r.result.back().str.resize(r_str_cols);
			vector<string> &v = r.result.back().str;

			for (int k = 0; k < this_str_cols; k++)
				v[this2r_str_pos[k]] = this->result[i].str[k];
		}
	}
}

void TempResult::doOptional(vector<bool> &binding, TempResult &x, TempResult &rn, TempResult &ra, bool add_no_binding)
{
	int this_id_cols = this->id_varset.getVarsetSize();
	int x_id_cols = x.id_varset.getVarsetSize();

	int rn_id_cols = rn.id_varset.getVarsetSize();
	vector<int> this2rn_id_pos = this->id_varset.mapTo(rn.id_varset);
	vector<int> x2rn_id_pos = x.id_varset.mapTo(rn.id_varset);

	int ra_id_cols = ra.id_varset.getVarsetSize();
	vector<int> this2ra_id_pos = this->id_varset.mapTo(ra.id_varset);
	vector<int> x2ra_id_pos = x.id_varset.mapTo(ra.id_varset);

	int this_str_cols = this->str_varset.getVarsetSize();
	int x_str_cols = x.str_varset.getVarsetSize();

	int rn_str_cols = rn.str_varset.getVarsetSize();
	vector<int> this2rn_str_pos = this->str_varset.mapTo(rn.str_varset);
	vector<int> x2rn_str_pos = x.str_varset.mapTo(rn.str_varset);

	int ra_str_cols = ra.str_varset.getVarsetSize();
	vector<int> this2ra_str_pos = this->str_varset.mapTo(ra.str_varset);
	vector<int> x2ra_str_pos = x.str_varset.mapTo(ra.str_varset);

	Varset common = this->getAllVarset() * x.getAllVarset();

	if (common.empty())
	{
		this->doJoin(x, ra);

		binding.resize((int)this->result.size());
		for (int i = 0; i < (int)this->result.size(); i++)
			binding[i] = true;	// val in resize is only for new insertions
	}
	else if (!x.result.empty())
	{
		vector<int> common2this = common.mapTo(this->getAllVarset());
		vector<int> common2x = common.mapTo(x.getAllVarset());
		x.sort(0, (int)x.result.size() - 1, common2x);

		int this_id_cols = this->id_varset.getVarsetSize();
		for (int i = 0; i < (int)this->result.size(); i++)
		{
			int left = x.findLeftBounder(common2x, this->result[i], this_id_cols, common2this);
			if (left == -1)	continue;
			int right = x.findRightBounder(common2x, this->result[i], this_id_cols, common2this);

			binding[i] = true;

			for (int j = left; j <= right; j++)
			{
				ra.result.push_back(ResultPair());

				if (ra_id_cols > 0)
				{
					ra.result.back().id = new unsigned [ra_id_cols];
					ra.result.back().sz = ra_id_cols;
					unsigned *v = ra.result.back().id;

					for (int k = 0; k < this_id_cols; k++)
						v[this2ra_id_pos[k]] = this->result[i].id[k];
					for (int k = 0; k < x_id_cols; k++)
						v[x2ra_id_pos[k]] = x.result[j].id[k];
				}

				if (ra_str_cols > 0)
				{
					ra.result.back().str.resize(ra_str_cols);
					vector<string> &v = ra.result.back().str;

					for (int k = 0; k < this_str_cols; k++)
						v[this2ra_str_pos[k]] = this->result[i].str[k];
					for (int k = 0; k < x_str_cols; k++)
						v[x2ra_str_pos[k]] = x.result[j].str[k];
				}
			}
		}
	}

	if (add_no_binding)
	{
		for (int i = 0; i < (int)this->result.size(); i++)
			if (!binding[i])
			{
				rn.result.push_back(ResultPair());

				if (rn_id_cols > 0)
				{
					rn.result.back().id = new unsigned [rn_id_cols];
					rn.result.back().sz = rn_id_cols;
					unsigned *v = rn.result.back().id;

					for (int k = 0; k < this_id_cols; k++)
						v[this2rn_id_pos[k]] = this->result[i].id[k];
				}

				if (rn_str_cols > 0)
				{
					rn.result.back().str.resize(rn_str_cols);
					vector<string> &v = rn.result.back().str;

					for (int k = 0; k < this_str_cols; k++)
						v[this2rn_str_pos[k]] = this->result[i].str[k];
				}
			}
	}
}

void TempResult::doMinus(TempResult &x, TempResult &r)
{
	if (x.result.empty())
		r = *this;
	
	int this_id_cols = this->id_varset.getVarsetSize();

	int r_id_cols = r.id_varset.getVarsetSize();
	vector<int> this2r_id_pos = this->id_varset.mapTo(r.id_varset);

	int this_str_cols = this->str_varset.getVarsetSize();

	int r_str_cols = r.str_varset.getVarsetSize();
	vector<int> this2r_str_pos = this->str_varset.mapTo(r.str_varset);

	Varset common = this->getAllVarset() * x.getAllVarset();

	if (common.empty())
	{
		for (int i = 0; i < (int)this->result.size(); i++)
		{
			r.result.push_back(ResultPair());

			if (r_id_cols > 0)
			{
				r.result.back().id = new unsigned [r_id_cols];
				r.result.back().sz = r_id_cols;
				unsigned *v = r.result.back().id;

				for (int k = 0; k < this_id_cols; k++)
					v[this2r_id_pos[k]] = this->result[i].id[k];
			}

			if (r_str_cols > 0)
			{
				r.result.back().str.resize(r_str_cols);
				vector<string> &v = r.result.back().str;

				for (int k = 0; k < this_str_cols; k++)
					v[this2r_str_pos[k]] = this->result[i].str[k];
			}
		}
	}
	else if (!x.result.empty())
	{
		vector<int> common2this = common.mapTo(this->getAllVarset());
		vector<int> common2x = common.mapTo(x.getAllVarset());
		x.sort(0, (int)x.result.size() - 1, common2x);

		int this_id_cols = this->id_varset.getVarsetSize();
		for (int i = 0; i < (int)this->result.size(); i++)
		{
			int left = x.findLeftBounder(common2x, this->result[i], this_id_cols, common2this);
			if (left == -1)
			{
				r.result.push_back(ResultPair());

				if (r_id_cols > 0)
				{
					r.result.back().id = new unsigned [r_id_cols];
					r.result.back().sz = r_id_cols;
					unsigned *v = r.result.back().id;

					for (int k = 0; k < this_id_cols; k++)
						v[this2r_id_pos[k]] = this->result[i].id[k];
				}

				if (r_str_cols > 0)
				{
					r.result.back().str.resize(r_str_cols);
					vector<string> &v = r.result.back().str;

					for (int k = 0; k < this_str_cols; k++)
						v[this2r_str_pos[k]] = this->result[i].str[k];
				}
			}
		}
	}
}

EvalMultitypeValue
TempResult::doComp(const CompTreeNode &root, ResultPair &row, int id_cols, KVstore *kvstore, Varset &this_varset)
{
	// Arithmetic and logical operations
	// if (root->lchild == NULL && root->rchild == NULL)	// leaf node
	if (root.children.size() == 0)	// leaf node
	{
		EvalMultitypeValue x;
		x.datatype = EvalMultitypeValue::rdf_term;
		if (root.val[0] == '?')	// variable
		{
			int pos;
			pos = Varset(root.val).mapTo(this_varset)[0];
			if (pos == -1)
			{
				x.datatype = EvalMultitypeValue::xsd_boolean;
				x.bool_value = EvalMultitypeValue::EffectiveBooleanValue::error_value;
				return x;
			}
			if (pos < id_cols)
			{
				int id = row.id[pos];
                x.term_value = kvstore->getStringByID(id);
			}
			else
				x.term_value = row.str[pos - id_cols];
		} else if (root.oprt == "NOW")
		{
			cout << "IN NOW" << endl;
			time_t now = time(0);
			tm *lctm = localtime(&now);
			x.datatype = EvalMultitypeValue::xsd_datetime;
			x.dt_value = EvalMultitypeValue::DateTime(1900 + lctm->tm_year, 1 + lctm->tm_mon, \
				lctm->tm_mday, 5+lctm->tm_hour, lctm->tm_min, lctm->tm_sec);
			return x;
		}
		else  	// literal
			x.term_value = root.val;
		x.deduceTypeValue();
		// cout << "x.term_value = " << x.term_value << endl;
		return x;
	}
	else if (root.children.size() == 1 && \
		(root.oprt == "+" || root.oprt == "-" || root.oprt == "!"))	// unary operator
	{
		EvalMultitypeValue lRes = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		if (root.oprt == "+")
			return lRes;
		else if (root.oprt == "-")
			return -lRes;
		else if (root.oprt == "!")
			return !lRes;
	}
	else if (root.children.size() == 2 && \
		(root.oprt == "||" || root.oprt == "&&" || root.oprt == "=" \
		|| root.oprt == "!=" || root.oprt == "<" || root.oprt == ">" \
		|| root.oprt == "<=" || root.oprt == ">=" || root.oprt == "+" \
		|| root.oprt == "-" || root.oprt == "*" || root.oprt == "/"))	// binary operator
	{
		EvalMultitypeValue lRes, rRes;
		lRes = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		if (lRes.datatype == EvalMultitypeValue::xsd_boolean) {
			if ((root.oprt == "||" && lRes.bool_value.getValue() == 1) || (root.oprt == "&&" && lRes.bool_value.getValue() != 1))
				return lRes;
		} else if (root.oprt == "&&")
			return lRes;
		rRes = doComp(root.children[1], row, id_cols, kvstore, this_varset);

		if (root.oprt == "||")
			return lRes || rRes;
		else if (root.oprt == "&&")
			return lRes && rRes;
		else if (root.oprt == "=")
			return lRes == rRes;
		else if (root.oprt == "!=")
			return lRes != rRes;
		else if (root.oprt == "<")
			return lRes < rRes;
		else if (root.oprt == ">")
			return lRes > rRes;
		else if (root.oprt == "<=")
			return lRes <= rRes;
		else if (root.oprt == ">=")
			return lRes >= rRes;
		else if (root.oprt == "+")
			return lRes + rRes;
		else if (root.oprt == "-")
			return lRes - rRes;
		else if (root.oprt == "*")
			return lRes * rRes;
		else if (root.oprt == "/")
			return lRes / rRes;
	}

	// Built-in function calls
	EvalMultitypeValue ret_femv;
	ret_femv.datatype = EvalMultitypeValue::xsd_boolean;
	ret_femv.bool_value = EvalMultitypeValue::EffectiveBooleanValue::error_value;

	if (root.oprt == "REGEX")
	{
		EvalMultitypeValue x, y, z;
		string t, p, f;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		if (x.datatype == EvalMultitypeValue::literal || x.datatype == EvalMultitypeValue::xsd_string)
		{
			t = x.str_value;
			t = t.substr(1, t.rfind('"') - 1);
		}
		else
			return ret_femv;
		y = doComp(root.children[1], row, id_cols, kvstore, this_varset);
		if (y.isSimpleLiteral())
		{
			p = y.str_value;
			p = p.substr(1, p.rfind('"') - 1);
		}
		else
			return ret_femv;
		if (root.children.size() >= 3)
		{
			z = doComp(root.children[2], row, id_cols, kvstore, this_varset);
			if (z.isSimpleLiteral())
			{
				f = z.str_value;
				f = f.substr(1, f.rfind('"') - 1);
			}
			else
				return ret_femv;
		}

		RegexExpression re;
		if (!re.compile(p, f))
			ret_femv.bool_value = EvalMultitypeValue::EffectiveBooleanValue::false_value;
		else
			ret_femv.bool_value = re.match(t);

		return ret_femv;
	} else if (root.oprt == "CONCAT") {
		size_t numChild = root.children.size();
		EvalMultitypeValue x;
		ret_femv.str_value = "";
		for (size_t i = 0; i < numChild; i++) {
			x = doComp(root.children[i], row, id_cols, kvstore, this_varset);
			if (x.datatype == EvalMultitypeValue::literal 
			|| x.datatype == EvalMultitypeValue::xsd_string) {
				size_t bIdx = x.str_value.find('\"'), eIdx = x.str_value.rfind('\"');
				if (bIdx == string::npos || eIdx == string::npos || bIdx == eIdx)
					ret_femv.str_value += x.str_value;
				else
					ret_femv.str_value += x.str_value.substr(bIdx + 1, eIdx - bIdx - 1);
			}
			else {
				ret_femv.str_value.clear();
				return ret_femv;
			}
		}
		ret_femv.str_value = '\"' + ret_femv.str_value + '\"';
		ret_femv.datatype = EvalMultitypeValue::literal;
		ret_femv.deduceTermValue();
		return ret_femv;
	}
	else if (root.oprt == "STR")
	{
		EvalMultitypeValue x;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		if (x.datatype == EvalMultitypeValue::literal)
			ret_femv.str_value = x.str_value.substr(0, x.str_value.rfind('"') + 1);
		else if (x.datatype == EvalMultitypeValue::iri)
			ret_femv.str_value = "\"" + x.str_value.substr(1, x.str_value.length() - 2) + "\"";
		else if (x.datatype == EvalMultitypeValue::xsd_string)
			ret_femv.str_value = x.str_value;
		else {
			x.deduceTermValue();
			ret_femv.str_value = x.term_value;
		}
		ret_femv.datatype = EvalMultitypeValue::literal;
		return ret_femv;
	}
	else if (root.oprt == "ISIRI" || root.oprt == "ISURI")
	{
		EvalMultitypeValue x;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		ret_femv.bool_value = (x.datatype == EvalMultitypeValue::iri);

		return ret_femv;
	}
	else if (root.oprt == "ISLITERAL")
	{
		EvalMultitypeValue x;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		ret_femv.bool_value = (x.datatype == EvalMultitypeValue::literal ||
							   x.datatype == EvalMultitypeValue::xsd_string ||
							   x.datatype == EvalMultitypeValue::xsd_boolean ||
							   x.datatype == EvalMultitypeValue::xsd_integer ||
							   x.datatype == EvalMultitypeValue::xsd_decimal ||
							   x.datatype == EvalMultitypeValue::xsd_float ||
							   x.datatype == EvalMultitypeValue::xsd_double ||
							   x.datatype == EvalMultitypeValue::xsd_datetime);

		return ret_femv;
	}
	else if (root.oprt == "ISNUMERIC")
	{
		EvalMultitypeValue x;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		ret_femv.bool_value = (x.datatype == EvalMultitypeValue::xsd_integer ||
							   x.datatype == EvalMultitypeValue::xsd_decimal ||
							   x.datatype == EvalMultitypeValue::xsd_float ||
							   x.datatype == EvalMultitypeValue::xsd_double);

		return ret_femv;
	}
	else if (root.oprt == "LANG")
	{
		EvalMultitypeValue x;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		if (x.datatype == EvalMultitypeValue::literal)
		{
			ret_femv.datatype = EvalMultitypeValue::literal;
			int p = x.str_value.rfind('@');
			if (p != -1)
				ret_femv.str_value = "\"" + x.str_value.substr(p + 1) + "\"";
			else
				ret_femv.str_value = "";
			return ret_femv;
		}
		else
			return ret_femv;
	}
	else if (root.oprt == "DATATYPE")
	{
		EvalMultitypeValue x;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		if (x.datatype == EvalMultitypeValue::rdf_term)
		{
			size_t p = x.str_value.rfind("^^");
			if (p != string::npos)
			{
				ret_femv.datatype = EvalMultitypeValue::iri;
				ret_femv.term_value = x.str_value.substr(p + 2);
			}
		}
		else if (x.datatype == EvalMultitypeValue::literal)
		{
			ret_femv.datatype = EvalMultitypeValue::iri;
			size_t p = x.str_value.rfind('@');
			if (p != string::npos)
				ret_femv.term_value = "<http://www.w3.org/1999/02/22-rdf-syntax-ns#langString>";
			else
				ret_femv.term_value = "<http://www.w3.org/2001/XMLSchema#string>";
		}
		else if (x.datatype == EvalMultitypeValue::xsd_string)
		{
			ret_femv.datatype = EvalMultitypeValue::iri;
			ret_femv.term_value = "<http://www.w3.org/2001/XMLSchema#string>";
		}
		else if (x.datatype == EvalMultitypeValue::xsd_boolean)
		{
			ret_femv.datatype = EvalMultitypeValue::iri;
			ret_femv.term_value = "<http://www.w3.org/2001/XMLSchema#boolean>";
		}
		else if (x.datatype == EvalMultitypeValue::xsd_integer)
		{
			ret_femv.datatype = EvalMultitypeValue::iri;
			ret_femv.term_value = "<http://www.w3.org/2001/XMLSchema#integer>";
		}
		else if (x.datatype == EvalMultitypeValue::xsd_decimal)
		{
			ret_femv.datatype = EvalMultitypeValue::iri;
			ret_femv.term_value = "<http://www.w3.org/2001/XMLSchema#decimal>";
		}
		else if (x.datatype == EvalMultitypeValue::xsd_float)
		{
			ret_femv.datatype = EvalMultitypeValue::iri;
			ret_femv.term_value = "<http://www.w3.org/2001/XMLSchema#float>";
		}
		else if (x.datatype == EvalMultitypeValue::xsd_double)
		{
			ret_femv.datatype = EvalMultitypeValue::iri;
			ret_femv.term_value = "<http://www.w3.org/2001/XMLSchema#double>";
		}
		else if (x.datatype == EvalMultitypeValue::xsd_datetime)
		{
			ret_femv.datatype = EvalMultitypeValue::iri;
			ret_femv.term_value = "<http://www.w3.org/2001/XMLSchema#dateTime>";
		}

		return ret_femv;
	}
	else if (root.oprt == "YEAR")
	{
		EvalMultitypeValue x;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		if (x.datatype == EvalMultitypeValue::xsd_datetime)
		{
			ret_femv.datatype = EvalMultitypeValue::xsd_integer;
			ret_femv.int_value = x.dt_value.date[0];
			return ret_femv;
		}
		else
			return ret_femv;
	}
	else if (root.oprt == "MONTH")
	{
		EvalMultitypeValue x;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		if (x.datatype == EvalMultitypeValue::xsd_datetime)
		{
			ret_femv.datatype = EvalMultitypeValue::xsd_integer;
			ret_femv.int_value = x.dt_value.date[1];
			return ret_femv;
		}
		else
			return ret_femv;
	}
	else if (root.oprt == "DAY")
	{
		EvalMultitypeValue x;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		if (x.datatype == EvalMultitypeValue::xsd_datetime)
		{
			ret_femv.datatype = EvalMultitypeValue::xsd_integer;
			ret_femv.int_value = x.dt_value.date[2];
			return ret_femv;
		}
		else
			return ret_femv;
	}
	else if (root.oprt == "HOURS")
	{
		EvalMultitypeValue x;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		if (x.datatype == EvalMultitypeValue::xsd_datetime)
		{
			ret_femv.datatype = EvalMultitypeValue::xsd_integer;
			ret_femv.int_value = x.dt_value.date[3];
			return ret_femv;
		}
		else
			return ret_femv;
	}
	else if (root.oprt == "MINUTES")
	{
		EvalMultitypeValue x;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		if (x.datatype == EvalMultitypeValue::xsd_datetime)
		{
			ret_femv.datatype = EvalMultitypeValue::xsd_integer;
			ret_femv.int_value = x.dt_value.date[4];
			return ret_femv;
		}
		else
			return ret_femv;
	}
	else if (root.oprt == "UCASE" || root.oprt == "LCASE")
	{
		EvalMultitypeValue x;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		string langTag = "";
		if (x.datatype == EvalMultitypeValue::xsd_string)
		{
			ret_femv.datatype = EvalMultitypeValue::xsd_string;
			ret_femv.str_value = x.str_value;
		}
		else if (x.datatype == EvalMultitypeValue::literal)
		{
			ret_femv.datatype = EvalMultitypeValue::literal;
			int p = x.str_value.rfind('@');
			if (p != -1)
			{
				ret_femv.str_value = x.str_value.substr(0, p);	// ADD lang tag!
				langTag = x.str_value.substr(p + 1);
			}
			else
				ret_femv.str_value = x.str_value;
		}

		if (root.oprt == "UCASE")
			transform(ret_femv.str_value.begin(), ret_femv.str_value.end(), \
				ret_femv.str_value.begin(), ::toupper);
		else 	// Builtin_lcase_type
			transform(ret_femv.str_value.begin(), ret_femv.str_value.end(), \
				ret_femv.str_value.begin(), ::tolower);
		if (langTag != "")
			ret_femv.str_value += "@" + langTag;

		return ret_femv;
	}
	else if (root.oprt == "CONTAINS" || root.oprt == "STRSTARTS")
	{
		EvalMultitypeValue x, y;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		y = doComp(root.children[1], row, id_cols, kvstore, this_varset);
		if(x.argCompatible(y))
		{
			string x_content = x.getStrContent(), y_content = y.getStrContent();
			size_t p = x_content.find(y_content);
			if ((root.oprt == "CONTAINS" && p != string::npos) || (root.oprt == "STRSTARTS" && p == 0))
				ret_femv.bool_value = EvalMultitypeValue::EffectiveBooleanValue::true_value;
			else
				ret_femv.bool_value = EvalMultitypeValue::EffectiveBooleanValue::false_value;
		}

		return ret_femv;
	}
	else if (root.oprt == "ABS")
	{
		EvalMultitypeValue x;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		if (x.datatype == EvalMultitypeValue::xsd_integer)
		{
			ret_femv.datatype = EvalMultitypeValue::xsd_integer;
			ret_femv.int_value = abs(x.int_value);
		}
		else if (x.datatype == EvalMultitypeValue::xsd_decimal)
		{
			ret_femv.datatype = EvalMultitypeValue::xsd_decimal;
			ret_femv.flt_value = fabs(x.flt_value);
		}
		else if (x.datatype == EvalMultitypeValue::xsd_float)
		{
			ret_femv.datatype = EvalMultitypeValue::xsd_float;
			ret_femv.flt_value = fabs(x.flt_value);
		}
		else if (x.datatype == EvalMultitypeValue::xsd_double)
		{
			ret_femv.datatype = EvalMultitypeValue::xsd_double;
			ret_femv.dbl_value = fabs(x.dbl_value);
		}

		return ret_femv;
	}
	else if (root.oprt == "LANGMATCHES")
	{
		EvalMultitypeValue x, y;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		if (!x.isSimpleLiteral())
			return ret_femv;
		y = doComp(root.children[1], row, id_cols, kvstore, this_varset);
		if (!y.isSimpleLiteral())
			return ret_femv;

		ret_femv.bool_value = ((x.str_value == y.str_value) || (x.str_value.length() > 0 && y.str_value == "\"*\""));

		return ret_femv;
	}
	else if (root.oprt == "BOUND")
	{
		EvalMultitypeValue x;
		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		// Only return false when x is xsd_boolean and has error_value
		if (x.datatype == EvalMultitypeValue::xsd_boolean && x.bool_value.getValue() == 2)
			ret_femv.bool_value = EvalMultitypeValue::EffectiveBooleanValue::false_value;
		else
			ret_femv.bool_value = EvalMultitypeValue::EffectiveBooleanValue::true_value;

		return ret_femv;
	}
	else if (root.oprt == "IN")
	{
		EvalMultitypeValue x;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		ret_femv.bool_value.value = EvalMultitypeValue::EffectiveBooleanValue::false_value;
		for (int i = 1; i < (int)root.children.size(); i++)
		{
			EvalMultitypeValue y;
			y = doComp(root.children[1], row, id_cols, kvstore, this_varset);
			EvalMultitypeValue equal = (x == y);
			if (i == 1)
				ret_femv = equal;
			else
				ret_femv = (ret_femv || equal);

			if (ret_femv.datatype == EvalMultitypeValue::xsd_boolean && ret_femv.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
				return ret_femv;
		}

		return ret_femv;
	}
	else if (root.oprt == "NOT IN")
	{
		EvalMultitypeValue x;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		ret_femv.bool_value.value = EvalMultitypeValue::EffectiveBooleanValue::true_value;
		for (int i = 1; i < (int)root.children.size(); i++)
		{
			EvalMultitypeValue y;
			y = doComp(root.children[1], row, id_cols, kvstore, this_varset);
			EvalMultitypeValue inequal = (x != y);
			if (i == 1)
				ret_femv = inequal;
			else
				ret_femv = (ret_femv && inequal);

			if (ret_femv.datatype == EvalMultitypeValue::xsd_boolean && ret_femv.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::false_value)
				return ret_femv;
		}

		return ret_femv;
	}
	else if (root.oprt == "IF")
	{
		EvalMultitypeValue x, y, z;

		x = doComp(root.children[0], row, id_cols, kvstore, this_varset);
		if ((x.datatype == EvalMultitypeValue::xsd_boolean && x.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::error_value)
			|| x.datatype != EvalMultitypeValue::xsd_boolean)
			return ret_femv;
		if (x.datatype == EvalMultitypeValue::xsd_boolean && x.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
			ret_femv = doComp(root.children[1], row, id_cols, kvstore, this_varset);
		else
			ret_femv = doComp(root.children[2], row, id_cols, kvstore, this_varset);
		
		return ret_femv;
	}

	return ret_femv;
}

void TempResult::doFilter(const CompTreeNode &filter, KVstore *kvstore, Varset &entity_literal_varset, unsigned limit_number) {
    unsigned original_size = this->result.size();
    unsigned delete_num = 0, save_num = 0;

    Varset this_varset = this->getAllVarset();
    int this_id_cols = this->id_varset.getVarsetSize();

    for (unsigned i = 0; save_num < limit_number && i < original_size-delete_num;) {
        EvalMultitypeValue ret_femv = doComp(filter, this->result[i], this_id_cols, kvstore, this_varset);
        if (ret_femv.datatype == EvalMultitypeValue::xsd_boolean && ret_femv.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value) {
            ++i;
            ++save_num;
        } else {
            swap(this->result[i], this->result[original_size - 1 - delete_num]);
            ++delete_num;
        }
    }

    this->result.erase(this->result.begin()+(save_num), this->result.end());
    this->result.shrink_to_fit();
}
void TempResult::doBind(const GroupPattern::Bind &bind, KVstore *kvstore, Varset &entity_literal_varset)
{
	Varset this_varset = this->getAllVarset();
	int this_id_cols = this->id_varset.getVarsetSize();
	this->str_varset.addVar(bind.var);
	for (int i = 0; i < (int)this->result.size(); i++)
	{
		EvalMultitypeValue ret_femv = doComp(bind.bindExpr, this->result[i], this_id_cols, kvstore, this_varset);
		this->result[i].str.push_back(ret_femv.getRep());
	}
	return;
}

void TempResult::print(int no)
{
	this->getAllVarset().print();

	if (no == -1)
		printf("temp result:\n");
	else
		printf("temp result no.%d:\n", no);

	for (int i = 0; i < (int)this->result.size(); i++)
	{
		printf("[%d]\n", i);
		printf("id_varset: ");
		id_varset.print();
		for (int j = 0; j < this->id_varset.getVarsetSize(); j++)
			printf("%d\t", this->result[i].id[j]);
		printf("\n");
		printf("str_varset: ");
		str_varset.print();
		for (int j = 0; j < this->str_varset.getVarsetSize(); j++)
			printf("%s\t", this->result[i].str[j].c_str());
		printf("\n");
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TempResultSet::release()
{
	for (int i = 0; i < (int)this->results.size(); i++)
		this->results[i].release();
}

int TempResultSet::findCompatibleResult(Varset &_id_varset, Varset &_str_varset)
{
	for (int i = 0; i < (int)this->results.size(); i++)
		if (this->results[i].id_varset == _id_varset && this->results[i].str_varset == _str_varset)
			return i;

	this->results.push_back(TempResult());
	this->results.back().id_varset = _id_varset;
	this->results.back().str_varset = _str_varset;

	return (int)this->results.size() - 1;
}

void TempResultSet::doJoin(TempResultSet &x, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset)
{
	long tv_begin = Util::get_cur_time();

	// if (this->results.empty() || x.results.empty())
	if (this->results.empty() && this->initial)
	{
		this->initial = false;
		this->doUnion(x, r);
		return;
	}

	Varset this_str_varset, x_str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		this_str_varset += this->results[i].str_varset;
	for (int i = 0; i < (int)x.results.size(); i++)
		x_str_varset += x.results[i].str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		if (this->results[i].id_varset.hasCommonVar(x_str_varset))
			this->results[i].convertId2Str(this->results[i].id_varset * x_str_varset, stringindex, entity_literal_varset);
	for (int i = 0; i < (int)x.results.size(); i++)
		if (x.results[i].id_varset.hasCommonVar(this_str_varset))
			x.results[i].convertId2Str(x.results[i].id_varset * this_str_varset, stringindex, entity_literal_varset);

	// long totalFindCompTime = 0, totalInnerJoinTime = 0;
	for (int i = 0; i < (int)this->results.size(); i++)
		for (int j = 0; j < (int)x.results.size(); j++)
		{
			Varset id_varset = this->results[i].id_varset + x.results[j].id_varset;
			Varset str_varset = this->results[i].str_varset + x.results[j].str_varset;

			// long small_begin, small_end;
			// small_begin = Util::get_cur_time();
			int pos = r.findCompatibleResult(id_varset, str_varset);
			// small_end = Util::get_cur_time();
			// totalFindCompTime += small_end - small_begin;

			// small_begin = Util::get_cur_time();
			this->results[i].doJoin(x.results[j], r.results[pos]);
			// small_end = Util::get_cur_time();
			// totalInnerJoinTime += small_end - small_begin;
		}

	long tv_end = Util::get_cur_time();
	printf("after doJoin, used %ld ms.\n", tv_end - tv_begin);
	// printf("after doJoin, used %ld ms (find compatible %ld ms, inner join %ld ms).\n", tv_end - tv_begin, totalFindCompTime, totalInnerJoinTime);
}

void TempResultSet::doUnion(TempResultSet &x, TempResultSet &r)
{
	long tv_begin = Util::get_cur_time();

	for (int i = 0; i < (int)this->results.size(); i++)
	{
		int pos = r.findCompatibleResult(this->results[i].id_varset, this->results[i].str_varset);
        if (!this->results[i].result.empty())
		    this->results[i].doUnion(r.results[pos]);
	}

	for (int i = 0; i < (int)x.results.size(); i++)
	{
		int pos = r.findCompatibleResult(x.results[i].id_varset, x.results[i].str_varset);
        if (!x.results[i].result.empty())
		    x.results[i].doUnion(r.results[pos]);
	}

	long tv_end = Util::get_cur_time();
	printf("after doUnion, used %ld ms.\n", tv_end - tv_begin);
}

void TempResultSet::doOptional(TempResultSet &x, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset)
{
	long tv_begin = Util::get_cur_time();

	// if (this->results.empty() || x.results.empty())
	if (this->results.empty() && this->initial)
	{
		this->initial = false;
		this->doUnion(x, r);
		return;
	}

	Varset this_str_varset, x_str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		this_str_varset += this->results[i].str_varset;
	for (int i = 0; i < (int)x.results.size(); i++)
		x_str_varset += x.results[i].str_varset;

	// Align this and x's str varset
	for (int i = 0; i < (int)this->results.size(); i++)
		if (this->results[i].id_varset.hasCommonVar(x_str_varset))
			this->results[i].convertId2Str(this->results[i].id_varset * x_str_varset, stringindex, entity_literal_varset);
	for (int i = 0; i < (int)x.results.size(); i++)
		if (x.results[i].id_varset.hasCommonVar(this_str_varset))
			x.results[i].convertId2Str(x.results[i].id_varset * this_str_varset, stringindex, entity_literal_varset);

	for (int i = 0; i < (int)this->results.size(); i++)
	{
		vector<bool> binding((int)this->results[i].result.size(), false);

		int noadd_pos = r.findCompatibleResult(this->results[i].id_varset, this->results[i].str_varset);

		for (int j = 0; j < (int)x.results.size(); j++)
		{
			Varset add_id_varset = this->results[i].id_varset + x.results[j].id_varset;
			Varset add_str_varset = this->results[i].str_varset + x.results[j].str_varset;

			int add_pos = r.findCompatibleResult(add_id_varset, add_str_varset);
			this->results[i].doOptional(binding, x.results[j], r.results[noadd_pos], r.results[add_pos], j + 1 == (int)x.results.size());
		}

		if(x.results.size() == 0){
			this->results[i].doUnion(r.results[noadd_pos]);
		}
	}

	long tv_end = Util::get_cur_time();
	printf("after doOptional, used %ld ms.\n", tv_end - tv_begin);
}

void TempResultSet::doMinus(TempResultSet &x, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset)
{
	long tv_begin = Util::get_cur_time();

	if (x.results.empty())
	{
		this->doUnion(x, r);
		return;
	}

	Varset this_str_varset, x_str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		this_str_varset += this->results[i].str_varset;
	for (int i = 0; i < (int)x.results.size(); i++)
		x_str_varset += x.results[i].str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		if (this->results[i].id_varset.hasCommonVar(x_str_varset))
			this->results[i].convertId2Str(this->results[i].id_varset * x_str_varset, stringindex, entity_literal_varset);
	for (int i = 0; i < (int)x.results.size(); i++)
		if (x.results[i].id_varset.hasCommonVar(this_str_varset))
			x.results[i].convertId2Str(x.results[i].id_varset * this_str_varset, stringindex, entity_literal_varset);

	for (int i = 0; i < (int)this->results.size(); i++)
	{
		vector<TempResult> temp;

		for (int j = 0; j < (int)x.results.size(); j++)
		{
			if (j == 0 && j + 1 == (int)x.results.size())
			{
				int pos = r.findCompatibleResult(this->results[i].id_varset, this->results[i].str_varset);
				this->results[i].doMinus(x.results[j], r.results[pos]);
			}
			else if (j == 0)
			{
				temp.push_back(TempResult());
				temp[0].id_varset = this->results[i].id_varset;
				temp[0].str_varset = this->results[i].str_varset;
				this->results[i].doMinus(x.results[j], temp[0]);
			}
			else if (j + 1 == (int)x.results.size())
			{
				int pos = r.findCompatibleResult(this->results[i].id_varset, this->results[i].str_varset);
				temp[j - 1].doMinus(x.results[j], r.results[pos]);
			}
			else
			{
				temp.push_back(TempResult());
				temp[j].id_varset = this->results[i].id_varset;
				temp[j].str_varset = this->results[i].str_varset;
				temp[j - 1].doMinus(x.results[j], temp[j]);
			}
		}
	}

	long tv_end = Util::get_cur_time();
	printf("after doMinus, used %ld ms.\n", tv_end - tv_begin);
}

void TempResultSet::doFilter(const CompTreeNode &filter, KVstore *kvstore, Varset &entity_literal_varset, unsigned limit_num) {
    unsigned before_size = results[0].result.size();
    long tv_begin = Util::get_cur_time();

    for (int i = 0; i < (int) this->results.size(); i++) {
        this->results[i].doFilter(filter, kvstore, entity_literal_varset, limit_num);
    }

    long tv_end = Util::get_cur_time();
    unsigned after_size = results[0].result.size();
    printf("after doFilter, used %ld ms. ", tv_end - tv_begin);
    printf("before filter size %d, after filter size %d.\n", before_size, after_size);
}

void TempResultSet::doBind(const GroupPattern::Bind &bind, KVstore *kvstore, Varset &entity_literal_varset)
{
	long tv_begin = Util::get_cur_time();

	if (this->results.size() == 0)
	{
		this->results.push_back(TempResult());
		this->results.back().result.push_back(TempResult::ResultPair());
	}
	
	for (int i = 0; i < (int)this->results.size(); i++)
		this->results[i].doBind(bind, kvstore,entity_literal_varset);

	long tv_end = Util::get_cur_time();
	printf("after doBind, used %ld ms.\n", tv_end - tv_begin);
}

void TempResultSet::doProjection1(Varset &proj, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset)
{
	long tv_begin = Util::get_cur_time();

	Varset this_str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		this_str_varset += this->results[i].str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		if (this->results[i].id_varset.hasCommonVar(this_str_varset * proj))
			this->results[i].convertId2Str(this->results[i].id_varset * this_str_varset * proj, stringindex, entity_literal_varset);

	Varset r_id_varset, r_str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		r_str_varset += this->results[i].str_varset * proj;
	r_id_varset = proj - r_str_varset;

	if (!r.results.empty())
		return;

	r.findCompatibleResult(r_id_varset, r_str_varset);

	int r_id_cols = r_id_varset.getVarsetSize();
	int r_str_cols = r_str_varset.getVarsetSize();

	for (int i = 0; i < (int)this->results.size(); i++)
		if (this->results[i].id_varset.hasCommonVar(r_id_varset) || this->results[i].str_varset.hasCommonVar(r_str_varset))
		{
			vector<int> r2this_id_pos = r_id_varset.mapTo(this->results[i].id_varset);
			vector<int> r2this_str_pos = r_str_varset.mapTo(this->results[i].str_varset);

			for (int j = 0; j < (int)this->results[i].result.size(); j++)
			{
				r.results[0].result.push_back(TempResult::ResultPair());

				if (r_id_cols > 0)
				{
					r.results[0].result.back().id = new unsigned [r_id_cols];
					r.results[0].result.back().sz = r_id_cols;
					unsigned *v = r.results[0].result.back().id;

					for (int k = 0; k < r_id_cols; k++)
						if (r2this_id_pos[k] == -1)
							v[k] = INVALID;
						else
							v[k] = this->results[i].result[j].id[r2this_id_pos[k]];
				}

				if (r_str_cols > 0)
				{
					r.results[0].result.back().str.resize(r_str_cols);
					vector<string> &v = r.results[0].result.back().str;

					for (int k = 0; k < r_str_cols; k++)
						if (r2this_str_pos[k] != -1)
							v[k] = this->results[i].result[j].str[r2this_str_pos[k]];
				}
			}
		}

	long tv_end = Util::get_cur_time();
	printf("after doProjection, used %ld ms.\n", tv_end - tv_begin);
}

void TempResultSet::doDistinct1(TempResultSet &r)
{
	long tv_begin = Util::get_cur_time();

	if ((int)this->results.size() != 1)
		return;

	TempResult &this_results0 = this->results[0];

	if (!r.results.empty())
		return;

	r.findCompatibleResult(this_results0.id_varset, this_results0.str_varset);
	TempResult &r_results0 = r.results[0];

	int this_id_cols = this_results0.id_varset.getVarsetSize();

	int r_id_cols = r_results0.id_varset.getVarsetSize();
	vector<int> this2r_id_pos = this_results0.id_varset.mapTo(r_results0.id_varset);

	int this_str_cols = this_results0.str_varset.getVarsetSize();

	int r_str_cols = r_results0.str_varset.getVarsetSize();
	vector<int> this2r_str_pos = this_results0.str_varset.mapTo(r_results0.str_varset);

	if (!this_results0.result.empty())
	{
		vector<int> r2this = r_results0.getAllVarset().mapTo(this_results0.getAllVarset());
		this_results0.sort(0, (int)this_results0.result.size() - 1, r2this);

		int this_result0_id_cols = this_results0.id_varset.getVarsetSize();
		for (int i = 0; i < (int)this_results0.result.size(); i++)
			if (i == 0 || TempResult::compareRow(this_results0.result[i], this_result0_id_cols, r2this, this_results0.result[i - 1], this_result0_id_cols, r2this) != 0)
			{
				r_results0.result.push_back(TempResult::ResultPair());

				if (r_id_cols > 0)
				{
					r_results0.result.back().id = new unsigned [r_id_cols];
					r_results0.result.back().sz = r_id_cols;
					unsigned *v = r_results0.result.back().id;

					for (int k = 0; k < this_id_cols; k++)
						v[this2r_id_pos[k]] = this_results0.result[i].id[k];
				}

				if (r_str_cols > 0)
				{
					r_results0.result.back().str.resize(r_str_cols);
					vector<string> &v = r_results0.result.back().str;

					for (int k = 0; k < this_str_cols; k++)
						v[this2r_str_pos[k]] = this_results0.result[i].str[k];
				}
			}
	}

	long tv_end = Util::get_cur_time();
	printf("after doDistinct, used %ld ms.\n", tv_end - tv_begin);
}

void TempResultSet::print()
{
	printf("total temp result: %d\n", (int)this->results.size());
	for (int i = 0; i < (int)this->results.size(); i++)
		this->results[i].print(i);
}
