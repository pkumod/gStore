/*
 * ResultSet.cpp
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 */

#include "ResultSet.h"
#include <sstream>

ResultSet::ResultSet()
{
	this->select_var_num = 0;
	this->var_name = NULL;
	this->ansNum = 0;
	this->answer = NULL;
}
ResultSet::~ResultSet()
{
	delete[] this->var_name;
	for(int i = 0; i < this->ansNum; i ++)
	{
		delete[] this->answer[i];
	}
	delete[] this->answer;
}
ResultSet::ResultSet(int _v_num, const string* _v_names)
{
	this->select_var_num = _v_num;
	this->var_name = new string[this->select_var_num];
	for(int i = 0; i < this->select_var_num; i ++)
	{
		this->var_name[i] = _v_names[i];
	}
}

/* convert to binary string */
Bstr* ResultSet::to_bstr()
{

	return NULL;
}

/* convert to usual string */
string ResultSet::to_str()
{
	if(this->ansNum == 0)
	{
		return "[empty result]";
	}


	std::stringstream _buf;

	//debug
//	_buf << "There has answer: " << this->ansNum << endl;
//	_buf << this->var_name[0];
//	for(int i = 1; i < this->select_var_num; i ++)
//	{
//		_buf << "\t" << this->var_name[i];
//	}
//	_buf << "\n";

	for(int i = 0; i < this->ansNum; i ++)
	{
		_buf << this->answer[i][0];
		for(int j = 1; j < this->select_var_num; j ++)
		{
			//_buf << "\t" << this->answer[i][j];
		    _buf << " " << this->answer[i][j];
		}
		_buf << "\n";
	}

	return _buf.str();
}

void ResultSet::setVar(const std::vector<string> & _var_names)
{
	this->select_var_num = _var_names.size();
	this->var_name = new string[this->select_var_num];
	for(int i = 0; i < this->select_var_num; i ++)
	{
		this->var_name[i] = _var_names[i];
	}
}


