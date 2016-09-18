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
#else
	this->stream = NULL;
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
#else
	delete this->stream;    //maybe NULL
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
#ifdef STREAM_ON
	this->stream = NULL;
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

//convert to usual string
string 
ResultSet::to_str()
{
	if(this->ansNum == 0)
	{
		return "[empty result]\n";
	}

	stringstream _buf;

//#ifdef DEBUG_PRECISE
	_buf << "There has answer: " << this->ansNum << endl;
	_buf << this->var_name[0];
	for(int i = 1; i < this->select_var_num; i ++)
	{
		_buf << "\t" << this->var_name[i];
	}
	_buf << "\n";
//#endif
#ifndef STREAM_ON
	for(int i = 0; i < this->ansNum; i++)
	{
		if (this->output_limit != -1 && i == this->output_offset + this->output_limit)
			break;
#ifdef DEBUG_PRECISE
		printf("to_str: well!\n");	//just for debug!
#endif	//DEBUG_PRECISE
		if (i >= this->output_offset)
		{
			_buf << this->answer[i][0];
			for(int j = 1; j < this->select_var_num; j++)
			{
				//there may be ' ' in spo, but no '\t'
				_buf << "\t" << this->answer[i][j];
				//_buf << " " << this->answer[i][j];
			}
			_buf << "\n";
		}
	}
#ifdef DEBUG_PRECISE
	printf("to_str: ends!\n");		//just for debug!
#endif	//DEBUG_PRECISE

#else	//STREAM_ON
	printf("using stream to produce to_str()!\n");
	_buf << this->readAllFromStream();
#endif	//STREAM_ON
	return _buf.str();
}

void
ResultSet::output(FILE* _fp)
{
#ifdef STREAM_ON
	fprintf(_fp, "%s", this->var_name[0].c_str());
	for(int i = 1; i < this->select_var_num; i++)
	{
		fprintf(_fp, "\t%s", this->var_name[i].c_str());
	}
	fprintf(_fp, "\n");


	if(this->ansNum == 0)
	{
		fprintf(_fp, "[empty result]\n");
		return;
	}
	const Bstr* bp;
	for(int i = 0; i < this->ansNum; i++)
	{
		if (this->output_limit != -1 && i == this->output_offset + this->output_limit)
			break;
		bp = this->stream->read();
		if (i >= this->output_offset)
		{
			fprintf(_fp, "%s", bp[0].getStr());
			//fprintf(_fp, "%s", bp->getStr());
			for(int j = 1; j < this->select_var_num; j++)
			{
				fprintf(_fp, "\t%s", bp[j].getStr());
				//bp = this->stream.read();
				//fprintf(_fp, "\t%s", bp->getStr());
			}
			fprintf(_fp, "\n");
		}
	}		
#endif
}

void
ResultSet::openStream(std::vector<int> &_keys, std::vector<bool> &_desc, int _output_offset, int _output_limit)
{
#ifdef STREAM_ON
#ifdef DEBUG_STREAM
	vector<int> debug_keys;
	vector<bool> debug_desc;
	for(int i = 0; i < this->select_var_num; ++i)
	{
		debug_keys.push_back(i);
		debug_desc.push_back(false);
	}
#endif
	if(this->stream != NULL)
	{
		delete this->stream;
		this->stream = NULL;
	}
#ifdef DEBUG_STREAM
	if(this->ansNum > 0)
		this->stream = new Stream(debug_keys, debug_desc, this->ansNum, this->select_var_num, true);
#else
	if(this->ansNum > 0)
		this->stream = new Stream(_keys, _desc, this->ansNum, this->select_var_num, _keys.size() > 0);
#endif  //DEBUG_STREAM
#endif  //STREAM_ON
	this->output_offset = _output_offset;
	this->output_limit = _output_limit;
}

void
ResultSet::resetStream()
{
#ifdef STREAM_ON
	//this->stream.reset();
	if(this->stream != NULL)
		this->stream->setEnd();
#endif
}

void
ResultSet::writeToStream(string& _s)
{
#ifdef STREAM_ON
	if(this->stream != NULL)
		this->stream->write(_s.c_str(), _s.length());	
#endif
}

//QUERY: how to manage when large?
string
ResultSet::readAllFromStream()
{
	stringstream buf;
#ifdef STREAM_ON
	if(this->stream == NULL)
		return "";

	this->resetStream();
	const Bstr* bp;
	for(int i = 0; i < this->ansNum; i++)
	{
		if (this->output_limit != -1 && i == this->output_offset + this->output_limit)
			break;
		bp = this->stream->read();
		if (i >= this->output_offset)
		{
			buf << bp[0].getStr();
			for(int j = 1; j < this->select_var_num; ++j)
			{
				buf << "\t"	<< bp[j].getStr();
			}

			//buf << bp->getStr();
			//for(int j = 1; j < this->select_var_num; j++)
			//{
				//bp = this->stream.read();
				//buf << "\t" << bp->getStr();
			//}
			buf << "\n";
		}
	}		
#endif
	return buf.str();
}

const Bstr*
ResultSet::getOneRecord()
{
#ifdef STREAM_ON
	if(this->stream == NULL)
	{
		fprintf(stderr, "ResultSet::getOneRecord(): no results now!\n");
		return NULL;
	}
	if(this->stream->isEnd())
	{
		fprintf(stderr, "ResultSet::getOneRecord(): read till end now!\n");
		return NULL;
	}
	//NOTICE:this is one record, and donot free the memory!
	//NOTICE:Bstr[] but only one element, used as Bstr*
	return this->stream->read();
#else
	return NULL;
#endif
}

