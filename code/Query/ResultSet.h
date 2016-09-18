/*=============================================================================
# Filename: ResultSet.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-24 21:57
# Description: originally written by liyouhuan, modified by zengli
=============================================================================*/

#ifndef _QUERY_RESULTSET_H
#define _QUERY_RESULTSET_H

#include "../Bstr/Bstr.h"
#include "../Util/Util.h"
#include "../Util/Stream.h"

class ResultSet
{
private:
#ifdef STREAM_ON
	Stream stream;
#endif
public:
	int select_var_num;
	int ansNum;
	std::string*  var_name;
#ifndef STREAM_ON
	std::string** answer;
#endif

	ResultSet();
	~ResultSet();
	ResultSet(int _v_num, const std::string* _v_names);

	//convert to binary string 
	//Bstr* to_bstr();

	//convert to usual string
	std::string to_str();
	void output(FILE* _fp);		//output all results using Stream
	void setVar(const std::vector<std::string> & _var_names);

	//operations on private stream from caller
	void openStream();
	void resetStream();
	void writeToStream(std::string& _s);
	std::string readFromStream();
};

#endif //_QUERY_RESULTSET_H

