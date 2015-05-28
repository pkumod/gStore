/*
 * gload.cpp
 *
 *  Created on: 2014-7-31
 *      Author: liyouhuan
 */
#include<iostream>
#include "../Database/Database.h"

using namespace std;
/*
 * [0]./gload [1]data_folder_path  [2]rdf_file_path
 */
int main(int argc, char * argv[])
{
	system("clock");
	cout << "gload..." << endl;
	{
		cout << "argc: " << argc << "\t";
		cout << "DB_store:" << argv[1] << "\t";
		cout << "RDF_data: " << argv[2] << "\t";
		cout << endl;
	}

	string _db_path = string(argv[1]);
	string _rdf = string(argv[2]);
	Database _db(_db_path);
	bool flag = _db.build(_rdf);
    if (flag)
    {
        cout << "import RDF file to database done." << endl;
    }
    else
    {
        cout << "import RDF file to database failed." << endl;
    }
	system("clock");
	return 0;
}



