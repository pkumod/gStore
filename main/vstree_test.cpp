/*
 * vstree_test.cpp
 *
 *  Created on: 2014-7-17
 *      Author: liyouhuan
 */
#include<iostream>
#include "../Database/Database.h"

using namespace std;
int main()
{
	cout << "vstree_test" << endl;
	string _yago_example = "/media/wip/common/data/yago_example";
	string _rdf = _yago_example;
	Database _db("db_test");
	_db.build(_rdf);
	Database::log("finish build");

	return 0;
}
