/*=============================================================================
# Filename: Varset.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-03-02 20:35
# Description: implement functions in Varset.h
=============================================================================*/

#include "Varset.h"

using namespace std;

Varset::Varset(string & _var)
{
	addVar(_var);
}

Varset::Varset(vector<string> & _varset)
{
	for (int i = 0; i < (int)_varset.size(); i++)
		addVar(_varset[i]);
}

bool Varset::findVar(string& _var)
{
	if ((int)this->varset.size() == 0)	return false;
	vector<string>::iterator i = find(this->varset.begin(), this->varset.end(), _var);
	return (i != this->varset.end());
}

void Varset::addVar(string& _var)
{
	if (!this->findVar(_var))
		this->varset.push_back(_var);
}

Varset Varset::operator + (Varset& x)
{
	Varset r;
	for (int i = 0; i < (int)this->varset.size(); i++)
		r.addVar(this->varset[i]);
	for (int i = 0; i < (int)x.varset.size(); i++)
		r.addVar(x.varset[i]);
	return r;
};

Varset Varset::operator * (Varset& x)
{
	Varset r;
	for (int i = 0; i < (int)this->varset.size(); i++)
		if (x.findVar(this->varset[i]))
			r.addVar(this->varset[i]);
	return r;
}

Varset Varset::operator - (Varset& x)
{
	Varset r;
	for (int i = 0; i < (int)this->varset.size(); i++)
		if (!x.findVar(this->varset[i]))
			r.addVar(this->varset[i]);
	return r;
}

bool Varset::operator ==(Varset &x)
{
	if ((int)this->varset.size() != (int)x.varset.size())	return false;
	for (int i = 0; i < (int)this->varset.size(); i++)
		if (!x.findVar(this->varset[i]))	return false;
	return true;
}

bool Varset::hasCommonVar(Varset &x)
{
	for (int i = 0; i < (int)this->varset.size(); i++)
		if (x.findVar(this->varset[i]))	return true;
	return false;
}

bool Varset::belongTo(Varset &x)
{
	for (int i = 0; i < (int)this->varset.size(); i++)
		if (!x.findVar(this->varset[i]))	return false;
	return true;
}

vector <int> Varset::mapTo(Varset& x)
{
	vector<int> r;
	for (int i = 0; i < (int)this->varset.size(); i++)
	{
		r.push_back(-1);
		for (int j = 0; j < (int)x.varset.size(); j++)
			if (this->varset[i] == x.varset[j])
				r[i] = j;
	}
	return r;
}

void Varset::print()
{
	printf("Varset: ");
	for (int i = 0; i < (int)this->varset.size(); i++)
	{
		printf("%s ", this->varset[i].c_str());
	}
	printf("\n");
}

