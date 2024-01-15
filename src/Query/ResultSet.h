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
#include "TempResult.h"

class ResultSet
{
private:
	Stream* stream;
	bool useStream;
	std::string username;
public:
	int select_var_num;	// Include order by keys
	int true_select_var_num;	// Exclude order by keys
	std::string* var_name;
	unsigned ansNum;
	std::string** answer;
	int output_offset, output_limit;
	ResultSet();
	~ResultSet();
	ResultSet(int _v_num, const std::string* _v_names);

	void setUseStream();
	bool checkUseStream();
	void setOutputOffsetLimit(int _output_offset, int _output_limit);

	//convert to binary string 
	//Bstr* to_bstr();

	//convert to TSV string
	std::string to_str();
	//convert to JSON string
	std::string to_JSON();
	TempResult to_tempresult();
	void output(FILE* _fp);		//output all results using Stream
	void prettyPrint();
	void setVar(const std::vector<std::string> & _var_names);

	//operations on private stream from caller
	void openStream(std::vector<unsigned> &_keys, std::vector<bool> &_desc);
	void resetStream();
	void writeToStream(std::string& _s);
	const Bstr* getOneRecord();
	void setUsername(const std::string& _username);
	std::string getUsername();
};

#endif //_QUERY_RESULTSET_H

