/*=============================================================================
# Filename: ResultFilter.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-05-03 15:36
# Description: implement functions in ResultFilter.h
=============================================================================*/

#include "ResultFilter.h"

using namespace std;

void ResultFilter::addVar(string var)
{
	if (this->hash_table.count(var) == 0)
		this->hash_table.insert(pair<string, vector<int> >(var, vector<int>(this->MAX_SIZE, 0)));
}

vector<int>* ResultFilter::findVar(string var)
{
	if (this->hash_table.count(var) == 0)
		return NULL;

	return &this->hash_table[var];
}

void ResultFilter::change(SPARQLquery& query, int value)
{
	for (int i = 0; i < query.getBasicQueryNum(); i++)
	{
		BasicQuery& basicquery = query.getBasicQuery(i);
		vector<int*>& basicquery_result =basicquery.getResultList();
		int result_num = basicquery_result.size();
		int var_num = basicquery.getVarNum();

		for (int j = 0; j < var_num; j++)
			this->addVar(basicquery.getVarName(j));

		vector<vector<int>*> refer;
		for (int j = 0; j < var_num; j++)
			refer.push_back(this->findVar(basicquery.getVarName(j)));

		for (int j = 0; j < result_num; j++)
			for (int k = 0; k < var_num; k++)
			{
				(*refer[k])[this->hash(basicquery_result[j][k])] += value;
			}
	}
}

void ResultFilter::candFilter(SPARQLquery& query)
{
	for (int i = 0; i < query.getBasicQueryNum(); i++)
	{
		BasicQuery& basicquery = query.getBasicQuery(i);
		for (int j = 0; j < basicquery.getVarNum(); j++)
		{
			vector<int>* col = this->findVar(basicquery.getVarName(j));
			if (col != NULL)
			{
				IDList& idlist = basicquery.getCandidateList(j);
				IDList new_idlist;

				printf("candFilter on %s\n", basicquery.getVarName(j).c_str());
				printf("before candFilter, size = %d\n", idlist.size());

				for (int k = 0; k < idlist.size(); k++)
				{
					int id = idlist.getID(k);
					if ((*col)[hash(id)] > 0)
					{
						new_idlist.addID(id);
					}
				}
				idlist = new_idlist;

				printf("after candFilter, size = %d\n", idlist.size());
			}
		}
	}
}
