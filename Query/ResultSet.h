/*=============================================================================
# Filename: ResultSet.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-24 21:57
# Description: originally written by liyouhuan, modified by zengli
=============================================================================*/

#ifndef _QUERY_RESULTSET_H
#define _QUERY_RESULTSET_H

#include "../Util/Util.h"
#include "../Util/Bstr.h"
#include "../Util/Stream.h"

class ResultSet
{
private:
	Stream* stream;
	bool useStream;
public:
	int select_var_num;
	int ansNum;
	std::string*  var_name;
	int output_offset, output_limit;
	std::string** answer;

	ResultSet();
	~ResultSet();
	ResultSet(int _v_num, const std::string* _v_names);

	void setUseStream();
	inline bool checkUseStream()
	{
		return this->useStream;
	}

	//convert to binary string 
	//Bstr* to_bstr();

	//convert to usual string
	std::string to_str();
	void output(FILE* _fp);		//output all results using Stream
	void setVar(const std::vector<std::string> & _var_names);

	//operations on private stream from caller
	void openStream(std::vector<int> &_keys, std::vector<bool> &_desc, int _output_offset, int _output_limit);
	void resetStream();
	void writeToStream(std::string& _s);
	std::string readAllFromStream();
	const Bstr* getOneRecord();
};

#endif //_QUERY_RESULTSET_H

