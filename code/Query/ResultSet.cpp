/*=============================================================================
# Filename: ResultSet.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-24 22:01
# Description: implement functions in ResultSet.h
=============================================================================*/

#include "ResultSet.h"

using namespace std;

ResultSet::ResultSet()
{
	this->select_var_num = 0;
	this->var_name = NULL;
	this->ansNum = 0;
#ifndef STREAM_ON
	this->answer = NULL;
#endif
}

ResultSet::~ResultSet()
{
	delete[] this->var_name;
#ifndef STREAM_ON
	for(int i = 0; i < this->ansNum; i ++)
	{
		delete[] this->answer[i];
	}
	delete[] this->answer;
#endif
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

//convert to usual string
string 
ResultSet::to_str()
{
	if(this->ansNum == 0)
	{
		return "[empty result]\n";
	}

	stringstream _buf;

#ifdef DEBUG_PRECISE
	_buf << "There has answer: " << this->ansNum << endl;
	_buf << this->var_name[0];
	for(int i = 1; i < this->select_var_num; i ++)
	{
		_buf << "\t" << this->var_name[i];
	}
	_buf << "\n";
#endif
#ifndef STREAM_ON
	for(int i = 0; i < this->ansNum; i++)
	{
#ifdef DEBUG_PRECISE
		printf("to_str: well!\n");	//just for debug!
#endif	//DEBUG_PRECISE
		_buf << this->answer[i][0];
		for(int j = 1; j < this->select_var_num; j++)
		{
			//there may be ' ' in spo, but no '\t'
			_buf << "\t" << this->answer[i][j];
			//_buf << " " << this->answer[i][j];
		}
		_buf << "\n";
	}
#ifdef DEBUG_PRECISE
	printf("to_str: ends!\n");		//just for debug!
#endif	//DEBUG_PRECISE
#endif	//STREAM_ON
	return _buf.str();
}

void
ResultSet::output(FILE* _fp)
{
#ifdef STREAM_ON
	if(this->ansNum == 0)
	{
		fprintf(_fp, "[empty result]\n");
	}
	const Bstr* bp;
	for(int i = 0; i < this->ansNum; i++)
	{
		bp = this->stream.read();
		fprintf(_fp, "%s", bp->getStr());
		for(int j = 1; j < this->select_var_num; j++)
		{
			bp = this->stream.read();
			fprintf(_fp, "\t%s", bp->getStr());
		}
		fprintf(_fp, "\n");
	}		
#endif
}

void 
ResultSet::setVar(const vector<string> & _var_names)
{
	this->select_var_num = _var_names.size();
	this->var_name = new string[this->select_var_num];
	for(int i = 0; i < this->select_var_num; i++)
	{
		this->var_name[i] = _var_names[i];
	}
}

void
ResultSet::openStream()
{
#ifdef STREAM_ON
	this->stream.open();
#endif
}

void
ResultSet::resetStream()
{
#ifdef STREAM_ON
	this->stream.reset();
#endif
}

void
ResultSet::writeToStream(string& _s)
{
#ifdef STREAM_ON
	//Bstr bstr(_s.c_str(), _s.length()); //this copy memory
	this->stream.write(_s.c_str(), _s.length());	
#endif
}

//const Bstr*
//ResultSet::getOneAnswer()
//{
//	//TODO
//}

//QUERY: how to manage when large?
string
ResultSet::readFromStream()
{
	stringstream buf;
#ifdef STREAM_ON
	const Bstr* bp;
	for(int i = 0; i < this->ansNum; i++)
	{
		bp = this->stream.read();
		buf << bp->getStr();
		for(int j = 1; j < this->select_var_num; j++)
		{
			bp = this->stream.read();
			buf << "\t" << bp->getStr();
		}
		buf << "\n";
	}		
#endif
	return buf.str();
}

