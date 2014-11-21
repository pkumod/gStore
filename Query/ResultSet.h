/*
 * ResultSet.h
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 */

#ifndef RESULTSET_H_
#define RESULTSET_H_

#include<string>
#include<vector>
#include "../Bstr/Bstr.h"
using namespace std;

class ResultSet{
public:
	int select_var_num;
	int ansNum;
	string*  var_name;
	string** answer;


	ResultSet();
	~ResultSet();
	ResultSet(int _v_num, const string* _v_names);

	/* convert to binary string */
	Bstr* to_bstr();

	/* convert to usual string */
	string to_str();

	/*  */
	void setVar(const std::vector<string> & _var_names);
};


#endif /* RESULTSET_H_ */
