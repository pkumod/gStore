/*=============================================================================
# Filename: TempResult.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-08-14
# Description: 
=============================================================================*/

#ifndef _QUERY_TEMPRESULT_H
#define _QUERY_TEMPRESULT_H

#include "../Util/Util.h"
#include "../StringIndex/StringIndex.h"
#include "QueryTree.h"
#include "RegexExpression.h"
#include "Varset.h"
#include "../Util/EvalMultitypeValue.h"

class TempResult
{
	public:
		class ResultPair
		{
			public:
				unsigned *id;
				std::vector<std::string> str;
				ResultPair():id(NULL){}
		};

		Varset id_varset, str_varset;
		std::vector<ResultPair> result;

		Varset getAllVarset();

		void release();

		static int compareRow(const ResultPair &x, const int x_id_cols, const std::vector<int> &x_pos,
							  const ResultPair &y, const int y_id_cols, const std::vector<int> &y_pos);
		void sort(int l, int r, const std::vector<int> &this_pos);
		int findLeftBounder(const std::vector<int> &this_pos, const ResultPair &x, const int x_id_cols, const std::vector<int> &x_pos) const;
		int findRightBounder(const std::vector<int> &this_pos, const ResultPair &x, const int x_id_cols, const std::vector<int> &x_pos) const;

		void convertId2Str(Varset convert_varset, StringIndex *stringindex, Varset &entity_literal_varset);
		void doJoin(TempResult &x, TempResult &r);
		void doUnion(TempResult &r);
		void doOptional(std::vector<bool> &binding, TempResult &x, TempResult &rn, TempResult &ra, bool add_no_binding);
		void doMinus(TempResult &x, TempResult &r);

		void getFilterString(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild &child, EvalMultitypeValue &femv, ResultPair &row, int id_cols, StringIndex *stringindex);
		EvalMultitypeValue matchFilterTree(QueryTree::GroupPattern::FilterTree::FilterTreeNode &filter, ResultPair &row, int id_cols, StringIndex *stringindex);
		void doFilter(QueryTree::GroupPattern::FilterTree::FilterTreeNode &filter, TempResult &r, StringIndex *stringindex, Varset &entity_literal_varset);

		EvalMultitypeValue doComp(QueryTree::CompTreeNode *root, ResultPair &row, int id_cols, StringIndex *stringindex, Varset &entity_literal_varset);

		void print(int no=-1);
};

class TempResultSet
{
	public:
		std::vector<TempResult> results;

		void release();

		int findCompatibleResult(Varset &_id_varset, Varset &_str_varset);

		void doJoin(TempResultSet &x, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset);
		void doUnion(TempResultSet &x, TempResultSet &r);
		void doOptional(TempResultSet &x, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset);
		void doMinus(TempResultSet &x, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset);
		void doFilter(QueryTree::GroupPattern::FilterTree::FilterTreeNode &filter, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset);

		void doProjection1(Varset &proj, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset);
		void doDistinct1(TempResultSet &r);

		void print();
};

#endif // _QUERY_TEMPRESULT_H

